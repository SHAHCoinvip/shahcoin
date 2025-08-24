// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

/**
 * Shahcoin Premium Telegram Mini App Integration
 * Handles premium subscription management and feature gating within Telegram
 */

import { Telegraf, Markup } from 'telegraf';
import fetch from 'node-fetch';

class PremiumTelegramMiniApp {
  constructor(botToken, apiBaseUrl = 'https://api.shah.vip') {
    this.bot = new Telegraf(botToken);
    this.apiBaseUrl = apiBaseUrl;
    this.premiumFeatures = {
      AI_WALLET_ASSISTANT: '🧠 AI Wallet Assistant',
      AI_INSIGHTS_PANEL: '📊 AI Insights Panel',
      AI_FEE_OPTIMIZER: '⚡ AI Fee Optimizer',
      AI_TROUBLESHOOTER: '🛠️ AI Troubleshooter',
      MULTILINGUAL_ASSISTANT: '🌐 Multilingual Assistant',
      AI_ART_BOOSTER: '🎨 AI Art Booster',
      NFT_COLLECTION_DROPS: '📦 NFT Collection Drops',
      AUCTION_MODULE: '🕰️ Auction Module',
      AI_PORTFOLIO_ANALYZER: '📈 AI Portfolio Analyzer',
      NFT_FLOOR_PRICE_ALERTS: '🔔 NFT Floor Price Alerts',
      USAGE_DASHBOARD: '🧾 Usage Dashboard',
      EMAIL_CONFIRMATION_TOGGLE: '📬 Email Confirmations'
    };
    
    this.setupCommands();
    this.setupCallbacks();
  }

  setupCommands() {
    // Premium status command
    this.bot.command('premium', async (ctx) => {
      await this.showPremiumStatus(ctx);
    });

    // Upgrade command
    this.bot.command('upgrade', async (ctx) => {
      await this.showUpgradeOptions(ctx);
    });

    // Features command
    this.bot.command('features', async (ctx) => {
      await this.showPremiumFeatures(ctx);
    });

    // Subscription management
    this.bot.command('subscription', async (ctx) => {
      await this.showSubscriptionManagement(ctx);
    });

    // AI Assistant (Premium Feature)
    this.bot.command('ai', async (ctx) => {
      const isUnlocked = await this.checkFeatureAccess(ctx.from.id, 'AI_WALLET_ASSISTANT');
      if (!isUnlocked) {
        await this.showFeatureGate(ctx, 'AI_WALLET_ASSISTANT');
        return;
      }
      await this.showAIAssistant(ctx);
    });

    // Portfolio Analysis (Premium Feature)
    this.bot.command('portfolio', async (ctx) => {
      const isUnlocked = await this.checkFeatureAccess(ctx.from.id, 'AI_PORTFOLIO_ANALYZER');
      if (!isUnlocked) {
        await this.showFeatureGate(ctx, 'AI_PORTFOLIO_ANALYZER');
        return;
      }
      await this.showPortfolioAnalysis(ctx);
    });

    // Price Alerts (Premium Feature)
    this.bot.command('alerts', async (ctx) => {
      const isUnlocked = await this.checkFeatureAccess(ctx.from.id, 'NFT_FLOOR_PRICE_ALERTS');
      if (!isUnlocked) {
        await this.showFeatureGate(ctx, 'NFT_FLOOR_PRICE_ALERTS');
        return;
      }
      await this.showPriceAlerts(ctx);
    });
  }

