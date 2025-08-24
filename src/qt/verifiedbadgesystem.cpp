// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#include "verifiedbadgesystem.h"
#include "svg_icons.h"
#include <QApplication>
#include <QStyle>
#include <QMessageBox>

VerifiedBadgeSystem::VerifiedBadgeSystem(QObject *parent)
    : QObject(parent)
    , m_showVerifiedOnly(false)
{
}

VerifiedBadgeSystem::~VerifiedBadgeSystem()
{
}

QLabel* VerifiedBadgeSystem::createVerifiedBadge(BadgeType type, const QString& assetId)
{
    QLabel* badge = new QLabel();
    badge->setFixedSize(24, 24);
    badge->setScaledContents(true);
    
    QString svgContent = getBadgeSVG(type);
    badge->setText(svgContent);
    
    // Set tooltip based on type
    QString tooltipText;
    switch (type) {
        case VerifiedToken:
            tooltipText = getVerifiedTokenTooltip();
            break;
        case VerifiedNFT:
            tooltipText = getVerifiedNFTTooltip();
            break;
        case TrustedCreator:
            tooltipText = "This creator has been verified and trusted by the SHAHCOIN team.";
            break;
        case LegalWarning:
            tooltipText = getLegalWarningText();
            break;
    }
    
    setupTooltip(badge, tooltipText);
    
    // Store asset info if provided
    if (!assetId.isEmpty()) {
        BadgeInfo info;
        info.tooltipText = tooltipText;
        info.legalText = getLegalWarningText();
        info.isVerified = (type == VerifiedToken || type == VerifiedNFT);
        info.type = type;
        m_verifiedAssets[assetId] = info;
    }
    
    return badge;
}

QWidget* VerifiedBadgeSystem::createVerifiedFilterWidget()
{
    QWidget* filterWidget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(filterWidget);
    
    QCheckBox* verifiedOnlyCheck = new QCheckBox("✔️ Show Verified Only");
    verifiedOnlyCheck->setChecked(m_showVerifiedOnly);
    verifiedOnlyCheck->setToolTip("Filter to show only verified SHAHCOIN assets");
    
    connect(verifiedOnlyCheck, &QCheckBox::toggled, this, &VerifiedBadgeSystem::onFilterToggled);
    
    layout->addWidget(verifiedOnlyCheck);
    layout->addStretch();
    
    return filterWidget;
}

QWidget* VerifiedBadgeSystem::createLegalWarningWidget()
{
    QWidget* warningWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(warningWidget);
    
    QLabel* warningIcon = new QLabel();
    warningIcon->setFixedSize(20, 20);
    warningIcon->setText(getBadgeSVG(LegalWarning));
    warningIcon->setScaledContents(true);
    
    QLabel* warningText = new QLabel(getLegalWarningText());
    warningText->setWordWrap(true);
    warningText->setStyleSheet("color: #E65100; font-weight: bold; padding: 8px;");
    
    QHBoxLayout* iconLayout = new QHBoxLayout();
    iconLayout->addWidget(warningIcon);
    iconLayout->addWidget(warningText);
    iconLayout->addStretch();
    
    layout->addLayout(iconLayout);
    
    return warningWidget;
}

void VerifiedBadgeSystem::addVerifiedAsset(const QString& assetId, BadgeType type)
{
    BadgeInfo info;
    info.tooltipText = (type == VerifiedToken) ? getVerifiedTokenTooltip() : getVerifiedNFTTooltip();
    info.legalText = getLegalWarningText();
    info.isVerified = true;
    info.type = type;
    
    m_verifiedAssets[assetId] = info;
}

void VerifiedBadgeSystem::removeVerifiedAsset(const QString& assetId)
{
    m_verifiedAssets.remove(assetId);
}

bool VerifiedBadgeSystem::isAssetVerified(const QString& assetId) const
{
    return m_verifiedAssets.contains(assetId) && m_verifiedAssets[assetId].isVerified;
}

VerifiedBadgeSystem::BadgeInfo VerifiedBadgeSystem::getBadgeInfo(const QString& assetId) const
{
    return m_verifiedAssets.value(assetId);
}

bool VerifiedBadgeSystem::getShowVerifiedOnly() const
{
    return m_showVerifiedOnly;
}

void VerifiedBadgeSystem::setShowVerifiedOnly(bool show)
{
    if (m_showVerifiedOnly != show) {
        m_showVerifiedOnly = show;
        emit filterChanged(show);
    }
}

QString VerifiedBadgeSystem::getVerifiedTokenTooltip() const
{
    return "🔐 This token is officially verified by the SHAHCOIN team.\n"
           "Smart contract and metadata have been reviewed for security and compliance.\n\n"
           "SHAHCOIN® is a registered trademark. All rights reserved.";
}

QString VerifiedBadgeSystem::getVerifiedNFTTooltip() const
{
    return "🖼 This NFT is part of a trusted, verified collection.\n"
           "Creator and assets meet SHAHCOIN quality standards.\n\n"
           "SHAHCOIN® is a registered trademark. All rights reserved.";
}

QString VerifiedBadgeSystem::getUnverifiedTooltip() const
{
    return "🚫 This item is user-created and not officially verified.\n"
           "DYOR (Do Your Own Research) before buying.\n\n"
           "SHAHCOIN® is a registered trademark. All rights reserved.";
}

QString VerifiedBadgeSystem::getLegalWarningText() const
{
    return "⚠️ You may not use \"SHAH\", \"SHAHCOIN\" or any official branding in your project name or logo unless explicitly authorized.\n\n"
           "See LICENSE and NOTICE files for legal terms.\n\n"
           "For licensing inquiries: Info@shah.vip";
}

void VerifiedBadgeSystem::onFilterToggled(bool checked)
{
    setShowVerifiedOnly(checked);
}

void VerifiedBadgeSystem::onBadgeClicked()
{
    QLabel* badge = qobject_cast<QLabel*>(sender());
    if (badge) {
        // Find asset ID from badge properties or emit signal
        emit badgeClicked("");
    }
}

QString VerifiedBadgeSystem::getBadgeSVG(BadgeType type) const
{
    switch (type) {
        case VerifiedToken:
            return ShahcoinIcons::VERIFIED_BADGE_OFFICIAL;
        case VerifiedNFT:
            return ShahcoinIcons::VERIFIED_BADGE_NFT;
        case TrustedCreator:
            return ShahcoinIcons::TRUSTED_CREATOR_TAG;
        case LegalWarning:
            return ShahcoinIcons::LEGAL_WARNING_ICON;
        default:
            return "";
    }
}

void VerifiedBadgeSystem::setupTooltip(QLabel* badge, const QString& tooltipText)
{
    badge->setToolTip(tooltipText);
    badge->setCursor(Qt::PointingHandCursor);
}
