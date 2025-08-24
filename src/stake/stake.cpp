// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stake/stake.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/standard.h>
#include <key.h>
#include <logging.h>
#include <util/time.h>
#include <chain.h>
#include <validation.h>
#include <crypto/sha256.h>

#include <algorithm>

// Global stake manager instance
std::unique_ptr<CStakeManager> g_stakeManager = std::make_unique<CStakeManager>();

// CStakeTx implementation
uint256 CStakeTx::GetHash() const
{
    CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
    hasher << stakeHash << stakerAddress << stakeAmount << stakeTime << stakeAge << kernelHash;
    return hasher.GetHash();
}

bool CStakeTx::IsValid() const
{
    return stakeAmount > 0 && stakeTime > 0 && !stakeHash.IsNull();
}

bool CStakeTx::IsMature() const
{
    return stakeAge >= 12 * 3600; // 12 hours minimum stake age
}

// CStakeManager implementation
CStakeManager::CStakeManager()
{
    m_stats = {0, 0, 0, 0, 0, GetTime()};
}

CStakeManager::~CStakeManager()
{
}

void CStakeManager::SetConfig(const StakingConfig& config)
{
    m_config = config;
    LogPrint(BCLog::STAKING, "Staking configuration updated\n");
}

bool CStakeManager::CreateStake(const CTxDestination& address, CAmount amount)
{
    if (!ValidateStakeAmount(amount)) {
        LogPrint(BCLog::STAKING, "Invalid stake amount: %d\n", amount);
        return false;
    }
    
    CStakeTx stakeTx;
    stakeTx.stakerAddress = address;
    stakeTx.stakeAmount = amount;
    stakeTx.stakeTime = GetTime();
    stakeTx.stakeAge = 0;
    stakeTx.stakeHash = stakeTx.GetHash();
    
    // Add to storage
    m_stakes[address] = stakeTx;
    UpdateStakeIndexes(stakeTx, true);
    UpdateStats();
    
    LogPrint(BCLog::STAKING, "Created stake for address %s, amount: %d\n", 
             EncodeDestination(address), amount);
    
    return true;
}

bool CStakeManager::ValidateStake(const CStakeTx& stakeTx)
{
    if (!stakeTx.IsValid()) {
        return false;
    }
    
    if (!ValidateStakeAmount(stakeTx.stakeAmount)) {
        return false;
    }
    
    if (!ValidateStakeAge(stakeTx)) {
        return false;
    }
    
    return true;
}

bool CStakeManager::CanStake(const CTxDestination& address, CAmount amount) const
{
    if (!ValidateStakeAmount(amount)) {
        return false;
    }
    
    // Check if address already has a stake
    auto it = m_stakes.find(address);
    if (it != m_stakes.end()) {
        return false; // Already staking
    }
    
    return true;
}

CAmount CStakeManager::GetStakeAmount(const CTxDestination& address) const
{
    auto it = m_stakes.find(address);
    if (it != m_stakes.end()) {
        return it->second.stakeAmount;
    }
    return 0;
}

bool CStakeManager::CreateStakeBlock(const CTxDestination& staker, CBlock& block, const CBlockIndex* pindexPrev)
{
    auto it = m_stakes.find(staker);
    if (it == m_stakes.end()) {
        return false;
    }
    
    CStakeTx& stakeTx = it->second;
    
    // Update stake age
    stakeTx.stakeAge = GetTime() - stakeTx.stakeTime;
    
    // Calculate kernel hash
    stakeTx.kernelHash = CalculateKernelHash(stakeTx, pindexPrev);
    
    // Set block type to PoS
    block.SetBlockType(BLOCK_TYPE_POS);
    
    // Add stake transaction to block
    if (!AddStakeTransaction(block, stakeTx)) {
        return false;
    }
    
    // Set stake data in block header
    block.hashStake = stakeTx.stakeHash;
    block.nStakeTime = stakeTx.stakeTime;
    block.hashStakeKernel = stakeTx.kernelHash;
    
    LogPrint(BCLog::STAKING, "Created PoS block for staker %s\n", EncodeDestination(staker));
    
    return true;
}

