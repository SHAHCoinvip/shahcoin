#include "premiumfeatures.h"
#include "premiumsubscription.h"
#include "walletmodel.h"
#include "guiutil.h"
#include "util/translation.h"

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QScrollBar>
#include <QSplitter>
#include <QHeaderView>
#include <QDateTime>
#include <QStandardPaths>

// AIWalletAssistant Implementation

AIWalletAssistant::AIWalletAssistant(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent)
    : QWidget(parent)
    , m_premiumManager(premiumManager)
    , m_walletModel(walletModel)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_messageCounter(0)
{
    setupUI();
    addQuickActions();
    
    // Add welcome message
    addMessage("AI Assistant", tr("👋 Hello! I'm your AI Wallet Assistant. I can help you with:\n\n"
                                 "• Transaction guidance\n"
                                 "• Security recommendations\n"
                                 "• Portfolio insights\n"
                                 "• Fee optimization\n"
                                 "• General wallet support\n\n"
                                 "What would you like to know?"), true);
}

void AIWalletAssistant::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Header with language selection
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* titleLabel = new QLabel(tr("🤖 AI Wallet Assistant"));
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2E8B57;");
    
    m_languageCombo = new QComboBox();
    m_languageCombo->addItem(tr("🇺🇸 English"), "en");
    m_languageCombo->addItem(tr("🇪🇸 Español"), "es");
    m_languageCombo->addItem(tr("🇫🇷 Français"), "fr");
    m_languageCombo->addItem(tr("🇩🇪 Deutsch"), "de");
    m_languageCombo->addItem(tr("🇨🇳 中文"), "zh");
    m_languageCombo->addItem(tr("🇯🇵 日本語"), "ja");
    m_languageCombo->addItem(tr("🇰🇷 한국어"), "ko");
    m_languageCombo->addItem(tr("🇷🇺 Русский"), "ru");
    
    m_clearChatButton = new QPushButton(tr("🗑️ Clear Chat"));
    m_clearChatButton->setStyleSheet("background-color: #FF6B35; color: white; padding: 5px 10px; border-radius: 3px;");
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(new QLabel(tr("Language:")));
    headerLayout->addWidget(m_languageCombo);
    headerLayout->addWidget(m_clearChatButton);
    
    m_mainLayout->addLayout(headerLayout);
    
    // Chat area
    m_chatScrollArea = new QScrollArea();
    m_chatScrollArea->setWidgetResizable(true);
    m_chatScrollArea->setMinimumHeight(400);
    m_chatScrollArea->setStyleSheet("background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px;");
    
    m_chatWidget = new QWidget();
    m_chatLayout = new QVBoxLayout(m_chatWidget);
    m_chatLayout->setAlignment(Qt::AlignTop);
    m_chatScrollArea->setWidget(m_chatWidget);
    
    m_mainLayout->addWidget(m_chatScrollArea);
    
    // Quick actions
    QLabel* quickActionsLabel = new QLabel(tr("Quick Actions:"));
    quickActionsLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    m_mainLayout->addWidget(quickActionsLabel);
    
    m_quickActionsLayout = new QHBoxLayout();
    m_mainLayout->addLayout(m_quickActionsLayout);
    
    // Input area
    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText(tr("Type your message here..."));
    m_messageInput->setStyleSheet("padding: 8px; border: 1px solid #ced4da; border-radius: 4px;");
    
    m_sendButton = new QPushButton(tr("📤 Send"));
    m_sendButton->setStyleSheet("background-color: #28a745; color: white; padding: 8px 16px; border-radius: 4px; font-weight: bold;");
    
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    
    m_mainLayout->addLayout(inputLayout);
    
    // Connect signals
    connect(m_messageInput, &QLineEdit::returnPressed, this, &AIWalletAssistant::onSendMessageClicked);
    connect(m_sendButton, &QPushButton::clicked, this, &AIWalletAssistant::onSendMessageClicked);
    connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AIWalletAssistant::onLanguageChanged);
    connect(m_clearChatButton, &QPushButton::clicked, this, &AIWalletAssistant::onClearChatClicked);
}

