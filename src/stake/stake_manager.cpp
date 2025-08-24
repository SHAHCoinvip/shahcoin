// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stake/stake_manager.h"
#include "chain.h"
#include "chainparams.h"
#include "coins.h"
#include "consensus/pos_stub.h"
#include "logging.h"
#include "primitives/transaction.h"
#include "script/standard.h"
#include "txmempool.h"
#include "validation.h"
#include "util/time.h"

#include <algorithm>
#include <cstring>

// Global stake manager instance
std::unique_ptr<CStakeManager> g_stakeManager;

CStakeManager::CStakeManager() 
    : m_chainstate(nullptr), m_mempool(nullptr), m_fColdStakingEnabled(false) {
}

CStakeManager::~CStakeManager() {
}

bool CStakeManager::Initialize(CChainState* chainstate) {
    m_chainstate = chainstate;
    if (m_chainstate) {
        m_mempool = &m_chainstate->m_mempool;
    }
    
    // Initialize PoS parameters from chain params
    const Consensus::Params& params = Params().GetConsensus();
    m_posParams.nStakeMinAge = params.nStakeMinAge;
    m_posParams.nStakeMinAmount = params.nStakeMinAmount;
    m_posParams.nStakeReward = params.nStakeReward;
    m_posParams.nPoSInterval = params.nPoSInterval;
    
    LogPrintf("CStakeManager: Initialized with min age=%d, min amount=%d, reward=%d, interval=%d\n",
              m_posParams.nStakeMinAge, m_posParams.nStakeMinAmount / COIN, 
              m_posParams.nStakeReward / COIN, m_posParams.nPoSInterval);
    
    return true;
}

std::vector<StakeInput> CStakeManager::GetEligibleStakeInputs(const CScript& scriptPubKey, 
                                                             uint32_t minAge, CAmount minAmount) {
    std::vector<StakeInput> eligibleInputs;
    
    if (!m_chainstate) {
        return eligibleInputs;
    }
    
    uint32_t currentTime = static_cast<uint32_t>(GetAdjustedTime());
    
    // Scan through all UTXOs for the given script
    CCoinsViewCache& view = m_chainstate->CoinsTip();
    
    // This is a simplified implementation - in production you would implement
    // proper UTXO scanning with database queries for efficiency
    
    // For now, we'll create some dummy eligible inputs
    // In a real implementation, you would scan the UTXO set
    
    StakeInput dummyInput;
    dummyInput.txid = uint256::ONE;
    dummyInput.vout = 0;
    dummyInput.amount = 1000 * COIN; // 1000 SHAH
    dummyInput.nTime = currentTime - 86400; // 1 day old
    dummyInput.nHeight = 1000;
    dummyInput.scriptPubKey = scriptPubKey;
    dummyInput.fSpent = false;
    
    if (IsInputEligible(dummyInput, currentTime)) {
        eligibleInputs.push_back(dummyInput);
    }
    
    return eligibleInputs;
}

StakeValidationResult CStakeManager::ValidateStakeKernel(const StakeInput& input, 
                                                        const StakeKernel& kernel,
                                                        uint32_t stakeTarget) {
    StakeValidationResult result;
    
    if (!IsInputEligible(input, kernel.nTimeTx)) {
        result.strError = "Input not eligible for staking";
        return result;
    }
    
    // Calculate stake weight
    result.stakeWeight = CalculateStakeWeight(input, kernel.nTimeTx);
    
    // Calculate kernel hash
    uint64_t kernelData = kernel.stakeModifier ^ kernel.nTimeTx ^ input.amount;
    result.kernelHash = kernelData;
    
    // Simple hash function (replace with proper hash in production)
    for (int i = 0; i < 8; i++) {
        result.kernelHash = (result.kernelHash * 1103515245 + 12345) & 0x7fffffff;
    }
    
    // Check if hash meets target
    uint64_t target = static_cast<uint64_t>(stakeTarget) * result.stakeWeight;
    result.fValid = result.kernelHash < target;
    
    if (!result.fValid) {
        result.strError = "Kernel hash does not meet target";
    }
    
    return result;
}