  setupCallbacks() {
    // Handle upgrade callbacks
    this.bot.action(/^upgrade_(.+)$/, async (ctx) => {
      const plan = ctx.match[1];
      await this.initiatePurchase(ctx, plan);
    });

    // Handle payment method callbacks
    this.bot.action(/^payment_(.+)_(.+)$/, async (ctx) => {
      const plan = ctx.match[1];
      const method = ctx.match[2];
      await this.processPurchase(ctx, plan, method);
    });

    // Handle subscription management callbacks
    this.bot.action('manage_subscription', async (ctx) => {
      await this.showSubscriptionManagement(ctx);
    });

    this.bot.action('cancel_subscription', async (ctx) => {
      await this.confirmCancellation(ctx);
    });

    this.bot.action('confirm_cancel', async (ctx) => {
      await this.cancelSubscription(ctx);
    });

    // Handle feature callbacks
    this.bot.action(/^feature_(.+)$/, async (ctx) => {
      const feature = ctx.match[1];
      const isUnlocked = await this.checkFeatureAccess(ctx.from.id, feature);
      if (!isUnlocked) {
        await this.showFeatureGate(ctx, feature);
        return;
      }
      await this.showFeatureDetails(ctx, feature);
    });

    // Handle Mini App callbacks
    this.bot.action('open_webapp', async (ctx) => {
      await this.openWebApp(ctx);
    });
  }

  async showPremiumStatus(ctx) {
    const userId = ctx.from.id;
    const subscription = await this.getSubscriptionData(userId);
    
    let statusMessage = '👑 <b>Shahcoin Premium Status</b>\n\n';
    
    if (subscription && subscription.isActive) {
      const daysRemaining = Math.ceil((new Date(subscription.expirationDate) - new Date()) / (1000 * 60 * 60 * 24));
      
      statusMessage += `✅ <b>Status:</b> Active Premium\n`;
      statusMessage += `📋 <b>Plan:</b> ${subscription.planName}\n`;
      statusMessage += `📅 <b>Expires:</b> ${new Date(subscription.expirationDate).toLocaleDateString()}\n`;
      statusMessage += `⏰ <b>Days Remaining:</b> ${daysRemaining}\n`;
      statusMessage += `🔄 <b>Auto-Renewal:</b> ${subscription.autoRenew ? 'Enabled' : 'Disabled'}\n\n`;
      
      if (daysRemaining <= 7) {
        statusMessage += '⚠️ <b>Your subscription expires soon!</b>\n\n';
      }
      
      statusMessage += '✨ <b>Unlocked Features:</b>\n';
      Object.values(this.premiumFeatures).forEach(feature => {
        statusMessage += `• ${feature}\n`;
      });
    } else {
      statusMessage += `🆓 <b>Status:</b> Free Plan\n`;
      statusMessage += `💫 <b>Available Features:</b> Basic wallet functions\n\n`;
      statusMessage += `🚀 <b>Upgrade to Premium to unlock:</b>\n`;
      Object.values(this.premiumFeatures).forEach(feature => {
        statusMessage += `• ${feature}\n`;
      });
    }

    const keyboard = subscription?.isActive ? 
      Markup.inlineKeyboard([
        [Markup.button.callback('⚙️ Manage Subscription', 'manage_subscription')],
        [Markup.button.callback('📱 Open Web App', 'open_webapp')],
        [Markup.button.callback('🔄 Refresh Status', 'premium')]
      ]) :
      Markup.inlineKeyboard([
        [Markup.button.callback('🚀 Upgrade Now', 'upgrade_yearly')],
        [Markup.button.callback('📋 View All Plans', 'upgrade_plans')],
        [Markup.button.callback('📱 Open Web App', 'open_webapp')]
      ]);

    await ctx.editMessageText(statusMessage, {
      parse_mode: 'HTML',
      ...keyboard
    });
  }

