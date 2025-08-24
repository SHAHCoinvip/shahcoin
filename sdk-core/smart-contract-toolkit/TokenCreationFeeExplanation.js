/**
 * Token Creation Fee Explanation & Confirmation Component
 * WebApp version with comprehensive fee explanation and confirmation flow
 */

import React, { useState, useEffect } from 'react';
import { TokenMonetizationSystem } from './token-monetization.js';
import './token-fee-explanation.css';

class TokenCreationFeeExplanation extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      // Token configuration
      tokenName: '',
      tokenSymbol: '',
      decimals: 18,
      maxSupply: '',
      initialSupply: '',
      
      // Fee calculation
      currentShahPrice: 0,
      totalFeeUSD: 49,
      totalFeeSHAH: 0,
      isCalculating: false,
      
      // Payment method
      paymentMethod: 'shah',
      
      // Add-ons
      selectedAddOns: [],
      
      // Terms and validation
      termsAccepted: false,
      errors: [],
      
      // UI state
      showConfirmation: false,
      isSubmitting: false
    };
    
    this.monetizationSystem = new TokenMonetizationSystem({
      stripePublicKey: process.env.REACT_APP_STRIPE_PUBLIC_KEY
    });
  }

  componentDidMount() {
    this.fetchShahPrice();
  }

  /**
   * Fetch current SHAH price from ShahSwap API
   */
  async fetchShahPrice() {
    this.setState({ isCalculating: true });
    
    try {
      const shahPrice = await this.monetizationSystem.getShahPrice();
      this.setState({ 
        currentShahPrice: shahPrice,
        isCalculating: false 
      });
      this.calculateTotalFee();
    } catch (error) {
      this.setState({ 
        errors: ['Failed to fetch SHAH price. Please try again.'],
        isCalculating: false 
      });
    }
  }

  /**
   * Calculate total fee including add-ons
   */
  calculateTotalFee() {
    const { selectedAddOns, currentShahPrice } = this.state;
    
    if (currentShahPrice <= 0) return;
    
    // Calculate total USD
    let totalUSD = 49; // Base fee
    
    selectedAddOns.forEach(addOn => {
      switch (addOn) {
        case 'customLogo':
          totalUSD += 10;
          break;
        case 'verifiedBadge':
          totalUSD += 20;
          break;
        case 'upgradeableContract':
          totalUSD += 15;
          break;
        case 'airdropTool':
          totalUSD += 8;
          break;
      }
    });
    
    const totalSHAH = totalUSD / currentShahPrice;
    
    this.setState({
      totalFeeUSD: totalUSD,
      totalFeeSHAH: totalSHAH
    });
  }

  /**
   * Handle input changes
   */
  handleInputChange = (field, value) => {
    this.setState({ [field]: value }, () => {
      this.validateInputs();
    });
  }

  /**
   * Handle add-on selection
   */
  handleAddOnToggle = (addOn) => {
    const { selectedAddOns } = this.state;
    const newSelectedAddOns = selectedAddOns.includes(addOn)
      ? selectedAddOns.filter(item => item !== addOn)
      : [...selectedAddOns, addOn];
    
    this.setState({ selectedAddOns: newSelectedAddOns }, () => {
      this.calculateTotalFee();
    });
  }

  /**
   * Handle payment method change
   */
  handlePaymentMethodChange = (method) => {
    this.setState({ paymentMethod: method });
  }

  /**
   * Validate all inputs
   */
  validateInputs() {
    const { tokenName, tokenSymbol, maxSupply, initialSupply, termsAccepted } = this.state;
    const errors = [];
    
    if (!tokenName.trim()) {
      errors.push('Token name is required');
    }
    
    if (!tokenSymbol.trim()) {
      errors.push('Token symbol is required');
    }
    
    if (!maxSupply.trim() || isNaN(parseFloat(maxSupply)) || parseFloat(maxSupply) <= 0) {
      errors.push('Valid max supply is required');
    }
    
    if (!initialSupply.trim() || isNaN(parseFloat(initialSupply)) || parseFloat(initialSupply) <= 0) {
      errors.push('Valid initial supply is required');
    }
    
    if (!termsAccepted) {
      errors.push('You must accept the terms and conditions');
    }
    
    this.setState({ errors });
    return errors.length === 0;
  }

  /**
   * Show confirmation dialog
   */
  handleConfirmCreation = () => {
    if (!this.validateInputs()) {
      return;
    }
    
    this.setState({ showConfirmation: true });
  }

  /**
   * Process token creation
   */
  handleProcessCreation = async () => {
    const { 
      tokenName, tokenSymbol, decimals, maxSupply, initialSupply,
      paymentMethod, selectedAddOns, totalFeeUSD, totalFeeSHAH 
    } = this.state;
    
    this.setState({ isSubmitting: true });
    
    try {
      // Create token configuration
      const tokenConfig = {
        name: tokenName,
        symbol: tokenSymbol,
        decimals: decimals,
        maxSupply: parseFloat(maxSupply),
        initialSupply: parseFloat(initialSupply)
      };
      
      // Calculate price data
      const priceData = await this.monetizationSystem.calculateTotalPrice(selectedAddOns);
      
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
      
      // Show success
      this.setState({ 
        showConfirmation: false, 
        isSubmitting: false 
      });
      
      if (this.props.onSuccess) {
        this.props.onSuccess({
          tokenConfig,
          paymentResult,
          priceData
        });
      }
      
    } catch (error) {
      this.setState({ 
        errors: [error.message],
        isSubmitting: false 
      });
      
      if (this.props.onError) {
        this.props.onError(error);
      }
    }
  }

  /**
   * Cancel confirmation
   */
  handleCancelConfirmation = () => {
    this.setState({ showConfirmation: false });
  }

  render() {
    const {
      tokenName, tokenSymbol, decimals, maxSupply, initialSupply,
      currentShahPrice, totalFeeUSD, totalFeeSHAH, isCalculating,
      paymentMethod, selectedAddOns, termsAccepted, errors,
      showConfirmation, isSubmitting
    } = this.state;

    const addOns = this.monetizationSystem.getPremiumAddOns();
    const paymentMethods = this.monetizationSystem.getPaymentMethods();

    return (
      <div className="token-creation-fee-explanation">
        <div className="fee-explanation-header">
          <h2 title="SHI-20 is the official user token standard on Shahcoin (like ERC-20 on Ethereum).">
            Create Your SHI-20 Token
          </h2>
          <p>Configure your SHI-20 token and review fees before creation</p>
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

        {/* Token Configuration Section */}
        <div className="token-config-section">
          <h3 title="SHI-20 is the official user token standard on Shahcoin (like ERC-20 on Ethereum).">
            SHI-20 Token Configuration
          </h3>
          <div className="form-grid">
            <div className="form-group">
              <label htmlFor="tokenName">Token Name:</label>
              <input
                type="text"
                id="tokenName"
                value={tokenName}
                onChange={(e) => this.handleInputChange('tokenName', e.target.value)}
                placeholder="My Awesome Token"
                className="form-input"
              />
            </div>
            
            <div className="form-group">
              <label htmlFor="tokenSymbol">Token Symbol:</label>
              <input
                type="text"
                id="tokenSymbol"
                value={tokenSymbol}
                onChange={(e) => this.handleInputChange('tokenSymbol', e.target.value.toUpperCase())}
                placeholder="MAT"
                className="form-input"
              />
            </div>
            
            <div className="form-group">
              <label htmlFor="decimals">Decimals:</label>
              <select
                id="decimals"
                value={decimals}
                onChange={(e) => this.handleInputChange('decimals', parseInt(e.target.value))}
                className="form-select"
              >
                {[...Array(19)].map((_, i) => (
                  <option key={i} value={i}>{i}</option>
                ))}
              </select>
            </div>
            
            <div className="form-group">
              <label htmlFor="maxSupply">Max Supply:</label>
              <input
                type="number"
                id="maxSupply"
                value={maxSupply}
                onChange={(e) => this.handleInputChange('maxSupply', e.target.value)}
                placeholder="1000000"
                className="form-input"
              />
            </div>
            
            <div className="form-group">
              <label htmlFor="initialSupply">Initial Supply:</label>
              <input
                type="number"
                id="initialSupply"
                value={initialSupply}
                onChange={(e) => this.handleInputChange('initialSupply', e.target.value)}
                placeholder="100000"
                className="form-input"
              />
            </div>
          </div>
        </div>

        {/* Fee Explanation Section */}
        <div className="fee-explanation-section">
          <h3>Fee Explanation</h3>
          <div className="fee-breakdown">
            <div className="fee-item">
              <div className="fee-label">
                <span>Token Creation Fee:</span>
                <button 
                  className="info-button"
                  title="Token fee is for ShahFactory setup and verification"
                >
                  ⓘ
                </button>
              </div>
              <div className="fee-amount">
                {currentShahPrice > 0 ? (
                  <span>
                    $49 worth of SHAH ({((49 / currentShahPrice)).toFixed(6)} SHAH) - calculated live via ShahSwap
                  </span>
                ) : (
                  <span>$49 worth of SHAH (calculating...)</span>
                )}
              </div>
            </div>
            
            <div className="fee-item">
              <div className="fee-label">
                <span>Blockchain Transaction Fee:</span>
                <button 
                  className="info-button"
                  title="TX fee is standard miner cost to record on blockchain"
                >
                  ⓘ
                </button>
              </div>
              <div className="fee-amount">
                <span>~0.001 SHAH paid to miners</span>
              </div>
            </div>
            
            <div className="fee-total">
              <span>Total Fee:</span>
              <span className="total-amount">
                ${totalFeeUSD.toFixed(2)} ({totalFeeSHAH > 0 ? (totalFeeSHAH + 0.001).toFixed(6) : '0.000000'} SHAH)
              </span>
            </div>
          </div>
          
          <button 
            className="calculate-fee-button"
            onClick={this.fetchShahPrice.bind(this)}
            disabled={isCalculating}
          >
            {isCalculating ? 'Calculating...' : 'Calculate Fee'}
          </button>
        </div>

        {/* Payment Method Section */}
        <div className="payment-method-section">
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

        {/* Premium Add-ons Section */}
        <div className="add-ons-section">
          <h3>Premium Add-ons (Optional)</h3>
          <div className="add-ons-grid">
            {Object.entries(addOns).map(([key, addOn]) => (
              <div key={key} className="add-on-item">
                <label className="add-on-checkbox">
                  <input
                    type="checkbox"
                    checked={selectedAddOns.includes(key)}
                    onChange={() => this.handleAddOnToggle(key)}
                  />
                  <div className="add-on-content">
                    <h4>{addOn.name} (+${addOn.priceUSD})</h4>
                    <p>{addOn.description}</p>
                  </div>
                </label>
              </div>
            ))}
          </div>
        </div>

        {/* Terms and Conditions */}
        <div className="terms-section">
          <label className="terms-checkbox">
            <input
              type="checkbox"
              checked={termsAccepted}
              onChange={(e) => this.handleInputChange('termsAccepted', e.target.checked)}
            />
            <span>I understand this is a permanent blockchain action</span>
          </label>
        </div>

        {/* Action Buttons */}
        <div className="action-buttons">
          <button 
            className="btn-secondary"
            onClick={this.props.onCancel}
          >
            Cancel
          </button>
                      <button 
              className="btn-primary"
              onClick={this.handleConfirmCreation}
              disabled={errors.length > 0 || !termsAccepted}
            >
              Confirm & Create SHI-20 Token
            </button>
        </div>

        {/* Confirmation Modal */}
        {showConfirmation && (
          <div className="confirmation-modal-overlay">
            <div className="confirmation-modal">
              <div className="confirmation-header">
                <h3>Confirm Token Creation</h3>
              </div>
              
              <div className="confirmation-content">
                                 <div className="confirmation-summary">
                   <h4>SHI-20 Token Summary:</h4>
                  <div className="summary-item">
                    <span>Name:</span>
                    <span>{tokenName}</span>
                  </div>
                  <div className="summary-item">
                    <span>Symbol:</span>
                    <span>{tokenSymbol}</span>
                  </div>
                  <div className="summary-item">
                    <span>Decimals:</span>
                    <span>{decimals}</span>
                  </div>
                  <div className="summary-item">
                    <span>Max Supply:</span>
                    <span>{maxSupply}</span>
                  </div>
                  <div className="summary-item">
                    <span>Initial Supply:</span>
                    <span>{initialSupply}</span>
                  </div>
                </div>
                
                <div className="confirmation-fees">
                  <h4>Fee Summary:</h4>
                  <div className="summary-item">
                    <span>Payment Method:</span>
                    <span>{paymentMethods[paymentMethod]}</span>
                  </div>
                  <div className="summary-item">
                    <span>Total Fee:</span>
                    <span>${totalFeeUSD.toFixed(2)} ({totalFeeSHAH > 0 ? (totalFeeSHAH + 0.001).toFixed(6) : '0.000000'} SHAH)</span>
                  </div>
                  {selectedAddOns.length > 0 && (
                    <div className="summary-item">
                      <span>Add-ons:</span>
                      <span>{selectedAddOns.map(addOn => addOns[addOn].name).join(', ')}</span>
                    </div>
                  )}
                </div>
                
                                 <div className="confirmation-warning">
                   <p>⚠️ This action will create a permanent SHI-20 token on the Shahcoin blockchain.</p>
                 </div>
              </div>
              
              <div className="confirmation-actions">
                <button 
                  className="btn-secondary"
                  onClick={this.handleCancelConfirmation}
                  disabled={isSubmitting}
                >
                  Cancel
                </button>
                                 <button 
                   className="btn-primary"
                   onClick={this.handleProcessCreation}
                   disabled={isSubmitting}
                 >
                   {isSubmitting ? 'Creating SHI-20 Token...' : 'Confirm & Create SHI-20 Token'}
                 </button>
              </div>
            </div>
          </div>
        )}
      </div>
    );
  }
}

export default TokenCreationFeeExplanation;
