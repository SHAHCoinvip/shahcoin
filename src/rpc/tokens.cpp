// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpc/server.h>
#include <rpc/util.h>
#include <wallet/rpcwallet.h>
#include <tokens/token.h>
#include <validation.h>
#include <chain.h>
#include <chainparams.h>
#include <util/strencodings.h>
#include <util/translation.h>

#include <univalue.h>

namespace {

RPCHelpMan createtoken()
{
    return RPCHelpMan{"createtoken",
        "\nCreates a new token with the specified parameters.\n",
        {
            {"name", RPCArg::Type::STR, RPCArg::Optional::NO, "Token name"},
            {"symbol", RPCArg::Type::STR, RPCArg::Optional::NO, "Token symbol"},
            {"decimals", RPCArg::Type::NUM, RPCArg::Optional::NO, "Number of decimal places"},
            {"total_supply", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "Total token supply"},
            {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "Address to receive the tokens"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "token_id", "The created token ID"},
                {RPCResult::Type::STR, "txid", "The transaction ID"},
            }
        },
        RPCExamples{
            HelpExampleCli("createtoken", "\"MyToken\" \"MTK\" 8 1000000 \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
            + HelpExampleRpc("createtoken", "\"MyToken\", \"MTK\", 8, 1000000, \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string name = request.params[0].get_str();
            std::string symbol = request.params[1].get_str();
            uint8_t decimals = request.params[2].get_int();
            uint64_t totalSupply = request.params[3].get_int64();
            std::string address_str = request.params[4].get_str();
            
            CTxDestination dest = DecodeDestination(address_str);
            if (!IsValidDestination(dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            // Validate parameters
            if (decimals > MAX_TOKEN_DECIMALS) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, 
                    strprintf("Decimals cannot exceed %d", MAX_TOKEN_DECIMALS));
            }
            
            if (totalSupply == 0 || totalSupply > MAX_TOKEN_SUPPLY) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, 
                    strprintf("Total supply must be between 1 and %d", MAX_TOKEN_SUPPLY));
            }
            
            // Create token transaction
            CTransactionRef tx = wallet->CreateTokenTransaction(name, symbol, decimals, totalSupply, dest);
            if (!tx) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to create token transaction");
            }
            
            // Broadcast transaction
            uint256 txid = tx->GetHash();
            if (!wallet->CommitTransaction(tx, {}, {})) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to commit token transaction");
            }
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("token_id", g_tokenManager->GetTokenBySymbol(symbol).tokenId.ToString());
            result.pushKV("txid", txid.ToString());
            
            return result;
        },
    };
}

RPCHelpMan transfertoken()
{
    return RPCHelpMan{"transfertoken",
        "\nTransfers tokens from one address to another.\n",
        {
            {"token_id", RPCArg::Type::STR, RPCArg::Optional::NO, "Token ID to transfer"},
            {"from_address", RPCArg::Type::STR, RPCArg::Optional::NO, "Source address"},
            {"to_address", RPCArg::Type::STR, RPCArg::Optional::NO, "Destination address"},
            {"amount", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "Amount to transfer"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "txid", "The transaction ID"},
            }
        },
        RPCExamples{
            HelpExampleCli("transfertoken", "\"token_id_here\" \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\" \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\" 100")
            + HelpExampleRpc("transfertoken", "\"token_id_here\", \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\", \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\", 100")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string tokenId_str = request.params[0].get_str();
            std::string from_address = request.params[1].get_str();
            std::string to_address = request.params[2].get_str();
            uint64_t amount = request.params[3].get_int64();
            
            uint256 tokenId = uint256S(tokenId_str);
            if (tokenId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid token ID");
            }
            
            CTxDestination from_dest = DecodeDestination(from_address);
            CTxDestination to_dest = DecodeDestination(to_address);
            if (!IsValidDestination(from_dest) || !IsValidDestination(to_dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            if (amount == 0) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Transfer amount cannot be zero");
            }
            
            // Create transfer transaction
            CTransactionRef tx = wallet->CreateTokenTransferTransaction(tokenId, from_dest, to_dest, amount);
            if (!tx) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to create token transfer transaction");
            }
            
            // Broadcast transaction
            uint256 txid = tx->GetHash();
            if (!wallet->CommitTransaction(tx, {}, {})) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to commit token transfer transaction");
            }
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("txid", txid.ToString());
            
            return result;
        },
    };
}

