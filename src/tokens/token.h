// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_TOKENS_TOKEN_H
#define SHAHCOIN_TOKENS_TOKEN_H

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
 * Token Creation Fee Configuration
 * Dynamic fee system based on SHAH price
 */
struct TokenFeeConfig {
    CAmount baseCreationFee;         // Base fee in SHAH (e.g., 500 SHAH)
    double usdTargetFee;             // Target fee in USD (e.g., $100)
    bool dynamicPricing;             // Enable dynamic pricing
    std::string priceOracleUrl;      // Price oracle endpoint
    int64_t priceUpdateInterval;     // Price update interval in seconds
    
    TokenFeeConfig() : baseCreationFee(500 * COIN), usdTargetFee(100.0), 
                       dynamicPricing(true), priceUpdateInterval(3600) {}
};

/**
 * Token Information
 * Represents a native token on SHAHCOIN Core
 */
struct CTokenInfo {
    uint256 tokenHash;               // Unique token hash
    std::string name;                // Token name
    std::string symbol;              // Token symbol
    uint8_t decimals;                // Token decimals (0-18)
    CAmount totalSupply;             // Total token supply
    CTxDestination creator;          // Token creator address
    uint256 creationTxHash;          // Transaction that created the token
    int64_t creationTime;            // When the token was created
    bool isActive;                   // Whether token is active
    std::string metadata;            // Additional metadata (JSON)
    
    CTokenInfo() : decimals(18), totalSupply(0), creationTime(0), isActive(true) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
    std::string GetDisplayName() const;
};

/**
 * Token Transaction Types
 * Different types of token operations
 */
enum class TokenTxType {
    CREATE = 1,      // Create new token
    TRANSFER = 2,    // Transfer tokens
    MINT = 3,        // Mint new tokens
    BURN = 4,        // Burn tokens
    APPROVE = 5,     // Approve spending
    TRANSFER_FROM = 6, // Transfer from approved address
};

/**
 * Token Transaction
 * Special transaction type for token operations
 */
struct CTokenTx {
    TokenTxType type;
    uint256 tokenHash;               // Token being operated on
    CTxDestination from;             // Source address
    CTxDestination to;               // Destination address
    CAmount amount;                  // Token amount
    uint256 approvalTxHash;          // Approval transaction hash (for transfer_from)
    int64_t timestamp;
    std::vector<unsigned char> signature;
    
    // Token creation specific fields
    std::string tokenName;
    std::string tokenSymbol;
    uint8_t tokenDecimals;
    CAmount tokenTotalSupply;
    
    CTokenTx() : type(TokenTxType::CREATE), amount(0), tokenDecimals(18), 
                 tokenTotalSupply(0), timestamp(0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
    bool IsCreationTx() const;
};

/**
 * Token Manager
 * Manages native tokens and their operations
 */
class CTokenManager
{
public:
    CTokenManager();
    ~CTokenManager();
    
    // Configuration
    void SetFeeConfig(const TokenFeeConfig& config);
    TokenFeeConfig GetFeeConfig() const { return m_feeConfig; }
    
    // Token creation
    bool CreateToken(const CTxDestination& creator, const std::string& name, 
                    const std::string& symbol, uint8_t decimals, CAmount totalSupply);
    bool ValidateTokenCreation(const CTokenTx& tokenTx);
    
    // Token operations
    bool TransferTokens(const uint256& tokenHash, const CTxDestination& from,
                       const CTxDestination& to, CAmount amount);
    bool MintTokens(const uint256& tokenHash, const CTxDestination& to, CAmount amount);
    bool BurnTokens(const uint256& tokenHash, const CTxDestination& from, CAmount amount);
    bool ApproveTokens(const uint256& tokenHash, const CTxDestination& owner,
                      const CTxDestination& spender, CAmount amount);
    
    // Token queries
    CTokenInfo GetToken(const uint256& tokenHash) const;
    std::vector<CTokenInfo> GetTokensByCreator(const CTxDestination& creator) const;
    std::vector<CTokenInfo> GetAllTokens() const;
    CAmount GetTokenBalance(const uint256& tokenHash, const CTxDestination& address) const;
    CAmount GetTokenAllowance(const uint256& tokenHash, const CTxDestination& owner,
                             const CTxDestination& spender) const;
    
    // Fee calculation
    CAmount CalculateTokenCreationFee() const;
    CAmount GetCurrentSHAHPrice() const;
    void UpdateSHAHPrice();
    
    // Transaction creation
    CTransactionRef CreateTokenCreationTx(const CTxDestination& creator, const std::string& name,
                                         const std::string& symbol, uint8_t decimals, CAmount totalSupply);
    CTransactionRef CreateTokenTransferTx(const uint256& tokenHash, const CTxDestination& from,
                                         const CTxDestination& to, CAmount amount);
    
    // Validation
    bool ValidateTokenTransaction(const CTransaction& tx) const;
    bool IsTokenTransaction(const CTransaction& tx) const;
    bool IsValidToken(const uint256& tokenHash) const;
    
