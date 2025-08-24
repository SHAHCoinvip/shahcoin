#include "premiumpanel.h"
#include "premiumsubscription.h"
#include "premiumfeatures.h"
#include "walletmodel.h"
#include "platformstyle.h"
#include "guiutil.h"
#include "util/translation.h"

#include <QApplication>
#include <QMessageBox>
#include <QSplitter>
#include <QHeaderView>
#include <QScrollBar>

// PremiumPanel Implementation

PremiumPanel::PremiumPanel(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, const PlatformStyle* platformStyle, QWidget* parent)
    : QWidget(parent)
    , m_premiumManager(premiumManager)
    , m_walletModel(walletModel)
    , m_platformStyle(platformStyle)
    , m_aiAssistant(nullptr)
    , m_aiInsights(nullptr)
    , m_feeOptimizer(nullptr)
    , m_troubleshooter(nullptr)
    , m_multilingualAssistant(nullptr)
    , m_artBooster(nullptr)
    , m_collectionDrops(nullptr)
    , m_auctionModule(nullptr)
    , m_portfolioAnalyzer(nullptr)
    , m_priceAlerts(nullptr)
    , m_usageDashboard(nullptr)
    , m_emailSettings(nullptr)
{
    setupUI();
    setupPremiumFeatures();
    
    // Connect to premium manager signals
    connect(m_premiumManager, &PremiumSubscriptionManager::subscriptionStatusChanged,
            this, &PremiumPanel::refreshPremiumStatus);
    connect(m_premiumManager, &PremiumSubscriptionManager::featureAccessChanged,
            this, &PremiumPanel::updateFeatureAccess);
    
    refreshPremiumStatus();
}

void PremiumPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Premium Status Header
    m_statusWidget = new QWidget();
    m_statusWidget->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #FF6B35, stop: 1 #F7931E); "
                                 "color: white; padding: 15px; border-radius: 8px; margin-bottom: 10px;");
    
    QHBoxLayout* statusLayout = new QHBoxLayout(m_statusWidget);
    
    m_statusIcon = new QLabel("👑");
    m_statusIcon->setStyleSheet("font-size: 24px;");
    statusLayout->addWidget(m_statusIcon);
    
    QVBoxLayout* statusTextLayout = new QVBoxLayout();
    m_statusText = new QLabel(tr("Shahcoin Premium"));
    m_statusText->setStyleSheet("font-size: 18px; font-weight: bold;");
    statusTextLayout->addWidget(m_statusText);
    
    QLabel* benefitsLabel = new QLabel(tr("Unlock AI-powered features, advanced analytics, and exclusive tools"));
    benefitsLabel->setStyleSheet("font-size: 12px; opacity: 0.9;");
    statusTextLayout->addWidget(benefitsLabel);
    
    statusLayout->addLayout(statusTextLayout);
    statusLayout->addStretch();
    
    m_upgradeButton = new QPushButton(tr("🚀 Upgrade Now"));
    m_upgradeButton->setStyleSheet("background-color: rgba(255, 255, 255, 0.2); color: white; padding: 10px 20px; "
                                  "border: 2px solid white; border-radius: 6px; font-weight: bold;");
    statusLayout->addWidget(m_upgradeButton);
    
    m_mainLayout->addWidget(m_statusWidget);
    
    // Tab Widget for Feature Categories
    m_tabWidget = new QTabWidget();
    m_tabWidget->setStyleSheet("QTabWidget::pane { border: 1px solid #ddd; border-radius: 4px; } "
                              "QTabBar::tab { padding: 10px 20px; margin-right: 2px; } "
                              "QTabBar::tab:selected { background-color: #007bff; color: white; }");
    
    // AI Features Tab
    m_aiTab = new QWidget();
    QVBoxLayout* aiLayout = new QVBoxLayout(m_aiTab);
    aiLayout->setAlignment(Qt::AlignTop);
    m_tabWidget->addTab(m_aiTab, tr("🤖 AI Features"));
    
    // Trading & NFT Tab
    m_tradingTab = new QWidget();
    QVBoxLayout* tradingLayout = new QVBoxLayout(m_tradingTab);
    tradingLayout->setAlignment(Qt::AlignTop);
    m_tabWidget->addTab(m_tradingTab, tr("💼 Trading & NFT"));
    
    // Tools & Analytics Tab
    m_toolsTab = new QWidget();
    QVBoxLayout* toolsLayout = new QVBoxLayout(m_toolsTab);
    toolsLayout->setAlignment(Qt::AlignTop);
    m_tabWidget->addTab(m_toolsTab, tr("🛠️ Tools & Analytics"));
    
    // Settings Tab
    m_analyticsTab = new QWidget();
    QVBoxLayout* analyticsLayout = new QVBoxLayout(m_analyticsTab);
    analyticsLayout->setAlignment(Qt::AlignTop);
    m_tabWidget->addTab(m_analyticsTab, tr("⚙️ Settings"));
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Connect signals
    connect(m_upgradeButton, &QPushButton::clicked, this, &PremiumPanel::onUpgradeClicked);
}

