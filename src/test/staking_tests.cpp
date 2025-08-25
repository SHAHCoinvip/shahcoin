// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <test/util/setup_common.h>
#include <test/util/wallet.h>
#include <test/util/blockfilter.h>
#include <test/util/coins.h>
#include <test/util/transaction_utils.h>

#include <stake/stake.h>
#include <wallet/staking.h>
#include <consensus/consensus.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/standard.h>
#include <key.h>
#include <key_io.h>
#include <wallet/wallet.h>
#include <validation.h>
#include <chain.h>
#include <chainparams.h>
#include <util/time.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(staking_tests, TestingSetup)

BOOST_AUTO_TEST_CASE(stake_validator_creation)
{
    // Test CStakeValidator creation and basic properties
    CKey key;
    key.MakeNewKey(true);
    CTxDestination address = key.GetPubKey().GetID();
    CAmount amount = POS_MIN_STAKE_AMOUNT;
    uint256 txHash = uint256::ONE;
    int64_t stakeTime = GetTime();
    
    CStakeValidator validator(address, amount, txHash, stakeTime);
    
    BOOST_CHECK_EQUAL(validator.address, address);
    BOOST_CHECK_EQUAL(validator.amount, amount);
    BOOST_CHECK_EQUAL(validator.txHash, txHash);
    BOOST_CHECK_EQUAL(validator.stakeTime, stakeTime);
}

BOOST_AUTO_TEST_CASE(stake_kernel_creation)
{
    // Test CStakeKernel creation and hash calculation
    CKey key;
    key.MakeNewKey(true);
    CTxDestination address = key.GetPubKey().GetID();
    CAmount amount = POS_MIN_STAKE_AMOUNT;
    int64_t stakeTime = GetTime();
    int64_t blockTime = GetTime() + 1;
    
    CStakeKernel kernel(address, amount, stakeTime, blockTime);
    
    BOOST_CHECK_EQUAL(kernel.address, address);
    BOOST_CHECK_EQUAL(kernel.amount, amount);
    BOOST_CHECK_EQUAL(kernel.stakeTime, stakeTime);
    BOOST_CHECK_EQUAL(kernel.blockTime, blockTime);
    
    // Test hash calculation
    uint256 hash = kernel.GetHash();
    BOOST_CHECK(!hash.IsNull());
}

BOOST_AUTO_TEST_CASE(stake_manager_basic_operations)
{
    // Test CStakeManager basic operations
    CKey key;
    key.MakeNewKey(true);
    CTxDestination address = key.GetPubKey().GetID();
    CAmount amount = POS_MIN_STAKE_AMOUNT;
    uint256 txHash = uint256::ONE;
    int64_t stakeTime = GetTime();
    
    // Test adding validator
    bool success = g_stakeManager->AddValidator(address, amount, txHash, stakeTime);
    BOOST_CHECK(success);
    
    // Test getting validator
    CStakeValidator validator = g_stakeManager->GetValidator(address);
    BOOST_CHECK_EQUAL(validator.address, address);
    BOOST_CHECK_EQUAL(validator.amount, amount);
    
    // Test updating validator
    CAmount newAmount = POS_MIN_STAKE_AMOUNT * 2;
    success = g_stakeManager->UpdateValidatorStake(address, newAmount);
    BOOST_CHECK(success);
    
    validator = g_stakeManager->GetValidator(address);
    BOOST_CHECK_EQUAL(validator.amount, newAmount);
    
    // Test removing validator
    success = g_stakeManager->RemoveValidator(address);
    BOOST_CHECK(success);
    
    // Verify validator is removed
    validator = g_stakeManager->GetValidator(address);
    BOOST_CHECK(validator.address == CTxDestination());
}

BOOST_AUTO_TEST_CASE(stake_validation_functions)
{
    // Test stake validation functions
    CKey key;
    key.MakeNewKey(true);
    CTxDestination address = key.GetPubKey().GetID();
    CAmount amount = POS_MIN_STAKE_AMOUNT;
    uint256 txHash = uint256::ONE;
    int64_t stakeTime = GetTime();
    
    CStakeValidator validator(address, amount, txHash, stakeTime);
    
    // Test stake age validation
    bool isValid = StakeValidation::IsStakeEligible(validator);
    BOOST_CHECK(!isValid); // Should be false because stake is too new
    
    // Test with old stake
    validator.stakeTime = GetTime() - POS_MIN_STAKE_AGE - 1;
    isValid = StakeValidation::IsStakeEligible(validator);
    BOOST_CHECK(isValid);
    
    // Test stake amount validation
    validator.amount = POS_MIN_STAKE_AMOUNT - 1;
    isValid = StakeValidation::IsStakeEligible(validator);
    BOOST_CHECK(!isValid); // Should be false because amount is too small
}

