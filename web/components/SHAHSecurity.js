// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

export const RiskLevel = {
  VERIFIED: 'verified',
  LOW: 'low',
  MEDIUM: 'medium',
  HIGH: 'high',
  UNKNOWN: 'unknown'
};

const LOCAL_BLOCKLIST = {
  domains: ['shahco1n.com', 'shahc0in.com', 'shahcoin-airdrop.com', 'free-shah.vip', 'claim-shah.vip'],
  patterns: [/airdrop/i, /free.*mint/i, /claim.*shah/i, /connect.*wallet.*now/i, /verify.*seed/i],
  allowlist: ['shah.vip', 'app.shah.vip']
};

export function checkUrlSafety(url) {
  try {
    const u = new URL(url);
    const host = u.hostname.toLowerCase();
    if (LOCAL_BLOCKLIST.allowlist.some(d => host.endsWith(d))) {
      return { status: 'safe', reason: 'allowlist', host };
    }
    if (LOCAL_BLOCKLIST.domains.includes(host)) {
      return { status: 'phishing', reason: 'blocklist', host };
    }
    const full = url.toLowerCase();
    if (LOCAL_BLOCKLIST.patterns.some(re => re.test(full))) {
      return { status: 'phishing', reason: 'pattern', host };
    }
    return { status: 'unknown', reason: 'no_match', host };
  } catch (e) {
    return { status: 'invalid', reason: 'parse_error', host: '' };
  }
}

export function getRiskBadge(risk) {
  switch (risk) {
    case RiskLevel.VERIFIED: return { icon: '✅', color: '#28a745', label: 'Verified' };
    case RiskLevel.LOW: return { icon: '🟢', color: '#2ecc71', label: 'Low' };
    case RiskLevel.MEDIUM: return { icon: '🟡', color: '#f1c40f', label: 'Medium' };
    case RiskLevel.HIGH: return { icon: '🔴', color: '#e74c3c', label: 'High' };
    default: return { icon: '⚪', color: '#95a5a6', label: 'Unknown' };
  }
}

export function isOfficialContract(address, registry) {
  const a = (address || '').toLowerCase();
  if (!registry) return false;
  return (registry.official || []).some(x => (x.address || '').toLowerCase() === a);
}

export function isTrustedContract(address, registry) {
  const a = (address || '').toLowerCase();
  if (!registry) return false;
  return (registry.trusted || []).some(x => (x.address || '').toLowerCase() === a) || isOfficialContract(a, registry);
}

export async function fetchJson(url) {
  try {
    const res = await fetch(url, { cache: 'no-cache' });
    if (!res.ok) throw new Error('Network error');
    return await res.json();
  } catch (e) {
    return null;
  }
}

export async function loadLocalRegistry() {
  // In dev, this may be served by the app. In prod, prefer remote signed source.
  return null;
}

export function shouldAutoBlock({ urlStatus, riskLevel }, userPrefs) {
  if (!userPrefs?.autoBlock) return false;
  if (urlStatus === 'phishing') return true;
  if (riskLevel === RiskLevel.HIGH) return true;
  return false;
}
