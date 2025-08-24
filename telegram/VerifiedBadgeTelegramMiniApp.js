// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

/**
 * Shahcoin Verified Badge System for Telegram Mini App
 * Handles verified badges, legal warnings, and filtering for tokens/NFTs
 */

class VerifiedBadgeTelegramMiniApp {
    constructor() {
        this.showVerifiedOnly = false;
        this.verifiedAssets = new Map();
        this.badgeTypes = {
            VERIFIED_TOKEN: 'verified-token',
            VERIFIED_NFT: 'verified-nft',
            TRUSTED_CREATOR: 'trusted-creator',
            LEGAL_WARNING: 'legal-warning'
        };
        
        this.tooltips = {
            [this.badgeTypes.VERIFIED_TOKEN]: "🔐 This token is officially verified by the SHAHCOIN team. Smart contract and metadata have been reviewed for security and compliance.\n\nSHAHCOIN® is a registered trademark. All rights reserved.",
            [this.badgeTypes.VERIFIED_NFT]: "🖼 This NFT is part of a trusted, verified collection. Creator and assets meet SHAHCOIN quality standards.\n\nSHAHCOIN® is a registered trademark. All rights reserved.",
            [this.badgeTypes.TRUSTED_CREATOR]: "This creator has been verified and trusted by the SHAHCOIN team.",
            [this.badgeTypes.LEGAL_WARNING]: "⚠️ You may not use \"SHAH\", \"SHAHCOIN\" or any official branding in your project name or logo unless explicitly authorized.\n\nSee LICENSE and NOTICE files for legal terms.\n\nFor licensing inquiries: Info@shah.vip"
        };
    }

    /**
     * Create verified badge SVG element
     */
    createBadgeSVG(type) {
        const svgMap = {
            [this.badgeTypes.VERIFIED_TOKEN]: `
                <svg width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <defs>
                        <linearGradient id="goldGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                            <stop offset="0%" style="stop-color:#FFD700;stop-opacity:1" />
                            <stop offset="50%" style="stop-color:#FFA500;stop-opacity:1" />
                            <stop offset="100%" style="stop-color:#FF8C00;stop-opacity:1" />
                        </linearGradient>
                        <filter id="glow">
                            <feGaussianBlur stdDeviation="2" result="coloredBlur"/>
                            <feMerge> 
                                <feMergeNode in="coloredBlur"/>
                                <feMergeNode in="SourceGraphic"/>
                            </feMerge>
                        </filter>
                    </defs>
                    <circle cx="12" cy="12" r="11" fill="url(#goldGradient)" stroke="#B8860B" stroke-width="2" filter="url(#glow)"/>
                    <path d="M9 15L7 13L8.4 11.6L9 12.2L15.6 5.6L17 7L9 15Z" fill="white" stroke="white" stroke-width="0.5"/>
                    <text x="12" y="21" text-anchor="middle" font-size="7" font-weight="bold" fill="white" font-family="Arial, sans-serif">SHAH</text>
                </svg>
            `,
            [this.badgeTypes.VERIFIED_NFT]: `
                <svg width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <defs>
                        <linearGradient id="purpleGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                            <stop offset="0%" style="stop-color:#9C27B0;stop-opacity:1" />
                            <stop offset="50%" style="stop-color:#673AB7;stop-opacity:1" />
                            <stop offset="100%" style="stop-color:#3F51B5;stop-opacity:1" />
                        </linearGradient>
                        <filter id="nftGlow">
                            <feGaussianBlur stdDeviation="1.5" result="coloredBlur"/>
                            <feMerge> 
                                <feMergeNode in="coloredBlur"/>
                                <feMergeNode in="SourceGraphic"/>
                            </feMerge>
                        </filter>
                    </defs>
                    <polygon points="12,2 15.09,8.26 22,9.27 17,14.14 18.18,21.02 12,17.77 5.82,21.02 7,14.14 2,9.27 8.91,8.26" 
                             fill="url(#purpleGradient)" stroke="#4A148C" stroke-width="1.5" filter="url(#nftGlow)"/>
                    <path d="M9 15L7 13L8.4 11.6L9 12.2L15.6 5.6L17 7L9 15Z" fill="white" stroke="white" stroke-width="0.5"/>
                    <text x="12" y="22" text-anchor="middle" font-size="6" font-weight="bold" fill="white" font-family="Arial, sans-serif">VERIFIED</text>
                </svg>
            `,
            [this.badgeTypes.TRUSTED_CREATOR]: `
                <svg width="20" height="20" viewBox="0 0 20 20" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <defs>
                        <linearGradient id="tealGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                            <stop offset="0%" style="stop-color:#00BCD4;stop-opacity:1" />
                            <stop offset="50%" style="stop-color:#009688;stop-opacity:1" />
                            <stop offset="100%" style="stop-color:#4CAF50;stop-opacity:1" />
                        </linearGradient>
                    </defs>
                    <circle cx="10" cy="10" r="9" fill="url(#tealGradient)" stroke="#00695C" stroke-width="1.5"/>
                    <path d="M10 3L12.09 7.26L16.5 8.27L13.5 11.14L14.18 15.52L10 13.77L5.82 15.52L6.5 11.14L3.5 8.27L7.91 7.26L10 3Z" fill="white" opacity="0.9"/>
                    <text x="10" y="17" text-anchor="middle" font-size="5" font-weight="bold" fill="white" font-family="Arial, sans-serif">TRUSTED</text>
                </svg>
            `,
            [this.badgeTypes.LEGAL_WARNING]: `
                <svg width="20" height="20" viewBox="0 0 20 20" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <defs>
                        <linearGradient id="warningGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                            <stop offset="0%" style="stop-color:#FF9800;stop-opacity:1" />
                            <stop offset="50%" style="stop-color:#F57C00;stop-opacity:1" />
                            <stop offset="100%" style="stop-color:#E65100;stop-opacity:1" />
                        </linearGradient>
                    </defs>
                    <polygon points="10,2 18,16 2,16" fill="url(#warningGradient)" stroke="#BF360C" stroke-width="1.5"/>
                    <path d="M10 6L10 12" stroke="white" stroke-width="2" stroke-linecap="round"/>
                    <circle cx="10" cy="14" r="1" fill="white"/>
                </svg>
            `
        };
        
        return svgMap[type] || '';
    }