BOOST_AUTO_TEST_CASE(proof_of_stake_block_creation)
{
    // Test PoS block creation
    CKey key;
    key.MakeNewKey(true);
    CTxDestination address = key.GetPubKey().GetID();
    CAmount amount = POS_MIN_STAKE_AMOUNT;
    uint256 txHash = uint256::ONE;
    int64_t stakeTime = GetTime() - POS_MIN_STAKE_AGE - 1;
    
    CStakeValidator validator(address, amount, txHash, stakeTime);
    
    // Create PoS block
    CBlock block;
    block.SetBlockType(BLOCK_TYPE_POS);
    block.nTime = GetTime();
    block.hashStake = validator.txHash;
    block.nStakeTime = validator.stakeTime;
    
    // Calculate stake kernel hash
    CStakeKernel kernel(validator.address, validator.amount, validator.stakeTime, block.nTime);
    block.hashStakeKernel = g_stakeManager->CalculateStakeKernelHash(kernel);
    
    // Verify block properties
    BOOST_CHECK(block.IsProofOfStake());
    BOOST_CHECK(!block.IsProofOfWork());
    BOOST_CHECK_EQUAL(block.GetBlockType(), BLOCK_TYPE_POS);
    BOOST_CHECK_EQUAL(block.hashStake, validator.txHash);
    BOOST_CHECK_EQUAL(block.nStakeTime, validator.stakeTime);
}

BOOST_AUTO_TEST_CASE(hybrid_consensus_block_alternation)
{
    // Test hybrid consensus block alternation
    for (int height = 1; height <= 10; height++) {
        bool shouldBePoS = StakeValidation::ShouldBeProofOfStake(height);
        
        if (shouldBePoS) {
            BOOST_CHECK_EQUAL(height % 2, 1); // Odd heights should be PoS
        } else {
            BOOST_CHECK_EQUAL(height % 2, 0); // Even heights should be PoW
        }
    }
}

BOOST_AUTO_TEST_CASE(stake_difficulty_calculation)
{
    // Test stake difficulty calculation
    uint256 difficulty = g_stakeManager->GetStakeDifficulty(nullptr);
    BOOST_CHECK(!difficulty.IsNull());
    
    // Test with different total stake amounts
    CKey key1, key2;
    key1.MakeNewKey(true);
    key2.MakeNewKey(true);
    CTxDestination address1 = key1.GetPubKey().GetID();
    CTxDestination address2 = key2.GetPubKey().GetID();
    
    g_stakeManager->AddValidator(address1, POS_MIN_STAKE_AMOUNT, uint256::ONE, GetTime());
    g_stakeManager->AddValidator(address2, POS_MIN_STAKE_AMOUNT * 2, uint256::TWO, GetTime());
    
    uint256 newDifficulty = g_stakeManager->GetStakeDifficulty(nullptr);
    BOOST_CHECK(newDifficulty != difficulty); // Difficulty should change with more stake
    
    // Clean up
    g_stakeManager->RemoveValidator(address1);
    g_stakeManager->RemoveValidator(address2);
}

BOOST_AUTO_TEST_CASE(wallet_staking_manager_basic)
{
    // Test CWalletStakingManager basic operations
    std::unique_ptr<CWallet> wallet = CreateSyncedWallet(*m_node.chain, m_args);
    
    CWalletStakingManager stakingManager(wallet.get());
    
    // Test enabling staking
    bool success = stakingManager.EnableStaking();
    BOOST_CHECK(success);
    BOOST_CHECK(stakingManager.IsStakingEnabled());
    
    // Test disabling staking
    success = stakingManager.DisableStaking();
    BOOST_CHECK(success);
    BOOST_CHECK(!stakingManager.IsStakingEnabled());
}

BOOST_AUTO_TEST_CASE(staking_info_calculation)
{
    // Test staking info calculation
    std::unique_ptr<CWallet> wallet = CreateSyncedWallet(*m_node.chain, m_args);
    CWalletStakingManager stakingManager(wallet.get());
    
    CStakingInfo info = stakingManager.GetStakingInfo();
    
    BOOST_CHECK_EQUAL(info.totalStake, 0);
    BOOST_CHECK_EQUAL(info.availableStake, 0);
    BOOST_CHECK_EQUAL(info.validatorCount, 0);
    BOOST_CHECK(!info.isStaking);
}