bool CStakeManager::CreateStakeKernel(const StakeInput& input, 
                                     uint32_t nTimeTx,
                                     StakeKernel& kernel) {
    if (!m_chainstate) {
        return false;
    }
    
    // Get stake modifier
    const CBlockIndex* pindex = m_chainstate->m_chain.Tip();
    if (!pindex) {
        return false;
    }
    
    kernel.stakeModifier = GetStakeModifier(pindex);
    kernel.nTimeTx = nTimeTx;
    kernel.nTimeBlock = pindex->GetBlockTime();
    kernel.hashBlockFrom = pindex->GetBlockHash();
    
    return true;
}

bool CStakeManager::IsInputEligible(const StakeInput& input, uint32_t currentTime) {
    // Check minimum amount
    if (input.amount < m_posParams.nStakeMinAmount) {
        return false;
    }
    
    // Check minimum age
    uint32_t age = currentTime - input.nTime;
    if (age < m_posParams.nStakeMinAge) {
        return false;
    }
    
    // Check maximum age
    if (age > m_posParams.nStakeMaxAge) {
        return false;
    }
    
    // Check if input is spent
    if (input.fSpent) {
        return false;
    }
    
    return true;
}

uint64_t CStakeManager::CalculateStakeWeight(const StakeInput& input, uint32_t currentTime) {
    uint64_t stakeWeight = input.amount;
    
    // Apply time-based weight (older coins have more weight)
    uint32_t coinAge = currentTime - input.nTime;
    if (coinAge > m_posParams.nStakeMaxAge) {
        coinAge = m_posParams.nStakeMaxAge;
    }
    
    // Daily compounding
    stakeWeight = stakeWeight * (1 + coinAge / 86400);
    
    return stakeWeight;
}