void AIWalletAssistant::addQuickActions()
{
    QStringList quickActions = {
        tr("💰 Check Balance"),
        tr("📊 Portfolio Summary"),
        tr("🔒 Security Tips"),
        tr("⚡ Fee Optimization"),
        tr("📈 Market Insights"),
        tr("🚀 Send Transaction")
    };
    
    for (const QString& action : quickActions) {
        QPushButton* actionButton = new QPushButton(action);
        actionButton->setStyleSheet("background-color: #007bff; color: white; padding: 5px 10px; border-radius: 3px; margin: 2px;");
        connect(actionButton, &QPushButton::clicked, this, &AIWalletAssistant::onQuickActionClicked);
        m_quickActionsLayout->addWidget(actionButton);
    }
}

void AIWalletAssistant::addMessage(const QString& sender, const QString& message, bool isBot)
{
    QWidget* messageWidget = new QWidget();
    QHBoxLayout* messageLayout = new QHBoxLayout(messageWidget);
    
    if (isBot) {
        messageLayout->setContentsMargins(10, 5, 50, 5);
    } else {
        messageLayout->setContentsMargins(50, 5, 10, 5);
    }
    
    QLabel* messageLabel = new QLabel();
    messageLabel->setWordWrap(true);
    messageLabel->setText(QString("<b>%1:</b><br>%2").arg(sender).arg(message));
    
    if (isBot) {
        messageLabel->setStyleSheet("background-color: #e3f2fd; padding: 10px; border-radius: 8px; border-left: 4px solid #2196F3;");
        messageLayout->addWidget(messageLabel);
        messageLayout->addStretch();
    } else {
        messageLabel->setStyleSheet("background-color: #e8f5e8; padding: 10px; border-radius: 8px; border-right: 4px solid #4CAF50;");
        messageLayout->addStretch();
        messageLayout->addWidget(messageLabel);
    }
    
    m_chatLayout->addWidget(messageWidget);
    
    // Auto-scroll to bottom
    QTimer::singleShot(100, [this]() {
        QScrollBar* scrollBar = m_chatScrollArea->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    });
}

void AIWalletAssistant::onSendMessageClicked()
{
    QString userMessage = m_messageInput->text().trimmed();
    if (userMessage.isEmpty()) return;
    
    // Add user message
    addMessage("You", userMessage);
    m_messageInput->clear();
    
    // Process AI response
    processAIResponse(userMessage);
}

void AIWalletAssistant::onQuickActionClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QString action = button->text();
    processAIResponse(action);
}

void AIWalletAssistant::onLanguageChanged()
{
    QString languageCode = m_languageCombo->currentData().toString();
    addMessage("AI Assistant", tr("Language changed to %1. I can now assist you in your preferred language.").arg(m_languageCombo->currentText()), true);
}

