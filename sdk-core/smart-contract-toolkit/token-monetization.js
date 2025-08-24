/**
 * Shahcoin Token Creation Monetization System
 * Implements dynamic pricing, premium add-ons, and multiple payment methods
 */

class TokenMonetizationSystem {
  constructor(config = {}) {
    this.config = {
      baseFeeUSD: 49,
      shahSwapAPI: 'https://api.shah.vip/price',
      stripePublicKey: config.stripePublicKey,
      ...config
    };
    
    this.premiumAddOns = {
      customLogo: {
        name: 'Custom Logo Upload',
        priceUSD: 10,
        description: 'Upload your own logo for the token'
      },
      verifiedBadge: {
        name: 'Verified Badge',
        priceUSD: 20,
        description: 'Get verified badge in ShahExplorer and Web Wallet'
      },
      upgradeableContract: {
        name: 'Upgradeable Contract',
        priceUSD: 15,
        description: 'Make your contract upgradeable for future improvements'
      },
      airdropTool: {
        name: 'Token Airdrop Tool',
        priceUSD: 8,
        description: 'Built-in airdrop functionality for your token'
      }
    };
    
    this.paymentMethods = {
      shah: 'Native SHAH',
      shahToken: 'SHAHtoken (SHI-20)',
      stripe: 'Credit Card (USD)'
    };
  }

  /** Tier USD helpers */
  getTierUsd(tier = 'Basic') {
    return tier === 'Premium' ? 129 : tier === 'Pro' ? 79 : 49;
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
   * Calculate upgrade price (USD and SHAH) between tiers
   */
  async calculateUpgradePrice(currentTier = 'Basic', newTier = 'Pro') {
    const shahPrice = await this.getShahPrice();
    const usd = Math.max(0, this.getTierUsd(newTier) - this.getTierUsd(currentTier));
    const shah = usd / shahPrice;
    return { usd, shah, shahPrice };
  }

  /**
   * Calculate base fee in SHAH
   */
  async calculateBaseFee(tier = 'Basic') {
    const shahPrice = await this.getShahPrice();
    const tierUsd = this.getTierUsd(tier);
    const baseFeeSHAH = tierUsd / shahPrice;
    
    return {
      usd: tierUsd,
      shah: baseFeeSHAH,
      shahPrice: shahPrice
    };
  }

  /**
   * Calculate total price with selected add-ons
   */
  async calculateTotalPrice(selectedAddOns = [], options = {}) {
    const tier = options?.tier || 'Basic';
    const baseFee = await this.calculateBaseFee(tier);
    
    // Calculate add-on costs
    let addOnsTotalUSD = 0;
    const addOnsDetails = [];
    
    selectedAddOns.forEach(addOnKey => {
      const addOn = this.premiumAddOns[addOnKey];
      if (addOn) {
        addOnsTotalUSD += addOn.priceUSD;
        addOnsDetails.push({
          key: addOnKey,
          name: addOn.name,
          priceUSD: addOn.priceUSD,
          priceSHAH: addOn.priceUSD / baseFee.shahPrice
        });
      }
    });
    
    const totalUSD = baseFee.usd + addOnsTotalUSD;
    const totalSHAH = totalUSD / baseFee.shahPrice;
    
    return {
      baseFee: baseFee,
      tier,
      addOns: addOnsDetails,
      totalUSD: totalUSD,
      totalSHAH: totalSHAH,
      breakdown: {
        base: baseFee.usd,
        addOns: addOnsTotalUSD,
        total: totalUSD
      }
    };
  }

  /**
   * Process payment based on selected method
   */
  async processPayment(paymentMethod, amount, tokenConfig) {
    switch (paymentMethod) {
      case 'shah':
        return await this.processShahPayment(amount);
      case 'shahToken':
        return await this.processShahTokenPayment(amount);
      case 'stripe':
        return await this.processStripePayment(amount);
      default:
        throw new Error('Invalid payment method');
    }
  }

  /**
   * Process native SHAH payment
   */
  async processShahPayment(amount) {
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
      
      // Send payment to ShahFactory
      const tx = await shahFactory.payTokenCreationFee({
        value: ethers.utils.parseEther(amount.shah.toString())
      });
      
      const receipt = await tx.wait();
      
      return {
        success: true,
        txHash: receipt.transactionHash,
        method: 'shah',
        amount: amount
      };
    } catch (error) {
      console.error('SHAH payment failed:', error);
      throw new Error('SHAH payment failed: ' + error.message);
    }
  }

