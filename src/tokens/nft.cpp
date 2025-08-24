// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <tokens/nft.h>
#include <chain.h>
#include <chainparams.h>
#include <coins.h>
#include <consensus/validation.h>
#include <key.h>
#include <logging.h>
#include <script/standard.h>
#include <serialize.h>
#include <util/time.h>
#include <validation.h>

#include <crypto/sha256.h>

// Global NFT manager instance
std::unique_ptr<CNFTManager> g_nftManager = std::make_unique<CNFTManager>();

CNFTManager::CNFTManager() : nextNFTId(uint256::ZERO) {
}

bool CNFTManager::CreateNFT(const std::string& name, const std::string& description,
                           const std::string& imageUri, const std::string& attributes,
                           const CTxDestination& creator, const uint256& txHash,
                           CValidationState& state) {
    LOCK(cs_nfts);
    
    if (!ValidateNFTCreation(name, description, imageUri, attributes, state)) {
        return false;
    }
    
    // Generate unique NFT ID
    uint256 nftId = GenerateNFTId(name, creator);
    
    // Create NFT metadata
    CNFTMetadata metadata(name, description, imageUri, attributes, creator, nftId, GetTime());
    mapNFTs[nftId] = metadata;
    
    // Set initial ownership to creator
    CNFTOwnership ownership(nftId, creator, GetTime(), txHash);
    mapOwnership[nftId] = ownership;
    mapOwnerNFTs[creator].push_back(nftId);
    
    LogPrint(BCLog::NFT, "Created NFT %s (%s) by %s\n", 
             name, nftId.ToString(), EncodeDestination(creator));
    return true;
}

bool CNFTManager::DestroyNFT(const uint256& nftId, const CTxDestination& owner,
                            CValidationState& state) {
    LOCK(cs_nfts);
    
    auto nftIt = mapNFTs.find(nftId);
    if (nftIt == mapNFTs.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "nft-not-found", 
                           "NFT does not exist");
    }
    
    auto ownershipIt = mapOwnership.find(nftId);
    if (ownershipIt == mapOwnership.end() || ownershipIt->second.owner != owner) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "not-nft-owner", 
                           "Only NFT owner can destroy NFT");
    }
    
    // Remove NFT and ownership
    mapNFTs.erase(nftIt);
    mapOwnership.erase(ownershipIt);
    
    // Remove from owner's NFT list
    auto ownerIt = mapOwnerNFTs.find(owner);
    if (ownerIt != mapOwnerNFTs.end()) {
        auto& nftList = ownerIt->second;
        nftList.erase(std::remove(nftList.begin(), nftList.end(), nftId), nftList.end());
        if (nftList.empty()) {
            mapOwnerNFTs.erase(ownerIt);
        }
    }
    
    LogPrint(BCLog::NFT, "Destroyed NFT %s\n", nftId.ToString());
    return true;
}

bool CNFTManager::TransferNFT(const uint256& nftId, const CTxDestination& from,
                             const CTxDestination& to, const uint256& txHash,
                             CValidationState& state) {
    LOCK(cs_nfts);
    
    if (!IsValidNFT(nftId)) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-nft", 
                           "NFT does not exist");
    }
    
    if (!IsNFTOwner(nftId, from)) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "not-nft-owner", 
                           "Sender is not NFT owner");
    }
    
    if (!IsTransferable(nftId)) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "nft-not-transferable", 
                           "NFT is not transferable");
    }
    
    // Update ownership
    if (!UpdateOwnership(nftId, to, txHash, state)) {
        return false;
    }
    
    LogPrint(BCLog::NFT, "Transferred NFT %s from %s to %s\n", 
             nftId.ToString(), EncodeDestination(from), EncodeDestination(to));
    return true;
}

