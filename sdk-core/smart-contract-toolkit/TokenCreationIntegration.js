/**
 * Token Creation Integration
 * Connects the monetization system with the Smart Contract Toolkit
 */

import { TokenMonetizationSystem } from './TokenMonetization.js';

class TokenCreationIntegration {
  constructor(config = {}) {
    this.monetizationSystem = new TokenMonetizationSystem(config);
    this.isInitialized = false;
    this.currentSession = null;
  }

  /**
   * Initialize the integration
   */
  async initialize() {
    try {
      // Test connection to ShahSwap API
      await this.monetizationSystem.getShahPrice();
      
      this.isInitialized = true;
      console.log('Token Creation Integration initialized successfully');
      
      return true;
    } catch (error) {
      console.error('Failed to initialize Token Creation Integration:', error);
      throw error;
    }
  }

  /**
   * Start a new token creation session
   */
  async startTokenCreationSession(tokenConfig) {
    if (!this.isInitialized) {
      await this.initialize();
    }

    // Validate token configuration
    const errors = this.monetizationSystem.validateTokenConfig(tokenConfig);
    if (errors.length > 0) {
      throw new Error(`Token configuration errors: ${errors.join(', ')}`);
    }

    // Create session
    this.currentSession = {
      id: this.generateSessionId(),
      tokenConfig: tokenConfig,
      selectedAddOns: [],
      paymentMethod: 'shah',
      tier: tokenConfig.tier || 'Basic',
      priceData: null,
      status: 'created',
      createdAt: new Date().toISOString()
    };

    // Calculate initial price
    await this.updateSessionPrice();

    return this.currentSession;
  }

  /**
   * Update selected add-ons for current session
   */
  async updateAddOns(selectedAddOns) {
    if (!this.currentSession) {
      throw new Error('No active token creation session');
    }

    this.currentSession.selectedAddOns = selectedAddOns;
    await this.updateSessionPrice();

    return this.currentSession;
  }

  /**
   * Update payment method for current session
   */
  async updatePaymentMethod(paymentMethod) {
    if (!this.currentSession) {
      throw new Error('No active token creation session');
    }

    this.currentSession.paymentMethod = paymentMethod;
    return this.currentSession;
  }

  /**
   * Update session price based on current configuration
   */
  async updateSessionPrice() {
    if (!this.currentSession) {
      throw new Error('No active token creation session');
    }

    try {
      const priceData = await this.monetizationSystem.calculateTotalPrice(
        this.currentSession.selectedAddOns,
        { tier: this.currentSession.tier || this.currentSession.tokenConfig?.tier || 'Basic' }
      );
      
      this.currentSession.priceData = priceData;
      return priceData;
    } catch (error) {
      console.error('Failed to update session price:', error);
      throw error;
    }
  }

  /**
   * Process payment and create token
   */
  async processTokenCreation() {
    if (!this.currentSession) {
      throw new Error('No active token creation session');
    }

    if (!this.currentSession.priceData) {
      await this.updateSessionPrice();
    }

    try {
      // Update session status
      this.currentSession.status = 'processing_payment';
      
      // Process payment
      const paymentResult = await this.monetizationSystem.processPayment(
        this.currentSession.paymentMethod,
        this.currentSession.priceData,
        this.currentSession.tokenConfig
      );

      // Save payment data
      await this.monetizationSystem.savePaymentData(
        paymentResult,
        this.currentSession.tokenConfig,
        this.currentSession.selectedAddOns
      );

      // Update session status
      this.currentSession.status = 'payment_completed';
      this.currentSession.paymentResult = paymentResult;

      // Generate and deploy contract
      const contractResult = await this.generateAndDeployContract();

      // Update session with final result
      this.currentSession.status = 'completed';
      this.currentSession.contractResult = contractResult;

      return {
        success: true,
        session: this.currentSession,
        contractAddress: contractResult.contractAddress,
        paymentResult: paymentResult
      };

    } catch (error) {
      this.currentSession.status = 'failed';
      this.currentSession.error = error.message;
      
      console.error('Token creation failed:', error);
      throw error;
    }
  }

