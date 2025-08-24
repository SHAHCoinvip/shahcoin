// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

/**
 * Shahcoin Legal Disclaimer System for Telegram Mini App
 * Handles legal notices, trademark protection, and build signatures
 */

class LegalDisclaimerTelegramMiniApp {
    constructor() {
        this.buildSignature = "Built by Shahcoin Team • © Hamid Rad";
        this.versionInfo = "Shahcoin Core v1.0.0 | Signed Build by Shahcoin Team © Hamid Rad";
    }

    /**
     * Create footer disclaimer element
     */
    createFooterDisclaimer() {
        const footerContainer = document.createElement('div');
        footerContainer.className = 'legal-footer-disclaimer';
        
        const disclaimerText = document.createElement('p');
        disclaimerText.className = 'disclaimer-text';
        disclaimerText.textContent = this.getFooterText();
        
        const buildInfo = document.createElement('p');
        buildInfo.className = 'build-info';
        buildInfo.textContent = this.buildSignature;
        
        footerContainer.appendChild(disclaimerText);
        footerContainer.appendChild(buildInfo);
        
        return footerContainer;
    }

    /**
     * Create about dialog disclaimer
     */
    createAboutDisclaimer() {
        const aboutContainer = document.createElement('div');
        aboutContainer.className = 'legal-about-disclaimer';
        
        const title = document.createElement('h3');
        title.textContent = 'SHAHCOIN® Legal Information';
        title.className = 'legal-title';
        
        const content = document.createElement('div');
        content.className = 'legal-content';
        content.innerHTML = this.getAboutText();
        
        const buildInfo = document.createElement('p');
        buildInfo.className = 'build-signature';
        buildInfo.textContent = this.buildSignature;
        
        aboutContainer.appendChild(title);
        aboutContainer.appendChild(content);
        aboutContainer.appendChild(buildInfo);
        
        return aboutContainer;
    }

    /**
     * Create creation warning element
     */
    createCreationWarning() {
        const warningContainer = document.createElement('div');
        warningContainer.className = 'legal-creation-warning';
        
        const warningIcon = document.createElement('span');
        warningIcon.textContent = '⚠️';
        warningIcon.className = 'warning-icon';
        
        const warningText = document.createElement('p');
        warningText.className = 'warning-text';
        warningText.textContent = this.getCreationWarningText();
        
        warningContainer.appendChild(warningIcon);
        warningContainer.appendChild(warningText);
        
        return warningContainer;
    }

    /**
     * Create full legal dialog
     */
    createFullLegalDialog() {
        const dialogContainer = document.createElement('div');
        dialogContainer.className = 'legal-full-dialog';
        
        const header = document.createElement('div');
        header.className = 'dialog-header';
        header.innerHTML = '<h2>SHAHCOIN® Legal Information</h2>';
        
        const content = document.createElement('div');
        content.className = 'dialog-content';
        content.innerHTML = this.getFullLegalText();
        
        const footer = document.createElement('div');
        footer.className = 'dialog-footer';
        
        const closeButton = document.createElement('button');
        closeButton.textContent = 'Close';
        closeButton.className = 'dialog-button close-button';
        closeButton.onclick = () => this.closeDialog(dialogContainer);
        
        const licenseButton = document.createElement('button');
        licenseButton.textContent = 'View License';
        licenseButton.className = 'dialog-button license-button';
        licenseButton.onclick = () => this.openLicense();
        
        footer.appendChild(licenseButton);
        footer.appendChild(closeButton);
        
        dialogContainer.appendChild(header);
        dialogContainer.appendChild(content);
        dialogContainer.appendChild(footer);
        
        return dialogContainer;
    }

    /**
     * Get footer text
     */
    getFooterText() {
        return 'SHAHCOIN® is a registered trademark owned by Hamid Rad. Licensed under Apache 2.0.';
    }

    /**
     * Get about text
     */
    getAboutText() {
        return `
            <p><strong>SHAHCOIN®, SHAH, SHAHI</strong>, and associated logos are registered trademarks of the Shahcoin project.</p>
            
            <h4>Trademark Notice:</h4>
            <ul>
                <li>SHAHCOIN®, SHAH, SHAHI are protected trademarks</li>
                <li>Unauthorized use of branding is strictly prohibited</li>
                <li>You may not use these names in forked projects</li>
            </ul>
            
            <h4>Code License:</h4>
            <ul>
                <li>Core blockchain: Apache 2.0 License</li>
                <li>UI modules: Apache 2.0 with branding restrictions</li>
            </ul>
            
            <p>For licensing inquiries: <strong>Info@shah.vip</strong></p>
        `;
    }

    /**
     * Get creation warning text
     */
    getCreationWarningText() {
        return '⚠️ You may not use "SHAH", "SHAHCOIN" or any official branding in your project name or logo unless explicitly authorized. See LICENSE and NOTICE files for legal terms.';
    }

