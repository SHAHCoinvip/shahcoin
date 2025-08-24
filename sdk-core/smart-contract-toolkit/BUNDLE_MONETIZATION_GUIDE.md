# Shahcoin Bundle Creation Monetization Guide

## 🎁 Overview

The Shahcoin Bundle Creation Monetization System offers a special discount when creating both a Token and NFT together. This bundle system provides significant savings while maintaining all the premium features and payment options of individual creation.

## 🚀 Quick Start

### 1. Basic Setup

```javascript
import { BundleMonetizationSystem } from './BundleMonetization.js';
import { BundleCreationWithMonetization } from './BundleMonetization.js';

// Initialize the bundle monetization system
const bundleMonetization = new BundleMonetizationSystem({
  stripePublicKey: 'pk_test_your_stripe_key_here'
});

// Initialize the integration
const bundleIntegration = new BundleCreationWithMonetization();
```

### 2. Create a Bundle with Payment

```javascript
// Define token configuration
const tokenConfig = {
  name: "My Awesome Token",
  symbol: "MAT",
  decimals: 18,
  maxSupply: 1000000,
  initialSupply: 100000
};

// Define NFT configuration
const nftConfig = {
  name: "My Awesome NFT",
  description: "A unique digital collectible",
  imageURI: "https://ipfs.io/ipfs/QmYourImageHash",
  maxSupply: 1000
};

// Select NFT add-ons (optional)
const selectedNFTAddOns = [
  'customImageUpload',
  'verifiedBadge',
  'unlockableContent'
];

// Create bundle with payment
const result = await bundleIntegration.createBundle(
  tokenConfig,
  nftConfig,
  selectedNFTAddOns,
  'shah' // payment method
);

console.log('Bundle created successfully!');
console.log('Token Address:', result.tokenAddress);
console.log('NFT Address:', result.nftAddress);
console.log('Payment Result:', result.paymentResult);
```

## 💰 Bundle Pricing Structure

### Normal Pricing (Separate)
- **Token Creation**: $49 USD worth of SHAH
- **NFT Creation**: $25 USD worth of SHAH
- **Total**: $74 USD

### Bundle Pricing (Combined)
- **Bundle Price**: $59 USD worth of SHAH
- **Bundle Discount**: $15 USD savings
- **Savings Percentage**: ~20.3%

### NFT Add-Ons Available in Bundle

| Add-On | Price | Description |
|--------|-------|-------------|
| Custom Image Upload | +$5 | Upload your own artwork (SVG, PNG, GIF) |
| Verified Badge | +$10 | ShahExplorer + Wallet badge |
| Unlockable Content | +$8 | Add secret content (text, file, video) |
| Trade Lock (Soulbound) | +$6 | Prevent transfer – great for proof NFTs |
| Booster Tag | +$9 | Eligible for staking/APR bonuses |

### Payment Methods

1. **Native SHAH** - Pay directly with SHAH coins
2. **SHAHtoken (SHI-20)** - Pay with SHAHtoken if you prefer SHI-20
3. **Credit Card (USD)** - Pay with credit card via Stripe integration

## 🔧 Advanced Usage

### Calculate Bundle Price

```javascript
// Calculate bundle pricing
const bundlePrice = await bundleMonetization.calculateBundlePrice(
  tokenConfig,
  nftConfig,
  selectedNFTAddOns
);

console.log('Bundle Price Breakdown:');
console.log(`Token: $${bundlePrice.breakdown.token.priceUSD}`);
console.log(`NFT: $${bundlePrice.breakdown.nft.priceUSD}`);
console.log(`Discount: -$${bundlePrice.breakdown.discount.priceUSD}`);
console.log(`Total: $${bundlePrice.pricing.withDiscount.usd}`);
console.log(`Savings: $${bundlePrice.pricing.savings.usd} (${bundlePrice.pricing.savings.percentage}%)`);
```

### Process Bundle Payment

```javascript
// Process payment
const paymentResult = await bundleMonetization.processBundlePayment(
  'shah', // payment method
  bundlePrice,
  tokenConfig,
  nftConfig,
  selectedNFTAddOns
);

console.log('Payment successful:', paymentResult);
```

### Gas Estimation