  async showUpgradeOptions(ctx) {
    const upgradeMessage = `
👑 <b>Upgrade to Shahcoin Premium</b>

<b>🎯 Choose Your Plan:</b>

💎 <b>Yearly Premium - $49.99/year</b>
• Save 17% vs monthly billing
• All premium features unlocked
• Priority customer support
• Bonus exclusive features

📅 <b>Monthly Premium - $4.99/month</b>
• Perfect for trying premium features
• All premium features unlocked
• Standard customer support

<b>💳 Payment Methods Available:</b>
• 💰 SHAH (Native blockchain)
• 🪙 SHAHtoken (ERC-20)
• 💳 Credit/Debit Card (Stripe)

<b>✨ Premium Features Include:</b>
${Object.values(this.premiumFeatures).map(feature => `• ${feature}`).join('\n')}
`;

    const keyboard = Markup.inlineKeyboard([
      [Markup.button.callback('🏆 Yearly ($49.99) - RECOMMENDED', 'upgrade_yearly')],
      [Markup.button.callback('📅 Monthly ($4.99)', 'upgrade_monthly')],
      [Markup.button.callback('📖 View Features Details', 'features')],
      [Markup.button.callback('← Back to Status', 'premium')]
    ]);

    await ctx.editMessageText(upgradeMessage, {
      parse_mode: 'HTML',
      ...keyboard
    });
  }

  async initiatePurchase(ctx, plan) {
    const planDetails = {
      yearly: { name: 'Yearly Premium', price: '$49.99/year' },
      monthly: { name: 'Monthly Premium', price: '$4.99/month' }
    };

    const purchaseMessage = `
💳 <b>Purchase ${planDetails[plan].name}</b>

<b>Plan:</b> ${planDetails[plan].name}
<b>Price:</b> ${planDetails[plan].price}

<b>Choose your payment method:</b>
`;

    const keyboard = Markup.inlineKeyboard([
      [Markup.button.callback('💰 Pay with SHAH', `payment_${plan}_shah_native`)],
      [Markup.button.callback('🪙 Pay with SHAHtoken', `payment_${plan}_shah_token`)],
      [Markup.button.callback('💳 Pay with Credit Card', `payment_${plan}_stripe`)],
      [Markup.button.callback('← Back to Plans', 'upgrade_plans')]
    ]);

    await ctx.editMessageText(purchaseMessage, {
      parse_mode: 'HTML',
      ...keyboard
    });
  }