    /**
     * Get full legal text
     */
    getFullLegalText() {
        return `
            <h3>Trademark Protection</h3>
            <p><strong>SHAHCOIN®, SHAH, SHAHI</strong>, and associated logos are registered trademarks of the Shahcoin project. 
            Unauthorized use of these names, marks, or graphical assets is strictly prohibited.</p>
            
            <h3>License Information</h3>
            <p>This project uses a dual licensing strategy:</p>
            <ul>
                <li><strong>Core blockchain code:</strong> Apache License 2.0</li>
                <li><strong>UI modules and premium features:</strong> Apache 2.0 with branding restrictions</li>
            </ul>
            
            <h3>Usage Restrictions</h3>
            <p>You may:</p>
            <ul>
                <li>Fork and modify the code under Apache 2.0 license</li>
                <li>Use the code in your own projects with attribution</li>
            </ul>
            
            <p>You may NOT:</p>
            <ul>
                <li>Use SHAHCOIN®, SHAH, or SHAHI names in your projects</li>
                <li>Use SHAHCOIN logos or branding without permission</li>
                <li>Create tokens/NFTs with SHAH-branded names</li>
                <li>Imply affiliation with the Shahcoin project</li>
            </ul>
            
            <h3>Enforcement</h3>
            <p>Violations of trademark rights will result in:</p>
            <ul>
                <li>Legal action and takedown requests</li>
                <li>DMCA notices for unauthorized use</li>
                <li>Potential legal proceedings</li>
            </ul>
            
            <h3>Contact Information</h3>
            <p>For licensing requests and legal inquiries: <strong>Info@shah.vip</strong></p>
            
            <hr>
            <p><em>${this.buildSignature}</em></p>
        `;
    }

    /**
     * Get build signature
     */
    getBuildSignature() {
        return this.buildSignature;
    }

    /**
     * Get version info
     */
    getVersionInfo() {
        return this.versionInfo;
    }

    /**
     * Close dialog
     */
    closeDialog(dialogContainer) {
        if (dialogContainer && dialogContainer.parentNode) {
            dialogContainer.parentNode.removeChild(dialogContainer);
        }
    }

    /**
     * Open license
     */
    openLicense() {
        if (window.Telegram && window.Telegram.WebApp) {
            window.Telegram.WebApp.openLink('https://github.com/shahcoin/shahcoin/blob/main/LICENSE.md');
        } else {
            window.open('https://github.com/shahcoin/shahcoin/blob/main/LICENSE.md', '_blank');
        }
    }

    /**
     * Show full legal dialog
     */
    showFullLegalDialog() {
        const dialog = this.createFullLegalDialog();
        document.body.appendChild(dialog);
        
        // Add overlay
        const overlay = document.createElement('div');
        overlay.className = 'dialog-overlay';
        overlay.onclick = () => this.closeDialog(dialog);
        document.body.appendChild(overlay);
    }

    /**
     * Initialize the legal disclaimer system
     */
    init() {
        console.log('Shahcoin Legal Disclaimer System initialized');
        
        // Add CSS styles
        this.addStyles();
        
        return this;
    }

    /**
     * Add CSS styles for legal components
     */
    addStyles() {
        const style = document.createElement('style');
        style.textContent = `
            .legal-footer-disclaimer {
                background: #f8f9fa;
                border-top: 1px solid #dee2e6;
                padding: 15px;
                text-align: center;
                font-size: 12px;
                color: #6c757d;
            }
            
            .disclaimer-text {
                margin: 0 0 10px 0;
                line-height: 1.4;
            }
            
            .build-info {
                margin: 0;
                font-size: 11px;
                font-style: italic;
                color: #6c757d;
            }
            
            .legal-creation-warning {
                background: #fff3cd;
                border: 1px solid #ffc107;
                border-radius: 4px;
                padding: 12px;
                margin: 10px 0;
                display: flex;
                align-items: flex-start;
                gap: 8px;
            }
            
            .warning-icon {
                font-size: 16px;
                color: #E65100;
                flex-shrink: 0;
            }
            
            .warning-text {
                margin: 0;
                color: #E65100;
                font-weight: bold;
                font-size: 12px;
                line-height: 1.4;
            }
            
            .legal-full-dialog {
                position: fixed;
                top: 50%;
                left: 50%;
                transform: translate(-50%, -50%);
                background: white;
                border-radius: 8px;
                box-shadow: 0 4px 20px rgba(0,0,0,0.3);
                max-width: 90vw;
                max-height: 80vh;
                overflow: hidden;
                z-index: 1001;
            }
            
            .dialog-overlay {
                position: fixed;
                top: 0;
                left: 0;
                right: 0;
                bottom: 0;
                background: rgba(0,0,0,0.5);
                z-index: 1000;
            }
            
            .dialog-header {
                padding: 20px 20px 0 20px;
                border-bottom: 1px solid #dee2e6;
            }
            
            .dialog-header h2 {
                margin: 0;
                color: #333;
                font-size: 18px;
            }
            
            .dialog-content {
                padding: 20px;
                max-height: 400px;
                overflow-y: auto;
                font-size: 14px;
                line-height: 1.6;
            }
            
            .dialog-footer {
                padding: 15px 20px;
                border-top: 1px solid #dee2e6;
                display: flex;
                justify-content: flex-end;
                gap: 10px;
            }
            
            .dialog-button {
                padding: 8px 16px;
                border: none;
                border-radius: 4px;
                cursor: pointer;
                font-size: 14px;
                transition: background-color 0.3s ease;
            }
            
            .close-button {
                background: #6c757d;
                color: white;
            }
            
            .close-button:hover {
                background: #5a6268;
            }
            
            .license-button {
                background: #007bff;
                color: white;
            }
            
            .license-button:hover {
                background: #0056b3;
            }
        `;
        document.head.appendChild(style);
    }
}

// Export for use in Telegram Mini App
if (typeof module !== 'undefined' && module.exports) {
    module.exports = LegalDisclaimerTelegramMiniApp;
} else {
    window.LegalDisclaimerTelegramMiniApp = LegalDisclaimerTelegramMiniApp;
}
