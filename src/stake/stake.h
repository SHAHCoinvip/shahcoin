// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_STAKE_STAKE_H
#define SHAHCOIN_STAKE_STAKE_H

#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/standard.h>
#include <key.h>
#include <logging.h>
#include <util/time.h>
#include <chain.h>

#include <vector>
#include <map>
#include <set>
#include <memory>

class CWallet;
class CBlockIndex;

/**
 * Block Types
 * SHAHCOIN Core supports both PoW and PoS blocks
 */
enum BlockType {
    BLOCK_TYPE_POW = 0,    // Proof of Work block
    BLOCK_TYPE_POS = 1,    // Proof of Stake block
};

/**
 * Staking Configuration
 * Configurable parameters for staking
 */
struct StakingConfig {
    CAmount minStakeAmount;           // Minimum amount to stake (333 SHAH)
    int64_t minStakeAge;              // Minimum stake age in seconds (12 hours)
    int64_t maxStakeAge;              // Maximum stake age in seconds
    double stakeRewardPercent;        // Stake reward percentage
    int64_t stakeMaturity;            // Stake maturity time
    bool enableSlashing;              // Enable slashing (disabled for now)
    int64_t stakeTargetSpacing;       // Target spacing between PoS blocks
    int64_t stakeDifficultyAdjustment; // Difficulty adjustment interval for staking
    
    StakingConfig() : minStakeAmount(333 * COIN), minStakeAge(12 * 3600), maxStakeAge(90 * 24 * 3600),
                      stakeRewardPercent(0.05), stakeMaturity(100), enableSlashing(false),
                      stakeTargetSpacing(150), stakeDifficultyAdjustment(2016) {}
};

/**
 * Stake Transaction
 * Special transaction type for staking operations
 */
struct CStakeTx {
    uint256 stakeHash;                // Hash of the stake transaction
    CTxDestination stakerAddress;     // Address of the staker
    CAmount stakeAmount;              // Amount being staked
    int64_t stakeTime;                // When the stake was created
    int64_t stakeAge;                 // Age of the stake
    uint256 kernelHash;               // Kernel hash for stake validation
    std::vector<unsigned char> signature; // Staker signature
    
    CStakeTx() : stakeAmount(0), stakeTime(0), stakeAge(0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
    bool IsMature() const;
};

/**
 * Stake Manager
 * Manages staking operations and validator set
 */
class CStakeManager
{
public:
    CStakeManager();
    ~CStakeManager();
    
    // Configuration
    void SetConfig(const StakingConfig& config);
    StakingConfig GetConfig() const { return m_config; }
    
    // Staking operations
    bool CreateStake(const CTxDestination& address, CAmount amount);
    bool ValidateStake(const CStakeTx& stakeTx);
    bool CanStake(const CTxDestination& address, CAmount amount) const;
    CAmount GetStakeAmount(const CTxDestination& address) const;
    
    // Block creation
    bool CreateStakeBlock(const CTxDestination& staker, CBlock& block, const CBlockIndex* pindexPrev);
    bool ValidateStakeBlock(const CBlock& block, const CBlockIndex* pindexPrev);
    
    // Validator set management
    std::vector<CTxDestination> GetActiveValidators() const;
    bool IsValidator(const CTxDestination& address) const;
    int GetValidatorCount() const;
    
    // Difficulty and rewards
    uint256 CalculateStakeDifficulty(const CBlockIndex* pindexPrev);
    CAmount CalculateStakeReward(const CBlockIndex* pindexPrev);
    bool CheckStakeKernel(const CStakeTx& stakeTx, const CBlockIndex* pindexPrev);
    
    // Statistics and monitoring
    struct StakeStats {
        uint64_t totalStakes;
        uint64_t activeStakes;
        CAmount totalStakedAmount;
        CAmount activeStakedAmount;
        uint64_t totalStakeRewards;
        int64_t lastUpdateTime;
    };
    
    StakeStats GetStats() const;
    void ResetStats();
    
    // Logging
    void LogStake(const CStakeTx& stakeTx);
    void LogStakeBlock(const CBlock& block);
    void LogStakeStats();

private:
    // Stake storage
    std::map<CTxDestination, CStakeTx> m_stakes;
    std::map<CTxDestination, std::vector<CStakeTx>> m_addressStakes;
    
    // Configuration and statistics
    StakingConfig m_config;
    StakeStats m_stats;
    
    // Helper functions
    void UpdateStakeIndexes(const CStakeTx& stakeTx, bool add);
    void RemoveStakeIndexes(const CStakeTx& stakeTx);
    bool ValidateStakeAmount(CAmount amount) const;
    bool ValidateStakeAge(const CStakeTx& stakeTx) const;
    void UpdateStats();
    void CleanupExpiredStakes();
    
    // Kernel calculation
    uint256 CalculateKernelHash(const CStakeTx& stakeTx, const CBlockIndex* pindexPrev);
    bool VerifyKernelHash(const CStakeTx& stakeTx, const CBlockIndex* pindexPrev);
    
    // Block helpers
    bool AddStakeTransaction(CBlock& block, const CStakeTx& stakeTx);
    bool ValidateStakeTransaction(const CTransaction& tx);
};

/**
 * Stake Transaction Builder
 * Handles creation of stake transactions
 */
class CStakeTxBuilder
{
public:
    CStakeTxBuilder();
    
    // Transaction building
    CTransactionRef BuildStakeTx(const CTxDestination& staker, CAmount amount);
    CTransactionRef BuildStakeRewardTx(const CTxDestination& staker, CAmount reward);
    
    // Script creation
    CScript CreateStakeScript(const CTxDestination& staker);
    CScript CreateStakeRewardScript(const CTxDestination& staker);
    
    // Validation
    bool ValidateStakeScript(const CScript& script);
    bool ValidateStakeRewardScript(const CScript& script);

private:
    // Script parsing
    bool ParseStakeScript(const CScript& script, CTxDestination& staker);
    bool ParseStakeRewardScript(const CScript& script, CTxDestination& staker);
    
    // Transaction helpers
    void AddStakeOutput(CTransaction& tx, const CTxDestination& staker, CAmount amount);
    void AddStakeRewardOutput(CTransaction& tx, const CTxDestination& staker, CAmount reward);
};

// Global stake manager instance
extern std::unique_ptr<CStakeManager> g_stakeManager;

// Utility functions
namespace StakeUtils {
    // Initialize staking system
    bool InitializeStaking();
    
    // Shutdown staking system
    void ShutdownStaking();
    
    // Check if staking is enabled
    bool IsStakingEnabled();
    
    // Enable/disable staking
    void SetStakingEnabled(bool enabled);
    
    // Staking helpers
    bool CanCreateStake(const CTxDestination& address, CAmount amount);
    bool CanValidateBlock(const CTxDestination& address);
    CAmount GetStakeReward(const CBlockIndex* pindexPrev);
    
    // Transaction validation
    bool IsStakeTransaction(const CTransaction& tx);
    bool ValidateStakeTransaction(const CTransaction& tx);
    
    // Block validation
    bool IsStakeBlock(const CBlock& block);
    bool ValidateStakeBlock(const CBlock& block, const CBlockIndex* pindexPrev);
    
    // Get human-readable status
    std::string GetStakeStatusName(const CStakeTx& stakeTx);
}

#endif // SHAHCOIN_STAKE_STAKE_H
