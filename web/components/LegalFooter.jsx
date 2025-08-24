// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

import React from 'react';
import './LegalFooter.css';

const LegalFooter = () => {
    return (
        <footer className="legal-footer">
            <div className="legal-footer-content">
                <div className="legal-disclaimer">
                    <p className="text-xs text-gray-500 text-center">
                        Shahcoin® is a registered trademark owned by Hamid Rad.  
                        All rights reserved.  
                        This website and related code are provided under Apache 2.0 License.  
                        Unauthorized use of the Shahcoin name, logo, or branding is strictly prohibited.
                    </p>
                </div>
                
                <div className="legal-links">
                    <a href="/license" className="legal-link">License</a>
                    <span className="separator">•</span>
                    <a href="/legal" className="legal-link">Legal</a>
                    <span className="separator">•</span>
                    <a href="/trademark" className="legal-link">Trademark</a>
                </div>
                
                <div className="build-signature">
                    <p className="build-info">
                        Built by Shahcoin Team • © Hamid Rad
                    </p>
                </div>
            </div>
        </footer>
    );
};

export default LegalFooter;
