// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_TOKENS_NFT_H
#define SHAHCOIN_TOKENS_NFT_H

#include <primitives/transaction.h>
#include <script/standard.h>
#include <key.h>
#include <logging.h>
#include <util/time.h>
#include <consensus/amount.h>

#include <vector>
#include <map>
#include <set>
#include <memory>
#include <string>

class CWallet;
class CBlockIndex;

/**
 * NFT Minting Fee Configuration
 * Dynamic fee system for NFT creation
 */
struct NFTFeeConfig {
    CAmount baseMintingFee;          // Base fee in SHAH (e.g., 100 SHAH)
    double usdTargetFee;             // Target fee in USD (e.g., $20)
    bool dynamicPricing;             // Enable dynamic pricing
    std::string priceOracleUrl;      // Price oracle endpoint
    int64_t priceUpdateInterval;     // Price update interval in seconds
    
    NFTFeeConfig() : baseMintingFee(100 * COIN), usdTargetFee(20.0), 
                     dynamicPricing(true), priceUpdateInterval(3600) {}
};

/**
 * NFT Information
 * Represents a native NFT on SHAHCOIN Core
 */
struct CNFTInfo {
    uint256 nftHash;                 // Unique NFT hash
    std::string name;                // NFT name
    std::string description;         // NFT description
    std::string imageUrl;            // Image URL or IPFS hash
    std::string metadata;            // Additional metadata (JSON)
    CTxDestination creator;          // NFT creator address
    CTxDestination owner;            // Current NFT owner
    uint256 creationTxHash;          // Transaction that created the NFT
    int64_t creationTime;            // When the NFT was created
    bool isActive;                   // Whether NFT is active
    uint256 tokenId;                 // Unique token ID within collection
    uint256 collectionHash;          // Collection hash (if part of collection)
    
    CNFTInfo() : creationTime(0), isActive(true), tokenId(0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
    std::string GetDisplayName() const;
    bool IsPartOfCollection() const;
};

/**
 * NFT Collection Information
 * Represents a collection of NFTs
 */
struct CNFTCollection {
    uint256 collectionHash;          // Unique collection hash
    std::string name;                // Collection name
    std::string description;         // Collection description
    std::string imageUrl;            // Collection image URL
    CTxDestination creator;          // Collection creator
    uint256 creationTxHash;          // Transaction that created the collection
    int64_t creationTime;            // When the collection was created
    bool isActive;                   // Whether collection is active
    uint64_t totalSupply;            // Total NFTs in collection
    uint64_t mintedCount;            // Number of NFTs minted
    std::string metadata;            // Additional metadata (JSON)
    
    CNFTCollection() : creationTime(0), isActive(true), totalSupply(0), mintedCount(0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
    std::string GetDisplayName() const;
    bool CanMint() const;
};

/**
 * NFT Transaction Types
 * Different types of NFT operations
 */
enum class NFTTxType {
    CREATE_COLLECTION = 1,   // Create new NFT collection
    MINT_NFT = 2,           // Mint new NFT
    TRANSFER_NFT = 3,       // Transfer NFT ownership
    BURN_NFT = 4,           // Burn NFT
    UPDATE_METADATA = 5,    // Update NFT metadata
    APPROVE_NFT = 6,        // Approve NFT transfer
    TRANSFER_FROM_NFT = 7,  // Transfer from approved address
};

/**
 * NFT Transaction
 * Special transaction type for NFT operations
 */
struct CNFTTx {
    NFTTxType type;
    uint256 nftHash;                 // NFT being operated on
    uint256 collectionHash;          // Collection hash (for collection operations)
    CTxDestination from;             // Source address
    CTxDestination to;               // Destination address
    uint256 tokenId;                 // Token ID within collection
    int64_t timestamp;
    std::vector<unsigned char> signature;
    
    // NFT creation specific fields
    std::string nftName;
    std::string nftDescription;
    std::string imageUrl;
    std::string metadata;
    
    // Collection creation specific fields
    std::string collectionName;
    std::string collectionDescription;
    std::string collectionImageUrl;
    uint64_t collectionTotalSupply;
    
    CNFTTx() : type(NFTTxType::MINT_NFT), tokenId(0), timestamp(0), collectionTotalSupply(0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
    bool IsCreationTx() const;
    bool IsCollectionCreationTx() const;
};

/**
 * NFT Manager
 * Manages native NFTs and their operations
 */
class CNFTManager
{
public:
    CNFTManager();
    ~CNFTManager();
    
    // Configuration
    void SetFeeConfig(const NFTFeeConfig& config);
    NFTFeeConfig GetFeeConfig() const { return m_feeConfig; }
    
