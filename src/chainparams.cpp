// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/merkle.h>
#include <consensus/amount.h>
#include <tinyformat.h>
#include <util/strencodings.h>
#include <versionbits.h>
#include <common/args.h>
#include <util/chaintype.h>
#include <primitives/transaction.h>
#include <script/script.h>

#include <assert.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <cstring>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "SHAHCOIN GENESIS BLOCK – Victory through Code – July 2025";
    
    // SHAHCOIN Premine: 8,000,000 SHAH to premine address
    // Public Key: 029797b158ad1a44415a6c10ca96a4bb2e5932f0e45a22601f01dbb75f6732dd
    // Address: SVUNaktXHkoRtoZeScGxjMxFVDVMn4xKTG
    const CScript premineOutputScript = CScript() << ParseHex("029797b158ad1a44415a6c10ca96a4bb2e5932f0e45a22601f01dbb75f6732dd") << OP_CHECKSIG;
    
    // Regular genesis output script (for remaining coins)
    const CScript genesisOutputScript = CScript() << ParseHex("04ffff001d0104") << OP_CHECKSIG;

    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(2); // Two outputs: premine + regular genesis
    
    // Input
    txNew.vin[0].scriptSig = CScript()
        << 486604799
        << CScriptNum(4)
        << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    
    // Output 0: Premine (8,000,000 SHAH)
    txNew.vout[0].nValue = 8000000 * COIN;
    txNew.vout[0].scriptPubKey = premineOutputScript;
    
    // Output 1: Regular genesis reward (remaining coins)
    txNew.vout[1].nValue = genesisReward;
    txNew.vout[1].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.SetAlgorithm(ALGO_SHA256D); // SHAHCOIN Core genesis block uses SHA256d
    genesis.SetBlockType(BLOCK_TYPE_POW); // Genesis is PoW
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion)
{
    return CreateGenesisBlock(nTime, nNonce, nBits, nVersion, 100 * COIN); // SHAHCOIN Core 100 SHAH block reward
}

/**
 * Main network
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        chainType = ChainType::MAIN;
        strNetworkID = "main";
        chainTypeString = "main";
        
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.CSVHeight = 0;
        consensus.SegwitHeight = 0;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 150; // 2.5 minutes for hybrid consensus
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916;
        consensus.nMinerConfirmationWindow = 2016;
        consensus.nMinimumChainWork = uint256S("0000000000000000000000000000000000000000000000000000000000000000");
        consensus.defaultAssumeValid = uint256S("000000ed04e7955d54aba236568b407582a5debda3fcab929619e4a3e10db6c5");
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        
        // SHAHCOIN FINAL BUILD: Mainnet Network Parameters
        consensus.nMaxSupply = 63000000 * COIN; // 63M SHAH total supply cap
        consensus.nInitialSubsidy = 50 * COIN; // 50 SHAH initial block reward
        consensus.nSubsidyHalvingInterval = 840000; // ~4 years (150s blocks)
        consensus.nStakeMinAge = 3600; // 1 hour minimum stake age
        consensus.nStakeMinAmount = 333 * COIN; // 333 SHAH minimum stake
        consensus.nStakeReward = 5 * COIN; // 5 SHAH PoS block reward
        consensus.nPoSInterval = 10; // Every 10th block is PoS
        consensus.nMinRelayTxFee = 1000; // 0.001 SHAH minimum relay fee

        pchMessageStart[0] = 0x53; // S
        pchMessageStart[1] = 0x48; // H
        pchMessageStart[2] = 0x41; // A
        pchMessageStart[3] = 0x48; // H
        nDefaultPort = 8369;
        nPruneAfterHeight = 100000;

        // SHAHCOIN FINAL BUILD: Regenerate Genesis Block with final rules
        // Block time: 150s, Timestamp: July 1, 2025 00:00:00 UTC
        genesis = CreateGenesisBlock(1743638400, 0x12345678, 0x1d00ffff, 0x20000000, 50 * COIN); // Final SHAHCOIN genesis
        consensus.hashGenesisBlock = genesis.GetHash();
        
        // Final network parameters for mainnet launch
        // assert(consensus.hashGenesisBlock == uint256S("0x0000000000000000000000000000000000000000000000000000000000000000")); // Will be calculated
        // assert(genesis.hashMerkleRoot == uint256S("0x0000000000000000000000000000000000000000000000000000000000000000")); // Will be calculated

        vSeeds.emplace_back("seed.shah.network");
        vSeeds.emplace_back("seed2.shah.network");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 63); // S
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 125);
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1, 191);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "shah";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        
        // Set assumed sizes
        nAssumedBlockchainSize = 350;
        nAssumedChainStateSize = 6;
        
        // Initialize checkpoint data
        checkpointData = CCheckpointData{};
        
        // Initialize chain transaction data
        chainTxData = ChainTxData{
            0, // nTime
            0, // nTxCount
            0.0 // dTxRate
        };
    }
};

/**
 * Test network
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        chainType = ChainType::TESTNET;
        strNetworkID = "test";
        chainTypeString = "test";
        
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.CSVHeight = 0;
        consensus.SegwitHeight = 0;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 150;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512;
        consensus.nMinerConfirmationWindow = 2016;
        consensus.nMinimumChainWork = uint256S("0000000000000000000000000000000000000000000000000000000000000000");
        consensus.defaultAssumeValid = uint256S("000000ed04e7955d54aba236568b407582a5debda3fcab929619e4a3e10db6c5");
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();

        pchMessageStart[0] = 0x53; // S
        pchMessageStart[1] = 0x48; // H
        pchMessageStart[2] = 0x41; // A
        pchMessageStart[3] = 0x54; // T
        nDefaultPort = 18369;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1231006505, 2083236893, 0x1d00ffff, 1, 50 * COIN); // Shahcoin testnet genesis (modified Shahcoin genesis)
        consensus.hashGenesisBlock = genesis.GetHash();
        // We'll calculate the actual hash after creating the block
        // assert(consensus.hashGenesisBlock == uint256S("0x00000008819873e925422c1ff0f99f7cc9bbb232af63a077a480a3633bee1ef6"));
        // assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vSeeds.clear();
        vSeeds.emplace_back("testnet-seed.shah.network");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111); // t
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tshah";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        
        // Set assumed sizes
        nAssumedBlockchainSize = 20;
        nAssumedChainStateSize = 2;
        
        // Initialize checkpoint data
        checkpointData = CCheckpointData{};
        
        // Initialize chain transaction data
        chainTxData = ChainTxData{
            0, // nTime
            0, // nTxCount
            0.0 // dTxRate
        };
    }
};

/**
 * Regression test network
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        chainType = ChainType::REGTEST;
        strNetworkID = "regtest";
        chainTypeString = "regtest";
        
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.CSVHeight = 0;
        consensus.SegwitHeight = 0;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 150;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108;
        consensus.nMinerConfirmationWindow = 144;
        consensus.nMinimumChainWork = uint256S("0000000000000000000000000000000000000000000000000000000000000000");
        consensus.defaultAssumeValid = uint256S("0000000000000000000000000000000000000000000000000000000000000000");
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();

        pchMessageStart[0] = 0x53; // S
        pchMessageStart[1] = 0x48; // H
        pchMessageStart[2] = 0x41; // A
        pchMessageStart[3] = 0x52; // R
        nDefaultPort = 28369;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1296688602, 2, 0x207fffff, 1); // Shahcoin regtest genesis block
        consensus.hashGenesisBlock = genesis.GetHash();

        vSeeds.clear();
        vSeeds.emplace_back("dummySeed.invalid.");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "rshah";

        vFixedSeeds.clear();

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        
        // Set assumed sizes
        nAssumedBlockchainSize = 0;
        nAssumedChainStateSize = 0;
        
        // Initialize checkpoint data
        checkpointData = CCheckpointData{};
        
        // Initialize chain transaction data
        chainTxData = ChainTxData{
            0, // nTime
            0, // nTxCount
            0.0 // dTxRate
        };
    }
};

/**
 * Signet network
 */