  async processPurchase(ctx, plan, method) {
    const userId = ctx.from.id;
    const walletAddress = await this.getWalletAddress(userId);
    
    if (!walletAddress) {
      await ctx.answerCbQuery('❌ Please connect your wallet first!');
      return;
    }

    const processingMessage = `
⏳ <b>Processing Payment...</b>

<b>Plan:</b> ${plan === 'yearly' ? 'Yearly Premium' : 'Monthly Premium'}
<b>Payment Method:</b> ${this.getPaymentMethodName(method)}

Please wait while we process your payment...
`;

    await ctx.editMessageText(processingMessage, {
      parse_mode: 'HTML'
    });

    try {
      // Call backend API to process payment
      const response = await fetch(`${this.apiBaseUrl}/premium/purchase`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${await this.getUserToken(userId)}`
        },
        body: JSON.stringify({
          plan: plan.toUpperCase(),
          paymentMethod: method.toUpperCase(),
          walletAddress,
          telegramUserId: userId
        })
      });

      const result = await response.json();

      if (response.ok && result.success) {
        const successMessage = `
🎉 <b>Welcome to Shahcoin Premium!</b>

Your subscription has been activated successfully!

<b>✅ Plan:</b> ${result.planName}
<b>📅 Expires:</b> ${new Date(result.expirationDate).toLocaleDateString()}
<b>💳 Payment:</b> ${result.transactionId}

<b>🚀 You now have access to all premium features!</b>

Start exploring your premium features below:
`;

        const keyboard = Markup.inlineKeyboard([
          [Markup.button.callback('🧠 AI Assistant', 'feature_AI_WALLET_ASSISTANT')],
          [Markup.button.callback('📊 Portfolio Analysis', 'feature_AI_PORTFOLIO_ANALYZER')],
          [Markup.button.callback('⚡ Fee Optimizer', 'feature_AI_FEE_OPTIMIZER')],
          [Markup.button.callback('👑 View Premium Status', 'premium')]
        ]);

        await ctx.editMessageText(successMessage, {
          parse_mode: 'HTML',
          ...keyboard
        });

        // Send welcome message with premium tips
        setTimeout(async () => {
          await ctx.reply(`
🎊 <b>Premium Tips & Tricks:</b>

• Use /ai to access your AI Wallet Assistant
• Use /portfolio for instant portfolio analysis
• Use /alerts to set up NFT price alerts
• Access the full Web App for advanced features

<b>Need help?</b> Our premium support team is here for you!
`, { parse_mode: 'HTML' });
        }, 2000);

      } else {
        throw new Error(result.error || 'Payment failed');
      }

    } catch (error) {
      const errorMessage = `
❌ <b>Payment Failed</b>

${error.message}

Please try again or contact support if the issue persists.
`;

      const keyboard = Markup.inlineKeyboard([
        [Markup.button.callback('🔄 Try Again', `upgrade_${plan}`)],
        [Markup.button.callback('🏠 Back to Status', 'premium')]
      ]);

      await ctx.editMessageText(errorMessage, {
        parse_mode: 'HTML',
        ...keyboard
      });
    }
  }

  async showFeatureGate(ctx, feature) {
    const featureName = this.premiumFeatures[feature];
    
    const gateMessage = `
🔒 <b>Premium Feature Locked</b>

<b>${featureName}</b>

This feature is available with Shahcoin Premium subscription.

<b>🚀 Upgrade now to unlock:</b>
• ${featureName}
• All other premium features
• Priority support
• Advanced analytics

<b>💎 Special Telegram Offer:</b>
Get 10% off your first year with code: TELEGRAM10
`;

    const keyboard = Markup.inlineKeyboard([
      [Markup.button.callback('🚀 Upgrade to Premium', 'upgrade_yearly')],
      [Markup.button.callback('📋 View All Features', 'features')],
      [Markup.button.callback('🏠 Back to Menu', 'premium')]
    ]);

    await ctx.reply(gateMessage, {
      parse_mode: 'HTML',
      ...keyboard
    });
  }

  async showAIAssistant(ctx) {
    const aiMessage = `
🧠 <b>AI Wallet Assistant</b>

Welcome to your personal AI assistant! I can help you with:

• 💰 Balance and transaction guidance
• 🔒 Security recommendations
• ⚡ Fee optimization tips
• 📊 Portfolio insights
• 🚀 Transaction assistance

<b>Ask me anything about your Shahcoin wallet!</b>

Example questions:
• "What's my current balance?"
• "How can I optimize my fees?"
• "Is my wallet secure?"
• "Show me my portfolio performance"
`;

    const keyboard = Markup.inlineKeyboard([
      [Markup.button.callback('💰 Check Balance', 'ai_balance')],
      [Markup.button.callback('🔒 Security Check', 'ai_security')],
      [Markup.button.callback('⚡ Optimize Fees', 'ai_fees')],
      [Markup.button.callback('📊 Portfolio Insights', 'ai_portfolio')]
    ]);

    await ctx.reply(aiMessage, {
      parse_mode: 'HTML',
      ...keyboard
    });
  }

  async showPortfolioAnalysis(ctx) {
    const userId = ctx.from.id;
    const portfolioData = await this.getPortfolioData(userId);

    const analysisMessage = `
📈 <b>AI Portfolio Analysis</b>

<b>📊 Portfolio Overview:</b>
• Total Value: ${portfolioData.totalValue} SHAH
• 24h Change: ${portfolioData.change24h}% ${portfolioData.change24h > 0 ? '📈' : '📉'}
• Risk Score: ${portfolioData.riskScore}/100

<b>🎯 AI Recommendations:</b>
${portfolioData.recommendations.map(rec => `• ${rec}`).join('\n')}

<b>⚡ Quick Actions:</b>
`;

    const keyboard = Markup.inlineKeyboard([
      [Markup.button.callback('📊 Detailed Analysis', 'portfolio_detailed')],
      [Markup.button.callback('🎯 Rebalance Tips', 'portfolio_rebalance')],
      [Markup.button.callback('📈 Performance History', 'portfolio_history')],
      [Markup.button.callback('🔄 Refresh Analysis', 'portfolio')]
    ]);

    await ctx.reply(analysisMessage, {
      parse_mode: 'HTML',
      ...keyboard
    });
  }

  async showSubscriptionManagement(ctx) {
    const userId = ctx.from.id;
    const subscription = await this.getSubscriptionData(userId);

    if (!subscription || !subscription.isActive) {
      await ctx.answerCbQuery('❌ No active subscription found');
      return;
    }

    const managementMessage = `
⚙️ <b>Subscription Management</b>

<b>Current Plan:</b> ${subscription.planName}
<b>Status:</b> ${subscription.isActive ? '✅ Active' : '❌ Inactive'}
<b>Expires:</b> ${new Date(subscription.expirationDate).toLocaleDateString()}
<b>Auto-Renewal:</b> ${subscription.autoRenew ? '✅ Enabled' : '❌ Disabled'}

<b>Payment History:</b>
Last Payment: ${subscription.lastPaymentDate}
Amount: $${subscription.lastPaymentAmount}

<b>📊 Usage Statistics:</b>
• AI Assistant queries: ${subscription.stats?.aiQueries || 0}
• Portfolio analyses: ${subscription.stats?.portfolioAnalyses || 0}
• Fee optimizations: ${subscription.stats?.feeOptimizations || 0}
`;

    const keyboard = Markup.inlineKeyboard([
      [Markup.button.callback('📋 Upgrade Plan', 'upgrade_plans')],
      [Markup.button.callback('💳 Payment History', 'payment_history')],
      [Markup.button.callback('🔄 Toggle Auto-Renewal', 'toggle_renewal')],
      [Markup.button.callback('❌ Cancel Subscription', 'cancel_subscription')],
      [Markup.button.callback('🏠 Back to Status', 'premium')]
    ]);

    await ctx.editMessageText(managementMessage, {
      parse_mode: 'HTML',
      ...keyboard
    });
  }

  async openWebApp(ctx) {
    const webAppUrl = `${this.apiBaseUrl.replace('api.', 'app.')}/premium?telegram_user_id=${ctx.from.id}`;
    
    const webAppMessage = `
📱 <b>Shahcoin Premium Web App</b>

Access the full premium experience with advanced features:

• 🧠 Complete AI Assistant interface
• 📊 Interactive portfolio analytics
• 🎨 AI Art generation tools
• 📦 NFT collection management
• 🕰️ Advanced auction features
• 📈 Real-time market insights

Click below to open the web app:
`;

    const keyboard = Markup.inlineKeyboard([
      [Markup.button.webApp('🚀 Open Premium Web App', webAppUrl)],
      [Markup.button.callback('🏠 Back to Menu', 'premium')]
    ]);

    await ctx.reply(webAppMessage, {
      parse_mode: 'HTML',
      ...keyboard
    });
  }

  // Utility Methods

  async checkFeatureAccess(userId, feature) {
    try {
      const subscription = await this.getSubscriptionData(userId);
      return subscription && subscription.isActive;
    } catch (error) {
      console.error('Error checking feature access:', error);
      return false;
    }
  }

  async getSubscriptionData(userId) {
    try {
      const response = await fetch(`${this.apiBaseUrl}/premium/subscription/${userId}`, {
        headers: {
          'Authorization': `Bearer ${await this.getUserToken(userId)}`
        }
      });
      
      if (response.ok) {
        return await response.json();
      }
      return null;
    } catch (error) {
      console.error('Error fetching subscription data:', error);
      return null;
    }
  }

  async getPortfolioData(userId) {
    try {
      const response = await fetch(`${this.apiBaseUrl}/premium/portfolio/${userId}`, {
        headers: {
          'Authorization': `Bearer ${await this.getUserToken(userId)}`
        }
      });
      
      if (response.ok) {
        return await response.json();
      }
      
      // Return mock data if API fails
      return {
        totalValue: '1,250.00',
        change24h: 2.5,
        riskScore: 75,
        recommendations: [
          'Consider diversifying with NFT investments',
          'Your portfolio shows good growth potential',
          'Optimal time for DCA strategy'
        ]
      };
    } catch (error) {
      console.error('Error fetching portfolio data:', error);
      return {
        totalValue: '0.00',
        change24h: 0,
        riskScore: 0,
        recommendations: ['Unable to load recommendations']
      };
    }
  }

  async getWalletAddress(userId) {
    // This would typically fetch from user database
    // For now, return a mock address
    return `shah1${userId.toString().padStart(39, '0')}`;
  }

  async getUserToken(userId) {
    // This would typically generate or fetch a JWT token for the user
    // For now, return a mock token
    return `telegram_${userId}_${Date.now()}`;
  }

  getPaymentMethodName(method) {
    const methods = {
      shah_native: '💰 SHAH (Native)',
      shah_token: '🪙 SHAHtoken (ERC-20)',
      stripe: '💳 Credit Card'
    };
    return methods[method] || method;
  }

  async cancelSubscription(ctx) {
    const userId = ctx.from.id;
    
    try {
      const response = await fetch(`${this.apiBaseUrl}/premium/cancel`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${await this.getUserToken(userId)}`
        },
        body: JSON.stringify({ telegramUserId: userId })
      });

