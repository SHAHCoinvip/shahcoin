# Shahcoin Smart Contract Creator (SHI-20/SHI-30)

## 🎯 Overview

The Smart Contract Creator module provides a comprehensive toolkit for building SHI-20 tokens and SHI-30 NFTs on the Shahcoin blockchain. This module includes a visual builder interface, pre-built templates, and deployment tools for creating verifiable smart contracts.

## ✨ Features

### 🏗️ Visual Contract Builder
- **Drag-and-Drop Interface**: Intuitive visual builder for contract creation
- **Real-time Preview**: Live preview of contract code and metadata
- **Template Library**: Pre-built templates for common use cases
- **Custom Logic Builder**: Advanced logic configuration without coding

### 📋 SHI-20 Token Templates
- **Standard Token**: Basic fungible token with transfer functionality
- **Mintable Token**: Token with minting capabilities
- **Burnable Token**: Token with burning functionality
- **Pausable Token**: Token with pause/unpause controls
- **Capped Token**: Token with maximum supply limit
- **Timelock Token**: Token with time-based restrictions

### 🖼️ SHI-30 NFT Templates
- **Standard NFT**: Basic non-fungible token
- **Mintable NFT**: NFT with minting capabilities
- **Royalty NFT**: NFT with royalty distribution
- **Soulbound NFT**: Non-transferable NFT
- **Dynamic NFT**: NFT with changeable metadata
- **Batch NFT**: NFT with batch minting

### 🔧 Advanced Features
- **Custom Logic**: Add custom functions and modifiers
- **Access Control**: Role-based access control
- **Upgradeable Contracts**: UUPS upgradeable contracts
- **Gas Optimization**: Automatic gas optimization
- **Security Analysis**: Built-in security checks

## 📦 Installation

```bash
npm install @shahcoin/contract-creator
```

## 🚀 Quick Start

### JavaScript SDK

```javascript
import { ContractCreator, SHI20Builder, SHI30Builder } from '@shahcoin/contract-creator';

// Initialize contract creator
const creator = new ContractCreator({
  network: 'shahcoin',
  compiler: 'solc',
  optimizer: true
});

// Create SHI-20 token
const shi20Builder = new SHI20Builder(creator);
const tokenContract = await shi20Builder.createToken({
  name: 'My Token',
  symbol: 'MTK',
  decimals: 18,
  totalSupply: '1000000000000000000000000', // 1M tokens
  features: ['mintable', 'burnable', 'pausable'],
  customLogic: {
    maxSupply: '10000000000000000000000000', // 10M max
    mintRole: 'MINTER_ROLE',
    pauseRole: 'PAUSER_ROLE'
  }
});

// Create SHI-30 NFT
const shi30Builder = new SHI30Builder(creator);
const nftContract = await shi30Builder.createNFT({
  name: 'My NFT Collection',
  symbol: 'MNFT',
  baseURI: 'https://api.example.com/metadata/',
  features: ['mintable', 'royalty', 'batch'],
  customLogic: {
    maxSupply: 10000,
    royaltyPercentage: 500, // 5%
    batchSize: 10
  }
});

// Deploy contracts
const tokenAddress = await creator.deploy(tokenContract);
const nftAddress = await creator.deploy(nftContract);

console.log('Token deployed at:', tokenAddress);
console.log('NFT deployed at:', nftAddress);
```

### Python SDK

```python
from shahcoin_contract_creator import ContractCreator, SHI20Builder, SHI30Builder

# Initialize contract creator
creator = ContractCreator(
    network='shahcoin',
    compiler='solc',
    optimizer=True
)

# Create SHI-20 token
shi20_builder = SHI20Builder(creator)
token_contract = await shi20_builder.create_token(
    name='My Token',
    symbol='MTK',
    decimals=18,
    total_supply='1000000000000000000000000',
    features=['mintable', 'burnable', 'pausable'],
    custom_logic={
        'max_supply': '10000000000000000000000000',
        'mint_role': 'MINTER_ROLE',
        'pause_role': 'PAUSER_ROLE'
    }
)

# Create SHI-30 NFT
shi30_builder = SHI30Builder(creator)
nft_contract = await shi30_builder.create_nft(
    name='My NFT Collection',
    symbol='MNFT',
    base_uri='https://api.example.com/metadata/',
    features=['mintable', 'royalty', 'batch'],
    custom_logic={
        'max_supply': 10000,
        'royalty_percentage': 500,
        'batch_size': 10
    }
)

# Deploy contracts
token_address = await creator.deploy(token_contract)
nft_address = await creator.deploy(nft_contract)

print(f'Token deployed at: {token_address}')
print(f'NFT deployed at: {nft_address}')
```

