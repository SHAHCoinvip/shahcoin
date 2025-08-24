// Shahcoin Marketplace API (client-side SDK)
// In-memory demo; replace data loaders with real backend/chain indexers

function isVerified(nft) {
  const tier = (nft && nft.tier) || (nft.metadata && nft.metadata.tier) || 'Basic';
  const hasSchema = nft && nft.metadata && typeof nft.metadata === 'object';
  const adminVerified = !!(nft && nft.metadata && nft.metadata.adminVerified);
  return adminVerified || ((tier === 'Pro' || tier === 'Premium') && hasSchema);
}

export async function getVerifiedNFTs({ limit = 50, offset = 0, filters = {} } = {}) {
  const all = await getAllNFTs({ limit: 1000, offset: 0, filters });
  const curated = all.filter(isVerified);
  return curated.slice(offset, offset + limit);
}

export async function getAllNFTs({ limit = 50, offset = 0, filters = {} } = {}) {
  // TODO: replace with chain indexer or backend call
  const sample = [
    {
      id: 'NFT_001',
      name: 'Shahcoin Genesis NFT',
      description: 'The first NFT ever minted on Shahcoin',
      imageURI: '',
      creator: 'S1a2h3c4o5i6n7G8e9n0e',
      owner: 'S1a2h3c4o5i6n7G8e9n0e',
      blockCreated: 12340,
      metadata: { tier: 'Premium', rarity: 'legendary', adminVerified: true },
      tier: 'Premium',
      verified: true,
    },
    {
      id: 'NFT_002',
      name: 'Shahcoin Art Collection #1',
      description: 'Beautiful digital art piece',
      imageURI: '',
      creator: 'S1a2h3c4o5i6n7A8r9t0i',
      owner: 'S1a2h3c4o5i6n7C0o1l2l3e',
      blockCreated: 12355,
      metadata: { tier: 'Basic', rarity: 'rare' },
      tier: 'Basic',
      verified: false,
    },
  ];

  // Simple filtering hooks
  let results = sample;
  if (filters.creator) results = results.filter(n => n.creator === filters.creator);
  if (filters.tier) results = results.filter(n => (n.tier || (n.metadata && n.metadata.tier)) === filters.tier);

  return results.slice(offset, offset + limit);
}

export async function getCreatorProfile(address) {
  // TODO: replace with analytics backend
  const nfts = await getAllNFTs({ filters: { creator: address }, limit: 1000 });
  const totalMinted = nfts.length;
  const trustedCreator = totalMinted >= 10; // placeholder threshold
  return {
    address,
    totalMinted,
    salesVolumeUSD: 0,
    averagePriceUSD: 0,
    refundComplaints: 0,
    trustedCreator,
    badges: trustedCreator ? ['Trusted Creator'] : [],
  };
}

// Backend client stubs
const API_BASE = process.env.SHAH_MARKETPLACE_API || 'https://api.shah.vip/marketplace';

export async function submitListing({ nftId, title, description, category, tags, priceSatoshis, priceUSD, submitForVerification }) {
  try {
    const res = await fetch(`${API_BASE}/listings`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ nftId, title, description, category, tags, priceSatoshis, priceUSD, submitForVerification })
    });
    if (!res.ok) throw new Error(`Listing failed: ${res.status}`);
    return await res.json();
  } catch (e) {
    console.warn('submitListing error', e);
    return { ok: false, error: e.message };
  }
}

export async function recordPurchase({ nftId, buyer, priceSatoshis, priceUSD, txid }) {
  try {
    const res = await fetch(`${API_BASE}/purchases`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ nftId, buyer, priceSatoshis, priceUSD, txid })
    });
    if (!res.ok) throw new Error(`Purchase record failed: ${res.status}`);
    return await res.json();
  } catch (e) {
    console.warn('recordPurchase error', e);
    return { ok: false, error: e.message };
  }
}

export async function setAdminVerificationFlag({ nftId, adminVerified }) {
  try {
    const res = await fetch(`${API_BASE}/verify`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ nftId, adminVerified })
    });
    if (!res.ok) throw new Error(`Verify flag failed: ${res.status}`);
    return await res.json();
  } catch (e) {
    console.warn('setAdminVerificationFlag error', e);
    return { ok: false, error: e.message };
  }
}
