// Stripe Payments for Shahcoin NFT Marketplace
// Handles credit card purchases for verified NFTs with fiat-to-SHAH conversion

import { getShahPrice } from './price-oracle.js';

class StripePayments {
  constructor(config = {}) {
    this.stripePublicKey = config.stripePublicKey || process.env.STRIPE_PUBLIC_KEY;
    this.apiBaseUrl = config.apiBaseUrl || 'https://api.shah.vip/marketplace';
    this.isTestMode = config.isTestMode || process.env.NODE_ENV === 'development';
    
    if (!this.stripePublicKey) {
      console.warn('Stripe public key not configured. Stripe payments will be disabled.');
    }
  }

  /**
   * Check if Stripe payments are available for an NFT
   * @param {Object} nft - NFT data object
   * @returns {boolean} - Whether Stripe payments are enabled for this NFT
   */
  isStripeEnabled(nft) {
    if (!this.stripePublicKey) return false;
    
    // Only allow Stripe for verified NFTs
    const isVerified = nft.verified || 
                      (nft.metadata && nft.metadata.adminVerified) ||
                      (nft.tier && ['Pro', 'Premium'].includes(nft.tier));
    
    // Check if NFT is listed for sale
    const isListed = nft.isListed || nft.listedPriceSatoshis > 0;
    
    return isVerified && isListed;
  }

  /**
   * Calculate USD price for an NFT
   * @param {Object} nft - NFT data object
   * @returns {Promise<number>} - USD price
   */
  async calculateUSDPrice(nft) {
    const shahPrice = await getShahPrice();
    const priceSatoshis = nft.listedPriceSatoshis || 0;
    const priceShah = priceSatoshis / 100000000; // Convert satoshis to SHAH
    return priceShah * shahPrice;
  }

  /**
   * Create a Stripe checkout session for NFT purchase
   * @param {Object} nft - NFT data object
   * @param {string} buyerAddress - Buyer's wallet address
   * @param {Object} options - Additional options
   * @returns {Promise<Object>} - Stripe checkout session
   */
  async createCheckoutSession(nft, buyerAddress, options = {}) {
    if (!this.isStripeEnabled(nft)) {
      throw new Error('Stripe payments not available for this NFT');
    }

    const usdPrice = await this.calculateUSDPrice(nft);
    
    const sessionData = {
      payment_method_types: ['card'],
      line_items: [{
        price_data: {
          currency: 'usd',
          product_data: {
            name: nft.name,
            description: nft.description || `NFT by ${nft.creator}`,
            images: nft.imageURI ? [nft.imageURI] : [],
            metadata: {
              nft_id: nft.id,
              creator: nft.creator,
              tier: nft.tier || 'Basic'
            }
          },
          unit_amount: Math.round(usdPrice * 100), // Stripe expects cents
        },
        quantity: 1,
      }],
      mode: 'payment',
      success_url: options.successUrl || `${window.location.origin}/marketplace/purchase/success?session_id={CHECKOUT_SESSION_ID}`,
      cancel_url: options.cancelUrl || `${window.location.origin}/marketplace/purchase/cancel`,
      metadata: {
        nft_id: nft.id,
        buyer_address: buyerAddress,
        seller_address: nft.owner,
        price_satoshis: nft.listedPriceSatoshis.toString(),
        price_usd: usdPrice.toString(),
        tier: nft.tier || 'Basic'
      },
      customer_email: options.customerEmail,
      allow_promotion_codes: true,
      billing_address_collection: 'required',
      shipping_address_collection: {
        allowed_countries: ['US', 'CA', 'GB', 'DE', 'FR', 'AU', 'JP', 'SG']
      }
    };

    try {
      const response = await fetch(`${this.apiBaseUrl}/stripe/create-checkout-session`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${options.apiKey || ''}`
        },
        body: JSON.stringify(sessionData)
      });

      if (!response.ok) {
        throw new Error(`Failed to create checkout session: ${response.status}`);
      }

      const result = await response.json();
      return result;
    } catch (error) {
      console.error('Stripe checkout session creation failed:', error);
      throw error;
    }
  }

  /**
   * Process a successful Stripe payment
   * @param {string} sessionId - Stripe checkout session ID
   * @param {Object} options - Additional options
   * @returns {Promise<Object>} - Payment processing result
   */
  async processPayment(sessionId, options = {}) {
    try {
      const response = await fetch(`${this.apiBaseUrl}/stripe/process-payment`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${options.apiKey || ''}`
        },
        body: JSON.stringify({
          session_id: sessionId,
          buyer_address: options.buyerAddress,
          auto_convert: options.autoConvert !== false // Default to true
        })
      });

