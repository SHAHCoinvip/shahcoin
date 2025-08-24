# Shahcoin Staking Developer Documentation

## Overview

This document provides technical details for developers working with Shahcoin's Proof of Stake (PoS) implementation. It covers the architecture, APIs, and integration points for the hybrid PoW/PoS consensus system.

## Architecture

### Core Components

1. **CStakeManager** (`src/stake/stake.h/cpp`)
   - Manages validator registration and stake tracking
   - Handles stake difficulty calculation
   - Provides validator selection logic

2. **CWalletStakingManager** (`src/wallet/staking.h/cpp`)
   - Wallet-side staking operations
   - Stake management and validation
   - Integration with wallet functionality

3. **CPoSMiner** (`src/mining/pos_miner.h/cpp`)
   - PoS block creation and submission
   - Mining loop and block validation
   - Network integration

4. **Validation Integration** (`src/validation.cpp`)
   - PoS block validation
   - Transaction validation for staking
   - Consensus rule enforcement

### Data Structures

#### CStakeValidator
```cpp
struct CStakeValidator {
    CTxDestination address;    // Validator address
    CAmount amount;           // Stake amount
    uint256 txHash;           // Stake transaction hash
    int64_t stakeTime;        // Stake creation time
};
```

#### CStakeKernel
```cpp
struct CStakeKernel {
    CTxDestination address;    // Validator address
    CAmount amount;           // Stake amount
    int64_t stakeTime;        // Stake creation time
    int64_t blockTime;        // Block creation time
    
    uint256 GetHash() const;  // Calculate kernel hash
};
```

#### CStakingInfo
```cpp
struct CStakingInfo {
    bool isStaking;           // Whether currently staking
    CAmount totalStake;       // Total staked amount
    CAmount availableStake;   // Available for staking
    size_t validatorCount;    // Number of validators
    int64_t expectedTime;     // Expected time to next block
};
```

## Consensus Rules

### PoS Block Requirements

1. **Minimum Stake**: 333 SHAH (POS_MIN_STAKE_AMOUNT)
2. **Minimum Age**: 24 hours (POS_MIN_STAKE_AGE)
3. **Block Type**: Must be BLOCK_TYPE_POS
4. **Stake Proof**: Valid stake kernel hash
5. **Timing**: Must be within acceptable time window

### Hybrid Consensus

- **Even blocks**: Proof of Work (BLOCK_TYPE_POW)
- **Odd blocks**: Proof of Stake (BLOCK_TYPE_POS)
- **Alternation**: Automatic based on block height

### Block Validation

```cpp
bool ValidatePoSBlock(const CBlock& block, int nHeight) {
    // Check block type
    if (block.GetBlockType() != BLOCK_TYPE_POS) return false;
    
    // Validate stake proof
    if (!ValidateStakeProof(block)) return false;
    
    // Check timing
    if (!ValidateBlockTiming(block, nHeight)) return false;
    
    // Verify difficulty
    if (!ValidateBlockDifficulty(block, nHeight)) return false;
    
    return true;
}
```

## APIs and Interfaces

### RPC Commands

#### Staking Commands
```cpp
// Enable/disable staking
bool enablestaking();
bool disablestaking();

// Stake management
bool addstake(const std::string& address, CAmount amount);
bool removestake(const std::string& address);
bool updatestake(const std::string& address, CAmount newAmount);

// Information queries
UniValue getstakinginfo();
UniValue getstakes();
UniValue getstakevalidators();
```

#### Token Commands
```cpp
// Token creation and management
UniValue createtoken(const std::string& name, const std::string& symbol, 
                     uint8_t decimals, uint64_t totalSupply, const std::string& address);
UniValue transfertoken(const std::string& tokenId, const std::string& fromAddress,
                       const std::string& toAddress, uint64_t amount);
UniValue gettokeninfo(const std::string& tokenId);
UniValue listtokens();
```

#### NFT Commands
```cpp
// NFT creation and management
UniValue mintnft(const std::string& name, const std::string& description,
                 const std::string& imageUri, const std::string& attributes,
                 const std::string& toAddress);
UniValue transfernft(const std::string& nftId, const std::string& fromAddress,
                     const std::string& toAddress);
UniValue getnftinfo(const std::string& nftId);
UniValue listnfts();
```

