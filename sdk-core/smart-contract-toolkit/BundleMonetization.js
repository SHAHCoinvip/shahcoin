/**
 * Shahcoin Bundle Creation Monetization System
 * Implements bundle discount for Token + NFT creation with premium add-ons
 */

import { TokenMonetizationSystem } from './token-monetization.js';
import { NFTMonetizationSystem } from './NFTMonetization.js';

class BundleMonetizationSystem {
  constructor(config = {}) {
    this.config = {
      bundleDiscountUSD: 15, // $15 discount for bundle
      shahSwapAPI: 'https://api.shah.vip/price',
      stripePublicKey: config.stripePublicKey,
      ...config
    };
    
    // Initialize individual monetization systems
    this.tokenMonetization = new TokenMonetizationSystem(config);
    this.nftMonetization = new NFTMonetizationSystem(config);
    
    this.paymentMethods = {
      shah: 'Native SHAH',
      shahToken: 'SHAHtoken (SHI-20)',
      stripe: 'Credit Card (USD)'
    };
  }

  /**
   * Get current SHAH price from ShahSwap
   */
  async getShahPrice() {
    try {
      const response = await fetch(this.config.shahSwapAPI);
      const data = await response.json();
      return data.shah.usd;
    } catch (error) {
      console.error('Failed to fetch SHAH price:', error);
      throw new Error('Unable to fetch current SHAH price');
    }
  }

  /**
   * Calculate bundle pricing
   */
  async calculateBundlePrice(tokenConfig, nftConfig, selectedNFTAddOns = []) {
    const shahPrice = await this.getShahPrice();
    
    // Calculate individual prices
    const tokenPrice = await this.tokenMonetization.calculateBaseFee();
    const nftPrice = await this.nftMonetization.calculateTotalPrice(selectedNFTAddOns);
    
    // Calculate total without discount
    const totalWithoutDiscountUSD = tokenPrice.usd + nftPrice.totalUSD;
    const totalWithoutDiscountSHAH = totalWithoutDiscountUSD / shahPrice;
    
    // Apply bundle discount
    const bundleDiscountSHAH = this.config.bundleDiscountUSD / shahPrice;
    const totalWithDiscountUSD = totalWithoutDiscountUSD - this.config.bundleDiscountUSD;
    const totalWithDiscountSHAH = totalWithDiscountSHAH - bundleDiscountSHAH;
    
    return {
      tokenPrice: tokenPrice,
      nftPrice: nftPrice,
      bundleDiscount: {
        usd: this.config.bundleDiscountUSD,
        shah: bundleDiscountSHAH
      },
      pricing: {
        withoutDiscount: {
          usd: totalWithoutDiscountUSD,
          shah: totalWithoutDiscountSHAH
        },
        withDiscount: {
          usd: totalWithDiscountUSD,
          shah: totalWithDiscountSHAH
        },
        savings: {
          usd: this.config.bundleDiscountUSD,
          shah: bundleDiscountSHAH,
          percentage: ((this.config.bundleDiscountUSD / totalWithoutDiscountUSD) * 100).toFixed(1)
        }
      },
      breakdown: {
        token: {
          name: 'SHI-20 Token',
          priceUSD: tokenPrice.usd,
          priceSHAH: tokenPrice.shah
        },
        nft: {
          name: 'Basic NFT',
          priceUSD: nftPrice.totalUSD,
          priceSHAH: nftPrice.totalSHAH,
          addOns: nftPrice.addOns
        },
        discount: {
          name: 'Bundle Discount',
          priceUSD: -this.config.bundleDiscountUSD,
          priceSHAH: -bundleDiscountSHAH
        }
      }
    };
  }

  /**
   * Process bundle payment
   */
  async processBundlePayment(paymentMethod, bundlePrice, tokenConfig, nftConfig, selectedNFTAddOns = []) {
    switch (paymentMethod) {
      case 'shah':
        return await this.processShahBundlePayment(bundlePrice);
      case 'shahToken':
        return await this.processShahTokenBundlePayment(bundlePrice);
      case 'stripe':
        return await this.processStripeBundlePayment(bundlePrice);
      default:
        throw new Error('Invalid payment method');
    }
  }

