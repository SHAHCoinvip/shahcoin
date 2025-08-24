# Shahcoin Testnet Faucet & Sandbox

[![Built on SHAHCOIN Core](https://img.shields.io/badge/Built%20on-Shahcoin%20Core-blue.svg)](https://shah.vip/dev)
[![Testnet](https://img.shields.io/badge/Testnet-Active-green.svg)](https://testnet.shah.vip)

A testnet faucet and sandbox environment for Shahcoin developers to get free testnet SHAH coins and test smart contracts in a safe environment.

## 🎯 Features

### 🚰 Testnet Faucet
- **Free Testnet SHAH**: Get free testnet coins for development
- **Wallet Address Input**: Enter your wallet address to receive coins
- **Rate Limiting**: Prevent abuse with daily limits and cooldowns
- **Multiple Networks**: Support for testnet and regtest networks
- **Transaction Tracking**: Monitor faucet transactions and balances

### 🏖️ Sandbox Environment
- **Safe Testing**: Test smart contracts without real funds
- **Simulated Blockchain**: Full blockchain simulation for testing
- **Token Creation**: Create and test tokens in sandbox mode
- **NFT Testing**: Mint and trade NFTs in the sandbox
- **ShahSwap Simulation**: Test DEX functionality with simulated liquidity

### 🔒 Security Features
- **Captcha Protection**: Prevent automated abuse
- **IP Rate Limiting**: Limit requests per IP address
- **Wallet Verification**: Verify wallet addresses before sending
- **Transaction Monitoring**: Monitor for suspicious activity
- **Admin Controls**: Admin panel for faucet management

## 🚀 Quick Start

### 1. Get Testnet SHAH
```javascript
// Request testnet SHAH from faucet
const faucet = new ShahcoinFaucet({
  network: 'testnet',
  apiKey: 'your-api-key'
});

// Request coins
const result = await faucet.requestCoins({
  walletAddress: '0x1234567890abcdef...',
  amount: 1000, // SHAH
  captchaToken: 'captcha-token'
});

console.log('Transaction:', result.txHash);
console.log('Balance:', result.newBalance);
```

### 2. Use Sandbox Mode
```javascript
// Initialize sandbox environment
const sandbox = new ShahcoinSandbox({
  network: 'sandbox',
  initialBalance: 10000 // SHAH
});

// Create test token in sandbox
const token = await sandbox.createToken({
  name: 'Test Token',
  symbol: 'TEST',
  decimals: 18,
  totalSupply: 1000000
});

console.log('Token created:', token.address);
```

### 3. Test Smart Contracts
```javascript
// Deploy contract to sandbox
const contract = await sandbox.deployContract({
  contractCode: contractSource,
  constructorArgs: ['My Token', 'MTK', 1000000]
});

// Test contract functions
const balance = await contract.balanceOf(walletAddress);
const transfer = await contract.transfer(recipient, 1000);

console.log('Balance:', balance);
console.log('Transfer successful:', transfer);
```

## 🚰 Faucet API

### Request Coins
```javascript
// Request testnet SHAH
const requestCoins = async (params) => {
  const response = await fetch('/api/faucet/request', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${apiKey}`
    },
    body: JSON.stringify({
      walletAddress: params.walletAddress,
      amount: params.amount || 1000,
      network: params.network || 'testnet',
      captchaToken: params.captchaToken
    })
  });
  
  return response.json();
};

// Example usage
const result = await requestCoins({
  walletAddress: '0x1234567890abcdef...',
  amount: 1000,
  network: 'testnet',
  captchaToken: 'captcha-token'
});
```

### Check Faucet Status
```javascript
// Check faucet balance and limits
const getFaucetStatus = async () => {
  const response = await fetch('/api/faucet/status', {
    headers: {
      'Authorization': `Bearer ${apiKey}`
    }
  });
  
  return response.json();
};

// Example response
const status = {
  faucetBalance: 50000, // SHAH
  dailyLimit: 10000, // SHAH per day
  requestsToday: 45,
  remainingToday: 9555,
  cooldownPeriod: 3600, // seconds
  isActive: true
};
```

### Get Transaction History
```javascript
// Get faucet transaction history
const getTransactionHistory = async (params) => {
  const response = await fetch('/api/faucet/history', {
    headers: {
      'Authorization': `Bearer ${apiKey}`
    }
  });
  
  return response.json();
};

