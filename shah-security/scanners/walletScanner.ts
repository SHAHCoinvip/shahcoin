// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

import type { AnalyzedInput } from '../engine/aiRules';

export type WalletReputation = {
  onBlacklist?: boolean;
  rugpullHistory?: boolean;
  spamNfts?: boolean;
  drainingTxs?: boolean;
};

export function analyzeWallet(address: string, rep: WalletReputation = {}): AnalyzedInput {
  const metadata: Record<string, any> = { ...rep };
  if (rep.onBlacklist || rep.rugpullHistory || rep.drainingTxs) metadata.onBlacklist = true;
  if (rep.spamNfts) metadata.spam = true;
  return { type: 'wallet', value: address, metadata };
}