      if (response.ok) {
        const cancelMessage = `
✅ <b>Subscription Cancelled</b>

Your premium subscription has been cancelled successfully.

• You'll continue to have access until your current billing period ends
• No further charges will be made
• You can reactivate anytime

<b>We're sorry to see you go!</b>
If you change your mind, you can always upgrade again.
`;

        await ctx.editMessageText(cancelMessage, {
          parse_mode: 'HTML',
          ...Markup.inlineKeyboard([
            [Markup.button.callback('🚀 Reactivate Premium', 'upgrade_plans')],
            [Markup.button.callback('🏠 Back to Status', 'premium')]
          ])
        });
      } else {
        throw new Error('Failed to cancel subscription');
      }
    } catch (error) {
      await ctx.answerCbQuery('❌ Failed to cancel subscription. Please try again.');
    }
  }

  async confirmCancellation(ctx) {
    const confirmMessage = `
⚠️ <b>Confirm Subscription Cancellation</b>

Are you sure you want to cancel your premium subscription?

<b>You will lose access to:</b>
${Object.values(this.premiumFeatures).map(feature => `• ${feature}`).join('\n')}

<b>This action cannot be undone.</b>
`;

    const keyboard = Markup.inlineKeyboard([
      [Markup.button.callback('❌ Yes, Cancel Subscription', 'confirm_cancel')],
      [Markup.button.callback('✅ Keep My Subscription', 'manage_subscription')]
    ]);

    await ctx.editMessageText(confirmMessage, {
      parse_mode: 'HTML',
      ...keyboard
    });
  }

  // Launch the bot
  launch() {
    this.bot.launch();
    console.log('Shahcoin Premium Telegram Bot started');
    
    // Graceful shutdown
    process.once('SIGINT', () => this.bot.stop('SIGINT'));
    process.once('SIGTERM', () => this.bot.stop('SIGTERM'));
  }
}

export default PremiumTelegramMiniApp;

// Usage example:
/*
import PremiumTelegramMiniApp from './PremiumTelegramMiniApp.js';

const premiumBot = new PremiumTelegramMiniApp(
  process.env.TELEGRAM_BOT_TOKEN,
  process.env.API_BASE_URL
);

premiumBot.launch();
*/