    // Statistics and monitoring
    struct TokenStats {
        uint64_t totalTokens;
        uint64_t activeTokens;
        uint64_t totalTransactions;
        CAmount totalCreationFees;
        int64_t lastUpdateTime;
    };
    
    TokenStats GetStats() const;
    void ResetStats();
    
    // Logging
    void LogTokenCreation(const CTokenInfo& token);
    void LogTokenTransaction(const CTokenTx& tokenTx);
    void LogTokenStats();

private:
    // Token storage
    std::map<uint256, CTokenInfo> m_tokens;
    std::map<CTxDestination, std::vector<uint256>> m_creatorTokens;
    std::map<std::pair<uint256, CTxDestination>, CAmount> m_tokenBalances;
    std::map<std::tuple<uint256, CTxDestination, CTxDestination>, CAmount> m_tokenAllowances;
    
    // Configuration and statistics
    TokenFeeConfig m_feeConfig;
    TokenStats m_stats;
    CAmount m_currentSHAHPrice;  // Current SHAH price in USD cents
    
    // Helper functions
    void UpdateTokenIndexes(const CTokenInfo& token, bool add);
    void RemoveTokenIndexes(const CTokenInfo& token);
    bool ValidateTokenName(const std::string& name) const;
    bool ValidateTokenSymbol(const std::string& symbol) const;
    bool ValidateTokenDecimals(uint8_t decimals) const;
    bool ValidateTokenSupply(CAmount supply) const;
    void UpdateStats();
    void CleanupInactiveTokens();
    
    // Balance management
    bool UpdateTokenBalance(const uint256& tokenHash, const CTxDestination& address, CAmount amount);
    bool UpdateTokenAllowance(const uint256& tokenHash, const CTxDestination& owner,
                             const CTxDestination& spender, CAmount amount);
    
    // Transaction helpers
    bool AddTokenTransaction(CTransaction& tx, const CTokenTx& tokenTx);
    bool ParseTokenTransaction(const CTransaction& tx, CTokenTx& tokenTx) const;
    
    // Price oracle
    CAmount FetchSHAHPrice() const;
    void CacheSHAHPrice(CAmount price);
};

/**
 * Token Transaction Builder
 * Handles creation of token transactions
 */
class CTokenTxBuilder
{
public:
    CTokenTxBuilder();
    
    // Transaction building
    CTransactionRef BuildTokenCreationTx(const CTxDestination& creator, const std::string& name,
                                        const std::string& symbol, uint8_t decimals, CAmount totalSupply);
    CTransactionRef BuildTokenTransferTx(const uint256& tokenHash, const CTxDestination& from,
                                        const CTxDestination& to, CAmount amount);
    CTransactionRef BuildTokenMintTx(const uint256& tokenHash, const CTxDestination& to, CAmount amount);
    CTransactionRef BuildTokenBurnTx(const uint256& tokenHash, const CTxDestination& from, CAmount amount);
    
    // Script creation
    CScript CreateTokenScript(const CTokenTx& tokenTx);
    CScript CreateTokenCreationScript(const CTokenTx& tokenTx);
    CScript CreateTokenTransferScript(const CTokenTx& tokenTx);
    
    // Validation
    bool ValidateTokenScript(const CScript& script);
    bool ValidateTokenCreationScript(const CScript& script);
    bool ValidateTokenTransferScript(const CScript& script);

private:
    // Script parsing
    bool ParseTokenScript(const CScript& script, CTokenTx& tokenTx);
    bool ParseTokenCreationScript(const CScript& script, CTokenTx& tokenTx);
    bool ParseTokenTransferScript(const CScript& script, CTokenTx& tokenTx);
    
    // Transaction helpers
    void AddTokenOutput(CTransaction& tx, const CTokenTx& tokenTx);
    void AddTokenCreationOutput(CTransaction& tx, const CTokenTx& tokenTx);
    void AddTokenTransferOutput(CTransaction& tx, const CTokenTx& tokenTx);
};

// Global token manager instance
extern std::unique_ptr<CTokenManager> g_tokenManager;

// Utility functions
namespace TokenUtils {
    // Initialize token system
    bool InitializeTokenSystem();
    
    // Shutdown token system
    void ShutdownTokenSystem();
    
    // Check if token system is enabled
    bool IsTokenSystemEnabled();
    
    // Enable/disable token system
    void SetTokenSystemEnabled(bool enabled);
    
    // Token helpers
    bool CanCreateToken(const CTxDestination& creator, CAmount fee);
    bool CanTransferTokens(const uint256& tokenHash, const CTxDestination& from, CAmount amount);
    bool CanMintTokens(const uint256& tokenHash, const CTxDestination& minter);
    CAmount GetTokenCreationFee();
    
    // Transaction validation
    bool IsTokenTransaction(const CTransaction& tx);
    bool ValidateTokenTransaction(const CTransaction& tx);
    
    // Get human-readable transaction type name
    std::string GetTokenTxTypeName(TokenTxType type);
    
    // Token address generation
    uint256 GenerateTokenAddress(const CTxDestination& creator, const std::string& name, 
                                const std::string& symbol, int64_t timestamp);
}

#endif // SHAHCOIN_TOKENS_TOKEN_H