void PremiumPanel::setupPremiumFeatures()
{
    // Initialize AI Features
    m_aiAssistant = new AIWalletAssistant(m_premiumManager, m_walletModel, this);
    m_aiInsights = new AIInsightsPanel(m_premiumManager, m_walletModel, this);
    m_feeOptimizer = new AIFeeOptimizer(m_premiumManager, m_walletModel, this);
    m_troubleshooter = new AITroubleshooter(m_premiumManager, m_walletModel, this);
    m_multilingualAssistant = new MultilingualAssistant(m_premiumManager, this);
    
    // Initialize Trading & NFT Features
    m_artBooster = new AIArtBooster(m_premiumManager, this);
    m_collectionDrops = new NFTCollectionDrops(m_premiumManager, m_walletModel, this);
    m_auctionModule = new AuctionModule(m_premiumManager, m_walletModel, this);
    
    // Initialize Tools & Analytics Features
    m_portfolioAnalyzer = new AIPortfolioAnalyzer(m_premiumManager, m_walletModel, this);
    m_priceAlerts = new NFTFloorPriceAlerts(m_premiumManager, this);
    m_usageDashboard = new UsageDashboard(m_premiumManager, m_walletModel, this);
    
    // Initialize Settings Features
    m_emailSettings = new EmailConfirmationSettings(m_premiumManager, this);
    
    // Create feature cards and add to appropriate tabs
    
    // AI Features Tab
    QVBoxLayout* aiLayout = qobject_cast<QVBoxLayout*>(m_aiTab->layout());
    aiLayout->addWidget(createFeatureCard(
        tr("🧠 AI Wallet Assistant"),
        tr("Get intelligent help with transactions, security, and wallet management"),
        "🤖",
        m_aiAssistant
    ));
    
    aiLayout->addWidget(createFeatureCard(
        tr("📊 AI Insights Panel"),
        tr("Real-time security analysis and performance insights powered by AI"),
        "📊",
        m_aiInsights
    ));
    
    aiLayout->addWidget(createFeatureCard(
        tr("⚡ AI Fee Optimizer"),
        tr("Automatically optimize transaction fees for cost and speed"),
        "⚡",
        m_feeOptimizer
    ));
    
    aiLayout->addWidget(createFeatureCard(
        tr("🛠️ AI Troubleshooter"),
        tr("Diagnose and fix wallet issues with AI-powered assistance"),
        "🛠️",
        m_troubleshooter
    ));
    
    aiLayout->addWidget(createFeatureCard(
        tr("🌐 Multilingual Assistant"),
        tr("Get support in multiple languages with real-time translation"),
        "🌐",
        m_multilingualAssistant
    ));
    
    // Trading & NFT Tab
    QVBoxLayout* tradingLayout = qobject_cast<QVBoxLayout*>(m_tradingTab->layout());
    tradingLayout->addWidget(createFeatureCard(
        tr("🎨 AI Art Booster"),
        tr("Generate and enhance NFT artwork with AI tools"),
        "🎨",
        m_artBooster
    ));
    
    tradingLayout->addWidget(createFeatureCard(
        tr("📦 NFT Collection Drops"),
        tr("Create and manage large-scale NFT collection launches"),
        "📦",
        m_collectionDrops
    ));
    
    tradingLayout->addWidget(createFeatureCard(
        tr("🕰️ Auction Module"),
        tr("Participate in and create NFT auctions with advanced features"),
        "🕰️",
        m_auctionModule
    ));
    
    // Tools & Analytics Tab
    QVBoxLayout* toolsLayout = qobject_cast<QVBoxLayout*>(m_toolsTab->layout());
    toolsLayout->addWidget(createFeatureCard(
        tr("📈 AI Portfolio Analyzer"),
        tr("Deep portfolio analysis with AI-powered recommendations"),
        "📈",
        m_portfolioAnalyzer
    ));
    
    toolsLayout->addWidget(createFeatureCard(
        tr("🔔 NFT Floor Price Alerts"),
        tr("Get notified when NFT collections hit your target prices"),
        "🔔",
        m_priceAlerts
    ));
    
    toolsLayout->addWidget(createFeatureCard(
        tr("🧾 Usage Dashboard"),
        tr("Comprehensive transaction history and usage analytics"),
        "🧾",
        m_usageDashboard
    ));
    
    // Settings Tab
    QVBoxLayout* analyticsLayout = qobject_cast<QVBoxLayout*>(m_analyticsTab->layout());
    analyticsLayout->addWidget(createFeatureCard(
        tr("📬 Email Confirmation Settings"),
        tr("Configure email notifications for transactions and security"),
        "📬",
        m_emailSettings
    ));
}

