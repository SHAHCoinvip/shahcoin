// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

import React, { useState, useEffect, useCallback } from 'react';
import {
  FaCrown, FaLock, FaUnlock, FaCreditCard, FaGift, FaRocket, FaCheckCircle,
  FaRobot, FaChartLine, FaBolt, FaTools, FaGlobeAmericas, FaPalette,
  FaBox, FaClock, FaAnalytics, FaBell, FaFileAlt, FaEnvelope, FaTimes, FaCog
} from 'react-icons/fa';
import './PremiumSubscription.css';

// Premium Features Definitions
const PREMIUM_FEATURES = {
  AI_WALLET_ASSISTANT: {
    icon: FaRobot,
    title: '🧠 AI Wallet Assistant',
    description: 'Get intelligent help with transactions, security, and wallet management',
    category: 'ai'
  },
  AI_INSIGHTS_PANEL: {
    icon: FaChartLine,
    title: '📊 AI Insights Panel',
    description: 'Real-time security analysis and performance insights powered by AI',
    category: 'ai'
  },
  AI_FEE_OPTIMIZER: {
    icon: FaBolt,
    title: '⚡ AI Fee Optimizer',
    description: 'Automatically optimize transaction fees for cost and speed',
    category: 'ai'
  },
  AI_TROUBLESHOOTER: {
    icon: FaTools,
    title: '🛠️ AI Troubleshooter',
    description: 'Diagnose and fix wallet issues with AI-powered assistance',
    category: 'ai'
  },
  MULTILINGUAL_ASSISTANT: {
    icon: FaGlobeAmericas,
    title: '🌐 Multilingual Assistant',
    description: 'Get support in multiple languages with real-time translation',
    category: 'ai'
  },
  AI_ART_BOOSTER: {
    icon: FaPalette,
    title: '🎨 AI Art Booster',
    description: 'Generate and enhance NFT artwork with AI tools',
    category: 'trading'
  },
  NFT_COLLECTION_DROPS: {
    icon: FaBox,
    title: '📦 NFT Collection Drops',
    description: 'Create and manage large-scale NFT collection launches',
    category: 'trading'
  },
  AUCTION_MODULE: {
    icon: FaClock,
    title: '🕰️ Auction Module',
    description: 'Participate in and create NFT auctions with advanced features',
    category: 'trading'
  },
  AI_PORTFOLIO_ANALYZER: {
    icon: FaAnalytics,
    title: '📈 AI Portfolio Analyzer',
    description: 'Deep portfolio analysis with AI-powered recommendations',
    category: 'tools'
  },
  NFT_FLOOR_PRICE_ALERTS: {
    icon: FaBell,
    title: '🔔 NFT Floor Price Alerts',
    description: 'Get notified when NFT collections hit your target prices',
    category: 'tools'
  },
  USAGE_DASHBOARD: {
    icon: FaFileAlt,
    title: '🧾 Usage Dashboard',
    description: 'Comprehensive transaction history and usage analytics',
    category: 'tools'
  },
  EMAIL_CONFIRMATION_TOGGLE: {
    icon: FaEnvelope,
    title: '📬 Email Confirmation Settings',
    description: 'Configure email notifications for transactions and security',
    category: 'settings'
  }
};

const SUBSCRIPTION_PLANS = {
  FREE: {
    id: 'free',
    name: 'Free',
    price: 0,
    interval: null,
    features: ['Basic wallet features', 'Standard transaction support', 'Basic NFT browsing'],
    recommended: false
  },
  MONTHLY: {
    id: 'monthly',
    name: 'Monthly Premium',
    price: 4.99,
    interval: 'month',
    features: ['All Premium Features', 'AI-powered assistance', 'Advanced analytics', 'Priority support'],
    recommended: false
  },
  YEARLY: {
    id: 'yearly',
    name: 'Yearly Premium',
    price: 49.99,
    interval: 'year',
    features: ['All Premium Features', 'AI-powered assistance', 'Advanced analytics', 'Priority support', '17% savings vs monthly'],
    recommended: true
  }
};

const PAYMENT_METHODS = {
  SHAH_NATIVE: { id: 'shah_native', name: '💰 SHAH (Native)', description: 'Pay with native SHAH from your wallet' },
  SHAH_TOKEN: { id: 'shah_token', name: '🪙 SHAHtoken (ERC-20)', description: 'Pay with SHAHtoken on Ethereum network' },
  STRIPE_CARD: { id: 'stripe_card', name: '💳 Credit/Debit Card', description: 'Pay with credit card through Stripe' }
};