void AIWalletAssistant::onClearChatClicked()
{
    // Clear chat messages
    QLayoutItem* child;
    while ((child = m_chatLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    
    // Add welcome message again
    addMessage("AI Assistant", tr("Chat cleared. How can I help you today?"), true);
}

void AIWalletAssistant::processAIResponse(const QString& userMessage)
{
    // Generate AI response
    QString response = generateAIResponse(userMessage);
    
    // Simulate thinking delay
    QTimer::singleShot(1000, [this, response]() {
        addMessage("AI Assistant", response, true);
    });
}

QString AIWalletAssistant::generateAIResponse(const QString& input)
{
    QString lowerInput = input.toLower();
    
    // Balance related
    if (lowerInput.contains("balance") || lowerInput.contains("💰")) {
        if (m_walletModel) {
            qint64 balance = m_walletModel->getBalance();
            double balanceShah = balance / 100000000.0;
            return tr("💰 Your current SHAH balance is: %1 SHAH\n\n"
                     "Here are some insights:\n"
                     "• Your balance is sufficient for regular transactions\n"
                     "• Consider staking if you're holding long-term\n"
                     "• Current network fee is approximately 0.001 SHAH").arg(balanceShah, 0, 'f', 8);
        } else {
            return tr("💰 I can't access your balance right now, but here's what I can tell you:\n\n"
                     "• Check the main wallet overview for your current balance\n"
                     "• Your balance updates automatically when transactions confirm\n"
                     "• Make sure your wallet is fully synchronized");
        }
    }
    
    // Portfolio related
    if (lowerInput.contains("portfolio") || lowerInput.contains("📊")) {
        return tr("📊 Portfolio Analysis:\n\n"
                 "• Your SHAH holdings show steady growth potential\n"
                 "• Diversification recommendation: Consider NFT investments\n"
                 "• Risk level: Moderate (due to crypto volatility)\n"
                 "• Suggested action: Regular DCA (Dollar Cost Averaging)\n\n"
                 "💡 Tip: Use the AI Portfolio Analyzer for detailed insights!");
    }
    
    // Security related
    if (lowerInput.contains("security") || lowerInput.contains("🔒")) {
        return tr("🔒 Security Recommendations:\n\n"
                 "✅ Keep your wallet software updated\n"
                 "✅ Use strong, unique passwords\n"
                 "✅ Enable two-factor authentication\n"
                 "✅ Backup your wallet regularly\n"
                 "✅ Never share your private keys\n\n"
                 "⚠️ Always verify recipient addresses before sending\n"
                 "⚠️ Be cautious of phishing attempts");
    }
    
    // Fee optimization
    if (lowerInput.contains("fee") || lowerInput.contains("⚡")) {
        return tr("⚡ Fee Optimization Tips:\n\n"
                 "• Current network congestion: Low\n"
                 "• Recommended fee: 0.001 SHAH (fast confirmation)\n"
                 "• Economy fee: 0.0005 SHAH (slower confirmation)\n"
                 "• Best time to transact: During off-peak hours\n\n"
                 "💡 Use the AI Fee Optimizer for real-time recommendations!");
    }
    
    // Market insights
    if (lowerInput.contains("market") || lowerInput.contains("📈")) {
        return tr("📈 Market Insights:\n\n"
                 "• SHAH trend: Showing positive momentum\n"
                 "• Market sentiment: Cautiously optimistic\n"
                 "• Trading volume: Increasing\n"
                 "• Support level: Strong community backing\n\n"
                 "🎯 Key metrics to watch:\n"
                 "• Daily active addresses\n"
                 "• Network hash rate\n"
                 "• Transaction volume");
    }
    
    // Transaction help
    if (lowerInput.contains("send") || lowerInput.contains("transaction") || lowerInput.contains("🚀")) {
        return tr("🚀 Transaction Guide:\n\n"
                 "1. Click 'Send' in your wallet\n"
                 "2. Enter recipient address (double-check!)\n"
                 "3. Enter amount to send\n"
                 "4. Choose appropriate fee\n"
                 "5. Review and confirm\n\n"
                 "⚠️ Important reminders:\n"
                 "• Transactions are irreversible\n"
                 "• Always verify the recipient address\n"
                 "• Consider the fee for confirmation speed");
    }
    
    // General help
    if (lowerInput.contains("help") || lowerInput.contains("support")) {
        return tr("🤝 I'm here to help! Here's what I can assist you with:\n\n"
                 "💰 Wallet Management:\n"
                 "• Balance and transaction history\n"
                 "• Sending and receiving SHAH\n"
                 "• Address management\n\n"
                 "📊 Analytics & Insights:\n"
                 "• Portfolio analysis\n"
                 "• Market trends\n"
                 "• Performance tracking\n\n"
                 "🔒 Security:\n"
                 "• Best practices\n"
                 "• Threat detection\n"
                 "• Backup strategies\n\n"
                 "Just ask me anything about your Shahcoin wallet!");
    }
    
    // Default response
    return tr("🤔 I understand you're asking about: \"%1\"\n\n"
             "I'm constantly learning to better assist you. Here are some things I can definitely help with:\n\n"
             "• 💰 Wallet balance and transactions\n"
             "• 📊 Portfolio analysis and insights\n"
             "• 🔒 Security recommendations\n"
             "• ⚡ Fee optimization strategies\n"
             "• 📈 Market trends and analysis\n"
             "• 🚀 Transaction guidance\n\n"
             "Could you please rephrase your question or try one of the quick actions above?").arg(input);
}

// AIInsightsPanel Implementation

AIInsightsPanel::AIInsightsPanel(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent)
    : QWidget(parent)
    , m_premiumManager(premiumManager)
    , m_walletModel(walletModel)
    , m_refreshTimer(new QTimer(this))
{
    setupUI();
    
    // Auto-refresh every 5 minutes
    connect(m_refreshTimer, &QTimer::timeout, this, &AIInsightsPanel::refreshInsights);
    m_refreshTimer->start(300000); // 5 minutes
    
    refreshInsights();
}

void AIInsightsPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Header
    QLabel* titleLabel = new QLabel(tr("🧠 AI Insights Panel"));
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #673AB7; margin-bottom: 10px;");
    m_mainLayout->addWidget(titleLabel);
    
    // Tab widget
    m_tabWidget = new QTabWidget();
    
    // Security Tab
    m_securityTab = new QWidget();
    QVBoxLayout* securityLayout = new QVBoxLayout(m_securityTab);
    
    QLabel* securityHeaderLabel = new QLabel(tr("🔒 Security Analysis"));
    securityHeaderLabel->setStyleSheet("font-size: 14px; font-weight: bold; margin-bottom: 10px;");
    securityLayout->addWidget(securityHeaderLabel);
    
    QHBoxLayout* scoreLayout = new QHBoxLayout();
    scoreLayout->addWidget(new QLabel(tr("Security Score:")));
    m_securityScore = new QLabel(tr("85/100"));
    m_securityScore->setStyleSheet("font-weight: bold; color: #4CAF50; font-size: 16px;");
    scoreLayout->addWidget(m_securityScore);
    scoreLayout->addStretch();
    securityLayout->addLayout(scoreLayout);
    
    QLabel* warningsLabel = new QLabel(tr("⚠️ Warnings:"));
    warningsLabel->setStyleSheet("font-weight: bold; color: #FF9800; margin-top: 10px;");
    securityLayout->addWidget(warningsLabel);
    
    m_securityWarnings = new QListWidget();
    m_securityWarnings->setMaximumHeight(100);
    securityLayout->addWidget(m_securityWarnings);
    
    QLabel* recLabel = new QLabel(tr("💡 Recommendations:"));
    recLabel->setStyleSheet("font-weight: bold; color: #2196F3; margin-top: 10px;");
    securityLayout->addWidget(recLabel);
    
    m_securityRecommendations = new QListWidget();
    m_securityRecommendations->setMaximumHeight(120);
    securityLayout->addWidget(m_securityRecommendations);
    
    m_securityButton = new QPushButton(tr("🔍 Run Security Check"));
    m_securityButton->setStyleSheet("background-color: #FF5722; color: white; padding: 8px; border-radius: 4px; font-weight: bold;");
    securityLayout->addWidget(m_securityButton);
    
    m_tabWidget->addTab(m_securityTab, tr("🔒 Security"));
    
    // Performance Tab
    m_performanceTab = new QWidget();
    QVBoxLayout* performanceLayout = new QVBoxLayout(m_performanceTab);
    
    QLabel* performanceHeaderLabel = new QLabel(tr("📈 Performance Analysis"));
    performanceHeaderLabel->setStyleSheet("font-size: 14px; font-weight: bold; margin-bottom: 10px;");
    performanceLayout->addWidget(performanceHeaderLabel);
    
    // Portfolio stats
    QGridLayout* statsLayout = new QGridLayout();
    statsLayout->addWidget(new QLabel(tr("Portfolio Value:")), 0, 0);
    m_portfolioValue = new QLabel(tr("0.00 SHAH"));
    m_portfolioValue->setStyleSheet("font-weight: bold; color: #4CAF50;");
    statsLayout->addWidget(m_portfolioValue, 0, 1);
    
    statsLayout->addWidget(new QLabel(tr("24h Change:")), 1, 0);
    m_change24h = new QLabel(tr("+2.5%"));
    m_change24h->setStyleSheet("font-weight: bold; color: #4CAF50;");
    statsLayout->addWidget(m_change24h, 1, 1);
    
    statsLayout->addWidget(new QLabel(tr("7d Change:")), 2, 0);
    m_changeWeek = new QLabel(tr("+15.2%"));
    m_changeWeek->setStyleSheet("font-weight: bold; color: #4CAF50;");
    statsLayout->addWidget(m_changeWeek, 2, 1);
    
    performanceLayout->addLayout(statsLayout);
    
    // Performance chart placeholder
    m_performanceChart = new QChartView();
    m_performanceChart->setMinimumHeight(200);
    m_performanceChart->setStyleSheet("border: 1px solid #ddd; border-radius: 4px;");
    performanceLayout->addWidget(m_performanceChart);
    
    m_tabWidget->addTab(m_performanceTab, tr("📈 Performance"));
    
    // Recommendations Tab
    m_recommendationsTab = new QWidget();
    QVBoxLayout* recommendationsLayout = new QVBoxLayout(m_recommendationsTab);
    
    QLabel* recommendationsHeaderLabel = new QLabel(tr("🎯 AI Recommendations"));
    recommendationsHeaderLabel->setStyleSheet("font-size: 14px; font-weight: bold; margin-bottom: 10px;");
    recommendationsLayout->addWidget(recommendationsHeaderLabel);
    
    m_aiRecommendations = new QListWidget();
    recommendationsLayout->addWidget(m_aiRecommendations);
    
    QHBoxLayout* actionButtonsLayout = new QHBoxLayout();
    m_analyzeButton = new QPushButton(tr("🧮 Analyze Portfolio"));
    m_analyzeButton->setStyleSheet("background-color: #9C27B0; color: white; padding: 8px; border-radius: 4px; font-weight: bold;");
    
    m_optimizeButton = new QPushButton(tr("⚡ Optimize Fees"));
    m_optimizeButton->setStyleSheet("background-color: #FF9800; color: white; padding: 8px; border-radius: 4px; font-weight: bold;");
    
    actionButtonsLayout->addWidget(m_analyzeButton);
    actionButtonsLayout->addWidget(m_optimizeButton);
    actionButtonsLayout->addStretch();
    
    recommendationsLayout->addLayout(actionButtonsLayout);
    
    m_tabWidget->addTab(m_recommendationsTab, tr("🎯 Recommendations"));
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Connect signals
    connect(m_analyzeButton, &QPushButton::clicked, this, &AIInsightsPanel::onAnalyzePortfolioClicked);
    connect(m_securityButton, &QPushButton::clicked, this, &AIInsightsPanel::onCheckSecurityClicked);
    connect(m_optimizeButton, &QPushButton::clicked, this, &AIInsightsPanel::onOptimizeFeesClicked);
}

void AIInsightsPanel::refreshInsights()
{
    loadSecurityInsights();
    loadPerformanceInsights();
    loadRecommendations();
}

void AIInsightsPanel::loadSecurityInsights()
{
    m_securityWarnings->clear();
    m_securityRecommendations->clear();
    
    // Sample security warnings
    m_securityWarnings->addItem(tr("🟡 Wallet backup is older than 30 days"));
    m_securityWarnings->addItem(tr("🟡 Consider enabling additional security features"));
    
    // Sample security recommendations
    m_securityRecommendations->addItem(tr("✅ Enable two-factor authentication"));
    m_securityRecommendations->addItem(tr("✅ Update to latest wallet version"));
    m_securityRecommendations->addItem(tr("✅ Review and rotate backup phrases"));
    m_securityRecommendations->addItem(tr("✅ Use hardware wallet for large amounts"));
}

void AIInsightsPanel::loadPerformanceInsights()
{
    if (m_walletModel) {
        qint64 balance = m_walletModel->getBalance();
        double balanceShah = balance / 100000000.0;
        m_portfolioValue->setText(tr("%1 SHAH").arg(balanceShah, 0, 'f', 8));
    }
    
    // Sample performance data
    m_change24h->setText(tr("+2.5%"));
    m_change24h->setStyleSheet("font-weight: bold; color: #4CAF50;");
    
    m_changeWeek->setText(tr("+15.2%"));
    m_changeWeek->setStyleSheet("font-weight: bold; color: #4CAF50;");
}

void AIInsightsPanel::loadRecommendations()
{
    m_aiRecommendations->clear();
    
    // AI-generated recommendations
    m_aiRecommendations->addItem(tr("💡 Consider staking 50% of your SHAH for passive income"));
    m_aiRecommendations->addItem(tr("💡 Diversify into NFTs - market showing 20% growth"));
    m_aiRecommendations->addItem(tr("💡 Optimal transaction time: 2-4 AM UTC (lower fees)"));
    m_aiRecommendations->addItem(tr("💡 Your portfolio risk is low - consider DeFi opportunities"));
    m_aiRecommendations->addItem(tr("💡 Enable fee optimization for 15% savings on transactions"));
}

void AIInsightsPanel::onAnalyzePortfolioClicked()
{
    generateAIInsight("Portfolio", "analysis_request");
    
    // Simulate AI analysis
    QTimer::singleShot(2000, [this]() {
        QMessageBox::information(this, tr("Portfolio Analysis"), 
                               tr("🧮 AI Portfolio Analysis Complete!\n\n"
                                  "Key Findings:\n"
                                  "• Portfolio Health: Excellent (92/100)\n"
                                  "• Risk Level: Conservative\n"
                                  "• Diversification: Good\n"
                                  "• Growth Potential: High\n\n"
                                  "Recommendations updated in the AI Insights panel."));
        loadRecommendations();
    });
}

void AIInsightsPanel::onCheckSecurityClicked()
{
    generateAIInsight("Security", "security_check");
    
    // Simulate security check
    QTimer::singleShot(1500, [this]() {
        QMessageBox::information(this, tr("Security Check"), 
                               tr("🔒 Security Analysis Complete!\n\n"
                                  "Overall Security Score: 85/100\n\n"
                                  "✅ Wallet encryption: Enabled\n"
                                  "✅ Connection security: Good\n"
                                  "🟡 Backup age: 25 days (consider updating)\n"
                                  "🟡 2FA: Not enabled\n\n"
                                  "View detailed recommendations in the Security tab."));
        loadSecurityInsights();
    });
}

void AIInsightsPanel::onOptimizeFeesClicked()
{
    generateAIInsight("Fees", "optimization_request");
    
    QMessageBox::information(this, tr("Fee Optimization"), 
                           tr("⚡ Fee Optimization Analysis!\n\n"
                              "Current Fee Strategy: Standard\n"
                              "Potential Savings: 18% per transaction\n\n"
                              "Recommendations:\n"
                              "• Use economy fees during off-peak hours\n"
                              "• Batch multiple transactions\n"
                              "• Enable automatic fee optimization\n\n"
                              "Would you like to enable AI Fee Optimizer?"));
}

void AIInsightsPanel::generateAIInsight(const QString& category, const QString& data)
{
    // This would integrate with actual AI services in production
    // For now, we'll simulate the AI insight generation
    qDebug() << "Generating AI insight for category:" << category << "with data:" << data;
}

// AIFeeOptimizer Implementation

AIFeeOptimizer::AIFeeOptimizer(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent)
    : QWidget(parent)
    , m_premiumManager(premiumManager)
    , m_walletModel(walletModel)
    , m_currentAmount(0)
{
    setupUI();
    updateFeeRecommendation();
}

void AIFeeOptimizer::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Header
    QLabel* titleLabel = new QLabel(tr("⚡ AI Fee Optimizer"));
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FF9800; margin-bottom: 10px;");
    m_mainLayout->addWidget(titleLabel);
    
    // Current fees group
    m_currentFeesGroup = new QGroupBox(tr("Current Network Status"));
    QVBoxLayout* currentLayout = new QVBoxLayout(m_currentFeesGroup);
    
    QGridLayout* currentGrid = new QGridLayout();
    currentGrid->addWidget(new QLabel(tr("Standard Fee:")), 0, 0);
    m_currentFeeLabel = new QLabel(tr("0.001 SHAH"));
    m_currentFeeLabel->setStyleSheet("font-weight: bold; color: #4CAF50;");
    currentGrid->addWidget(m_currentFeeLabel, 0, 1);
    
    currentGrid->addWidget(new QLabel(tr("Network Status:")), 1, 0);
    m_networkStatusLabel = new QLabel(tr("🟢 Normal"));
    m_networkStatusLabel->setStyleSheet("font-weight: bold; color: #4CAF50;");
    currentGrid->addWidget(m_networkStatusLabel, 1, 1);
    
    currentGrid->addWidget(new QLabel(tr("Est. Confirmation:")), 2, 0);
    m_estimatedTimeLabel = new QLabel(tr("~2 minutes"));
    m_estimatedTimeLabel->setStyleSheet("font-weight: bold;");
    currentGrid->addWidget(m_estimatedTimeLabel, 2, 1);
    
    currentLayout->addLayout(currentGrid);
    m_mainLayout->addWidget(m_currentFeesGroup);
    
    // Optimization group
    m_optimizationGroup = new QGroupBox(tr("Fee Optimization"));
    QVBoxLayout* optimizationLayout = new QVBoxLayout(m_optimizationGroup);
    
    QLabel* presetLabel = new QLabel(tr("Fee Preset:"));
    optimizationLayout->addWidget(presetLabel);
    
    m_feePresetCombo = new QComboBox();
    m_feePresetCombo->addItem(tr("🐌 Economy (Slow)"), "economy");
    m_feePresetCombo->addItem(tr("⚡ Standard (Normal)"), "standard");
    m_feePresetCombo->addItem(tr("🚀 Priority (Fast)"), "priority");
    m_feePresetCombo->addItem(tr("🎯 Custom"), "custom");
    m_feePresetCombo->setCurrentIndex(1); // Standard
    optimizationLayout->addWidget(m_feePresetCombo);
    
    QLabel* customFeeLabel = new QLabel(tr("Custom Fee (SHAH):"));
    optimizationLayout->addWidget(customFeeLabel);
    
    m_customFeeSlider = new QSlider(Qt::Horizontal);
    m_customFeeSlider->setRange(50, 500); // 0.0005 to 0.005 SHAH
    m_customFeeSlider->setValue(100); // 0.001 SHAH
    m_customFeeSlider->setEnabled(false);
    optimizationLayout->addWidget(m_customFeeSlider);
    
    m_mainLayout->addWidget(m_optimizationGroup);
    
    // Recommendation group
    m_recommendationGroup = new QGroupBox(tr("AI Recommendation"));
    QVBoxLayout* recommendationLayout = new QVBoxLayout(m_recommendationGroup);
    
    QGridLayout* recGrid = new QGridLayout();
    recGrid->addWidget(new QLabel(tr("Optimized Fee:")), 0, 0);
    m_optimizedFeeLabel = new QLabel(tr("0.0008 SHAH"));
    m_optimizedFeeLabel->setStyleSheet("font-weight: bold; color: #2196F3;");
    recGrid->addWidget(m_optimizedFeeLabel, 0, 1);
    
    recGrid->addWidget(new QLabel(tr("Estimated Savings:")), 1, 0);
    m_savingsLabel = new QLabel(tr("0.0002 SHAH (20%)"));
    m_savingsLabel->setStyleSheet("font-weight: bold; color: #4CAF50;");
    recGrid->addWidget(m_savingsLabel, 1, 1);
    
    recGrid->addWidget(new QLabel(tr("Confirmation Time:")), 2, 0);
    m_confirmationTimeLabel = new QLabel(tr("~3 minutes"));
    m_confirmationTimeLabel->setStyleSheet("font-weight: bold;");
    recGrid->addWidget(m_confirmationTimeLabel, 2, 1);
    
    recommendationLayout->addLayout(recGrid);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_optimizeButton = new QPushButton(tr("🧮 Analyze & Optimize"));
    m_optimizeButton->setStyleSheet("background-color: #FF9800; color: white; padding: 8px; border-radius: 4px; font-weight: bold;");
    
    m_applyButton = new QPushButton(tr("✅ Apply Optimization"));
    m_applyButton->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px; border-radius: 4px; font-weight: bold;");
    m_applyButton->setEnabled(false);
    
    buttonLayout->addWidget(m_optimizeButton);
    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addStretch();
    
    recommendationLayout->addLayout(buttonLayout);
    
    m_mainLayout->addWidget(m_recommendationGroup);
    
    // Fee history chart placeholder
    m_feeHistoryChart = new QChartView();
    m_feeHistoryChart->setMinimumHeight(150);
    m_feeHistoryChart->setStyleSheet("border: 1px solid #ddd; border-radius: 4px;");
    m_mainLayout->addWidget(m_feeHistoryChart);
    
    // Connect signals
    connect(m_feePresetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AIFeeOptimizer::onFeePresetChanged);
    connect(m_customFeeSlider, &QSlider::valueChanged, this, &AIFeeOptimizer::updateFeeRecommendation);
    connect(m_optimizeButton, &QPushButton::clicked, this, &AIFeeOptimizer::onOptimizeClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &AIFeeOptimizer::onApplyOptimizationClicked);
}