bool CStakeManager::ValidateStakeBlock(const CBlock& block, const CBlockIndex* pindexPrev)
{
    if (!block.IsProofOfStake()) {
        return false;
    }
    
    // Validate stake transaction
    if (block.vtx.empty()) {
        return false;
    }
    
    const CTransaction& stakeTx = *block.vtx[0];
    if (!ValidateStakeTransaction(stakeTx)) {
        return false;
    }
    
    // Validate kernel hash
    CStakeTx stakeData;
    stakeData.stakeHash = block.hashStake;
    stakeData.stakeTime = block.nStakeTime;
    stakeData.kernelHash = block.hashStakeKernel;
    
    if (!VerifyKernelHash(stakeData, pindexPrev)) {
        return false;
    }
    
    return true;
}

std::vector<CTxDestination> CStakeManager::GetActiveValidators() const
{
    std::vector<CTxDestination> validators;
    for (const auto& pair : m_stakes) {
        if (ValidateStakeAge(pair.second)) {
            validators.push_back(pair.first);
        }
    }
    return validators;
}

bool CStakeManager::IsValidator(const CTxDestination& address) const
{
    auto it = m_stakes.find(address);
    if (it == m_stakes.end()) {
        return false;
    }
    return ValidateStakeAge(it->second);
}

int CStakeManager::GetValidatorCount() const
{
    return GetActiveValidators().size();
}

uint256 CStakeManager::CalculateStakeDifficulty(const CBlockIndex* pindexPrev)
{
    if (!pindexPrev) {
        return uint256();
    }
    
    // Simple difficulty calculation based on total staked amount
    CAmount totalStaked = 0;
    for (const auto& pair : m_stakes) {
        if (ValidateStakeAge(pair.second)) {
            totalStaked += pair.second.stakeAmount;
        }
    }
    
    if (totalStaked == 0) {
        return uint256();
    }
    
    // Difficulty inversely proportional to total staked amount
    arith_uint256 difficulty = arith_uint256(1000000) / arith_uint256(totalStaked / COIN);
    return ArithToUint256(difficulty);
}

CAmount CStakeManager::CalculateStakeReward(const CBlockIndex* pindexPrev)
{
    if (!pindexPrev) {
        return 0;
    }
    
    // Calculate reward based on total staked amount and block height
    CAmount totalStaked = 0;
    for (const auto& pair : m_stakes) {
        if (ValidateStakeAge(pair.second)) {
            totalStaked += pair.second.stakeAmount;
        }
    }
    
    if (totalStaked == 0) {
        return 0;
    }
    
    // Base reward is 100 SHAH, reduced by staking percentage
    CAmount baseReward = 100 * COIN;
    double stakingRatio = (double)totalStaked / (21 * 1000000 * COIN); // Assuming 21M total supply
    
    return (CAmount)(baseReward * (1.0 - stakingRatio * m_config.stakeRewardPercent));
}

bool CStakeManager::CheckStakeKernel(const CStakeTx& stakeTx, const CBlockIndex* pindexPrev)
{
    return VerifyKernelHash(stakeTx, pindexPrev);
}

CStakeManager::StakeStats CStakeManager::GetStats() const
{
    return m_stats;
}

void CStakeManager::ResetStats()
{
    m_stats = {0, 0, 0, 0, 0, GetTime()};
    LogPrint(BCLog::STAKING, "Staking statistics reset\n");
}

void CStakeManager::LogStake(const CStakeTx& stakeTx)
{
    LogPrint(BCLog::STAKING, "Stake: address=%s, amount=%d, time=%d, age=%d\n",
             EncodeDestination(stakeTx.stakerAddress), stakeTx.stakeAmount,
             stakeTx.stakeTime, stakeTx.stakeAge);
}

void CStakeManager::LogStakeBlock(const CBlock& block)
{
    LogPrint(BCLog::STAKING, "PoS Block: hash=%s, stake=%s, time=%d\n",
             block.GetHash().ToString(), block.hashStake.ToString(), block.nStakeTime);
}

void CStakeManager::LogStakeStats()
{
    LogPrint(BCLog::STAKING, "Staking statistics:\n");
    LogPrint(BCLog::STAKING, "  Total stakes: %d\n", m_stats.totalStakes);
    LogPrint(BCLog::STAKING, "  Active stakes: %d\n", m_stats.activeStakes);
    LogPrint(BCLog::STAKING, "  Total staked amount: %d\n", m_stats.totalStakedAmount);
    LogPrint(BCLog::STAKING, "  Active staked amount: %d\n", m_stats.activeStakedAmount);
    LogPrint(BCLog::STAKING, "  Total stake rewards: %d\n", m_stats.totalStakeRewards);
}