### C++ SDK

```cpp
#include <shahcoin/contract_creator.hpp>

// Initialize contract creator
ContractCreator creator({
    {"network", "shahcoin"},
    {"compiler", "solc"},
    {"optimizer", true}
});

// Create SHI-20 token
SHI20Builder shi20Builder(creator);
auto tokenContract = shi20Builder.createToken({
    {"name", "My Token"},
    {"symbol", "MTK"},
    {"decimals", 18},
    {"totalSupply", "1000000000000000000000000"},
    {"features", {"mintable", "burnable", "pausable"}},
    {"customLogic", {
        {"maxSupply", "10000000000000000000000000"},
        {"mintRole", "MINTER_ROLE"},
        {"pauseRole", "PAUSER_ROLE"}
    }}
});

// Create SHI-30 NFT
SHI30Builder shi30Builder(creator);
auto nftContract = shi30Builder.createNFT({
    {"name", "My NFT Collection"},
    {"symbol", "MNFT"},
    {"baseURI", "https://api.example.com/metadata/"},
    {"features", {"mintable", "royalty", "batch"}},
    {"customLogic", {
        {"maxSupply", 10000},
        {"royaltyPercentage", 500},
        {"batchSize", 10}
    }}
});

// Deploy contracts
auto tokenAddress = creator.deploy(tokenContract);
auto nftAddress = creator.deploy(nftContract);

std::cout << "Token deployed at: " << tokenAddress << std::endl;
std::cout << "NFT deployed at: " << nftAddress << std::endl;
```

## 🔧 Configuration

### Contract Creator Configuration

```javascript
const config = {
  // Network settings
  network: 'shahcoin',
  rpcUrl: 'https://rpc.shah.vip',
  chainId: 8369,
  
  // Compiler settings
  compiler: 'solc',
  compilerVersion: '0.8.19',
  optimizer: true,
  optimizerRuns: 200,
  
  // Deployment settings
  gasLimit: 5000000,
  gasPrice: 'auto',
  confirmations: 3,
  
  // Security settings
  securityChecks: true,
  auditMode: false,
  
  // Storage settings
  saveArtifacts: true,
  artifactsPath: './contracts/artifacts',
};
```

### SHI-20 Configuration

```javascript
const shi20Config = {
  // Basic token settings
  name: 'My Token',
  symbol: 'MTK',
  decimals: 18,
  totalSupply: '1000000000000000000000000',
  
  // Features to enable
  features: [
    'mintable',      // Minting functionality
    'burnable',      // Burning functionality
    'pausable',      // Pause/unpause
    'capped',        // Maximum supply
    'timelock',      // Time-based restrictions
    'blacklist',     // Blacklist functionality
    'whitelist'      // Whitelist functionality
  ],
  
  // Custom logic
  customLogic: {
    maxSupply: '10000000000000000000000000',
    mintRole: 'MINTER_ROLE',
    burnRole: 'BURNER_ROLE',
    pauseRole: 'PAUSER_ROLE',
    timelockDuration: 86400, // 24 hours
    blacklistRole: 'BLACKLISTER_ROLE',
    whitelistRole: 'WHITELISTER_ROLE'
  },
  
  // Access control
  accessControl: {
    defaultAdminRole: 'DEFAULT_ADMIN_ROLE',
    minterRole: 'MINTER_ROLE',
    burnerRole: 'BURNER_ROLE',
    pauserRole: 'PAUSER_ROLE'
  }
};
```

### SHI-30 Configuration

