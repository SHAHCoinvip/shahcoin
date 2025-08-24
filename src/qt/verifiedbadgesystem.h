// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#ifndef VERIFIEDBADGESYSTEM_H
#define VERIFIEDBADGESYSTEM_H

#include <QWidget>
#include <QLabel>
#include <QToolTip>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QString>
#include <QMap>

class VerifiedBadgeSystem : public QObject
{
    Q_OBJECT

public:
    enum BadgeType {
        VerifiedToken,
        VerifiedNFT,
        TrustedCreator,
        LegalWarning
    };

    struct BadgeInfo {
        QString tooltipText;
        QString legalText;
        bool isVerified;
        BadgeType type;
    };

    explicit VerifiedBadgeSystem(QObject *parent = nullptr);
    ~VerifiedBadgeSystem();

    // Badge creation methods
    QLabel* createVerifiedBadge(BadgeType type, const QString& assetId = "");
    QWidget* createVerifiedFilterWidget();
    QWidget* createLegalWarningWidget();

    // Badge management
    void addVerifiedAsset(const QString& assetId, BadgeType type);
    void removeVerifiedAsset(const QString& assetId);
    bool isAssetVerified(const QString& assetId) const;
    BadgeInfo getBadgeInfo(const QString& assetId) const;

    // Filter functionality
    bool getShowVerifiedOnly() const;
    void setShowVerifiedOnly(bool show);

    // Legal text templates
    QString getVerifiedTokenTooltip() const;
    QString getVerifiedNFTTooltip() const;
    QString getUnverifiedTooltip() const;
    QString getLegalWarningText() const;

signals:
    void filterChanged(bool showVerifiedOnly);
    void badgeClicked(const QString& assetId);

private slots:
    void onFilterToggled(bool checked);
    void onBadgeClicked();

private:
    QMap<QString, BadgeInfo> m_verifiedAssets;
    bool m_showVerifiedOnly;
    
    QString getBadgeSVG(BadgeType type) const;
    void setupTooltip(QLabel* badge, const QString& tooltipText);
};

#endif // VERIFIEDBADGESYSTEM_H