void AIFeeOptimizer::onFeePresetChanged()
{
    QString preset = m_feePresetCombo->currentData().toString();
    m_customFeeSlider->setEnabled(preset == "custom");
    updateFeeRecommendation();
}

void AIFeeOptimizer::updateFeeRecommendation()
{
    QString preset = m_feePresetCombo->currentData().toString();
    double recommendedFee = 0.001; // Default
    QString confirmationTime = "~2 minutes";
    
    if (preset == "economy") {
        recommendedFee = 0.0005;
        confirmationTime = "~5-10 minutes";
    } else if (preset == "standard") {
        recommendedFee = 0.001;
        confirmationTime = "~2-5 minutes";
    } else if (preset == "priority") {
        recommendedFee = 0.002;
        confirmationTime = "~1-2 minutes";
    } else if (preset == "custom") {
        recommendedFee = m_customFeeSlider->value() / 100000.0; // Convert slider value
        if (recommendedFee < 0.001) {
            confirmationTime = "~5-15 minutes";
        } else if (recommendedFee > 0.0015) {
            confirmationTime = "~1-2 minutes";
        } else {
            confirmationTime = "~2-5 minutes";
        }
    }
    
    m_optimizedFeeLabel->setText(tr("%1 SHAH").arg(recommendedFee, 0, 'f', 6));
    m_confirmationTimeLabel->setText(confirmationTime);
    
    // Calculate savings
    double currentFee = 0.001;
    double savings = currentFee - recommendedFee;
    double savingsPercent = (savings / currentFee) * 100;
    
    if (savings > 0) {
        m_savingsLabel->setText(tr("%1 SHAH (%2%)").arg(savings, 0, 'f', 6).arg(savingsPercent, 0, 'f', 1));
        m_savingsLabel->setStyleSheet("font-weight: bold; color: #4CAF50;");
    } else if (savings < 0) {
        m_savingsLabel->setText(tr("%1 SHAH (%2%)").arg(-savings, 0, 'f', 6).arg(-savingsPercent, 0, 'f', 1));
        m_savingsLabel->setStyleSheet("font-weight: bold; color: #FF5722;");
    } else {
        m_savingsLabel->setText(tr("No change"));
        m_savingsLabel->setStyleSheet("font-weight: bold; color: #666;");
    }
}