bool CNFTManager::MintNFT(const std::string& name, const std::string& description,
                         const std::string& imageUri, const std::string& attributes,
                         const CTxDestination& to, const uint256& txHash,
                         CValidationState& state) {
    LOCK(cs_nfts);
    
    if (!ValidateNFTCreation(name, description, imageUri, attributes, state)) {
        return false;
    }
    
    // Generate unique NFT ID
    uint256 nftId = GenerateNFTId(name, to);
    
    // Create NFT metadata
    CNFTMetadata metadata(name, description, imageUri, attributes, to, nftId, GetTime());
    mapNFTs[nftId] = metadata;
    
    // Set ownership to recipient
    CNFTOwnership ownership(nftId, to, GetTime(), txHash);
    mapOwnership[nftId] = ownership;
    mapOwnerNFTs[to].push_back(nftId);
    
    LogPrint(BCLog::NFT, "Minted NFT %s (%s) to %s\n", 
             name, nftId.ToString(), EncodeDestination(to));
    return true;
}

CNFTMetadata CNFTManager::GetNFT(const uint256& nftId) const {
    LOCK(cs_nfts);
    
    auto it = mapNFTs.find(nftId);
    if (it != mapNFTs.end()) {
        return it->second;
    }
    return CNFTMetadata();
}

CNFTOwnership CNFTManager::GetNFTOwnership(const uint256& nftId) const {
    LOCK(cs_nfts);
    
    auto it = mapOwnership.find(nftId);
    if (it != mapOwnership.end()) {
        return it->second;
    }
    return CNFTOwnership();
}

CTxDestination CNFTManager::GetNFTOwner(const uint256& nftId) const {
    LOCK(cs_nfts);
    
    auto it = mapOwnership.find(nftId);
    if (it != mapOwnership.end()) {
        return it->second.owner;
    }
    return CTxDestination();
}

std::vector<uint256> CNFTManager::GetNFTsByOwner(const CTxDestination& owner) const {
    LOCK(cs_nfts);
    
    auto it = mapOwnerNFTs.find(owner);
    if (it != mapOwnerNFTs.end()) {
        return it->second;
    }
    return std::vector<uint256>();
}

std::vector<CNFTMetadata> CNFTManager::GetAllNFTs() const {
    LOCK(cs_nfts);
    
    std::vector<CNFTMetadata> nfts;
    for (const auto& pair : mapNFTs) {
        nfts.push_back(pair.second);
    }
    return nfts;
}

std::vector<CNFTMetadata> CNFTManager::GetNFTsByCreator(const CTxDestination& creator) const {
    LOCK(cs_nfts);
    
    std::vector<CNFTMetadata> nfts;
    for (const auto& pair : mapNFTs) {
        if (pair.second.creator == creator) {
            nfts.push_back(pair.second);
        }
    }
    return nfts;
}

bool CNFTManager::IsValidNFT(const uint256& nftId) const {
    LOCK(cs_nfts);
    return mapNFTs.find(nftId) != mapNFTs.end();
}

bool CNFTManager::IsNFTOwner(const uint256& nftId, const CTxDestination& owner) const {
    LOCK(cs_nfts);
    
    auto it = mapOwnership.find(nftId);
    return it != mapOwnership.end() && it->second.owner == owner;
}

bool CNFTManager::IsTransferable(const uint256& nftId) const {
    LOCK(cs_nfts);
    
    auto it = mapNFTs.find(nftId);
    return it != mapNFTs.end() && it->second.isTransferable;
}

bool CNFTManager::ValidateNFTCreation(const std::string& name, const std::string& description,
                                     const std::string& imageUri, const std::string& attributes,
                                     CValidationState& state) const {
    if (!NFTValidation::ValidateNFTName(name, state) ||
        !NFTValidation::ValidateNFTDescription(description, state) ||
        !NFTValidation::ValidateNFTImageUri(imageUri, state) ||
        !NFTValidation::ValidateNFTAttributes(attributes, state)) {
        return false;
    }
    
    return true;
}

void CNFTManager::Serialize(CAutoFile& file) const {
    LOCK(cs_nfts);
    
    file << mapNFTs;
    file << mapOwnership;
    file << mapOwnerNFTs;
    file << nextNFTId;
}