```javascript
// Estimate deployment gas cost
const gasEstimate = await bundleMonetization.estimateBundleDeploymentGas(
  tokenConfig,
  nftConfig,
  selectedNFTAddOns
);

console.log(`Total gas estimate: ${gasEstimate.totalGasEstimate} gas`);
console.log(`Gas cost: ${gasEstimate.costInShah} SHAH ($${gasEstimate.costInUSD})`);
console.log(`Token gas: ${gasEstimate.tokenGas.gasEstimate} gas`);
console.log(`NFT gas: ${gasEstimate.nftGas.gasEstimate} gas`);
```

## 🎨 UI Integration

### React Component Usage

```jsx
import React, { useState } from 'react';
import { BundleCreationPaymentModal } from './BundleMonetization.js';
import './bundle-monetization.css';

const BundleCreationPage = () => {
  const [showBundleModal, setShowBundleModal] = useState(false);
  const [tokenConfig, setTokenConfig] = useState({
    name: '',
    symbol: '',
    decimals: 18,
    maxSupply: 1000000,
    initialSupply: 100000
  });
  const [nftConfig, setNftConfig] = useState({
    name: '',
    description: '',
    imageURI: '',
    maxSupply: 1000
  });

  const handleBundleSuccess = (result) => {
    console.log('Bundle creation successful:', result);
    setShowBundleModal(false);
    // Handle success (redirect, show success message, etc.)
  };

  const handleBundleError = (error) => {
    console.error('Bundle creation failed:', error);
    // Handle error (show error message, etc.)
  };

  return (
    <div>
      {/* Your token and NFT configuration forms */}
      <div className="configuration-forms">
        <div className="token-config">
          <h3>Token Configuration</h3>
          <input
            type="text"
            placeholder="Token Name"
            value={tokenConfig.name}
            onChange={(e) => setTokenConfig({...tokenConfig, name: e.target.value})}
          />
          <input
            type="text"
            placeholder="Token Symbol"
            value={tokenConfig.symbol}
            onChange={(e) => setTokenConfig({...tokenConfig, symbol: e.target.value})}
          />
          {/* More token form fields */}
        </div>

        <div className="nft-config">
          <h3>NFT Configuration</h3>
          <input
            type="text"
            placeholder="NFT Name"
            value={nftConfig.name}
            onChange={(e) => setNftConfig({...nftConfig, name: e.target.value})}
          />
          <textarea
            placeholder="NFT Description"
            value={nftConfig.description}
            onChange={(e) => setNftConfig({...nftConfig, description: e.target.value})}
          />
          {/* More NFT form fields */}
        </div>
      </div>

      <button 
        className="bundle-creation-btn"
        onClick={() => setShowBundleModal(true)}
      >
        🎁 Create Bundle (Save $15)
      </button>

      {showBundleModal && (
        <BundleCreationPaymentModal
          tokenConfig={tokenConfig}
          nftConfig={nftConfig}
          onSuccess={handleBundleSuccess}
          onError={handleBundleError}
          onCancel={() => setShowBundleModal(false)}
        />
      )}
    </div>
  );
};
```

### Bundle Toggle in Creation UI

```jsx
const CreationOptions = () => {
  const [creationMode, setCreationMode] = useState('individual'); // 'individual' or 'bundle'

  return (
    <div className="creation-options">
      <div className="mode-selector">
        <label className="mode-option">
          <input
            type="radio"
            name="creationMode"
            value="individual"
            checked={creationMode === 'individual'}
            onChange={(e) => setCreationMode(e.target.value)}
          />
          <span>Create Separately</span>
        </label>
        
        <label className="mode-option bundle-option">
          <input
            type="radio"
            name="creationMode"
            value="bundle"
            checked={creationMode === 'bundle'}
            onChange={(e) => setCreationMode(e.target.value)}
          />
          <span>🎁 Bundle (Save $15)</span>
          <div className="bundle-badge">SAVE 20%</div>
        </label>
      </div>

      {creationMode === 'bundle' && (
        <div className="bundle-info">
          <p>Create both Token + NFT together and save $15!</p>
          <ul>
            <li>1 SHI-20 Token (Standard features)</li>
            <li>1 Basic NFT (Shahcoin-branded)</li>
            <li>Optional NFT add-ons available</li>
            <li>Same payment methods as individual creation</li>
          </ul>
        </div>
      )}
    </div>
  );
};
```

