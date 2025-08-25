// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_STAKE_STAKE_MANAGER_H
#define SHAHCOIN_STAKE_STAKE_MANAGER_H

#include <cstdint>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <primitives/transaction.h>
#include <primitives/block.h>
#include <consensus/amount.h>

// Forward declarations
class CBlockIndex;
class CChainState;
class CTxMemPool;

// Stake input information
struct StakeInput {
    uint256 txid;
    uint32_t vout;
    CAmount amount;
    uint32_t nTime;
    uint32_t nHeight;
    CScript scriptPubKey;
    bool fSpent;
    
    StakeInput() : vout(0), amount(0), nTime(0), nHeight(0), fSpent(false) {}
};

// Stake kernel information
struct StakeKernel {
    uint64_t stakeModifier;
    uint32_t nTimeTx;
    uint32_t nTimeBlock;
    uint256 hashBlockFrom;
    
    StakeKernel() : stakeModifier(0), nTimeTx(0), nTimeBlock(0) {}
};

// Stake validation result
struct StakeValidationResult {
    bool fValid;
    std::string strError;
    uint64_t stakeWeight;
    uint64_t kernelHash;
    
    StakeValidationResult() : fValid(false), stakeWeight(0), kernelHash(0) {}
};

// PoS staking manager
class CStakeManager {
public:
    CStakeManager();
    ~CStakeManager();
    
    // Initialize stake manager
    bool Initialize(CChainState* chainstate);
    
    // Get eligible stake inputs for a given address
    std::vector<StakeInput> GetEligibleStakeInputs(const CScript& scriptPubKey, 
                                                  uint32_t minAge = 3600,
                                                  CAmount minAmount = 333 * COIN);
    
    // Validate stake kernel
    StakeValidationResult ValidateStakeKernel(const StakeInput& input, 
                                             const StakeKernel& kernel,
                                             uint32_t stakeTarget);
    
    // Create stake kernel
    bool CreateStakeKernel(const StakeInput& input, 
                          uint32_t nTimeTx,
                          StakeKernel& kernel);
    
    // Check if input is eligible for staking
    bool IsInputEligible(const StakeInput& input, uint32_t currentTime);
    
    // Calculate stake weight
    uint64_t CalculateStakeWeight(const StakeInput& input, uint32_t currentTime);
    
    // Get stake modifier
    uint64_t GetStakeModifier(const CBlockIndex* pindex);
    
    // Update stake modifier
    void UpdateStakeModifier(const CBlockIndex* pindex);
    
    // Create coinstake transaction
    bool CreateCoinstakeTransaction(const std::vector<StakeInput>& inputs,
                                  const CScript& scriptPubKey,
                                  CTransaction& coinstakeTx);
    
    // Validate coinstake transaction
    bool ValidateCoinstakeTransaction(const CTransaction& coinstakeTx,
                                    const CBlockIndex* pindexPrev);
    
    // Get stake reward
    CAmount GetStakeReward(uint32_t nHeight);
    
    // Check if block should be PoS
    bool ShouldBePoSBlock(uint32_t nHeight);
    
    // Get PoS parameters
    struct PoSParams {
        uint32_t nStakeMinAge;
        uint32_t nStakeMaxAge;
        CAmount nStakeMinAmount;
        CAmount nStakeReward;
        uint32_t nPoSInterval;
        
        PoSParams() : nStakeMinAge(3600), nStakeMaxAge(7776000), 
                     nStakeMinAmount(333 * COIN), nStakeReward(5 * COIN), 
                     nPoSInterval(10) {}
    };
    
    PoSParams GetPoSParams() const { return m_posParams; }
    void SetPoSParams(const PoSParams& params) { m_posParams = params; }
    
    // Stake statistics
    struct StakeStats {
        uint64_t totalStakeWeight;
        uint32_t eligibleInputs;
        uint32_t totalInputs;
        CAmount totalStakeAmount;
        
        StakeStats() : totalStakeWeight(0), eligibleInputs(0), 
                      totalInputs(0), totalStakeAmount(0) {}
    };
    
    StakeStats GetStakeStats(const CScript& scriptPubKey);
    
    // Cold staking support
    bool IsColdStakingEnabled() const { return m_fColdStakingEnabled; }
    void SetColdStakingEnabled(bool enabled) { m_fColdStakingEnabled = enabled; }
    
    // Delegated staking
    bool DelegateStake(const CScript& ownerScript, const CScript& stakerScript);
    bool RevokeDelegation(const CScript& ownerScript);
    CScript GetDelegatedStaker(const CScript& ownerScript);

private:
    CChainState* m_chainstate;
    CTxMemPool* m_mempool;
    PoSParams m_posParams;
    bool m_fColdStakingEnabled;
    
    // Stake modifier cache
    std::map<uint256, uint64_t> m_stakeModifierCache;
    std::mutex m_stakeModifierMutex;
    
    // Delegation map
    std::map<CScript, CScript> m_delegationMap;
    std::mutex m_delegationMutex;
    
    // Helper functions
    uint64_t ComputeStakeModifier(uint64_t prevModifier, const uint256& seed);
    bool IsValidCoinstakeTimestamp(uint32_t nTimeTx, uint32_t nTimeBlock);
    CAmount CalculateStakeReward(const std::vector<StakeInput>& inputs);
};

// Global stake manager instance
extern std::unique_ptr<CStakeManager> g_stakeManager;

#endif // SHAHCOIN_STAKE_STAKE_MANAGER_H