void CNFTManager::Unserialize(CAutoFile& file) {
    LOCK(cs_nfts);
    
    file >> mapNFTs;
    file >> mapOwnership;
    file >> mapOwnerNFTs;
    file >> nextNFTId;
}

uint256 CNFTManager::GenerateNFTId(const std::string& name, const CTxDestination& creator) {
    CHashWriter ss(SER_GETHASH, 0);
    ss << name;
    ss << creator;
    ss << GetTime();
    ss << nextNFTId;
    
    nextNFTId = ss.GetHash();
    return nextNFTId;
}

bool CNFTManager::UpdateOwnership(const uint256& nftId, const CTxDestination& newOwner,
                                 const uint256& txHash, CValidationState& state) {
    auto ownershipIt = mapOwnership.find(nftId);
    if (ownershipIt == mapOwnership.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "nft-not-found", 
                           "NFT does not exist");
    }
    
    CTxDestination oldOwner = ownershipIt->second.owner;
    
    // Update ownership record
    ownershipIt->second.owner = newOwner;
    ownershipIt->second.acquisitionTime = GetTime();
    ownershipIt->second.acquisitionTx = txHash;
    
    // Remove from old owner's list
    auto oldOwnerIt = mapOwnerNFTs.find(oldOwner);
    if (oldOwnerIt != mapOwnerNFTs.end()) {
        auto& nftList = oldOwnerIt->second;
        nftList.erase(std::remove(nftList.begin(), nftList.end(), nftId), nftList.end());
        if (nftList.empty()) {
            mapOwnerNFTs.erase(oldOwnerIt);
        }
    }
    
    // Add to new owner's list
    mapOwnerNFTs[newOwner].push_back(nftId);
    
    return true;
}

// NFT validation functions
namespace NFTValidation {

bool ValidateNFTMintTx(const CTransaction& tx, const CCoinsViewCache& view,
                      CValidationState& state) {
    if (tx.vout.empty()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "no-outputs", 
                           "NFT mint transaction must have outputs");
    }
    
    // Check if first output contains NFT mint data
    CNFTTxData nftData;
    if (!ExtractNFTData(tx.vout[0].scriptPubKey, nftData)) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-nft-data", 
                           "Invalid NFT mint data");
    }
    
    if (nftData.type != NFT_TX_MINT) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "wrong-tx-type", 
                           "Transaction type mismatch");
    }
    
    // Validate NFT metadata from metadata field
    CNFTMetadata nftMetadata;
    if (!ParseNFTMetadata(nftData.metadata, nftMetadata, state)) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-metadata", 
                           "Invalid NFT metadata");
    }
    
    return true;
}

bool ValidateNFTTransferTx(const CTransaction& tx, const CCoinsViewCache& view,
                          CValidationState& state) {
    if (tx.vout.empty()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "no-outputs", 
                           "NFT transfer transaction must have outputs");
    }
    
    CNFTTxData nftData;
    if (!ExtractNFTData(tx.vout[0].scriptPubKey, nftData)) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-nft-data", 
                           "Invalid NFT transfer data");
    }
    
    if (nftData.type != NFT_TX_TRANSFER) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "wrong-tx-type", 
                           "Transaction type mismatch");
    }
    
    if (nftData.nftId.IsNull()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-nft-id", 
                           "Invalid NFT ID");
    }
    
    return true;
}

bool ExtractNFTData(const CScript& script, CNFTTxData& data) {
    // Parse script to extract NFT transaction data
    // This is a simplified implementation - in practice, you'd need more sophisticated parsing
    
    if (script.size() < 10) {
        return false;
    }
    
    // Look for NFT transaction marker
    std::vector<unsigned char> nftMarker = {0x4E, 0x46, 0x54}; // "NFT"
    auto pos = std::search(script.begin(), script.end(), nftMarker.begin(), nftMarker.end());
    
    if (pos == script.end()) {
        return false;
    }
    
    // Extract transaction type
    if (pos + 3 < script.end()) {
        data.type = static_cast<NFTTxType>(*(pos + 3));
    }
    
    // Extract other data (simplified)
    // In a real implementation, you'd parse the full script structure
    
    return true;
}

