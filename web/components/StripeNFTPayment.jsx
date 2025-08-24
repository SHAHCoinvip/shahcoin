import React, { useState, useEffect, useCallback } from 'react';
import { FaCreditCard, FaApple, FaGoogle, FaShieldAlt, FaExchangeAlt, FaCheckCircle } from 'react-icons/fa';
import { useStripePayments } from '../sdk-core/marketplace-api/stripe-payments.js';
import { usePriceOracle } from '../sdk-core/marketplace-api/price-oracle.js';
import './StripeNFTPayment.css';

const StripeNFTPayment = ({ 
  nft, 
  buyerAddress, 
  onPaymentSuccess, 
  onPaymentCancel, 
  onPaymentError 
}) => {
  const [paymentMethod, setPaymentMethod] = useState('card');
  const [termsAccepted, setTermsAccepted] = useState(false);
  const [isProcessing, setIsProcessing] = useState(false);
  const [processingStep, setProcessingStep] = useState('');
  const [showPaymentDialog, setShowPaymentDialog] = useState(false);

  const { 
    createPayment, 
    checkPaymentStatus, 
    isLoading, 
    error, 
    isStripeEnabled 
  } = useStripePayments();

  const { price: shahPrice, satoshisToUSD } = usePriceOracle();

  const [usdPrice, setUsdPrice] = useState(0);

  // Calculate USD price when component mounts or price changes
  useEffect(() => {
    const calculateUSDPrice = async () => {
      if (nft.listedPriceSatoshis && satoshisToUSD) {
        const usd = await satoshisToUSD(nft.listedPriceSatoshis);
        setUsdPrice(usd);
      }
    };
    calculateUSDPrice();
  }, [nft.listedPriceSatoshis, satoshisToUSD]);

  // Check if Stripe is enabled for this NFT
  const stripeEnabled = isStripeEnabled(nft);

  const handlePayment = useCallback(async () => {
    if (!termsAccepted) {
      onPaymentError?.('Please accept the terms and conditions');
      return;
    }

    setIsProcessing(true);
    setProcessingStep('Creating payment session...');

    try {
      // Create Stripe checkout session
      const session = await createPayment(nft, buyerAddress, {
        successUrl: `${window.location.origin}/marketplace/purchase/success?session_id={CHECKOUT_SESSION_ID}`,
        cancelUrl: `${window.location.origin}/marketplace/purchase/cancel`,
        customerEmail: '', // Could be passed as prop
        paymentMethod: paymentMethod
      });

      setProcessingStep('Redirecting to payment...');
      
      // The createPayment function will handle the redirect to Stripe
      // If we reach here, it means the redirect was successful
      
    } catch (err) {
      setIsProcessing(false);
      setProcessingStep('');
      onPaymentError?.(err.message || 'Payment failed');
    }
  }, [nft, buyerAddress, termsAccepted, paymentMethod, createPayment, onPaymentError]);

  const handleCancel = () => {
    if (!isProcessing) {
      onPaymentCancel?.();
    }
  };

  if (!stripeEnabled) {
    return (
      <div className="stripe-payment-disabled">
        <FaShieldAlt className="icon" />
        <h3>Stripe Payment Unavailable</h3>
        <p>Stripe payments are only available for verified NFTs.</p>
        <button onClick={onPaymentCancel} className="btn-secondary">
          Continue with SHAH
        </button>
      </div>
    );
  }

  if (isProcessing) {
    return (
      <div className="stripe-payment-processing">
        <div className="processing-spinner"></div>
        <h3>Processing Payment</h3>
        <p>{processingStep}</p>
        <div className="processing-steps">
          <div className="step">
            <FaExchangeAlt className="step-icon" />
            <span>Converting USD to SHAH</span>
          </div>
          <div className="step">
            <FaCheckCircle className="step-icon" />
            <span>Finalizing transaction</span>
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="stripe-payment-container">
      <div className="payment-header">
        <h2>💳 Complete Purchase with Card</h2>
        <p>Pay with credit card - USD will be converted to SHAH automatically</p>
      </div>

      <div className="nft-info">
        <div className="nft-image">
          <img src={nft.imageURI || '/placeholder-nft.png'} alt={nft.name} />
        </div>
        <div className="nft-details">
          <h3>{nft.name}</h3>
          <p className="nft-description">{nft.description}</p>
          <div className="price-info">
            <div className="price-single">
              <span className="price-label">Price:</span>
              <span className="price-amount">${usdPrice.toFixed(2)} USD</span>
            </div>
            <div className="price-conversion">
              <span className="conversion-text">
                ≈ {(nft.listedPriceSatoshis / 100000000).toFixed(8)} SHAH
              </span>
              <span className="conversion-rate">
                (Rate: ${shahPrice?.toFixed(6) || '0.000000'} per SHAH)
              </span>
            </div>
          </div>
        </div>
      </div>

      <div className="payment-options">
        <h4>Payment Method</h4>
        <div className="payment-methods">
          <label className={`payment-method ${paymentMethod === 'card' ? 'selected' : ''}`}>
            <input
              type="radio"
              name="paymentMethod"
              value="card"
              checked={paymentMethod === 'card'}
              onChange={(e) => setPaymentMethod(e.target.value)}
            />
            <FaCreditCard className="method-icon" />
            <span>Credit/Debit Card</span>
          </label>

          <label className={`payment-method ${paymentMethod === 'apple' ? 'selected' : ''}`}>
            <input
              type="radio"
              name="paymentMethod"
              value="apple"
              checked={paymentMethod === 'apple'}
              onChange={(e) => setPaymentMethod(e.target.value)}
            />
            <FaApple className="method-icon" />
            <span>Apple Pay</span>
          </label>

          <label className={`payment-method ${paymentMethod === 'google' ? 'selected' : ''}`}>
            <input
              type="radio"
              name="paymentMethod"
              value="google"
              checked={paymentMethod === 'google'}
              onChange={(e) => setPaymentMethod(e.target.value)}
            />
            <FaGoogle className="method-icon" />
            <span>Google Pay</span>
          </label>
        </div>
      </div>

      <div className="payment-security">
        <FaShieldAlt className="security-icon" />
        <div className="security-info">
          <h4>Secure Payment</h4>
          <ul>
            <li>PCI DSS compliant</li>
            <li>256-bit SSL encryption</li>
            <li>No card data stored</li>
            <li>Instant SHAH conversion</li>
          </ul>
        </div>
      </div>

      <div className="terms-section">
        <label className="terms-checkbox">
          <input
            type="checkbox"
            checked={termsAccepted}
            onChange={(e) => setTermsAccepted(e.target.checked)}
          />
          <span>
            I agree to the{' '}
            <a href="/terms" target="_blank" rel="noopener noreferrer">
              Terms of Service
            </a>{' '}
            and{' '}
            <a href="/privacy" target="_blank" rel="noopener noreferrer">
              Privacy Policy
            </a>
          </span>
        </label>
      </div>

      {error && (
        <div className="payment-error">
          <p>{error}</p>
        </div>
      )}

      <div className="payment-actions">
        <button
          onClick={handlePayment}
          disabled={!termsAccepted || isLoading}
          className="btn-pay"
        >
          {isLoading ? (
            <>
              <div className="spinner"></div>
              Processing...
            </>
          ) : (
            <>
              <FaCreditCard />
              Pay ${usdPrice.toFixed(2)}
            </>
          )}
        </button>
        
        <button
          onClick={handleCancel}
          disabled={isLoading}
          className="btn-cancel"
        >
          Cancel
        </button>
      </div>

      <div className="payment-footer">
        <p>
          <small>
            By completing this purchase, you authorize the conversion of USD to SHAH 
            at the current market rate via ShahSwap.
          </small>
        </p>
      </div>
    </div>
  );
};

export default StripeNFTPayment;
