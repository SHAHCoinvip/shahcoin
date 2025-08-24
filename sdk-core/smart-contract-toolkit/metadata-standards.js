/**
 * Shahcoin Token Standards Metadata & Badge Detection System
 * 
 * This module defines the metadata structure for SHI-20 and SHI-30 tokens,
 * along with badge detection and UI display logic across the Shahcoin ecosystem.
 */

// Metadata Schema Constants
export const METADATA_STANDARDS = {
  SHI_20: {
    standard: "shahcoin.token.standard.v1",
    type: "SHI-20",
    version: "1.0.0"
  },
  SHI_30: {
    standard: "shahcoin.nft.standard.v1", 
    type: "SHI-30",
    version: "1.0.0"
  }
};

// Badge Definitions
export const BADGE_DEFINITIONS = {
  SHI_20: {
    icon: "🪙",
    iconCode: "&#x1FA99;", // Silver token with "20"
    label: "Official Shahcoin Token (SHI-20)",
    tooltip: "Created via ShahFactory - Official Shahcoin Standard",
    color: "#6B7280", // Gray
    bgColor: "#F3F4F6"
  },
  SHI_30: {
    icon: "💎", 
    iconCode: "&#x1F48E;", // Gold gem with "NFT"
    label: "Official Shahcoin NFT (SHI-30)",
    tooltip: "Created via ShahFactory - Official Shahcoin Standard",
    color: "#F59E0B", // Amber
    bgColor: "#FEF3C7"
  },
  VERIFIED: {
    icon: "✅",
    iconCode: "&#x2705;", // Green checkmark
    label: "Verified by SHAH Admin Team",
    tooltip: "This asset has been verified by the Shahcoin Admin Team",
    color: "#10B981", // Green
    bgColor: "#D1FAE5"
  },
  UNKNOWN: {
    icon: "❓",
    iconCode: "&#x2753;", // Question mark
    label: "Unknown Token",
    tooltip: "Token type could not be determined",
    color: "#9CA3AF", // Gray
    bgColor: "#F9FAFB"
  }
};

/**
 * Token/NFT Metadata Schema
 * Enforces the required fields during token or NFT creation
 */
export class ShahcoinMetadataSchema {
  constructor(type, config = {}) {
    this.type = type; // "SHI-20" or "SHI-30"
    this.config = config;
  }

  /**
   * Generate metadata for SHI-20 token
   */
  static generateSHI20Metadata(tokenConfig) {
    return {
      standard: METADATA_STANDARDS.SHI_20.standard,
      type: METADATA_STANDARDS.SHI_20.type,
      name: tokenConfig.name,
      symbol: tokenConfig.symbol,
      decimals: tokenConfig.decimals || 18,
      totalSupply: tokenConfig.totalSupply,
      maxSupply: tokenConfig.maxSupply,
      createdBy: "ShahFactory",
      verified: false,
      createdAt: new Date().toISOString(),
      version: METADATA_STANDARDS.SHI_20.version,
      // Optional fields
      description: tokenConfig.description,
      website: tokenConfig.website,
      logo: tokenConfig.logo,
      // Add-ons metadata
      addOns: {
        customLogo: tokenConfig.customLogo || false,
        verifiedBadge: tokenConfig.verifiedBadge || false,
        upgradeable: tokenConfig.upgradeable || false,
        airdropTool: tokenConfig.airdropTool || false
      }
    };
  }

  /**
   * Generate metadata for SHI-30 NFT
   */
  static generateSHI30Metadata(nftConfig) {
    return {
      standard: METADATA_STANDARDS.SHI_30.standard,
      type: METADATA_STANDARDS.SHI_30.type,
      name: nftConfig.name,
      symbol: nftConfig.symbol,
      description: nftConfig.description,
      image: nftConfig.image,
      createdBy: "ShahFactory",
      verified: false,
      createdAt: new Date().toISOString(),
      version: METADATA_STANDARDS.SHI_30.version,
      // NFT specific fields
      attributes: nftConfig.attributes || [],
      externalUrl: nftConfig.externalUrl,
      animationUrl: nftConfig.animationUrl,
      // Add-ons metadata
      addOns: {
        customImage: nftConfig.customImage || false,
        verifiedBadge: nftConfig.verifiedBadge || false,
        unlockableContent: nftConfig.unlockableContent || false,
        tradeLock: nftConfig.tradeLock || false,
        boosterTag: nftConfig.boosterTag || false
      }
    };
  }