CScript CreateNFTScript(const CNFTTxData& data) {
    CScript script;
    
    // Add NFT transaction marker
    script << std::vector<unsigned char>{0x4E, 0x46, 0x54}; // "NFT"
    
    // Add transaction type
    script << static_cast<uint8_t>(data.type);
    
    // Add NFT ID
    script << data.nftId;
    
    // Add addresses
    script << GetScriptForDestination(data.from);
    script << GetScriptForDestination(data.to);
    
    // Add metadata
    if (!data.metadata.empty()) {
        script << data.metadata;
    }
    
    return script;
}

bool IsNFTTransaction(const CTransaction& tx) {
    if (tx.vout.empty()) {
        return false;
    }
    
    // Check if any output contains NFT data
    for (const CTxOut& txout : tx.vout) {
        CNFTTxData nftData;
        if (ExtractNFTData(txout.scriptPubKey, nftData)) {
            return true;
        }
    }
    
    return false;
}

bool ValidateNFTName(const std::string& name, CValidationState& state) {
    if (name.empty() || name.length() > MAX_NFT_NAME_LENGTH) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-name", 
                           "Invalid NFT name");
    }
    
    // Check for valid characters (alphanumeric, spaces, and common punctuation)
    for (char c : name) {
        if (!std::isalnum(c) && c != ' ' && c != '-' && c != '_' && c != '.') {
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-name-chars", 
                               "NFT name contains invalid characters");
        }
    }
    
    return true;
}

bool ValidateNFTDescription(const std::string& description, CValidationState& state) {
    if (description.length() > MAX_NFT_DESCRIPTION_LENGTH) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "description-too-long", 
                           "NFT description too long");
    }
    
    return true;
}

bool ValidateNFTImageUri(const std::string& uri, CValidationState& state) {
    if (uri.empty()) {
        return true; // Optional field
    }
    
    if (uri.length() > 512) { // Reasonable URI length limit
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "uri-too-long", 
                           "NFT image URI too long");
    }
    
    // Basic URI validation (http/https/ipfs)
    if (uri.substr(0, 7) != "http://" && 
        uri.substr(0, 8) != "https://" && 
        uri.substr(0, 5) != "ipfs:") {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-uri", 
                           "Invalid NFT image URI format");
    }
    
    return true;
}

bool ValidateNFTAttributes(const std::string& attributes, CValidationState& state) {
    if (attributes.length() > MAX_NFT_METADATA_SIZE) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "attributes-too-large", 
                           "NFT attributes too large");
    }
    
    // Basic JSON validation (simplified)
    if (!attributes.empty()) {
        // Check for basic JSON structure
        if (attributes[0] != '{' || attributes[attributes.length()-1] != '}') {
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-json", 
                               "NFT attributes must be valid JSON");
        }
    }
    
    return true;
}

bool ParseNFTMetadata(const std::string& metadata, CNFTMetadata& nftData, 
                     CValidationState& state) {
    // This is a simplified JSON parser - in practice, you'd use a proper JSON library
    
    if (metadata.empty()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "empty-metadata", 
                           "NFT metadata cannot be empty");
    }
    
    // Basic JSON parsing (simplified)
    // Extract name, description, imageUri, attributes from JSON string
    // This is a placeholder implementation
    
    // For now, we'll just validate the basic structure
    if (metadata.length() > MAX_NFT_METADATA_SIZE) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "metadata-too-large", 
                           "NFT metadata too large");
    }
    
    // In a real implementation, you'd parse the JSON and populate nftData
    // For now, we'll just return true if the metadata isn't empty
    return true;
}

} // namespace NFTValidation