```javascript
const shi30Config = {
  // Basic NFT settings
  name: 'My NFT Collection',
  symbol: 'MNFT',
  baseURI: 'https://api.example.com/metadata/',
  
  // Features to enable
  features: [
    'mintable',      // Minting functionality
    'royalty',       // Royalty distribution
    'soulbound',     // Non-transferable
    'dynamic',       // Changeable metadata
    'batch',         // Batch minting
    'lazy',          // Lazy minting
    'reveal'         // Reveal functionality
  ],
  
  // Custom logic
  customLogic: {
    maxSupply: 10000,
    royaltyPercentage: 500, // 5%
    batchSize: 10,
    revealDelay: 86400, // 24 hours
    metadataURI: 'https://api.example.com/metadata/'
  },
  
  // Access control
  accessControl: {
    defaultAdminRole: 'DEFAULT_ADMIN_ROLE',
    minterRole: 'MINTER_ROLE',
    royaltyRole: 'ROYALTY_ROLE'
  }
};
```

## 📊 API Reference

### Contract Creator API

#### `createToken(config)`
Create a SHI-20 token contract.

```javascript
const token = await creator.createToken({
  name: 'My Token',
  symbol: 'MTK',
  decimals: 18,
  totalSupply: '1000000000000000000000000',
  features: ['mintable', 'burnable']
});
```

#### `createNFT(config)`
Create a SHI-30 NFT contract.

```javascript
const nft = await creator.createNFT({
  name: 'My NFT Collection',
  symbol: 'MNFT',
  baseURI: 'https://api.example.com/metadata/',
  features: ['mintable', 'royalty']
});
```

#### `deploy(contract)`
Deploy a contract to the blockchain.

```javascript
const address = await creator.deploy(contract);
console.log('Contract deployed at:', address);
```

#### `verify(contract, address)`
Verify a deployed contract on the blockchain explorer.

```javascript
const verified = await creator.verify(contract, address);
console.log('Contract verified:', verified);
```

### SHI-20 Builder API

#### `addFeature(feature)`
Add a feature to the token.

```javascript
shi20Builder.addFeature('mintable');
shi20Builder.addFeature('burnable');
```

#### `setCustomLogic(logic)`
Set custom logic for the token.

```javascript
shi20Builder.setCustomLogic({
  maxSupply: '10000000000000000000000000',
  mintRole: 'MINTER_ROLE'
});
```

#### `generateCode()`
Generate the contract source code.

```javascript
const sourceCode = shi20Builder.generateCode();
console.log('Generated source code:', sourceCode);
```

### SHI-30 Builder API

#### `addFeature(feature)`
Add a feature to the NFT.

```javascript
shi30Builder.addFeature('mintable');
shi30Builder.addFeature('royalty');
```

#### `setCustomLogic(logic)`
Set custom logic for the NFT.

```javascript
shi30Builder.setCustomLogic({
  maxSupply: 10000,
  royaltyPercentage: 500
});
```

#### `generateCode()`
Generate the contract source code.

```javascript
const sourceCode = shi30Builder.generateCode();
console.log('Generated source code:', sourceCode);
```

## 🔌 Integration Examples

### Web Application Integration

```javascript
// React component for contract creation
import React, { useState } from 'react';
import { ContractCreator, SHI20Builder } from '@shahcoin/contract-creator';

function ContractCreatorApp() {
  const [contract, setContract] = useState(null);
  const [loading, setLoading] = useState(false);

  const createToken = async (config) => {
    setLoading(true);
    
    try {
      const creator = new ContractCreator();
      const builder = new SHI20Builder(creator);
      
      const tokenContract = await builder.createToken(config);
      setContract(tokenContract);
      
      // Deploy contract
      const address = await creator.deploy(tokenContract);
      console.log('Token deployed at:', address);
      
    } catch (error) {
      console.error('Error creating token:', error);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div>
      <h2>Create SHI-20 Token</h2>
      <button 
        onClick={() => createToken({
          name: 'My Token',
          symbol: 'MTK',
          decimals: 18,
          totalSupply: '1000000000000000000000000',
          features: ['mintable', 'burnable']
        })}
        disabled={loading}
      >
        {loading ? 'Creating...' : 'Create Token'}
      </button>
      
      {contract && (
        <div>
          <h3>Generated Contract</h3>
          <pre>{contract.sourceCode}</pre>
        </div>
      )}
    </div>
  );
}
```

### Smart Contract Integration

