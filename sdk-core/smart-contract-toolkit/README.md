# Shahcoin Smart Contract Toolkit

[![Built on SHAHCOIN Core](https://img.shields.io/badge/Built%20on-Shahcoin%20Core-blue.svg)](https://shah.vip/dev)
[![Smart Contracts](https://img.shields.io/badge/Smart%20Contracts-Solidity%20Compatible-green.svg)](https://docs.soliditylang.org/)

A visual/no-code toolkit for building custom smart contracts on Shahcoin blockchain. Think of it as a Shahcoin-native version of OpenZeppelin Wizard.

## 🎯 Features

### 🔧 Prebuilt Templates
- **SHI-20 Tokens**: Standard fungible tokens with customizable parameters (Shahcoin Standard)
- **NFT Collections**: ERC-721 compatible non-fungible tokens
- **Staking Contracts**: DeFi staking with rewards and lock periods
- **DAO Governance**: Decentralized autonomous organization contracts
- **Multi-signature Wallets**: Multi-sig wallet implementations

### 🎨 Custom Logic Builder
- **Max Supply Control**: Set maximum token supply limits
- **Decimal Configuration**: Customize token decimals (0-18)
- **Mint Permissions**: Control who can mint new tokens
- **Royalty System**: Implement creator royalties for NFTs
- **Time Locks**: Add time-based restrictions and vesting
- **Access Control**: Role-based permissions and ownership

### 📤 Export Options
- **Direct Deployment**: Deploy directly to Shahcoin blockchain via ShahFactory
- **Remix Compatible**: Export Solidity code for Remix IDE
- **Hardhat/Truffle**: Generate project files for popular frameworks
- **Documentation**: Auto-generate contract documentation

## 🚀 Quick Start

### 1. Choose Template
```javascript
// Select from prebuilt templates
const templates = {
  erc20: "Standard Token",
  nft: "NFT Collection", 
  staking: "Staking Contract",
  dao: "DAO Governance",
  multisig: "Multi-signature Wallet"
};
```

### 2. Configure Parameters
```javascript
// Token configuration example
const tokenConfig = {
  name: "My Token",
  symbol: "MTK",
  decimals: 18,
  maxSupply: 1000000,
  initialSupply: 100000,
  mintable: true,
  burnable: true,
  pausable: true,
  accessControl: "Ownable"
};
```

### 3. Generate Contract
```javascript
// Generate contract code
const contractCode = await generateContract(template, config);

// Deploy directly to Shahcoin
const deployedAddress = await deployToShahcoin(contractCode);
```

## 📋 Template Examples

### SHI-20 Token Template
```solidity
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/security/Pausable.sol";

contract MyToken is ERC20, Ownable, Pausable {
    uint256 public maxSupply;
    
    constructor(
        string memory name,
        string memory symbol,
        uint256 initialSupply,
        uint256 _maxSupply
    ) ERC20(name, symbol) {
        _mint(msg.sender, initialSupply * 10**decimals());
        maxSupply = _maxSupply * 10**decimals();
    }
    
    function mint(address to, uint256 amount) public onlyOwner {
        require(totalSupply() + amount <= maxSupply, "Exceeds max supply");
        _mint(to, amount);
    }
    
    function pause() public onlyOwner {
        _pause();
    }
    
    function unpause() public onlyOwner {
        _unpause();
    }
    
    function _beforeTokenTransfer(
        address from,
        address to,
        uint256 amount
    ) internal override whenNotPaused {
        super._beforeTokenTransfer(from, to, amount);
    }
}
```

### NFT Collection Template
```solidity
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "@openzeppelin/contracts/token/ERC721/ERC721.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/utils/Counters.sol";

contract MyNFTCollection is ERC721, Ownable {
    using Counters for Counters.Counter;
    
    Counters.Counter private _tokenIds;
    uint256 public maxSupply;
    uint256 public mintPrice;
    string public baseURI;
    
    constructor(
        string memory name,
        string memory symbol,
        uint256 _maxSupply,
        uint256 _mintPrice
    ) ERC721(name, symbol) {
        maxSupply = _maxSupply;
        mintPrice = _mintPrice;
    }
    
    function mint() public payable {
        require(msg.value >= mintPrice, "Insufficient payment");
        require(_tokenIds.current() < maxSupply, "Max supply reached");
        
        _tokenIds.increment();
        _mint(msg.sender, _tokenIds.current());
    }
    
    function setBaseURI(string memory _baseURI) public onlyOwner {
        baseURI = _baseURI;
    }
    
    function _baseURI() internal view override returns (string memory) {
        return baseURI;
    }
}
```

### Staking Contract Template
```solidity
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "@openzeppelin/contracts/token/ERC20/IERC20.sol";
import "@openzeppelin/contracts/security/ReentrancyGuard.sol";
import "@openzeppelin/contracts/access/Ownable.sol";

contract StakingContract is ReentrancyGuard, Ownable {
    IERC20 public stakingToken;
    IERC20 public rewardToken;
    
    uint256 public rewardRate;
    uint256 public lastUpdateTime;
    uint256 public rewardPerTokenStored;
    
    mapping(address => uint256) public userRewardPerTokenPaid;
    mapping(address => uint256) public rewards;
    mapping(address => uint256) public balanceOf;
    
    uint256 public totalSupply;
    
    constructor(
        address _stakingToken,
        address _rewardToken,
        uint256 _rewardRate
    ) {
        stakingToken = IERC20(_stakingToken);
        rewardToken = IERC20(_rewardToken);
        rewardRate = _rewardRate;
    }
    
    function stake(uint256 amount) external nonReentrant {
        totalSupply += amount;
        balanceOf[msg.sender] += amount;
        stakingToken.transferFrom(msg.sender, address(this), amount);
        updateReward(msg.sender);
    }
    
    function withdraw(uint256 amount) external nonReentrant {
        totalSupply -= amount;
        balanceOf[msg.sender] -= amount;
        stakingToken.transfer(msg.sender, amount);
        updateReward(msg.sender);
    }
    
    function getReward() external nonReentrant {
        updateReward(msg.sender);
        uint256 reward = rewards[msg.sender];
        if (reward > 0) {
            rewards[msg.sender] = 0;
            rewardToken.transfer(msg.sender, reward);
        }
    }
    
    function updateReward(address account) internal {
        rewardPerTokenStored = rewardPerToken();
        lastUpdateTime = block.timestamp;
        
        if (account != address(0)) {
            rewards[account] = earned(account);
            userRewardPerTokenPaid[account] = rewardPerTokenStored;
        }
    }
    
    function rewardPerToken() public view returns (uint256) {
        if (totalSupply == 0) {
            return rewardPerTokenStored;
        }
        return rewardPerTokenStored + (
            ((block.timestamp - lastUpdateTime) * rewardRate * 1e18) / totalSupply
        );
    }
    
    function earned(address account) public view returns (uint256) {
        return (
            balanceOf[account] * 
            (rewardPerToken() - userRewardPerTokenPaid[account])
        ) / 1e18 + rewards[account];
    }
}
```

## 🎨 UI Components

### Contract Builder Interface
```javascript
// React component for contract builder
import React, { useState } from 'react';

const ContractBuilder = () => {
  const [template, setTemplate] = useState('erc20');
  const [config, setConfig] = useState({});
  const [preview, setPreview] = useState('');

  const generatePreview = async () => {
    const code = await generateContract(template, config);
    setPreview(code);
  };

  return (
    <div className="contract-builder">
      <div className="template-selector">
        <h3>Choose Template</h3>
        <select value={template} onChange={(e) => setTemplate(e.target.value)}>
          <option value="shi20">SHI-20 Token</option>
          <option value="nft">NFT Collection</option>
          <option value="staking">Staking Contract</option>
          <option value="dao">DAO Governance</option>
        </select>
      </div>
      
      <div className="config-panel">
        <h3>Configure Parameters</h3>
        <ContractConfig 
          template={template} 
          config={config} 
          onChange={setConfig} 
        />
      </div>
      
      <div className="preview-panel">
        <h3>Contract Preview</h3>
        <button onClick={generatePreview}>Generate Preview</button>
        <pre>{preview}</pre>
      </div>
      
      <div className="export-panel">
        <h3>Export Options</h3>
        <button onClick={() => deployToShahcoin(preview)}>
          Deploy to Shahcoin
        </button>
        <button onClick={() => exportForRemix(preview)}>
          Export for Remix
        </button>
      </div>
    </div>
  );
};
```

### Validation System
```javascript
// Contract validation
const validateContract = (template, config) => {
  const errors = [];
  
  switch (template) {
    case 'erc20':
      if (!config.name || config.name.length < 1) {
        errors.push('Token name is required');
      }
      if (!config.symbol || config.symbol.length < 1) {
        errors.push('Token symbol is required');
      }
      if (config.decimals < 0 || config.decimals > 18) {
        errors.push('Decimals must be between 0 and 18');
      }
      if (config.maxSupply <= 0) {
        errors.push('Max supply must be greater than 0');
      }
      break;
      
    case 'nft':
      if (!config.name || config.name.length < 1) {
        errors.push('Collection name is required');
      }
      if (config.maxSupply <= 0) {
        errors.push('Max supply must be greater than 0');
      }
      break;
      
    case 'staking':
      if (!config.stakingToken) {
        errors.push('Staking token address is required');
      }
      if (!config.rewardToken) {
        errors.push('Reward token address is required');
      }
      if (config.rewardRate <= 0) {
        errors.push('Reward rate must be greater than 0');
      }
      break;
  }
  
  return errors;
};
```

## 🔗 Integration with Shahcoin

### ShahFactory Integration
```javascript
// Deploy to Shahcoin via ShahFactory
const deployToShahcoin = async (contractCode, config) => {
  try {
    // Connect to Shahcoin network
    const provider = new ethers.providers.JsonRpcProvider(
      'https://rpc.shah.vip'
    );
    
    // Get ShahFactory contract
    const shahFactory = new ethers.Contract(
      SHAHFACTORY_ADDRESS,
      SHAHFACTORY_ABI,
      provider
    );
    
    // Deploy contract
    const tx = await shahFactory.deployContract(
      contractCode,
      config.constructorArgs,
      { gasLimit: 5000000 }
    );
    
    const receipt = await tx.wait();
    return receipt.contractAddress;
    
  } catch (error) {
    console.error('Deployment failed:', error);
    throw error;
  }
};
```

### Price API Integration
```javascript
// Get current SHAH price for gas estimation
const getShahPrice = async () => {
  const response = await fetch('https://api.shah.vip/price');
  const data = await response.json();
  return data.shah.usd;
};

// Estimate deployment cost
const estimateDeploymentCost = async (contractCode) => {
  const shahPrice = await getShahPrice();
  const gasEstimate = await estimateGas(contractCode);
  const gasPrice = await getGasPrice();
  
  const costInWei = gasEstimate.mul(gasPrice);
  const costInShah = ethers.utils.formatEther(costInWei);
  const costInUSD = costInShah * shahPrice;
  
  return {
    gasEstimate: gasEstimate.toString(),
    costInShah,
    costInUSD
  };
};
```

## 📚 Documentation

### Template Reference
- **SHI-20**: Standard fungible token implementation (Shahcoin Standard)
- **NFT**: Non-fungible token collection with metadata
- **Staking**: DeFi staking with rewards and lock periods
- **DAO**: Decentralized governance with voting mechanisms
- **Multi-sig**: Multi-signature wallet for secure transactions

### Custom Logic Examples
- **Access Control**: Role-based permissions and ownership
- **Time Locks**: Vesting schedules and time-based restrictions
- **Royalties**: Creator royalties for NFT sales
- **Mint Permissions**: Controlled token minting
- **Burn Mechanisms**: Token burning and deflationary features

### Best Practices
- **Security**: Follow security best practices for smart contracts
- **Gas Optimization**: Optimize contracts for efficient gas usage
- **Testing**: Thoroughly test contracts before deployment
- **Documentation**: Document contract functions and parameters
- **Auditing**: Consider professional audits for production contracts

## 🛠 Development

### Local Development
```bash
# Install dependencies
npm install

# Start development server
npm run dev

# Build for production
npm run build

# Run tests
npm test
```

### Contributing
1. Fork the repository
2. Create a feature branch
3. Add new templates or features
4. Write tests for new functionality
5. Submit a pull request

## 📞 Support

- 📖 **Documentation**: [https://shah.vip/dev](https://shah.vip/dev)
- 🐛 **Issues**: [GitHub Issues](https://github.com/SHAHCoinvip/shahcoin-core/issues)
- 💬 **Discord**: [Shahcoin Community](https://discord.gg/shahcoin)

## 📄 License

MIT License - see [LICENSE](./LICENSE) for details.

---

**Built with love by SHAHCOIN Core Contributors** ❤️
