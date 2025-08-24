// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpc/server.h>
#include <rpc/server_util.h>
#include <rpc/util.h>
#include <consensus/hybrid.h>
#include <pow.h>
#include <chain.h>
#include <chainparams.h>
#include <univalue.h>

static RPCHelpMan getalgoinfo()
{
    return RPCHelpMan{"getalgoinfo",
        "\nReturns information about the current mining algorithm and hybrid consensus parameters.",
        {},
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "current_algo", "Current mining algorithm"},
                {RPCResult::Type::NUM, "current_height", "Current block height"},
                {RPCResult::Type::NUM, "next_algo", "Next algorithm in rotation"},
                {RPCResult::Type::NUM, "blocks_until_rotation", "Blocks until next algorithm rotation"},
                {RPCResult::Type::ARR, "supported_algorithms", "List of supported algorithms",
                {
                    {RPCResult::Type::OBJ, "", "",
                    {
                        {RPCResult::Type::STR, "name", "Algorithm name"},
                        {RPCResult::Type::NUM, "id", "Algorithm ID"},
                        {RPCResult::Type::NUM, "weight", "Algorithm weight in difficulty adjustment"},
                    }},
                }},
            }},
        },
        RPCExamples{
            HelpExampleCli("getalgoinfo", "")
            + HelpExampleRpc("getalgoinfo", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            NodeContext& node = EnsureAnyNodeContext(request.context);
            ChainstateManager& chainman = EnsureChainman(node);
            LOCK(cs_main);
            
            const CChain& active_chain = chainman.ActiveChain();
            int current_height = active_chain.Height();
            
            UniValue obj(UniValue::VOBJ);
            
            // Current algorithm
            AlgoType current_algo = SelectNextAlgo(current_height);
            obj.pushKV("current_algo", AlgoName(current_algo));
            obj.pushKV("current_height", current_height);
            
            // Next algorithm
            AlgoType next_algo = SelectNextAlgo(current_height + 1);
            obj.pushKV("next_algo", AlgoName(next_algo));
            
            // Blocks until rotation
            int blocks_until_rotation = 3 - (current_height % 3);
            obj.pushKV("blocks_until_rotation", blocks_until_rotation);
            
            // Supported algorithms
            UniValue algos(UniValue::VARR);
            for (int i = 0; i < 3; i++) {
                AlgoType algo = static_cast<AlgoType>(i);
                UniValue algo_obj(UniValue::VOBJ);
                algo_obj.pushKV("name", AlgoName(algo));
                algo_obj.pushKV("id", i);
                algo_obj.pushKV("weight", i == 2 ? 34 : 33); // 33%, 33%, 34%
                algos.push_back(algo_obj);
            }
            obj.pushKV("supported_algorithms", algos);
            
            return obj;
        },
    };
}

