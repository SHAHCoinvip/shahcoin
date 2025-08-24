// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

import React, { useState, useEffect, useCallback } from 'react';
import './SmartUnitDisplay.css';

const SmartUnitDisplay = ({ shahiAmount, displayMode = 'smart', showTooltip = true, className = '' }) => {
    const [smartUnitEnabled, setSmartUnitEnabled] = useState(true);
    const [currentDisplayMode, setCurrentDisplayMode] = useState(displayMode);
    const [showUnitExplorer, setShowUnitExplorer] = useState(false);

    // Unit definitions
    const units = {
        // Sub-Units (below 1 SHAH)
        shahi: { name: 'Shahi', displayName: 'Shahi', icon: '●', shahiValue: 1, description: 'Base unit (like satoshi)', color: '#6c757d' },
        nano: { name: 'Nano Shah', displayName: 'Nano Shah', icon: '⚡', shahiValue: 10, description: '10 Shahi', color: '#007bff' },
        micro: { name: 'Micro Shah', displayName: 'Micro Shah', icon: '✨', shahiValue: 100, description: '100 Shahi', color: '#6f42c1' },
        mesin: { name: 'Mesin Shah', displayName: 'Mesin Shah', icon: '🔶', shahiValue: 1000, description: '1,000 Shahi', color: '#fd7e14' },
        simin: { name: 'Simin Shah', displayName: 'Simin Shah', icon: '⚪', shahiValue: 10000, description: '10,000 Shahi', color: '#20c997' },
        zarrin: { name: 'Zarrin Shah', displayName: 'Zarrin Shah', icon: '🟡', shahiValue: 100000, description: '100,000 Shahi', color: '#ffc107' },
        shahcent: { name: 'Shahcent', displayName: 'Shahcent', icon: '💎', shahiValue: 1000000, description: '1,000,000 Shahi', color: '#dc3545' },
        daric: { name: 'Daric Shah', displayName: 'Daric Shah', icon: '🏛️', shahiValue: 10000000, description: '10,000,000 Shahi', color: '#28a745' },
        shah: { name: 'SHAH', displayName: 'SHAH', icon: '🪙', shahiValue: 100000000, description: '100,000,000 Shahi', color: '#17a2b8' },

        // Mega-Units (above 1 SHAH)
        meta: { name: 'Meta Shah', displayName: 'Meta Shah', icon: '🔷', shahiValue: 1000000000, description: '10 SHAH', color: '#6f42c1' },
        beta: { name: 'Beta Shah', displayName: 'Beta Shah', icon: '🟠', shahiValue: 10000000000, description: '100 SHAH', color: '#fd7e14' },
        sigma: { name: 'Sigma Shah', displayName: 'Sigma Shah', icon: '⚙️', shahiValue: 100000000000, description: '1,000 SHAH', color: '#20c997' },
        mega: { name: 'Mega Shah', displayName: 'Mega Shah', icon: '🌟', shahiValue: 1000000000000, description: '10,000 SHAH', color: '#ffc107' },
        giga: { name: 'Giga Shah', displayName: 'Giga Shah', icon: '🔵', shahiValue: 10000000000000, description: '100,000 SHAH', color: '#dc3545' },
        alpha: { name: 'Alpha Shah', displayName: 'Alpha Shah', icon: '💠', shahiValue: 100000000000000, description: '1,000,000 SHAH', color: '#28a745' }
    };

    // Utility functions
    const formatNumber = useCallback((value) => {
        return value.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ',');
    }, []);

    const getSubUnitBreakdown = useCallback((shahiAmount) => {
        if (shahiAmount === 0) return '0 Shahi';

        const parts = [];
        let remaining = shahiAmount;

        // Start from largest sub-unit and work down
        const subUnits = ['daric', 'shahcent', 'zarrin', 'simin', 'mesin', 'micro', 'nano', 'shahi'];
        
        for (const unitKey of subUnits) {
            const unit = units[unitKey];
            if (remaining >= unit.shahiValue) {
                const count = Math.floor(remaining / unit.shahiValue);
                remaining = remaining % unit.shahiValue;
                parts.push(`${count} ${unit.displayName}`);
            }
        }

        return parts.join(' + ');
    }, [units]);

    const getMegaUnitBreakdown = useCallback((shahiAmount) => {
        if (shahiAmount < 100000000) return getSubUnitBreakdown(shahiAmount);

        const shahAmount = Math.floor(shahiAmount / 100000000);
        const remainingShahi = shahiAmount % 100000000;

        const parts = [];

        // Handle mega-units
        const megaUnits = ['alpha', 'giga', 'mega', 'sigma', 'beta', 'meta'];
        
        for (const unitKey of megaUnits) {
            const unit = units[unitKey];
            const unitValue = Math.floor(unit.shahiValue / 100000000); // Convert to SHAH
            if (shahAmount >= unitValue) {
                const count = Math.floor(shahAmount / unitValue);
                shahAmount = shahAmount % unitValue;
                parts.push(`${formatNumber(count)} ${unit.displayName}`);
            }
        }

        // Add remaining SHAH if any
        if (shahAmount > 0) {
            parts.push(`${formatNumber(shahAmount)} SHAH`);
        }

        // Add remaining Shahi if any
        if (remainingShahi > 0) {
            parts.push(getSubUnitBreakdown(remainingShahi));
        }

        return parts.join(' + ');
    }, [units, getSubUnitBreakdown, formatNumber]);

    const getSmartUnitBreakdown = useCallback((shahiAmount) => {
        if (shahiAmount < 100000000) {
            return getSubUnitBreakdown(shahiAmount);
        } else {
            return getMegaUnitBreakdown(shahiAmount);
        }
    }, [getSubUnitBreakdown, getMegaUnitBreakdown]);

    const getFullSHAHValue = useCallback((shahiAmount) => {
        const shahValue = shahiAmount / 100000000.0;
        return `${shahValue.toFixed(8)} SHAH`;
    }, []);

    const getUSDEquivalent = useCallback((shahiAmount, usdPrice = 0.0) => {
        if (usdPrice <= 0.0) {
            return `${getSmartUnitBreakdown(shahiAmount)} (USD price not available)`;
        }
        
        const shahValue = shahiAmount / 100000000.0;
        const usdValue = shahValue * usdPrice;
        
        return `$${usdValue.toFixed(2)} USD`;
    }, [getSmartUnitBreakdown]);

    const formatBalance = useCallback((shahiAmount, mode = currentDisplayMode) => {
        switch (mode) {
            case 'full':
                return getFullSHAHValue(shahiAmount);
            case 'smart':
                return getSmartUnitBreakdown(shahiAmount);
            case 'usd':
                return getUSDEquivalent(shahiAmount);
            default:
                return getSmartUnitBreakdown(shahiAmount);
        }
    }, [currentDisplayMode, getFullSHAHValue, getSmartUnitBreakdown, getUSDEquivalent]);

    const createTooltip = useCallback((shahiAmount) => {
        const tooltipParts = [
            `Smart: ${getSmartUnitBreakdown(shahiAmount)}`,
            `Full: ${getFullSHAHValue(shahiAmount)}`
        ];

        // Add USD equivalent if available
        // const usdPrice = getUSDPrice(); // This would come from your price API
        // if (usdPrice > 0.0) {
        //     tooltipParts.push(`USD: ${getUSDEquivalent(shahiAmount, usdPrice)}`);
        // }

        return tooltipParts.join('\n');
    }, [getSmartUnitBreakdown, getFullSHAHValue]);

    // Settings component
    const SettingsWidget = () => (
        <div className="smart-unit-settings">
            <div className="setting-group">
                <h4>🧠 Smart Unit Display</h4>
                <label className="toggle-switch">
                    <input
                        type="checkbox"
                        checked={smartUnitEnabled}
                        onChange={(e) => setSmartUnitEnabled(e.target.checked)}
                    />
                    <span className="slider"></span>
                    <span className="label">Enable Smart Unit Mode</span>
                </label>
            </div>

            <div className="setting-group">
                <h4>Display Mode</h4>
                <select
                    value={currentDisplayMode}
                    onChange={(e) => setCurrentDisplayMode(e.target.value)}
                    className="display-mode-select"
                >
                    <option value="smart">Smart Unit Mode</option>
                    <option value="full">Full SHAH Value</option>
                    <option value="usd">USD Equivalent</option>
                </select>
            </div>

            <button
                className="unit-explorer-btn"
                onClick={() => setShowUnitExplorer(true)}
            >
                🔎 SHAH Unit Explorer
            </button>
        </div>
    );

    // Unit Explorer Dialog
    const UnitExplorerDialog = () => (
        <div className={`unit-explorer-overlay ${showUnitExplorer ? 'show' : ''}`}>
            <div className="unit-explorer-dialog">
                <div className="dialog-header">
                    <h2>🔎 SHAH Unit Explorer</h2>
                    <button
                        className="close-btn"
                        onClick={() => setShowUnitExplorer(false)}
                    >
                        ×
                    </button>
                </div>

                <div className="dialog-content">
                    <div className="unit-section">
                        <h3>Sub-Units (Below 1 SHAH)</h3>
                        <div className="unit-grid">
                            {['shahi', 'nano', 'micro', 'mesin', 'simin', 'zarrin', 'shahcent', 'daric'].map(unitKey => {
                                const unit = units[unitKey];
                                return (
                                    <div key={unitKey} className="unit-item">
                                        <span className="unit-icon" style={{ color: unit.color }}>
                                            {unit.icon}
                                        </span>
                                        <span className="unit-name" style={{ color: unit.color }}>
                                            {unit.displayName}
                                        </span>
                                        <span className="unit-value">
                                            = {formatNumber(unit.shahiValue)} Shahi
                                        </span>
                                        <span className="unit-description">
                                            {unit.description}
                                        </span>
                                    </div>
                                );
                            })}
                        </div>
                    </div>

                    <div className="unit-section">
                        <h3>Mega-Units (Above 1 SHAH)</h3>
                        <div className="unit-grid">
                            {['shah', 'meta', 'beta', 'sigma', 'mega', 'giga', 'alpha'].map(unitKey => {
                                const unit = units[unitKey];
                                const valueText = unitKey === 'shah' 
                                    ? '= 100,000,000 Shahi'
                                    : `= ${formatNumber(Math.floor(unit.shahiValue / 100000000))} SHAH`;
                                
                                return (
                                    <div key={unitKey} className="unit-item">
                                        <span className="unit-icon" style={{ color: unit.color }}>
                                            {unit.icon}
                                        </span>
                                        <span className="unit-name" style={{ color: unit.color }}>
                                            {unit.displayName}
                                        </span>
                                        <span className="unit-value">
                                            {valueText}
                                        </span>
                                        <span className="unit-description">
                                            {unit.description}
                                        </span>
                                    </div>
                                );
                            })}
                        </div>
                    </div>

                    <div className="unit-section">
                        <h3>Examples</h3>
                        <div className="examples-list">
                            {[
                                '110 Shahi → 1 Micro Shah + 1 Nano Shah',
                                '1,540,000 Shahi → 1 Shahcent + 5 Simin Shah + 4 Mesin Shah',
                                '1,000 SHAH → 1 Sigma Shah',
                                '10,000 SHAH → 1 Mega Shah',
                                '1,000,000 SHAH → 1 Alpha Shah'
                            ].map((example, index) => (
                                <div key={index} className="example-item">
                                    {example}
                                </div>
                            ))}
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );

    // Main display component
    const displayText = smartUnitEnabled 
        ? formatBalance(shahiAmount, currentDisplayMode)
        : getFullSHAHValue(shahiAmount);

    return (
        <>
            <span
                className={`smart-unit-display ${className}`}
                title={showTooltip ? createTooltip(shahiAmount) : ''}
            >
                {displayText}
            </span>

            <UnitExplorerDialog />
        </>
    );
};

// Export settings widget as a separate component
export const SmartUnitSettings = () => {
    return <SmartUnitDisplay.SettingsWidget />;
};

export default SmartUnitDisplay;
