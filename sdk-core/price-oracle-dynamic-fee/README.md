# Shahcoin Price Oracle + Dynamic Fee Module

## 🎯 Overview

The Price Oracle + Dynamic Fee Module provides real-time SHAH price feeds and dynamic fee calculation capabilities for the Shahcoin ecosystem. This module enables developers to implement dynamic pricing strategies based on live market data.

## ✨ Features

### 🔄 Real-time Price Feeds
- **ShahSwap Integration**: Direct integration with ShahSwap DEX for accurate pricing
- **Multiple Sources**: Fallback to external exchanges for redundancy
- **Price Validation**: Cross-reference multiple sources for accuracy
- **Historical Data**: Access to historical price data for analysis

### 💰 Dynamic Fee Calculation
- **USD-based Pricing**: All fees calculated in USD and converted to SHAH
- **Market-responsive**: Fees adjust automatically based on SHAH price
- **Configurable Strategies**: Custom fee calculation algorithms
- **Real-time Updates**: Live fee updates as prices change

### 🛠️ Developer Tools
- **API Integration**: RESTful API for price and fee data
- **WebSocket Support**: Real-time price streaming
- **SDK Libraries**: JavaScript, Python, and C++ SDKs
- **Webhook Support**: Notifications for price changes

## 📦 Installation

```bash
npm install @shahcoin/price-oracle
```

## 🚀 Quick Start

### JavaScript SDK

```javascript
import { ShahcoinPriceOracle, DynamicFeeCalculator } from '@shahcoin/price-oracle';

// Initialize price oracle
const oracle = new ShahcoinPriceOracle({
  sources: ['shahswap', 'binance', 'coinbase'],
  updateInterval: 30000, // 30 seconds
  cacheTimeout: 60000    // 1 minute
});

// Get current SHAH price
const price = await oracle.getCurrentPrice();
console.log(`Current SHAH price: $${price.usd}`);

// Initialize dynamic fee calculator
const feeCalculator = new DynamicFeeCalculator({
  baseFeeUSD: 49,        // Base fee in USD
  priceOracle: oracle,
  updateInterval: 60000  // Update every minute
});

// Calculate dynamic fee
const fee = await feeCalculator.calculateFee('token_creation');
console.log(`Token creation fee: ${fee.shah} SHAH ($${fee.usd})`);
```

### Python SDK

```python
from shahcoin_price_oracle import ShahcoinPriceOracle, DynamicFeeCalculator

# Initialize price oracle
oracle = ShahcoinPriceOracle(
    sources=['shahswap', 'binance', 'coinbase'],
    update_interval=30,
    cache_timeout=60
)

# Get current price
price = await oracle.get_current_price()
print(f"Current SHAH price: ${price['usd']}")

# Calculate dynamic fee
fee_calc = DynamicFeeCalculator(
    base_fee_usd=49,
    price_oracle=oracle
)

fee = await fee_calc.calculate_fee('token_creation')
print(f"Token creation fee: {fee['shah']} SHAH (${fee['usd']})")
```

### C++ SDK

```cpp
#include <shahcoin/price_oracle.hpp>
#include <shahcoin/dynamic_fee_calculator.hpp>

// Initialize price oracle
ShahcoinPriceOracle oracle({
    {"sources", {"shahswap", "binance", "coinbase"}},
    {"update_interval", 30000},
    {"cache_timeout", 60000}
});

// Get current price
auto price = oracle.getCurrentPrice();
std::cout << "Current SHAH price: $" << price.usd << std::endl;

// Calculate dynamic fee
DynamicFeeCalculator feeCalc({
    {"base_fee_usd", 49.0},
    {"price_oracle", oracle}
});

auto fee = feeCalc.calculateFee("token_creation");
std::cout << "Token creation fee: " << fee.shah << " SHAH ($" << fee.usd << ")" << std::endl;
```

## 🔧 Configuration

### Price Oracle Configuration

