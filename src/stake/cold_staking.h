// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_STAKE_COLD_STAKING_H
#define SHAHCOIN_STAKE_COLD_STAKING_H

#include <primitives/transaction.h>
#include <primitives/block.h>
#include <script/standard.h>
#include <key.h>
#include <logging.h>
#include <util/time.h>
#include <stake/stake.h>

#include <vector>
#include <map>
#include <set>
#include <memory>

class CWallet;
class CBlockIndex;

/**
 * Cold Staking Delegation Status
 * Represents the status of a cold staking delegation
 */
enum class ColdStakingStatus {
    ACTIVE = 0,         // Delegation is active
    REVOKED = 1,        // Delegation has been revoked
    EXPIRED = 2,        // Delegation has expired
    PENDING = 3,        // Delegation is pending activation
};

/**
 * Cold Staking Delegation
 * Represents a cold staking delegation from cold wallet to hot wallet
 */
struct CColdStakingDelegation
{
    CTxDestination coldWalletAddress;   // Address of the cold wallet (funds owner)
    CTxDestination hotWalletAddress;    // Address of the hot wallet (staking delegate)
    CAmount delegatedAmount;            // Amount delegated for staking
    uint256 delegationTxHash;           // Transaction that created the delegation
    int64_t delegationTime;             // When the delegation was created
    int64_t expiryTime;                 // When the delegation expires (0 = no expiry)
    ColdStakingStatus status;           // Current status of the delegation
    uint256 revocationTxHash;           // Transaction that revoked the delegation (if any)
    int64_t revocationTime;             // When the delegation was revoked (if any)
    
    CColdStakingDelegation() : delegatedAmount(0), delegationTime(0), expiryTime(0),
                              status(ColdStakingStatus::PENDING), revocationTime(0) {}
    
    bool IsActive() const;
    bool IsExpired() const;
    bool IsRevoked() const;
    uint256 GetHash() const;
};

/**
 * Cold Staking Transaction
 * Special transaction type for cold staking operations
 */
struct CColdStakingTx
{
    enum class TxType {
        DELEGATE = 1,       // Delegate funds for cold staking
        REVOKE = 2,         // Revoke delegation
        STAKE = 3,          // Create stake using delegated funds
        WITHDRAW = 4,       // Withdraw staking rewards
    };
    
    TxType type;
    CTxDestination coldWalletAddress;
    CTxDestination hotWalletAddress;
    CAmount amount;
    uint256 delegationTxHash;  // Reference to original delegation
    int64_t timestamp;
    std::vector<unsigned char> signature;  // Cold wallet signature
    
    CColdStakingTx() : type(DELEGATE), amount(0), timestamp(0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
};

/**
 * Cold Staking Manager
 * Manages cold staking delegations and operations
 */
class CColdStakingManager
{
public:
    CColdStakingManager();
    ~CColdStakingManager();
    
    // Delegation management
    bool CreateDelegation(const CTxDestination& coldWallet, const CTxDestination& hotWallet,
                         CAmount amount, int64_t expiryTime = 0);
    bool RevokeDelegation(const CTxDestination& coldWallet, const CTxDestination& hotWallet);
    bool UpdateDelegation(const CColdStakingDelegation& delegation);
    
    // Delegation queries
    CColdStakingDelegation GetDelegation(const CTxDestination& coldWallet, const CTxDestination& hotWallet) const;
    std::vector<CColdStakingDelegation> GetDelegationsForColdWallet(const CTxDestination& coldWallet) const;
    std::vector<CColdStakingDelegation> GetDelegationsForHotWallet(const CTxDestination& hotWallet) const;
    std::vector<CColdStakingDelegation> GetActiveDelegations() const;
    
    // Staking operations
    bool CanStakeWithDelegation(const CTxDestination& hotWallet, const CTxDestination& coldWallet) const;
    CAmount GetDelegatedAmount(const CTxDestination& hotWallet) const;
    bool CreateStakeWithDelegation(const CTxDestination& hotWallet, const CTxDestination& coldWallet,
                                  CAmount amount, CBlock& block);
    
    // Transaction creation
    CTransactionRef CreateDelegationTransaction(const CTxDestination& coldWallet, const CTxDestination& hotWallet,
                                               CAmount amount, int64_t expiryTime = 0);
    CTransactionRef CreateRevocationTransaction(const CTxDestination& coldWallet, const CTxDestination& hotWallet);
    CTransactionRef CreateStakeTransaction(const CTxDestination& hotWallet, const CTxDestination& coldWallet,
                                          CAmount amount);
    
    // Validation
    bool ValidateDelegation(const CColdStakingDelegation& delegation) const;
    bool ValidateColdStakingTransaction(const CTransaction& tx) const;
    bool IsValidDelegation(const CTxDestination& coldWallet, const CTxDestination& hotWallet) const;
    
    // Statistics and monitoring
    struct ColdStakingStats {
        uint64_t totalDelegations;
        uint64_t activeDelegations;
        uint64_t revokedDelegations;
        uint64_t expiredDelegations;
        CAmount totalDelegatedAmount;
        CAmount activeDelegatedAmount;
        int64_t lastUpdateTime;
    };
    