  /**
   * Generate contract with selected add-ons and deploy
   */
  async generateAndDeployContract() {
    const { tokenConfig, selectedAddOns, tier } = this.currentSession;

    // Generate base contract (carry tier)
    let contractCode = await this.generateBaseContract({ ...tokenConfig, tier: tier || tokenConfig.tier || 'Basic' });

    // Apply add-ons
    if (selectedAddOns.includes('upgradeableContract')) {
      contractCode = this.addUpgradeableFeature(contractCode, tokenConfig);
    }

    if (selectedAddOns.includes('airdropTool')) {
      contractCode = this.addAirdropFeature(contractCode, tokenConfig);
    }

    if (selectedAddOns.includes('customLogo')) {
      contractCode = this.addCustomLogoFeature(contractCode, tokenConfig);
    }

    // Deploy contract
    const deployedAddress = await this.deployContract(contractCode, { ...tokenConfig, tier: tier || tokenConfig.tier || 'Basic' });

    return {
      contractAddress: deployedAddress,
      contractCode: contractCode,
      deployedAt: new Date().toISOString()
    };
  }

  /**
   * Generate base SHI-20 contract with tier metadata
   */
  async generateBaseContract(tokenConfig) {
    const { name, symbol, decimals, maxSupply, initialSupply, tier = 'Basic' } = tokenConfig;

    return `// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/security/Pausable.sol";

contract ${name.replace(/\s+/g, '')} is ERC20, Ownable, Pausable {
    uint256 public maxSupply;
    
    // SHI-20 Metadata
    string public constant TOKEN_STANDARD = "shahcoin.token.standard.v1";
    string public constant TOKEN_TYPE = "SHI-20";
    string public constant CREATED_BY = "ShahFactory";
    bool public verified = false;
    string public tier;
    
    constructor(
        string memory name_,
        string memory symbol_,
        uint256 initialSupply_,
        uint256 maxSupply_,
        string memory tier_
    ) ERC20(name_, symbol_) {
        _mint(msg.sender, initialSupply_ * 10**decimals());
        maxSupply = maxSupply_ * 10**decimals();
        tier = tier_;
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
    
    // Metadata getter
    function getTokenMetadata() public view returns (
        string memory standard,
        string memory type_,
        string memory name_,
        string memory symbol_,
        uint8 decimals_,
        uint256 totalSupply_,
        uint256 maxSupply_,
        string memory createdBy,
        bool verified_,
        string memory tier_
    ) {
        return (
            TOKEN_STANDARD,
            TOKEN_TYPE,
            name(),
            symbol(),
            decimals(),
            totalSupply(),
            maxSupply,
            CREATED_BY,
            verified,
            tier
        );
    }
    
    // Admin function to set verification status
    function setVerified(bool _verified) public onlyOwner {
        verified = _verified;
    }
}`;
  }

  /**
   * Add upgradeable contract feature
   */
  addUpgradeableFeature(contractCode, tokenConfig) {
    const { name } = tokenConfig;
    
    return `// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "@openzeppelin/contracts-upgradeable/token/ERC20/ERC20Upgradeable.sol";
import "@openzeppelin/contracts-upgradeable/access/OwnableUpgradeable.sol";
import "@openzeppelin/contracts-upgradeable/security/PausableUpgradeable.sol";
import "@openzeppelin/contracts-upgradeable/proxy/utils/Initializable.sol";
import "@openzeppelin/contracts-upgradeable/proxy/utils/UUPSUpgradeable.sol";

contract ${name.replace(/\s+/g, '')} is 
    Initializable, 
    ERC20Upgradeable, 
    OwnableUpgradeable, 
    PausableUpgradeable, 
    UUPSUpgradeable 
{
    uint256 public maxSupply;
    
    /// @custom:oz-upgrades-unsafe-allow constructor
    constructor() {
        _disableInitializers();
    }
    
    function initialize(
        string memory name_,
        string memory symbol_,
        uint256 initialSupply_,
        uint256 maxSupply_
    ) public initializer {
        __ERC20_init(name_, symbol_);
        __Ownable_init();
        __Pausable_init();
        __UUPSUpgradeable_init();
        
        _mint(msg.sender, initialSupply_ * 10**decimals());
        maxSupply = maxSupply_ * 10**decimals();
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
    
    function _authorizeUpgrade(address newImplementation) internal override onlyOwner {}
    
    function _beforeTokenTransfer(
        address from,
        address to,
        uint256 amount
    ) internal override whenNotPaused {
        super._beforeTokenTransfer(from, to, amount);
    }
}`;
  }