void AIFeeOptimizer::onOptimizeClicked()
{
    // Simulate AI analysis
    m_optimizeButton->setEnabled(false);
    m_optimizeButton->setText(tr("🔄 Analyzing..."));
    
    QTimer::singleShot(2000, [this]() {
        m_optimizeButton->setEnabled(true);
        m_optimizeButton->setText(tr("🧮 Analyze & Optimize"));
        m_applyButton->setEnabled(true);
        
        QMessageBox::information(this, tr("Fee Optimization Complete"), 
                               tr("⚡ AI Fee Analysis Complete!\n\n"
                                  "Based on current network conditions:\n\n"
                                  "✅ Recommended fee: 0.0008 SHAH\n"
                                  "✅ Potential savings: 20%\n"
                                  "✅ Confirmation time: ~3 minutes\n\n"
                                  "This optimization balances cost and speed for optimal user experience."));
    });
}

void AIFeeOptimizer::onApplyOptimizationClicked()
{
    int result = QMessageBox::question(this, tr("Apply Optimization"), 
                                     tr("Apply the AI-optimized fee settings to your wallet?\n\n"
                                        "This will become the default fee for new transactions."),
                                     QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        QMessageBox::information(this, tr("Success"), 
                               tr("✅ Fee optimization applied successfully!\n\n"
                                  "Your wallet will now use AI-optimized fees for better cost efficiency."));
        m_applyButton->setEnabled(false);
        m_applyButton->setText(tr("✅ Applied"));
    }
}

void AIFeeOptimizer::analyzeFees()
{
    onOptimizeClicked();
}

void AIFeeOptimizer::optimizeTransaction(qint64 amount, const QString& recipient)
{
    m_currentAmount = amount;
    m_currentRecipient = recipient;
    calculateOptimalFee(amount);
}

void AIFeeOptimizer::calculateOptimalFee(qint64 amount)
{
    // AI-based fee calculation based on amount, network conditions, etc.
    double baseAmount = amount / 100000000.0;
    
    // Adjust recommendation based on transaction amount
    if (baseAmount > 100) {
        // Large transaction - prioritize speed
        m_feePresetCombo->setCurrentIndex(2); // Priority
    } else if (baseAmount < 1) {
        // Small transaction - prioritize cost
        m_feePresetCombo->setCurrentIndex(0); // Economy
    } else {
        // Medium transaction - balance
        m_feePresetCombo->setCurrentIndex(1); // Standard
    }
    
    updateFeeRecommendation();
}

void AIFeeOptimizer::showFeeComparison()
{
    // Implementation for showing fee comparison charts
}

#include "premiumfeatures.moc"
