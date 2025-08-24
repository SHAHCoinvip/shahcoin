// SHAH & SHAHtoken price oracle
// Endpoint: /api/upgrade/price
// Method: GET
// Response: { usdPrice, requiredShah, requiredShahToken }

import type { NextApiRequest, NextApiResponse } from 'next';

const PREMIUM_PRICE_USD = 9.99;

async function getFromShahSwap(): Promise<{ shah?: number; shahtoken?: number }> {
  try {
    // TODO: Replace with actual ShahSwap endpoints
    // Example (pseudo):
    // const r1 = await fetch('https://api.shahswap.xyz/v1/price/SHAHUSD');
    // const { price: shah } = await r1.json();
    // const r2 = await fetch('https://api.shahswap.xyz/v1/price/SHAHTOKENUSD');
    // const { price: shahtoken } = await r2.json();
    return { shah: undefined, shahtoken: undefined };
  } catch {
    return {};
  }
}

async function getFromGeckoTerminal(): Promise<{ shah?: number; shahtoken?: number }> {
  try {
    // TODO: Replace with GeckoTerminal pair endpoints
    return { shah: undefined, shahtoken: undefined };
  } catch {
    return {};
  }
}

function pickPrice(a?: number, b?: number, fallback = 0.10) {
  if (a && a > 0) return a;
  if (b && b > 0) return b;
  return fallback; // safe fallback
}

export default async function handler(req: NextApiRequest, res: NextApiResponse) {
  try {
    const [s1, s2] = await Promise.all([getFromShahSwap(), getFromGeckoTerminal()]);

    const usdPrice = pickPrice(s1.shah, s2.shah, 0.10);
    const usdPriceToken = pickPrice(s1.shahtoken, s2.shahtoken, usdPrice);

    const requiredShah = Math.round((PREMIUM_PRICE_USD / usdPrice) * 1e4) / 1e4;
    const requiredShahToken = Math.round((PREMIUM_PRICE_USD / usdPriceToken) * 1e4) / 1e4;

    return res.status(200).json({
      usdPrice,
      requiredShah,
      requiredShahToken,
    });
  } catch (e: any) {
    return res.status(500).json({ error: e?.message || 'oracle-error' });
  }
}


