// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/finality.h>
#include <chain.h>
#include <chainparams.h>
#include <logging.h>
#include <util/time.h>
#include <validation.h>

#include <algorithm>

// Global finality manager instance
std::unique_ptr<CFinalityManager> g_finalityManager = std::make_unique<CFinalityManager>();

// CFinalityManager implementation
CFinalityManager::CFinalityManager()
{
    m_stats = {0, 0, 0, 0, 0, GetTime()};
}

CFinalityManager::~CFinalityManager()
{
}

void CFinalityManager::SetConfig(const FinalityConfig& config)
{
    m_config = config;
    LogPrint(BCLog::CONSENSUS, "Finality configuration updated\n");
}

FinalityStatus CFinalityManager::GetBlockFinalityStatus(const CBlockIndex* pindex) const
{
    if (!pindex || !m_config.enableFinalityRules) {
        return FinalityStatus::PENDING;
    }
    
    // Check cache first
    auto it = m_finalityCache.find(pindex);
    if (it != m_finalityCache.end()) {
        return it->second;
    }
    
    // Calculate and cache status
    FinalityStatus status = CalculateFinalityStatus(pindex);
    m_finalityCache[pindex] = status;
    
    return status;
}

bool CFinalityManager::IsBlockFinal(const CBlockIndex* pindex) const
{
    if (!pindex || !m_config.enableFinalityRules) {
        return false;
    }
    
    FinalityStatus status = GetBlockFinalityStatus(pindex);
    return status >= FinalityStatus::SOFT_FINAL;
}

bool CFinalityManager::IsBlockIrreversible(const CBlockIndex* pindex) const
{
    if (!pindex || !m_config.enableFinalityRules) {
        return false;
    }
    
    FinalityStatus status = GetBlockFinalityStatus(pindex);
    return status == FinalityStatus::IRREVERSIBLE;
}

bool CFinalityManager::CanReorganize(const CBlockIndex* pindex) const
{
    if (!pindex || !m_config.enableFinalityRules) {
        return true;
    }
    
    return !IsBlockFinal(pindex);
}

bool CFinalityManager::ValidateReorganization(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew) const
{
    if (!m_config.enableFinalityRules) {
        return true;
    }
    
    if (!pindexOld || !pindexNew) {
        return false;
    }
    
    // Check if old chain is finalized
    if (IsBlockFinal(pindexOld)) {
        LogPrint(BCLog::CONSENSUS, "Reorganization blocked: old chain is finalized\n");
        m_stats.blockedReorganizations++;
        return false;
    }
    
    // Check reorganization depth
    int reorgDepth = GetReorganizationDepth(pindexOld, pindexNew);
    if (reorgDepth > m_config.maxReorgDepth) {
        LogPrint(BCLog::CONSENSUS, "Reorganization blocked: depth %d exceeds maximum %d\n", 
                reorgDepth, m_config.maxReorgDepth);
        m_stats.blockedReorganizations++;
        return false;
    }
    
    return true;
}

bool CFinalityManager::IsReorganizationAllowed(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew) const
{
    return ValidateReorganization(pindexOld, pindexNew);
}

int CFinalityManager::GetFinalityDepth(const CBlockIndex* pindex) const
{
    if (!pindex) {
        return 0;
    }
    
    return CalculateConfirmations(pindex);
}

std::vector<const CBlockIndex*> CFinalityManager::GetFinalizedBlocks() const
{
    std::vector<const CBlockIndex*> finalizedBlocks;
    
    const CBlockIndex* pindex = ::ChainActive().Tip();
    while (pindex && pindex->pprev) {
        if (IsBlockFinal(pindex)) {
            finalizedBlocks.push_back(pindex);
        }
        pindex = pindex->pprev;
    }
    
    return finalizedBlocks;
}

std::vector<const CBlockIndex*> CFinalityManager::GetIrreversibleBlocks() const
{
    std::vector<const CBlockIndex*> irreversibleBlocks;
    
    const CBlockIndex* pindex = ::ChainActive().Tip();
    while (pindex && pindex->pprev) {
        if (IsBlockIrreversible(pindex)) {
            irreversibleBlocks.push_back(pindex);
        }
        pindex = pindex->pprev;
    }
    
    return irreversibleBlocks;
}

