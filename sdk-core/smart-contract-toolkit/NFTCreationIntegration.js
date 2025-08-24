/**
 * NFT Creation Integration
 * Connects the NFT monetization system with the Smart Contract Toolkit
 */

import { NFTMonetizationSystem } from './NFTMonetization.js';

class NFTCreationIntegration {
  constructor(config = {}) {
    this.monetizationSystem = new NFTMonetizationSystem(config);
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
      console.log('NFT Creation Integration initialized successfully');
      
      return true;
    } catch (error) {
      console.error('Failed to initialize NFT Creation Integration:', error);
      throw error;
    }
  }

  /**
   * Start a new NFT creation session
   */
  async startNFTCreationSession(nftConfig) {
    if (!this.isInitialized) {
      await this.initialize();
    }

    // Validate NFT configuration
    const errors = this.monetizationSystem.validateNFTConfig(nftConfig);
    if (errors.length > 0) {
      throw new Error(`NFT configuration errors: ${errors.join(', ')}`);
    }

    // Create session
    this.currentSession = {
      id: this.generateSessionId(),
      nftConfig: { ...nftConfig, tier: (nftConfig && nftConfig.tier) ? nftConfig.tier : 'Basic' },
      selectedAddOns: [],
      paymentMethod: 'shah',
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
      throw new Error('No active NFT creation session');
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
      throw new Error('No active NFT creation session');
    }

    this.currentSession.paymentMethod = paymentMethod;
    return this.currentSession;
  }

  /**
   * Update session price based on current configuration
   */
  async updateSessionPrice() {
    if (!this.currentSession) {
      throw new Error('No active NFT creation session');
    }

    try {
      const priceData = await this.monetizationSystem.calculateTotalPrice(
        this.currentSession.selectedAddOns,
        { tier: (this.currentSession.nftConfig && this.currentSession.nftConfig.tier) ? this.currentSession.nftConfig.tier : 'Basic' }
      );
      
      this.currentSession.priceData = priceData;
      return priceData;
    } catch (error) {
      console.error('Failed to update session price:', error);
      throw error;
    }
  }

  /**
   * Process payment and create NFT
   */
  async processNFTCreation() {
    if (!this.currentSession) {
      throw new Error('No active NFT creation session');
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
        this.currentSession.nftConfig
      );

      // Save payment data
      await this.monetizationSystem.savePaymentData(
        paymentResult,
        this.currentSession.nftConfig,
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
      
      console.error('NFT creation failed:', error);
      throw error;
    }
  }

  /**
   * Generate contract with selected add-ons and deploy
   */
  async generateAndDeployContract() {
    const { nftConfig, selectedAddOns } = this.currentSession;

    // Generate base contract
    let contractCode = await this.generateBaseContract(nftConfig);

    // Apply add-ons
    if (selectedAddOns.includes('unlockableContent')) {
      contractCode = this.addUnlockableContentFeature(contractCode, nftConfig);
    }

    if (selectedAddOns.includes('tradeLock')) {
      contractCode = this.addTradeLockFeature(contractCode, nftConfig);
    }

    if (selectedAddOns.includes('boosterTag')) {
      contractCode = this.addBoosterTagFeature(contractCode, nftConfig);
    }

    if (selectedAddOns.includes('customImageUpload')) {
      contractCode = this.addCustomImageFeature(contractCode, nftConfig);
    }

    if (selectedAddOns.includes('verifiedBadge')) {
      contractCode = this.addVerifiedBadgeFeature(contractCode, nftConfig);
    }

    // Deploy contract
    const deployedAddress = await this.deployContract(contractCode, nftConfig);

    return {
      contractAddress: deployedAddress,
      contractCode: contractCode,
      deployedAt: new Date().toISOString()
    };
  }

  /**
   * Generate base SHI-30 (ERC-721) contract with tier metadata
   */
  async generateBaseContract(nftConfig) {
    const { name, symbol, baseURI, tier } = nftConfig;

    return `// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "@openzeppelin/contracts/token/ERC721/ERC721.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/utils/Counters.sol";

// SHI-30 NFT (Shahcoin Standard)
contract ${name.replace(/\s+/g, '')} is ERC721, Ownable {
    using Counters for Counters.Counter;
    
    Counters.Counter private _tokenIds;
    string private _baseTokenURI;
    string public collectionTier; // Basic | Pro | Premium
    
    constructor(
        string memory name_,
        string memory symbol_,
        string memory baseURI_,
        string memory tier_
    ) ERC721(name_, symbol_) {
        _baseTokenURI = baseURI_;
        collectionTier = tier_;
    }
    
    function setCollectionTier(string memory newTier) public onlyOwner {
        collectionTier = newTier;
    }
    
    function getCollectionMetadata() external view returns (
        string memory nameOut,
        string memory symbolOut,
        string memory baseURIOut,
        string memory tierOut
    ) {
        nameOut = name();
        symbolOut = symbol();
        baseURIOut = _baseTokenURI;
        tierOut = collectionTier;
    }
    
    function mint(address to) public onlyOwner returns (uint256) {
        _tokenIds.increment();
        uint256 newTokenId = _tokenIds.current();
        _mint(to, newTokenId);
        return newTokenId;
    }
    
    function _baseURI() internal view override returns (string memory) {
        return _baseTokenURI;
    }
    
    function setBaseURI(string memory baseURI_) public onlyOwner {
        _baseTokenURI = baseURI_;
    }
}`;
  }

  /**
   * Add unlockable content feature
   */
  addUnlockableContentFeature(contractCode, nftConfig) {
    const unlockableFunction = `
    mapping(uint256 => string) private unlockableContent;
    
    function setUnlockableContent(uint256 tokenId, string memory content) public onlyOwner {
        require(_exists(tokenId), "Token does not exist");
        unlockableContent[tokenId] = content;
    }
    
    function getUnlockableContent(uint256 tokenId) public view returns (string memory) {
        require(_exists(tokenId), "Token does not exist");
        return unlockableContent[tokenId];
    }
    
    function hasUnlockableContent(uint256 tokenId) public view returns (bool) {
        require(_exists(tokenId), "Token does not exist");
        return bytes(unlockableContent[tokenId]).length > 0;
    }`;
    
    return contractCode.replace('}', unlockableFunction + '\n}');
  }

  /**
   * Add trade lock (soulbound) feature
   */
  addTradeLockFeature(contractCode, nftConfig) {
    const tradeLockFunction = `
    mapping(uint256 => bool) private tradeLocked;
    
    function setTradeLock(uint256 tokenId, bool locked) public onlyOwner {
        require(_exists(tokenId), "Token does not exist");
        tradeLocked[tokenId] = locked;
    }
    
    function isTradeLocked(uint256 tokenId) public view returns (bool) {
        require(_exists(tokenId), "Token does not exist");
        return tradeLocked[tokenId];
    }
    
    function _beforeTokenTransfer(
        address from,
        address to,
        uint256 tokenId
    ) internal override {
        require(!tradeLocked[tokenId], "Token is trade locked (soulbound)");
        super._beforeTokenTransfer(from, to, tokenId);
    }`;
    
    return contractCode.replace('}', tradeLockFunction + '\n}');
  }

  /**
   * Add booster tag feature
   */
  addBoosterTagFeature(contractCode, nftConfig) {
    const boosterFunction = `
    mapping(uint256 => bool) private boosterTagged;
    
    function setBoosterTag(uint256 tokenId, bool tagged) public onlyOwner {
        require(_exists(tokenId), "Token does not exist");
        boosterTagged[tokenId] = tagged;
    }
    
    function hasBoosterTag(uint256 tokenId) public view returns (bool) {
        require(_exists(tokenId), "Token does not exist");
        return boosterTagged[tokenId];
    }
    
    function getBoosterMultiplier(uint256 tokenId) public view returns (uint256) {
        require(_exists(tokenId), "Token does not exist");
        return boosterTagged[tokenId] ? 2 : 1; // 2x multiplier for staking/APR
    }`;
    
    return contractCode.replace('}', boosterFunction + '\n}');
  }

  /**
   * Add custom image upload feature
   */
  addCustomImageFeature(contractCode, nftConfig) {
    const customImageFunction = `
    mapping(uint256 => string) private customImageURIs;
    
    function setCustomImageURI(uint256 tokenId, string memory imageURI) public onlyOwner {
        require(_exists(tokenId), "Token does not exist");
        customImageURIs[tokenId] = imageURI;
    }
    
    function getCustomImageURI(uint256 tokenId) public view returns (string memory) {
        require(_exists(tokenId), "Token does not exist");
        return customImageURIs[tokenId];
    }
    
    function hasCustomImage(uint256 tokenId) public view returns (bool) {
        require(_exists(tokenId), "Token does not exist");
        return bytes(customImageURIs[tokenId]).length > 0;
    }`;
    
    return contractCode.replace('}', customImageFunction + '\n}');
  }

  /**
   * Add verified badge feature
   */
  addVerifiedBadgeFeature(contractCode, nftConfig) {
    const verifiedFunction = `
    mapping(uint256 => bool) private verifiedBadges;
    
    function setVerifiedBadge(uint256 tokenId, bool verified) public onlyOwner {
        require(_exists(tokenId), "Token does not exist");
        verifiedBadges[tokenId] = verified;
    }
    
    function isVerified(uint256 tokenId) public view returns (bool) {
        require(_exists(tokenId), "Token does not exist");
        return verifiedBadges[tokenId];
    }
    
    function getVerifiedBadgeURI(uint256 tokenId) public view returns (string memory) {
        require(_exists(tokenId), "Token does not exist");
        return verifiedBadges[tokenId] ? "https://shah.vip/badges/verified.svg" : "";
    }`;
    
    return contractCode.replace('}', verifiedFunction + '\n}');
  }

  /**
   * Deploy contract to Shahcoin blockchain
   */
  async deployContract(contractCode, nftConfig) {
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
      
      // Prepare constructor arguments
      const constructorArgs = [
        nftConfig.name,
        nftConfig.symbol,
        nftConfig.baseURI || '',
        (nftConfig && nftConfig.tier) ? nftConfig.tier : 'Basic'
      ];
      
      // Deploy contract
      const tx = await shahFactory.deployNFT(
        contractCode,
        constructorArgs,
        { gasLimit: 5000000 }
      );
      
      const receipt = await tx.wait();
      
      // Extract contract address from events
      const deployedAddress = receipt.events.find(
        event => event.event === 'NFTDeployed'
      )?.args?.nftAddress;
      
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
    return 'nft_session_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
  }

  /**
   * Validate session state
   */
  validateSession() {
    if (!this.currentSession) {
      return { valid: false, error: 'No active session' };
    }

    if (!this.currentSession.nftConfig) {
      return { valid: false, error: 'Missing NFT configuration' };
    }

    const errors = this.monetizationSystem.validateNFTConfig(this.currentSession.nftConfig);
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
  async estimateDeploymentGas(nftConfig, selectedAddOns = []) {
    try {
      // Generate contract code
      let contractCode = await this.generateBaseContract(nftConfig);
      
      if (selectedAddOns.includes('unlockableContent')) {
        contractCode = this.addUnlockableContentFeature(contractCode, nftConfig);
      }
      
      if (selectedAddOns.includes('tradeLock')) {
        contractCode = this.addTradeLockFeature(contractCode, nftConfig);
      }
      
      if (selectedAddOns.includes('boosterTag')) {
        contractCode = this.addBoosterTagFeature(contractCode, nftConfig);
      }
      
      if (selectedAddOns.includes('customImageUpload')) {
        contractCode = this.addCustomImageFeature(contractCode, nftConfig);
      }
      
      if (selectedAddOns.includes('verifiedBadge')) {
        contractCode = this.addVerifiedBadgeFeature(contractCode, nftConfig);
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

  /**
   * Mint NFT with metadata
   */
  async mintNFT(tokenId, metadata) {
    try {
      const provider = new ethers.providers.JsonRpcProvider('https://rpc.shah.vip');
      const signer = provider.getSigner();
      
      // Get NFT contract
      const nftContract = new ethers.Contract(
        this.currentSession.contractResult.contractAddress,
        NFT_ABI,
        signer
      );
      
      // Mint NFT
      const tx = await nftContract.mint(metadata.owner || await signer.getAddress());
      const receipt = await tx.wait();
      
      // Set metadata if provided
      if (metadata.unlockableContent) {
        await nftContract.setUnlockableContent(tokenId, metadata.unlockableContent);
      }
      
      if (metadata.customImageURI) {
        await nftContract.setCustomImageURI(tokenId, metadata.customImageURI);
      }
      
      if (metadata.verifiedBadge) {
        await nftContract.setVerifiedBadge(tokenId, true);
      }
      
      if (metadata.tradeLocked) {
        await nftContract.setTradeLock(tokenId, true);
      }
      
      if (metadata.boosterTag) {
        await nftContract.setBoosterTag(tokenId, true);
      }
      
      return {
        success: true,
        tokenId: tokenId,
        txHash: receipt.transactionHash,
        metadata: metadata
      };
      
    } catch (error) {
      console.error('NFT minting failed:', error);
      throw new Error(`Minting failed: ${error.message}`);
    }
  }

  /**
   * Upgrade collection tier (per collection), charges price delta via monetization
   */
  async upgradeTier({ contractAddress, currentTier, newTier, paymentMethod = 'shah' }) {
    try {
      // Calculate difference
      const priceDiff = this.monetizationSystem.calculateUpgradePrice(currentTier, newTier);
      if (priceDiff.usd <= 0) {
        return { success: true, message: 'No upgrade needed' };
      }

      // Build a synthetic priceData for the difference
      const priceData = {
        usdTotal: priceDiff.usd,
        shahTotal: priceDiff.shah,
        tier: newTier
      };

      // Process payment for the difference
      const paymentResult = await this.monetizationSystem.processPayment(
        paymentMethod,
        priceData,
        { type: 'nftTierUpgrade', contractAddress, currentTier, newTier }
      );

      await this.monetizationSystem.savePaymentData(paymentResult, { contractAddress, currentTier, newTier }, []);

      // Attempt to update on-chain if function exists
      try {
        const provider = new ethers.providers.JsonRpcProvider('https://rpc.shah.vip');
        const signer = provider.getSigner();
        const nftContract = new ethers.Contract(contractAddress, NFT_ABI, signer);
        if (nftContract.functions && nftContract.functions.setCollectionTier) {
          const tx = await nftContract.setCollectionTier(newTier);
          await tx.wait();
        }
      } catch (e) {
        console.warn('On-chain tier update not available, falling back to registry:', e.message);
      }

      // Record upgrade in registry (stub)
      await this.recordNFTTierUpgrade(contractAddress, newTier);

      return { success: true, paymentResult };
    } catch (error) {
      console.error('NFT tier upgrade failed:', error);
      throw error;
    }
  }

  async recordNFTTierUpgrade(contractAddress, newTier) {
    // TODO: integrate with ShahFactory registry or backend API
    console.log('Recorded NFT tier upgrade', { contractAddress, newTier });
    return true;
  }
}

// Export for use in other modules
export { NFTCreationIntegration };
