// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpc/server.h>
#include <rpc/util.h>
#include <wallet/rpcwallet.h>
#include <dex/dex.h>
#include <validation.h>
#include <chain.h>
#include <chainparams.h>
#include <util/strencodings.h>
#include <util/translation.h>

#include <univalue.h>

namespace {

RPCHelpMan createtradingpair()
{
    return RPCHelpMan{"createtradingpair",
        "\nCreates a new trading pair for token swapping.\n",
        {
            {"token_a", RPCArg::Type::STR, RPCArg::Optional::NO, "First token ID"},
            {"token_b", RPCArg::Type::STR, RPCArg::Optional::NO, "Second token ID"},
            {"symbol", RPCArg::Type::STR, RPCArg::Optional::NO, "Trading pair symbol"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "pair_id", "The created trading pair ID"},
                {RPCResult::Type::STR, "txid", "The transaction ID"},
            }
        },
        RPCExamples{
            HelpExampleCli("createtradingpair", "\"token_a_id\" \"token_b_id\" \"SHAH-MTK\"")
            + HelpExampleRpc("createtradingpair", "\"token_a_id\", \"token_b_id\", \"SHAH-MTK\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string tokenA_str = request.params[0].get_str();
            std::string tokenB_str = request.params[1].get_str();
            std::string symbol = request.params[2].get_str();
            
            uint256 tokenA = uint256S(tokenA_str);
            uint256 tokenB = uint256S(tokenB_str);
            
            if (tokenA.IsNull() || tokenB.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid token ID");
            }
            
            if (tokenA == tokenB) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot create pair with same tokens");
            }
            
            // Create trading pair transaction
            CTransactionRef tx = wallet->CreateTradingPairTransaction(tokenA, tokenB, symbol);
            if (!tx) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to create trading pair transaction");
            }
            
            // Broadcast transaction
            uint256 txid = tx->GetHash();
            if (!wallet->CommitTransaction(tx, {}, {})) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to commit trading pair transaction");
            }
            
            // Get the created pair ID
            uint256 pairId = g_dexManager->GetTradingPairBySymbol(symbol).pairId;
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("pair_id", pairId.ToString());
            result.pushKV("txid", txid.ToString());
            
            return result;
        },
    };
}

RPCHelpMan addliquidity()
{
    return RPCHelpMan{"addliquidity",
        "\nAdds liquidity to a trading pair.\n",
        {
            {"pair_id", RPCArg::Type::STR, RPCArg::Optional::NO, "Trading pair ID"},
            {"amount_a", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "Amount of token A"},
            {"amount_b", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "Amount of token B"},
            {"provider_address", RPCArg::Type::STR, RPCArg::Optional::NO, "Liquidity provider address"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "txid", "The transaction ID"},
                {RPCResult::Type::NUM, "liquidity_tokens", "Liquidity tokens received"},
            }
        },
        RPCExamples{
            HelpExampleCli("addliquidity", "\"pair_id_here\" 1000 5000 \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
            + HelpExampleRpc("addliquidity", "\"pair_id_here\", 1000, 5000, \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string pairId_str = request.params[0].get_str();
            uint64_t amountA = request.params[1].get_int64();
            uint64_t amountB = request.params[2].get_int64();
            std::string provider_address = request.params[3].get_str();
            
            uint256 pairId = uint256S(pairId_str);
            if (pairId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid pair ID");
            }
            
            CTxDestination provider_dest = DecodeDestination(provider_address);
            if (!IsValidDestination(provider_dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            if (amountA == 0 || amountB == 0) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Liquidity amounts cannot be zero");
            }
            
            // Create add liquidity transaction
            CTransactionRef tx = wallet->CreateAddLiquidityTransaction(pairId, provider_dest, amountA, amountB);
            if (!tx) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to create add liquidity transaction");
            }
            
            // Broadcast transaction
            uint256 txid = tx->GetHash();
            if (!wallet->CommitTransaction(tx, {}, {})) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to commit add liquidity transaction");
            }
            
            // Calculate liquidity tokens
            uint64_t liquidityTokens = g_dexManager->CalculateLiquidityTokens(pairId, amountA, amountB);
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("txid", txid.ToString());
            result.pushKV("liquidity_tokens", liquidityTokens);
            
            return result;
        },
    };
}

