// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpc/server.h>
#include <rpc/util.h>
#include <policy/honeypot_filter.h>
#include <consensus/finality.h>
#include <stake/cold_staking.h>
#include <validation.h>
#include <chain.h>
#include <logging.h>

#include <univalue.h>

static RPCHelpMan gethoneypotstats()
{
    return RPCHelpMan{"gethoneypotstats",
        "\nReturns statistics about honeypot transaction filtering.\n",
        {},
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::NUM, "total_transactions", "Total transactions processed"},
                {RPCResult::Type::NUM, "suspicious_transactions", "Number of suspicious transactions detected"},
                {RPCResult::Type::NUM, "blocked_transactions", "Number of transactions blocked"},
                {RPCResult::Type::OBJ, "filter_type_counts", "Counts by filter type",
                    {
                        {RPCResult::Type::NUM, "excessive_op_return", "Excessive OP_RETURN outputs"},
                        {RPCResult::Type::NUM, "invalid_script", "Invalid script patterns"},
                        {RPCResult::Type::NUM, "suspicious_dust", "Suspicious dust amounts"},
                        {RPCResult::Type::NUM, "malformed_input", "Malformed input structures"},
                        {RPCResult::Type::NUM, "spam_pattern", "Known spam patterns"},
                        {RPCResult::Type::NUM, "exploit_attempt", "Exploit attempts"},
                        {RPCResult::Type::NUM, "excessive_size", "Excessive transaction size"},
                        {RPCResult::Type::NUM, "rate_limit_violation", "Rate limit violations"},
                    }
                },
                {RPCResult::Type::NUM, "last_reset_time", "Timestamp of last statistics reset"},
            }
        },
        RPCExamples{
            HelpExampleCli("gethoneypotstats", "")
            + HelpExampleRpc("gethoneypotstats", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            if (!HoneypotUtils::IsHoneypotFilteringEnabled()) {
                throw JSONRPCError(RPC_INVALID_REQUEST, "Honeypot filtering is not enabled");
            }

            auto stats = g_honeypotFilter->GetStats();
            UniValue result(UniValue::VOBJ);
            result.pushKV("total_transactions", stats.totalTransactions);
            result.pushKV("suspicious_transactions", stats.suspiciousTransactions);
            result.pushKV("blocked_transactions", stats.blockedTransactions);
            
            UniValue filterCounts(UniValue::VOBJ);
            for (const auto& pair : stats.filterTypeCounts) {
                filterCounts.pushKV(HoneypotUtils::GetFilterTypeName(pair.first), pair.second);
            }
            result.pushKV("filter_type_counts", filterCounts);
            result.pushKV("last_reset_time", stats.lastResetTime);
            
            return result;
        }
    };
}

