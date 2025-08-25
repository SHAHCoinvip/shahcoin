// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_POS_MINER_H
#define SHAHCOIN_POS_MINER_H

#include <primitives/block.h>
#include <primitives/transaction.h>
#include <stake/stake.h>
#include <wallet/staking.h>
#include <chain.h>
#include <chainparams.h>
#include <key.h>
#include <logging.h>
#include <node/miner.h>
#include <script/standard.h>
#include <util/time.h>
#include <validation.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

class CWallet;

/**
 * PoS Mining Manager
 * Handles Proof of Stake block creation and submission
 */
class CPoSMiner
{
public:
    CPoSMiner();
    ~CPoSMiner();

    // Initialize the PoS miner
    bool Initialize(CWallet* wallet);
    
    // Start/stop PoS mining
    void StartMining();
    void StopMining();
    
    // Check if PoS mining is active
    bool IsMining() const { return m_mining.load(); }
    
    // Get mining statistics
    struct MiningStats {
        uint64_t blocksCreated;
        uint64_t blocksSubmitted;
        uint64_t blocksAccepted;
        int64_t lastBlockTime;
        CAmount totalRewards;
    };
    MiningStats GetStats() const;

private:
    // Main mining loop
    void MiningLoop();
    
    // Try to create and submit a PoS block
    bool TryCreatePoSBlock();
    
    // Create a PoS block with the given stake
    bool CreatePoSBlock(const CStakeValidator& validator, CBlock& block);
    
    // Submit block to the network
    bool SubmitBlock(const CBlock& block);
    
    // Validate stake for block creation
    bool ValidateStakeForBlock(const CStakeValidator& validator, int nHeight);
    
    // Calculate stake difficulty
    uint256 CalculateStakeDifficulty(const CStakeValidator& validator, int nHeight);
    
    // Check if it's time for a PoS block
    bool ShouldCreatePoSBlock(int nHeight);

private:
    CWallet* m_wallet;
    std::unique_ptr<CWalletStakingManager> m_stakingManager;
    std::unique_ptr<CStakingThread> m_stakingThread;
    
    std::atomic<bool> m_mining;
    std::atomic<bool> m_shouldStop;
    std::thread m_miningThread;
    
    // Statistics
    mutable std::mutex m_statsMutex;
    MiningStats m_stats;
    
    // Mining parameters
    static constexpr int MINING_INTERVAL_MS = 1000; // 1 second
    static constexpr int STAKE_CHECK_INTERVAL_MS = 10000; // 10 seconds
};

/**
 * PoS Block Builder
 * Handles the construction of PoS blocks
 */
class CPoSBlockBuilder
{
public:
    CPoSBlockBuilder();
    
    // Build a PoS block with the given stake
    bool BuildBlock(const CStakeValidator& validator, CBlock& block, int nHeight);
    
    // Add transactions to the block
    bool AddTransactions(CBlock& block, int nHeight);
    
    // Create coinbase transaction for PoS block
    CTransactionRef CreateCoinbaseTx(const CTxDestination& address, int nHeight);
    
    // Finalize the block
    bool FinalizeBlock(CBlock& block);

private:
    // Select transactions for the block
    std::vector<CTransactionRef> SelectTransactions(int nHeight);
    
    // Calculate block size
    size_t CalculateBlockSize(const CBlock& block);
    
    // Check transaction fees
    CAmount CalculateTotalFees(const std::vector<CTransactionRef>& transactions);
};

/**
 * PoS Block Validator
 * Validates PoS blocks before submission
 */
class CPoSBlockValidator
{
public:
    // Validate a PoS block
    static bool ValidateBlock(const CBlock& block, int nHeight);
    
    // Validate stake proof
    static bool ValidateStakeProof(const CBlock& block, const CStakeValidator& validator);
    
    // Validate block timing
    static bool ValidateBlockTiming(const CBlock& block, int nHeight);
    
    // Validate block difficulty
    static bool ValidateBlockDifficulty(const CBlock& block, int nHeight);

private:
    // Check stake age requirement
    static bool CheckStakeAge(const CStakeValidator& validator);
    
    // Check stake amount requirement
    static bool CheckStakeAmount(const CStakeValidator& validator);
    
    // Verify stake kernel
    static bool VerifyStakeKernel(const CBlock& block, const CStakeValidator& validator);
};

// Global PoS miner instance
extern std::unique_ptr<CPoSMiner> g_posMiner;

// Initialize PoS mining system
bool InitializePoSMining(CWallet* wallet);

// Shutdown PoS mining system
void ShutdownPoSMining();

#endif // SHAHCOIN_POS_MINER_H