// Premium Subscription Hook
export function usePremiumSubscription() {
  const [subscriptionData, setSubscriptionData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const loadSubscription = useCallback(async () => {
    try {
      setLoading(true);
      // In real implementation, this would fetch from API
      const response = await fetch('/api/premium/subscription', {
        headers: { 'Authorization': `Bearer ${localStorage.getItem('authToken')}` }
      });
      
      if (response.ok) {
        const data = await response.json();
        setSubscriptionData(data);
      } else {
        // No active subscription
        setSubscriptionData(null);
      }
    } catch (err) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  }, []);

  const purchaseSubscription = useCallback(async (plan, paymentMethod, email = '') => {
    try {
      setLoading(true);
      const response = await fetch('/api/premium/purchase', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${localStorage.getItem('authToken')}`
        },
        body: JSON.stringify({ plan, paymentMethod, email })
      });

      if (response.ok) {
        const result = await response.json();
        await loadSubscription(); // Refresh subscription data
        return { success: true, data: result };
      } else {
        const error = await response.json();
        return { success: false, error: error.message };
      }
    } catch (err) {
      return { success: false, error: err.message };
    } finally {
      setLoading(false);
    }
  }, [loadSubscription]);

  const cancelSubscription = useCallback(async () => {
    try {
      setLoading(true);
      const response = await fetch('/api/premium/cancel', {
        method: 'POST',
        headers: { 'Authorization': `Bearer ${localStorage.getItem('authToken')}` }
      });

      if (response.ok) {
        await loadSubscription(); // Refresh subscription data
        return { success: true };
      } else {
        const error = await response.json();
        return { success: false, error: error.message };
      }
    } catch (err) {
      return { success: false, error: err.message };
    } finally {
      setLoading(false);
    }
  }, [loadSubscription]);

  const isFeatureUnlocked = useCallback((feature) => {
    return subscriptionData?.isActive && subscriptionData?.plan !== 'free';
  }, [subscriptionData]);

  useEffect(() => {
    loadSubscription();
  }, [loadSubscription]);

  return {
    subscriptionData,
    loading,
    error,
    purchaseSubscription,
    cancelSubscription,
    isFeatureUnlocked,
    refreshSubscription: loadSubscription
  };
}

// Premium Status Widget
export function PremiumStatusWidget({ className = '' }) {
  const { subscriptionData, loading } = usePremiumSubscription();

  if (loading) {
    return (
      <div className={`premium-status-widget ${className}`}>
        <div className="premium-status-loading">Loading...</div>
      </div>
    );
  }

  const isPremium = subscriptionData?.isActive;
  const daysRemaining = subscriptionData?.daysRemaining || 0;

  return (
    <div className={`premium-status-widget ${isPremium ? 'premium' : 'free'} ${className}`}>
      <div className="premium-status-icon">
        {isPremium ? '👑' : '🆓'}
      </div>
      <div className="premium-status-text">
        {isPremium ? (
          <>
            <span className="premium-plan">{subscriptionData.planName}</span>
            <span className="premium-expiry">
              {daysRemaining > 7 ? 'Active' : daysRemaining > 0 ? `${daysRemaining} days left` : 'Expired'}
            </span>
          </>
        ) : (
          <>
            <span className="free-plan">Free Plan</span>
            <span className="upgrade-text">Upgrade available</span>
          </>
        )}
      </div>
    </div>
  );
}

// Feature Gate Component
export function FeatureGate({ feature, children, className = '' }) {
  const { isFeatureUnlocked } = usePremiumSubscription();
  const [showUpgrade, setShowUpgrade] = useState(false);

  const unlocked = isFeatureUnlocked(feature);

  if (unlocked) {
    return <div className={className}>{children}</div>;
  }

  return (
    <div className={`feature-gate ${className}`}>
      <div className="feature-gate-overlay">
        <div className="feature-gate-content">
          <FaLock className="feature-gate-icon" />
          <h3 className="feature-gate-title">Premium Feature</h3>
          <p className="feature-gate-description">
            This feature is available with Shahcoin Premium
          </p>
          <button 
            className="feature-gate-upgrade-btn"
            onClick={() => setShowUpgrade(true)}
          >
            <FaRocket /> Upgrade Now
          </button>
        </div>
      </div>
      <div className="feature-gate-disabled-content">
        {children}
      </div>
      {showUpgrade && (
        <PremiumUpgradeModal onClose={() => setShowUpgrade(false)} />
      )}
    </div>
  );
}

// Premium Upgrade Modal
export function PremiumUpgradeModal({ onClose }) {
  const [selectedPlan, setSelectedPlan] = useState('yearly');
  const [selectedPayment, setSelectedPayment] = useState('stripe_card');
  const [email, setEmail] = useState('');
  const [loading, setLoading] = useState(false);
  const [step, setStep] = useState('plans'); // plans, payment, summary
  
  const { purchaseSubscription } = usePremiumSubscription();

  const handlePurchase = async () => {
    if (selectedPayment === 'stripe_card' && !email) {
      alert('Please enter your email address for Stripe payments');
      return;
    }

    setLoading(true);
    const result = await purchaseSubscription(selectedPlan, selectedPayment, email);
    setLoading(false);

    if (result.success) {
      alert('🎉 Welcome to Shahcoin Premium! Your subscription is now active.');
      onClose();
    } else {
      alert(`Payment failed: ${result.error}`);
    }
  };

  return (
    <div className="premium-upgrade-modal-overlay">
      <div className="premium-upgrade-modal">
        <div className="premium-upgrade-header">
          <h2>
            <FaCrown /> Upgrade to Shahcoin Premium
          </h2>
          <button className="premium-upgrade-close" onClick={onClose}>
            <FaTimes />
          </button>
        </div>

        <div className="premium-upgrade-content">
          {step === 'plans' && (
            <div className="premium-plans-step">
              <h3>Choose Your Plan</h3>
              <div className="premium-plans-grid">
                {Object.entries(SUBSCRIPTION_PLANS).filter(([key]) => key !== 'FREE').map(([key, plan]) => (
                  <div 
                    key={key}
                    className={`premium-plan-card ${selectedPlan === key ? 'selected' : ''} ${plan.recommended ? 'recommended' : ''}`}
                    onClick={() => setSelectedPlan(key)}
                  >
                    {plan.recommended && <div className="premium-plan-badge">🏆 RECOMMENDED</div>}
                    <h4>{plan.name}</h4>
                    <div className="premium-plan-price">
                      ${plan.price}
                      <span className="premium-plan-interval">/{plan.interval}</span>
                    </div>
                    <ul className="premium-plan-features">
                      {plan.features.map((feature, index) => (
                        <li key={index}>
                          <FaCheckCircle /> {feature}
                        </li>
                      ))}
                    </ul>
                  </div>
                ))}
              </div>
              <button 
                className="premium-upgrade-continue"
                onClick={() => setStep('payment')}
              >
                Continue
              </button>
            </div>
          )}

          {step === 'payment' && (
            <div className="premium-payment-step">
              <h3>Choose Payment Method</h3>
              <div className="premium-payment-methods">
                {Object.entries(PAYMENT_METHODS).map(([key, method]) => (
                  <div 
                    key={key}
                    className={`premium-payment-method ${selectedPayment === key ? 'selected' : ''}`}
                    onClick={() => setSelectedPayment(key)}
                  >
                    <div className="premium-payment-method-name">{method.name}</div>
                    <div className="premium-payment-method-desc">{method.description}</div>
                  </div>
                ))}
              </div>

              {selectedPayment === 'stripe_card' && (
                <div className="premium-email-input">
                  <label>Email Address (for receipts):</label>
                  <input
                    type="email"
                    value={email}
                    onChange={(e) => setEmail(e.target.value)}
                    placeholder="Enter your email address"
                    required
                  />
                </div>
              )}

              <div className="premium-payment-buttons">
                <button onClick={() => setStep('plans')}>Back</button>
                <button 
                  className="premium-upgrade-continue"
                  onClick={() => setStep('summary')}
                >
                  Continue
                </button>
              </div>
            </div>
          )}

          {step === 'summary' && (
            <div className="premium-summary-step">
              <h3>Order Summary</h3>
              <div className="premium-summary-details">
                <div className="premium-summary-item">
                  <span>Plan:</span>
                  <span>{SUBSCRIPTION_PLANS[selectedPlan].name}</span>
                </div>
                <div className="premium-summary-item">
                  <span>Payment Method:</span>
                  <span>{PAYMENT_METHODS[selectedPayment].name}</span>
                </div>
                <div className="premium-summary-item">
                  <span>Total:</span>
                  <span className="premium-summary-price">
                    ${SUBSCRIPTION_PLANS[selectedPlan].price}
                  </span>
                </div>
                {email && (
                  <div className="premium-summary-item">
                    <span>Email:</span>
                    <span>{email}</span>
                  </div>
                )}
              </div>

              <div className="premium-summary-buttons">
                <button onClick={() => setStep('payment')}>Back</button>
                <button 
                  className="premium-upgrade-purchase"
                  onClick={handlePurchase}
                  disabled={loading}
                >
                  {loading ? 'Processing...' : '🚀 Purchase Premium'}
                </button>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}

// Premium Features Dashboard
export function PremiumFeaturesDashboard() {
  const { subscriptionData, isFeatureUnlocked } = usePremiumSubscription();
  const [activeTab, setActiveTab] = useState('ai');

  const featuresByCategory = Object.entries(PREMIUM_FEATURES).reduce((acc, [key, feature]) => {
    if (!acc[feature.category]) acc[feature.category] = [];
    acc[feature.category].push({ key, ...feature });
    return acc;
  }, {});

  const categories = [
    { id: 'ai', name: '🤖 AI Features', icon: FaRobot },
    { id: 'trading', name: '💼 Trading & NFT', icon: FaBox },
    { id: 'tools', name: '🛠️ Tools & Analytics', icon: FaTools },
    { id: 'settings', name: '⚙️ Settings', icon: FaCog }
  ];

  return (
    <div className="premium-features-dashboard">
      <div className="premium-features-header">
        <h2>
          <FaCrown /> Premium Features
        </h2>
        <PremiumStatusWidget />
      </div>

      <div className="premium-features-tabs">
        {categories.map(category => (
          <button
            key={category.id}
            className={`premium-features-tab ${activeTab === category.id ? 'active' : ''}`}
            onClick={() => setActiveTab(category.id)}
          >
            <category.icon /> {category.name}
          </button>
        ))}
      </div>

      <div className="premium-features-content">
        {featuresByCategory[activeTab]?.map(feature => (
          <div key={feature.key} className="premium-feature-card">
            <div className="premium-feature-header">
              <feature.icon className="premium-feature-icon" />
              <div className="premium-feature-info">
                <h3>{feature.title}</h3>
                <p>{feature.description}</p>
              </div>
              <div className="premium-feature-status">
                {isFeatureUnlocked(feature.key) ? (
                  <FaUnlock className="feature-unlocked" title="Unlocked" />
                ) : (
                  <FaLock className="feature-locked" title="Premium Required" />
                )}
              </div>
            </div>
            
            {!isFeatureUnlocked(feature.key) && (
              <div className="premium-feature-upgrade">
                <span>Upgrade to Premium to unlock this feature</span>
              </div>
            )}
          </div>
        ))}
      </div>
    </div>
  );
}

// Subscription Management Component
export function SubscriptionManagement() {
  const { subscriptionData, loading, cancelSubscription } = usePremiumSubscription();
  const [showCancel, setShowCancel] = useState(false);

  if (loading) {
    return <div className="subscription-loading">Loading subscription details...</div>;
  }

  if (!subscriptionData?.isActive) {
    return (
      <div className="subscription-inactive">
        <h3>No Active Subscription</h3>
        <p>You don't have an active premium subscription.</p>
        <PremiumUpgradeModal onClose={() => {}} />
      </div>
    );
  }

  const handleCancel = async () => {
    if (window.confirm('Are you sure you want to cancel your subscription? You will lose access to premium features.')) {
      const result = await cancelSubscription();
      if (result.success) {
        alert('Subscription cancelled successfully.');
        setShowCancel(false);
      } else {
        alert(`Failed to cancel subscription: ${result.error}`);
      }
    }
  };

  return (
    <div className="subscription-management">
      <div className="subscription-details">
        <h3>
          <FaCrown /> {subscriptionData.planName}
        </h3>
        <div className="subscription-info">
          <div className="subscription-info-item">
            <span>Status:</span>
            <span className={`status ${subscriptionData.isActive ? 'active' : 'inactive'}`}>
              {subscriptionData.isActive ? 'Active' : 'Inactive'}
            </span>
          </div>
          <div className="subscription-info-item">
            <span>Expires:</span>
            <span>{new Date(subscriptionData.expirationDate).toLocaleDateString()}</span>
          </div>
          <div className="subscription-info-item">
            <span>Auto-Renewal:</span>
            <span>{subscriptionData.autoRenew ? 'Enabled' : 'Disabled'}</span>
          </div>
        </div>
      </div>

      <div className="subscription-actions">
        <button className="subscription-action-btn primary">
          <FaGift /> Upgrade Plan
        </button>
        <button className="subscription-action-btn secondary">
          <FaCreditCard /> Payment History
        </button>
        <button 
          className="subscription-action-btn danger"
          onClick={() => setShowCancel(true)}
        >
          <FaTimes /> Cancel Subscription
        </button>
      </div>

      {showCancel && (
        <div className="subscription-cancel-confirm">
          <h4>Cancel Subscription</h4>
          <p>Are you sure you want to cancel your premium subscription?</p>
          <div className="subscription-cancel-actions">
            <button onClick={() => setShowCancel(false)}>Keep Subscription</button>
            <button className="danger" onClick={handleCancel}>
              Yes, Cancel
            </button>
          </div>
        </div>
      )}
    </div>
  );
}

export default {
  usePremiumSubscription,
  PremiumStatusWidget,
  FeatureGate,
  PremiumUpgradeModal,
  PremiumFeaturesDashboard,
  SubscriptionManagement,
  PREMIUM_FEATURES,
  SUBSCRIPTION_PLANS,
  PAYMENT_METHODS
};