  /**
   * Process SHAHtoken (SHI-20) payment
   */
  async processShahTokenPayment(amount) {
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
      const shahTokenAmount = ethers.utils.parseEther(amount.shah.toString());
      
      // Approve ShahFactory to spend SHAHtoken
      await shahToken.approve(SHAHFACTORY_ADDRESS, shahTokenAmount);
      
      // Pay with SHAHtoken
      const shahFactory = new ethers.Contract(
        SHAHFACTORY_ADDRESS,
        SHAHFACTORY_ABI,
        signer
      );
      
      const tx = await shahFactory.payWithShahToken(shahTokenAmount);
      const receipt = await tx.wait();
      
      return {
        success: true,
        txHash: receipt.transactionHash,
        method: 'shahToken',
        amount: amount
      };
    } catch (error) {
      console.error('SHAHtoken payment failed:', error);
      throw new Error('SHAHtoken payment failed: ' + error.message);
    }
  }

  /**
   * Process Stripe payment
   */
  async processStripePayment(amount) {
    try {
      // Initialize Stripe
      const stripe = Stripe(this.config.stripePublicKey);
      
      // Create payment intent
      const response = await fetch('/api/create-payment-intent', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          amount: Math.round(amount.totalUSD * 100), // Convert to cents
          currency: 'usd'
        })
      });
      
      const { clientSecret } = await response.json();
      
      // Confirm payment
      const { error, paymentIntent } = await stripe.confirmCardPayment(clientSecret, {
        payment_method: {
          card: elements.getElement('card'),
          billing_details: {
            name: 'Token Creator'
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
        amount: amount
      };
    } catch (error) {
      console.error('Stripe payment failed:', error);
      throw new Error('Stripe payment failed: ' + error.message);
    }
  }

  /**
   * Save payment data for admin tracking
   */
  async savePaymentData(paymentResult, tokenConfig, selectedAddOns) {
    try {
      const paymentData = {
        timestamp: new Date().toISOString(),
        paymentMethod: paymentResult.method,
        amount: paymentResult.amount,
        tokenConfig: tokenConfig,
        selectedAddOns: selectedAddOns,
        txHash: paymentResult.txHash || paymentResult.paymentIntentId,
        status: 'completed'
      };
      
      // Save to database (implement your storage solution)
      await this.saveToDatabase(paymentData);
      
      return paymentData;
    } catch (error) {
      console.error('Failed to save payment data:', error);
      // Don't throw error as payment was successful
    }
  }

  /**
   * Get all available premium add-ons
   */
  getPremiumAddOns() {
    return this.premiumAddOns;
  }

  /**
   * Get all available payment methods
   */
  getPaymentMethods() {
    return this.paymentMethods;
  }

  /**
   * Validate token configuration
   */
  validateTokenConfig(config) {
    const errors = [];
    
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
    
    return errors;
  }
}

/**
 * React Component for Token Creation Payment Modal
 */