  /**
   * Add airdrop tool feature
   */
  addAirdropFeature(contractCode, tokenConfig) {
    const airdropFunction = `
    function airdrop(address[] memory recipients, uint256[] memory amounts) public onlyOwner {
        require(recipients.length == amounts.length, "Arrays length mismatch");
        for (uint256 i = 0; i < recipients.length; i++) {
            require(totalSupply() + amounts[i] <= maxSupply, "Exceeds max supply");
            _mint(recipients[i], amounts[i]);
        }
    }
    
    function batchTransfer(address[] memory recipients, uint256[] memory amounts) public {
        require(recipients.length == amounts.length, "Arrays length mismatch");
        for (uint256 i = 0; i < recipients.length; i++) {
            _transfer(msg.sender, recipients[i], amounts[i]);
        }
    }`;
    
    return contractCode.replace('}', airdropFunction + '\n}');
  }

  /**
   * Add custom logo feature
   */
  addCustomLogoFeature(contractCode, tokenConfig) {
    const logoFunction = `
    string public logoURI;
    
    function setLogoURI(string memory _logoURI) public onlyOwner {
        logoURI = _logoURI;
    }`;
    
    return contractCode.replace('}', logoFunction + '\n}');
  }

  /**
   * Deploy contract to Shahcoin blockchain (pass tier in args)
   */
  async deployContract(contractCode, tokenConfig) {
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
      
      // Prepare constructor arguments, including tier
      const constructorArgs = [
        tokenConfig.name,
        tokenConfig.symbol,
        tokenConfig.initialSupply,
        tokenConfig.maxSupply,
        (tokenConfig.tier || 'Basic')
      ];
      
      // Deploy contract
      const tx = await shahFactory.deployToken(
        contractCode,
        constructorArgs,
        { gasLimit: 5000000 }
      );
      
      const receipt = await tx.wait();
      
      // Extract contract address from events
      const deployedAddress = receipt.events.find(
        event => event.event === 'TokenDeployed'
      )?.args?.tokenAddress;
      
      if (!deployedAddress) {
        throw new Error('Failed to extract deployed contract address');
      }
      
      return deployedAddress;
      
    } catch (error) {
      console.error('Contract deployment failed:', error);
      throw new Error(`Deployment failed: ${error.message}`);
    }
  }

  /**
   * Upgrade tier for an existing token: charges USD difference per tier and records the upgrade.
   * currentTier: 'Basic' | 'Pro' | 'Premium'
   * newTier: 'Pro' | 'Premium'
   * paymentMethod: 'shah' | 'shahToken' | 'stripe'
   */
  async upgradeTier({ contractAddress, currentTier = 'Basic', newTier = 'Pro', paymentMethod = 'shah' }) {
    if (!contractAddress) throw new Error('Missing contractAddress');
    if (currentTier === newTier) throw new Error('Already at requested tier');

    // 1) Calculate difference
    const upgradeAmount = await this.monetizationSystem.calculateUpgradePrice(currentTier, newTier);

    // 2) Process payment for the difference
    const paymentResult = await this.monetizationSystem.processPayment(paymentMethod, {
      totalUSD: upgradeAmount.usd,
      totalSHAH: upgradeAmount.shah,
      baseFee: { usd: upgradeAmount.usd, shah: upgradeAmount.shah, shahPrice: upgradeAmount.shahPrice },
      addOns: []
    }, { contractAddress, currentTier, newTier });

    await this.monetizationSystem.savePaymentData(paymentResult, { contractAddress, currentTier, newTier }, []);

    // 3) Record the upgrade in the registry (stub)
    // In production, replace with ShahFactory registry call: setTier(contractAddress, newTier)
    await this.recordTierUpgrade(contractAddress, newTier);

    return { success: true, paymentResult, contractAddress, newTier };
  }

  /**
   * Stub: record tier in off-chain registry or admin index until chain support is added
   */
  async recordTierUpgrade(contractAddress, newTier) {
    try {
      // Placeholder: implement your API call to backend registry
      await fetch('/api/shahfactory/registry/tier', {
        method: 'POST',
        headers: { 'content-type': 'application/json' },
        body: JSON.stringify({ contractAddress, newTier })
      });
    } catch (e) {
      // Do not throw to avoid blocking client; backend can retry
      console.warn('Registry write failed (will rely on explorer refresh):', e?.message);
    }
  }

  /**
   * Get current session
   */
  getCurrentSession() {
    return this.currentSession;
  }

  /**
   * Clear current session
   */
  clearSession() {
    this.currentSession = null;
  }

  /**
   * Get session history
   */
  getSessionHistory() {
    // This would typically fetch from a database
    // For now, return current session if exists
    return this.currentSession ? [this.currentSession] : [];
  }

  /**
   * Generate unique session ID
   */
  generateSessionId() {
    return 'session_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
  }

  /**
   * Validate session state
   */
  validateSession() {
    if (!this.currentSession) {
      return { valid: false, error: 'No active session' };
    }

    if (!this.currentSession.tokenConfig) {
      return { valid: false, error: 'Missing token configuration' };
    }

    const errors = this.monetizationSystem.validateTokenConfig(this.currentSession.tokenConfig);
    if (errors.length > 0) {
      return { valid: false, error: `Configuration errors: ${errors.join(', ')}` };
    }

    return { valid: true };
  }

  /**
   * Get available add-ons
   */
  getAvailableAddOns() {
    return this.monetizationSystem.getPremiumAddOns();
  }

  /**
   * Get available payment methods
   */
  getAvailablePaymentMethods() {
    return this.monetizationSystem.getPaymentMethods();
  }

  /**
   * Estimate gas cost for deployment
   */
  async estimateDeploymentGas(tokenConfig, selectedAddOns = []) {
    try {
      // Generate contract code
      let contractCode = await this.generateBaseContract(tokenConfig);
      
      if (selectedAddOns.includes('upgradeableContract')) {
        contractCode = this.addUpgradeableFeature(contractCode, tokenConfig);
      }
      
      if (selectedAddOns.includes('airdropTool')) {
        contractCode = this.addAirdropFeature(contractCode, tokenConfig);
      }
      
      if (selectedAddOns.includes('customLogo')) {
        contractCode = this.addCustomLogoFeature(contractCode, tokenConfig);
      }

      // Connect to provider
      const provider = new ethers.providers.JsonRpcProvider('https://rpc.shah.vip');
      
      // Estimate gas
      const gasEstimate = await provider.estimateGas({
        data: contractCode,
        value: 0
      });

      // Get current gas price
      const gasPrice = await provider.getGasPrice();
      
      // Calculate cost
      const costInWei = gasEstimate.mul(gasPrice);
      const costInShah = ethers.utils.formatEther(costInWei);
      
      // Get SHAH price for USD conversion
      const shahPrice = await this.monetizationSystem.getShahPrice();
      const costInUSD = parseFloat(costInShah) * shahPrice;

      return {
        gasEstimate: gasEstimate.toString(),
        gasPrice: ethers.utils.formatUnits(gasPrice, 'gwei') + ' gwei',
        costInShah: parseFloat(costInShah).toFixed(6),
        costInUSD: costInUSD.toFixed(2),
        shahPrice: shahPrice
      };
      
    } catch (error) {
      console.error('Gas estimation failed:', error);
      throw new Error(`Gas estimation failed: ${error.message}`);
    }
  }
}

// Export for use in other modules
export { TokenCreationIntegration };