QWidget* PremiumPanel::createFeatureCard(const QString& title, const QString& description, const QString& icon, QWidget* featureWidget, bool isPremium)
{
    QWidget* cardWidget = new QWidget();
    cardWidget->setStyleSheet("QWidget { background-color: white; border: 1px solid #ddd; border-radius: 8px; "
                             "margin: 5px; } QWidget:hover { border-color: #007bff; }");
    cardWidget->setMinimumHeight(120);
    
    QVBoxLayout* cardLayout = new QVBoxLayout(cardWidget);
    
    // Header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* iconLabel = new QLabel(icon);
    iconLabel->setStyleSheet("font-size: 24px;");
    headerLayout->addWidget(iconLabel);
    
    QVBoxLayout* titleLayout = new QVBoxLayout();
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");
    titleLayout->addWidget(titleLabel);
    
    QLabel* descLabel = new QLabel(description);
    descLabel->setStyleSheet("font-size: 12px; color: #666;");
    descLabel->setWordWrap(true);
    titleLayout->addWidget(descLabel);
    
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    
    if (isPremium) {
        QLabel* premiumBadge = new QLabel(tr("👑 PREMIUM"));
        premiumBadge->setStyleSheet("background-color: #FF6B35; color: white; padding: 2px 8px; "
                                   "border-radius: 10px; font-size: 10px; font-weight: bold;");
        headerLayout->addWidget(premiumBadge);
    }
    
    cardLayout->addLayout(headerLayout);
    
    // Feature widget with gate
    if (isPremium) {
        FeatureGateWidget* gateWidget = new FeatureGateWidget(static_cast<PremiumFeature>(0), m_premiumManager, cardWidget);
        gateWidget->setGatedWidget(featureWidget);
        cardLayout->addWidget(gateWidget);
        m_featureGates[featureWidget] = gateWidget;
    } else {
        cardLayout->addWidget(featureWidget);
    }
    
    return cardWidget;
}

void PremiumPanel::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    
    if (m_premiumManager) {
        m_premiumManager->setWalletModel(walletModel);
    }
    
    // Update all features with new wallet model
    if (m_aiAssistant) m_aiAssistant->setParent(this); // Placeholder - need proper setWalletModel methods
    if (m_aiInsights) m_aiInsights->setParent(this);
    if (m_feeOptimizer) m_feeOptimizer->setParent(this);
    if (m_troubleshooter) m_troubleshooter->setParent(this);
    if (m_collectionDrops) m_collectionDrops->setParent(this);
    if (m_auctionModule) m_auctionModule->setParent(this);
    if (m_portfolioAnalyzer) m_portfolioAnalyzer->setParent(this);
    if (m_usageDashboard) m_usageDashboard->setParent(this);
}