class TokenCreationPaymentModal extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      selectedAddOns: [],
      paymentMethod: 'shah',
      loading: false,
      priceData: null,
      errors: [],
      tier: 'Basic'
    };
    
    this.monetizationSystem = new TokenMonetizationSystem({
      stripePublicKey: process.env.REACT_APP_STRIPE_PUBLIC_KEY
    });
  }

  async componentDidMount() {
    await this.calculatePrice();
  }

  async calculatePrice() {
    try {
      const priceData = await this.monetizationSystem.calculateTotalPrice(
        this.state.selectedAddOns,
        { tier: this.state.tier }
      );
      this.setState({ priceData });
    } catch (error) {
      this.setState({ errors: [error.message] });
    }
  }

  handleAddOnToggle = async (addOnKey) => {
    const { selectedAddOns } = this.state;
    const newSelectedAddOns = selectedAddOns.includes(addOnKey)
      ? selectedAddOns.filter(key => key !== addOnKey)
      : [...selectedAddOns, addOnKey];
    
    this.setState({ selectedAddOns: newSelectedAddOns });
    await this.calculatePrice();
  }

  handlePaymentMethodChange = (method) => {
    this.setState({ paymentMethod: method });
  }

  handleTierChange = async (e) => {
    this.setState({ tier: e.target.value }, async () => {
      await this.calculatePrice();
    });
  }

  handlePayment = async () => {
    const { tokenConfig, onSuccess, onError } = this.props;
    const { selectedAddOns, paymentMethod, priceData, tier } = this.state;
    
    // Inject tier into config that will be validated and saved with payment
    const cfg = { ...tokenConfig, tier };
    
    // Validate token config
    const errors = this.monetizationSystem.validateTokenConfig(cfg);
    if (errors.length > 0) {
      this.setState({ errors });
      return;
    }
    
    this.setState({ loading: true, errors: [] });
    
    try {
      // Process payment
      const paymentResult = await this.monetizationSystem.processPayment(
        paymentMethod,
        priceData,
        cfg
      );
      
      // Save payment data
      await this.monetizationSystem.savePaymentData(
        paymentResult,
        cfg,
        selectedAddOns
      );
      
      onSuccess(paymentResult, cfg);
    } catch (error) {
      this.setState({ errors: [error.message] });
      onError(error);
    } finally {
      this.setState({ loading: false });
    }
  }

  renderTierSelector() {
    const { tier, priceData } = this.state;
    const benefits = tier === 'Premium'
      ? 'Includes Pro + advanced analytics, DAO-ready toggle, NFT airdrop integration, custom landing page.'
      : tier === 'Pro'
      ? 'Includes Basic + logo upload, verified badge, ShahSwap auto-listing, wallet visibility.'
      : 'Name, symbol, supply; no-code SHI-20; standard listing.';

    return (
      <div className="tier-section">
        <h3>Tier (Per Token)</h3>
        <div className="tier-row">
          <select value={tier} onChange={this.handleTierChange}>
            <option value="Basic">Basic — $49 per token</option>
            <option value="Pro">Pro — $79 per token</option>
            <option value="Premium">Premium — $129 per token</option>
          </select>
        </div>
        <div className="tier-benefits"><small>{benefits}</small></div>
        {priceData && (
          <div className="tier-price"><small>
            Base Fee: ${priceData.baseFee.usd} ({priceData.baseFee.shah.toFixed(6)} SHAH)
          </small></div>
        )}
      </div>
    );
  }

  render() {
    const { 
      selectedAddOns, 
      paymentMethod, 
      loading, 
      priceData, 
      errors 
    } = this.state;
    
    const premiumAddOns = this.monetizationSystem.getPremiumAddOns();
    const paymentMethods = this.monetizationSystem.getPaymentMethods();

    return (
      <div className="token-creation-payment-modal">
        <div className="modal-header">
          <h2>Token Creation Payment</h2>
          <p>Select tier (per token), add-ons, and payment method</p>
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

        {this.renderTierSelector()}

        <div className="premium-add-ons">
          <h3>Premium Add-Ons (Optional)</h3>
          <div className="add-ons-grid">
            {Object.entries(premiumAddOns).map(([key, addOn]) => (
              <div key={key} className="add-on-item">
                <label className="add-on-checkbox">
                  <input
                    type="checkbox"
                    checked={selectedAddOns.includes(key)}
                    onChange={() => this.handleAddOnToggle(key)}
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

        {priceData && (
          <div className="price-summary">
            <h3>Price Summary</h3>
            <div className="price-breakdown">
              <div className="price-item">
                <span>Base Fee:</span>
                <span>${priceData.baseFee.usd} ({priceData.baseFee.shah.toFixed(6)} SHAH)</span>
              </div>
              
              {priceData.addOns.map(addOn => (
                <div key={addOn.key} className="price-item add-on">
                  <span>{addOn.name}:</span>
                  <span>${addOn.priceUSD} ({addOn.priceSHAH.toFixed(6)} SHAH)</span>
                </div>
              ))}
              
              <div className="price-item total">
                <span>Total:</span>
                <span>${priceData.totalUSD} ({priceData.totalSHAH.toFixed(6)} SHAH)</span>
              </div>
            </div>
            
            <div className="network-fee-note">
              <small>
                * Network transaction fee (0.001 SHAH) will be charged separately
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
            disabled={loading || !priceData}
          >
            {loading ? 'Processing Payment...' : 'Pay & Verify'}
          </button>
        </div>
      </div>
    );
  }
}

/**
 * Integration with Smart Contract Toolkit
 */
class TokenCreationWithMonetization {
  constructor() {
    this.monetizationSystem = new TokenMonetizationSystem();
  }

  /**
   * Create token with monetization
   */
  async createToken(tokenConfig, selectedAddOns = [], paymentMethod = 'shah') {
    try {
      // Calculate total price
      const priceData = await this.monetizationSystem.calculateTotalPrice(selectedAddOns, { tier: tokenConfig?.tier || 'Basic' });
      
      // Process payment
      const paymentResult = await this.monetizationSystem.processPayment(
        paymentMethod,
        priceData,
        tokenConfig
      );
      
      // Save payment data
      await this.monetizationSystem.savePaymentData(
        paymentResult,
        tokenConfig,
        selectedAddOns
      );
      
      // Generate contract with add-ons
      const contractCode = await this.generateContractWithAddOns(tokenConfig, selectedAddOns);
      
      // Deploy contract
      const deployedAddress = await this.deployContract(contractCode, tokenConfig);
      
      return {
        success: true,
        contractAddress: deployedAddress,
        paymentResult: paymentResult,
        priceData: priceData
      };
    } catch (error) {
      console.error('Token creation failed:', error);
      throw error;
    }
  }

  /**
   * Generate contract with selected add-ons
   */
  async generateContractWithAddOns(tokenConfig, selectedAddOns) {
    let contractCode = await this.generateBaseContract(tokenConfig);
    
    // Apply add-ons
    if (selectedAddOns.includes('upgradeableContract')) {
      contractCode = this.addUpgradeableFeature(contractCode);
    }
    
    if (selectedAddOns.includes('airdropTool')) {
      contractCode = this.addAirdropFeature(contractCode);
    }
    
    return contractCode;
  }

  /**
   * Add upgradeable contract feature
   */
  addUpgradeableFeature(contractCode) {
    // Implementation for upgradeable contract
    return contractCode.replace(
      'contract MyToken is ERC20, Ownable, Pausable {',
      'contract MyToken is ERC20, Ownable, Pausable, Initializable, UUPSUpgradeable {'
    );
  }

  /**
   * Add airdrop tool feature
   */
  addAirdropFeature(contractCode) {
    const airdropFunction = `
    function airdrop(address[] memory recipients, uint256[] memory amounts) public onlyOwner {
        require(recipients.length == amounts.length, "Arrays length mismatch");
        for (uint256 i = 0; i < recipients.length; i++) {
            _mint(recipients[i], amounts[i]);
        }
    }`;
    
    return contractCode.replace(
      '}',
      airdropFunction + '\n}'
    );
  }
}

// Export for use in other modules
export {
  TokenMonetizationSystem,
  TokenCreationPaymentModal,
  TokenCreationWithMonetization
};