BOOST_AUTO_TEST_CASE(consensus_constants)
{
    // Test consensus constants
    BOOST_CHECK_GT(POS_MIN_STAKE_AMOUNT, 0);
    BOOST_CHECK_GT(POS_MIN_STAKE_AGE, 0);
    BOOST_CHECK_GT(POS_BLOCK_REWARD, 0);
    BOOST_CHECK_GT(POS_BLOCK_INTERVAL, 0);
    BOOST_CHECK_GT(POS_DIFFICULTY_ADJUSTMENT_INTERVAL, 0);
    
    // Test block type constants
    BOOST_CHECK_EQUAL(BLOCK_TYPE_POW, 0);
    BOOST_CHECK_EQUAL(BLOCK_TYPE_POS, 1);
    BOOST_CHECK_EQUAL(BLOCK_TYPE_COUNT, 2);
}

BOOST_AUTO_TEST_CASE(transaction_type_constants)
{
    // Test transaction type constants
    BOOST_CHECK_EQUAL(TX_TYPE_STANDARD, 0);
    BOOST_CHECK_EQUAL(TX_TYPE_TOKEN_CREATE, 1);
    BOOST_CHECK_EQUAL(TX_TYPE_TOKEN_TRANSFER, 2);
    BOOST_CHECK_EQUAL(TX_TYPE_NFT_MINT, 3);
    BOOST_CHECK_EQUAL(TX_TYPE_NFT_TRANSFER, 4);
    BOOST_CHECK_EQUAL(TX_TYPE_DEX_SWAP, 5);
    BOOST_CHECK_EQUAL(TX_TYPE_DEX_LIQUIDITY, 6);
    BOOST_CHECK_EQUAL(TX_TYPE_COUNT, 7);
}

BOOST_AUTO_TEST_CASE(stake_kernel_hash_consistency)
{
    // Test that stake kernel hash calculation is consistent
    CKey key;
    key.MakeNewKey(true);
    CTxDestination address = key.GetPubKey().GetID();
    CAmount amount = POS_MIN_STAKE_AMOUNT;
    int64_t stakeTime = GetTime();
    int64_t blockTime = GetTime() + 1;
    
    CStakeKernel kernel1(address, amount, stakeTime, blockTime);
    CStakeKernel kernel2(address, amount, stakeTime, blockTime);
    
    uint256 hash1 = kernel1.GetHash();
    uint256 hash2 = kernel2.GetHash();
    
    BOOST_CHECK_EQUAL(hash1, hash2); // Same inputs should produce same hash
    
    // Test with different inputs
    CStakeKernel kernel3(address, amount + 1, stakeTime, blockTime);
    uint256 hash3 = kernel3.GetHash();
    BOOST_CHECK_NE(hash1, hash3); // Different inputs should produce different hash
}

BOOST_AUTO_TEST_CASE(stake_manager_validator_selection)
{
    // Test validator selection logic
    CKey key1, key2, key3;
    key1.MakeNewKey(true);
    key2.MakeNewKey(true);
    key3.MakeNewKey(true);
    
    CTxDestination address1 = key1.GetPubKey().GetID();
    CTxDestination address2 = key2.GetPubKey().GetID();
    CTxDestination address3 = key3.GetPubKey().GetID();
    
    // Add validators with different amounts
    g_stakeManager->AddValidator(address1, POS_MIN_STAKE_AMOUNT, uint256::ONE, GetTime() - POS_MIN_STAKE_AGE - 1);
    g_stakeManager->AddValidator(address2, POS_MIN_STAKE_AMOUNT * 2, uint256::TWO, GetTime() - POS_MIN_STAKE_AGE - 1);
    g_stakeManager->AddValidator(address3, POS_MIN_STAKE_AMOUNT * 3, uint256::THREE, GetTime() - POS_MIN_STAKE_AGE - 1);
    
    // Get active validators
    std::vector<CStakeValidator> validators = g_stakeManager->GetActiveValidators();
    BOOST_CHECK_EQUAL(validators.size(), 3);
    
    // Clean up
    g_stakeManager->RemoveValidator(address1);
    g_stakeManager->RemoveValidator(address2);
    g_stakeManager->RemoveValidator(address3);
}

BOOST_AUTO_TEST_CASE(block_type_validation)
{
    // Test block type validation
    CBlock powBlock;
    powBlock.SetBlockType(BLOCK_TYPE_POW);
    BOOST_CHECK(powBlock.IsProofOfWork());
    BOOST_CHECK(!powBlock.IsProofOfStake());
    BOOST_CHECK_EQUAL(powBlock.GetBlockType(), BLOCK_TYPE_POW);
    
    CBlock posBlock;
    posBlock.SetBlockType(BLOCK_TYPE_POS);
    BOOST_CHECK(posBlock.IsProofOfStake());
    BOOST_CHECK(!posBlock.IsProofOfWork());
    BOOST_CHECK_EQUAL(posBlock.GetBlockType(), BLOCK_TYPE_POS);
}

BOOST_AUTO_TEST_SUITE_END()