    ColdStakingStats GetStats() const;
    void ResetStats();
    
    // Logging
    void LogDelegation(const CColdStakingDelegation& delegation);
    void LogRevocation(const CColdStakingDelegation& delegation);
    void LogColdStakingStats();

private:
    // Delegation storage
    std::map<std::pair<CTxDestination, CTxDestination>, CColdStakingDelegation> m_delegations;
    std::map<CTxDestination, std::vector<CColdStakingDelegation>> m_coldWalletDelegations;
    std::map<CTxDestination, std::vector<CColdStakingDelegation>> m_hotWalletDelegations;
    
    // Statistics
    ColdStakingStats m_stats;
    
    // Helper functions
    void UpdateDelegationIndexes(const CColdStakingDelegation& delegation, bool add);
    void RemoveDelegationIndexes(const CColdStakingDelegation& delegation);
    bool ValidateDelegationAmount(CAmount amount) const;
    bool ValidateDelegationAddresses(const CTxDestination& coldWallet, const CTxDestination& hotWallet) const;
    void UpdateStats();
    void CleanupExpiredDelegations();
    
    // Transaction helpers
    bool SignDelegationTransaction(CTransaction& tx, const CKey& coldWalletKey);
    bool VerifyDelegationSignature(const CTransaction& tx, const CPubKey& coldWalletPubKey);
    CScript CreateDelegationScript(const CTxDestination& coldWallet, const CTxDestination& hotWallet);
    bool ParseDelegationScript(const CScript& script, CTxDestination& coldWallet, CTxDestination& hotWallet);
};

/**
 * Cold Staking Transaction Builder
 * Handles creation of cold staking transactions
 */
class CColdStakingTxBuilder
{
public:
    CColdStakingTxBuilder();
    
    // Transaction building
    CTransactionRef BuildDelegationTx(const CTxDestination& coldWallet, const CTxDestination& hotWallet,
                                     CAmount amount, int64_t expiryTime = 0);
    CTransactionRef BuildRevocationTx(const CTxDestination& coldWallet, const CTxDestination& hotWallet);
    CTransactionRef BuildStakeTx(const CTxDestination& hotWallet, const CTxDestination& coldWallet,
                                CAmount amount, const CBlockIndex* pindexPrev);
    
    // Script creation
    CScript CreateDelegationScript(const CTxDestination& coldWallet, const CTxDestination& hotWallet);
    CScript CreateRevocationScript(const CTxDestination& coldWallet, const CTxDestination& hotWallet);
    CScript CreateStakeScript(const CTxDestination& hotWallet, const CTxDestination& coldWallet);
    
    // Validation
    bool ValidateDelegationScript(const CScript& script);
    bool ValidateRevocationScript(const CScript& script);
    bool ValidateStakeScript(const CScript& script);

private:
    // Script parsing
    bool ParseDelegationScript(const CScript& script, CTxDestination& coldWallet, CTxDestination& hotWallet);
    bool ParseRevocationScript(const CScript& script, CTxDestination& coldWallet, CTxDestination& hotWallet);
    bool ParseStakeScript(const CScript& script, CTxDestination& hotWallet, CTxDestination& coldWallet);
    
    // Transaction helpers
    void AddDelegationOutput(CTransaction& tx, const CTxDestination& coldWallet, const CTxDestination& hotWallet,
                            CAmount amount, int64_t expiryTime);
    void AddRevocationOutput(CTransaction& tx, const CTxDestination& coldWallet, const CTxDestination& hotWallet);
    void AddStakeOutput(CTransaction& tx, const CTxDestination& hotWallet, const CTxDestination& coldWallet,
                       CAmount amount);
};

// Global cold staking manager instance
extern std::unique_ptr<CColdStakingManager> g_coldStakingManager;

// Utility functions
namespace ColdStakingUtils {
    // Initialize cold staking system
    bool InitializeColdStaking();
    
    // Shutdown cold staking system
    void ShutdownColdStaking();
    
    // Check if cold staking is enabled
    bool IsColdStakingEnabled();
    
    // Enable/disable cold staking
    void SetColdStakingEnabled(bool enabled);
    
    // Delegation helpers
    bool CanCreateDelegation(const CTxDestination& coldWallet, CAmount amount);
    bool CanRevokeDelegation(const CTxDestination& coldWallet, const CTxDestination& hotWallet);
    bool CanStakeWithDelegation(const CTxDestination& hotWallet, const CTxDestination& coldWallet);
    
    // Transaction validation
    bool IsColdStakingTransaction(const CTransaction& tx);
    bool ValidateColdStakingTransaction(const CTransaction& tx);
    
    // Get human-readable status name
    std::string GetColdStakingStatusName(ColdStakingStatus status);
    
    // Get human-readable transaction type name
    std::string GetColdStakingTxTypeName(CColdStakingTx::TxType type);
}

#endif // SHAHCOIN_STAKE_COLD_STAKING_H