void PremiumPanel::refreshPremiumStatus()
{
    bool isPremium = m_premiumManager->isPremiumActive();
    
    if (isPremium) {
        m_statusIcon->setText("👑");
        QString planName = m_premiumManager->getPlanDisplayName(m_premiumManager->getCurrentPlan());
        int daysRemaining = m_premiumManager->getDaysRemaining();
        
        m_statusText->setText(tr("Shahcoin Premium - %1").arg(planName));
        m_upgradeButton->setText(tr("⚙️ Manage Subscription"));
        
        // Update status widget style for active premium
        m_statusWidget->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #4CAF50, stop: 1 #8BC34A); "
                                     "color: white; padding: 15px; border-radius: 8px; margin-bottom: 10px;");
    } else {
        m_statusIcon->setText("🆓");
        m_statusText->setText(tr("Shahcoin Premium - Free Plan"));
        m_upgradeButton->setText(tr("🚀 Upgrade Now"));
        
        // Reset to upgrade style
        m_statusWidget->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #FF6B35, stop: 1 #F7931E); "
                                     "color: white; padding: 15px; border-radius: 8px; margin-bottom: 10px;");
    }
    
    updateFeatureAccess();
}

void PremiumPanel::updateFeatureAccess()
{
    // Update all feature gates
    for (auto it = m_featureGates.begin(); it != m_featureGates.end(); ++it) {
        it.value()->updateGateStatus();
    }
}

void PremiumPanel::onUpgradeClicked()
{
    bool isPremium = m_premiumManager->isPremiumActive();
    
    if (isPremium) {
        // Show subscription management
        SubscriptionManagementDialog dialog(m_premiumManager, this);
        dialog.exec();
    } else {
        // Show upgrade dialog
        PremiumPurchaseDialog dialog(m_premiumManager, this);
        dialog.exec();
    }
}

void PremiumPanel::onFeatureClicked()
{
    // Handle feature-specific actions
}

// PremiumStatusBar Implementation

PremiumStatusBar::PremiumStatusBar(PremiumSubscriptionManager* premiumManager, QWidget* parent)
    : QWidget(parent)
    , m_premiumManager(premiumManager)
    , m_isClickable(true)
{
    setupUI();
    updateStatus();
    
    // Connect to premium manager signals
    connect(m_premiumManager, &PremiumSubscriptionManager::subscriptionStatusChanged,
            this, &PremiumStatusBar::updateStatus);
}

void PremiumStatusBar::setupUI()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(5);
    
    m_statusIcon = new QLabel();
    m_statusIcon->setFixedSize(16, 16);
    m_layout->addWidget(m_statusIcon);
    
    m_statusText = new QLabel();
    m_statusText->setStyleSheet("font-size: 12px; font-weight: bold;");
    m_layout->addWidget(m_statusText);
    
    m_expiryText = new QLabel();
    m_expiryText->setStyleSheet("font-size: 11px; color: #666;");
    m_layout->addWidget(m_expiryText);
    
    m_layout->addStretch();
    
    // Make clickable
    setStyleSheet("QWidget:hover { background-color: #f0f0f0; border-radius: 3px; }");
    setCursor(Qt::PointingHandCursor);
}

void PremiumStatusBar::updateStatus()
{
    bool isPremium = m_premiumManager->isPremiumActive();
    
    if (isPremium) {
        m_statusIcon->setText("👑");
        QString planName = m_premiumManager->getPlanDisplayName(m_premiumManager->getCurrentPlan());
        int daysRemaining = m_premiumManager->getDaysRemaining();
        
        m_statusText->setText(tr("Premium"));
        m_statusText->setStyleSheet("font-size: 12px; font-weight: bold; color: #4CAF50;");
        
        if (daysRemaining <= 7 && daysRemaining > 0) {
            m_expiryText->setText(tr("Expires in %1 days").arg(daysRemaining));
            m_expiryText->setStyleSheet("font-size: 11px; color: #FF9800;");
        } else if (daysRemaining > 7) {
            m_expiryText->setText(tr("Active"));
            m_expiryText->setStyleSheet("font-size: 11px; color: #4CAF50;");
        } else {
            m_expiryText->setText(tr("Expired"));
            m_expiryText->setStyleSheet("font-size: 11px; color: #F44336;");
        }
    } else {
        m_statusIcon->setText("🆓");
        m_statusText->setText(tr("Free"));
        m_statusText->setStyleSheet("font-size: 12px; font-weight: bold; color: #666;");
        m_expiryText->setText(tr("Upgrade available"));
        m_expiryText->setStyleSheet("font-size: 11px; color: #FF6B35;");
    }
}

