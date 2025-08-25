// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpc/server.h>
#include <rpc/util.h>
#include <wallet/rpcwallet.h>
#include <wallet/staking.h>
#include <stake/stake.h>
#include <validation.h>
#include <chain.h>
#include <chainparams.h>
#include <util/strencodings.h>
#include <util/translation.h>

#include <univalue.h>

namespace {

RPCHelpMan getstakinginfo()
{
    return RPCHelpMan{"getstakinginfo",
        "\nReturns an object containing staking-related information.\n",
        {},
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::BOOL, "enabled", "Whether staking is enabled"},
                {RPCResult::Type::NUM, "staking", "Whether the wallet is currently staking"},
                {RPCResult::Type::STR_AMOUNT, "total_stake", "Total amount staked"},
                {RPCResult::Type::STR_AMOUNT, "available_stake", "Available amount for staking"},
                {RPCResult::Type::NUM, "validator_count", "Number of active validators"},
                {RPCResult::Type::NUM, "stake_difficulty", "Current stake difficulty"},
                {RPCResult::Type::NUM, "expected_time", "Expected time to find a stake block (in seconds)"},
                {RPCResult::Type::STR_AMOUNT, "stake_reward", "Reward for staking a block"},
                {RPCResult::Type::NUM, "min_stake_amount", "Minimum stake amount required"},
                {RPCResult::Type::NUM, "min_stake_age", "Minimum stake age required (in seconds)"},
            }
        },
        RPCExamples{
            HelpExampleCli("getstakinginfo", "")
            + HelpExampleRpc("getstakinginfo", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            CWalletStakingManager stakingManager(wallet.get());
            CStakingInfo info = stakingManager.GetStakingInfo();
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("enabled", stakingManager.IsStakingEnabled());
            result.pushKV("staking", info.isStaking);
            result.pushKV("total_stake", ValueFromAmount(info.totalStake));
            result.pushKV("available_stake", ValueFromAmount(info.availableStake));
            result.pushKV("validator_count", info.validatorCount);
            result.pushKV("stake_difficulty", g_stakeManager->GetStakeDifficulty(nullptr));
            result.pushKV("expected_time", info.expectedTime);
            result.pushKV("stake_reward", ValueFromAmount(POS_BLOCK_REWARD));
            result.pushKV("min_stake_amount", ValueFromAmount(POS_MIN_STAKE_AMOUNT));
            result.pushKV("min_stake_age", POS_MIN_STAKE_AGE);
            
            return result;
        },
    };
}