#### DEX Commands
```cpp
// DEX operations
UniValue createtradingpair(const std::string& tokenA, const std::string& tokenB,
                           const std::string& symbol);
UniValue addliquidity(const std::string& pairId, uint64_t amountA, uint64_t amountB,
                      const std::string& providerAddress);
UniValue swaptokens(const std::string& pairId, bool isBuy, uint64_t amountIn,
                    uint64_t amountOutMin, const std::string& traderAddress);
UniValue listtradingpairs();
```

### C++ APIs

#### Stake Manager
```cpp
class CStakeManager {
public:
    // Validator management
    bool AddValidator(const CTxDestination& address, CAmount amount, 
                     const uint256& txHash, int64_t stakeTime);
    bool RemoveValidator(const CTxDestination& address);
    bool UpdateValidatorStake(const CTxDestination& address, CAmount newAmount);
    
    // Queries
    CStakeValidator GetValidator(const CTxDestination& address) const;
    std::vector<CStakeValidator> GetActiveValidators() const;
    CAmount GetTotalStake() const;
    
    // Difficulty calculation
    uint256 GetStakeDifficulty(const CBlockIndex* pindexPrev) const;
    uint256 CalculateStakeKernelHash(const CStakeKernel& kernel) const;
};
```

#### Wallet Staking Manager
```cpp
class CWalletStakingManager {
public:
    // Staking control
    bool EnableStaking();
    bool DisableStaking();
    bool IsStakingEnabled() const;
    
    // Stake management
    bool AddStake(const CTxDestination& address, CAmount amount);
    bool RemoveStake(const CTxDestination& address);
    bool UpdateStake(const CTxDestination& address, CAmount newAmount);
    
    // Block creation
    bool CreateStakeBlock(CBlock& block, const CStakeValidator& validator);
    bool SignStakeBlock(CBlock& block, const CKey& key);
    
    // Information
    CStakingInfo GetStakingInfo() const;
    std::vector<CStakeValidator> GetValidStakes() const;
    CStakeValidator SelectStakeForBlock() const;
};
```

#### PoS Miner
```cpp
class CPoSMiner {
public:
    // Initialization
    bool Initialize(CWallet* wallet);
    
    // Mining control
    void StartMining();
    void StopMining();
    bool IsMining() const;
    
    // Statistics
    MiningStats GetStats() const;
};
```

## Integration Points

### Wallet Integration

#### Adding Staking to Wallet
```cpp
// In wallet.cpp
#include <wallet/staking.h>

class CWallet {
private:
    std::unique_ptr<CWalletStakingManager> m_stakingManager;
    
public:
    // Staking methods
    bool EnableStaking();
    bool DisableStaking();
    bool AddStake(const CTxDestination& address, CAmount amount);
    CStakingInfo GetStakingInfo() const;
};
```

#### GUI Integration
```cpp
// In qt/walletmodel.cpp
class WalletModel {
public:
    // Staking signals
    void stakingStatusChanged(bool enabled);
    void stakingRewardReceived(CAmount amount);
    void stakeAdded(const QString& address, CAmount amount);
};
```

### Network Integration

#### Block Submission
```cpp
// In mining/pos_miner.cpp
bool CPoSMiner::SubmitBlock(const CBlock& block) {
    // Validate block
    if (!CPoSBlockValidator::ValidateBlock(block, nHeight)) {
        return false;
    }
    
    // Submit to network
    // This integrates with the existing network submission system
    return SubmitBlockToNetwork(block);
}
```

#### P2P Protocol
```cpp
// In net_processing.cpp
bool ProcessNewBlock(const CBlock& block, bool fForceProcessing) {
    // Handle PoS blocks
    if (block.IsProofOfStake()) {
        return ProcessPoSBlock(block, fForceProcessing);
    }
    
    // Handle PoW blocks
    return ProcessPoWBlock(block, fForceProcessing);
}
```

## Configuration

### Consensus Parameters

```cpp
// In consensus/consensus.h
static const int64_t POS_MIN_STAKE_AMOUNT = 333 * 100000000; // 333 SHAH
static const int64_t POS_MIN_STAKE_AGE = 24 * 60 * 60;       // 24 hours
static const int64_t POS_BLOCK_REWARD = 50 * 100000000;      // 50 SHAH
static const int POS_BLOCK_INTERVAL = 2;                     // Every 2 blocks
static const int POS_DIFFICULTY_ADJUSTMENT_INTERVAL = 2016;  // Every 2016 blocks
```