void PremiumStatusBar::onStatusClicked()
{
    if (!m_isClickable) return;
    
    bool isPremium = m_premiumManager->isPremiumActive();
    
    if (isPremium) {
        // Show subscription management
        SubscriptionManagementDialog dialog(m_premiumManager, this);
        dialog.exec();
    } else {
        // Show upgrade dialog
        PremiumPurchaseDialog dialog(m_premiumManager, this);
        dialog.exec();
    }
}

void PremiumStatusBar::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    onStatusClicked();
}

// PremiumWelcomeDialog Implementation

PremiumWelcomeDialog::PremiumWelcomeDialog(PremiumSubscriptionManager* premiumManager, QWidget* parent)
    : QDialog(parent)
    , m_premiumManager(premiumManager)
{
    setWindowTitle(tr("Welcome to Shahcoin Premium"));
    setModal(true);
    resize(500, 600);
    
    setupUI();
}

void PremiumWelcomeDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Header
    QLabel* headerLabel = new QLabel(tr("👑 Unlock the Full Power of Shahcoin"));
    headerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #FF6B35; text-align: center; margin: 20px;");
    headerLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(headerLabel);
    
    QLabel* subtitleLabel = new QLabel(tr("Get access to AI-powered features, advanced analytics, and exclusive tools"));
    subtitleLabel->setStyleSheet("font-size: 14px; color: #666; text-align: center; margin-bottom: 30px;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);
    m_mainLayout->addWidget(subtitleLabel);
    
    // Features overview
    QScrollArea* featuresArea = new QScrollArea();
    QWidget* featuresWidget = new QWidget();
    QVBoxLayout* featuresLayout = new QVBoxLayout(featuresWidget);
    
    QStringList features = {
        tr("🧠 AI Wallet Assistant - Get intelligent help 24/7"),
        tr("📊 AI Insights Panel - Real-time security analysis"),
        tr("⚡ AI Fee Optimizer - Save money on transactions"),
        tr("🛠️ AI Troubleshooter - Fix issues automatically"),
        tr("🌐 Multilingual Support - Available in 8+ languages"),
        tr("🎨 AI Art Booster - Create stunning NFT artwork"),
        tr("📦 NFT Collection Drops - Launch large collections"),
        tr("🕰️ Auction Module - Advanced NFT trading"),
        tr("📈 AI Portfolio Analyzer - Optimize your holdings"),
        tr("🔔 Price Alerts - Never miss an opportunity"),
        tr("🧾 Usage Dashboard - Comprehensive analytics"),
        tr("📬 Email Confirmations - Stay informed")
    };
    
    for (const QString& feature : features) {
        QLabel* featureLabel = new QLabel(feature);
        featureLabel->setStyleSheet("font-size: 13px; padding: 8px; background-color: #f8f9fa; "
                                   "border-left: 3px solid #007bff; margin: 2px;");
        featuresLayout->addWidget(featureLabel);
    }
    
    featuresArea->setWidget(featuresWidget);
    featuresArea->setMaximumHeight(300);
    m_mainLayout->addWidget(featuresArea);
    
    // Pricing info
    QGroupBox* pricingGroup = new QGroupBox(tr("Choose Your Plan"));
    QHBoxLayout* pricingLayout = new QHBoxLayout(pricingGroup);
    
    // Monthly plan
    QWidget* monthlyWidget = new QWidget();
    monthlyWidget->setStyleSheet("background-color: white; border: 2px solid #ddd; border-radius: 8px; padding: 15px;");
    QVBoxLayout* monthlyLayout = new QVBoxLayout(monthlyWidget);
    
    QLabel* monthlyTitle = new QLabel(tr("Monthly"));
    monthlyTitle->setStyleSheet("font-size: 16px; font-weight: bold;");
    monthlyLayout->addWidget(monthlyTitle);
    
    QLabel* monthlyPrice = new QLabel(tr("$4.99/month"));
    monthlyPrice->setStyleSheet("font-size: 24px; font-weight: bold; color: #007bff;");
    monthlyLayout->addWidget(monthlyPrice);
    
    QLabel* monthlyDesc = new QLabel(tr("Perfect for trying premium features"));
    monthlyDesc->setStyleSheet("font-size: 12px; color: #666;");
    monthlyLayout->addWidget(monthlyDesc);
    
    pricingLayout->addWidget(monthlyWidget);
    
    // Yearly plan (recommended)
    QWidget* yearlyWidget = new QWidget();
    yearlyWidget->setStyleSheet("background-color: #FF6B35; color: white; border: 2px solid #FF6B35; border-radius: 8px; padding: 15px;");
    QVBoxLayout* yearlyLayout = new QVBoxLayout(yearlyWidget);
    
    QLabel* yearlyBadge = new QLabel(tr("🏆 RECOMMENDED"));
    yearlyBadge->setStyleSheet("font-size: 10px; font-weight: bold; background-color: rgba(255,255,255,0.2); "
                              "padding: 2px 8px; border-radius: 10px;");
    yearlyBadge->setAlignment(Qt::AlignCenter);
    yearlyLayout->addWidget(yearlyBadge);
    
    QLabel* yearlyTitle = new QLabel(tr("Yearly"));
    yearlyTitle->setStyleSheet("font-size: 16px; font-weight: bold;");
    yearlyLayout->addWidget(yearlyTitle);
    
    QLabel* yearlyPrice = new QLabel(tr("$49.99/year"));
    yearlyPrice->setStyleSheet("font-size: 24px; font-weight: bold;");
    yearlyLayout->addWidget(yearlyPrice);
    
    QLabel* yearlyDesc = new QLabel(tr("Save 17% vs monthly + bonus features"));
    yearlyDesc->setStyleSheet("font-size: 12px; opacity: 0.9;");
    yearlyLayout->addWidget(yearlyDesc);
    
    pricingLayout->addWidget(yearlyWidget);
    
    m_mainLayout->addWidget(pricingGroup);
    
    // Action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* tourButton = new QPushButton(tr("📖 Take Tour"));
    tourButton->setStyleSheet("background-color: #6c757d; color: white; padding: 10px 20px; border-radius: 5px;");
    
    QPushButton* upgradeButton = new QPushButton(tr("🚀 Upgrade Now"));
    upgradeButton->setStyleSheet("background-color: #28a745; color: white; padding: 10px 20px; border-radius: 5px; font-weight: bold;");
    
    QPushButton* closeButton = new QPushButton(tr("Maybe Later"));
    closeButton->setStyleSheet("color: #6c757d; padding: 10px 20px;");
    
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(tourButton);
    buttonLayout->addWidget(upgradeButton);
    
    m_mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(tourButton, &QPushButton::clicked, this, &PremiumWelcomeDialog::onTakeTourClicked);
    connect(upgradeButton, &QPushButton::clicked, this, &PremiumWelcomeDialog::onUpgradeClicked);
    connect(closeButton, &QPushButton::clicked, this, &PremiumWelcomeDialog::onCloseClicked);
}

void PremiumWelcomeDialog::onTakeTourClicked()
{
    // TODO: Implement premium features tour
    QMessageBox::information(this, tr("Premium Tour"), 
                           tr("🎯 Welcome to the Premium Features Tour!\n\n"
                              "This interactive tour will guide you through all the amazing premium features.\n\n"
                              "Coming soon in the next update!"));
}

void PremiumWelcomeDialog::onUpgradeClicked()
{
    accept(); // Close welcome dialog
    
    // Show upgrade dialog
    PremiumPurchaseDialog dialog(m_premiumManager, parentWidget());
    dialog.exec();
}

void PremiumWelcomeDialog::onCloseClicked()
{
    reject();
}

#include "premiumpanel.moc"