```javascript
const config = {
  // Price sources (in order of preference)
  sources: ['shahswap', 'binance', 'coinbase', 'kraken'],
  
  // Update intervals
  updateInterval: 30000,        // 30 seconds
  cacheTimeout: 60000,          // 1 minute
  
  // Validation settings
  priceDeviationThreshold: 0.05, // 5% deviation threshold
  minValidSources: 2,           // Minimum valid sources
  
  // API settings
  apiKey: 'your-api-key',
  rateLimit: 100,               // Requests per minute
  
  // WebSocket settings
  enableWebSocket: true,
  wsReconnectInterval: 5000,    // 5 seconds
};
```

### Dynamic Fee Configuration

```javascript
const feeConfig = {
  // Base fees in USD
  baseFees: {
    token_creation: 49,
    nft_creation: 25,
    bundle_creation: 59,
    premium_features: {
      custom_logo: 10,
      verified_badge: 20,
      upgradeable_contract: 15,
      airdrop_tool: 8
    }
  },
  
  // Fee calculation strategies
  strategies: {
    token_creation: 'dynamic_usd',
    nft_creation: 'dynamic_usd',
    bundle_creation: 'discounted_dynamic',
    premium_features: 'fixed_usd'
  },
  
  // Discount settings
  discounts: {
    bundle_creation: 0.20,      // 20% discount
    volume_discount: {
      threshold: 1000,          // $1000 USD
      discount: 0.10            // 10% discount
    }
  },
  
  // Update settings
  updateInterval: 60000,        // 1 minute
  priceBuffer: 0.02,            // 2% price buffer
};
```

## 📊 API Reference

### Price Oracle API

#### `getCurrentPrice()`
Get the current SHAH price from all sources.

```javascript
const price = await oracle.getCurrentPrice();
// Returns: { usd: 0.85, shah: 1, timestamp: 1640995200000, sources: ['shahswap'] }
```

#### `getHistoricalPrice(timestamp)`
Get historical SHAH price at a specific timestamp.

```javascript
const price = await oracle.getHistoricalPrice(1640995200000);
// Returns: { usd: 0.82, shah: 1, timestamp: 1640995200000 }
```

#### `getPriceChange(duration)`
Get price change over a specified duration.

```javascript
const change = await oracle.getPriceChange('24h');
// Returns: { change: 0.05, percentage: 6.25, direction: 'up' }
```

#### `subscribeToPriceUpdates(callback)`
Subscribe to real-time price updates.

```javascript
const unsubscribe = oracle.subscribeToPriceUpdates((price) => {
  console.log(`New price: $${price.usd}`);
});
```

### Dynamic Fee Calculator API

#### `calculateFee(feeType, options)`
Calculate dynamic fee for a specific service.

```javascript
const fee = await feeCalculator.calculateFee('token_creation', {
  addOns: ['custom_logo', 'verified_badge'],
  volume: 5000 // USD
});
// Returns: { shah: 82.35, usd: 70.00, breakdown: {...} }
```

#### `getFeeBreakdown(feeType, options)`
Get detailed fee breakdown.

```javascript
const breakdown = await feeCalculator.getFeeBreakdown('token_creation', {
  addOns: ['custom_logo']
});
// Returns: { base: 49, addOns: 10, total: 59, shah: 69.41 }
```

#### `subscribeToFeeUpdates(callback)`
Subscribe to fee updates.

```javascript
const unsubscribe = feeCalculator.subscribeToFeeUpdates((fees) => {
  console.log('Updated fees:', fees);
});
```

## 🔌 Integration Examples

### Web Application Integration

