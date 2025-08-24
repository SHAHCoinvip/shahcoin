// Stripe Subscription checkout for monthly AI Wallet
// Endpoint: /api/upgrade/subscribe
// Method: POST
// Body: { wallet?: string, successUrl?: string, cancelUrl?: string }

import type { NextApiRequest, NextApiResponse } from 'next';
import Stripe from 'stripe';

const STRIPE_SECRET_KEY = process.env.STRIPE_SECRET_KEY || '';
const DEFAULT_SUCCESS_URL = process.env.SUBS_SUCCESS_URL || 'https://shahcoin.app/sub/success';
const DEFAULT_CANCEL_URL = process.env.SUBS_CANCEL_URL || 'https://shahcoin.app/sub/cancel';
// Prefer using a pre-created price id for recurring plan
const STRIPE_PRICE_ID = process.env.STRIPE_MONTHLY_PRICE_ID; // e.g. price_123

const stripe = new Stripe(STRIPE_SECRET_KEY, { apiVersion: '2024-06-20' });

export default async function handler(req: NextApiRequest, res: NextApiResponse) {
  if (req.method !== 'POST') {
    res.setHeader('Allow', 'POST');
    return res.status(405).json({ error: 'Method Not Allowed' });
  }
  if (!STRIPE_SECRET_KEY) return res.status(500).json({ error: 'Stripe not configured' });

  try {
    const { wallet, successUrl, cancelUrl } = (req.body || {}) as {
      wallet?: string;
      successUrl?: string;
      cancelUrl?: string;
    };

    const lineItems: Stripe.Checkout.SessionCreateParams.LineItem[] = [];
    if (STRIPE_PRICE_ID) {
      lineItems.push({ quantity: 1, price: STRIPE_PRICE_ID });
    } else {
      // Fallback inline price (recurring $4.99/month). Prefer STRIPE_PRICE_ID in production.
      lineItems.push({
        quantity: 1,
        price_data: {
          currency: 'usd',
          recurring: { interval: 'month' },
          unit_amount: 499,
          product_data: { name: 'AI Wallet Subscription (Monthly)' },
        },
      });
    }

    const session = await stripe.checkout.sessions.create({
      mode: 'subscription',
      line_items: lineItems,
      success_url: (successUrl || DEFAULT_SUCCESS_URL) + '?session_id={CHECKOUT_SESSION_ID}',
      cancel_url: cancelUrl || DEFAULT_CANCEL_URL,
      metadata: {
        type: 'ai_wallet_subscription',
        wallet: wallet || '',
      },
    });

    return res.status(200).json({ url: session.url, id: session.id });
  } catch (e: any) {
    console.error('subscribe error', e);
    return res.status(500).json({ error: e?.message || 'Subscribe error' });
  }
}


