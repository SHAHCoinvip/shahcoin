# Shahcoin Marketplace API (Curated SHI-30 Listings)

Purpose
- Provide curated/verified SHI-30 NFT listings and creator profiles to WebApp and Telegram Mini App.
- Enforce quality and security defaults: verified-by-tier, schema-conformant metadata, optional admin approval (future).

Endpoints (SDK functions)
- getVerifiedNFTs({ limit = 50, offset = 0, filters = {} }): Promise<NFT[]> — Returns curated marketplace NFTs (default: verified-only).
- getAllNFTs({ limit = 50, offset = 0, filters = {} }): Promise<NFT[]> — Returns all NFTs (unfiltered).
- getCreatorProfile(address): Promise<CreatorProfile> — Returns creator KPIs and badges.

Verification Rules (v1)
- Verified if:
  - Tier is Pro or Premium
  - Metadata parses as valid JSON object and includes standard fields
  - (Future) Admin approval or registry flag

Data Shapes
- NFT
  - id: string
  - name: string
  - description: string
  - imageURI: string
  - creator: string (wallet address)
  - owner: string (wallet address)
  - blockCreated: number
  - metadata: object (must include `tier`)
  - tier: 'Basic' | 'Pro' | 'Premium'
  - verified: boolean

- CreatorProfile
  - address: string
  - totalMinted: number
  - salesVolumeUSD: number (future)
  - averagePriceUSD: number (future)
  - refundComplaints: number (future)
  - trustedCreator: boolean
  - badges: string[]

Usage
```javascript
import { getVerifiedNFTs, getCreatorProfile } from './index.js';

const nfts = await getVerifiedNFTs({ limit: 24 });
const profile = await getCreatorProfile('S1abc...');
```

Integration Notes
- WebApp and Telegram should default to curated listings (verified-only) with an explicit toggle to view all.
- Qt Wallet now supports a Verified Only filter; keep consistent across platforms.
- Future: add server endpoints and persistence; this SDK can point to `https://api.shah.vip/marketplace`.
