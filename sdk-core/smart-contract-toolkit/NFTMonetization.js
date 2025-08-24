/**
 * Shahcoin NFT Creation Monetization System
 * Implements dynamic pricing, premium add-ons, and multiple payment methods for NFT creation
 */

class NFTMonetizationSystem {
  constructor(config = {}) {
    this.config = {
      baseFeeUSD: 25,
      shahSwapAPI: 'https://api.shah.vip/price',
      stripePublicKey: config.stripePublicKey,
      ...config
    };
    
    this.premiumAddOns = {
      customImageUpload: {
        name: 'Custom Image Upload',
        priceUSD: 5,
        description: 'Upload your own artwork (SVG, PNG, GIF)'
      },
      verifiedBadge: {
        name: 'Verified Badge',
        priceUSD: 10,
        description: 'ShahExplorer + Wallet badge'
      },
      unlockableContent: {
        name: 'Unlockable Content',
        priceUSD: 8,
        description: 'Add secret content (text, file, video)'
      },
      tradeLock: {
        name: 'Trade Lock (Soulbound)',
        priceUSD: 6,
        description: 'Prevent transfer – great for proof NFTs'
      },
      boosterTag: {
        name: 'Booster Tag',
        priceUSD: 9,
        description: 'Eligible for staking/APR bonuses'
      }
    };
    
    this.paymentMethods = {
      shah: 'Native SHAH',
      shahToken: 'SHAHtoken (SHI-20)',
      stripe: 'Credit Card (USD)'
    };
  }