static RPCHelpMan resethoneypotstats()
{
    return RPCHelpMan{"resethoneypotstats",
        "\nReset honeypot filtering statistics.\n",
        {},
        RPCResult{
            RPCResult::Type::STR, "", "Success message"
        },
        RPCExamples{
            HelpExampleCli("resethoneypotstats", "")
            + HelpExampleRpc("resethoneypotstats", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            if (!HoneypotUtils::IsHoneypotFilteringEnabled()) {
                throw JSONRPCError(RPC_INVALID_REQUEST, "Honeypot filtering is not enabled");
            }

            g_honeypotFilter->ResetStats();
            return "Honeypot statistics reset successfully";
        }
    };
}

static RPCHelpMan getfinalitystatus()
{
    return RPCHelpMan{"getfinalitystatus",
        "\nReturns finality status for a block.\n",
        {
            {"blockhash", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "The block hash to check (default: current tip)"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "blockhash", "The block hash"},
                {RPCResult::Type::NUM, "height", "The block height"},
                {RPCResult::Type::STR, "finality_status", "The finality status (PENDING, SOFT_FINAL, HARD_FINAL, IRREVERSIBLE)"},
                {RPCResult::Type::NUM, "confirmations", "Number of confirmations"},
                {RPCResult::Type::BOOL, "is_final", "Whether the block is considered final"},
                {RPCResult::Type::BOOL, "is_irreversible", "Whether the block is irreversible"},
            }
        },
        RPCExamples{
            HelpExampleCli("getfinalitystatus", "")
            + HelpExampleCli("getfinalitystatus", "\"0000000000000000000000000000000000000000000000000000000000000000\"")
            + HelpExampleRpc("getfinalitystatus", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            if (!FinalityUtils::IsFinalityEnabled()) {
                throw JSONRPCError(RPC_INVALID_REQUEST, "Finality rules are not enabled");
            }

            const CBlockIndex* pindex = nullptr;
            if (request.params[0].isStr()) {
                uint256 blockhash = ParseHashV(request.params[0], "blockhash");
                pindex = LookupBlockIndex(blockhash);
                if (!pindex) {
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");
                }
            } else {
                pindex = ::ChainActive().Tip();
                if (!pindex) {
                    throw JSONRPCError(RPC_INVALID_REQUEST, "No active chain tip");
                }
            }

            FinalityStatus status = g_finalityManager->GetBlockFinalityStatus(pindex);
            int confirmations = FinalityUtils::GetFinalityDepth(pindex);

            UniValue result(UniValue::VOBJ);
            result.pushKV("blockhash", pindex->GetBlockHash().GetHex());
            result.pushKV("height", pindex->nHeight);
            result.pushKV("finality_status", FinalityUtils::GetFinalityStatusName(status));
            result.pushKV("confirmations", confirmations);
            result.pushKV("is_final", FinalityUtils::IsBlockFinal(pindex));
            result.pushKV("is_irreversible", g_finalityManager->IsBlockIrreversible(pindex));
            
            return result;
        }
    };
}

static RPCHelpMan getfinalitystats()
{
    return RPCHelpMan{"getfinalitystats",
        "\nReturns statistics about block finality.\n",
        {},
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::NUM, "total_blocks", "Total blocks processed"},
                {RPCResult::Type::NUM, "soft_final_blocks", "Number of soft final blocks"},
                {RPCResult::Type::NUM, "hard_final_blocks", "Number of hard final blocks"},
                {RPCResult::Type::NUM, "irreversible_blocks", "Number of irreversible blocks"},
                {RPCResult::Type::NUM, "blocked_reorganizations", "Number of blocked reorganizations"},
                {RPCResult::Type::NUM, "last_check_time", "Timestamp of last check"},
            }
        },
        RPCExamples{
            HelpExampleCli("getfinalitystats", "")
            + HelpExampleRpc("getfinalitystats", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            if (!FinalityUtils::IsFinalityEnabled()) {
                throw JSONRPCError(RPC_INVALID_REQUEST, "Finality rules are not enabled");
            }

            auto stats = g_finalityManager->GetStats();
            UniValue result(UniValue::VOBJ);
            result.pushKV("total_blocks", stats.totalBlocks);
            result.pushKV("soft_final_blocks", stats.softFinalBlocks);
            result.pushKV("hard_final_blocks", stats.hardFinalBlocks);
            result.pushKV("irreversible_blocks", stats.irreversibleBlocks);
            result.pushKV("blocked_reorganizations", stats.blockedReorganizations);
            result.pushKV("last_check_time", stats.lastCheckTime);
            
            return result;
        }
    };
}

