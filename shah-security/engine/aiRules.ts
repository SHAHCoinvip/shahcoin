// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

export type RiskSignal = {
  id: string;
  label: string;
  weight: number; // 0..1
  reason: string;
};

export type AnalyzedInput = {
  type: 'wallet' | 'token' | 'nft' | 'url';
  value: string;
  metadata?: Record<string, any>;
};

// Simple heuristics; in production, enrich with ML outputs
export function evaluateSignals(input: AnalyzedInput): RiskSignal[] {
  const signals: RiskSignal[] = [];
  const v = (input.value || '').toLowerCase();

  // Suspicious naming / impersonation
  if (input.type === 'token' || input.type === 'nft') {
    const name = (input.metadata?.name || '').toLowerCase();
    const symbol = (input.metadata?.symbol || '').toLowerCase();
    if (/shahcoin|shah\b/.test(name) || /shah/.test(symbol)) {
      if (!input.metadata?.isOfficial) {
        signals.push({ id: 'impersonation', label: 'Impersonation risk', weight: 0.5, reason: 'Uses SHAH brand without verification' });
      }
    }
    if (/(airdrop|free|claim|pump)/.test(name)) {
      signals.push({ id: 'promo-bait', label: 'Promotional bait', weight: 0.4, reason: 'Suspicious promo wording' });
    }
  }

  // URL patterns
  if (input.type === 'url') {
    if (/(airdrop|free.*mint|claim.*shah|connect.*wallet.*now|verify.*seed)/i.test(v)) {
      signals.push({ id: 'phishing-pattern', label: 'Phishing pattern', weight: 0.7, reason: 'Known phishing phrase' });
    }
    try {
      const host = new URL(input.value).hostname.toLowerCase();
      if (/shahco1n|shahc0in|airdrop|free|claim/.test(host)) {
        signals.push({ id: 'suspicious-domain', label: 'Suspicious domain', weight: 0.6, reason: `Host looks suspicious: ${host}` });
      }
    } catch {}
  }

  // Transfer/centralization hints (metadata precomputed)
  if (input.metadata?.largeDumps) {
    signals.push({ id: 'large-dumps', label: 'Large dumps', weight: 0.6, reason: 'Detected large dumps' });
  }
  if (input.metadata?.centralizedSupply) {
    signals.push({ id: 'centralized', label: 'Centralized supply', weight: 0.5, reason: 'Supply concentrated in few wallets' });
  }
  if (input.metadata?.missingMetadata) {
    signals.push({ id: 'missing-meta', label: 'Missing metadata', weight: 0.3, reason: 'Token/NFT lacks proper metadata' });
  }

  return signals;
}
