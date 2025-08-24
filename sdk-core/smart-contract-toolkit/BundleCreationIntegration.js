/**
 * Bundle Creation Integration
 * Connects the bundle monetization system with the Smart Contract Toolkit
 */

import { BundleMonetizationSystem } from './BundleMonetization.js';
import { TokenCreationIntegration } from './TokenCreationIntegration.js';
import { NFTCreationIntegration } from './NFTCreationIntegration.js';

class BundleCreationIntegration {
  constructor(config = {}) {
    this.bundleMonetization = new BundleMonetizationSystem(config);
    this.tokenIntegration = new TokenCreationIntegration(config);
    this.nftIntegration = new NFTCreationIntegration(config);
    this.isInitialized = false;
    this.currentSession = null;
  }

  /**
   * Initialize the integration
   */
  async initialize() {
    try {
      // Initialize all systems
      await this.bundleMonetization.initialize();
      await this.tokenIntegration.initialize();
      await this.nftIntegration.initialize();
      
      this.isInitialized = true;
      console.log('Bundle Creation Integration initialized successfully');
      
      return true;
    } catch (error) {
      console.error('Failed to initialize Bundle Creation Integration:', error);
      throw error;
    }
  }

  /**
   * Start a new bundle creation session
   */
  async startBundleCreationSession(tokenConfig, nftConfig) {
    if (!this.isInitialized) {
      await this.initialize();
    }

    // Validate bundle configuration
    const errors = this.bundleMonetization.validateBundleConfig(tokenConfig, nftConfig);
    if (errors.length > 0) {
      throw new Error(`Bundle configuration errors: ${errors.join(', ')}`);
    }

    // Create session
    this.currentSession = {
      id: this.generateSessionId(),
      tokenConfig: tokenConfig,
      nftConfig: nftConfig,
      selectedNFTAddOns: [],
      paymentMethod: 'shah',
      bundlePrice: null,
      status: 'created',
      createdAt: new Date().toISOString()
    };

    // Calculate initial bundle price
    await this.updateBundlePrice();

    return this.currentSession;
  }

  /**
   * Update selected NFT add-ons for current session
   */
  async updateNFTAddOns(selectedNFTAddOns) {
    if (!this.currentSession) {
      throw new Error('No active bundle creation session');
    }

    this.currentSession.selectedNFTAddOns = selectedNFTAddOns;
    await this.updateBundlePrice();

    return this.currentSession;
  }

  /**
   * Update payment method for current session
   */
  async updatePaymentMethod(paymentMethod) {
    if (!this.currentSession) {
      throw new Error('No active bundle creation session');
    }

    this.currentSession.paymentMethod = paymentMethod;
    return this.currentSession;
  }

  /**
   * Update bundle price based on current configuration
   */
  async updateBundlePrice() {
    if (!this.currentSession) {
      throw new Error('No active bundle creation session');
    }

    try {
      const bundlePrice = await this.bundleMonetization.calculateBundlePrice(
        this.currentSession.tokenConfig,
        this.currentSession.nftConfig,
        this.currentSession.selectedNFTAddOns
      );
      
      this.currentSession.bundlePrice = bundlePrice;
      return bundlePrice;
    } catch (error) {
      console.error('Failed to update bundle price:', error);
      throw error;
    }
  }

  /**
   * Process payment and create bundle
   */
  async processBundleCreation() {
    if (!this.currentSession) {
      throw new Error('No active bundle creation session');
    }

    if (!this.currentSession.bundlePrice) {
      await this.updateBundlePrice();
    }

    try {
      // Update session status
      this.currentSession.status = 'processing_payment';
      
      // Process bundle payment
      const paymentResult = await this.bundleMonetization.processBundlePayment(
        this.currentSession.paymentMethod,
        this.currentSession.bundlePrice,
        this.currentSession.tokenConfig,
        this.currentSession.nftConfig,
        this.currentSession.selectedNFTAddOns
      );

      // Save payment data
      await this.bundleMonetization.saveBundlePaymentData(
        paymentResult,
        this.currentSession.tokenConfig,
        this.currentSession.nftConfig,
        this.currentSession.selectedNFTAddOns
      );

      // Update session status
      this.currentSession.status = 'payment_completed';
      this.currentSession.paymentResult = paymentResult;

      // Generate and deploy contracts
      const contractResult = await this.generateAndDeployBundleContracts();

      // Update session with final result
      this.currentSession.status = 'completed';
      this.currentSession.contractResult = contractResult;

      return {
        success: true,
        session: this.currentSession,
        tokenAddress: contractResult.tokenAddress,
        nftAddress: contractResult.nftAddress,
        paymentResult: paymentResult
      };

    } catch (error) {
      this.currentSession.status = 'failed';
      this.currentSession.error = error.message;
      
      console.error('Bundle creation failed:', error);
      throw error;
    }
  }