```solidity
// Solidity contract using generated SHI-20 token
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/access/AccessControl.sol";

contract MyToken is ERC20, AccessControl {
    bytes32 public constant MINTER_ROLE = keccak256("MINTER_ROLE");
    bytes32 public constant BURNER_ROLE = keccak256("BURNER_ROLE");
    
    uint256 public maxSupply;
    
    constructor(
        string memory name,
        string memory symbol,
        uint256 initialSupply,
        uint256 _maxSupply
    ) ERC20(name, symbol) {
        _mint(msg.sender, initialSupply);
        maxSupply = _maxSupply;
        
        _grantRole(DEFAULT_ADMIN_ROLE, msg.sender);
        _grantRole(MINTER_ROLE, msg.sender);
        _grantRole(BURNER_ROLE, msg.sender);
    }
    
    function mint(address to, uint256 amount) public onlyRole(MINTER_ROLE) {
        require(totalSupply() + amount <= maxSupply, "Exceeds max supply");
        _mint(to, amount);
    }
    
    function burn(uint256 amount) public onlyRole(BURNER_ROLE) {
        _burn(msg.sender, amount);
    }
}
```

### Qt Wallet Integration

```cpp
// C++ integration for Qt Wallet
#include <shahcoin/contract_creator.hpp>

class ShahcoinQtWallet {
private:
    ContractCreator m_contractCreator;
    SHI20Builder m_shi20Builder;
    SHI30Builder m_shi30Builder;
    
public:
    ShahcoinQtWallet() {
        m_contractCreator = ContractCreator({
            {"network", "shahcoin"},
            {"compiler", "solc"},
            {"optimizer", true}
        });
        
        m_shi20Builder = SHI20Builder(m_contractCreator);
        m_shi30Builder = SHI30Builder(m_contractCreator);
    }
    
    void createToken(const QString& name, const QString& symbol, 
                    int decimals, const QString& totalSupply) {
        auto config = {
            {"name", name.toStdString()},
            {"symbol", symbol.toStdString()},
            {"decimals", decimals},
            {"totalSupply", totalSupply.toStdString()},
            {"features", {"mintable", "burnable"}}
        };
        
        auto contract = m_shi20Builder.createToken(config);
        auto address = m_contractCreator.deploy(contract);
        
        emit tokenCreated(QString::fromStdString(address));
    }
    
    void createNFT(const QString& name, const QString& symbol, 
                   const QString& baseURI) {
        auto config = {
            {"name", name.toStdString()},
            {"symbol", symbol.toStdString()},
            {"baseURI", baseURI.toStdString()},
            {"features", {"mintable", "royalty"}}
        };
        
        auto contract = m_shi30Builder.createNFT(config);
        auto address = m_contractCreator.deploy(contract);
        
        emit nftCreated(QString::fromStdString(address));
    }
    
signals:
    void tokenCreated(const QString& address);
    void nftCreated(const QString& address);
};
```

## 🔒 Security Features

### Contract Security
- **Security Analysis**: Built-in security checks for common vulnerabilities
- **Access Control**: Role-based access control for all functions
- **Reentrancy Protection**: Protection against reentrancy attacks
- **Overflow Protection**: Protection against integer overflow

### Deployment Security
- **Gas Optimization**: Automatic gas optimization for cost efficiency
- **Constructor Validation**: Validation of constructor parameters
- **Deployment Verification**: Automatic verification after deployment

## 🚀 Performance Optimization

### Compilation Optimization
- **Incremental Compilation**: Only recompile changed contracts
- **Parallel Compilation**: Compile multiple contracts in parallel
- **Caching**: Cache compilation results for faster builds

### Deployment Optimization
- **Gas Estimation**: Accurate gas estimation for deployment
- **Batch Deployment**: Deploy multiple contracts in a single transaction
- **Deployment Verification**: Automatic verification after deployment

## 📚 Documentation

- **API Reference**: Complete API documentation
- **Integration Guides**: Step-by-step integration tutorials
- **Examples**: Code examples for common use cases
- **Troubleshooting**: Common issues and solutions

## 🤝 Support

- **GitHub Issues**: Report bugs and request features
- **Discord Community**: Get help from the community
- **Documentation**: Comprehensive documentation
- **Email Support**: Direct support for enterprise users

---

**Built with ❤️ by the SHAHCOIN Core Team**
