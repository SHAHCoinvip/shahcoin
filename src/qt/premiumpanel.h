#ifndef SHAHCOIN_QT_PREMIUMPANEL_H
#define SHAHCOIN_QT_PREMIUMPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QGridLayout>

class PremiumSubscriptionManager;
class WalletModel;
class PlatformStyle;
class AIWalletAssistant;
class AIInsightsPanel;
class AIFeeOptimizer;
class AITroubleshooter;
class MultilingualAssistant;
class AIArtBooster;
class NFTCollectionDrops;
class AuctionModule;
class AIPortfolioAnalyzer;
class NFTFloorPriceAlerts;
class UsageDashboard;
class EmailConfirmationSettings;
class FeatureGateWidget;

/**
 * Premium Features Panel - Main container for all premium functionality
 */
class PremiumPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PremiumPanel(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, const PlatformStyle* platformStyle, QWidget* parent = nullptr);

    void setWalletModel(WalletModel* walletModel);

public Q_SLOTS:
    void refreshPremiumStatus();

private Q_SLOTS:
    void onUpgradeClicked();
    void onFeatureClicked();

private:
    void setupUI();
    void setupPremiumFeatures();
    void updateFeatureAccess();
    QWidget* createFeatureCard(const QString& title, const QString& description, const QString& icon, QWidget* featureWidget, bool isPremium = true);

    PremiumSubscriptionManager* m_premiumManager;
    WalletModel* m_walletModel;
    const PlatformStyle* m_platformStyle;
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Premium Status Section
    QWidget* m_statusWidget;
    QLabel* m_statusIcon;
    QLabel* m_statusText;
    QPushButton* m_upgradeButton;
    
    // Feature Categories
    QWidget* m_aiTab;
    QWidget* m_tradingTab;
    QWidget* m_toolsTab;
    QWidget* m_analyticsTab;
    
    // Premium Features
    AIWalletAssistant* m_aiAssistant;
    AIInsightsPanel* m_aiInsights;
    AIFeeOptimizer* m_feeOptimizer;
    AITroubleshooter* m_troubleshooter;
    MultilingualAssistant* m_multilingualAssistant;
    AIArtBooster* m_artBooster;
    NFTCollectionDrops* m_collectionDrops;
    AuctionModule* m_auctionModule;
    AIPortfolioAnalyzer* m_portfolioAnalyzer;
    NFTFloorPriceAlerts* m_priceAlerts;
    UsageDashboard* m_usageDashboard;
    EmailConfirmationSettings* m_emailSettings;
    
    // Feature Gates
    QMap<QWidget*, FeatureGateWidget*> m_featureGates;
};

/**
 * Premium Status Bar Widget - Shows premium status in main wallet
 */
class PremiumStatusBar : public QWidget
{
    Q_OBJECT

public:
    explicit PremiumStatusBar(PremiumSubscriptionManager* premiumManager, QWidget* parent = nullptr);

public Q_SLOTS:
    void updateStatus();

private Q_SLOTS:
    void onStatusClicked();

private:
    void setupUI();

    PremiumSubscriptionManager* m_premiumManager;
    QHBoxLayout* m_layout;
    QLabel* m_statusIcon;
    QLabel* m_statusText;
    QLabel* m_expiryText;
    bool m_isClickable;
};

/**
 * Premium Welcome Dialog - Shown to new users to introduce premium features
 */
class PremiumWelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PremiumWelcomeDialog(PremiumSubscriptionManager* premiumManager, QWidget* parent = nullptr);

private Q_SLOTS:
    void onTakeTourClicked();
    void onUpgradeClicked();
    void onCloseClicked();

private:
    void setupUI();

    PremiumSubscriptionManager* m_premiumManager;
    QVBoxLayout* m_mainLayout;
};

#endif // SHAHCOIN_QT_PREMIUMPANEL_H