  /**
   * Process native SHAH bundle payment
   */
  async processShahBundlePayment(bundlePrice) {
    try {
      // Connect to Shahcoin network
      const provider = new ethers.providers.JsonRpcProvider('https://rpc.shah.vip');
      const signer = provider.getSigner();
      
      // Get ShahFactory contract
      const shahFactory = new ethers.Contract(
        SHAHFACTORY_ADDRESS,
        SHAHFACTORY_ABI,
        signer
      );
      
      // Send bundle payment to ShahFactory
      const tx = await shahFactory.payBundleCreationFee({
        value: ethers.utils.parseEther(bundlePrice.pricing.withDiscount.shah.toString())
      });
      
      const receipt = await tx.wait();
      
      return {
        success: true,
        txHash: receipt.transactionHash,
        method: 'shah',
        amount: bundlePrice.pricing.withDiscount,
        bundleDiscount: bundlePrice.bundleDiscount
      };
    } catch (error) {
      console.error('SHAH bundle payment failed:', error);
      throw new Error('SHAH bundle payment failed: ' + error.message);
    }
  }

  /**
   * Process SHAHtoken (SHI-20) bundle payment
   */
  async processShahTokenBundlePayment(bundlePrice) {
    try {
      const provider = new ethers.providers.JsonRpcProvider('https://rpc.shah.vip');
      const signer = provider.getSigner();
      
      // Get SHAHtoken contract
      const shahToken = new ethers.Contract(
        SHAHTOKEN_ADDRESS,
        ERC20_ABI,
        signer
      );
      
      // Calculate SHAHtoken amount (assuming 1:1 with SHAH)
      const shahTokenAmount = ethers.utils.parseEther(bundlePrice.pricing.withDiscount.shah.toString());
      
      // Approve ShahFactory to spend SHAHtoken
      await shahToken.approve(SHAHFACTORY_ADDRESS, shahTokenAmount);
      
      // Pay with SHAHtoken
      const shahFactory = new ethers.Contract(
        SHAHFACTORY_ADDRESS,
        SHAHFACTORY_ABI,
        signer
      );
      
      const tx = await shahFactory.payBundleWithShahToken(shahTokenAmount);
      const receipt = await tx.wait();
      
      return {
        success: true,
        txHash: receipt.transactionHash,
        method: 'shahToken',
        amount: bundlePrice.pricing.withDiscount,
        bundleDiscount: bundlePrice.bundleDiscount
      };
    } catch (error) {
      console.error('SHAHtoken bundle payment failed:', error);
      throw new Error('SHAHtoken bundle payment failed: ' + error.message);
    }
  }

