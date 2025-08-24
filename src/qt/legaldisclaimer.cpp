// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#include "legaldisclaimer.h"
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

LegalDisclaimer::LegalDisclaimer(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

LegalDisclaimer::~LegalDisclaimer()
{
}

QWidget* LegalDisclaimer::createFooterDisclaimer()
{
    QWidget* footerWidget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(footerWidget);
    
    QLabel* disclaimerLabel = new QLabel(getFooterText());
    disclaimerLabel->setWordWrap(true);
    disclaimerLabel->setStyleSheet("color: #666; font-size: 10px; padding: 2px;");
    
    layout->addWidget(disclaimerLabel);
    layout->addStretch();
    
    return footerWidget;
}

QWidget* LegalDisclaimer::createAboutDisclaimer()
{
    QWidget* aboutWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(aboutWidget);
    
    QLabel* titleLabel = new QLabel("SHAHCOIN® Legal Information");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333; margin-bottom: 10px;");
    
    QTextEdit* legalText = new QTextEdit();
    legalText->setPlainText(getAboutText());
    legalText->setReadOnly(true);
    legalText->setMaximumHeight(200);
    legalText->setStyleSheet("border: 1px solid #ddd; border-radius: 4px; padding: 8px;");
    
    QLabel* buildInfo = new QLabel(getBuildSignature());
    buildInfo->setStyleSheet("font-size: 10px; color: #666; font-style: italic; margin-top: 10px;");
    
    layout->addWidget(titleLabel);
    layout->addWidget(legalText);
    layout->addWidget(buildInfo);
    
    return aboutWidget;
}

QWidget* LegalDisclaimer::createCreationWarning()
{
    QWidget* warningWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(warningWidget);
    
    QLabel* warningIcon = new QLabel("⚠️");
    warningIcon->setStyleSheet("font-size: 16px; color: #E65100;");
    
    QLabel* warningText = new QLabel(getCreationWarningText());
    warningText->setWordWrap(true);
    warningText->setStyleSheet("color: #E65100; font-weight: bold; font-size: 12px; padding: 8px; background: #fff3cd; border: 1px solid #ffc107; border-radius: 4px;");
    
    layout->addWidget(warningIcon);
    layout->addWidget(warningText);
    
    return warningWidget;
}

QDialog* LegalDisclaimer::createFullLegalDialog()
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("SHAHCOIN® Legal Information");
    dialog->setModal(true);
    dialog->resize(600, 400);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    QTextEdit* legalText = new QTextEdit();
    legalText->setHtml(formatLegalText(getFullLegalText()));
    legalText->setReadOnly(true);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* closeButton = new QPushButton("Close");
    QPushButton* licenseButton = new QPushButton("View License");
    
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);
    connect(licenseButton, &QPushButton::clicked, this, &LegalDisclaimer::onLegalLinkClicked);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(licenseButton);
    buttonLayout->addWidget(closeButton);
    
    layout->addWidget(legalText);
    layout->addLayout(buttonLayout);
    
    return dialog;
}

QString LegalDisclaimer::getFooterText() const
{
    return "SHAHCOIN® is a registered trademark owned by Hamid Rad. Licensed under Apache 2.0.";
}

QString LegalDisclaimer::getAboutText() const
{
    return QString(
        "SHAHCOIN® Legal Information\n\n"
        "SHAHCOIN®, SHAH, SHAHI, and associated logos are registered trademarks of the Shahcoin project.\n\n"
        "This software is provided under the Apache License 2.0 for core blockchain code.\n\n"
        "Trademark Notice:\n"
        "• SHAHCOIN®, SHAH, SHAHI are protected trademarks\n"
        "• Unauthorized use of branding is strictly prohibited\n"
        "• You may not use these names in forked projects\n\n"
        "Code License:\n"
        "• Core blockchain: Apache 2.0 License\n"
        "• UI modules: Apache 2.0 with branding restrictions\n\n"
        "For licensing inquiries: Info@shah.vip\n\n"
        "Build Information:\n"
        "%1"
    ).arg(getBuildSignature());
}

QString LegalDisclaimer::getCreationWarningText() const
{
    return "⚠️ You may not use \"SHAH\", \"SHAHCOIN\" or any official branding in your project name or logo unless explicitly authorized. See LICENSE and NOTICE files for legal terms.";
}

QString LegalDisclaimer::getFullLegalText() const
{
    return QString(
        "<h2>SHAHCOIN® Legal Information</h2>"
        "<h3>Trademark Protection</h3>"
        "<p><strong>SHAHCOIN®, SHAH, SHAHI</strong>, and associated logos are registered trademarks of the Shahcoin project. "
        "Unauthorized use of these names, marks, or graphical assets is strictly prohibited.</p>"
        
        "<h3>License Information</h3>"
        "<p>This project uses a dual licensing strategy:</p>"
        "<ul>"
        "<li><strong>Core blockchain code:</strong> Apache License 2.0</li>"
        "<li><strong>UI modules and premium features:</strong> Apache 2.0 with branding restrictions</li>"
        "</ul>"
        
        "<h3>Usage Restrictions</h3>"
        "<p>You may:</p>"
        "<ul>"
        "<li>Fork and modify the code under Apache 2.0 license</li>"
        "<li>Use the code in your own projects with attribution</li>"
        "</ul>"
        
        "<p>You may NOT:</p>"
        "<ul>"
        "<li>Use SHAHCOIN®, SHAH, or SHAHI names in your projects</li>"
        "<li>Use SHAHCOIN logos or branding without permission</li>"
        "<li>Create tokens/NFTs with SHAH-branded names</li>"
        "<li>Imply affiliation with the Shahcoin project</li>"
        "</ul>"
        
        "<h3>Enforcement</h3>"
        "<p>Violations of trademark rights will result in:</p>"
        "<ul>"
        "<li>Legal action and takedown requests</li>"
        "<li>DMCA notices for unauthorized use</li>"
        "<li>Potential legal proceedings</li>"
        "</ul>"
        
        "<h3>Contact Information</h3>"
        "<p>For licensing requests and legal inquiries: <strong>Info@shah.vip</strong></p>"
        
        "<hr>"
        "<p><em>%1</em></p>"
    ).arg(getBuildSignature());
}

QString LegalDisclaimer::getBuildSignature() const
{
    return "Built by Shahcoin Team • © Hamid Rad";
}

QString LegalDisclaimer::getVersionInfo() const
{
    return "SHAHCOIN Core v1.0.0 | Signed Build by Shahcoin Team © Hamid Rad";
}

void LegalDisclaimer::onLegalLinkClicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/SHAHCoinvip/shahcoin/blob/main/LICENSE.md"));
}

void LegalDisclaimer::showFullLegalDialog()
{
    QDialog* dialog = createFullLegalDialog();
    dialog->exec();
    dialog->deleteLater();
}

void LegalDisclaimer::setupUI()
{
    // Setup completed in constructor
}

QString LegalDisclaimer::formatLegalText(const QString& text) const
{
    return text;
}
