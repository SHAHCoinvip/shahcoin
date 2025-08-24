// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

class SmartUnitDisplayTelegramMiniApp {
    constructor() {
        this.smartUnitEnabled = true;
        this.currentDisplayMode = 'smart';
        this.usdPrice = 0.0;
        
        this.units = {
            shahi: { name: 'Shahi', icon: '●', shahiValue: 1, color: '#6c757d' },
            nano: { name: 'Nano Shah', icon: '⚡', shahiValue: 10, color: '#007bff' },
            micro: { name: 'Micro Shah', icon: '✨', shahiValue: 100, color: '#6f42c1' },
            mesin: { name: 'Mesin Shah', icon: '🔶', shahiValue: 1000, color: '#fd7e14' },
            simin: { name: 'Simin Shah', icon: '⚪', shahiValue: 10000, color: '#20c997' },
            zarrin: { name: 'Zarrin Shah', icon: '🟡', shahiValue: 100000, color: '#ffc107' },
            shahcent: { name: 'Shahcent', icon: '💎', shahiValue: 1000000, color: '#dc3545' },
            daric: { name: 'Daric Shah', icon: '🏛️', shahiValue: 10000000, color: '#28a745' },
            shah: { name: 'SHAH', icon: '🪙', shahiValue: 100000000, color: '#17a2b8' },
            meta: { name: 'Meta Shah', icon: '🔷', shahiValue: 1000000000, color: '#6f42c1' },
            beta: { name: 'Beta Shah', icon: '🟠', shahiValue: 10000000000, color: '#fd7e14' },
            sigma: { name: 'Sigma Shah', icon: '⚙️', shahiValue: 100000000000, color: '#20c997' },
            mega: { name: 'Mega Shah', icon: '🌟', shahiValue: 1000000000000, color: '#ffc107' },
            giga: { name: 'Giga Shah', icon: '🔵', shahiValue: 10000000000000, color: '#dc3545' },
            alpha: { name: 'Alpha Shah', icon: '💠', shahiValue: 100000000000000, color: '#28a745' }
        };

        this.initialize();
    }

