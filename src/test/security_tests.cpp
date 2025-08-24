// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <test/util/setup_common.h>
#include <policy/honeypot_filter.h>
#include <consensus/finality.h>
#include <stake/cold_staking.h>
#include <primitives/transaction.h>
#include <primitives/block.h>
#include <script/standard.h>
#include <key.h>
#include <logging.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(security_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(honeypot_filter_test)
{
    // Test honeypot filtering initialization
    BOOST_CHECK(HoneypotUtils::InitializeHoneypotFiltering());
    BOOST_CHECK(HoneypotUtils::IsHoneypotFilteringEnabled());
    
    // Test normal transaction (should not be flagged)
    CMutableTransaction normalTx;
    normalTx.vin.resize(1);
    normalTx.vout.resize(1);
    normalTx.vout[0].nValue = 1000 * COIN;
    normalTx.vout[0].scriptPubKey = GetScriptForDestination(CKeyID(uint160()));
    
    CTransaction tx(normalTx);
    BOOST_CHECK(!HoneypotUtils::ShouldRejectTransaction(tx));
    
    // Test transaction with excessive OP_RETURN outputs
    CMutableTransaction suspiciousTx;
    suspiciousTx.vin.resize(1);
    suspiciousTx.vout.resize(15); // More than default limit of 10
    
    for (size_t i = 0; i < 15; i++) {
        suspiciousTx.vout[i].nValue = 0;
        suspiciousTx.vout[i].scriptPubKey = CScript() << OP_RETURN << std::vector<unsigned char>(50, 0x01);
    }
    
    CTransaction suspiciousTransaction(suspiciousTx);
    // This should be flagged as suspicious
    BOOST_CHECK(g_honeypotFilter->CheckTransaction(suspiciousTransaction).isSuspicious);
    
    // Test statistics
    auto stats = g_honeypotFilter->GetStats();
    BOOST_CHECK_GT(stats.totalTransactions, 0);
    
    // Cleanup
    HoneypotUtils::ShutdownHoneypotFiltering();
    BOOST_CHECK(!HoneypotUtils::IsHoneypotFilteringEnabled());
}

BOOST_AUTO_TEST_CASE(finality_rules_test)
{
    // Test finality system initialization
    BOOST_CHECK(FinalityUtils::InitializeFinalitySystem());
    BOOST_CHECK(FinalityUtils::IsFinalityEnabled());
    
    // Create a mock block index for testing
    CBlockIndex mockIndex;
    mockIndex.nHeight = 1000;
    mockIndex.nTime = GetTime();
    
    // Test finality status calculation
    FinalityStatus status = g_finalityManager->GetBlockFinalityStatus(&mockIndex);
    BOOST_CHECK(status == FinalityStatus::PENDING); // New block should be pending
    
    // Test finality depth calculation
    int depth = FinalityUtils::GetFinalityDepth(&mockIndex);
    BOOST_CHECK_GE(depth, 0);
    
    // Test statistics
    auto stats = g_finalityManager->GetStats();
    BOOST_CHECK_GE(stats.totalBlocks, 0);
    
    // Cleanup
    FinalityUtils::ShutdownFinalitySystem();
    BOOST_CHECK(!FinalityUtils::IsFinalityEnabled());
}

BOOST_AUTO_TEST_CASE(cold_staking_test)
{
    // Test cold staking system initialization
    BOOST_CHECK(ColdStakingUtils::InitializeColdStaking());
    BOOST_CHECK(ColdStakingUtils::IsColdStakingEnabled());
    
    // Generate test addresses
    CKey coldKey, hotKey;
    coldKey.MakeNewKey(true);
    hotKey.MakeNewKey(true);
    
    CTxDestination coldAddress = coldKey.GetPubKey().GetID();
    CTxDestination hotAddress = hotKey.GetPubKey().GetID();
    
    // Test delegation creation
    CAmount delegationAmount = 1000 * COIN;
    BOOST_CHECK(g_coldStakingManager->CreateDelegation(coldAddress, hotAddress, delegationAmount));
    
    // Test delegation retrieval
    auto delegation = g_coldStakingManager->GetDelegation(coldAddress, hotAddress);
    BOOST_CHECK_EQUAL(delegation.delegatedAmount, delegationAmount);
    BOOST_CHECK(delegation.status == ColdStakingStatus::ACTIVE);
    
    // Test delegation queries
    auto coldDelegations = g_coldStakingManager->GetDelegationsForColdWallet(coldAddress);
    BOOST_CHECK_EQUAL(coldDelegations.size(), 1);
    
    auto hotDelegations = g_coldStakingManager->GetDelegationsForHotWallet(hotAddress);
    BOOST_CHECK_EQUAL(hotDelegations.size(), 1);
    
    // Test staking capability
    BOOST_CHECK(g_coldStakingManager->CanStakeWithDelegation(hotAddress, coldAddress));
    
    // Test delegation revocation
    BOOST_CHECK(g_coldStakingManager->RevokeDelegation(coldAddress, hotAddress));
    
    // Verify revocation
    delegation = g_coldStakingManager->GetDelegation(coldAddress, hotAddress);
    BOOST_CHECK(delegation.status == ColdStakingStatus::REVOKED);
    
    // Test statistics
    auto stats = g_coldStakingManager->GetStats();
    BOOST_CHECK_GT(stats.totalDelegations, 0);
    
    // Cleanup
    ColdStakingUtils::ShutdownColdStaking();
    BOOST_CHECK(!ColdStakingUtils::IsColdStakingEnabled());
}

BOOST_AUTO_TEST_CASE(security_integration_test)
{
    // Test that all security systems work together
    BOOST_CHECK(HoneypotUtils::InitializeHoneypotFiltering());
    BOOST_CHECK(FinalityUtils::InitializeFinalitySystem());
    BOOST_CHECK(ColdStakingUtils::InitializeColdStaking());
    
    // Verify all systems are enabled
    BOOST_CHECK(HoneypotUtils::IsHoneypotFilteringEnabled());
    BOOST_CHECK(FinalityUtils::IsFinalityEnabled());
    BOOST_CHECK(ColdStakingUtils::IsColdStakingEnabled());
    
    // Test utility functions
    BOOST_CHECK_EQUAL(HoneypotUtils::GetFilterTypeName(HoneypotFilterType::EXCESSIVE_OP_RETURN), "EXCESSIVE_OP_RETURN");
    BOOST_CHECK_EQUAL(FinalityUtils::GetFinalityStatusName(FinalityStatus::PENDING), "PENDING");
    BOOST_CHECK_EQUAL(ColdStakingUtils::GetColdStakingStatusName(ColdStakingStatus::ACTIVE), "ACTIVE");
    
    // Cleanup all systems
    HoneypotUtils::ShutdownHoneypotFiltering();
    FinalityUtils::ShutdownFinalitySystem();
    ColdStakingUtils::ShutdownColdStaking();
    
    BOOST_CHECK(!HoneypotUtils::IsHoneypotFilteringEnabled());
    BOOST_CHECK(!FinalityUtils::IsFinalityEnabled());
    BOOST_CHECK(!ColdStakingUtils::IsColdStakingEnabled());
}

BOOST_AUTO_TEST_SUITE_END()
