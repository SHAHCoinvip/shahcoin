#ifndef SHAHCOIN_CHAINPARAMS_H
#define SHAHCOIN_CHAINPARAMS_H

#include <memory>
#include <vector>
#include <string>
#include <stdint.h>
#include <array>
#include <optional>
#include "chainparamsbase.h"
#include "consensus/params.h"
#include "checkpoint_data.h"
#include "chain_tx_data.h"
#include "kernel/messagestartchars.h"
#include "uint256.h"
#include "common/args.h"
#include "util/chaintype.h"
#include "./primitives/block.h"
#include "chain.h"
#include "chainparamsseeds.h"

struct CDNSSeedData {
    std::string name, host;
    CDNSSeedData(const std::string& strName, const std::string& strHost) : name(strName), host(strHost) {}
};

class CBlockIndex;
class CBlock;

// Base58 type enum
enum Base58Type {
    PUBKEY_ADDRESS,
    SCRIPT_ADDRESS,
    SECRET_KEY,
    EXT_PUBLIC_KEY,
    EXT_SECRET_KEY,
    MAX_BASE58_TYPES
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
    
    // Additional parameters
    bool RequireStandard() const { return fRequireStandard; }
    bool MineBlocksOnDemand() const { return fMineBlocksOnDemand; }

protected:
    Consensus::Params consensus;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
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
};

// Forward declarations
std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain);
const CChainParams& Params();

#endif // SHAHCOIN_CHAINPARAMS_H