  /**
   * Process Stripe bundle payment
   */
  async processStripeBundlePayment(bundlePrice) {
    try {
      // Initialize Stripe
      const stripe = Stripe(this.config.stripePublicKey);
      
      // Create payment intent
      const response = await fetch('/api/create-bundle-payment-intent', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          amount: Math.round(bundlePrice.pricing.withDiscount.usd * 100), // Convert to cents
          currency: 'usd',
          bundleType: 'token-nft'
        })
      });
      
      const { clientSecret } = await response.json();
      
      // Confirm payment
      const { error, paymentIntent } = await stripe.confirmCardPayment(clientSecret, {
        payment_method: {
          card: elements.getElement('card'),
          billing_details: {
            name: 'Bundle Creator'
          }
        }
      });
      
      if (error) {
        throw new Error(error.message);
      }
      
      return {
        success: true,
        paymentIntentId: paymentIntent.id,
        method: 'stripe',
        amount: bundlePrice.pricing.withDiscount,
        bundleDiscount: bundlePrice.bundleDiscount
      };
    } catch (error) {
      console.error('Stripe bundle payment failed:', error);
      throw new Error('Stripe bundle payment failed: ' + error.message);
    }
  }

  /**
   * Save bundle payment data for admin tracking
   */
  async saveBundlePaymentData(paymentResult, tokenConfig, nftConfig, selectedNFTAddOns) {
    try {
      const paymentData = {
        timestamp: new Date().toISOString(),
        paymentMethod: paymentResult.method,
        amount: paymentResult.amount,
        bundleDiscount: paymentResult.bundleDiscount,
        tokenConfig: tokenConfig,
        nftConfig: nftConfig,
        selectedNFTAddOns: selectedNFTAddOns,
        txHash: paymentResult.txHash || paymentResult.paymentIntentId,
        status: 'completed',
        bundleType: 'token-nft'
      };
      
      // Save to database (implement your storage solution)
      await this.saveToDatabase(paymentData);
      
      return paymentData;
    } catch (error) {
      console.error('Failed to save bundle payment data:', error);
      // Don't throw error as payment was successful
    }
  }

  /**
   * Get available NFT add-ons for bundle
   */
  getAvailableNFTAddOns() {
    return this.nftMonetization.getPremiumAddOns();
  }

  /**
   * Get available payment methods
   */
  getPaymentMethods() {
    return this.paymentMethods;
  }

  /**
   * Validate bundle configuration
   */
  validateBundleConfig(tokenConfig, nftConfig) {
    const errors = [];
    
    // Validate token configuration
    const tokenErrors = this.tokenMonetization.validateTokenConfig(tokenConfig);
    errors.push(...tokenErrors.map(error => `Token: ${error}`));
    
    // Validate NFT configuration
    const nftErrors = this.nftMonetization.validateNFTConfig(nftConfig);
    errors.push(...nftErrors.map(error => `NFT: ${error}`));
    
    return errors;
  }

  /**
   * Estimate gas cost for bundle deployment
   */
  async estimateBundleDeploymentGas(tokenConfig, nftConfig, selectedNFTAddOns = []) {
    try {
      // Estimate gas for token deployment
      const tokenGasEstimate = await this.estimateTokenDeploymentGas(tokenConfig);
      
      // Estimate gas for NFT deployment
      const nftGasEstimate = await this.estimateNFTDeploymentGas(nftConfig, selectedNFTAddOns);
      
      // Total gas estimation
      const totalGasEstimate = tokenGasEstimate.gasEstimate + nftGasEstimate.gasEstimate;
      
      // Get current gas price
      const provider = new ethers.providers.JsonRpcProvider('https://rpc.shah.vip');
      const gasPrice = await provider.getGasPrice();
      
      // Calculate cost
      const costInWei = totalGasEstimate.mul(gasPrice);
      const costInShah = ethers.utils.formatEther(costInWei);
      
      // Get SHAH price for USD conversion
      const shahPrice = await this.getShahPrice();
      const costInUSD = parseFloat(costInShah) * shahPrice;

      return {
        tokenGas: tokenGasEstimate,
        nftGas: nftGasEstimate,
        totalGasEstimate: totalGasEstimate.toString(),
        gasPrice: ethers.utils.formatUnits(gasPrice, 'gwei') + ' gwei',
        costInShah: parseFloat(costInShah).toFixed(6),
        costInUSD: costInUSD.toFixed(2),
        shahPrice: shahPrice
      };
      
    } catch (error) {
      console.error('Bundle gas estimation failed:', error);
      throw new Error(`Bundle gas estimation failed: ${error.message}`);
    }
  }

  /**
   * Estimate token deployment gas
   */
  async estimateTokenDeploymentGas(tokenConfig) {
    // This would typically call the token integration's gas estimation
    // For now, return a basic estimate
    return {
      gasEstimate: ethers.BigNumber.from('2000000'), // 2M gas estimate
      costInShah: '0.002',
      costInUSD: '0.50'
    };
  }

  /**
   * Estimate NFT deployment gas
   */
  async estimateNFTDeploymentGas(nftConfig, selectedNFTAddOns = []) {
    // This would typically call the NFT integration's gas estimation
    // For now, return a basic estimate
    let baseGas = ethers.BigNumber.from('1500000'); // 1.5M gas base
    
    // Add gas for add-ons
    if (selectedNFTAddOns.includes('unlockableContent')) {
      baseGas = baseGas.add(ethers.BigNumber.from('100000')); // +100K gas
    }
    if (selectedNFTAddOns.includes('tradeLock')) {
      baseGas = baseGas.add(ethers.BigNumber.from('50000')); // +50K gas
    }
    if (selectedNFTAddOns.includes('boosterTag')) {
      baseGas = baseGas.add(ethers.BigNumber.from('75000')); // +75K gas
    }
    
    return {
      gasEstimate: baseGas,
      costInShah: '0.0015',
      costInUSD: '0.38'
    };
  }
}

/**
 * React Component for Bundle Creation Payment Modal
 */