    // Collection management
    bool CreateCollection(const CTxDestination& creator, const std::string& name,
                         const std::string& description, const std::string& imageUrl,
                         uint64_t totalSupply);
    bool ValidateCollectionCreation(const CNFTTx& nftTx);
    
    // NFT minting
    bool MintNFT(const uint256& collectionHash, const CTxDestination& creator,
                const std::string& name, const std::string& description,
                const std::string& imageUrl, const std::string& metadata);
    bool ValidateNFTMinting(const CNFTTx& nftTx);
    
    // NFT operations
    bool TransferNFT(const uint256& nftHash, const CTxDestination& from,
                    const CTxDestination& to);
    bool BurnNFT(const uint256& nftHash, const CTxDestination& owner);
    bool ApproveNFT(const uint256& nftHash, const CTxDestination& owner,
                   const CTxDestination& spender);
    bool UpdateNFTMetadata(const uint256& nftHash, const CTxDestination& owner,
                          const std::string& metadata);
    
    // NFT queries
    CNFTInfo GetNFT(const uint256& nftHash) const;
    CNFTCollection GetCollection(const uint256& collectionHash) const;
    std::vector<CNFTInfo> GetNFTsByOwner(const CTxDestination& owner) const;
    std::vector<CNFTInfo> GetNFTsByCreator(const CTxDestination& creator) const;
    std::vector<CNFTInfo> GetNFTsInCollection(const uint256& collectionHash) const;
    std::vector<CNFTCollection> GetCollectionsByCreator(const CTxDestination& creator) const;
    std::vector<CNFTCollection> GetAllCollections() const;
    bool IsNFTOwner(const uint256& nftHash, const CTxDestination& address) const;
    bool IsNFTApproved(const uint256& nftHash, const CTxDestination& owner,
                      const CTxDestination& spender) const;
    
    // Fee calculation
    CAmount CalculateNFTMintingFee() const;
    CAmount GetCurrentSHAHPrice() const;
    void UpdateSHAHPrice();
    
    // Transaction creation
    CTransactionRef CreateCollectionCreationTx(const CTxDestination& creator, const std::string& name,
                                             const std::string& description, const std::string& imageUrl,
                                             uint64_t totalSupply);
    CTransactionRef CreateNFTMintingTx(const uint256& collectionHash, const CTxDestination& creator,
                                      const std::string& name, const std::string& description,
                                      const std::string& imageUrl, const std::string& metadata);
    CTransactionRef CreateNFTTransferTx(const uint256& nftHash, const CTxDestination& from,
                                       const CTxDestination& to);
    
    // Validation
    bool ValidateNFTTransaction(const CTransaction& tx) const;
    bool IsNFTTransaction(const CTransaction& tx) const;
    bool IsValidNFT(const uint256& nftHash) const;
    bool IsValidCollection(const uint256& collectionHash) const;
    
    // Statistics and monitoring
    struct NFTStats {
        uint64_t totalCollections;
        uint64_t activeCollections;
        uint64_t totalNFTs;
        uint64_t activeNFTs;
        uint64_t totalTransactions;
        CAmount totalMintingFees;
        int64_t lastUpdateTime;
    };
    
    NFTStats GetStats() const;
    void ResetStats();
    
    // Logging
    void LogCollectionCreation(const CNFTCollection& collection);
    void LogNFTMinting(const CNFTInfo& nft);
    void LogNFTTransaction(const CNFTTx& nftTx);
    void LogNFTStats();

private:
    // NFT storage
    std::map<uint256, CNFTInfo> m_nfts;
    std::map<uint256, CNFTCollection> m_collections;
    std::map<CTxDestination, std::vector<uint256>> m_ownerNFTs;
    std::map<CTxDestination, std::vector<uint256>> m_creatorNFTs;
    std::map<CTxDestination, std::vector<uint256>> m_creatorCollections;
    std::map<uint256, std::vector<uint256>> m_collectionNFTs;
    std::map<std::pair<uint256, CTxDestination>, CTxDestination> m_nftOwners;
    std::map<std::tuple<uint256, CTxDestination, CTxDestination>, bool> m_nftApprovals;
    
    // Configuration and statistics
    NFTFeeConfig m_feeConfig;
    NFTStats m_stats;
    CAmount m_currentSHAHPrice;  // Current SHAH price in USD cents
    