CFinalityManager::FinalityStats CFinalityManager::GetStats() const
{
    return m_stats;
}

void CFinalityManager::ResetStats()
{
    m_stats = {0, 0, 0, 0, 0, GetTime()};
    LogPrint(BCLog::CONSENSUS, "Finality statistics reset\n");
}

void CFinalityManager::LogFinalityStatus(const CBlockIndex* pindex)
{
    if (!pindex) {
        return;
    }
    
    FinalityStatus status = GetBlockFinalityStatus(pindex);
    int confirmations = CalculateConfirmations(pindex);
    
    LogPrint(BCLog::CONSENSUS, "Block %s finality status: %s (confirmations: %d)\n",
            pindex->GetBlockHash().ToString(),
            FinalityUtils::GetFinalityStatusName(status),
            confirmations);
}

void CFinalityManager::LogReorganizationAttempt(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew)
{
    if (!pindexOld || !pindexNew) {
        return;
    }
    
    int reorgDepth = GetReorganizationDepth(pindexOld, pindexNew);
    bool allowed = IsReorganizationAllowed(pindexOld, pindexNew);
    
    LogPrint(BCLog::CONSENSUS, "Reorganization attempt: old=%s, new=%s, depth=%d, allowed=%s\n",
            pindexOld->GetBlockHash().ToString(),
            pindexNew->GetBlockHash().ToString(),
            reorgDepth,
            allowed ? "yes" : "no");
}

void CFinalityManager::LogFinalityStats()
{
    LogPrint(BCLog::CONSENSUS, "Finality statistics:\n");
    LogPrint(BCLog::CONSENSUS, "  Total blocks: %d\n", m_stats.totalBlocks);
    LogPrint(BCLog::CONSENSUS, "  Soft final blocks: %d\n", m_stats.softFinalBlocks);
    LogPrint(BCLog::CONSENSUS, "  Hard final blocks: %d\n", m_stats.hardFinalBlocks);
    LogPrint(BCLog::CONSENSUS, "  Irreversible blocks: %d\n", m_stats.irreversibleBlocks);
    LogPrint(BCLog::CONSENSUS, "  Blocked reorganizations: %d\n", m_stats.blockedReorganizations);
}

int CFinalityManager::CalculateConfirmations(const CBlockIndex* pindex) const
{
    if (!pindex) {
        return 0;
    }
    
    const CBlockIndex* pindexTip = ::ChainActive().Tip();
    if (!pindexTip) {
        return 0;
    }
    
    // Check if block is in main chain
    if (!IsBlockInMainChain(pindex)) {
        return 0;
    }
    
    return pindexTip->nHeight - pindex->nHeight + 1;
}

bool CFinalityManager::IsBlockInMainChain(const CBlockIndex* pindex) const
{
    if (!pindex) {
        return false;
    }
    
    const CBlockIndex* pindexTip = ::ChainActive().Tip();
    if (!pindexTip) {
        return false;
    }
    
    // Check if block is in the main chain
    while (pindexTip && pindexTip != pindex) {
        pindexTip = pindexTip->pprev;
    }
    
    return pindexTip == pindex;
}

int CFinalityManager::GetReorganizationDepth(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew) const
{
    if (!pindexOld || !pindexNew) {
        return 0;
    }
    
    // Find common ancestor
    const CBlockIndex* pindexCommon = pindexOld;
    while (pindexCommon && !IsBlockInMainChain(pindexCommon)) {
        pindexCommon = pindexCommon->pprev;
    }
    
    if (!pindexCommon) {
        return 0;
    }
    
    // Calculate depth from common ancestor to old tip
    int depth = 0;
    const CBlockIndex* pindex = pindexOld;
    while (pindex && pindex != pindexCommon) {
        depth++;
        pindex = pindex->pprev;
    }
    
    return depth;
}

void CFinalityManager::UpdateFinalityCache(const CBlockIndex* pindex) const
{
    if (!pindex) {
        return;
    }
    
    FinalityStatus status = CalculateFinalityStatus(pindex);
    m_finalityCache[pindex] = status;
    
    // Update statistics
    m_stats.totalBlocks++;
    switch (status) {
        case FinalityStatus::SOFT_FINAL:
            m_stats.softFinalBlocks++;
            break;
        case FinalityStatus::HARD_FINAL:
            m_stats.hardFinalBlocks++;
            break;
        case FinalityStatus::IRREVERSIBLE:
            m_stats.irreversibleBlocks++;
            break;
        default:
            break;
    }
}