// Private helper functions
void CStakeManager::UpdateStakeIndexes(const CStakeTx& stakeTx, bool add)
{
    if (add) {
        m_addressStakes[stakeTx.stakerAddress].push_back(stakeTx);
    } else {
        auto& stakes = m_addressStakes[stakeTx.stakerAddress];
        stakes.erase(std::remove(stakes.begin(), stakes.end(), stakeTx), stakes.end());
    }
}

void CStakeManager::RemoveStakeIndexes(const CStakeTx& stakeTx)
{
    UpdateStakeIndexes(stakeTx, false);
}

bool CStakeManager::ValidateStakeAmount(CAmount amount) const
{
    return amount >= m_config.minStakeAmount;
}

bool CStakeManager::ValidateStakeAge(const CStakeTx& stakeTx) const
{
    int64_t currentAge = GetTime() - stakeTx.stakeTime;
    return currentAge >= m_config.minStakeAge && currentAge <= m_config.maxStakeAge;
}

void CStakeManager::UpdateStats()
{
    m_stats.totalStakes = m_stakes.size();
    m_stats.totalStakedAmount = 0;
    m_stats.activeStakes = 0;
    m_stats.activeStakedAmount = 0;
    
    for (const auto& pair : m_stakes) {
        m_stats.totalStakedAmount += pair.second.stakeAmount;
        if (ValidateStakeAge(pair.second)) {
            m_stats.activeStakes++;
            m_stats.activeStakedAmount += pair.second.stakeAmount;
        }
    }
    
    m_stats.lastUpdateTime = GetTime();
}

void CStakeManager::CleanupExpiredStakes()
{
    std::vector<CTxDestination> toRemove;
    
    for (const auto& pair : m_stakes) {
        if (!ValidateStakeAge(pair.second)) {
            toRemove.push_back(pair.first);
        }
    }
    
    for (const auto& address : toRemove) {
        auto it = m_stakes.find(address);
        if (it != m_stakes.end()) {
            RemoveStakeIndexes(it->second);
            m_stakes.erase(it);
        }
    }
}

uint256 CStakeManager::CalculateKernelHash(const CStakeTx& stakeTx, const CBlockIndex* pindexPrev)
{
    CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
    hasher << stakeTx.stakeHash;
    hasher << stakeTx.stakeTime;
    if (pindexPrev) {
        hasher << pindexPrev->GetBlockHash();
    }
    hasher << stakeTx.stakeAmount;
    return hasher.GetHash();
}

bool CStakeManager::VerifyKernelHash(const CStakeTx& stakeTx, const CBlockIndex* pindexPrev)
{
    uint256 expectedHash = CalculateKernelHash(stakeTx, pindexPrev);
    return stakeTx.kernelHash == expectedHash;
}

bool CStakeManager::AddStakeTransaction(CBlock& block, const CStakeTx& stakeTx)
{
    // Create stake transaction
    CMutableTransaction tx;
    tx.nVersion = 1;
    tx.vin.resize(1);
    tx.vout.resize(1);
    
    // Add stake output
    tx.vout[0].nValue = stakeTx.stakeAmount;
    tx.vout[0].scriptPubKey = GetScriptForDestination(stakeTx.stakerAddress);
    
    block.vtx.push_back(MakeTransactionRef(std::move(tx)));
    return true;
}

bool CStakeManager::ValidateStakeTransaction(const CTransaction& tx)
{
    if (tx.vin.empty() || tx.vout.empty()) {
        return false;
    }
    
    // Check that output amount is valid
    CAmount totalOutput = 0;
    for (const auto& output : tx.vout) {
        totalOutput += output.nValue;
    }
    
    return ValidateStakeAmount(totalOutput);
}

// CStakeTxBuilder implementation
CStakeTxBuilder::CStakeTxBuilder()
{
}

CTransactionRef CStakeTxBuilder::BuildStakeTx(const CTxDestination& staker, CAmount amount)
{
    CMutableTransaction tx;
    tx.nVersion = 1;
    tx.vin.resize(1);
    tx.vout.resize(1);
    
    AddStakeOutput(tx, staker, amount);
    
    return MakeTransactionRef(std::move(tx));
}