RPCHelpMan minttoken()
{
    return RPCHelpMan{"minttoken",
        "\nMints new tokens to the specified address.\n",
        {
            {"token_id", RPCArg::Type::STR, RPCArg::Optional::NO, "Token ID to mint"},
            {"to_address", RPCArg::Type::STR, RPCArg::Optional::NO, "Address to receive the tokens"},
            {"amount", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "Amount to mint"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "txid", "The transaction ID"},
            }
        },
        RPCExamples{
            HelpExampleCli("minttoken", "\"token_id_here\" \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\" 1000")
            + HelpExampleRpc("minttoken", "\"token_id_here\", \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\", 1000")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string tokenId_str = request.params[0].get_str();
            std::string to_address = request.params[1].get_str();
            uint64_t amount = request.params[2].get_int64();
            
            uint256 tokenId = uint256S(tokenId_str);
            if (tokenId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid token ID");
            }
            
            CTxDestination to_dest = DecodeDestination(to_address);
            if (!IsValidDestination(to_dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            if (amount == 0) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Mint amount cannot be zero");
            }
            
            // Create mint transaction
            CTransactionRef tx = wallet->CreateTokenMintTransaction(tokenId, to_dest, amount);
            if (!tx) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to create token mint transaction");
            }
            
            // Broadcast transaction
            uint256 txid = tx->GetHash();
            if (!wallet->CommitTransaction(tx, {}, {})) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to commit token mint transaction");
            }
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("txid", txid.ToString());
            
            return result;
        },
    };
}

RPCHelpMan burntoken()
{
    return RPCHelpMan{"burntoken",
        "\nBurns tokens from the specified address.\n",
        {
            {"token_id", RPCArg::Type::STR, RPCArg::Optional::NO, "Token ID to burn"},
            {"from_address", RPCArg::Type::STR, RPCArg::Optional::NO, "Address to burn tokens from"},
            {"amount", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "Amount to burn"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "txid", "The transaction ID"},
            }
        },
        RPCExamples{
            HelpExampleCli("burntoken", "\"token_id_here\" \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\" 100")
            + HelpExampleRpc("burntoken", "\"token_id_here\", \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\", 100")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string tokenId_str = request.params[0].get_str();
            std::string from_address = request.params[1].get_str();
            uint64_t amount = request.params[2].get_int64();
            
            uint256 tokenId = uint256S(tokenId_str);
            if (tokenId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid token ID");
            }
            
            CTxDestination from_dest = DecodeDestination(from_address);
            if (!IsValidDestination(from_dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            if (amount == 0) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Burn amount cannot be zero");
            }
            
            // Create burn transaction
            CTransactionRef tx = wallet->CreateTokenBurnTransaction(tokenId, from_dest, amount);
            if (!tx) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to create token burn transaction");
            }
            
            // Broadcast transaction
            uint256 txid = tx->GetHash();
            if (!wallet->CommitTransaction(tx, {}, {})) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to commit token burn transaction");
            }
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("txid", txid.ToString());
            
            return result;
        },
    };
}

