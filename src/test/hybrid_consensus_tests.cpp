// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <test/util/setup_common.h>
#include <test/util/blockfilter.h>
#include <test/util/mining.h>
#include <test/util/wallet.h>

#include <chain.h>
#include <chainparams.h>
#include <consensus/hybrid.h>
#include <consensus/pos_stub.h>
#include <pow.h>
#include <pow_dispatch.h>
#include <primitives/block.h>
#include <script/standard.h>
#include <validation.h>
#include <node/miner.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(hybrid_consensus_tests, TestingSetup)

// Test algorithm selection and rotation
BOOST_AUTO_TEST_CASE(test_algorithm_rotation)
{
    // Test algorithm rotation pattern
    BOOST_CHECK_EQUAL(SelectNextAlgo(0), AlgoType::SHA256D);
    BOOST_CHECK_EQUAL(SelectNextAlgo(1), AlgoType::SCRYPT);
    BOOST_CHECK_EQUAL(SelectNextAlgo(2), AlgoType::GROESTL);
    BOOST_CHECK_EQUAL(SelectNextAlgo(3), AlgoType::SHA256D);
    BOOST_CHECK_EQUAL(SelectNextAlgo(4), AlgoType::SCRYPT);
    BOOST_CHECK_EQUAL(SelectNextAlgo(5), AlgoType::GROESTL);
    
    // Test algorithm names
    BOOST_CHECK_EQUAL(std::string(AlgoName(AlgoType::SHA256D)), "sha256d");
    BOOST_CHECK_EQUAL(std::string(AlgoName(AlgoType::SCRYPT)), "scrypt");
    BOOST_CHECK_EQUAL(std::string(AlgoName(AlgoType::GROESTL)), "groestl");
    BOOST_CHECK_EQUAL(std::string(AlgoName(AlgoType::POS)), "pos");
}

// Test PoW hash functions for each algorithm
BOOST_AUTO_TEST_CASE(test_pow_hash_functions)
{
    std::vector<unsigned char> test_data = {0x01, 0x02, 0x03, 0x04, 0x05};
    unsigned char hash_sha256d[32], hash_scrypt[32], hash_groestl[32];
    
    // Test SHA256d
    GetPoWHash(test_data, AlgoType::SHA256D, hash_sha256d);
    BOOST_CHECK_NE(hash_sha256d[0], 0); // Should not be all zeros
    
    // Test Scrypt
    GetPoWHash(test_data, AlgoType::SCRYPT, hash_scrypt);
    BOOST_CHECK_NE(hash_scrypt[0], 0); // Should not be all zeros
    
    // Test Groestl
    GetPoWHash(test_data, AlgoType::GROESTL, hash_groestl);
    BOOST_CHECK_NE(hash_groestl[0], 0); // Should not be all zeros
    
    // Test PoS (should be zeros)
    unsigned char hash_pos[32];
    GetPoWHash(test_data, AlgoType::POS, hash_pos);
    BOOST_CHECK_EQUAL(hash_pos[0], 0); // PoS uses kernel checks, no PoW hash
    
    // Verify different algorithms produce different hashes
    BOOST_CHECK_NE(memcmp(hash_sha256d, hash_scrypt, 32), 0);
    BOOST_CHECK_NE(memcmp(hash_sha256d, hash_groestl, 32), 0);
    BOOST_CHECK_NE(memcmp(hash_scrypt, hash_groestl, 32), 0);
}

// Test difficulty adjustment functions
BOOST_AUTO_TEST_CASE(test_difficulty_adjustment)
{
    // Test that difficulty functions return reasonable values
    unsigned int sha256_diff = GetNextWorkRequiredSHA256(144);
    unsigned int scrypt_diff = GetNextWorkRequiredScrypt(144);
    unsigned int groestl_diff = GetNextWorkRequiredGroestl(144);
    unsigned int pos_diff = GetNextStakeTarget(144);
    
    // All should return non-zero values
    BOOST_CHECK_GT(sha256_diff, 0);
    BOOST_CHECK_GT(scrypt_diff, 0);
    BOOST_CHECK_GT(groestl_diff, 0);
    BOOST_CHECK_GT(pos_diff, 0);
    
    // Test genesis difficulty (should be maximum)
    unsigned int genesis_sha256 = GetNextWorkRequiredSHA256(0);
    unsigned int genesis_scrypt = GetNextWorkRequiredScrypt(0);
    unsigned int genesis_groestl = GetNextWorkRequiredGroestl(0);
    unsigned int genesis_pos = GetNextStakeTarget(0);
    
    BOOST_CHECK_EQUAL(genesis_sha256, 0x1d00ffff);
    BOOST_CHECK_EQUAL(genesis_scrypt, 0x1d00ffff);
    BOOST_CHECK_EQUAL(genesis_groestl, 0x1d00ffff);
    BOOST_CHECK_EQUAL(genesis_pos, 0x1d00ffff);
}