## 📊 Bundle Analytics

### Payment Analytics

```javascript
// Get bundle payment statistics
const analytics = await bundleMonetization.getBundleAnalytics({
  startDate: '2024-01-01',
  endDate: '2024-12-31'
});

console.log('Bundle Analytics:');
console.log('Total bundles sold:', analytics.totalBundles);
console.log('Total revenue:', analytics.totalRevenue);
console.log('Average savings per bundle:', analytics.averageSavings);
console.log('Most popular NFT add-ons:', analytics.popularAddOns);
```

### Bundle Performance Tracking

```javascript
// Track bundle performance
const bundleMetrics = {
  bundlesCreated: 150,
  totalSavings: 2250, // $15 * 150 bundles
  averageAddOnsPerBundle: 2.3,
  conversionRate: 0.35, // 35% of users choose bundle over individual
  revenueIncrease: 0.25 // 25% increase in revenue due to bundles
};
```

## 🔐 Security Features

### Bundle Payment Security

1. **Unified Payment Processing** - Single transaction for both token and NFT
2. **Atomic Operations** - Both contracts deploy or both fail
3. **Encrypted Storage** - All bundle payment data is encrypted
4. **Rate Limiting** - Prevents abuse and spam
5. **Input Validation** - Comprehensive validation of both token and NFT configs

### Contract Security

1. **Sequential Deployment** - Token deploys first, then NFT
2. **Rollback Capability** - Can rollback if NFT deployment fails
3. **Gas Optimization** - Optimized gas usage for bundle deployment
4. **Error Handling** - Comprehensive error handling and recovery

## 🚀 Deployment Examples

### Basic Bundle Creation

```javascript
// Simple bundle without add-ons
const basicToken = {
  name: "Basic Token",
  symbol: "BTK",
  decimals: 18,
  maxSupply: 1000000,
  initialSupply: 100000
};

const basicNFT = {
  name: "Basic NFT",
  description: "A simple digital collectible",
  imageURI: "https://ipfs.io/ipfs/QmDefaultImage",
  maxSupply: 1000
};

const result = await bundleIntegration.createBundle(
  basicToken,
  basicNFT,
  [], // no add-ons
  'shah'
);
```

### Premium Bundle with All Add-Ons

```javascript
// Premium bundle with all NFT features
const premiumToken = {
  name: "Premium Token",
  symbol: "PTK",
  decimals: 18,
  maxSupply: 10000000,
  initialSupply: 1000000
};

const premiumNFT = {
  name: "Premium NFT",
  description: "A premium digital collectible with all features",
  imageURI: "https://ipfs.io/ipfs/QmPremiumImage",
  maxSupply: 100
};

const result = await bundleIntegration.createBundle(
  premiumToken,
  premiumNFT,
  [
    'customImageUpload',
    'verifiedBadge',
    'unlockableContent',
    'tradeLock',
    'boosterTag'
  ],
  'stripe'
);
```

### Batch Bundle Creation

```javascript
// Create multiple bundles in batch
const bundleConfigs = [
  {
    token: { name: "Token 1", symbol: "TK1", /* ... */ },
    nft: { name: "NFT 1", description: "First NFT", /* ... */ },
    addOns: ['customImageUpload', 'verifiedBadge']
  },
  {
    token: { name: "Token 2", symbol: "TK2", /* ... */ },
    nft: { name: "NFT 2", description: "Second NFT", /* ... */ },
    addOns: ['unlockableContent']
  }
];

for (const config of bundleConfigs) {
  try {
    const result = await bundleIntegration.createBundle(
      config.token,
      config.nft,
      config.addOns,
      'shah'
    );
    console.log(`Bundle ${config.token.name} created:`, result);
  } catch (error) {
    console.error(`Failed to create bundle ${config.token.name}:`, error);
  }
}
```

## 🔧 Error Handling

### Common Bundle Errors and Solutions