class BundleCreationPaymentModal extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      selectedNFTAddOns: [],
      paymentMethod: 'shah',
      loading: false,
      bundlePrice: null,
      errors: []
    };
    
    this.bundleMonetization = new BundleMonetizationSystem({
      stripePublicKey: process.env.REACT_APP_STRIPE_PUBLIC_KEY
    });
  }

  async componentDidMount() {
    await this.calculateBundlePrice();
  }

  async calculateBundlePrice() {
    try {
      const { tokenConfig, nftConfig } = this.props;
      const bundlePrice = await this.bundleMonetization.calculateBundlePrice(
        tokenConfig,
        nftConfig,
        this.state.selectedNFTAddOns
      );
      this.setState({ bundlePrice });
    } catch (error) {
      this.setState({ errors: [error.message] });
    }
  }

  handleNFTAddOnToggle = async (addOnKey) => {
    const { selectedNFTAddOns } = this.state;
    const newSelectedNFTAddOns = selectedNFTAddOns.includes(addOnKey)
      ? selectedNFTAddOns.filter(key => key !== addOnKey)
      : [...selectedNFTAddOns, addOnKey];
    
    this.setState({ selectedNFTAddOns: newSelectedNFTAddOns });
    await this.calculateBundlePrice();
  }

  handlePaymentMethodChange = (method) => {
    this.setState({ paymentMethod: method });
  }

  handlePayment = async () => {
    const { tokenConfig, nftConfig, onSuccess, onError } = this.props;
    const { selectedNFTAddOns, paymentMethod, bundlePrice } = this.state;
    
    // Validate bundle configuration
    const errors = this.bundleMonetization.validateBundleConfig(tokenConfig, nftConfig);
    if (errors.length > 0) {
      this.setState({ errors });
      return;
    }
    
    this.setState({ loading: true, errors: [] });
    
    try {
      // Process payment
      const paymentResult = await this.bundleMonetization.processBundlePayment(
        paymentMethod,
        bundlePrice,
        tokenConfig,
        nftConfig,
        selectedNFTAddOns
      );
      
      // Save payment data
      await this.bundleMonetization.saveBundlePaymentData(
        paymentResult,
        tokenConfig,
        nftConfig,
        selectedNFTAddOns
      );
      
      onSuccess(paymentResult);
    } catch (error) {
      this.setState({ errors: [error.message] });
      onError(error);
    } finally {
      this.setState({ loading: false });
    }
  }

  render() {
    const {
      selectedNFTAddOns,
      paymentMethod,
      loading,
      bundlePrice,
      errors
    } = this.state;
    
    const nftAddOns = this.bundleMonetization.getAvailableNFTAddOns();
    const paymentMethods = this.bundleMonetization.getPaymentMethods();

    return (
      <div className="bundle-creation-payment-modal">
        <div className="modal-header">
          <h2>🎁 Bundle Creation Payment</h2>
          <p>Create both Token + NFT with bundle discount</p>
        </div>

        {errors.length > 0 && (
          <div className="error-messages">
            {errors.map((error, index) => (
              <div key={index} className="error-message">
                {error}
              </div>
            ))}
          </div>
        )}

        <div className="bundle-info">
          <h3>Bundle Contents</h3>
          <div className="bundle-items">
            <div className="bundle-item">
              <span className="item-icon">🪙</span>
              <span className="item-name">1 SHI-20 Token</span>
                              <span className="item-description">Standard SHI-20 token with basic features</span>
            </div>
            <div className="bundle-item">
              <span className="item-icon">🖼️</span>
              <span className="item-name">1 Basic NFT</span>
              <span className="item-description">Shahcoin-branded NFT with default artwork</span>
            </div>
          </div>
        </div>

        <div className="nft-add-ons">
          <h3>NFT Premium Add-Ons (Optional)</h3>
          <div className="add-ons-grid">
            {Object.entries(nftAddOns).map(([key, addOn]) => (
              <div key={key} className="add-on-item">
                <label className="add-on-checkbox">
                  <input
                    type="checkbox"
                    checked={selectedNFTAddOns.includes(key)}
                    onChange={() => this.handleNFTAddOnToggle(key)}
                  />
                  <div className="add-on-content">
                    <h4>{addOn.name}</h4>
                    <p>{addOn.description}</p>
                    <span className="add-on-price">+${addOn.priceUSD}</span>
                  </div>
                </label>
              </div>
            ))}
          </div>
        </div>

        <div className="payment-method">
          <h3>Payment Method</h3>
          <div className="payment-methods">
            {Object.entries(paymentMethods).map(([key, name]) => (
              <label key={key} className="payment-method-option">
                <input
                  type="radio"
                  name="paymentMethod"
                  value={key}
                  checked={paymentMethod === key}
                  onChange={(e) => this.handlePaymentMethodChange(e.target.value)}
                />
                <span>{name}</span>
              </label>
            ))}
          </div>
        </div>

        {bundlePrice && (
          <div className="price-summary">
            <h3>Bundle Price Summary</h3>
            <div className="price-breakdown">
              <div className="price-item">
                <span>Token Creation:</span>
                <span>${bundlePrice.breakdown.token.priceUSD} ({bundlePrice.breakdown.token.priceSHAH.toFixed(6)} SHAH)</span>
              </div>
              
              <div className="price-item">
                <span>NFT Creation:</span>
                <span>${bundlePrice.breakdown.nft.priceUSD} ({bundlePrice.breakdown.nft.priceSHAH.toFixed(6)} SHAH)</span>
              </div>
              
              {bundlePrice.breakdown.nft.addOns.map(addOn => (
                <div key={addOn.key} className="price-item add-on">
                  <span>{addOn.name}:</span>
                  <span>${addOn.priceUSD} ({addOn.priceSHAH.toFixed(6)} SHAH)</span>
                </div>
              ))}
              
              <div className="price-item discount">
                <span>🎁 Bundle Discount:</span>
                <span>-${bundlePrice.breakdown.discount.priceUSD} (-{bundlePrice.breakdown.discount.priceSHAH.toFixed(6)} SHAH)</span>
              </div>
              
              <div className="price-item total">
                <span>Total (with discount):</span>
                <span>${bundlePrice.pricing.withDiscount.usd} ({bundlePrice.pricing.withDiscount.shah.toFixed(6)} SHAH)</span>
              </div>
              
              <div className="savings-info">
                <span>You save: ${bundlePrice.pricing.savings.usd} ({bundlePrice.pricing.savings.percentage}%)</span>
              </div>
            </div>
            
            <div className="network-fee-note">
              <small>
                * Network transaction fees (~0.002 SHAH) will be charged separately
              </small>
            </div>
          </div>
        )}

        <div className="modal-actions">
          <button
            className="btn-secondary"
            onClick={this.props.onCancel}
            disabled={loading}
          >
            Cancel
          </button>
          <button
            className="btn-primary"
            onClick={this.handlePayment}
            disabled={loading || !bundlePrice}
          >
            {loading ? 'Processing Payment...' : 'Create Bundle'}
          </button>
        </div>
      </div>
    );
  }
}

