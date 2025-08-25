// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <wallet/staking.h>
#include <wallet/wallet.h>
#include <stake/stake.h>
#include <chain.h>
#include <chainparams.h>
#include <consensus/validation.h>
#include <key.h>
#include <logging.h>
#include <node/miner.h>
#include <script/standard.h>
#include <util/time.h>
#include <validation.h>

#include <algorithm>
#include <chrono>
#include <thread>

CWalletStakingManager::CWalletStakingManager(CWallet* wallet) 
    : m_wallet(wallet), m_isStakingEnabled(false) {
}

bool CWalletStakingManager::EnableStaking() {
    if (m_isStakingEnabled) {
        return true; // Already enabled
    }
    
    // Check if wallet has sufficient balance for staking
    CAmount totalBalance = m_wallet->GetBalance();
    if (totalBalance < POS_MIN_STAKE_AMOUNT) {
        LogPrint(BCLog::STAKING, "Insufficient balance for staking: %s < %s\n", 
                 FormatMoney(totalBalance), FormatMoney(POS_MIN_STAKE_AMOUNT));
        return false;
    }
    
    m_isStakingEnabled = true;
    LogPrint(BCLog::STAKING, "Staking enabled for wallet\n");
    return true;
}

bool CWalletStakingManager::DisableStaking() {
    if (!m_isStakingEnabled) {
        return true; // Already disabled
    }
    
    m_isStakingEnabled = false;
    LogPrint(BCLog::STAKING, "Staking disabled for wallet\n");
    return true;
}

bool CWalletStakingManager::AddStake(const CTxDestination& address, CAmount amount) {
    if (amount < POS_MIN_STAKE_AMOUNT) {
        LogPrint(BCLog::STAKING, "Stake amount too small: %s < %s\n", 
                 FormatMoney(amount), FormatMoney(POS_MIN_STAKE_AMOUNT));
        return false;
    }
    
    // Check if address has sufficient balance
    CAmount addressBalance = m_wallet->GetAddressBalance(address);
    if (addressBalance < amount) {
        LogPrint(BCLog::STAKING, "Insufficient balance for address %s: %s < %s\n", 
                 EncodeDestination(address), FormatMoney(addressBalance), FormatMoney(amount));
        return false;
    }
    
    // Add to stake manager
    uint256 dummyTxHash; // This would be the actual transaction hash in practice
    bool success = g_stakeManager->AddValidator(address, amount, dummyTxHash, GetTime());
    
    if (success) {
        // Add to local validators list
        CStakeValidator validator(address, amount, dummyTxHash, GetTime());
        m_validators.push_back(validator);
        
        LogPrint(BCLog::STAKING, "Added stake %s for address %s\n", 
                 FormatMoney(amount), EncodeDestination(address));
    }
    
    return success;
}

bool CWalletStakingManager::RemoveStake(const CTxDestination& address) {
    // Remove from stake manager
    bool success = g_stakeManager->RemoveValidator(address);
    
    if (success) {
        // Remove from local validators list
        m_validators.erase(
            std::remove_if(m_validators.begin(), m_validators.end(),
                [&address](const CStakeValidator& validator) {
                    return validator.address == address;
                }),
            m_validators.end()
        );
        
        LogPrint(BCLog::STAKING, "Removed stake for address %s\n", EncodeDestination(address));
    }
    
    return success;
}

bool CWalletStakingManager::UpdateStake(const CTxDestination& address, CAmount newAmount) {
    if (newAmount < POS_MIN_STAKE_AMOUNT) {
        LogPrint(BCLog::STAKING, "New stake amount too small: %s < %s\n", 
                 FormatMoney(newAmount), FormatMoney(POS_MIN_STAKE_AMOUNT));
        return false;
    }
    
    // Update in stake manager
    bool success = g_stakeManager->UpdateValidatorStake(address, newAmount);
    
    if (success) {
        // Update in local validators list
        for (auto& validator : m_validators) {
            if (validator.address == address) {
                validator.amount = newAmount;
                break;
            }
        }
        
        LogPrint(BCLog::STAKING, "Updated stake to %s for address %s\n", 
                 FormatMoney(newAmount), EncodeDestination(address));
    }
    
    return success;
}

bool CWalletStakingManager::CreateStakeBlock(CBlock& block, const CStakeValidator& validator) {
    // Create a new PoS block
    block.SetBlockType(BLOCK_TYPE_POS);
    block.nTime = GetTime();
    
    // Set stake data
    block.hashStake = validator.txHash;
    block.nStakeTime = validator.stakeTime;
    
    // Calculate stake kernel hash
    CStakeKernel kernel(validator.address, validator.amount, validator.stakeTime, block.nTime);
    block.hashStakeKernel = g_stakeManager->CalculateStakeKernelHash(kernel);
    
    // Create coinbase transaction
    CTransactionRef coinbaseTx = CreateCoinbaseTransaction(validator.address, POS_BLOCK_REWARD, 0);
    block.vtx.push_back(coinbaseTx);
    
    // Set merkle root
    block.hashMerkleRoot = BlockMerkleRoot(block);
    
    LogPrint(BCLog::STAKING, "Created PoS block with stake %s\n", FormatMoney(validator.amount));
    return true;
}

