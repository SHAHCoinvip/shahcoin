// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

import type { AnalyzedInput } from '../engine/aiRules';

export type ContractMeta = {
  name?: string;
  symbol?: string;
  isOfficial?: boolean;
  isTrusted?: boolean;
  missingMetadata?: boolean;
  largeDumps?: boolean;
  centralizedSupply?: boolean;
  metadataUrl?: string;
};

export function analyzeContract(address: string, meta: ContractMeta = {}, kind: 'token' | 'nft' = 'token'): AnalyzedInput {
  const metadata: Record<string, any> = { ...meta };
  if (meta.metadataUrl) metadata.url = meta.metadataUrl;
  return { type: kind, value: address, metadata };
}