  /**
   * Generate and deploy bundle contracts
   */
  async generateAndDeployBundleContracts() {
    const { tokenConfig, nftConfig, selectedNFTAddOns } = this.currentSession;

    try {
      // Deploy token first
      console.log('Deploying token contract...');
      const tokenResult = await this.deployTokenContract(tokenConfig);
      
      // Deploy NFT second
      console.log('Deploying NFT contract...');
      const nftResult = await this.deployNFTContract(nftConfig, selectedNFTAddOns);

      return {
        tokenAddress: tokenResult.contractAddress,
        nftAddress: nftResult.contractAddress,
        tokenDeployment: tokenResult,
        nftDeployment: nftResult,
        deployedAt: new Date().toISOString()
      };

    } catch (error) {
      console.error('Bundle contract deployment failed:', error);
      
      // If token was deployed but NFT failed, we need to handle this
      if (error.tokenDeployed) {
        console.warn('Token was deployed but NFT failed. Token address:', error.tokenAddress);
        // Could implement rollback logic here if needed
      }
      
      throw error;
    }
  }

  /**
   * Deploy token contract
   */
  async deployTokenContract(tokenConfig) {
    try {
      // Use the token integration to deploy
      const tokenSession = await this.tokenIntegration.startTokenCreationSession(tokenConfig);
      const tokenResult = await this.tokenIntegration.processTokenCreation();
      
      return {
        contractAddress: tokenResult.contractAddress,
        contractCode: tokenResult.contractCode,
        deployedAt: new Date().toISOString()
      };
    } catch (error) {
      console.error('Token deployment failed:', error);
      throw new Error(`Token deployment failed: ${error.message}`);
    }
  }