```javascript
try {
  const result = await bundleIntegration.createBundle(tokenConfig, nftConfig, addOns, paymentMethod);
} catch (error) {
  switch (error.message) {
    case 'Insufficient SHAH balance':
      console.error('Please ensure you have enough SHAH for bundle payment');
      break;
    case 'Invalid token configuration':
      console.error('Please check your token configuration');
      break;
    case 'Invalid NFT configuration':
      console.error('Please check your NFT configuration');
      break;
    case 'Bundle payment failed':
      console.error('Bundle payment processing failed, please try again');
      break;
    case 'Token deployment failed':
      console.error('Token deployment failed, NFT will not be deployed');
      break;
    case 'NFT deployment failed':
      console.error('NFT deployment failed, token was deployed successfully');
      break;
    default:
      console.error('Unexpected bundle error:', error.message);
  }
}
```

### Bundle Validation

```javascript
// Validate bundle configuration before processing
const errors = bundleMonetization.validateBundleConfig(tokenConfig, nftConfig);
if (errors.length > 0) {
  console.error('Bundle configuration errors:', errors);
  return;
}

// Check bundle eligibility
const eligibility = await bundleMonetization.checkBundleEligibility(tokenConfig, nftConfig);
if (!eligibility.eligible) {
  console.error('Bundle not eligible:', eligibility.reason);
  return;
}
```

## 📈 Bundle Marketing Features

### Bundle Promotions

```javascript
// Seasonal bundle discounts
const seasonalDiscounts = {
  'christmas': { discount: 25, name: 'Christmas Bundle' },
  'newyear': { discount: 20, name: 'New Year Bundle' },
  'summer': { discount: 15, name: 'Summer Bundle' }
};

// Limited time offers
const limitedOffers = {
  'flash_sale': { discount: 30, duration: '2 hours' },
  'early_bird': { discount: 25, duration: '24 hours' }
};
```

### Bundle Recommendations

```javascript
// Recommend bundle based on user behavior
const recommendations = await bundleMonetization.getBundleRecommendations(userId);

console.log('Recommended bundles:');
recommendations.forEach(rec => {
  console.log(`- ${rec.name}: ${rec.description} (Save $${rec.savings})`);
});
```

## 🆘 Support and Troubleshooting

### Getting Help

1. **Documentation** - Comprehensive guides and examples
2. **Community** - Join the Shahcoin developer community
3. **Support Tickets** - Submit bundle-specific issues
4. **Discord** - Real-time support for bundle creation

### Common Bundle Issues

1. **Payment Processing** - Ensure sufficient SHAH balance for bundle
2. **Gas Estimation** - Bundle requires more gas than individual creation
3. **Contract Deployment** - Both contracts must deploy successfully
4. **Add-On Compatibility** - Verify NFT add-ons are compatible with bundle

### Debug Mode

```javascript
// Enable debug mode for detailed logging
const debugBundle = new BundleMonetizationSystem({
  debug: true,
  logLevel: 'verbose'
});

// Check bundle system status
const status = await debugBundle.getBundleSystemStatus();
console.log('Bundle system status:', status);
```

## 📝 Bundle Terms and Conditions

### Bundle Eligibility

1. **One Bundle Per Transaction** - Cannot combine multiple bundles
2. **Add-On Restrictions** - Only NFT add-ons available in bundle
3. **Payment Requirements** - Full payment required before deployment
4. **Refund Policy** - No partial refunds if one contract fails

### Bundle Limitations

1. **Token Features** - Standard token features only (no token add-ons)
2. **NFT Customization** - Limited to available NFT add-ons
3. **Deployment Order** - Token deploys first, then NFT
4. **Gas Requirements** - Higher gas requirements than individual creation

## 🎯 Future Enhancements

### Planned Bundle Features

1. **Dynamic Bundle Discounts** - Automatic discounts based on network activity
2. **Bundle Templates** - Pre-configured bundle templates for common use cases
3. **Bundle Marketplace** - Marketplace for buying/selling bundle templates
4. **Advanced Analytics** - Detailed analytics dashboard for bundle performance
5. **Multi-language Support** - Internationalization for global users

### Customization Options

1. **White-label Bundles** - Custom branding for enterprise clients
2. **Bundle API** - REST APIs for third-party bundle integrations
3. **Custom Add-Ons** - Ability to create custom bundle add-ons
4. **Bundle Subscriptions** - Monthly/yearly bundle subscriptions

---

For more information, visit the [Shahcoin Developer Portal](https://shah.vip/dev) or join our [Discord community](https://discord.gg/shahcoin).
