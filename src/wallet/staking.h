// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_WALLET_STAKING_H
#define SHAHCOIN_WALLET_STAKING_H

#include <wallet/wallet.h>
#include <stake/stake.h>
#include <consensus/amount.h>
#include <script/standard.h>

#include <vector>
#include <memory>

class CWallet;
class CBlock;
class CKey;

/** SHAHCOIN Core Wallet Staking Information */
struct CStakingInfo {
    bool isStakingEnabled;     // Whether staking is enabled for this wallet
    CAmount totalStake;        // Total amount staked
    CAmount availableStake;    // Amount available for staking
    int64_t stakeAge;         // Age of oldest stake
    uint32_t lastStakeTime;   // Last time a stake block was created
    std::vector<CTxDestination> stakingAddresses; // Addresses used for staking
    
    CStakingInfo() : isStakingEnabled(false), totalStake(0), availableStake(0), 
                     stakeAge(0), lastStakeTime(0) {}
};

/** SHAHCOIN Core Wallet Staking Manager */
class CWalletStakingManager {
private:
    CWallet* m_wallet;
    CStakingInfo m_stakingInfo;
    std::vector<CStakeValidator> m_validators;
    bool m_isStakingEnabled;
    
public:
    explicit CWalletStakingManager(CWallet* wallet);
    
    // Staking control
    bool EnableStaking();
    bool DisableStaking();
    bool IsStakingEnabled() const { return m_isStakingEnabled; }
    
    // Stake management
    bool AddStake(const CTxDestination& address, CAmount amount);
    bool RemoveStake(const CTxDestination& address);
    bool UpdateStake(const CTxDestination& address, CAmount newAmount);
    
    // Block creation
    bool CreateStakeBlock(CBlock& block, const CStakeValidator& validator);
    bool SignStakeBlock(CBlock& block, const CKey& key);
    
    // Stake validation
    bool IsValidStake(const CTransaction& tx) const;
    std::vector<CStakeValidator> GetValidStakes() const;
    CAmount GetTotalStake() const;
    CAmount GetAvailableStake() const;
    
    // Stake selection
    CStakeValidator SelectStakeForBlock() const;
    bool HasEligibleStake() const;
    
    // Information
    CStakingInfo GetStakingInfo() const;
    void UpdateStakingInfo();
    
    // Utility
    bool IsAddressStaking(const CTxDestination& address) const;
    std::vector<CTxDestination> GetStakingAddresses() const;
};

/** SHAHCOIN Core Staking RPC Functions */
namespace StakingRPC {
    // Get staking information
    UniValue GetStakingInfo(const CWallet& wallet);
    
    // Get stake statistics
    UniValue GetStakeStats(const CWallet& wallet);
    
    // Get validator list
    UniValue GetValidators(const CWallet& wallet);
    
    // Enable/disable staking
    bool EnableStaking(CWallet& wallet);
    bool DisableStaking(CWallet& wallet);
    
    // Stake management
    bool AddStake(CWallet& wallet, const CTxDestination& address, CAmount amount);
    bool RemoveStake(CWallet& wallet, const CTxDestination& address);
    
    // Block creation
    UniValue CreateStakeBlock(CWallet& wallet, const CTxDestination& address);
}

/** SHAHCOIN Core Staking Thread Management */
class CStakingThread {
private:
    CWallet* m_wallet;
    std::atomic<bool> m_running;
    std::thread m_thread;
    
public:
    explicit CStakingThread(CWallet* wallet);
    ~CStakingThread();
    
    void Start();
    void Stop();
    bool IsRunning() const { return m_running.load(); }
    
private:
    void StakingLoop();
    bool TryCreateStakeBlock();
    void SleepForStakeInterval();
};

#endif // SHAHCOIN_WALLET_STAKING_H