```javascript
// React component for dynamic fee display
import React, { useState, useEffect } from 'react';
import { ShahcoinPriceOracle, DynamicFeeCalculator } from '@shahcoin/price-oracle';

function DynamicFeeDisplay() {
  const [fee, setFee] = useState(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const oracle = new ShahcoinPriceOracle();
    const feeCalc = new DynamicFeeCalculator({
      baseFeeUSD: 49,
      priceOracle: oracle
    });

    const updateFee = async () => {
      const calculatedFee = await feeCalc.calculateFee('token_creation');
      setFee(calculatedFee);
      setLoading(false);
    };

    updateFee();
    const interval = setInterval(updateFee, 60000); // Update every minute

    return () => clearInterval(interval);
  }, []);

  if (loading) return <div>Loading fee...</div>;

  return (
    <div>
      <h3>Token Creation Fee</h3>
      <p>${fee.usd} USD ({fee.shah} SHAH)</p>
      <small>Updated every minute</small>
    </div>
  );
}
```

### Smart Contract Integration

```solidity
// Solidity contract for dynamic fee calculation
contract DynamicFeeContract {
    ShahcoinPriceOracle public priceOracle;
    DynamicFeeCalculator public feeCalculator;
    
    constructor(address _priceOracle, address _feeCalculator) {
        priceOracle = ShahcoinPriceOracle(_priceOracle);
        feeCalculator = DynamicFeeCalculator(_feeCalculator);
    }
    
    function createToken(string memory name, string memory symbol) external payable {
        uint256 requiredFee = feeCalculator.calculateFee("token_creation");
        require(msg.value >= requiredFee, "Insufficient fee");
        
        // Token creation logic
        // ...
    }
}
```

### Qt Wallet Integration

```cpp
// C++ integration for Qt Wallet
#include <shahcoin/price_oracle.hpp>

class ShahcoinQtWallet {
private:
    ShahcoinPriceOracle m_priceOracle;
    DynamicFeeCalculator m_feeCalculator;
    
public:
    ShahcoinQtWallet() {
        m_priceOracle = ShahcoinPriceOracle({
            {"sources", {"shahswap", "binance"}},
            {"update_interval", 30000}
        });
        
        m_feeCalculator = DynamicFeeCalculator({
            {"base_fee_usd", 49.0},
            {"price_oracle", m_priceOracle}
        });
    }
    
    void updateFeeDisplay() {
        auto fee = m_feeCalculator.calculateFee("token_creation");
        emit feeUpdated(fee.shah, fee.usd);
    }
};
```

## 📈 Monitoring & Analytics

### Price Monitoring Dashboard

```javascript
// Real-time price monitoring
const monitor = new PriceMonitor({
  alertThreshold: 0.10, // 10% price change
  updateInterval: 30000
});

monitor.on('priceAlert', (alert) => {
  console.log(`Price alert: ${alert.message}`);
  // Send notification
});

monitor.on('sourceFailure', (source) => {
  console.log(`Source failed: ${source}`);
  // Switch to backup source
});
```

### Fee Analytics

```javascript
// Fee usage analytics
const analytics = new FeeAnalytics({
  tracking: true,
  storage: 'local'
});

// Track fee calculation
analytics.trackFeeCalculation({
  type: 'token_creation',
  fee: 82.35,
  price: 0.85,
  timestamp: Date.now()
});

// Get analytics report
const report = analytics.getReport('24h');
console.log('Fee analytics:', report);
```

## 🔒 Security Features

### Price Validation
- **Cross-source validation**: Compare prices across multiple sources
- **Deviation detection**: Alert on unusual price movements
- **Source reliability**: Weight sources based on historical accuracy

### Fee Protection
- **Price buffer**: Add buffer to prevent fee manipulation
- **Rate limiting**: Prevent excessive API calls
- **Audit trail**: Log all fee calculations for transparency

## 🚀 Performance Optimization

### Caching Strategy
- **Price caching**: Cache prices for 1 minute
- **Fee caching**: Cache calculated fees for 5 minutes
- **Historical caching**: Cache historical data for 1 hour

### Network Optimization
- **Connection pooling**: Reuse connections to price sources
- **Compression**: Compress API responses
- **CDN integration**: Use CDN for global distribution

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