  /**
   * Deploy NFT contract
   */
  async deployNFTContract(nftConfig, selectedNFTAddOns) {
    try {
      // Use the NFT integration to deploy
      const nftSession = await this.nftIntegration.startNFTCreationSession(nftConfig);
      await this.nftIntegration.updateAddOns(selectedNFTAddOns);
      const nftResult = await this.nftIntegration.processNFTCreation();
      
      return {
        contractAddress: nftResult.contractAddress,
        contractCode: nftResult.contractCode,
        deployedAt: new Date().toISOString()
      };
    } catch (error) {
      console.error('NFT deployment failed:', error);
      throw new Error(`NFT deployment failed: ${error.message}`);
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
    return 'bundle_session_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
  }

  /**
   * Validate session state
   */
  validateSession() {
    if (!this.currentSession) {
      return { valid: false, error: 'No active session' };
    }

    if (!this.currentSession.tokenConfig || !this.currentSession.nftConfig) {
      return { valid: false, error: 'Missing token or NFT configuration' };
    }

    const errors = this.bundleMonetization.validateBundleConfig(
      this.currentSession.tokenConfig,
      this.currentSession.nftConfig
    );
    if (errors.length > 0) {
      return { valid: false, error: `Configuration errors: ${errors.join(', ')}` };
    }

    return { valid: true };
  }

  /**
   * Get available NFT add-ons
   */
  getAvailableNFTAddOns() {
    return this.bundleMonetization.getAvailableNFTAddOns();
  }

  /**
   * Get available payment methods
   */
  getAvailablePaymentMethods() {
    return this.bundleMonetization.getPaymentMethods();
  }

  /**
   * Estimate gas cost for bundle deployment
   */
  async estimateBundleDeploymentGas(tokenConfig, nftConfig, selectedNFTAddOns = []) {
    try {
      return await this.bundleMonetization.estimateBundleDeploymentGas(
        tokenConfig,
        nftConfig,
        selectedNFTAddOns
      );
    } catch (error) {
      console.error('Bundle gas estimation failed:', error);
      throw error;
    }
  }

  /**
   * Check bundle eligibility
   */
  async checkBundleEligibility(tokenConfig, nftConfig) {
    try {
      // Basic eligibility checks
      const eligibility = {
        eligible: true,
        reasons: []
      };

      // Check if token config is valid
      const tokenErrors = this.tokenIntegration.monetizationSystem.validateTokenConfig(tokenConfig);
      if (tokenErrors.length > 0) {
        eligibility.eligible = false;
        eligibility.reasons.push(`Token: ${tokenErrors.join(', ')}`);
      }

      // Check if NFT config is valid
      const nftErrors = this.nftIntegration.monetizationSystem.validateNFTConfig(nftConfig);
      if (nftErrors.length > 0) {
        eligibility.eligible = false;
        eligibility.reasons.push(`NFT: ${nftErrors.join(', ')}`);
      }

      // Check if user has sufficient balance (if using SHAH payment)
      try {
        const bundlePrice = await this.bundleMonetization.calculateBundlePrice(
          tokenConfig,
          nftConfig,
          []
        );
        
        // This would typically check actual wallet balance
        // For now, just return eligible
        eligibility.bundlePrice = bundlePrice;
      } catch (error) {
        eligibility.eligible = false;
        eligibility.reasons.push('Unable to calculate bundle price');
      }

      return eligibility;
    } catch (error) {
      console.error('Bundle eligibility check failed:', error);
      throw error;
    }
  }

  /**
   * Get bundle analytics
   */
  async getBundleAnalytics(filters = {}) {
    try {
      // This would typically fetch from a database
      // For now, return mock analytics
      return {
        totalBundles: 150,
        totalRevenue: 8850, // $59 * 150 bundles
        totalSavings: 2250, // $15 * 150 bundles
        averageSavings: 15,
        popularAddOns: [
          { name: 'Custom Image Upload', count: 120, percentage: 80 },
          { name: 'Verified Badge', count: 95, percentage: 63 },
          { name: 'Unlockable Content', count: 75, percentage: 50 },
          { name: 'Trade Lock', count: 45, percentage: 30 },
          { name: 'Booster Tag', count: 60, percentage: 40 }
        ],
        paymentMethods: {
          shah: { count: 90, percentage: 60 },
          shahToken: { count: 30, percentage: 20 },
          stripe: { count: 30, percentage: 20 }
        },
        conversionRate: 0.35, // 35% of users choose bundle over individual
        revenueIncrease: 0.25 // 25% increase in revenue due to bundles
      };
    } catch (error) {
      console.error('Failed to get bundle analytics:', error);
      throw error;
    }
  }

  /**
   * Get bundle recommendations
   */
  async getBundleRecommendations(userId) {
    try {
      // This would typically analyze user behavior and preferences
      // For now, return default recommendations
      return [
        {
          name: 'Starter Bundle',
          description: 'Perfect for beginners - basic token and NFT',
          savings: 15,
          recommended: true
        },
        {
          name: 'Premium Bundle',
          description: 'Advanced features with all NFT add-ons',
          savings: 15,
          recommended: false
        },
        {
          name: 'Gaming Bundle',
          description: 'Token + NFT optimized for gaming projects',
          savings: 15,
          recommended: false
        }
      ];
    } catch (error) {
      console.error('Failed to get bundle recommendations:', error);
      throw error;
    }
  }

  /**
   * Get bundle system status
   */
  async getBundleSystemStatus() {
    try {
      return {
        bundleMonetization: this.bundleMonetization.isInitialized,
        tokenIntegration: this.tokenIntegration.isInitialized,
        nftIntegration: this.nftIntegration.isInitialized,
        currentSession: this.currentSession ? this.currentSession.status : null,
        timestamp: new Date().toISOString()
      };
    } catch (error) {
      console.error('Failed to get bundle system status:', error);
      throw error;
    }
  }

  /**
   * Rollback bundle creation (if needed)
   */
  async rollbackBundleCreation() {
    if (!this.currentSession) {
      throw new Error('No active session to rollback');
    }

    try {
      // This would implement rollback logic
      // For now, just clear the session
      this.clearSession();
      
      return {
        success: true,
        message: 'Bundle creation rolled back successfully'
      };
    } catch (error) {
      console.error('Bundle rollback failed:', error);
      throw error;
    }
  }
}

// Export for use in other modules
export { BundleCreationIntegration };
