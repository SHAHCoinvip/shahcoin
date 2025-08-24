// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

import { evaluateSignals, type AnalyzedInput, type RiskSignal } from './aiRules';

export type SecurityScore = {
  score: number; // 0..100, higher = riskier
  badge: 'safe' | 'caution' | 'risky';
  tooltip: string;
  signals: RiskSignal[];
};

export function SHAHSecurityScore(input: AnalyzedInput): SecurityScore {
  const signals = evaluateSignals(input);

  // Base score
  let s = 10;

  // Aggregate weighted risk
  const totalWeight = signals.reduce((acc, x) => acc + x.weight, 0);
  s += Math.min(80, Math.round(totalWeight * 100));

  // Adjustments from metadata (e.g., verified/trusted)
  if (input.metadata?.isOfficial || input.metadata?.isTrusted) s = Math.max(0, s - 40);
  if (input.metadata?.onBlacklist) s = Math.min(100, s + 40);

  // Clamp
  s = Math.max(0, Math.min(100, s));

  const badge: SecurityScore['badge'] = s <= 25 ? 'safe' : s <= 60 ? 'caution' : 'risky';
  const tooltip = buildTooltip(input, s, signals);
  return { score: s, badge, tooltip, signals };
}

function buildTooltip(input: AnalyzedInput, score: number, signals: RiskSignal[]): string {
  const lines = [
    `Type: ${input.type}`,
    `Score: ${score}/100`,
    ...signals.map(s => `• ${s.label}: ${s.reason}`)
  ];
  return lines.join('\n');
}
