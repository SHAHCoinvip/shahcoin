// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

import React, { useState, useEffect } from 'react';
import './VerifiedBadgeSystem.css';

const VerifiedBadgeSystem = ({ assetId, badgeType, showFilter = false, onFilterChange }) => {
    const [showVerifiedOnly, setShowVerifiedOnly] = useState(false);

    const badgeTypes = {
        VERIFIED_TOKEN: 'verified-token',
        VERIFIED_NFT: 'verified-nft',
        TRUSTED_CREATOR: 'trusted-creator',
        LEGAL_WARNING: 'legal-warning'
    };

    const tooltips = {
        [badgeTypes.VERIFIED_TOKEN]: "🔐 This token is officially verified by the SHAHCOIN team. Smart contract and metadata have been reviewed for security and compliance.\n\nSHAHCOIN® is a registered trademark. All rights reserved.",
        [badgeTypes.VERIFIED_NFT]: "🖼 This NFT is part of a trusted, verified collection. Creator and assets meet SHAHCOIN quality standards.\n\nSHAHCOIN® is a registered trademark. All rights reserved.",
        [badgeTypes.TRUSTED_CREATOR]: "This creator has been verified and trusted by the SHAHCOIN team.",
        [badgeTypes.LEGAL_WARNING]: "⚠️ You may not use \"SHAH\", \"SHAHCOIN\" or any official branding in your project name or logo unless explicitly authorized.\n\nSee LICENSE and NOTICE files for legal terms.\n\nFor licensing inquiries: Info@shah.vip"
    };

    const getBadgeSVG = (type) => {
        switch (type) {
            case badgeTypes.VERIFIED_TOKEN:
                return (
                    <svg width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                        <defs>
                            <linearGradient id="goldGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                                <stop offset="0%" style={{stopColor: '#FFD700', stopOpacity: 1}} />
                                <stop offset="50%" style={{stopColor: '#FFA500', stopOpacity: 1}} />
                                <stop offset="100%" style={{stopColor: '#FF8C00', stopOpacity: 1}} />
                            </linearGradient>
                            <filter id="glow">
                                <feGaussianBlur stdDeviation="2" result="coloredBlur"/>
                                <feMerge> 
                                    <feMergeNode in="coloredBlur"/>
                                    <feMergeNode in="SourceGraphic"/>
                                </feMerge>
                            </filter>
                        </defs>
                        <circle cx="12" cy="12" r="11" fill="url(#goldGradient)" stroke="#B8860B" strokeWidth="2" filter="url(#glow)"/>
                        <path d="M9 15L7 13L8.4 11.6L9 12.2L15.6 5.6L17 7L9 15Z" fill="white" stroke="white" strokeWidth="0.5"/>
                        <text x="12" y="21" textAnchor="middle" fontSize="7" fontWeight="bold" fill="white" fontFamily="Arial, sans-serif">SHAH</text>
                    </svg>
                );
            case badgeTypes.VERIFIED_NFT:
                return (
                    <svg width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                        <defs>
                            <linearGradient id="purpleGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                                <stop offset="0%" style={{stopColor: '#9C27B0', stopOpacity: 1}} />
                                <stop offset="50%" style={{stopColor: '#673AB7', stopOpacity: 1}} />
                                <stop offset="100%" style={{stopColor: '#3F51B5', stopOpacity: 1}} />
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
                                 fill="url(#purpleGradient)" stroke="#4A148C" strokeWidth="1.5" filter="url(#nftGlow)"/>
                        <path d="M9 15L7 13L8.4 11.6L9 12.2L15.6 5.6L17 7L9 15Z" fill="white" stroke="white" strokeWidth="0.5"/>
                        <text x="12" y="22" textAnchor="middle" fontSize="6" fontWeight="bold" fill="white" fontFamily="Arial, sans-serif">VERIFIED</text>
                    </svg>
                );
            case badgeTypes.TRUSTED_CREATOR:
                return (
                    <svg width="20" height="20" viewBox="0 0 20 20" fill="none" xmlns="http://www.w3.org/2000/svg">
                        <defs>
                            <linearGradient id="tealGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                                <stop offset="0%" style={{stopColor: '#00BCD4', stopOpacity: 1}} />
                                <stop offset="50%" style={{stopColor: '#009688', stopOpacity: 1}} />
                                <stop offset="100%" style={{stopColor: '#4CAF50', stopOpacity: 1}} />
                            </linearGradient>
                        </defs>
                        <circle cx="10" cy="10" r="9" fill="url(#tealGradient)" stroke="#00695C" strokeWidth="1.5"/>
                        <path d="M10 3L12.09 7.26L16.5 8.27L13.5 11.14L14.18 15.52L10 13.77L5.82 15.52L6.5 11.14L3.5 8.27L7.91 7.26L10 3Z" fill="white" opacity="0.9"/>
                        <text x="10" y="17" textAnchor="middle" fontSize="5" fontWeight="bold" fill="white" fontFamily="Arial, sans-serif">TRUSTED</text>
                    </svg>
                );
            case badgeTypes.LEGAL_WARNING:
                return (
                    <svg width="20" height="20" viewBox="0 0 20 20" fill="none" xmlns="http://www.w3.org/2000/svg">
                        <defs>
                            <linearGradient id="warningGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                                <stop offset="0%" style={{stopColor: '#FF9800', stopOpacity: 1}} />
                                <stop offset="50%" style={{stopColor: '#F57C00', stopOpacity: 1}} />
                                <stop offset="100%" style={{stopColor: '#E65100', stopOpacity: 1}} />
                            </linearGradient>
                        </defs>
                        <polygon points="10,2 18,16 2,16" fill="url(#warningGradient)" stroke="#BF360C" strokeWidth="1.5"/>
                        <path d="M10 6L10 12" stroke="white" strokeWidth="2" strokeLinecap="round"/>
                        <circle cx="10" cy="14" r="1" fill="white"/>
                    </svg>
                );
            default:
                return null;
        }
    };

    const handleFilterChange = (checked) => {
        setShowVerifiedOnly(checked);
        if (onFilterChange) {
            onFilterChange(checked);
        }
    };

    if (showFilter) {
        return (
            <div className="verified-filter-widget">
                <label className="verified-filter-checkbox">
                    <input
                        type="checkbox"
                        checked={showVerifiedOnly}
                        onChange={(e) => handleFilterChange(e.target.checked)}
                    />
                    <span className="checkmark">✔️</span>
                    Show Verified Only
                </label>
            </div>
        );
    }

    if (!badgeType) {
        return null;
    }

    return (
        <div 
            className={`verified-badge ${badgeType}`}
            title={tooltips[badgeType]}
            data-asset-id={assetId}
        >
            {getBadgeSVG(badgeType)}
        </div>
    );
};

export default VerifiedBadgeSystem;