      if (!response.ok) {
        throw new Error(`Payment processing failed: ${response.status}`);
      }

      const result = await response.json();
      return result;
    } catch (error) {
      console.error('Stripe payment processing failed:', error);
      throw error;
    }
  }

  /**
   * Get payment status
   * @param {string} sessionId - Stripe checkout session ID
   * @returns {Promise<Object>} - Payment status
   */
  async getPaymentStatus(sessionId) {
    try {
      const response = await fetch(`${this.apiBaseUrl}/stripe/payment-status/${sessionId}`);
      
      if (!response.ok) {
        throw new Error(`Failed to get payment status: ${response.status}`);
      }

      return await response.json();
    } catch (error) {
      console.error('Failed to get payment status:', error);
      throw error;
    }
  }

  /**
   * Create a refund for a payment
   * @param {string} sessionId - Stripe checkout session ID
   * @param {Object} options - Refund options
   * @returns {Promise<Object>} - Refund result
   */
  async createRefund(sessionId, options = {}) {
    try {
      const response = await fetch(`${this.apiBaseUrl}/stripe/refund`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${options.apiKey || ''}`
        },
        body: JSON.stringify({
          session_id: sessionId,
          reason: options.reason || 'requested_by_customer',
          amount: options.amount // Optional: partial refund amount in cents
        })
      });

      if (!response.ok) {
        throw new Error(`Refund failed: ${response.status}`);
      }

      return await response.json();
    } catch (error) {
      console.error('Stripe refund failed:', error);
      throw error;
    }
  }

  /**
   * Get payment history for a wallet address
   * @param {string} walletAddress - Wallet address
   * @param {Object} options - Query options
   * @returns {Promise<Array>} - Payment history
   */
  async getPaymentHistory(walletAddress, options = {}) {
    try {
      const params = new URLSearchParams({
        address: walletAddress,
        limit: options.limit || 50,
        offset: options.offset || 0
      });

      const response = await fetch(`${this.apiBaseUrl}/stripe/payment-history?${params}`);
      
      if (!response.ok) {
        throw new Error(`Failed to get payment history: ${response.status}`);
      }

      return await response.json();
    } catch (error) {
      console.error('Failed to get payment history:', error);
      throw error;
    }
  }

  /**
   * Validate payment session
   * @param {string} sessionId - Stripe checkout session ID
   * @returns {Promise<boolean>} - Whether session is valid
   */
  async validateSession(sessionId) {
    try {
      const status = await this.getPaymentStatus(sessionId);
      return status.status === 'complete' && status.payment_status === 'paid';
    } catch (error) {
      return false;
    }
  }
}

// Export singleton instance
export const stripePayments = new StripePayments();

// Export class for custom instances
export { StripePayments };

// React hook for Stripe payments
export function useStripePayments(config = {}) {
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState(null);
  const [paymentStatus, setPaymentStatus] = useState(null);

  const createPayment = useCallback(async (nft, buyerAddress, options = {}) => {
    setIsLoading(true);
    setError(null);
    
    try {
      const session = await stripePayments.createCheckoutSession(nft, buyerAddress, options);
      
      // Redirect to Stripe checkout
      if (window.Stripe) {
        const stripe = window.Stripe(stripePayments.stripePublicKey);
        const { error } = await stripe.redirectToCheckout({
          sessionId: session.id
        });
        
        if (error) {
          throw error;
        }
      } else {
        // Fallback: redirect to Stripe hosted checkout
        window.location.href = session.url;
      }
      
      return session;
    } catch (err) {
      setError(err.message);
      throw err;
    } finally {
      setIsLoading(false);
    }
  }, []);

  const checkPaymentStatus = useCallback(async (sessionId) => {
    try {
      const status = await stripePayments.getPaymentStatus(sessionId);
      setPaymentStatus(status);
      return status;
    } catch (err) {
      setError(err.message);
      throw err;
    }
  }, []);

  return {
    isLoading,
    error,
    paymentStatus,
    createPayment,
    checkPaymentStatus,
    isStripeEnabled: (nft) => stripePayments.isStripeEnabled(nft),
    calculateUSDPrice: (nft) => stripePayments.calculateUSDPrice(nft)
  };
}