/**
 * Integration with Smart Contract Toolkit
 */
class BundleCreationWithMonetization {
  constructor() {
    this.bundleMonetization = new BundleMonetizationSystem();
  }

  /**
   * Create bundle with monetization
   */
  async createBundle(tokenConfig, nftConfig, selectedNFTAddOns = [], paymentMethod = 'shah') {
    try {
      // Calculate bundle price
      const bundlePrice = await this.bundleMonetization.calculateBundlePrice(
        tokenConfig,
        nftConfig,
        selectedNFTAddOns
      );
      
      // Process payment
      const paymentResult = await this.bundleMonetization.processBundlePayment(
        paymentMethod,
        bundlePrice,
        tokenConfig,
        nftConfig,
        selectedNFTAddOns
      );
      
      // Save payment data
      await this.bundleMonetization.saveBundlePaymentData(
        paymentResult,
        tokenConfig,
        nftConfig,
        selectedNFTAddOns
      );
      
      // Generate and deploy contracts
      const contractResult = await this.generateAndDeployBundleContracts(
        tokenConfig,
        nftConfig,
        selectedNFTAddOns
      );
      
      return {
        success: true,
        tokenAddress: contractResult.tokenAddress,
        nftAddress: contractResult.nftAddress,
        paymentResult: paymentResult,
        bundlePrice: bundlePrice
      };
    } catch (error) {
      console.error('Bundle creation failed:', error);
      throw error;
    }
  }

  /**
   * Generate and deploy bundle contracts
   */
  async generateAndDeployBundleContracts(tokenConfig, nftConfig, selectedNFTAddOns = []) {
    // This would integrate with the existing token and NFT creation systems
    // For now, return mock addresses
    return {
      tokenAddress: '0x1234567890123456789012345678901234567890',
      nftAddress: '0x0987654321098765432109876543210987654321',
      deployedAt: new Date().toISOString()
    };
  }
}

// Export for use in other modules
export {
  BundleMonetizationSystem,
  BundleCreationPaymentModal,
  BundleCreationWithMonetization
};