  /**
   * Validate metadata structure
   */
  static validateMetadata(metadata) {
    const requiredFields = ['standard', 'type', 'name', 'createdBy', 'verified'];
    const missingFields = requiredFields.filter(field => !metadata[field]);
    
    if (missingFields.length > 0) {
      throw new Error(`Missing required metadata fields: ${missingFields.join(', ')}`);
    }

    // Validate standard
    const validStandards = Object.values(METADATA_STANDARDS).map(s => s.standard);
    if (!validStandards.includes(metadata.standard)) {
      throw new Error(`Invalid standard: ${metadata.standard}`);
    }

    // Validate type
    const validTypes = Object.values(METADATA_STANDARDS).map(s => s.type);
    if (!validTypes.includes(metadata.type)) {
      throw new Error(`Invalid type: ${metadata.type}`);
    }

    return true;
  }
}

/**
 * Badge Detection and Display System
 */
export class BadgeDetectionSystem {
  constructor() {
    this.cache = new Map();
  }

  /**
   * Detect token type and get badge information
   */
  detectTokenBadge(metadata) {
    if (!metadata) {
      return BADGE_DEFINITIONS.UNKNOWN;
    }

    // Check if verified
    if (metadata.verified === true) {
      return BADGE_DEFINITIONS.VERIFIED;
    }

    // Check for SHI-20
    if (metadata.type === METADATA_STANDARDS.SHI_20.type || 
        metadata.standard === METADATA_STANDARDS.SHI_20.standard) {
      return BADGE_DEFINITIONS.SHI_20;
    }

    // Check for SHI-30
    if (metadata.type === METADATA_STANDARDS.SHI_30.type || 
        metadata.standard === METADATA_STANDARDS.SHI_30.standard) {
      return BADGE_DEFINITIONS.SHI_30;
    }

    // Smart fallback detection
    return this.smartFallbackDetection(metadata);
  }