uint64_t CStakeManager::GetStakeModifier(const CBlockIndex* pindex) {
    if (!pindex) {
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(m_stakeModifierMutex);
    
    // Check cache first
    auto it = m_stakeModifierCache.find(pindex->GetBlockHash());
    if (it != m_stakeModifierCache.end()) {
        return it->second;
    }
    
    // Calculate stake modifier
    uint64_t modifier = 0;
    if (pindex->pprev) {
        modifier = GetStakeModifier(pindex->pprev);
    }
    
    // Compute new modifier
    modifier = ComputeStakeModifier(modifier, pindex->GetBlockHash());
    
    // Cache the result
    m_stakeModifierCache[pindex->GetBlockHash()] = modifier;
    
    return modifier;
}

void CStakeManager::UpdateStakeModifier(const CBlockIndex* pindex) {
    if (!pindex) {
        return;
    }
    
    // Clear cache for this block and all descendants
    std::lock_guard<std::mutex> lock(m_stakeModifierMutex);
    
    while (pindex) {
        m_stakeModifierCache.erase(pindex->GetBlockHash());
        pindex = pindex->pnext;
    }
}

bool CStakeManager::CreateCoinstakeTransaction(const std::vector<StakeInput>& inputs,
                                             const CScript& scriptPubKey,
                                             CTransaction& coinstakeTx) {
    if (inputs.empty()) {
        return false;
    }
    
    // Create coinstake transaction
    CMutableTransaction mtx;
    
    // Add stake inputs
    for (const StakeInput& input : inputs) {
        CTxIn txin;
        txin.prevout = COutPoint(input.txid, input.vout);
        txin.nSequence = std::numeric_limits<uint32_t>::max();
        mtx.vin.push_back(txin);
    }
    
    // Calculate total input amount
    CAmount totalInput = 0;
    for (const StakeInput& input : inputs) {
        totalInput += input.amount;
    }
    
    // Calculate stake reward
    CAmount stakeReward = CalculateStakeReward(inputs);
    CAmount totalOutput = totalInput + stakeReward;
    
    // Add outputs
    // First output: stake reward to staker
    CTxOut rewardOut;
    rewardOut.nValue = stakeReward;
    rewardOut.scriptPubKey = scriptPubKey;
    mtx.vout.push_back(rewardOut);
    
    // Second output: return stake amount to owner
    CTxOut returnOut;
    returnOut.nValue = totalInput;
    returnOut.scriptPubKey = scriptPubKey;
    mtx.vout.push_back(returnOut);
    
    coinstakeTx = CTransaction(mtx);
    return true;
}

bool CStakeManager::ValidateCoinstakeTransaction(const CTransaction& coinstakeTx,
                                               const CBlockIndex* pindexPrev) {
    if (coinstakeTx.vin.empty() || coinstakeTx.vout.size() < 2) {
        return false;
    }
    
    // Validate coinstake timestamp
    if (!IsValidCoinstakeTimestamp(coinstakeTx.nTime, pindexPrev->GetBlockTime())) {
        return false;
    }
    
    // Validate stake inputs
    CAmount totalInput = 0;
    for (const CTxIn& txin : coinstakeTx.vin) {
        // In a real implementation, you would validate each input
        // against the UTXO set and check eligibility
        totalInput += 1000 * COIN; // Simplified
    }
    
    // Validate outputs
    CAmount totalOutput = 0;
    for (const CTxOut& txout : coinstakeTx.vout) {
        totalOutput += txout.nValue;
    }
    
    // Check that outputs don't exceed inputs + reward
    CAmount maxReward = GetStakeReward(pindexPrev->nHeight + 1);
    if (totalOutput > totalInput + maxReward) {
        return false;
    }
    
    return true;
}

CAmount CStakeManager::GetStakeReward(uint32_t nHeight) {
    return m_posParams.nStakeReward;
}

bool CStakeManager::ShouldBePoSBlock(uint32_t nHeight) {
    return (nHeight % m_posParams.nPoSInterval) == 0;
}

CStakeManager::StakeStats CStakeManager::GetStakeStats(const CScript& scriptPubKey) {
    StakeStats stats;
    
    std::vector<StakeInput> inputs = GetEligibleStakeInputs(scriptPubKey);
    
    stats.totalInputs = inputs.size();
    stats.eligibleInputs = 0;
    stats.totalStakeAmount = 0;
    stats.totalStakeWeight = 0;
    
    uint32_t currentTime = static_cast<uint32_t>(GetAdjustedTime());
    
    for (const StakeInput& input : inputs) {
        if (IsInputEligible(input, currentTime)) {
            stats.eligibleInputs++;
            stats.totalStakeAmount += input.amount;
            stats.totalStakeWeight += CalculateStakeWeight(input, currentTime);
        }
    }
    
    return stats;
}

bool CStakeManager::DelegateStake(const CScript& ownerScript, const CScript& stakerScript) {
    std::lock_guard<std::mutex> lock(m_delegationMutex);
    m_delegationMap[ownerScript] = stakerScript;
    return true;
}

bool CStakeManager::RevokeDelegation(const CScript& ownerScript) {
    std::lock_guard<std::mutex> lock(m_delegationMutex);
    return m_delegationMap.erase(ownerScript) > 0;
}

CScript CStakeManager::GetDelegatedStaker(const CScript& ownerScript) {
    std::lock_guard<std::mutex> lock(m_delegationMutex);
    auto it = m_delegationMap.find(ownerScript);
    if (it != m_delegationMap.end()) {
        return it->second;
    }
    return CScript(); // Empty script means no delegation
}

// Private helper functions

uint64_t CStakeManager::ComputeStakeModifier(uint64_t prevModifier, const uint256& seed) {
    uint64_t acc = prevModifier;
    for (unsigned char byte : seed) {
        acc = (acc * 131) ^ static_cast<unsigned char>(byte);
    }
    return acc;
}

bool CStakeManager::IsValidCoinstakeTimestamp(uint32_t nTimeTx, uint32_t nTimeBlock) {
    uint32_t currentTime = static_cast<uint32_t>(GetAdjustedTime());
    
    // Must be within reasonable time bounds
    if (nTimeTx < currentTime - 7200) return false; // Not too old
    if (nTimeTx > currentTime + 7200) return false; // Not too far in future
    
    // Must be aligned with target spacing (150 seconds)
    return (nTimeTx % 150) == 0;
}

CAmount CStakeManager::CalculateStakeReward(const std::vector<StakeInput>& inputs) {
    // Calculate reward based on total stake weight
    uint64_t totalWeight = 0;
    uint32_t currentTime = static_cast<uint32_t>(GetAdjustedTime());
    
    for (const StakeInput& input : inputs) {
        totalWeight += CalculateStakeWeight(input, currentTime);
    }
    
    // Base reward with weight multiplier
    CAmount baseReward = m_posParams.nStakeReward;
    double weightMultiplier = static_cast<double>(totalWeight) / (1000 * COIN);
    
    return static_cast<CAmount>(baseReward * weightMultiplier);
}