    formatNumber(value) {
        return value.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ',');
    }

    getSubUnitBreakdown(shahiAmount) {
        if (shahiAmount === 0) return '0 Shahi';
        const parts = [];
        let remaining = shahiAmount;
        const subUnits = ['daric', 'shahcent', 'zarrin', 'simin', 'mesin', 'micro', 'nano', 'shahi'];
        
        for (const unitKey of subUnits) {
            const unit = this.units[unitKey];
            if (remaining >= unit.shahiValue) {
                const count = Math.floor(remaining / unit.shahiValue);
                remaining = remaining % unit.shahiValue;
                parts.push(`${count} ${unit.name}`);
            }
        }
        return parts.join(' + ');
    }

    getMegaUnitBreakdown(shahiAmount) {
        if (shahiAmount < 100000000) return this.getSubUnitBreakdown(shahiAmount);
        const shahAmount = Math.floor(shahiAmount / 100000000);
        const remainingShahi = shahiAmount % 100000000;
        const parts = [];
        const megaUnits = ['alpha', 'giga', 'mega', 'sigma', 'beta', 'meta'];
        
        for (const unitKey of megaUnits) {
            const unit = this.units[unitKey];
            const unitValue = Math.floor(unit.shahiValue / 100000000);
            if (shahAmount >= unitValue) {
                const count = Math.floor(shahAmount / unitValue);
                shahAmount = shahAmount % unitValue;
                parts.push(`${this.formatNumber(count)} ${unit.name}`);
            }
        }
        if (shahAmount > 0) parts.push(`${this.formatNumber(shahAmount)} SHAH`);
        if (remainingShahi > 0) parts.push(this.getSubUnitBreakdown(remainingShahi));
        return parts.join(' + ');
    }

    getSmartUnitBreakdown(shahiAmount) {
        return shahiAmount < 100000000 ? this.getSubUnitBreakdown(shahiAmount) : this.getMegaUnitBreakdown(shahiAmount);
    }

    getFullSHAHValue(shahiAmount) {
        const shahValue = shahiAmount / 100000000.0;
        return `${shahValue.toFixed(8)} SHAH`;
    }

    getUSDEquivalent(shahiAmount) {
        if (this.usdPrice <= 0.0) return `${this.getSmartUnitBreakdown(shahiAmount)} (USD price not available)`;
        const shahValue = shahiAmount / 100000000.0;
        const usdValue = shahValue * this.usdPrice;
        return `$${usdValue.toFixed(2)} USD`;
    }

    formatBalance(shahiAmount) {
        switch (this.currentDisplayMode) {
            case 'full': return this.getFullSHAHValue(shahiAmount);
            case 'smart': return this.getSmartUnitBreakdown(shahiAmount);
            case 'usd': return this.getUSDEquivalent(shahiAmount);
            default: return this.getSmartUnitBreakdown(shahiAmount);
        }
    }

    createBalanceElement(shahiAmount) {
        const displayText = this.smartUnitEnabled ? this.formatBalance(shahiAmount) : this.getFullSHAHValue(shahiAmount);
        const element = document.createElement('span');
        element.className = 'smart-unit-display';
        element.textContent = displayText;
        element.title = `Smart: ${this.getSmartUnitBreakdown(shahiAmount)}\nFull: ${this.getFullSHAHValue(shahiAmount)}`;
        
        element.addEventListener('click', () => {
            this.cycleDisplayMode();
            element.textContent = this.smartUnitEnabled ? this.formatBalance(shahiAmount) : this.getFullSHAHValue(shahiAmount);
        });
        
        return element;
    }

    cycleDisplayMode() {
        const modes = ['smart', 'full', 'usd'];
        const currentIndex = modes.indexOf(this.currentDisplayMode);
        this.currentDisplayMode = modes[(currentIndex + 1) % modes.length];
        this.saveSettings();
    }

    toggleSmartUnit() {
        this.smartUnitEnabled = !this.smartUnitEnabled;
        this.saveSettings();
    }

    saveSettings() {
        try {
            const settings = { smartUnitEnabled: this.smartUnitEnabled, displayMode: this.currentDisplayMode };
            localStorage.setItem('shahcoin_smart_unit_settings', JSON.stringify(settings));
        } catch (error) {
            console.warn('Failed to save Smart Unit settings:', error);
        }
    }

    loadSettings() {
        try {
            const settings = JSON.parse(localStorage.getItem('shahcoin_smart_unit_settings') || '{}');
            this.smartUnitEnabled = settings.smartUnitEnabled !== false;
            this.currentDisplayMode = settings.displayMode || 'smart';
        } catch (error) {
            console.warn('Failed to load Smart Unit settings:', error);
        }
    }

    showUnitExplorer() {
        const message = '🔎 SHAH Unit Explorer\n\n' +
            '📊 Sub-Units (Below 1 SHAH):\n' +
            '● Shahi = 1 Shahi\n' +
            '⚡ Nano Shah = 10 Shahi\n' +
            '✨ Micro Shah = 100 Shahi\n' +
            '🔶 Mesin Shah = 1,000 Shahi\n' +
            '⚪ Simin Shah = 10,000 Shahi\n' +
            '🟡 Zarrin Shah = 100,000 Shahi\n' +
            '💎 Shahcent = 1,000,000 Shahi\n' +
            '🏛️ Daric Shah = 10,000,000 Shahi\n\n' +
            '📈 Mega-Units (Above 1 SHAH):\n' +
            '🪙 SHAH = 100,000,000 Shahi\n' +
            '🔷 Meta Shah = 10 SHAH\n' +
            '🟠 Beta Shah = 100 SHAH\n' +
            '⚙️ Sigma Shah = 1,000 SHAH\n' +
            '🌟 Mega Shah = 10,000 SHAH\n' +
            '🔵 Giga Shah = 100,000 SHAH\n' +
            '💠 Alpha Shah = 1,000,000 SHAH';

        if (window.Telegram && window.Telegram.WebApp) {
            window.Telegram.WebApp.showAlert(message);
        } else {
            alert(message);
        }
    }

    createSettingsWidget() {
        const container = document.createElement('div');
        container.className = 'smart-unit-settings';
        
        const toggleLabel = document.createElement('h4');
        toggleLabel.textContent = '🧠 Smart Unit Display';
        container.appendChild(toggleLabel);

        const toggleSwitch = document.createElement('label');
        toggleSwitch.className = 'toggle-switch';
        
        const toggleInput = document.createElement('input');
        toggleInput.type = 'checkbox';
        toggleInput.checked = this.smartUnitEnabled;
        toggleInput.addEventListener('change', (e) => {
            this.smartUnitEnabled = e.target.checked;
            this.saveSettings();
        });

        const slider = document.createElement('span');
        slider.className = 'slider';
        
        const label = document.createElement('span');
        label.className = 'label';
        label.textContent = 'Enable Smart Unit Mode';

        toggleSwitch.appendChild(toggleInput);
        toggleSwitch.appendChild(slider);
        toggleSwitch.appendChild(label);
        container.appendChild(toggleSwitch);

        const explorerButton = document.createElement('button');
        explorerButton.className = 'unit-explorer-btn';
        explorerButton.textContent = '🔎 SHAH Unit Explorer';
        explorerButton.addEventListener('click', () => this.showUnitExplorer());
        container.appendChild(explorerButton);

        return container;
    }

    initialize() {
        this.loadSettings();
        this.addStyles();
        console.log('Smart Unit Display System initialized for Telegram Mini App');
    }

    addStyles() {
        if (document.getElementById('smart-unit-display-styles')) return;

        const style = document.createElement('style');
        style.id = 'smart-unit-display-styles';
        style.textContent = `
            .smart-unit-display {
                font-weight: bold;
                color: #333;
                cursor: pointer;
                padding: 2px 4px;
                border-radius: 4px;
                background: linear-gradient(135deg, #f8f9fa 0%, #e9ecef 100%);
                border: 1px solid #dee2e6;
                transition: all 0.3s ease;
            }
            .smart-unit-display:hover {
                background: linear-gradient(135deg, #e9ecef 0%, #dee2e6 100%);
                transform: translateY(-1px);
                box-shadow: 0 2px 8px rgba(0,0,0,0.1);
            }
            .smart-unit-settings {
                background: white;
                border-radius: 8px;
                padding: 20px;
                box-shadow: 0 2px 10px rgba(0,0,0,0.1);
                border: 1px solid #dee2e6;
                margin: 10px 0;
            }
            .toggle-switch {
                display: flex;
                align-items: center;
                cursor: pointer;
                margin-bottom: 15px;
            }
            .toggle-switch input { display: none; }
            .toggle-switch .slider {
                width: 50px;
                height: 24px;
                background: #ccc;
                border-radius: 12px;
                position: relative;
                margin-right: 10px;
                transition: background 0.3s ease;
            }
            .toggle-switch .slider:before {
                content: '';
                position: absolute;
                width: 18px;
                height: 18px;
                border-radius: 50%;
                background: white;
                top: 3px;
                left: 3px;
                transition: transform 0.3s ease;
                box-shadow: 0 2px 4px rgba(0,0,0,0.2);
            }
            .toggle-switch input:checked + .slider { background: #007bff; }
            .toggle-switch input:checked + .slider:before { transform: translateX(26px); }
            .toggle-switch .label { font-size: 14px; color: #555; }
            .unit-explorer-btn {
                width: 100%;
                padding: 10px 16px;
                background: linear-gradient(135deg, #007bff 0%, #0056b3 100%);
                color: white;
                border: none;
                border-radius: 6px;
                font-size: 14px;
                font-weight: 500;
                cursor: pointer;
                transition: all 0.3s ease;
                box-shadow: 0 2px 4px rgba(0,123,255,0.3);
            }
            .unit-explorer-btn:hover {
                background: linear-gradient(135deg, #0056b3 0%, #004085 100%);
                transform: translateY(-1px);
                box-shadow: 0 4px 8px rgba(0,123,255,0.4);
            }
        `;
        document.head.appendChild(style);
    }
}

// Auto-initialize
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        window.shahcoinSmartUnitDisplay = new SmartUnitDisplayTelegramMiniApp();
    });
} else {
    window.shahcoinSmartUnitDisplay = new SmartUnitDisplayTelegramMiniApp();
}