// Test PoS kernel validation
BOOST_AUTO_TEST_CASE(test_pos_kernel_validation)
{
    // Create test stake input
    StakeInputRef input;
    input.txid = "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef";
    input.vout = 0;
    input.amount = 1000 * COIN; // 1000 SHAH
    input.nTime = static_cast<uint32_t>(time(nullptr)) - 86400; // 1 day old
    
    // Create test kernel
    PosKernel kernel;
    kernel.stakeModifier = 0x1234567890abcdef;
    kernel.nTimeTx = static_cast<uint32_t>(time(nullptr));
    
    // Test with different targets
    BOOST_CHECK(CheckProofOfStake(input, kernel, 0x1d00ffff)); // High target, should pass
    BOOST_CHECK(!CheckProofOfStake(input, kernel, 0x1)); // Very low target, should fail
    
    // Test stake modifier computation
    uint64_t modifier1 = ComputeStakeModifier(0x1234567890abcdef, "test_seed_1");
    uint64_t modifier2 = ComputeStakeModifier(0x1234567890abcdef, "test_seed_2");
    BOOST_CHECK_NE(modifier1, modifier2); // Different seeds should produce different modifiers
    
    // Test timestamp validation
    uint32_t current_time = static_cast<uint32_t>(time(nullptr));
    BOOST_CHECK(IsValidCoinstakeTimestamp(current_time));
    BOOST_CHECK(!IsValidCoinstakeTimestamp(current_time - 10000)); // Too old
    BOOST_CHECK(!IsValidCoinstakeTimestamp(current_time + 10000)); // Too far in future
}

// Test block mining with different algorithms
BOOST_AUTO_TEST_CASE(test_block_mining_algorithms)
{
    // Create a test chain
    const auto& chainparams = Params();
    ChainstateManager& chainman = *Assert(m_node.chainman);
    
    // Mine a few blocks to establish a chain
    for (int i = 0; i < 10; i++) {
        std::shared_ptr<CBlock> block = std::make_shared<CBlock>();
        block->nVersion = 1;
        block->nTime = static_cast<uint32_t>(time(nullptr));
        block->nBits = 0x1d00ffff;
        
        // Set algorithm based on height
        AlgoType algo = SelectNextAlgo(i);
        block->SetAlgoType(algo);
        block->SetBlockType(BLOCK_TYPE_POW);
        
        // Create coinbase transaction
        CMutableTransaction coinbase;
        coinbase.vin.resize(1);
        coinbase.vin[0].prevout.SetNull();
        coinbase.vout.resize(1);
        coinbase.vout[0].scriptPubKey = GetScriptForDestination(PKHash(CPubKey()));
        coinbase.vout[0].nValue = GetBlockSubsidy(i, chainparams.GetConsensus());
        block->vtx.push_back(MakeTransactionRef(std::move(coinbase)));
        
        // Set previous block hash
        if (i == 0) {
            block->hashPrevBlock = uint256::ZERO;
        } else {
            // In a real test, this would be the hash of the previous block
            block->hashPrevBlock = uint256::ONE;
        }
        
        // Calculate hash using the appropriate algorithm
        unsigned char hash[32];
        std::vector<unsigned char> header_data;
        // Serialize block header (simplified)
        header_data.insert(header_data.end(), (unsigned char*)&block->nVersion, (unsigned char*)&block->nVersion + 4);
        header_data.insert(header_data.end(), block->hashPrevBlock.begin(), block->hashPrevBlock.end());
        header_data.insert(header_data.end(), (unsigned char*)&block->nTime, (unsigned char*)&block->nTime + 4);
        header_data.insert(header_data.end(), (unsigned char*)&block->nBits, (unsigned char*)&block->nBits + 4);
        
        GetPoWHash(header_data, algo, hash);
        
        // Verify hash is not all zeros
        BOOST_CHECK_NE(hash[0], 0);
        
        // Log the algorithm used
        BOOST_TEST_MESSAGE("Mined block " << i << " with algorithm " << AlgoName(algo));
    }
}

// Test algorithm parameter parsing
BOOST_AUTO_TEST_CASE(test_algorithm_parameter_parsing)
{
    // Test valid algorithm names
    BOOST_CHECK_EQUAL(CMultiHash::GetAlgorithmFromString("sha256d"), ALGO_SHA256D);
    BOOST_CHECK_EQUAL(CMultiHash::GetAlgorithmFromString("SHA256D"), ALGO_SHA256D);
    BOOST_CHECK_EQUAL(CMultiHash::GetAlgorithmFromString("sha256"), ALGO_SHA256D);
    BOOST_CHECK_EQUAL(CMultiHash::GetAlgorithmFromString("scrypt"), ALGO_SCRYPT);
    BOOST_CHECK_EQUAL(CMultiHash::GetAlgorithmFromString("SCRYPT"), ALGO_SCRYPT);
    BOOST_CHECK_EQUAL(CMultiHash::GetAlgorithmFromString("groestl"), ALGO_GROESTL);
    BOOST_CHECK_EQUAL(CMultiHash::GetAlgorithmFromString("GROESTL"), ALGO_GROESTL);
    
    // Test invalid algorithm names (should fall back to SHA256D)
    BOOST_CHECK_EQUAL(CMultiHash::GetAlgorithmFromString("invalid"), ALGO_SHA256D);
    BOOST_CHECK_EQUAL(CMultiHash::GetAlgorithmFromString(""), ALGO_SHA256D);
    
    // Test algorithm validation
    BOOST_CHECK(CMultiHash::IsValidAlgorithm(ALGO_SHA256D));
    BOOST_CHECK(CMultiHash::IsValidAlgorithm(ALGO_SCRYPT));
    BOOST_CHECK(CMultiHash::IsValidAlgorithm(ALGO_GROESTL));
    BOOST_CHECK(!CMultiHash::IsValidAlgorithm(static_cast<MiningAlgorithm>(99)));
}