// Example response
const history = [
  {
    txHash: '0x123...',
    walletAddress: '0xabc...',
    amount: 1000,
    timestamp: '2024-01-15T10:30:00Z',
    status: 'confirmed'
  }
];
```

## 🏖️ Sandbox API

### Initialize Sandbox
```javascript
// Create new sandbox environment
const createSandbox = async (config) => {
  const response = await fetch('/api/sandbox/create', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${apiKey}`
    },
    body: JSON.stringify({
      initialBalance: config.initialBalance || 10000,
      network: config.network || 'sandbox',
      features: config.features || ['tokens', 'nfts', 'dex']
    })
  });
  
  return response.json();
};

// Example usage
const sandbox = await createSandbox({
  initialBalance: 10000,
  network: 'sandbox',
  features: ['tokens', 'nfts', 'dex', 'staking']
});
```

### Deploy Smart Contract
```javascript
// Deploy contract to sandbox
const deployContract = async (params) => {
  const response = await fetch('/api/sandbox/deploy', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${apiKey}`
    },
    body: JSON.stringify({
      sandboxId: params.sandboxId,
      contractCode: params.contractCode,
      constructorArgs: params.constructorArgs,
      gasLimit: params.gasLimit || 5000000
    })
  });
  
  return response.json();
};

// Example usage
const contract = await deployContract({
  sandboxId: 'sandbox-123',
  contractCode: tokenContractSource,
  constructorArgs: ['My Token', 'MTK', 1000000]
});
```

### Test Contract Functions
```javascript
// Call contract function in sandbox
const callContract = async (params) => {
  const response = await fetch('/api/sandbox/call', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${apiKey}`
    },
    body: JSON.stringify({
      sandboxId: params.sandboxId,
      contractAddress: params.contractAddress,
      functionName: params.functionName,
      functionArgs: params.functionArgs,
      fromAddress: params.fromAddress
    })
  });
  
  return response.json();
};

// Example usage
const result = await callContract({
  sandboxId: 'sandbox-123',
  contractAddress: '0xcontract...',
  functionName: 'transfer',
  functionArgs: ['0xrecipient...', 1000],
  fromAddress: '0xsender...'
});
```

## 🎨 UI Components

### Faucet Interface
```javascript
// React component for faucet interface
import React, { useState } from 'react';

const FaucetInterface = () => {
  const [walletAddress, setWalletAddress] = useState('');
  const [amount, setAmount] = useState(1000);
  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState(null);

  const requestCoins = async () => {
    setLoading(true);
    try {
      const response = await fetch('/api/faucet/request', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          walletAddress,
          amount,
          network: 'testnet'
        })
      });
      
      const data = await response.json();
      setResult(data);
    } catch (error) {
      console.error('Error:', error);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="faucet-interface">
      <h2>Shahcoin Testnet Faucet</h2>
      
      <div className="input-group">
        <label>Wallet Address:</label>
        <input
          type="text"
          value={walletAddress}
          onChange={(e) => setWalletAddress(e.target.value)}
          placeholder="0x1234567890abcdef..."
        />
      </div>
      
      <div className="input-group">
        <label>Amount (SHAH):</label>
        <select value={amount} onChange={(e) => setAmount(Number(e.target.value))}>
          <option value={100}>100 SHAH</option>
          <option value={500}>500 SHAH</option>
          <option value={1000}>1000 SHAH</option>
          <option value={5000}>5000 SHAH</option>
        </select>
      </div>
      
      <button 
        onClick={requestCoins} 
        disabled={loading || !walletAddress}
      >
        {loading ? 'Requesting...' : 'Request Testnet SHAH'}
      </button>
      
      {result && (
        <div className="result">
          <h3>Transaction Result</h3>
          <p>Transaction Hash: {result.txHash}</p>
          <p>Status: {result.status}</p>
          <p>New Balance: {result.newBalance} SHAH</p>
        </div>
      )}
    </div>
  );
};
```

### Sandbox Interface
```javascript
// React component for sandbox interface
import React, { useState, useEffect } from 'react';

const SandboxInterface = () => {
  const [sandboxId, setSandboxId] = useState(null);
  const [balance, setBalance] = useState(0);
  const [contracts, setContracts] = useState([]);
  const [loading, setLoading] = useState(false);

  useEffect(() => {
    if (sandboxId) {
      loadSandboxData();
    }
  }, [sandboxId]);

  const createSandbox = async () => {
    setLoading(true);
    try {
      const response = await fetch('/api/sandbox/create', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          initialBalance: 10000,
          network: 'sandbox'
        })
      });
      
      const data = await response.json();
      setSandboxId(data.sandboxId);
    } catch (error) {
      console.error('Error:', error);
    } finally {
      setLoading(false);
    }
  };

  const loadSandboxData = async () => {
    const response = await fetch(`/api/sandbox/${sandboxId}/status`);
    const data = await response.json();
    setBalance(data.balance);
    setContracts(data.contracts);
  };

  return (
    <div className="sandbox-interface">
      <h2>Shahcoin Sandbox Environment</h2>
      
      {!sandboxId ? (
        <button onClick={createSandbox} disabled={loading}>
          {loading ? 'Creating...' : 'Create New Sandbox'}
        </button>
      ) : (
        <div className="sandbox-dashboard">
          <div className="sandbox-info">
            <h3>Sandbox ID: {sandboxId}</h3>
            <p>Balance: {balance} SHAH</p>
          </div>
          
          <div className="contracts-list">
            <h3>Deployed Contracts</h3>
            {contracts.map(contract => (
              <div key={contract.address} className="contract-item">
                <p>Address: {contract.address}</p>
                <p>Type: {contract.type}</p>
                <p>Name: {contract.name}</p>
              </div>
            ))}
          </div>
          
          <div className="sandbox-actions">
            <button onClick={() => deployContract()}>
              Deploy Contract
            </button>
            <button onClick={() => createToken()}>
              Create Token
            </button>
            <button onClick={() => mintNFT()}>
              Mint NFT
            </button>
          </div>
        </div>
      )}
    </div>
  );
};
```

## 🔒 Security & Rate Limiting

### Rate Limiting Configuration
```javascript
// Rate limiting configuration
const rateLimitConfig = {
  requestsPerDay: 10,
  requestsPerHour: 2,
  cooldownPeriod: 3600, // 1 hour
  maxAmountPerRequest: 5000,
  maxTotalPerDay: 10000,
  ipWhitelist: ['trusted-ip-1', 'trusted-ip-2'],
  walletBlacklist: ['blacklisted-wallet-1']
};

// Check rate limits
const checkRateLimit = async (walletAddress, ipAddress) => {
  const response = await fetch('/api/faucet/rate-limit', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      walletAddress,
      ipAddress,
      timestamp: Date.now()
    })
  });
  
  return response.json();
};
```

### Captcha Integration
```javascript
// Google reCAPTCHA integration
const verifyCaptcha = async (captchaToken) => {
  const response = await fetch('/api/captcha/verify', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      captchaToken,
      secretKey: process.env.RECAPTCHA_SECRET
    })
  });
  
  return response.json();
};

// Frontend captcha component
const CaptchaComponent = ({ onVerify }) => {
  const [captchaToken, setCaptchaToken] = useState('');

  const handleCaptchaChange = (token) => {
    setCaptchaToken(token);
    onVerify(token);
  };

  return (
    <div className="captcha-container">
      <ReCAPTCHA
        sitekey={process.env.RECAPTCHA_SITE_KEY}
        onChange={handleCaptchaChange}
      />
    </div>
  );
};
```

## 📊 Monitoring & Analytics

### Faucet Analytics
```javascript
// Get faucet usage analytics
const getFaucetAnalytics = async () => {
  const response = await fetch('/api/faucet/analytics', {
    headers: { 'Authorization': `Bearer ${apiKey}` }
  });
  
  return response.json();
};

// Example analytics data
const analytics = {
  totalRequests: 15000,
  totalSHAHDistributed: 75000000,
  uniqueWallets: 8500,
  averageRequestAmount: 5000,
  topRequestingIPs: [
    { ip: '192.168.1.1', requests: 150 },
    { ip: '10.0.0.1', requests: 120 }
  ],
  dailyStats: [
    { date: '2024-01-15', requests: 500, amount: 2500000 },
    { date: '2024-01-16', requests: 480, amount: 2400000 }
  ]
};
```

### Sandbox Analytics
```javascript
// Get sandbox usage analytics
const getSandboxAnalytics = async () => {
  const response = await fetch('/api/sandbox/analytics', {
    headers: { 'Authorization': `Bearer ${apiKey}` }
  });
  
  return response.json();
};

// Example sandbox analytics
const sandboxAnalytics = {
  totalSandboxes: 2500,
  activeSandboxes: 1800,
  totalContracts: 15000,
  averageContractsPerSandbox: 6,
  mostPopularTemplates: [
    { template: 'ERC20', deployments: 5000 },
    { template: 'NFT', deployments: 3000 },
    { template: 'Staking', deployments: 2000 }
  ],
  averageSessionTime: 45 // minutes
};
```

## 🛠 Development

### Local Development
```bash
# Install dependencies
npm install

# Start development server
npm run dev

# Start testnet node
npm run testnet

# Start sandbox environment
npm run sandbox

# Run tests
npm test
```

### Environment Variables
```bash
# Required environment variables
SHAHCOIN_RPC_URL=https://testnet-rpc.shah.vip
FAUCET_PRIVATE_KEY=your_faucet_private_key
RECAPTCHA_SITE_KEY=your_recaptcha_site_key
RECAPTCHA_SECRET_KEY=your_recaptcha_secret_key
RATE_LIMIT_REDIS_URL=your_redis_url
```

## 📞 Support

- 📖 **Documentation**: [https://shah.vip/dev/faucet](https://shah.vip/dev/faucet)
- 🐛 **Issues**: [GitHub Issues](https://github.com/SHAHCoinvip/shahcoin-core/issues)
- 💬 **Discord**: [Shahcoin Community](https://discord.gg/shahcoin)

## 📄 License

MIT License - see [LICENSE](./LICENSE) for details.

---

**Built with love by SHAHCOIN Core Contributors** ❤️