RPCHelpMan gettokeninfo()
{
    return RPCHelpMan{"gettokeninfo",
        "\nReturns information about a specific token.\n",
        {
            {"token_id", RPCArg::Type::STR, RPCArg::Optional::NO, "Token ID"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "token_id", "The token ID"},
                {RPCResult::Type::STR, "name", "Token name"},
                {RPCResult::Type::STR, "symbol", "Token symbol"},
                {RPCResult::Type::NUM, "decimals", "Number of decimal places"},
                {RPCResult::Type::STR_AMOUNT, "total_supply", "Total token supply"},
                {RPCResult::Type::STR, "creator", "Token creator address"},
                {RPCResult::Type::NUM, "creation_time", "Token creation timestamp"},
            }
        },
        RPCExamples{
            HelpExampleCli("gettokeninfo", "\"token_id_here\"")
            + HelpExampleRpc("gettokeninfo", "\"token_id_here\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::string tokenId_str = request.params[0].get_str();
            uint256 tokenId = uint256S(tokenId_str);
            
            if (tokenId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid token ID");
            }
            
            CTokenMetadata token = g_tokenManager->GetToken(tokenId);
            if (token.tokenId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Token not found");
            }
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("token_id", token.tokenId.ToString());
            result.pushKV("name", token.name);
            result.pushKV("symbol", token.symbol);
            result.pushKV("decimals", token.decimals);
            result.pushKV("total_supply", token.totalSupply);
            result.pushKV("creator", EncodeDestination(token.creator));
            result.pushKV("creation_time", token.creationTime);
            
            return result;
        },
    };
}

RPCHelpMan gettokenbalance()
{
    return RPCHelpMan{"gettokenbalance",
        "\nReturns the token balance for a specific address.\n",
        {
            {"token_id", RPCArg::Type::STR, RPCArg::Optional::NO, "Token ID"},
            {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "Address to check balance for"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "token_id", "The token ID"},
                {RPCResult::Type::STR, "address", "The address"},
                {RPCResult::Type::STR_AMOUNT, "balance", "Token balance"},
            }
        },
        RPCExamples{
            HelpExampleCli("gettokenbalance", "\"token_id_here\" \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
            + HelpExampleRpc("gettokenbalance", "\"token_id_here\", \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::string tokenId_str = request.params[0].get_str();
            std::string address_str = request.params[1].get_str();
            
            uint256 tokenId = uint256S(tokenId_str);
            if (tokenId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid token ID");
            }
            
            CTxDestination dest = DecodeDestination(address_str);
            if (!IsValidDestination(dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            uint64_t balance = g_tokenManager->GetTokenBalance(tokenId, dest);
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("token_id", tokenId_str);
            result.pushKV("address", address_str);
            result.pushKV("balance", balance);
            
            return result;
        },
    };
}

RPCHelpMan listtokens()
{
    return RPCHelpMan{"listtokens",
        "\nReturns a list of all tokens in the system.\n",
        {},
        RPCResult{
            RPCResult::Type::ARR, "", "",
            {
                {RPCResult::Type::OBJ, "", "",
                {
                    {RPCResult::Type::STR, "token_id", "The token ID"},
                    {RPCResult::Type::STR, "name", "Token name"},
                    {RPCResult::Type::STR, "symbol", "Token symbol"},
                    {RPCResult::Type::NUM, "decimals", "Number of decimal places"},
                    {RPCResult::Type::STR_AMOUNT, "total_supply", "Total token supply"},
                    {RPCResult::Type::STR, "creator", "Token creator address"},
                }}
            }
        },
        RPCExamples{
            HelpExampleCli("listtokens", "")
            + HelpExampleRpc("listtokens", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::vector<CTokenMetadata> tokens = g_tokenManager->GetAllTokens();
            
            UniValue result(UniValue::VARR);
            for (const auto& token : tokens) {
                UniValue token_obj(UniValue::VOBJ);
                token_obj.pushKV("token_id", token.tokenId.ToString());
                token_obj.pushKV("name", token.name);
                token_obj.pushKV("symbol", token.symbol);
                token_obj.pushKV("decimals", token.decimals);
                token_obj.pushKV("total_supply", token.totalSupply);
                token_obj.pushKV("creator", EncodeDestination(token.creator));
                result.push_back(token_obj);
            }
            
            return result;
        },
    };
}

} // namespace

void RegisterTokenRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        {"tokens", &createtoken},
        {"tokens", &transfertoken},
        {"tokens", &minttoken},
        {"tokens", &burntoken},
        {"tokens", &gettokeninfo},
        {"tokens", &gettokenbalance},
        {"tokens", &listtokens},
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}