static RPCHelpMan getalgodifficulty()
{
    return RPCHelpMan{"getalgodifficulty",
        "\nReturns difficulty information for all supported algorithms.",
        {},
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::OBJ, "sha256d", "SHA256d algorithm difficulty",
                {
                    {RPCResult::Type::NUM, "difficulty", "Current difficulty"},
                    {RPCResult::Type::NUM, "next_target", "Next difficulty target"},
                }},
                {RPCResult::Type::OBJ, "scrypt", "Scrypt algorithm difficulty",
                {
                    {RPCResult::Type::NUM, "difficulty", "Current difficulty"},
                    {RPCResult::Type::NUM, "next_target", "Next difficulty target"},
                }},
                {RPCResult::Type::OBJ, "groestl", "Groestl algorithm difficulty",
                {
                    {RPCResult::Type::NUM, "difficulty", "Current difficulty"},
                    {RPCResult::Type::NUM, "next_target", "Next difficulty target"},
                }},
                {RPCResult::Type::OBJ, "pos", "Proof of Stake difficulty",
                {
                    {RPCResult::Type::NUM, "difficulty", "Current difficulty"},
                    {RPCResult::Type::NUM, "next_target", "Next difficulty target"},
                }},
            }},
        },
        RPCExamples{
            HelpExampleCli("getalgodifficulty", "")
            + HelpExampleRpc("getalgodifficulty", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            NodeContext& node = EnsureAnyNodeContext(request.context);
            ChainstateManager& chainman = EnsureChainman(node);
            LOCK(cs_main);
            
            const CChain& active_chain = chainman.ActiveChain();
            int current_height = active_chain.Height();
            
            UniValue obj(UniValue::VOBJ);
            
            // SHA256d difficulty
            UniValue sha256d_obj(UniValue::VOBJ);
            unsigned int sha256_target = GetNextWorkRequiredSHA256(current_height + 1);
            sha256d_obj.pushKV("difficulty", 0x1d00ffff / (double)sha256_target);
            sha256d_obj.pushKV("next_target", sha256_target);
            obj.pushKV("sha256d", sha256d_obj);
            
            // Scrypt difficulty
            UniValue scrypt_obj(UniValue::VOBJ);
            unsigned int scrypt_target = GetNextWorkRequiredScrypt(current_height + 1);
            scrypt_obj.pushKV("difficulty", 0x1d00ffff / (double)scrypt_target);
            scrypt_obj.pushKV("next_target", scrypt_target);
            obj.pushKV("scrypt", scrypt_obj);
            
            // Groestl difficulty
            UniValue groestl_obj(UniValue::VOBJ);
            unsigned int groestl_target = GetNextWorkRequiredGroestl(current_height + 1);
            groestl_obj.pushKV("difficulty", 0x1d00ffff / (double)groestl_target);
            groestl_obj.pushKV("next_target", groestl_target);
            obj.pushKV("groestl", groestl_obj);
            
            // PoS difficulty
            UniValue pos_obj(UniValue::VOBJ);
            unsigned int pos_target = GetNextStakeTarget(current_height + 1);
            pos_obj.pushKV("difficulty", 0x1d00ffff / (double)pos_target);
            pos_obj.pushKV("next_target", pos_target);
            obj.pushKV("pos", pos_obj);
            
            return obj;
        },
    };
}

static RPCHelpMan getstakinginfo()
{
    return RPCHelpMan{"getstakinginfo",
        "\nReturns information about Proof of Stake parameters and status.",
        {},
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::BOOL, "enabled", "Whether PoS is enabled"},
                {RPCResult::Type::NUM, "min_stake_age", "Minimum stake age in seconds"},
                {RPCResult::Type::NUM, "min_stake_amount", "Minimum stake amount in SHAH"},
                {RPCResult::Type::NUM, "stake_reward", "PoS block reward in SHAH"},
                {RPCResult::Type::NUM, "pos_interval", "PoS block interval"},
                {RPCResult::Type::NUM, "current_height", "Current block height"},
                {RPCResult::Type::BOOL, "next_block_is_pos", "Whether next block should be PoS"},
            }},
        },
        RPCExamples{
            HelpExampleCli("getstakinginfo", "")
            + HelpExampleRpc("getstakinginfo", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            NodeContext& node = EnsureAnyNodeContext(request.context);
            ChainstateManager& chainman = EnsureChainman(node);
            LOCK(cs_main);
            
            const CChain& active_chain = chainman.ActiveChain();
            int current_height = active_chain.Height();
            
            UniValue obj(UniValue::VOBJ);
            
            // PoS parameters from chainparams
            const Consensus::Params& params = chainman.GetParams().GetConsensus();
            
            obj.pushKV("enabled", true); // PoS is always enabled in hybrid consensus
            obj.pushKV("min_stake_age", params.nStakeMinAge);
            obj.pushKV("min_stake_amount", params.nStakeMinAmount / COIN);
            obj.pushKV("stake_reward", params.nStakeReward / COIN);
            obj.pushKV("pos_interval", params.nPoSInterval);
            obj.pushKV("current_height", current_height);
            
            // Check if next block should be PoS
            bool next_block_is_pos = (current_height + 1) % params.nPoSInterval == 0;
            obj.pushKV("next_block_is_pos", next_block_is_pos);
            
            return obj;
        },
    };
}

void RegisterHybridRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        {"mining", &getalgoinfo},
        {"mining", &getalgodifficulty},
        {"mining", &getstakinginfo},
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}
