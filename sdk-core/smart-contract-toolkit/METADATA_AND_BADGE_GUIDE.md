# Shahcoin Token Standards Metadata & Badge System

## Overview

The Shahcoin Token Standards Metadata & Badge System provides a comprehensive framework for identifying, displaying, and managing SHI-20 and SHI-30 tokens across the Shahcoin ecosystem. This system ensures consistent token identification, verification status, and user experience across Qt Wallet, WebApp, and Telegram Mini App interfaces.

## 🏷️ Token Standards

### SHI-20 (Shahcoin Token Standard)
- **Standard**: `shahcoin.token.standard.v1`
- **Type**: `SHI-20`
- **Description**: Official fungible token standard on Shahcoin blockchain
- **Icon**: 🪙 (Silver token with "20")
- **Color**: Gray (#6B7280)
- **Background**: Light Gray (#F3F4F6)

### SHI-30 (Shahcoin NFT Standard)
- **Standard**: `shahcoin.nft.standard.v1`
- **Type**: `SHI-30`
- **Description**: Official non-fungible token standard on Shahcoin blockchain
- **Icon**: 💎 (Gold gem with "NFT")
- **Color**: Amber (#F59E0B)
- **Background**: Light Amber (#FEF3C7)

### Verified Badge
- **Icon**: ✅ (Green checkmark)
- **Description**: Verified by SHAH Admin Team
- **Color**: Green (#10B981)
- **Background**: Light Green (#D1FAE5)

## 📋 Metadata Schema

### Required Fields
```json
{
  "standard": "shahcoin.token.standard.v1" | "shahcoin.nft.standard.v1",
  "type": "SHI-20" | "SHI-30",
  "name": "Token/NFT Name",
  "symbol": "SYMB",
  "createdBy": "ShahFactory",
  "verified": false
}
```

### SHI-20 Additional Fields
```json
{
  "decimals": 18,
  "totalSupply": "1000000",
  "maxSupply": "1000000",
  "description": "Optional description",
  "website": "https://example.com",
  "logo": "https://example.com/logo.png",
  "addOns": {
    "customLogo": false,
    "verifiedBadge": false,
    "upgradeable": false,
    "airdropTool": false
  }
}
```

### SHI-30 Additional Fields
```json
{
  "description": "NFT description",
  "image": "https://example.com/image.png",
  "attributes": [
    {
      "trait_type": "Rarity",
      "value": "Legendary"
    }
  ],
  "externalUrl": "https://example.com",
  "animationUrl": "https://example.com/animation.mp4",
  "addOns": {
    "customImage": false,
    "verifiedBadge": false,
    "unlockableContent": false,
    "tradeLock": false,
    "boosterTag": false
  }
}
```

## 🔧 Implementation

### JavaScript/TypeScript (WebApp)

#### Basic Usage
```javascript
import { TokenBadge, BadgeDetectionSystem } from './metadata-standards.js';

// Create badge from metadata
const badgeSystem = new BadgeDetectionSystem();
const metadata = {
  standard: "shahcoin.token.standard.v1",
  type: "SHI-20",
  name: "My Token",
  symbol: "MTK",
  createdBy: "ShahFactory",
  verified: false
};

const badge = badgeSystem.detectTokenBadge(metadata);
console.log(badge.label); // "Official Shahcoin Token (SHI-20)"
```

#### React Component Usage
```jsx
import TokenBadge from './TokenBadge.jsx';

function TokenDisplay({ tokenData }) {
  return (
    <div className="token-item">
      <span className="token-name">{tokenData.name}</span>
      <TokenBadge 
        metadata={tokenData.metadata}
        size="medium"
        showLabel={true}
        showIcon={true}
      />
    </div>
  );
}
```

#### Badge Group
```jsx
import { TokenBadgeGroup } from './TokenBadge.jsx';

function TokenWithMultipleBadges({ tokenData }) {
  const badges = [
    { metadata: tokenData.metadata },
    { metadata: { verified: true } }
  ];

  return (
    <TokenBadgeGroup 
      badges={badges}
      size="medium"
    />
  );
}
```

### Qt C++ (Qt Wallet)

#### Basic Usage
```cpp
#include "tokenbadge.h"

// Create badge from metadata
QVariantMap metadata;
metadata["standard"] = "shahcoin.token.standard.v1";
metadata["type"] = "SHI-20";
metadata["name"] = "My Token";
metadata["symbol"] = "MTK";
metadata["createdBy"] = "ShahFactory";
metadata["verified"] = false;

TokenBadge *badge = new TokenBadge(metadata, this);
badge->setBadgeSize(TokenBadgeSize::MEDIUM);
badge->setShowLabel(true);
badge->setShowIcon(true);

// Add to layout
layout->addWidget(badge);
```

#### Badge Manager Usage
```cpp
#include "tokenbadge.h"

// Get badge manager instance
TokenBadgeManager *manager = TokenBadgeManager::instance();

// Store metadata
manager->storeMetadata(contractAddress, metadata);

// Create badge from contract address
TokenBadge *badge = manager->createBadge(contractAddress, this);

// Update verification status
manager->updateVerificationStatus(contractAddress, true);
```

#### Badge Group
```cpp
#include "tokenbadge.h"

TokenBadgeGroup *badgeGroup = new TokenBadgeGroup(this);

// Add multiple badges
TokenBadge *shi20Badge = TokenBadge::createSHI20Badge();
TokenBadge *verifiedBadge = TokenBadge::createVerifiedBadge();

badgeGroup->addBadge(shi20Badge);
badgeGroup->addBadge(verifiedBadge);
```

## 🎨 Styling

### CSS Classes
```css
/* Base badge */
.shahcoin-badge {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  padding: 4px 8px;
  border-radius: 6px;
  font-size: 12px;
  font-weight: 600;
  transition: all 0.2s ease-in-out;
}

/* SHI-20 badge */
.shahcoin-badge.badge-shi20 {
  background-color: #F3F4F6;
  color: #6B7280;
  border-color: #D1D5DB;
}

/* SHI-30 badge */
.shahcoin-badge.badge-shi30 {
  background-color: #FEF3C7;
  color: #F59E0B;
  border-color: #FCD34D;
}

/* Verified badge */
.shahcoin-badge.badge-verified {
  background-color: #D1FAE5;
  color: #10B981;
  border-color: #A7F3D0;
}

/* Badge sizes */
.shahcoin-badge.badge-sm { padding: 2px 6px; font-size: 10px; }
.shahcoin-badge.badge-md { padding: 4px 8px; font-size: 12px; }
.shahcoin-badge.badge-lg { padding: 6px 12px; font-size: 14px; }
```

### Qt Stylesheet
```cpp
QString styleSheet = QString(
    "TokenBadge {"
    "  background-color: %1;"
    "  color: %2;"
    "  border: 1px solid %2;"
    "  border-radius: 6px;"
    "  padding: 4px 8px;"
    "  font-size: 12px;"
    "  font-weight: 600;"
    "}"
).arg(bgColor, color);
```

## 🔍 Badge Detection Logic

### Primary Detection
1. **Verified Check**: If `verified: true`, show verified badge
2. **Standard Check**: Check `standard` field for SHI-20/SHI-30
3. **Type Check**: Check `type` field for SHI-20/SHI-30

### Smart Fallback Detection
```javascript
function smartFallbackDetection(metadata) {
  // Check if created by ShahFactory
  if (metadata.createdBy === "ShahFactory") {
    // Infer from properties
    if (metadata.decimals !== undefined || metadata.totalSupply !== undefined) {
      return BADGE_DEFINITIONS.SHI_20;
    }
    if (metadata.attributes || metadata.image) {
      return BADGE_DEFINITIONS.SHI_30;
    }
  }
  
  // Check for ERC-20 like properties
  if (metadata.decimals !== undefined && metadata.totalSupply !== undefined) {
    return BADGE_DEFINITIONS.SHI_20;
  }
  
  // Check for NFT like properties
  if (metadata.attributes || metadata.image || metadata.description) {
    return BADGE_DEFINITIONS.SHI_30;
  }
  
  return BADGE_DEFINITIONS.UNKNOWN;
}
```

## 📱 Platform Integration

### Qt Wallet Integration
- **File**: `src/qt/tokenbadge.h` and `src/qt/tokenbadge.cpp`
- **UI**: `src/qt/forms/tokenbadge.ui`
- **Usage**: Token lists, transaction history, token details

### WebApp Integration
- **File**: `sdk-core/smart-contract-toolkit/TokenBadge.jsx`
- **Styles**: `sdk-core/smart-contract-toolkit/badge-styles.css`
- **Usage**: Token explorer, swap interface, portfolio

### Telegram Mini App Integration
- **File**: `sdk-core/smart-contract-toolkit/metadata-standards.js`
- **Usage**: Token explorer, inline keyboards, emoji display

## 🔧 Smart Contract Integration

### SHI-20 Contract Template
```solidity
contract MyToken is ERC20, Ownable, Pausable {
    // SHI-20 Metadata
    string public constant TOKEN_STANDARD = "shahcoin.token.standard.v1";
    string public constant TOKEN_TYPE = "SHI-20";
    string public constant CREATED_BY = "ShahFactory";
    bool public verified = false;
    
    // Metadata getter
    function getTokenMetadata() public view returns (
        string memory standard,
        string memory type_,
        string memory name_,
        string memory symbol_,
        uint8 decimals_,
        uint256 totalSupply_,
        uint256 maxSupply_,
        string memory createdBy,
        bool verified_
    ) {
        return (
            TOKEN_STANDARD,
            TOKEN_TYPE,
            name(),
            symbol(),
            decimals(),
            totalSupply(),
            maxSupply,
            CREATED_BY,
            verified
        );
    }
    
    // Admin function to set verification status
    function setVerified(bool _verified) public onlyOwner {
        verified = _verified;
    }
}
```

### SHI-30 Contract Template
```solidity
contract MyNFT is ERC721, Ownable {
    // SHI-30 Metadata
    string public constant TOKEN_STANDARD = "shahcoin.nft.standard.v1";
    string public constant TOKEN_TYPE = "SHI-30";
    string public constant CREATED_BY = "ShahFactory";
    bool public verified = false;
    
    // Metadata getter
    function getTokenMetadata() public view returns (
        string memory standard,
        string memory type_,
        string memory name_,
        string memory symbol_,
        string memory createdBy,
        bool verified_
    ) {
        return (
            TOKEN_STANDARD,
            TOKEN_TYPE,
            name(),
            symbol(),
            CREATED_BY,
            verified
        );
    }
    
    // Admin function to set verification status
    function setVerified(bool _verified) public onlyOwner {
        verified = _verified;
    }
}
```

## 🗄️ Metadata Storage

### Local Storage (Qt Wallet)
```cpp
// Cache metadata locally
QString cacheFile = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + 
                   "/shahcoin/token_metadata_cache.json";

// Store metadata
void TokenBadgeManager::storeMetadata(const QString& contractAddress, const QVariantMap& metadata) {
    m_metadataCache[contractAddress] = metadata;
    saveCachedMetadata();
}
```

### Blockchain Storage
- Metadata stored on-chain via contract functions
- Verification status managed by admin functions
- Metadata retrieval via contract calls

### API Storage (WebApp)
```javascript
// Store metadata in API
async function storeMetadata(contractAddress, metadata) {
  const response = await fetch('/api/metadata', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ contractAddress, metadata })
  });
  return response.json();
}
```

## 🔄 Verification Process

### Admin Verification
1. **Manual Review**: Admin reviews token/NFT
2. **Set Verified**: Call `setVerified(true)` on contract
3. **Update Metadata**: Update metadata storage
4. **Badge Display**: Badge automatically shows verified status

### Verification Criteria
- **SHI-20**: Valid contract, proper metadata, no malicious code
- **SHI-30**: Valid NFT, proper metadata, appropriate content
- **General**: Created via ShahFactory, follows standards

## 🎯 Usage Examples

### Token List Display
```jsx
function TokenList({ tokens }) {
  return (
    <div className="token-list">
      {tokens.map(token => (
        <div key={token.address} className="token-item">
          <span className="token-name">{token.name}</span>
          <TokenBadge 
            metadata={token.metadata}
            size="small"
            showLabel={false}
          />
          <span className="token-balance">{token.balance}</span>
        </div>
      ))}
    </div>
  );
}
```

### Token Details Page
```jsx
function TokenDetails({ token }) {
  return (
    <div className="token-details">
      <h1>{token.name}</h1>
      <TokenBadgeGroup 
        badges={[
          { metadata: token.metadata },
          { metadata: { verified: token.verified } }
        ]}
        size="large"
      />
      <div className="token-info">
        <p>Symbol: {token.symbol}</p>
        <p>Total Supply: {token.totalSupply}</p>
        <p>Decimals: {token.decimals}</p>
      </div>
    </div>
  );
}
```

### Qt Wallet Token Explorer
```cpp
void TokenExplorer::addToken(const QString& name, const QVariantMap& metadata) {
    QWidget *tokenWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tokenWidget);
    
    QLabel *nameLabel = new QLabel(name, tokenWidget);
    TokenBadge *badge = new TokenBadge(metadata, tokenWidget);
    
    layout->addWidget(nameLabel);
    layout->addWidget(badge);
    layout->addStretch();
    
    m_tokenList->addWidget(tokenWidget);
}
```

## 🚀 Future Enhancements

### Planned Features
1. **Dynamic Badges**: Real-time badge updates based on on-chain events
2. **Custom Badges**: User-defined badges for special tokens
3. **Badge Analytics**: Track badge usage and verification statistics
4. **Multi-language Support**: Localized badge labels and tooltips
5. **Badge Animations**: Enhanced visual effects and transitions

### API Extensions
1. **Metadata API**: RESTful API for metadata management
2. **Verification API**: Automated verification processes
3. **Badge API**: Dynamic badge generation and management
4. **Analytics API**: Badge usage and token statistics

## 📚 Additional Resources

- [Shahcoin Token Standards Documentation](./README.md)
- [Smart Contract Toolkit Guide](./TOKEN_MONETIZATION_GUIDE.md)
- [Badge Styling Reference](./badge-styles.css)
- [Qt Integration Examples](./tokenbadge.cpp)
- [React Component API](./TokenBadge.jsx)

## 🤝 Contributing

To contribute to the metadata and badge system:

1. **Follow Standards**: Ensure all changes follow the established metadata schema
2. **Test Badge Detection**: Verify badge detection works across all platforms
3. **Update Documentation**: Keep this guide and related docs up to date
4. **Cross-platform Testing**: Test changes on Qt, WebApp, and Telegram
5. **Backward Compatibility**: Maintain compatibility with existing tokens

## 📄 License

This metadata and badge system is part of the SHAHCOIN Core project and is distributed under the MIT software license.
