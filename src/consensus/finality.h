// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_CONSENSUS_FINALITY_H
#define SHAHCOIN_CONSENSUS_FINALITY_H

#include <primitives/block.h>
#include <chain.h>
#include <chainparams.h>
#include <logging.h>
#include <util/time.h>

#include <map>
#include <set>
#include <vector>

class CBlockIndex;
class CChain;

/**
 * Finality Status
 * Represents the finality status of a block
 */
enum class FinalityStatus {
    PENDING = 0,        // Block is pending finality
    SOFT_FINAL = 1,     // Block is soft final (configurable confirmations)
    HARD_FINAL = 2,     // Block is hard final (economic finality)
    IRREVERSIBLE = 3,   // Block is irreversible (deep confirmations)
};

/**
 * Finality Configuration
 * Configurable parameters for finality rules
 */
struct FinalityConfig {
    int softFinalityConfirmations;     // Confirmations needed for soft finality (default: 100)
    int hardFinalityConfirmations;     // Confirmations needed for hard finality (default: 1000)
    int irreversibleConfirmations;     // Confirmations needed for irreversibility (default: 10000)
    int maxReorgDepth;                 // Maximum reorganization depth allowed
    bool enableFinalityRules;          // Whether finality rules are enabled
    int64_t finalityCheckInterval;     // Interval for finality checks in seconds
    
    FinalityConfig() : softFinalityConfirmations(100), hardFinalityConfirmations(1000),
                      irreversibleConfirmations(10000), maxReorgDepth(100),
                      enableFinalityRules(true), finalityCheckInterval(60) {}
};

/**
 * Finality Manager
 * Manages block finality and reorganization protection
 */
class CFinalityManager
{
public:
    CFinalityManager();
    ~CFinalityManager();
    
    // Configuration
    void SetConfig(const FinalityConfig& config);
    FinalityConfig GetConfig() const { return m_config; }
    
    // Finality checks
    FinalityStatus GetBlockFinalityStatus(const CBlockIndex* pindex) const;
    bool IsBlockFinal(const CBlockIndex* pindex) const;
    bool IsBlockIrreversible(const CBlockIndex* pindex) const;
    bool CanReorganize(const CBlockIndex* pindex) const;
    
    // Reorganization protection
    bool ValidateReorganization(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew) const;
    bool IsReorganizationAllowed(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew) const;
    
    // Chain analysis
    int GetFinalityDepth(const CBlockIndex* pindex) const;
    std::vector<const CBlockIndex*> GetFinalizedBlocks() const;
    std::vector<const CBlockIndex*> GetIrreversibleBlocks() const;
    
    // Statistics and monitoring
    struct FinalityStats {
        uint64_t totalBlocks;
        uint64_t softFinalBlocks;
        uint64_t hardFinalBlocks;
        uint64_t irreversibleBlocks;
        uint64_t blockedReorganizations;
        int64_t lastCheckTime;
    };
    
    FinalityStats GetStats() const;
    void ResetStats();
    
    // Logging
    void LogFinalityStatus(const CBlockIndex* pindex);
    void LogReorganizationAttempt(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew);
    void LogFinalityStats();

private:
    FinalityConfig m_config;
    FinalityStats m_stats;
    
    // Cached finality status
    mutable std::map<const CBlockIndex*, FinalityStatus> m_finalityCache;
    
    // Helper functions
    int CalculateConfirmations(const CBlockIndex* pindex) const;
    bool IsBlockInMainChain(const CBlockIndex* pindex) const;
    int GetReorganizationDepth(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew) const;
    void UpdateFinalityCache(const CBlockIndex* pindex) const;
    void CleanupFinalityCache();
    
    // Finality calculation
    FinalityStatus CalculateFinalityStatus(const CBlockIndex* pindex) const;
    bool MeetsSoftFinalityCriteria(const CBlockIndex* pindex) const;
    bool MeetsHardFinalityCriteria(const CBlockIndex* pindex) const;
    bool MeetsIrreversibleCriteria(const CBlockIndex* pindex) const;
};

// Global finality manager instance
extern std::unique_ptr<CFinalityManager> g_finalityManager;

// Utility functions
namespace FinalityUtils {
    // Initialize finality system
    bool InitializeFinalitySystem();
    
    // Shutdown finality system
    void ShutdownFinalitySystem();
    
    // Check if block is final
    bool IsBlockFinal(const CBlockIndex* pindex);
    
    // Check if reorganization is allowed
    bool IsReorganizationAllowed(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew);
    
    // Get human-readable finality status
    std::string GetFinalityStatusName(FinalityStatus status);
    
    // Check if finality rules are enabled
    bool IsFinalityEnabled();
    
    // Enable/disable finality rules
    void SetFinalityEnabled(bool enabled);
    
    // Get finality depth for a block
    int GetFinalityDepth(const CBlockIndex* pindex);
    
    // Validate chain reorganization
    bool ValidateChainReorganization(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew);
}

#endif // SHAHCOIN_CONSENSUS_FINALITY_H