    // Helper functions
    void UpdateNFTIndexes(const CNFTInfo& nft, bool add);
    void RemoveNFTIndexes(const CNFTInfo& nft);
    void UpdateCollectionIndexes(const CNFTCollection& collection, bool add);
    void RemoveCollectionIndexes(const CNFTCollection& collection);
    bool ValidateNFTName(const std::string& name) const;
    bool ValidateCollectionName(const std::string& name) const;
    bool ValidateImageUrl(const std::string& url) const;
    bool ValidateMetadata(const std::string& metadata) const;
    void UpdateStats();
    void CleanupInactiveNFTs();
    
    // Ownership management
    bool UpdateNFTOwner(const uint256& nftHash, const CTxDestination& owner);
    bool UpdateNFTApproval(const uint256& nftHash, const CTxDestination& owner,
                          const CTxDestination& spender, bool approved);
    
    // Transaction helpers
    bool AddNFTTransaction(CTransaction& tx, const CNFTTx& nftTx);
    bool ParseNFTTransaction(const CTransaction& tx, CNFTTx& nftTx) const;
    
    // Price oracle
    CAmount FetchSHAHPrice() const;
    void CacheSHAHPrice(CAmount price);
    
    // Token ID generation
    uint256 GenerateTokenId(const uint256& collectionHash, const CTxDestination& creator);
    uint256 GenerateCollectionHash(const CTxDestination& creator, const std::string& name, int64_t timestamp);
};

/**
 * NFT Transaction Builder
 * Handles creation of NFT transactions
 */
class CNFTTxBuilder
{
public:
    CNFTTxBuilder();
    
    // Transaction building
    CTransactionRef BuildCollectionCreationTx(const CTxDestination& creator, const std::string& name,
                                            const std::string& description, const std::string& imageUrl,
                                            uint64_t totalSupply);
    CTransactionRef BuildNFTMintingTx(const uint256& collectionHash, const CTxDestination& creator,
                                     const std::string& name, const std::string& description,
                                     const std::string& imageUrl, const std::string& metadata);
    CTransactionRef BuildNFTTransferTx(const uint256& nftHash, const CTxDestination& from,
                                      const CTxDestination& to);
    
    // Script creation
    CScript CreateNFTScript(const CNFTTx& nftTx);
    CScript CreateCollectionCreationScript(const CNFTTx& nftTx);
    CScript CreateNFTMintingScript(const CNFTTx& nftTx);
    CScript CreateNFTTransferScript(const CNFTTx& nftTx);
    
    // Validation
    bool ValidateNFTScript(const CScript& script);
    bool ValidateCollectionCreationScript(const CScript& script);
    bool ValidateNFTMintingScript(const CScript& script);
    bool ValidateNFTTransferScript(const CScript& script);

private:
    // Script parsing
    bool ParseNFTScript(const CScript& script, CNFTTx& nftTx);
    bool ParseCollectionCreationScript(const CScript& script, CNFTTx& nftTx);
    bool ParseNFTMintingScript(const CScript& script, CNFTTx& nftTx);
    bool ParseNFTTransferScript(const CScript& script, CNFTTx& nftTx);
    
    // Transaction helpers
    void AddNFTOutput(CTransaction& tx, const CNFTTx& nftTx);
    void AddCollectionCreationOutput(CTransaction& tx, const CNFTTx& nftTx);
    void AddNFTMintingOutput(CTransaction& tx, const CNFTTx& nftTx);
    void AddNFTTransferOutput(CTransaction& tx, const CNFTTx& nftTx);
};

// Global NFT manager instance
extern std::unique_ptr<CNFTManager> g_nftManager;

// Utility functions
namespace NFTUtils {
    // Initialize NFT system
    bool InitializeNFTSystem();
    
    // Shutdown NFT system
    void ShutdownNFTSystem();
    
    // Check if NFT system is enabled
    bool IsNFTSystemEnabled();
    
    // Enable/disable NFT system
    void SetNFTSystemEnabled(bool enabled);
    
    // NFT helpers
    bool CanCreateCollection(const CTxDestination& creator, CAmount fee);
    bool CanMintNFT(const uint256& collectionHash, const CTxDestination& creator, CAmount fee);
    bool CanTransferNFT(const uint256& nftHash, const CTxDestination& from);
    CAmount GetNFTMintingFee();
    
    // Transaction validation
    bool IsNFTTransaction(const CTransaction& tx);
    bool ValidateNFTTransaction(const CTransaction& tx);
    
    // Get human-readable transaction type name
    std::string GetNFTTxTypeName(NFTTxType type);
    
    // NFT address generation
    uint256 GenerateNFTHash(const uint256& collectionHash, const CTxDestination& creator,
                           const std::string& name, int64_t timestamp);
    uint256 GenerateCollectionHash(const CTxDestination& creator, const std::string& name, int64_t timestamp);
}

#endif // SHAHCOIN_TOKENS_NFT_H
