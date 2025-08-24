// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_CHAINPARAMS_H
#define SHAHCOIN_CHAINPARAMS_H

#include <memory>
#include <vector>
#include <string>
#include <stdint.h>
#include <array>
#include <optional>
#include <map>
#include "chainparamsbase.h"
#include "consensus/params.h"
#include "checkpoint_data.h"
#include "chain_tx_data.h"
#include "kernel/messagestartchars.h"
#include "uint256.h"
#include "common/args.h"
#include "util/chaintype.h"
#include "primitives/block.h"
#include "chain.h"
#include "chainparamsseeds.h"
#include "base58type.h"

struct CDNSSeedData {
    std::string name, host;
    CDNSSeedData(const std::string& strName, const std::string& strHost) : name(strName), host(strHost) {}
    CDNSSeedData(const std::string& strHost) : name(strHost), host(strHost) {} // For backward compatibility
};

class CBlockIndex;

// Type alias for AssumeutxoHash
using AssumeutxoHash = uint256;

// Simple option structs for network configuration
struct SigNetOptions {
    std::optional<std::vector<uint8_t>> challenge;
    std::optional<std::vector<std::string>> seeds;
};

struct RegTestOptions {
    bool fastprune{false};
    std::map<Consensus::DeploymentPos, int> activation_heights;
    std::map<Consensus::DeploymentPos, Consensus::BIP9Deployment> version_bits_parameters;
};

// Complete AssumeutxoData definition for mainnet-only (assumeutxo disabled)
struct AssumeutxoData {
    int height;
    AssumeutxoHash hash_serialized;
    unsigned int nChainTx;
    uint256 blockhash;
};

class CChainParams {
public:
    const Consensus::Params& GetConsensus() const { return consensus; }
    const std::vector<unsigned char>& Base58Prefix(Base58Type type) const { return base58Prefixes[type]; }
    int DefaultPort() const { return nDefaultPort; }
    int GetDefaultPort() const { return nDefaultPort; }
    const std::string& NetworkIDString() const { return strNetworkID; }
    std::string GetChainTypeString() const { return chainTypeString; }
    uint64_t AssumedBlockchainSize() const { return nAssumedBlockchainSize; }
    uint64_t AssumedChainStateSize() const { return nAssumedChainStateSize; }
    bool DefaultConsistencyChecks() const { return fDefaultConsistencyChecks; }
    const CCheckpointData& Checkpoints() const { return checkpointData; }
    const ChainTxData& TxData() const { return chainTxData; }
    
    // Message start characters for network protocol
    const MessageStartChars& MessageStart() const { return pchMessageStart; }
    
    // Genesis block
    const CBlock& GenesisBlock() const { return genesis; }
    
    // Network seeds
    const std::vector<CDNSSeedData>& DNSSeeds() const { return vSeeds; }
    const std::vector<SeedSpec6>& FixedSeeds() const { return vFixedSeeds; }
    
    // Bech32 human readable part
    const std::string& Bech32HRP() const { return bech32_hrp; }
    
    // Prune after height
    int PruneAfterHeight() const { return nPruneAfterHeight; }
    
    // Chain type
    ChainType GetChainType() const { return chainType; }
    
    // Test chain check
    bool IsTestChain() const { return chainType != ChainType::MAIN; }
    
    // Mockable chain check (disabled for mainnet-only)
    bool IsMockableChain() const { return false; }
    
    // Assumeutxo functionality (disabled for mainnet-only)
    std::optional<AssumeutxoData> AssumeutxoForBlockhash(const uint256& blockhash) const { return std::nullopt; }
    std::optional<AssumeutxoData> AssumeutxoForHeight(int height) const { return std::nullopt; }
    
    // Additional parameters
    bool RequireStandard() const { return fRequireStandard; }
    bool MineBlocksOnDemand() const { return fMineBlocksOnDemand; }

    // Static factory methods
    static std::unique_ptr<const CChainParams> Main();
    static std::unique_ptr<const CChainParams> TestNet();
    static std::unique_ptr<const CChainParams> SigNet(const SigNetOptions& options);
    static std::unique_ptr<const CChainParams> RegTest(const RegTestOptions& options);

protected:
    Consensus::Params consensus;
    std::vector<unsigned char> base58Prefixes[Base58Type::MAX_BASE58_TYPES];
    int nDefaultPort;
    std::string strNetworkID;
    std::string chainTypeString;
    uint64_t nAssumedBlockchainSize;
    uint64_t nAssumedChainStateSize;
    bool fDefaultConsistencyChecks;
    CCheckpointData checkpointData;
    ChainTxData chainTxData;
    
    // Message start characters
    MessageStartChars pchMessageStart;
    
    // Genesis block
    CBlock genesis;
    
    // Network seeds
    std::vector<CDNSSeedData> vSeeds;
    std::vector<SeedSpec6> vFixedSeeds;
    
    // Bech32 human readable part
    std::string bech32_hrp;
    
    // Prune after height
    int nPruneAfterHeight;
    
    // Chain type
    ChainType chainType;
    
    // Additional parameters
    bool fRequireStandard;
    bool fMineBlocksOnDemand;
    
    // Additional member variables for kernel compatibility
    ChainType m_chain_type;
    uint64_t m_assumed_blockchain_size;
    uint64_t m_assumed_chain_state_size;
    bool m_is_mockable_chain;
    std::map<uint256, AssumeutxoData> m_assumeutxo_data;
};

// Forward declarations
std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain);
const CChainParams& Params();
void SelectParams(const std::string& chain);

// Seed arrays for different networks
extern const uint8_t chainparams_seed_main[];
extern const uint8_t chainparams_seed_test[];

#endif // SHAHCOIN_CHAINPARAMS_H