  /**
   * Smart fallback detection for unknown tokens
   */
  smartFallbackDetection(metadata) {
    // Check if created by ShahFactory
    if (metadata.createdBy === "ShahFactory") {
      // Infer from other properties
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

  /**
   * Get badge HTML for UI display
   */
  getBadgeHTML(badge, size = "medium") {
    const sizeClasses = {
      small: "badge-sm",
      medium: "badge-md", 
      large: "badge-lg"
    };

    return `
      <span class="shahcoin-badge ${sizeClasses[size]}" 
            style="background-color: ${badge.bgColor}; color: ${badge.color};"
            title="${badge.tooltip}">
        <span class="badge-icon">${badge.icon}</span>
        <span class="badge-label">${badge.label}</span>
      </span>
    `;
  }

  /**
   * Get badge CSS classes for styling
   */
  getBadgeCSS(badge) {
    return {
      'shahcoin-badge': true,
      'badge-shi20': badge === BADGE_DEFINITIONS.SHI_20,
      'badge-shi30': badge === BADGE_DEFINITIONS.SHI_30,
      'badge-verified': badge === BADGE_DEFINITIONS.VERIFIED,
      'badge-unknown': badge === BADGE_DEFINITIONS.UNKNOWN
    };
  }
}

/**
 * Qt Wallet Badge Integration
 */
export class QtBadgeIntegration {
  constructor() {
    this.badgeSystem = new BadgeDetectionSystem();
  }

  /**
   * Get badge for Qt Wallet token display
   */
  getQtTokenBadge(tokenData) {
    const badge = this.badgeSystem.detectTokenBadge(tokenData.metadata);
    
    return {
      icon: badge.icon,
      label: badge.label,
      tooltip: badge.tooltip,
      color: badge.color,
      bgColor: badge.bgColor,
      // Qt specific properties
      qtIcon: this.getQtIcon(badge),
      qtStyle: this.getQtStyle(badge)
    };
  }

  /**
   * Get Qt icon representation
   */
  getQtIcon(badge) {
    const iconMap = {
      [BADGE_DEFINITIONS.SHI_20]: ":/icons/token_silver",
      [BADGE_DEFINITIONS.SHI_30]: ":/icons/nft_gold", 
      [BADGE_DEFINITIONS.VERIFIED]: ":/icons/verified_check",
      [BADGE_DEFINITIONS.UNKNOWN]: ":/icons/token_unknown"
    };
    return iconMap[badge] || iconMap[BADGE_DEFINITIONS.UNKNOWN];
  }

  /**
   * Get Qt style sheet
   */
  getQtStyle(badge) {
    return `
      QLabel {
        background-color: ${badge.bgColor};
        color: ${badge.color};
        border: 1px solid ${badge.color};
        border-radius: 4px;
        padding: 2px 6px;
        font-size: 10px;
        font-weight: bold;
      }
    `;
  }
}

/**
 * WebApp Badge Integration
 */
export class WebAppBadgeIntegration {
  constructor() {
    this.badgeSystem = new BadgeDetectionSystem();
  }

  /**
   * Get badge for WebApp token display
   */
  getWebAppTokenBadge(tokenData) {
    const badge = this.badgeSystem.detectTokenBadge(tokenData.metadata);
    
    return {
      html: this.badgeSystem.getBadgeHTML(badge),
      css: this.badgeSystem.getBadgeCSS(badge),
      data: badge
    };
  }

  /**
   * Get React component props
   */
  getReactBadgeProps(tokenData) {
    const badge = this.badgeSystem.detectTokenBadge(tokenData.metadata);
    
    return {
      icon: badge.icon,
      label: badge.label,
      tooltip: badge.tooltip,
      color: badge.color,
      bgColor: badge.bgColor,
      className: Object.keys(this.badgeSystem.getBadgeCSS(badge)).join(' ')
    };
  }
}

/**
 * Telegram Mini App Badge Integration
 */
export class TelegramBadgeIntegration {
  constructor() {
    this.badgeSystem = new BadgeDetectionSystem();
  }

  /**
   * Get badge for Telegram Mini App
   */
  getTelegramTokenBadge(tokenData) {
    const badge = this.badgeSystem.detectTokenBadge(tokenData.metadata);
    
    return {
      emoji: badge.icon,
      text: badge.label,
      tooltip: badge.tooltip,
      // Telegram specific formatting
      telegramText: `${badge.icon} ${badge.label}`,
      inlineKeyboard: this.getTelegramInlineKeyboard(badge)
    };
  }

  /**
   * Get Telegram inline keyboard for badge info
   */
  getTelegramInlineKeyboard(badge) {
    return [
      [{
        text: `${badge.icon} ${badge.label}`,
        callback_data: `badge_info_${badge.type || 'unknown'}`
      }]
    ];
  }
}

/**
 * Metadata Storage and Retrieval
 */
export class MetadataStorage {
  constructor() {
    this.storage = new Map();
  }

  /**
   * Store metadata for a token/NFT
   */
  storeMetadata(contractAddress, metadata) {
    // Validate metadata
    ShahcoinMetadataSchema.validateMetadata(metadata);
    
    // Store with timestamp
    this.storage.set(contractAddress.toLowerCase(), {
      ...metadata,
      storedAt: new Date().toISOString(),
      lastAccessed: new Date().toISOString()
    });
  }

  /**
   * Retrieve metadata for a token/NFT
   */
  getMetadata(contractAddress) {
    const metadata = this.storage.get(contractAddress.toLowerCase());
    if (metadata) {
      metadata.lastAccessed = new Date().toISOString();
    }
    return metadata;
  }

  /**
   * Update verification status
   */
  updateVerificationStatus(contractAddress, verified) {
    const metadata = this.getMetadata(contractAddress);
    if (metadata) {
      metadata.verified = verified;
      metadata.verifiedAt = verified ? new Date().toISOString() : null;
      this.storeMetadata(contractAddress, metadata);
    }
  }

  /**
   * Get all tokens by type
   */
  getTokensByType(type) {
    const tokens = [];
    for (const [address, metadata] of this.storage) {
      if (metadata.type === type) {
        tokens.push({ address, metadata });
      }
    }
    return tokens;
  }

  /**
   * Get verified tokens
   */
  getVerifiedTokens() {
    const tokens = [];
    for (const [address, metadata] of this.storage) {
      if (metadata.verified) {
        tokens.push({ address, metadata });
      }
    }
    return tokens;
  }
}

// Export main classes and utilities
export default {
  METADATA_STANDARDS,
  BADGE_DEFINITIONS,
  ShahcoinMetadataSchema,
  BadgeDetectionSystem,
  QtBadgeIntegration,
  WebAppBadgeIntegration,
  TelegramBadgeIntegration,
  MetadataStorage
};