  // Tier helpers (per NFT)
  getTierUsd(tier = 'Basic') {
    return tier === 'Premium' ? 39 : tier === 'Pro' ? 25 : 15;
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

  // Upgrade price between NFT tiers (per NFT)
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
    const baseUsd = this.getTierUsd(tier);
    const baseFeeSHAH = baseUsd / shahPrice;
    
    return {
      usd: baseUsd,
      shah: baseFeeSHAH,
      shahPrice: shahPrice,
      tier
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
  async processPayment(paymentMethod, amount, nftConfig) {
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
      const tx = await shahFactory.payNFTCreationFee({
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
            name: 'NFT Creator'
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
  async savePaymentData(paymentResult, nftConfig, selectedAddOns) {
    try {
      const paymentData = {
        timestamp: new Date().toISOString(),
        paymentMethod: paymentResult.method,
        amount: paymentResult.amount,
        nftConfig: nftConfig,
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
   * Validate NFT configuration
   */
  validateNFTConfig(config) {
    const errors = [];
    
    if (!config.name || config.name.length < 1) {
      errors.push('NFT name is required');
    }
    
    if (!config.description || config.description.length < 1) {
      errors.push('NFT description is required');
    }
    
    if (!config.imageURI || config.imageURI.length < 1) {
      errors.push('NFT image URI is required');
    }
    
    if (config.maxSupply && config.maxSupply <= 0) {
      errors.push('Max supply must be greater than 0');
    }
    
    return errors;
  }
}

/**
 * React Component for NFT Creation Payment Modal
 */
class NFTCreationPaymentModal extends React.Component {
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
    
    this.monetizationSystem = new NFTMonetizationSystem({
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
    const { nftConfig, onSuccess, onError } = this.props;
    const { selectedAddOns, paymentMethod, priceData, tier } = this.state;
    
    const cfg = { ...nftConfig, tier };
    
    const errors = this.monetizationSystem.validateNFTConfig(cfg);
    if (errors.length > 0) {
      this.setState({ errors });
      return;
    }
    
    this.setState({ loading: true, errors: [] });
    
    try {
      const paymentResult = await this.monetizationSystem.processPayment(
        paymentMethod,
        priceData,
        cfg
      );
      
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
      ? 'Includes Pro + smart trait filters, unlockable content, bundle mint, spotlight.'
      : tier === 'Pro'
      ? 'Includes Basic + animation/multimedia, verified creator, premium metadata, higher search rank.'
      : 'Upload image, set name/description, standard mint, visible in explorer and wallet.';

    return (
      <div className="tier-section">
        <h3>Tier (Per NFT)</h3>
        <div className="tier-row">
          <select value={tier} onChange={this.handleTierChange}>
            <option value="Basic">Basic — $15 per NFT</option>
            <option value="Pro">Pro — $25 per NFT</option>
            <option value="Premium">Premium — $39 per NFT</option>
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
      <div className="nft-creation-payment-modal">
        <div className="modal-header">
          <h2>NFT Creation Payment</h2>
          <p>Select tier (per NFT), add-ons, and payment method</p>
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
class NFTCreationWithMonetization {
  constructor() {
    this.monetizationSystem = new NFTMonetizationSystem();
  }

  /**
   * Create NFT with monetization
   */
  async createNFT(nftConfig, selectedAddOns = [], paymentMethod = 'shah') {
    try {
      // Calculate total price (tier-aware)
      const priceData = await this.monetizationSystem.calculateTotalPrice(selectedAddOns, { tier: nftConfig?.tier || 'Basic' });
      
      // Process payment
      const paymentResult = await this.monetizationSystem.processPayment(
        paymentMethod,
        priceData,
        nftConfig
      );
      
      // Save payment data
      await this.monetizationSystem.savePaymentData(
        paymentResult,
        nftConfig,
        selectedAddOns
      );
      
      // Generate contract with add-ons
      const contractCode = await this.generateContractWithAddOns(nftConfig, selectedAddOns);
      
      // Deploy contract
      const deployedAddress = await this.deployContract(contractCode, nftConfig);
      
      return {
        success: true,
        contractAddress: deployedAddress,
        paymentResult: paymentResult,
        priceData: priceData
      };
    } catch (error) {
      console.error('NFT creation failed:', error);
      throw error;
    }
  }

  async generateContractWithAddOns(nftConfig, selectedAddOns) {
    let contractCode = await this.generateBaseContract(nftConfig);
    
    // Apply add-ons
    if (selectedAddOns.includes('unlockableContent')) {
      contractCode = this.addUnlockableContentFeature(contractCode);
    }
    
    if (selectedAddOns.includes('tradeLock')) {
      contractCode = this.addTradeLockFeature(contractCode);
    }
    
    if (selectedAddOns.includes('boosterTag')) {
      contractCode = this.addBoosterTagFeature(contractCode);
    }
    
    return contractCode;
  }

  addUnlockableContentFeature(contractCode) {
    const unlockableFunction = `
    mapping(uint256 => string) private unlockableContent;
    
    function setUnlockableContent(uint256 tokenId, string memory content) public onlyOwner {
        unlockableContent[tokenId] = content;
    }
    
    function getUnlockableContent(uint256 tokenId) public view returns (string memory) {
        require(_exists(tokenId), "Token does not exist");
        return unlockableContent[tokenId];
    }`;
    
    return contractCode.replace('}', unlockableFunction + '\n}');
  }

  addTradeLockFeature(contractCode) {
    const tradeLockFunction = `
    mapping(uint256 => bool) private tradeLocked;
    
    function setTradeLock(uint256 tokenId, bool locked) public onlyOwner {
        tradeLocked[tokenId] = locked;
    }
    
    function isTradeLocked(uint256 tokenId) public view returns (bool) {
        return tradeLocked[tokenId];
    }
    
    function _beforeTokenTransfer(
        address from,
        address to,
        uint256 tokenId
    ) internal override {
        require(!tradeLocked[tokenId], "Token is trade locked");
        super._beforeTokenTransfer(from, to, tokenId);
    }`;
    
    return contractCode.replace('}', tradeLockFunction + '\n}');
  }

  addBoosterTagFeature(contractCode) {
    const boosterFunction = `
    mapping(uint256 => bool) private boosterTagged;
    
    function setBoosterTag(uint256 tokenId, bool tagged) public onlyOwner {
        boosterTagged[tokenId] = tagged;
    }
    
    function hasBoosterTag(uint256 tokenId) public view returns (bool) {
        return boosterTagged[tokenId];
    }`;
    
    return contractCode.replace('}', boosterFunction + '\n}');
  }
}

// Export for use in other modules
export {
  NFTMonetizationSystem,
  NFTCreationPaymentModal,
  NFTCreationWithMonetization
};
