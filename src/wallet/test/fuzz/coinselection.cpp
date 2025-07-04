// Copyright (c) 2022 The Shahcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <policy/feerate.h>
#include <policy/policy.h>
#include <primitives/transaction.h>
#include <test/fuzz/FuzzedDataProvider.h>
#include <test/fuzz/fuzz.h>
#include <test/fuzz/util.h>
#include <test/util/setup_common.h>
#include <wallet/coinselection.h>

#include <vector>

namespace wallet {

static void AddCoin(const CAmount& value, int n_input, int n_input_bytes, int locktime, std::vector<COutput>& coins, CFeeRate fee_rate)
{
    CMutableTransaction tx;
    tx.vout.resize(n_input + 1);
    tx.vout[n_input].nValue = value;
    tx.nLockTime = locktime; // all transactions get different hashes
    coins.emplace_back(COutPoint(tx.GetHash(), n_input), tx.vout.at(n_input), /*depth=*/0, n_input_bytes, /*spendable=*/true, /*solvable=*/true, /*safe=*/true, /*time=*/0, /*from_me=*/true, fee_rate);
}

// Randomly distribute coins to instances of OutputGroup
static void GroupCoins(FuzzedDataProvider& fuzzed_data_provider, const std::vector<COutput>& coins, const CoinSelectionParams& coin_params, bool positive_only, std::vector<OutputGroup>& output_groups)
{
    auto output_group = OutputGroup(coin_params);
    bool valid_outputgroup{false};
    for (auto& coin : coins) {
        if (!positive_only || (positive_only && coin.GetEffectiveValue() > 0)) {
            output_group.Insert(std::make_shared<COutput>(coin), /*ancestors=*/0, /*descendants=*/0);
        }
        // If positive_only was specified, nothing was inserted, leading to an empty output group
        // that would be invalid for the BnB algorithm
        valid_outputgroup = !positive_only || output_group.GetSelectionAmount() > 0;
        if (valid_outputgroup && fuzzed_data_provider.ConsumeBool()) {
            output_groups.push_back(output_group);
            output_group = OutputGroup(coin_params);
            valid_outputgroup = false;
        }
    }
    if (valid_outputgroup) output_groups.push_back(output_group);
}

static CAmount CreateCoins(FuzzedDataProvider& fuzzed_data_provider, std::vector<COutput>& utxo_pool, CoinSelectionParams& coin_params, int& next_locktime)
{
    CAmount total_balance{0};
    LIMITED_WHILE(fuzzed_data_provider.ConsumeBool(), 10000)
    {
        const int n_input{fuzzed_data_provider.ConsumeIntegralInRange<int>(0, 10)};
        const int n_input_bytes{fuzzed_data_provider.ConsumeIntegralInRange<int>(41, 10000)};
        const CAmount amount{fuzzed_data_provider.ConsumeIntegralInRange<CAmount>(1, MAX_MONEY)};
        if (total_balance + amount >= MAX_MONEY) {
            break;
        }
        AddCoin(amount, n_input, n_input_bytes, ++next_locktime, utxo_pool, coin_params.m_effective_feerate);
        total_balance += amount;
    }

    return total_balance;
}

static SelectionResult ManualSelection(std::vector<COutput>& utxos, const CAmount& total_amount, const bool& subtract_fee_outputs)
{
    SelectionResult result(total_amount, SelectionAlgorithm::MANUAL);
    std::set<std::shared_ptr<COutput>> utxo_pool;
    for (const auto& utxo : utxos) {
        utxo_pool.insert(std::make_shared<COutput>(utxo));
    }
    result.AddInputs(utxo_pool, subtract_fee_outputs);
    return result;
}

// Returns true if the result contains an error and the message is not empty
static bool HasErrorMsg(const util::Result<SelectionResult>& res) { return !util::ErrorString(res).empty(); }

FUZZ_TARGET(coinselection)
{
    FuzzedDataProvider fuzzed_data_provider{buffer.data(), buffer.size()};
    std::vector<COutput> utxo_pool;

    const CFeeRate long_term_fee_rate{ConsumeMoney(fuzzed_data_provider, /*max=*/COIN)};
    const CFeeRate effective_fee_rate{ConsumeMoney(fuzzed_data_provider, /*max=*/COIN)};
    // Discard feerate must be at least dust relay feerate
    const CFeeRate discard_fee_rate{fuzzed_data_provider.ConsumeIntegralInRange<CAmount>(DUST_RELAY_TX_FEE, COIN)};
    const CAmount min_viable_change{ConsumeMoney(fuzzed_data_provider, /*max=*/COIN)};
    const CAmount target{fuzzed_data_provider.ConsumeIntegralInRange<CAmount>(1, MAX_MONEY)};
    const bool subtract_fee_outputs{fuzzed_data_provider.ConsumeBool()};

    FastRandomContext fast_random_context{ConsumeUInt256(fuzzed_data_provider)};
    CoinSelectionParams coin_params{fast_random_context};
    coin_params.m_subtract_fee_outputs = subtract_fee_outputs;
    coin_params.m_long_term_feerate = long_term_fee_rate;
    coin_params.m_effective_feerate = effective_fee_rate;
    coin_params.min_viable_change = min_viable_change;
    coin_params.change_output_size = fuzzed_data_provider.ConsumeIntegralInRange<int>(10, 1000);
    coin_params.m_change_fee = effective_fee_rate.GetFee(coin_params.change_output_size);
    coin_params.m_discard_feerate = discard_fee_rate;
    coin_params.change_spend_size = fuzzed_data_provider.ConsumeIntegralInRange<int>(41, 1000);
    coin_params.m_cost_of_change = coin_params.m_change_fee + coin_params.m_discard_feerate.GetFee(coin_params.change_spend_size);

    int next_locktime{0};
    CAmount total_balance{CreateCoins(fuzzed_data_provider, utxo_pool, coin_params, next_locktime)};

    std::vector<OutputGroup> group_pos;
    GroupCoins(fuzzed_data_provider, utxo_pool, coin_params, /*positive_only=*/true, group_pos);
    std::vector<OutputGroup> group_all;
    GroupCoins(fuzzed_data_provider, utxo_pool, coin_params, /*positive_only=*/false, group_all);

    for (const OutputGroup& group : group_all) {
        const CoinEligibilityFilter filter(fuzzed_data_provider.ConsumeIntegral<int>(), fuzzed_data_provider.ConsumeIntegral<int>(), fuzzed_data_provider.ConsumeIntegral<uint64_t>());
        (void)group.EligibleForSpending(filter);
    }

    // Run coinselection algorithms
    auto result_bnb = SelectCoinsBnB(group_pos, target, coin_params.m_cost_of_change, MAX_STANDARD_TX_WEIGHT);
    if (result_bnb) {
        assert(result_bnb->GetChange(coin_params.m_cost_of_change, CAmount{0}) == 0);
        assert(result_bnb->GetSelectedValue() >= target);
        (void)result_bnb->GetShuffledInputVector();
        (void)result_bnb->GetInputSet();
    }

    auto result_srd = SelectCoinsSRD(group_pos, target, coin_params.m_change_fee, fast_random_context, MAX_STANDARD_TX_WEIGHT);
    if (result_srd) {
        assert(result_srd->GetSelectedValue() >= target);
        assert(result_srd->GetChange(CHANGE_LOWER, coin_params.m_change_fee) > 0); // Demonstrate that SRD creates change of at least CHANGE_LOWER
        result_srd->ComputeAndSetWaste(coin_params.min_viable_change, coin_params.m_cost_of_change, coin_params.m_change_fee);
        (void)result_srd->GetShuffledInputVector();
        (void)result_srd->GetInputSet();
    }

    CAmount change_target{GenerateChangeTarget(target, coin_params.m_change_fee, fast_random_context)};
    auto result_knapsack = KnapsackSolver(group_all, target, change_target, fast_random_context, MAX_STANDARD_TX_WEIGHT);
    if (result_knapsack) {
        assert(result_knapsack->GetSelectedValue() >= target);
        result_knapsack->ComputeAndSetWaste(coin_params.min_viable_change, coin_params.m_cost_of_change, coin_params.m_change_fee);
        (void)result_knapsack->GetShuffledInputVector();
        (void)result_knapsack->GetInputSet();
    }

    // If the total balance is sufficient for the target and we are not using
    // effective values, Knapsack should always find a solution (unless the selection exceeded the max tx weight).
    if (total_balance >= target && subtract_fee_outputs && !HasErrorMsg(result_knapsack)) {
        assert(result_knapsack);
    }

    std::vector<COutput> utxos;
    std::vector<util::Result<SelectionResult>> results{result_srd, result_knapsack, result_bnb};
    CAmount new_total_balance{CreateCoins(fuzzed_data_provider, utxos, coin_params, next_locktime)};
    if (new_total_balance > 0) {
        std::set<std::shared_ptr<COutput>> new_utxo_pool;
        for (const auto& utxo : utxos) {
            new_utxo_pool.insert(std::make_shared<COutput>(utxo));
        }
        for (auto& result : results) {
            if (!result) continue;
            const auto weight{result->GetWeight()};
            result->AddInputs(new_utxo_pool, subtract_fee_outputs);
            assert(result->GetWeight() > weight);
        }
    }

    std::vector<COutput> manual_inputs;
    CAmount manual_balance{CreateCoins(fuzzed_data_provider, manual_inputs, coin_params, next_locktime)};
    if (manual_balance == 0) return;
    auto manual_selection{ManualSelection(manual_inputs, manual_balance, coin_params.m_subtract_fee_outputs)};
    for (auto& result : results) {
        if (!result) continue;
        const CAmount old_target{result->GetTarget()};
        const std::set<std::shared_ptr<COutput>> input_set{result->GetInputSet()};
        const int old_weight{result->GetWeight()};
        result->Merge(manual_selection);
        assert(result->GetInputSet().size() == input_set.size() + manual_inputs.size());
        assert(result->GetTarget() == old_target + manual_selection.GetTarget());
        assert(result->GetWeight() == old_weight + manual_selection.GetWeight());
    }
}

} // namespace wallet