bool CWalletStakingManager::SignStakeBlock(CBlock& block, const CKey& key) {
    // Sign the block header
    uint256 blockHash = block.GetHash();
    std::vector<unsigned char> signature;
    
    if (!key.Sign(blockHash, signature)) {
        LogPrint(BCLog::STAKING, "Failed to sign PoS block\n");
        return false;
    }
    
    // Add signature to block (this would be stored in a custom field)
    // For now, we'll just log the signature
    LogPrint(BCLog::STAKING, "Signed PoS block with signature\n");
    return true;
}

bool CWalletStakingManager::IsValidStake(const CTransaction& tx) const {
    // Check if transaction is stakeable
    CCoinsViewCache view(nullptr); // This should be the actual UTXO view
    return StakeValidation::IsStakeable(tx, view);
}

std::vector<CStakeValidator> CWalletStakingManager::GetValidStakes() const {
    return m_validators;
}

CAmount CWalletStakingManager::GetTotalStake() const {
    CAmount total = 0;
    for (const auto& validator : m_validators) {
        total += validator.amount;
    }
    return total;
}

CAmount CWalletStakingManager::GetAvailableStake() const {
    CAmount total = 0;
    for (const auto& validator : m_validators) {
        // Check if stake is eligible (meets age requirement)
        int64_t stakeAge = GetTime() - validator.stakeTime;
        if (stakeAge >= POS_MIN_STAKE_AGE) {
            total += validator.amount;
        }
    }
    return total;
}

CStakeValidator CWalletStakingManager::SelectStakeForBlock() const {
    // Simple selection: choose the stake with highest amount and sufficient age
    CStakeValidator bestStake;
    CAmount bestAmount = 0;
    
    for (const auto& validator : m_validators) {
        int64_t stakeAge = GetTime() - validator.stakeTime;
        if (stakeAge >= POS_MIN_STAKE_AGE && validator.amount > bestAmount) {
            bestStake = validator;
            bestAmount = validator.amount;
        }
    }
    
    return bestStake;
}

bool CWalletStakingManager::HasEligibleStake() const {
    for (const auto& validator : m_validators) {
        int64_t stakeAge = GetTime() - validator.stakeTime;
        if (stakeAge >= POS_MIN_STAKE_AGE) {
            return true;
        }
    }
    return false;
}

CStakingInfo CWalletStakingManager::GetStakingInfo() const {
    CStakingInfo info;
    info.isStaking = m_isStakingEnabled && HasEligibleStake();
    info.totalStake = GetTotalStake();
    info.availableStake = GetAvailableStake();
    info.validatorCount = m_validators.size();
    info.expectedTime = CalculateExpectedStakeTime();
    return info;
}

void CWalletStakingManager::UpdateStakingInfo() {
    // Update staking information
    // This could include recalculating expected times, updating validator status, etc.
}

bool CWalletStakingManager::IsAddressStaking(const CTxDestination& address) const {
    for (const auto& validator : m_validators) {
        if (validator.address == address) {
            return true;
        }
    }
    return false;
}

std::vector<CTxDestination> CWalletStakingManager::GetStakingAddresses() const {
    std::vector<CTxDestination> addresses;
    for (const auto& validator : m_validators) {
        addresses.push_back(validator.address);
    }
    return addresses;
}

int64_t CWalletStakingManager::CalculateExpectedStakeTime() const {
    // Simple calculation: based on total stake and network difficulty
    CAmount totalStake = GetTotalStake();
    if (totalStake == 0) {
        return 0;
    }
    
    // This is a simplified calculation - in practice, it would be more complex
    // based on the actual stake difficulty and network parameters
    return 3600; // 1 hour as placeholder
}