### Runtime Configuration

```cpp
// In init.cpp
bool AppInitMain() {
    // Initialize staking system
    if (!InitializePoSMining(pwallet)) {
        return false;
    }
    
    // Register RPC commands
    RegisterStakingRPCCommands(tableRPC);
    RegisterTokenRPCCommands(tableRPC);
    RegisterNFTRPCCommands(tableRPC);
    RegisterDEXRPCCommands(tableRPC);
    
    return true;
}
```

## Testing

### Unit Tests

```cpp
// In test/staking_tests.cpp
BOOST_AUTO_TEST_CASE(stake_validator_creation) {
    CKey key;
    key.MakeNewKey(true);
    CTxDestination address = key.GetPubKey().GetID();
    CAmount amount = POS_MIN_STAKE_AMOUNT;
    
    CStakeValidator validator(address, amount, uint256::ONE, GetTime());
    
    BOOST_CHECK_EQUAL(validator.address, address);
    BOOST_CHECK_EQUAL(validator.amount, amount);
}
```

### Functional Tests

```python
# In test/functional/feature_staking.py
def test_basic_staking(self):
    # Generate coins
    self.nodes[0].generate(101)
    
    # Enable staking
    result = self.nodes[0].enablestaking()
    assert result
    
    # Add stake
    address = self.nodes[0].getnewaddress()
    result = self.nodes[0].addstake(address, 1000)
    assert result
```

## Security Considerations

### Stake Security

1. **Private Key Protection**: Stakes require private key access
2. **UTXO Validation**: Ensure stakes are not double-spent
3. **Age Verification**: Prevent stake age manipulation
4. **Kernel Validation**: Verify stake kernel integrity

### Network Security

1. **Sybil Resistance**: Minimum stake requirements
2. **Nothing at Stake**: Stake age requirements
3. **Long Range Attacks**: Block finality mechanisms
4. **Economic Attacks**: Stake amount validation

### Implementation Security

1. **Input Validation**: Validate all RPC inputs
2. **Memory Safety**: Use RAII and smart pointers
3. **Thread Safety**: Proper synchronization
4. **Error Handling**: Graceful failure modes

## Performance Considerations

### Optimization Strategies

1. **Caching**: Cache frequently accessed stake data
2. **Indexing**: Index stakes by address and amount
3. **Batch Operations**: Process multiple stakes together
4. **Lazy Loading**: Load stake data on demand

### Resource Management

1. **Memory Usage**: Monitor stake manager memory
2. **CPU Usage**: Optimize stake selection algorithms
3. **Network I/O**: Minimize network overhead
4. **Storage**: Efficient stake data storage

## Future Enhancements

### Planned Features

1. **Delegated Staking**: Allow stake delegation
2. **Stake Pools**: Pooled staking for small holders
3. **Advanced Rewards**: Dynamic reward calculation
4. **Governance**: Stake-based governance voting

### Scalability Improvements

1. **Sharding**: Stake sharding for large networks
2. **Lightweight Clients**: SPV staking support
3. **Cross-Chain**: Interoperability with other chains
4. **Layer 2**: Off-chain staking solutions

## Troubleshooting

### Common Issues

1. **Stake Not Eligible**: Check minimum requirements
2. **Block Creation Fails**: Verify stake validation
3. **Network Sync Issues**: Check consensus rules
4. **Performance Problems**: Monitor resource usage

### Debug Tools

```cpp
// Debug staking state
void DebugStakingState() {
    LogPrint(BCLog::STAKING, "Total stake: %s\n", 
             FormatMoney(g_stakeManager->GetTotalStake()));
    LogPrint(BCLog::STAKING, "Active validators: %d\n", 
             g_stakeManager->GetActiveValidators().size());
}
```

## Conclusion

This documentation provides a comprehensive overview of Shahcoin's staking implementation. For additional details, refer to the source code and test files. The system is designed to be secure, efficient, and extensible for future enhancements.

For questions or contributions, please refer to the project's GitHub repository and community channels.