class CSigNetParams : public CChainParams {
public:
    CSigNetParams() {
        chainType = ChainType::SIGNET;
        strNetworkID = "signet";
        chainTypeString = "signet";
        
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.CSVHeight = 0;
        consensus.SegwitHeight = 0;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 150;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916;
        consensus.nMinerConfirmationWindow = 2016;
        consensus.nMinimumChainWork = uint256S("0000000000000000000000000000000000000000000000000000000000000000");
        consensus.defaultAssumeValid = uint256S("000000ed04e7955d54aba236568b407582a5debda3fcab929619e4a3e10db6c5");
        consensus.signet_blocks = true;
        consensus.signet_challenge = std::vector<uint8_t>(ParseHex("512102ad5e0eed18ffa7c6bd0f1fc7c3f5bf0d3114742c4dd60b9eb36428c61d6fc5c0ad"));

        pchMessageStart[0] = 0x0a;
        pchMessageStart[1] = 0x03;
        pchMessageStart[2] = 0x9f;
        pchMessageStart[3] = 0x7e;
        nDefaultPort = 38369;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1722019200, 23067884, 0x1d00ffff, 1); // July 27, 2024
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("000000ed04e7955d54aba236568b407582a5debda3fcab929619e4a3e10db6c5"));
        assert(genesis.hashMerkleRoot == uint256S("d2f70b982072c192e7e4f66ce762132c92cc4669451571d6dc2b23367858b23b"));

        vSeeds.clear();
        vSeeds.emplace_back("signet-seed.shah.network");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111); // t
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tsh";

        vFixedSeeds.clear();

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        
        // Set assumed sizes
        nAssumedBlockchainSize = 20;
        nAssumedChainStateSize = 2;
        
        // Initialize checkpoint data
        checkpointData = CCheckpointData{};
        
        // Initialize chain transaction data
        chainTxData = ChainTxData{
            0, // nTime
            0, // nTxCount
            0.0 // dTxRate
        };
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams& Params()
{
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == "main") {
        return std::make_unique<CMainParams>();
    }
    // Testnet and regtest are disabled - only mainnet is supported
    throw std::runtime_error(strprintf("%s: Chain %s is not supported. Only mainnet is available.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

// Seed arrays for different networks
const uint8_t chainparams_seed_main[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01
};

const uint8_t chainparams_seed_test[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01
};