RPCHelpMan removeliquidity()
{
    return RPCHelpMan{"removeliquidity",
        "\nRemoves liquidity from a trading pair.\n",
        {
            {"pair_id", RPCArg::Type::STR, RPCArg::Optional::NO, "Trading pair ID"},
            {"liquidity_tokens", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "Amount of liquidity tokens to burn"},
            {"provider_address", RPCArg::Type::STR, RPCArg::Optional::NO, "Liquidity provider address"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "txid", "The transaction ID"},
                {RPCResult::Type::NUM, "amount_a", "Amount of token A received"},
                {RPCResult::Type::NUM, "amount_b", "Amount of token B received"},
            }
        },
        RPCExamples{
            HelpExampleCli("removeliquidity", "\"pair_id_here\" 100 \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
            + HelpExampleRpc("removeliquidity", "\"pair_id_here\", 100, \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string pairId_str = request.params[0].get_str();
            uint64_t liquidityTokens = request.params[1].get_int64();
            std::string provider_address = request.params[2].get_str();
            
            uint256 pairId = uint256S(pairId_str);
            if (pairId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid pair ID");
            }
            
            CTxDestination provider_dest = DecodeDestination(provider_address);
            if (!IsValidDestination(provider_dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            if (liquidityTokens == 0) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Liquidity tokens cannot be zero");
            }
            
            // Create remove liquidity transaction
            CTransactionRef tx = wallet->CreateRemoveLiquidityTransaction(pairId, provider_dest, liquidityTokens);
            if (!tx) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to create remove liquidity transaction");
            }
            
            // Broadcast transaction
            uint256 txid = tx->GetHash();
            if (!wallet->CommitTransaction(tx, {}, {})) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to commit remove liquidity transaction");
            }
            
            // Calculate amounts received
            auto [amountA, amountB] = g_dexManager->CalculateLiquidityRemoval(pairId, liquidityTokens);
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("txid", txid.ToString());
            result.pushKV("amount_a", amountA);
            result.pushKV("amount_b", amountB);
            
            return result;
        },
    };
}

RPCHelpMan swaptokens()
{
    return RPCHelpMan{"swaptokens",
        "\nSwaps tokens using a trading pair.\n",
        {
            {"pair_id", RPCArg::Type::STR, RPCArg::Optional::NO, "Trading pair ID"},
            {"is_buy", RPCArg::Type::BOOL, RPCArg::Optional::NO, "True to buy token B with token A, false to sell"},
            {"amount_in", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "Amount of input tokens"},
            {"amount_out_min", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "Minimum amount of output tokens"},
            {"trader_address", RPCArg::Type::STR, RPCArg::Optional::NO, "Trader address"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "txid", "The transaction ID"},
                {RPCResult::Type::NUM, "amount_out", "Amount of tokens received"},
            }
        },
        RPCExamples{
            HelpExampleCli("swaptokens", "\"pair_id_here\" true 1000 950 \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
            + HelpExampleRpc("swaptokens", "\"pair_id_here\", true, 1000, 950, \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string pairId_str = request.params[0].get_str();
            bool isBuy = request.params[1].get_bool();
            uint64_t amountIn = request.params[2].get_int64();
            uint64_t amountOutMin = request.params[3].get_int64();
            std::string trader_address = request.params[4].get_str();
            
            uint256 pairId = uint256S(pairId_str);
            if (pairId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid pair ID");
            }
            
            CTxDestination trader_dest = DecodeDestination(trader_address);
            if (!IsValidDestination(trader_dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            if (amountIn == 0) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Input amount cannot be zero");
            }
            
            // Calculate expected output
            uint64_t expectedOutput = g_dexManager->CalculateSwapOutput(pairId, isBuy, amountIn);
            if (expectedOutput < amountOutMin) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, 
                    strprintf("Expected output %d is less than minimum %d", expectedOutput, amountOutMin));
            }
            
            // Create swap transaction
            CTransactionRef tx = wallet->CreateSwapTransaction(pairId, trader_dest, isBuy, amountIn, amountOutMin);
            if (!tx) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to create swap transaction");
            }
            
            // Broadcast transaction
            uint256 txid = tx->GetHash();
            if (!wallet->CommitTransaction(tx, {}, {})) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to commit swap transaction");
            }
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("txid", txid.ToString());
            result.pushKV("amount_out", expectedOutput);
            
            return result;
        },
    };
}