RPCHelpMan enablestaking()
{
    return RPCHelpMan{"enablestaking",
        "\nEnables staking for the wallet.\n",
        {},
        RPCResult{
            RPCResult::Type::BOOL, "", "true if staking was successfully enabled"
        },
        RPCExamples{
            HelpExampleCli("enablestaking", "")
            + HelpExampleRpc("enablestaking", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            CWalletStakingManager stakingManager(wallet.get());
            bool success = stakingManager.EnableStaking();
            
            if (success) {
                return UniValue(true);
            } else {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to enable staking");
            }
        },
    };
}

RPCHelpMan disablestaking()
{
    return RPCHelpMan{"disablestaking",
        "\nDisables staking for the wallet.\n",
        {},
        RPCResult{
            RPCResult::Type::BOOL, "", "true if staking was successfully disabled"
        },
        RPCExamples{
            HelpExampleCli("disablestaking", "")
            + HelpExampleRpc("disablestaking", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            CWalletStakingManager stakingManager(wallet.get());
            bool success = stakingManager.DisableStaking();
            
            if (success) {
                return UniValue(true);
            } else {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to disable staking");
            }
        },
    };
}

RPCHelpMan addstake()
{
    return RPCHelpMan{"addstake",
        "\nAdds a stake to the wallet for staking.\n",
        {
            {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "The address to stake from"},
            {"amount", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "The amount to stake"},
        },
        RPCResult{
            RPCResult::Type::BOOL, "", "true if stake was successfully added"
        },
        RPCExamples{
            HelpExampleCli("addstake", "\"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\" 1000")
            + HelpExampleRpc("addstake", "\"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\", 1000")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string address_str = request.params[0].get_str();
            CAmount amount = AmountFromValue(request.params[1]);
            
            CTxDestination dest = DecodeDestination(address_str);
            if (!IsValidDestination(dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            if (amount < POS_MIN_STAKE_AMOUNT) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, 
                    strprintf("Stake amount must be at least %s SHAH", 
                             FormatMoney(POS_MIN_STAKE_AMOUNT)));
            }
            
            CWalletStakingManager stakingManager(wallet.get());
            bool success = stakingManager.AddStake(dest, amount);
            
            if (success) {
                return UniValue(true);
            } else {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to add stake");
            }
        },
    };
}

RPCHelpMan removestake()
{
    return RPCHelpMan{"removestake",
        "\nRemoves a stake from the wallet.\n",
        {
            {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "The address to remove stake from"},
        },
        RPCResult{
            RPCResult::Type::BOOL, "", "true if stake was successfully removed"
        },
        RPCExamples{
            HelpExampleCli("removestake", "\"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
            + HelpExampleRpc("removestake", "\"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string address_str = request.params[0].get_str();
            
            CTxDestination dest = DecodeDestination(address_str);
            if (!IsValidDestination(dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            CWalletStakingManager stakingManager(wallet.get());
            bool success = stakingManager.RemoveStake(dest);
            
            if (success) {
                return UniValue(true);
            } else {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to remove stake");
            }
        },
    };
}

RPCHelpMan getstakes()
{
    return RPCHelpMan{"getstakes",
        "\nReturns a list of all stakes in the wallet.\n",
        {},
        RPCResult{
            RPCResult::Type::ARR, "", "",
            {
                {RPCResult::Type::OBJ, "", "",
                {
                    {RPCResult::Type::STR, "address", "The staking address"},
                    {RPCResult::Type::STR_AMOUNT, "amount", "The staked amount"},
                    {RPCResult::Type::NUM, "age", "Stake age in seconds"},
                    {RPCResult::Type::BOOL, "eligible", "Whether the stake is eligible for staking"},
                }}
            }
        },
        RPCExamples{
            HelpExampleCli("getstakes", "")
            + HelpExampleRpc("getstakes", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            CWalletStakingManager stakingManager(wallet.get());
            std::vector<CStakeValidator> validators = stakingManager.GetValidStakes();
            
            UniValue result(UniValue::VARR);
            for (const auto& validator : validators) {
                UniValue stake(UniValue::VOBJ);
                stake.pushKV("address", EncodeDestination(validator.address));
                stake.pushKV("amount", ValueFromAmount(validator.amount));
                stake.pushKV("age", GetTime() - validator.stakeTime);
                stake.pushKV("eligible", validator.amount >= POS_MIN_STAKE_AMOUNT && 
                             (GetTime() - validator.stakeTime) >= POS_MIN_STAKE_AGE);
                result.push_back(stake);
            }
            
            return result;
        },
    };
}

RPCHelpMan getstakevalidators()
{
    return RPCHelpMan{"getstakevalidators",
        "\nReturns a list of all active validators in the network.\n",
        {},
        RPCResult{
            RPCResult::Type::ARR, "", "",
            {
                {RPCResult::Type::OBJ, "", "",
                {
                    {RPCResult::Type::STR, "address", "The validator address"},
                    {RPCResult::Type::STR_AMOUNT, "amount", "The staked amount"},
                    {RPCResult::Type::NUM, "age", "Stake age in seconds"},
                }}
            }
        },
        RPCExamples{
            HelpExampleCli("getstakevalidators", "")
            + HelpExampleRpc("getstakevalidators", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::vector<CStakeValidator> validators = g_stakeManager->GetActiveValidators();
            
            UniValue result(UniValue::VARR);
            for (const auto& validator : validators) {
                UniValue validator_obj(UniValue::VOBJ);
                validator_obj.pushKV("address", EncodeDestination(validator.address));
                validator_obj.pushKV("amount", ValueFromAmount(validator.amount));
                validator_obj.pushKV("age", GetTime() - validator.stakeTime);
                result.push_back(validator_obj);
            }
            
            return result;
        },
    };
}

} // namespace

void RegisterStakingRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        {"staking", &getstakinginfo},
        {"staking", &enablestaking},
        {"staking", &disablestaking},
        {"staking", &addstake},
        {"staking", &removestake},
        {"staking", &getstakes},
        {"staking", &getstakevalidators},
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}
