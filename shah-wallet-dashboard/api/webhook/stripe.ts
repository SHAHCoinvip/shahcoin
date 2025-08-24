// Stripe webhook to mark upgrades as complete
// Endpoint: /api/webhook/stripe
// Method: POST
// Configure your Stripe endpoint secret in STRIPE_WEBHOOK_SECRET

import type { NextApiRequest, NextApiResponse } from 'next';
import Stripe from 'stripe';

export const config = {
  api: {
    bodyParser: false, // Stripe requires raw body
  },
};

const STRIPE_SECRET_KEY = process.env.STRIPE_SECRET_KEY || '';
const STRIPE_WEBHOOK_SECRET = process.env.STRIPE_WEBHOOK_SECRET || '';
const stripe = new Stripe(STRIPE_SECRET_KEY, { apiVersion: '2024-06-20' });

function buffer(readable: any) {
  return new Promise<Buffer>((resolve, reject) => {
    const chunks: any[] = [];
    readable.on('data', (chunk: any) => chunks.push(Buffer.from(chunk)));
    readable.on('end', () => resolve(Buffer.concat(chunks)));
    readable.on('error', reject);
  });
}

export default async function handler(req: NextApiRequest, res: NextApiResponse) {
  if (req.method !== 'POST') {
    res.setHeader('Allow', 'POST');
    return res.status(405).end('Method Not Allowed');
  }
  if (!STRIPE_WEBHOOK_SECRET) return res.status(500).end('Webhook not configured');

  let event: Stripe.Event;
  const buf = await buffer(req);
  const sig = req.headers['stripe-signature'] as string;
  try {
    event = stripe.webhooks.constructEvent(buf, sig, STRIPE_WEBHOOK_SECRET);
  } catch (err: any) {
    console.error('⚠️ Webhook signature verification failed.', err);
    return res.status(400).send(`Webhook Error: ${err.message}`);
  }

  try {
    switch (event.type) {
      case 'checkout.session.completed': {
        const session = event.data.object as Stripe.Checkout.Session;
        const wallet = session.metadata?.wallet || '';
        if (session.mode === 'payment') {
          // One-time upgrade
          console.log('Upgrade complete for wallet:', wallet, 'session:', session.id);
          // TODO: mark wallet premium in DB
        }
        if (session.mode === 'subscription') {
          // Monthly subscription
          console.log('Subscription started for wallet:', wallet, 'session:', session.id);
          // TODO: mark wallet subscription active in DB and set renewal due +30 days (or use Stripe sub period end)
        }
        break;
      }
      default:
        break;
    }
    res.status(200).json({ received: true });
  } catch (e: any) {
    console.error('webhook error', e);
    res.status(500).json({ error: e?.message || 'webhook-error' });
  }
}


