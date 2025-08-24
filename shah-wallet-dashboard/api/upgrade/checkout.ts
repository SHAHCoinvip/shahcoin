// Stripe Checkout session creator for AI Wallet upgrade
// Endpoint: /api/upgrade/checkout
// Method: POST
// Body (JSON): { wallet?: string, successUrl?: string, cancelUrl?: string }

import type { NextApiRequest, NextApiResponse } from 'next';
import Stripe from 'stripe';

const STRIPE_SECRET_KEY = process.env.STRIPE_SECRET_KEY || '';
const DEFAULT_SUCCESS_URL = process.env.CHECKOUT_SUCCESS_URL || 'https://shahcoin.app/upgrade/success';
const DEFAULT_CANCEL_URL = process.env.CHECKOUT_CANCEL_URL || 'https://shahcoin.app/upgrade/cancel';

const stripe = new Stripe(STRIPE_SECRET_KEY, { apiVersion: '2024-06-20' });

export default async function handler(req: NextApiRequest, res: NextApiResponse) {
  if (req.method !== 'POST') {
    res.setHeader('Allow', 'POST');
    return res.status(405).json({ error: 'Method Not Allowed' });
  }

  try {
    const { wallet, successUrl, cancelUrl } = (req.body || {}) as {
      wallet?: string;
      successUrl?: string;
      cancelUrl?: string;
    };

    if (!STRIPE_SECRET_KEY) {
      return res.status(500).json({ error: 'Stripe not configured' });
    }

    const session = await stripe.checkout.sessions.create({
      mode: 'payment',
      submit_type: 'pay',
      line_items: [
        {
          quantity: 1,
          price_data: {
            currency: 'usd',
            unit_amount: 999, // $9.99
            product_data: {
              name: 'AI Wallet Upgrade',
              description: 'One-time upgrade to Premium AI Wallet access',
            },
          },
        },
      ],
      allow_promotion_codes: false,
      success_url: (successUrl || DEFAULT_SUCCESS_URL) + '?session_id={CHECKOUT_SESSION_ID}',
      cancel_url: cancelUrl || DEFAULT_CANCEL_URL,
      metadata: {
        type: 'ai_wallet_upgrade',
        wallet: wallet || '',
      },
    });

    return res.status(200).json({ url: session.url, id: session.id });
  } catch (e: any) {
    console.error('checkout error', e);
    return res.status(500).json({ error: e?.message || 'Checkout error' });
  }
}