static RPCHelpMan getcoldstakingdelegations()
{
    return RPCHelpMan{"getcoldstakingdelegations",
        "\nReturns cold staking delegations for a wallet address.\n",
        {
            {"address", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "The wallet address to check (default: all delegations)"},
            {"type", RPCArg::Type::STR, RPCArg::Default{"all"}, "Type of delegations to return: 'cold', 'hot', or 'all'"},
        },
        RPCResult{
            RPCResult::Type::ARR, "", "",
            {
                {RPCResult::Type::OBJ, "", "",
                    {
                        {RPCResult::Type::STR, "cold_wallet", "Cold wallet address"},
                        {RPCResult::Type::STR, "hot_wallet", "Hot wallet address"},
                        {RPCResult::Type::STR, "delegated_amount", "Amount delegated (in SHAH)"},
                        {RPCResult::Type::STR, "delegation_tx_hash", "Transaction hash that created the delegation"},
                        {RPCResult::Type::NUM, "delegation_time", "Timestamp when delegation was created"},
                        {RPCResult::Type::NUM, "expiry_time", "Timestamp when delegation expires (0 = no expiry)"},
                        {RPCResult::Type::STR, "status", "Delegation status (ACTIVE, REVOKED, EXPIRED, PENDING)"},
                    }
                }
            }
        },
        RPCExamples{
            HelpExampleCli("getcoldstakingdelegations", "")
            + HelpExampleCli("getcoldstakingdelegations", "\"S1a2h3c4o5i6n7A8d9d0r1e2s3s4\"")
            + HelpExampleCli("getcoldstakingdelegations", "\"S1a2h3c4o5i6n7A8d9d0r1e2s3s4\" \"cold\"")
            + HelpExampleRpc("getcoldstakingdelegations", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            if (!ColdStakingUtils::IsColdStakingEnabled()) {
                throw JSONRPCError(RPC_INVALID_REQUEST, "Cold staking is not enabled");
            }

            std::string type = "all";
            if (request.params[1].isStr()) {
                type = request.params[1].get_str();
                if (type != "cold" && type != "hot" && type != "all") {
                    throw JSONRPCError(RPC_INVALID_PARAMETER, "Type must be 'cold', 'hot', or 'all'");
                }
            }

            UniValue result(UniValue::VARR);
            std::vector<CColdStakingDelegation> delegations;

            if (request.params[0].isStr()) {
                CTxDestination address = DecodeDestination(request.params[0].get_str());
                if (!IsValidDestination(address)) {
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
                }

                if (type == "cold" || type == "all") {
                    auto coldDelegations = g_coldStakingManager->GetDelegationsForColdWallet(address);
                    delegations.insert(delegations.end(), coldDelegations.begin(), coldDelegations.end());
                }
                if (type == "hot" || type == "all") {
                    auto hotDelegations = g_coldStakingManager->GetDelegationsForHotWallet(address);
                    delegations.insert(delegations.end(), hotDelegations.begin(), hotDelegations.end());
                }
            } else {
                delegations = g_coldStakingManager->GetActiveDelegations();
            }

            for (const auto& delegation : delegations) {
                UniValue obj(UniValue::VOBJ);
                obj.pushKV("cold_wallet", EncodeDestination(delegation.coldWalletAddress));
                obj.pushKV("hot_wallet", EncodeDestination(delegation.hotWalletAddress));
                obj.pushKV("delegated_amount", ValueFromAmount(delegation.delegatedAmount));
                obj.pushKV("delegation_tx_hash", delegation.delegationTxHash.GetHex());
                obj.pushKV("delegation_time", delegation.delegationTime);
                obj.pushKV("expiry_time", delegation.expiryTime);
                obj.pushKV("status", ColdStakingUtils::GetColdStakingStatusName(delegation.status));
                result.push_back(obj);
            }
            
            return result;
        }
    };
}

static RPCHelpMan getcoldstakingstats()
{
    return RPCHelpMan{"getcoldstakingstats",
        "\nReturns statistics about cold staking.\n",
        {},
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::NUM, "total_delegations", "Total number of delegations"},
                {RPCResult::Type::NUM, "active_delegations", "Number of active delegations"},
                {RPCResult::Type::NUM, "revoked_delegations", "Number of revoked delegations"},
                {RPCResult::Type::NUM, "expired_delegations", "Number of expired delegations"},
                {RPCResult::Type::STR, "total_delegated_amount", "Total amount delegated (in SHAH)"},
                {RPCResult::Type::STR, "active_delegated_amount", "Active amount delegated (in SHAH)"},
                {RPCResult::Type::NUM, "last_update_time", "Timestamp of last update"},
            }
        },
        RPCExamples{
            HelpExampleCli("getcoldstakingstats", "")
            + HelpExampleRpc("getcoldstakingstats", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            if (!ColdStakingUtils::IsColdStakingEnabled()) {
                throw JSONRPCError(RPC_INVALID_REQUEST, "Cold staking is not enabled");
            }

            auto stats = g_coldStakingManager->GetStats();
            UniValue result(UniValue::VOBJ);
            result.pushKV("total_delegations", stats.totalDelegations);
            result.pushKV("active_delegations", stats.activeDelegations);
            result.pushKV("revoked_delegations", stats.revokedDelegations);
            result.pushKV("expired_delegations", stats.expiredDelegations);
            result.pushKV("total_delegated_amount", ValueFromAmount(stats.totalDelegatedAmount));
            result.pushKV("active_delegated_amount", ValueFromAmount(stats.activeDelegatedAmount));
            result.pushKV("last_update_time", stats.lastUpdateTime);
            
            return result;
        }
    };
}

void RegisterSecurityRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        {"security", &gethoneypotstats},
        {"security", &resethoneypotstats},
        {"security", &getfinalitystatus},
        {"security", &getfinalitystats},
        {"staking", &getcoldstakingdelegations},
        {"staking", &getcoldstakingstats},
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}