RPCHelpMan gettradingpairinfo()
{
    return RPCHelpMan{"gettradingpairinfo",
        "\nReturns information about a specific trading pair.\n",
        {
            {"pair_id", RPCArg::Type::STR, RPCArg::Optional::NO, "Trading pair ID"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "pair_id", "The trading pair ID"},
                {RPCResult::Type::STR, "token_a", "First token ID"},
                {RPCResult::Type::STR, "token_b", "Second token ID"},
                {RPCResult::Type::STR, "symbol", "Trading pair symbol"},
                {RPCResult::Type::NUM, "reserve_a", "Reserve of token A"},
                {RPCResult::Type::NUM, "reserve_b", "Reserve of token B"},
                {RPCResult::Type::NUM, "total_liquidity", "Total liquidity tokens"},
                {RPCResult::Type::NUM, "creation_time", "Pair creation timestamp"},
            }
        },
        RPCExamples{
            HelpExampleCli("gettradingpairinfo", "\"pair_id_here\"")
            + HelpExampleRpc("gettradingpairinfo", "\"pair_id_here\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::string pairId_str = request.params[0].get_str();
            uint256 pairId = uint256S(pairId_str);
            
            if (pairId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid pair ID");
            }
            
            CTradingPair pair = g_dexManager->GetTradingPair(pairId);
            if (pair.pairId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Trading pair not found");
            }
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("pair_id", pair.pairId.ToString());
            result.pushKV("token_a", pair.tokenA.ToString());
            result.pushKV("token_b", pair.tokenB.ToString());
            result.pushKV("symbol", pair.symbol);
            result.pushKV("reserve_a", pair.reserveA);
            result.pushKV("reserve_b", pair.reserveB);
            result.pushKV("total_liquidity", pair.totalLiquidity);
            result.pushKV("creation_time", pair.creationTime);
            
            return result;
        },
    };
}

RPCHelpMan listtradingpairs()
{
    return RPCHelpMan{"listtradingpairs",
        "\nReturns a list of all trading pairs in the system.\n",
        {},
        RPCResult{
            RPCResult::Type::ARR, "", "",
            {
                {RPCResult::Type::OBJ, "", "",
                {
                    {RPCResult::Type::STR, "pair_id", "The trading pair ID"},
                    {RPCResult::Type::STR, "token_a", "First token ID"},
                    {RPCResult::Type::STR, "token_b", "Second token ID"},
                    {RPCResult::Type::STR, "symbol", "Trading pair symbol"},
                    {RPCResult::Type::NUM, "reserve_a", "Reserve of token A"},
                    {RPCResult::Type::NUM, "reserve_b", "Reserve of token B"},
                    {RPCResult::Type::NUM, "total_liquidity", "Total liquidity tokens"},
                }}
            }
        },
        RPCExamples{
            HelpExampleCli("listtradingpairs", "")
            + HelpExampleRpc("listtradingpairs", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::vector<CTradingPair> pairs = g_dexManager->GetAllTradingPairs();
            
            UniValue result(UniValue::VARR);
            for (const auto& pair : pairs) {
                UniValue pair_obj(UniValue::VOBJ);
                pair_obj.pushKV("pair_id", pair.pairId.ToString());
                pair_obj.pushKV("token_a", pair.tokenA.ToString());
                pair_obj.pushKV("token_b", pair.tokenB.ToString());
                pair_obj.pushKV("symbol", pair.symbol);
                pair_obj.pushKV("reserve_a", pair.reserveA);
                pair_obj.pushKV("reserve_b", pair.reserveB);
                pair_obj.pushKV("total_liquidity", pair.totalLiquidity);
                result.push_back(pair_obj);
            }
            
            return result;
        },
    };
}

} // namespace

void RegisterDEXRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        {"dex", &createtradingpair},
        {"dex", &addliquidity},
        {"dex", &removeliquidity},
        {"dex", &swaptokens},
        {"dex", &gettradingpairinfo},
        {"dex", &listtradingpairs},
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}