void CFinalityManager::CleanupFinalityCache()
{
    // Remove entries for blocks that are no longer in memory
    // This is a simplified cleanup - in practice, you might want more sophisticated cache management
    m_finalityCache.clear();
}

FinalityStatus CFinalityManager::CalculateFinalityStatus(const CBlockIndex* pindex) const
{
    if (!pindex) {
        return FinalityStatus::PENDING;
    }
    
    int confirmations = CalculateConfirmations(pindex);
    
    if (MeetsIrreversibleCriteria(pindex)) {
        return FinalityStatus::IRREVERSIBLE;
    } else if (MeetsHardFinalityCriteria(pindex)) {
        return FinalityStatus::HARD_FINAL;
    } else if (MeetsSoftFinalityCriteria(pindex)) {
        return FinalityStatus::SOFT_FINAL;
    } else {
        return FinalityStatus::PENDING;
    }
}

bool CFinalityManager::MeetsSoftFinalityCriteria(const CBlockIndex* pindex) const
{
    if (!pindex) {
        return false;
    }
    
    int confirmations = CalculateConfirmations(pindex);
    return confirmations >= m_config.softFinalityConfirmations;
}

bool CFinalityManager::MeetsHardFinalityCriteria(const CBlockIndex* pindex) const
{
    if (!pindex) {
        return false;
    }
    
    int confirmations = CalculateConfirmations(pindex);
    return confirmations >= m_config.hardFinalityConfirmations;
}

bool CFinalityManager::MeetsIrreversibleCriteria(const CBlockIndex* pindex) const
{
    if (!pindex) {
        return false;
    }
    
    int confirmations = CalculateConfirmations(pindex);
    return confirmations >= m_config.irreversibleConfirmations;
}

// Utility functions implementation
namespace FinalityUtils {
    bool InitializeFinalitySystem()
    {
        if (!g_finalityManager) {
            g_finalityManager = std::make_unique<CFinalityManager>();
        }
        LogPrint(BCLog::CONSENSUS, "Finality system initialized\n");
        return true;
    }
    
    void ShutdownFinalitySystem()
    {
        if (g_finalityManager) {
            g_finalityManager->LogFinalityStats();
            g_finalityManager.reset();
        }
        LogPrint(BCLog::CONSENSUS, "Finality system shutdown\n");
    }
    
    bool IsBlockFinal(const CBlockIndex* pindex)
    {
        if (!g_finalityManager) {
            return false;
        }
        
        return g_finalityManager->IsBlockFinal(pindex);
    }
    
    bool IsReorganizationAllowed(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew)
    {
        if (!g_finalityManager) {
            return true;
        }
        
        return g_finalityManager->IsReorganizationAllowed(pindexOld, pindexNew);
    }
    
    std::string GetFinalityStatusName(FinalityStatus status)
    {
        switch (status) {
            case FinalityStatus::PENDING: return "PENDING";
            case FinalityStatus::SOFT_FINAL: return "SOFT_FINAL";
            case FinalityStatus::HARD_FINAL: return "HARD_FINAL";
            case FinalityStatus::IRREVERSIBLE: return "IRREVERSIBLE";
            default: return "UNKNOWN";
        }
    }
    
    bool IsFinalityEnabled()
    {
        return g_finalityManager != nullptr && g_finalityManager->GetConfig().enableFinalityRules;
    }
    
    void SetFinalityEnabled(bool enabled)
    {
        if (g_finalityManager) {
            FinalityConfig config = g_finalityManager->GetConfig();
            config.enableFinalityRules = enabled;
            g_finalityManager->SetConfig(config);
        }
    }
    
    int GetFinalityDepth(const CBlockIndex* pindex)
    {
        if (!g_finalityManager) {
            return 0;
        }
        
        return g_finalityManager->GetFinalityDepth(pindex);
    }
    
    bool ValidateChainReorganization(const CBlockIndex* pindexOld, const CBlockIndex* pindexNew)
    {
        if (!g_finalityManager) {
            return true;
        }
        
        return g_finalityManager->ValidateReorganization(pindexOld, pindexNew);
    }
}
