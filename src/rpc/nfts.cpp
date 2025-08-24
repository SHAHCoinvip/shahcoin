// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpc/server.h>
#include <rpc/util.h>
#include <wallet/rpcwallet.h>
#include <tokens/nft.h>
#include <validation.h>
#include <chain.h>
#include <chainparams.h>
#include <util/strencodings.h>
#include <util/translation.h>

#include <univalue.h>

namespace {

RPCHelpMan mintnft()
{
    return RPCHelpMan{"mintnft",
        "\nMints a new NFT with the specified metadata.\n",
        {
            {"name", RPCArg::Type::STR, RPCArg::Optional::NO, "NFT name"},
            {"description", RPCArg::Type::STR, RPCArg::Optional::NO, "NFT description"},
            {"image_uri", RPCArg::Type::STR, RPCArg::Optional::YES, "Image URI (optional)"},
            {"attributes", RPCArg::Type::STR, RPCArg::Optional::YES, "JSON attributes (optional)"},
            {"to_address", RPCArg::Type::STR, RPCArg::Optional::NO, "Address to receive the NFT"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "nft_id", "The created NFT ID"},
                {RPCResult::Type::STR, "txid", "The transaction ID"},
            }
        },
        RPCExamples{
            HelpExampleCli("mintnft", "\"MyNFT\" \"A unique digital asset\" \"https://example.com/image.png\" \"{\\\"rarity\\\":\\\"legendary\\\"}\" \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
            + HelpExampleRpc("mintnft", "\"MyNFT\", \"A unique digital asset\", \"https://example.com/image.png\", \"{\\\"rarity\\\":\\\"legendary\\\"}\", \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string name = request.params[0].get_str();
            std::string description = request.params[1].get_str();
            std::string imageUri = request.params.size() > 2 ? request.params[2].get_str() : "";
            std::string attributes = request.params.size() > 3 ? request.params[3].get_str() : "";
            std::string to_address = request.params.size() > 4 ? request.params[4].get_str() : request.params[2].get_str();
            
            CTxDestination dest = DecodeDestination(to_address);
            if (!IsValidDestination(dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            // Create NFT transaction
            CTransactionRef tx = wallet->CreateNFTTransaction(name, description, imageUri, attributes, dest);
            if (!tx) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to create NFT transaction");
            }
            
            // Broadcast transaction
            uint256 txid = tx->GetHash();
            if (!wallet->CommitTransaction(tx, {}, {})) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to commit NFT transaction");
            }
            
            // Get the created NFT ID
            uint256 nftId = g_nftManager->GetNFTsByCreator(dest).back().nftId;
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("nft_id", nftId.ToString());
            result.pushKV("txid", txid.ToString());
            
            return result;
        },
    };
}

RPCHelpMan transfernft()
{
    return RPCHelpMan{"transfernft",
        "\nTransfers an NFT from one address to another.\n",
        {
            {"nft_id", RPCArg::Type::STR, RPCArg::Optional::NO, "NFT ID to transfer"},
            {"from_address", RPCArg::Type::STR, RPCArg::Optional::NO, "Source address"},
            {"to_address", RPCArg::Type::STR, RPCArg::Optional::NO, "Destination address"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "txid", "The transaction ID"},
            }
        },
        RPCExamples{
            HelpExampleCli("transfernft", "\"nft_id_here\" \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\" \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
            + HelpExampleRpc("transfernft", "\"nft_id_here\", \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\", \"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
            if (!wallet) return NullUniValue;
            
            std::string nftId_str = request.params[0].get_str();
            std::string from_address = request.params[1].get_str();
            std::string to_address = request.params[2].get_str();
            
            uint256 nftId = uint256S(nftId_str);
            if (nftId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid NFT ID");
            }
            
            CTxDestination from_dest = DecodeDestination(from_address);
            CTxDestination to_dest = DecodeDestination(to_address);
            if (!IsValidDestination(from_dest) || !IsValidDestination(to_dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            // Create transfer transaction
            CTransactionRef tx = wallet->CreateNFTTransferTransaction(nftId, from_dest, to_dest);
            if (!tx) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to create NFT transfer transaction");
            }
            
            // Broadcast transaction
            uint256 txid = tx->GetHash();
            if (!wallet->CommitTransaction(tx, {}, {})) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Failed to commit NFT transfer transaction");
            }
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("txid", txid.ToString());
            
            return result;
        },
    };
}

RPCHelpMan getnftinfo()
{
    return RPCHelpMan{"getnftinfo",
        "\nReturns information about a specific NFT.\n",
        {
            {"nft_id", RPCArg::Type::STR, RPCArg::Optional::NO, "NFT ID"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR, "nft_id", "The NFT ID"},
                {RPCResult::Type::STR, "name", "NFT name"},
                {RPCResult::Type::STR, "description", "NFT description"},
                {RPCResult::Type::STR, "image_uri", "Image URI"},
                {RPCResult::Type::STR, "attributes", "JSON attributes"},
                {RPCResult::Type::STR, "creator", "NFT creator address"},
                {RPCResult::Type::STR, "owner", "Current NFT owner"},
                {RPCResult::Type::NUM, "creation_time", "NFT creation timestamp"},
                {RPCResult::Type::BOOL, "transferable", "Whether the NFT is transferable"},
            }
        },
        RPCExamples{
            HelpExampleCli("getnftinfo", "\"nft_id_here\"")
            + HelpExampleRpc("getnftinfo", "\"nft_id_here\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::string nftId_str = request.params[0].get_str();
            uint256 nftId = uint256S(nftId_str);
            
            if (nftId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid NFT ID");
            }
            
            CNFTMetadata nft = g_nftManager->GetNFT(nftId);
            if (nft.nftId.IsNull()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "NFT not found");
            }
            
            CNFTOwnership ownership = g_nftManager->GetNFTOwnership(nftId);
            
            UniValue result(UniValue::VOBJ);
            result.pushKV("nft_id", nft.nftId.ToString());
            result.pushKV("name", nft.name);
            result.pushKV("description", nft.description);
            result.pushKV("image_uri", nft.imageUri);
            result.pushKV("attributes", nft.attributes);
            result.pushKV("creator", EncodeDestination(nft.creator));
            result.pushKV("owner", EncodeDestination(ownership.owner));
            result.pushKV("creation_time", nft.creationTime);
            result.pushKV("transferable", nft.isTransferable);
            
            return result;
        },
    };
}

RPCHelpMan getnftsbyowner()
{
    return RPCHelpMan{"getnftsbyowner",
        "\nReturns a list of NFTs owned by a specific address.\n",
        {
            {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "Address to check"},
        },
        RPCResult{
            RPCResult::Type::ARR, "", "",
            {
                {RPCResult::Type::OBJ, "", "",
                {
                    {RPCResult::Type::STR, "nft_id", "The NFT ID"},
                    {RPCResult::Type::STR, "name", "NFT name"},
                    {RPCResult::Type::STR, "description", "NFT description"},
                    {RPCResult::Type::STR, "image_uri", "Image URI"},
                    {RPCResult::Type::NUM, "acquisition_time", "When the NFT was acquired"},
                }}
            }
        },
        RPCExamples{
            HelpExampleCli("getnftsbyowner", "\"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
            + HelpExampleRpc("getnftsbyowner", "\"SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::string address_str = request.params[0].get_str();
            
            CTxDestination dest = DecodeDestination(address_str);
            if (!IsValidDestination(dest)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            
            std::vector<uint256> nftIds = g_nftManager->GetNFTsByOwner(dest);
            
            UniValue result(UniValue::VARR);
            for (const auto& nftId : nftIds) {
                CNFTMetadata nft = g_nftManager->GetNFT(nftId);
                CNFTOwnership ownership = g_nftManager->GetNFTOwnership(nftId);
                
                UniValue nft_obj(UniValue::VOBJ);
                nft_obj.pushKV("nft_id", nftId.ToString());
                nft_obj.pushKV("name", nft.name);
                nft_obj.pushKV("description", nft.description);
                nft_obj.pushKV("image_uri", nft.imageUri);
                nft_obj.pushKV("acquisition_time", ownership.acquisitionTime);
                result.push_back(nft_obj);
            }
            
            return result;
        },
    };
}

RPCHelpMan listnfts()
{
    return RPCHelpMan{"listnfts",
        "\nReturns a list of all NFTs in the system.\n",
        {},
        RPCResult{
            RPCResult::Type::ARR, "", "",
            {
                {RPCResult::Type::OBJ, "", "",
                {
                    {RPCResult::Type::STR, "nft_id", "The NFT ID"},
                    {RPCResult::Type::STR, "name", "NFT name"},
                    {RPCResult::Type::STR, "description", "NFT description"},
                    {RPCResult::Type::STR, "creator", "NFT creator address"},
                    {RPCResult::Type::STR, "owner", "Current NFT owner"},
                    {RPCResult::Type::NUM, "creation_time", "NFT creation timestamp"},
                }}
            }
        },
        RPCExamples{
            HelpExampleCli("listnfts", "")
            + HelpExampleRpc("listnfts", "")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::vector<CNFTMetadata> nfts = g_nftManager->GetAllNFTs();
            
            UniValue result(UniValue::VARR);
            for (const auto& nft : nfts) {
                CNFTOwnership ownership = g_nftManager->GetNFTOwnership(nft.nftId);
                
                UniValue nft_obj(UniValue::VOBJ);
                nft_obj.pushKV("nft_id", nft.nftId.ToString());
                nft_obj.pushKV("name", nft.name);
                nft_obj.pushKV("description", nft.description);
                nft_obj.pushKV("creator", EncodeDestination(nft.creator));
                nft_obj.pushKV("owner", EncodeDestination(ownership.owner));
                nft_obj.pushKV("creation_time", nft.creationTime);
                result.push_back(nft_obj);
            }
            
            return result;
        },
    };
}

} // namespace

void RegisterNFTRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        {"nfts", &mintnft},
        {"nfts", &transfernft},
        {"nfts", &getnftinfo},
        {"nfts", &getnftsbyowner},
        {"nfts", &listnfts},
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}