CTransactionRef CStakeTxBuilder::BuildStakeRewardTx(const CTxDestination& staker, CAmount reward)
{
    CMutableTransaction tx;
    tx.nVersion = 1;
    tx.vin.resize(1);
    tx.vout.resize(1);
    
    AddStakeRewardOutput(tx, staker, reward);
    
    return MakeTransactionRef(std::move(tx));
}

CScript CStakeTxBuilder::CreateStakeScript(const CTxDestination& staker)
{
    return GetScriptForDestination(staker);
}

CScript CStakeTxBuilder::CreateStakeRewardScript(const CTxDestination& staker)
{
    return GetScriptForDestination(staker);
}

bool CStakeTxBuilder::ValidateStakeScript(const CScript& script)
{
    CTxDestination dest;
    return ExtractDestination(script, dest);
}

bool CStakeTxBuilder::ValidateStakeRewardScript(const CScript& script)
{
    CTxDestination dest;
    return ExtractDestination(script, dest);
}

bool CStakeTxBuilder::ParseStakeScript(const CScript& script, CTxDestination& staker)
{
    return ExtractDestination(script, staker);
}

bool CStakeTxBuilder::ParseStakeRewardScript(const CScript& script, CTxDestination& staker)
{
    return ExtractDestination(script, staker);
}

void CStakeTxBuilder::AddStakeOutput(CTransaction& tx, const CTxDestination& staker, CAmount amount)
{
    tx.vout[0].nValue = amount;
    tx.vout[0].scriptPubKey = CreateStakeScript(staker);
}

void CStakeTxBuilder::AddStakeRewardOutput(CTransaction& tx, const CTxDestination& staker, CAmount reward)
{
    tx.vout[0].nValue = reward;
    tx.vout[0].scriptPubKey = CreateStakeRewardScript(staker);
}

// Utility functions implementation
namespace StakeUtils {
    bool InitializeStaking()
    {
        if (!g_stakeManager) {
            g_stakeManager = std::make_unique<CStakeManager>();
        }
        LogPrint(BCLog::STAKING, "Staking system initialized\n");
        return true;
    }
    
    void ShutdownStaking()
    {
        if (g_stakeManager) {
            g_stakeManager->LogStakeStats();
            g_stakeManager.reset();
        }
        LogPrint(BCLog::STAKING, "Staking system shutdown\n");
    }
    
    bool IsStakingEnabled()
    {
        return g_stakeManager != nullptr;
    }
    
    void SetStakingEnabled(bool enabled)
    {
        if (enabled && !g_stakeManager) {
            InitializeStaking();
        } else if (!enabled && g_stakeManager) {
            ShutdownStaking();
        }
    }
    
    bool CanCreateStake(const CTxDestination& address, CAmount amount)
    {
        if (!g_stakeManager) {
            return false;
        }
        return g_stakeManager->CanStake(address, amount);
    }
    
    bool CanValidateBlock(const CTxDestination& address)
    {
        if (!g_stakeManager) {
            return false;
        }
        return g_stakeManager->IsValidator(address);
    }
    
    CAmount GetStakeReward(const CBlockIndex* pindexPrev)
    {
        if (!g_stakeManager) {
            return 0;
        }
        return g_stakeManager->CalculateStakeReward(pindexPrev);
    }
    
    bool IsStakeTransaction(const CTransaction& tx)
    {
        // Check if transaction has stake-specific characteristics
        if (tx.vout.empty()) {
            return false;
        }
        
        // Look for stake script patterns
        for (const auto& output : tx.vout) {
            if (output.scriptPubKey.size() > 0) {
                // Add stake detection logic here
                return true;
            }
        }
        
        return false;
    }
    
    bool ValidateStakeTransaction(const CTransaction& tx)
    {
        if (!g_stakeManager) {
            return false;
        }
        return g_stakeManager->ValidateStakeTransaction(tx);
    }
    
    bool IsStakeBlock(const CBlock& block)
    {
        return block.IsProofOfStake();
    }
    
    bool ValidateStakeBlock(const CBlock& block, const CBlockIndex* pindexPrev)
    {
        if (!g_stakeManager) {
            return false;
        }
        return g_stakeManager->ValidateStakeBlock(block, pindexPrev);
    }
    
    std::string GetStakeStatusName(const CStakeTx& stakeTx)
    {
        if (!stakeTx.IsValid()) {
            return "INVALID";
        }
        if (stakeTx.IsMature()) {
            return "MATURE";
        }
        return "IMMATURE";
    }
}