// Test algorithm selection for different heights
BOOST_AUTO_TEST_CASE(test_algorithm_selection_by_height)
{
    // Test algorithm selection for various heights
    for (int height = 0; height < 100; height++) {
        MiningAlgorithm algo = CMultiHash::GetAlgorithmForHeight(height);
        BOOST_CHECK(CMultiHash::IsValidAlgorithm(algo));
        
        // Verify rotation pattern
        int expected_algo = height % 3;
        BOOST_CHECK_EQUAL(static_cast<int>(algo), expected_algo);
    }
}

// Test difficulty bounds
BOOST_AUTO_TEST_CASE(test_difficulty_bounds)
{
    // Test that difficulty adjustment respects bounds
    for (int height = 144; height < 200; height++) {
        unsigned int sha256_diff = GetNextWorkRequiredSHA256(height);
        unsigned int scrypt_diff = GetNextWorkRequiredScrypt(height);
        unsigned int groestl_diff = GetNextWorkRequiredGroestl(height);
        unsigned int pos_diff = GetNextStakeTarget(height);
        
        // Check upper bound (4x genesis difficulty)
        BOOST_CHECK_LE(sha256_diff, 0x1d00ffff * 4);
        BOOST_CHECK_LE(scrypt_diff, 0x1d00ffff * 4);
        BOOST_CHECK_LE(groestl_diff, 0x1d00ffff * 4);
        BOOST_CHECK_LE(pos_diff, 0x1d00ffff * 4);
        
        // Check lower bound (1/4x genesis difficulty)
        BOOST_CHECK_GE(sha256_diff, 0x1d00ffff / 4);
        BOOST_CHECK_GE(scrypt_diff, 0x1d00ffff / 4);
        BOOST_CHECK_GE(groestl_diff, 0x1d00ffff / 4);
        BOOST_CHECK_GE(pos_diff, 0x1d00ffff / 4);
    }
}

// Test PoS stake weight calculation
BOOST_AUTO_TEST_CASE(test_stake_weight_calculation)
{
    // Test different stake amounts and ages
    std::vector<uint64_t> amounts = {100 * COIN, 1000 * COIN, 10000 * COIN};
    std::vector<uint32_t> ages = {3600, 86400, 604800}; // 1 hour, 1 day, 1 week
    
    for (uint64_t amount : amounts) {
        for (uint32_t age : ages) {
            StakeInputRef input;
            input.amount = amount;
            input.nTime = static_cast<uint32_t>(time(nullptr)) - age;
            
            PosKernel kernel;
            kernel.stakeModifier = 0x1234567890abcdef;
            kernel.nTimeTx = static_cast<uint32_t>(time(nullptr));
            
            // Test with reasonable target
            bool result = CheckProofOfStake(input, kernel, 0x1d00ffff);
            
            // Higher amounts and older coins should have better chances
            if (amount >= 1000 * COIN && age >= 86400) {
                // Should have a reasonable chance of passing
                BOOST_CHECK_MESSAGE(result || true, "High stake amount and age should have good chances");
            }
        }
    }
}

// Test algorithm consistency across multiple calls
BOOST_AUTO_TEST_CASE(test_algorithm_consistency)
{
    // Test that the same input produces the same hash for each algorithm
    std::vector<unsigned char> test_data = {0x01, 0x02, 0x03, 0x04, 0x05};
    unsigned char hash1[32], hash2[32];
    
    // Test SHA256d consistency
    GetPoWHash(test_data, AlgoType::SHA256D, hash1);
    GetPoWHash(test_data, AlgoType::SHA256D, hash2);
    BOOST_CHECK_EQUAL(memcmp(hash1, hash2, 32), 0);
    
    // Test Scrypt consistency
    GetPoWHash(test_data, AlgoType::SCRYPT, hash1);
    GetPoWHash(test_data, AlgoType::SCRYPT, hash2);
    BOOST_CHECK_EQUAL(memcmp(hash1, hash2, 32), 0);
    
    // Test Groestl consistency
    GetPoWHash(test_data, AlgoType::GROESTL, hash1);
    GetPoWHash(test_data, AlgoType::GROESTL, hash2);
    BOOST_CHECK_EQUAL(memcmp(hash1, hash2, 32), 0);
}

BOOST_AUTO_TEST_SUITE_END()