// StakingRPC namespace implementation
namespace StakingRPC {

UniValue GetStakingInfo(const CWallet& wallet) {
    CWalletStakingManager stakingManager(const_cast<CWallet*>(&wallet));
    CStakingInfo info = stakingManager.GetStakingInfo();
    
    UniValue result(UniValue::VOBJ);
    result.pushKV("enabled", stakingManager.IsStakingEnabled());
    result.pushKV("staking", info.isStaking);
    result.pushKV("total_stake", ValueFromAmount(info.totalStake));
    result.pushKV("available_stake", ValueFromAmount(info.availableStake));
    result.pushKV("validator_count", info.validatorCount);
    result.pushKV("expected_time", info.expectedTime);
    
    return result;
}

UniValue GetStakeStats(const CWallet& wallet) {
    CWalletStakingManager stakingManager(const_cast<CWallet*>(&wallet));
    
    UniValue result(UniValue::VOBJ);
    result.pushKV("total_stake", ValueFromAmount(stakingManager.GetTotalStake()));
    result.pushKV("available_stake", ValueFromAmount(stakingManager.GetAvailableStake()));
    result.pushKV("validator_count", stakingManager.GetValidStakes().size());
    result.pushKV("has_eligible_stake", stakingManager.HasEligibleStake());
    
    return result;
}

UniValue GetValidators(const CWallet& wallet) {
    CWalletStakingManager stakingManager(const_cast<CWallet*>(&wallet));
    std::vector<CStakeValidator> validators = stakingManager.GetValidStakes();
    
    UniValue result(UniValue::VARR);
    for (const auto& validator : validators) {
        UniValue validator_obj(UniValue::VOBJ);
        validator_obj.pushKV("address", EncodeDestination(validator.address));
        validator_obj.pushKV("amount", ValueFromAmount(validator.amount));
        validator_obj.pushKV("age", GetTime() - validator.stakeTime);
        validator_obj.pushKV("eligible", (GetTime() - validator.stakeTime) >= POS_MIN_STAKE_AGE);
        result.push_back(validator_obj);
    }
    
    return result;
}

bool EnableStaking(CWallet& wallet) {
    CWalletStakingManager stakingManager(&wallet);
    return stakingManager.EnableStaking();
}

bool DisableStaking(CWallet& wallet) {
    CWalletStakingManager stakingManager(&wallet);
    return stakingManager.DisableStaking();
}

bool AddStake(CWallet& wallet, const CTxDestination& address, CAmount amount) {
    CWalletStakingManager stakingManager(&wallet);
    return stakingManager.AddStake(address, amount);
}

bool RemoveStake(CWallet& wallet, const CTxDestination& address) {
    CWalletStakingManager stakingManager(&wallet);
    return stakingManager.RemoveStake(address);
}

UniValue CreateStakeBlock(CWallet& wallet, const CTxDestination& address) {
    CWalletStakingManager stakingManager(&wallet);
    
    // Find the validator for this address
    CStakeValidator validator;
    bool found = false;
    for (const auto& v : stakingManager.GetValidStakes()) {
        if (v.address == address) {
            validator = v;
            found = true;
            break;
        }
    }
    
    if (!found) {
        throw std::runtime_error("No stake found for address");
    }
    
    // Create stake block
    CBlock block;
    if (!stakingManager.CreateStakeBlock(block, validator)) {
        throw std::runtime_error("Failed to create stake block");
    }
    
    UniValue result(UniValue::VOBJ);
    result.pushKV("block_hash", block.GetHash().ToString());
    result.pushKV("stake_amount", ValueFromAmount(validator.amount));
    result.pushKV("stake_age", GetTime() - validator.stakeTime);
    
    return result;
}

} // namespace StakingRPC

// CStakingThread implementation
CStakingThread::CStakingThread(CWallet* wallet) 
    : m_wallet(wallet), m_running(false) {
}

CStakingThread::~CStakingThread() {
    Stop();
}

void CStakingThread::Start() {
    if (m_running.load()) {
        return; // Already running
    }
    
    m_running = true;
    m_thread = std::thread(&CStakingThread::StakingLoop, this);
    LogPrint(BCLog::STAKING, "Staking thread started\n");
}

void CStakingThread::Stop() {
    if (!m_running.load()) {
        return; // Not running
    }
    
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
    LogPrint(BCLog::STAKING, "Staking thread stopped\n");
}

void CStakingThread::StakingLoop() {
    while (m_running.load()) {
        try {
            if (TryCreateStakeBlock()) {
                // Successfully created a stake block
                SleepForStakeInterval();
            } else {
                // No eligible stake or other conditions not met
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
        } catch (const std::exception& e) {
            LogPrint(BCLog::STAKING, "Error in staking loop: %s\n", e.what());
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
}

bool CStakingThread::TryCreateStakeBlock() {
    CWalletStakingManager stakingManager(m_wallet);
    
    // Check if staking is enabled
    if (!stakingManager.IsStakingEnabled()) {
        return false;
    }
    
    // Check if we have eligible stakes
    if (!stakingManager.HasEligibleStake()) {
        return false;
    }
    
    // Check if it's time for a PoS block
    int currentHeight = ::ChainActive().Height();
    if (!StakeValidation::ShouldBeProofOfStake(currentHeight + 1)) {
        return false;
    }
    
    // Select best stake
    CStakeValidator validator = stakingManager.SelectStakeForBlock();
    if (validator.address == CTxDestination()) {
        return false;
    }
    
    // Create stake block
    CBlock block;
    if (!stakingManager.CreateStakeBlock(block, validator)) {
        return false;
    }
    
    // Try to sign the block
    CKey key;
    if (!m_wallet->GetKey(validator.address, key)) {
        LogPrint(BCLog::STAKING, "No key found for staking address\n");
        return false;
    }
    
    if (!stakingManager.SignStakeBlock(block, key)) {
        LogPrint(BCLog::STAKING, "Failed to sign stake block\n");
        return false;
    }
    
    // Submit the block (this would integrate with the mining system)
    LogPrint(BCLog::STAKING, "Created and signed PoS block with stake %s\n", 
             FormatMoney(validator.amount));
    
    return true;
}

void CStakingThread::SleepForStakeInterval() {
    // Sleep for a reasonable interval before trying again
    std::this_thread::sleep_for(std::chrono::seconds(60));
}