    /**
     * Create verified badge element
     */
    createVerifiedBadge(type, assetId = '') {
        const badgeContainer = document.createElement('div');
        badgeContainer.className = `verified-badge ${type}`;
        badgeContainer.setAttribute('data-asset-id', assetId);
        badgeContainer.title = this.tooltips[type] || '';
        
        badgeContainer.innerHTML = this.createBadgeSVG(type);
        
        // Add click handler
        badgeContainer.addEventListener('click', () => {
            this.onBadgeClicked(assetId);
        });
        
        // Add hover effects
        badgeContainer.addEventListener('mouseenter', () => {
            badgeContainer.style.transform = 'scale(1.1)';
            badgeContainer.style.filter = 'brightness(1.2)';
        });
        
        badgeContainer.addEventListener('mouseleave', () => {
            badgeContainer.style.transform = 'scale(1)';
            badgeContainer.style.filter = 'brightness(1)';
        });
        
        return badgeContainer;
    }

    /**
     * Create verified filter widget
     */
    createVerifiedFilterWidget() {
        const filterContainer = document.createElement('div');
        filterContainer.className = 'verified-filter-widget';
        
        const checkbox = document.createElement('input');
        checkbox.type = 'checkbox';
        checkbox.id = 'verified-only-filter';
        checkbox.checked = this.showVerifiedOnly;
        
        const label = document.createElement('label');
        label.htmlFor = 'verified-only-filter';
        label.innerHTML = '<span class="checkmark">✔️</span> Show Verified Only';
        
        checkbox.addEventListener('change', (e) => {
            this.setShowVerifiedOnly(e.target.checked);
        });
        
        filterContainer.appendChild(checkbox);
        filterContainer.appendChild(label);
        
        return filterContainer;
    }

    /**
     * Create legal warning widget
     */
    createLegalWarningWidget() {
        const warningContainer = document.createElement('div');
        warningContainer.className = 'legal-warning-widget';
        
        const warningIcon = document.createElement('div');
        warningIcon.className = 'warning-icon';
        warningIcon.innerHTML = this.createBadgeSVG(this.badgeTypes.LEGAL_WARNING);
        
        const warningText = document.createElement('div');
        warningText.className = 'warning-text';
        warningText.textContent = this.tooltips[this.badgeTypes.LEGAL_WARNING];
        
        warningContainer.appendChild(warningIcon);
        warningContainer.appendChild(warningText);
        
        return warningContainer;
    }

    /**
     * Add verified asset
     */
    addVerifiedAsset(assetId, type) {
        this.verifiedAssets.set(assetId, {
            type: type,
            tooltipText: this.tooltips[type],
            isVerified: true
        });
    }

    /**
     * Remove verified asset
     */
    removeVerifiedAsset(assetId) {
        this.verifiedAssets.delete(assetId);
    }

    /**
     * Check if asset is verified
     */
    isAssetVerified(assetId) {
        return this.verifiedAssets.has(assetId) && this.verifiedAssets.get(assetId).isVerified;
    }

    /**
     * Set show verified only filter
     */
    setShowVerifiedOnly(show) {
        this.showVerifiedOnly = show;
        this.onFilterChanged(show);
    }

    /**
     * Get show verified only filter state
     */
    getShowVerifiedOnly() {
        return this.showVerifiedOnly;
    }

    /**
     * Handle badge click
     */
    onBadgeClicked(assetId) {
        console.log(`Badge clicked for asset: ${assetId}`);
        // Emit event or call callback
        if (window.Telegram && window.Telegram.WebApp) {
            window.Telegram.WebApp.showAlert(`Verified SHAHCOIN Asset: ${assetId}`);
        }
    }

    /**
     * Handle filter change
     */
    onFilterChanged(showVerifiedOnly) {
        console.log(`Filter changed: ${showVerifiedOnly ? 'Show verified only' : 'Show all'}`);
        // Emit event or call callback
        if (window.Telegram && window.Telegram.WebApp) {
            window.Telegram.WebApp.showAlert(`Filter: ${showVerifiedOnly ? 'Verified only' : 'All assets'}`);
        }
    }

    /**
     * Initialize the verified badge system
     */
    init() {
        console.log('Shahcoin Verified Badge System initialized');
        
        // Add some sample verified assets
        this.addVerifiedAsset('SHAH-TOKEN-001', this.badgeTypes.VERIFIED_TOKEN);
        this.addVerifiedAsset('SHAH-NFT-001', this.badgeTypes.VERIFIED_NFT);
        this.addVerifiedAsset('CREATOR-001', this.badgeTypes.TRUSTED_CREATOR);
        
        return this;
    }
}

// Export for use in Telegram Mini App
if (typeof module !== 'undefined' && module.exports) {
    module.exports = VerifiedBadgeTelegramMiniApp;
} else {
    window.VerifiedBadgeTelegramMiniApp = VerifiedBadgeTelegramMiniApp;
}
