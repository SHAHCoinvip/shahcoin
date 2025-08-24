// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

import type { AnalyzedInput } from '../engine/aiRules';

export function analyzeUrl(url: string, opts?: { allowlist?: string[]; blocklist?: string[] }): AnalyzedInput {
  const metadata: Record<string, any> = {};
  try {
    const u = new URL(url);
    metadata.host = u.hostname.toLowerCase();
    metadata.path = u.pathname;
  } catch {}

  const allow = opts?.allowlist || ['shah.vip', 'app.shah.vip'];
  const block = opts?.blocklist || ['shahco1n.com', 'shahc0in.com', 'shahcoin-airdrop.com', 'free-shah.vip', 'claim-shah.vip'];

  const host = (metadata.host || '').toLowerCase();
  if (block.includes(host)) metadata.onBlacklist = true;
  if (allow.some(d => host.endsWith(d))) metadata.isTrusted = true;

  return { type: 'url', value: url, metadata };
}
