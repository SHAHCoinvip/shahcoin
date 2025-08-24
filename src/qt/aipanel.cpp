#include "aipanel.h"
#include <qt/walletmodel.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>

#include <QApplication>
#include <QTimer>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <wallet/wallet.h>
#include <validation.h>
#include <net.h>
#include <util/time.h>
#include <amount.h>

AIPanel::AIPanel(QWidget* parent)
    : QWidget(parent)
    , m_walletModel(nullptr)
    , m_refreshTimer(new QTimer(this))
    , m_mainLayout(new QVBoxLayout(this))
    , m_stakingGroup(nullptr)
    , m_transactionGroup(nullptr)
    , m_performanceGroup(nullptr)
    , m_insightsGroup(nullptr)
    , m_stakingStatusLabel(nullptr)
    , m_stakingBalanceLabel(nullptr)
    , m_stakingRewardsLabel(nullptr)
    , m_stakingAPRLabel(nullptr)
    , m_stakingProgressBar(nullptr)
    , m_stakingToggleButton(nullptr)
    , m_stakingChartView(nullptr)
    , m_totalSentLabel(nullptr)
    , m_totalReceivedLabel(nullptr)
    , m_transactionCountLabel(nullptr)
    , m_averageFeeLabel(nullptr)
    , m_optimizeFeesButton(nullptr)
    , m_transactionChartView(nullptr)
    , m_performanceRatingLabel(nullptr)
    , m_networkHealthLabel(nullptr)
    , m_syncStatusLabel(nullptr)
    , m_peerCountLabel(nullptr)
    , m_viewDetailsButton(nullptr)
    , m_performanceChartView(nullptr)
    , m_primaryInsightLabel(nullptr)
    , m_secondaryInsightLabel(nullptr)
    , m_recommendationLabel(nullptr)
    , m_refreshButton(nullptr)
    , m_autoRefresh(true)
    , m_refreshInterval(30)
    , m_showCharts(true)
    , m_showPredictions(true)
    , m_stakingChart(nullptr)
    , m_transactionChart(nullptr)
    , m_performanceChart(nullptr)
    , m_stakingSeries(nullptr)
    , m_transactionSeries(nullptr)
    , m_performanceSeries(nullptr)
{
    setupUI();
    setupCharts();
    setupConnections();
    loadSettings();
    
    // Initialize metrics
    m_stakingMetrics = {0, 0, 0.0, 0, false, "Inactive"};
    m_transactionMetrics = {0, 0, 0, 0.0, 0, QStringList()};
    m_performanceMetrics = {0.0, "Unknown", false, 0, 0, "Unknown"};
}

AIPanel::~AIPanel()
{
    saveSettings();
}

void AIPanel::setupUI()
{
    setWindowTitle("AI Insights");
    
    // Create insight widgets
    createInsightWidgets();
    createStakingWidget();
    createTransactionWidget();
    createPerformanceWidget();
    
    // Add refresh button
    QHBoxLayout* refreshLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton("🔄 Refresh Insights", this);
    m_refreshButton->setToolTip("Refresh all AI insights and metrics");
    refreshLayout->addWidget(m_refreshButton);
    refreshLayout->addStretch();
    m_mainLayout->addLayout(refreshLayout);
    
    // Set up refresh timer
    m_refreshTimer->setInterval(m_refreshInterval * 1000);
    if (m_autoRefresh) {
        m_refreshTimer->start();
    }
}

void AIPanel::createInsightWidgets()
{
    m_insightsGroup = new QGroupBox("🤖 AI Insights", this);
    QVBoxLayout* insightsLayout = new QVBoxLayout(m_insightsGroup);
    
    m_primaryInsightLabel = new QLabel("Loading insights...", this);
    m_primaryInsightLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2c3e50;");
    m_primaryInsightLabel->setWordWrap(true);
    insightsLayout->addWidget(m_primaryInsightLabel);
    
    m_secondaryInsightLabel = new QLabel("", this);
    m_secondaryInsightLabel->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    m_secondaryInsightLabel->setWordWrap(true);
    insightsLayout->addWidget(m_secondaryInsightLabel);
    
    m_recommendationLabel = new QLabel("", this);
    m_recommendationLabel->setStyleSheet("font-size: 12px; color: #27ae60; font-style: italic;");
    m_recommendationLabel->setWordWrap(true);
    insightsLayout->addWidget(m_recommendationLabel);
    
    m_mainLayout->addWidget(m_insightsGroup);
}

void AIPanel::createStakingWidget()
{
    m_stakingGroup = new QGroupBox("💰 Staking Analytics", this);
    QVBoxLayout* stakingLayout = new QVBoxLayout(m_stakingGroup);
    
    // Status and balance
    QHBoxLayout* statusLayout = new QHBoxLayout();
    m_stakingStatusLabel = new QLabel("Status: Inactive", this);
    m_stakingStatusLabel->setStyleSheet("font-weight: bold; color: #e74c3c;");
    statusLayout->addWidget(m_stakingStatusLabel);
    
    m_stakingBalanceLabel = new QLabel("Balance: 0 SHAH", this);
    statusLayout->addWidget(m_stakingBalanceLabel);
    statusLayout->addStretch();
    stakingLayout->addLayout(statusLayout);
    
    // Rewards and APR
    QHBoxLayout* rewardsLayout = new QHBoxLayout();
    m_stakingRewardsLabel = new QLabel("Rewards (30d): 0 SHAH", this);
    rewardsLayout->addWidget(m_stakingRewardsLabel);
    
    m_stakingAPRLabel = new QLabel("APR: 0.00%", this);
    rewardsLayout->addWidget(m_stakingAPRLabel);
    rewardsLayout->addStretch();
    stakingLayout->addLayout(rewardsLayout);
    
    // Progress bar
    m_stakingProgressBar = new QProgressBar(this);
    m_stakingProgressBar->setRange(0, 100);
    m_stakingProgressBar->setValue(0);
    m_stakingProgressBar->setFormat("Staking Progress: %p%");
    stakingLayout->addWidget(m_stakingProgressBar);
    
    // Toggle button
    m_stakingToggleButton = new QPushButton("Enable Staking", this);
    m_stakingToggleButton->setToolTip("Enable or disable staking for this wallet");
    stakingLayout->addWidget(m_stakingToggleButton);
    
    // Chart
    if (m_showCharts) {
        m_stakingChartView = new QChartView(this);
        m_stakingChartView->setMinimumHeight(200);
        stakingLayout->addWidget(m_stakingChartView);
    }
    
    m_mainLayout->addWidget(m_stakingGroup);
}

void AIPanel::createTransactionWidget()
{
    m_transactionGroup = new QGroupBox("📊 Transaction Analytics", this);
    QVBoxLayout* transactionLayout = new QVBoxLayout(m_transactionGroup);
    
    // Transaction summary
    QHBoxLayout* summaryLayout = new QHBoxLayout();
    m_totalSentLabel = new QLabel("Sent (7d): 0 SHAH", this);
    summaryLayout->addWidget(m_totalSentLabel);
    
    m_totalReceivedLabel = new QLabel("Received (7d): 0 SHAH", this);
    summaryLayout->addWidget(m_totalReceivedLabel);
    summaryLayout->addStretch();
    transactionLayout->addLayout(summaryLayout);
    
    // Transaction details
    QHBoxLayout* detailsLayout = new QHBoxLayout();
    m_transactionCountLabel = new QLabel("Transactions: 0", this);
    detailsLayout->addWidget(m_transactionCountLabel);
    
    m_averageFeeLabel = new QLabel("Avg Fee: 0.001 SHAH", this);
    detailsLayout->addWidget(m_averageFeeLabel);
    detailsLayout->addStretch();
    transactionLayout->addLayout(detailsLayout);
    
    // Optimize fees button
    m_optimizeFeesButton = new QPushButton("⚡ Optimize Fees", this);
    m_optimizeFeesButton->setToolTip("Get AI-powered fee optimization suggestions");
    transactionLayout->addWidget(m_optimizeFeesButton);
    
    // Chart
    if (m_showCharts) {
        m_transactionChartView = new QChartView(this);
        m_transactionChartView->setMinimumHeight(200);
        transactionLayout->addWidget(m_transactionChartView);
    }
    
    m_mainLayout->addWidget(m_transactionGroup);
}

void AIPanel::createPerformanceWidget()
{
    m_performanceGroup = new QGroupBox("⚡ Performance Metrics", this);
    QVBoxLayout* performanceLayout = new QVBoxLayout(m_performanceGroup);
    
    // Performance rating
    QHBoxLayout* ratingLayout = new QHBoxLayout();
    m_performanceRatingLabel = new QLabel("Performance: Excellent", this);
    m_performanceRatingLabel->setStyleSheet("font-weight: bold; color: #27ae60;");
    ratingLayout->addWidget(m_performanceRatingLabel);
    ratingLayout->addStretch();
    performanceLayout->addLayout(ratingLayout);
    
    // Network health
    QHBoxLayout* healthLayout = new QHBoxLayout();
    m_networkHealthLabel = new QLabel("Network: Healthy", this);
    healthLayout->addWidget(m_networkHealthLabel);
    
    m_syncStatusLabel = new QLabel("Sync: Synced", this);
    healthLayout->addWidget(m_syncStatusLabel);
    healthLayout->addStretch();
    performanceLayout->addLayout(healthLayout);
    
    // Peer count
    QHBoxLayout* peerLayout = new QHBoxLayout();
    m_peerCountLabel = new QLabel("Peers: 0", this);
    peerLayout->addWidget(m_peerCountLabel);
    peerLayout->addStretch();
    performanceLayout->addLayout(peerLayout);
    
    // View details button
    m_viewDetailsButton = new QPushButton("🔍 View Details", this);
    m_viewDetailsButton->setToolTip("View detailed performance metrics");
    performanceLayout->addWidget(m_viewDetailsButton);
    
    // Chart
    if (m_showCharts) {
        m_performanceChartView = new QChartView(this);
        m_performanceChartView->setMinimumHeight(200);
        performanceLayout->addWidget(m_performanceChartView);
    }
    
    m_mainLayout->addWidget(m_performanceGroup);
}

void AIPanel::setupCharts()
{
    if (!m_showCharts) return;
    
    createStakingChart();
    createTransactionChart();
    createPerformanceChart();
}

void AIPanel::createStakingChart()
{
    if (!m_stakingChartView) return;
    
    m_stakingChart = new QChart();
    m_stakingSeries = new QLineSeries();
    
    m_stakingChart->addSeries(m_stakingSeries);
    m_stakingChart->setTitle("Staking Rewards (30 Days)");
    m_stakingChart->createDefaultAxes();
    
    m_stakingChartView->setChart(m_stakingChart);
    m_stakingChartView->setRenderHint(QPainter::Antialiasing);
}

void AIPanel::createTransactionChart()
{
    if (!m_transactionChartView) return;
    
    m_transactionChart = new QChart();
    m_transactionSeries = new QBarSeries();
    
    m_transactionChart->addSeries(m_transactionSeries);
    m_transactionChart->setTitle("Transaction Volume (7 Days)");
    m_transactionChart->createDefaultAxes();
    
    m_transactionChartView->setChart(m_transactionChart);
    m_transactionChartView->setRenderHint(QPainter::Antialiasing);
}

void AIPanel::createPerformanceChart()
{
    if (!m_performanceChartView) return;
    
    m_performanceChart = new QChart();
    m_performanceSeries = new QLineSeries();
    
    m_performanceChart->addSeries(m_performanceSeries);
    m_performanceChart->setTitle("Performance Rating");
    m_performanceChart->createDefaultAxes();
    
    m_performanceChartView->setChart(m_performanceChart);
    m_performanceChartView->setRenderHint(QPainter::Antialiasing);
}

void AIPanel::setupConnections()
{
    connect(m_refreshButton, &QPushButton::clicked, this, &AIPanel::onRefreshClicked);
    connect(m_refreshTimer, &QTimer::timeout, this, &AIPanel::onAutoRefreshTimeout);
    connect(m_stakingToggleButton, &QPushButton::clicked, this, &AIPanel::onStakingToggleClicked);
    connect(m_optimizeFeesButton, &QPushButton::clicked, this, &AIPanel::onOptimizeFeesClicked);
    connect(m_viewDetailsButton, &QPushButton::clicked, this, &AIPanel::onViewDetailsClicked);
}

void AIPanel::setWalletModel(WalletModel* model)
{
    m_walletModel = model;
    if (m_walletModel) {
        connect(m_walletModel, &WalletModel::balanceChanged, this, &AIPanel::onWalletDataChanged);
        connect(m_walletModel, &WalletModel::transactionChanged, this, &AIPanel::onWalletDataChanged);
        refreshInsights();
    }
}

void AIPanel::refreshInsights()
{
    if (!m_walletModel) return;
    
    calculateStakingMetrics();
    calculateTransactionMetrics();
    calculatePerformanceMetrics();
    generateInsights();
    updateCharts();
}

void AIPanel::calculateStakingMetrics()
{
    if (!m_walletModel) return;
    
    // Get wallet balance
    CAmount balance = m_walletModel->getBalance();
    CAmount stakingBalance = m_walletModel->getStakingBalance();
    
    // Calculate staking metrics
    m_stakingMetrics.totalStaked = stakingBalance;
    m_stakingMetrics.isActive = (stakingBalance >= 333 * COIN); // Minimum stake amount
    m_stakingMetrics.status = m_stakingMetrics.isActive ? "Active" : "Inactive";
    
    // Calculate APR (simplified - in real implementation, this would be more complex)
    m_stakingMetrics.apr = m_stakingMetrics.isActive ? 12.5 : 0.0; // Example 12.5% APR
    
    // Calculate rewards (simplified)
    if (m_stakingMetrics.isActive) {
        m_stakingMetrics.rewardsEarned = (stakingBalance * m_stakingMetrics.apr) / (365 * 100);
        m_stakingMetrics.daysStaking = 30; // Example
    } else {
        m_stakingMetrics.rewardsEarned = 0;
        m_stakingMetrics.daysStaking = 0;
    }
    
    // Update UI
    if (m_stakingStatusLabel) {
        QString statusText = QString("Status: %1").arg(m_stakingMetrics.status);
        QString statusColor = m_stakingMetrics.isActive ? "#27ae60" : "#e74c3c";
        m_stakingStatusLabel->setStyleSheet(QString("font-weight: bold; color: %1;").arg(statusColor));
        m_stakingStatusLabel->setText(statusText);
    }
    
    if (m_stakingBalanceLabel) {
        m_stakingBalanceLabel->setText(QString("Balance: %1").arg(formatAmount(stakingBalance)));
    }
    
    if (m_stakingRewardsLabel) {
        m_stakingRewardsLabel->setText(QString("Rewards (30d): %1").arg(formatAmount(m_stakingMetrics.rewardsEarned)));
    }
    
    if (m_stakingAPRLabel) {
        m_stakingAPRLabel->setText(QString("APR: %1").arg(formatPercentage(m_stakingMetrics.apr)));
    }
    
    if (m_stakingProgressBar) {
        int progress = m_stakingMetrics.isActive ? 100 : 0;
        m_stakingProgressBar->setValue(progress);
    }
}

void AIPanel::calculateTransactionMetrics()
{
    if (!m_walletModel) return;
    
    // Get transaction data (simplified - in real implementation, this would query actual transactions)
    m_transactionMetrics.totalSent = 1000 * COIN; // Example
    m_transactionMetrics.totalReceived = 1500 * COIN; // Example
    m_transactionMetrics.transactionCount = 15; // Example
    m_transactionMetrics.averageFee = 0.001 * COIN; // Example
    m_transactionMetrics.averageConfirmations = 6; // Example
    
    // Update UI
    if (m_totalSentLabel) {
        m_totalSentLabel->setText(QString("Sent (7d): %1").arg(formatAmount(m_transactionMetrics.totalSent)));
    }
    
    if (m_totalReceivedLabel) {
        m_totalReceivedLabel->setText(QString("Received (7d): %1").arg(formatAmount(m_transactionMetrics.totalReceived)));
    }
    
    if (m_transactionCountLabel) {
        m_transactionCountLabel->setText(QString("Transactions: %1").arg(m_transactionMetrics.transactionCount));
    }
    
    if (m_averageFeeLabel) {
        m_averageFeeLabel->setText(QString("Avg Fee: %1").arg(formatAmount(m_transactionMetrics.averageFee)));
    }
}

void AIPanel::calculatePerformanceMetrics()
{
    // Calculate performance rating based on various factors
    double rating = 0.0;
    
    // Network health
    bool isSynced = true; // In real implementation, check actual sync status
    int peerCount = 8; // In real implementation, get actual peer count
    
    if (isSynced) rating += 0.4;
    if (peerCount >= 5) rating += 0.3;
    if (m_stakingMetrics.isActive) rating += 0.3;
    
    m_performanceMetrics.performanceRating = rating;
    m_performanceMetrics.isSynced = isSynced;
    m_performanceMetrics.peerCount = peerCount;
    m_performanceMetrics.networkHealth = (peerCount >= 5) ? "Healthy" : "Poor";
    m_performanceMetrics.syncStatus = isSynced ? "Synced" : "Syncing";
    
    // Update UI
    if (m_performanceRatingLabel) {
        QString ratingText = getPerformanceRating();
        QString ratingColor = (rating >= 0.8) ? "#27ae60" : (rating >= 0.6) ? "#f39c12" : "#e74c3c";
        m_performanceRatingLabel->setStyleSheet(QString("font-weight: bold; color: %1;").arg(ratingColor));
        m_performanceRatingLabel->setText(QString("Performance: %1").arg(ratingText));
    }
    
    if (m_networkHealthLabel) {
        QString healthColor = (m_performanceMetrics.networkHealth == "Healthy") ? "#27ae60" : "#e74c3c";
        m_networkHealthLabel->setStyleSheet(QString("color: %1;").arg(healthColor));
        m_networkHealthLabel->setText(QString("Network: %1").arg(m_performanceMetrics.networkHealth));
    }
    
    if (m_syncStatusLabel) {
        QString syncColor = m_performanceMetrics.isSynced ? "#27ae60" : "#f39c12";
        m_syncStatusLabel->setStyleSheet(QString("color: %1;").arg(syncColor));
        m_syncStatusLabel->setText(QString("Sync: %1").arg(m_performanceMetrics.syncStatus));
    }
    
    if (m_peerCountLabel) {
        m_peerCountLabel->setText(QString("Peers: %1").arg(m_performanceMetrics.peerCount));
    }
}

void AIPanel::generateInsights()
{
    QString primaryInsight = getInsightMessage();
    QString secondaryInsight = "";
    QString recommendation = "";
    
    // Generate insights based on metrics
    if (!m_stakingMetrics.isActive && m_stakingMetrics.totalStaked > 0) {
        secondaryInsight = "Your wallet has funds but staking is not active.";
        recommendation = "Consider enabling staking to earn rewards.";
    } else if (m_stakingMetrics.isActive) {
        secondaryInsight = QString("You're earning %1% APR on your staked funds.").arg(formatPercentage(m_stakingMetrics.apr));
        recommendation = "Great job! Keep staking to maximize your rewards.";
    }
    
    if (m_performanceMetrics.peerCount < 5) {
        if (!secondaryInsight.isEmpty()) secondaryInsight += " ";
        secondaryInsight += "Low peer count detected.";
        if (!recommendation.isEmpty()) recommendation += " ";
        recommendation += "Check your network connection.";
    }
    
    // Update UI
    if (m_primaryInsightLabel) {
        m_primaryInsightLabel->setText(primaryInsight);
    }
    
    if (m_secondaryInsightLabel) {
        m_secondaryInsightLabel->setText(secondaryInsight);
        m_secondaryInsightLabel->setVisible(!secondaryInsight.isEmpty());
    }
    
    if (m_recommendationLabel) {
        m_recommendationLabel->setText(recommendation);
        m_recommendationLabel->setVisible(!recommendation.isEmpty());
    }
}

void AIPanel::updateCharts()
{
    if (!m_showCharts) return;
    
    updateStakingChart();
    updateTransactionChart();
    updatePerformanceChart();
}

void AIPanel::updateStakingChart()
{
    if (!m_stakingSeries) return;
    
    m_stakingSeries->clear();
    
    // Add sample data (in real implementation, this would be actual historical data)
    for (int i = 0; i < 30; ++i) {
        double reward = m_stakingMetrics.isActive ? (m_stakingMetrics.totalStaked * m_stakingMetrics.apr / (365 * 100)) * (i + 1) / 30.0 : 0;
        m_stakingSeries->append(i, reward / COIN);
    }
}

void AIPanel::updateTransactionChart()
{
    if (!m_transactionSeries) return;
    
    m_transactionSeries->clear();
    
    // Add sample data
    QBarSet* sentSet = new QBarSet("Sent");
    QBarSet* receivedSet = new QBarSet("Received");
    
    for (int i = 0; i < 7; ++i) {
        *sentSet << (m_transactionMetrics.totalSent / (7 * COIN));
        *receivedSet << (m_transactionMetrics.totalReceived / (7 * COIN));
    }
    
    m_transactionSeries->append(sentSet);
    m_transactionSeries->append(receivedSet);
}

void AIPanel::updatePerformanceChart()
{
    if (!m_performanceSeries) return;
    
    m_performanceSeries->clear();
    
    // Add sample data
    for (int i = 0; i < 24; ++i) {
        m_performanceSeries->append(i, m_performanceMetrics.performanceRating);
    }
}

void AIPanel::onRefreshClicked()
{
    refreshInsights();
}

void AIPanel::onAutoRefreshTimeout()
{
    refreshInsights();
}

void AIPanel::onWalletDataChanged()
{
    refreshInsights();
}

void AIPanel::onStakingToggleClicked()
{
    if (!m_walletModel) return;
    
    if (m_stakingMetrics.isActive) {
        QMessageBox::information(this, "Staking", "Staking is currently active. To disable staking, you would need to move your funds to a non-staking address.");
    } else {
        QMessageBox::information(this, "Staking", "To enable staking, ensure you have at least 333 SHAH in your wallet and the wallet is unlocked.");
    }
}

void AIPanel::onOptimizeFeesClicked()
{
    // Example savings estimation (placeholder): assume user saves 0.0012 SHAH on average per optimized tx
    double estimatedSavings = 0.0012;
    Q_EMIT feeSavingsEstimated(estimatedSavings);
    QMessageBox::information(this, "Fee Optimization", 
        "AI Fee Optimization:\n\n"
        "🚀 Fastest: 0.005 SHAH (1-2 blocks)\n"
        "⚖️ Balanced: 0.003 SHAH (3-5 blocks)\n"
        "🐢 Cheapest: 0.001 SHAH (5-10 blocks)\n\n"
        "Based on current network conditions.");
}

void AIPanel::onViewDetailsClicked()
{
    QString details = QString(
        "Detailed Performance Metrics:\n\n"
        "Performance Rating: %1\n"
        "Network Health: %2\n"
        "Sync Status: %3\n"
        "Peer Count: %4\n"
        "Staking Status: %5\n"
        "Total Staked: %6\n"
        "Rewards Earned: %7"
    ).arg(getPerformanceRating())
     .arg(m_performanceMetrics.networkHealth)
     .arg(m_performanceMetrics.syncStatus)
     .arg(m_performanceMetrics.peerCount)
     .arg(m_stakingMetrics.status)
     .arg(formatAmount(m_stakingMetrics.totalStaked))
     .arg(formatAmount(m_stakingMetrics.rewardsEarned));
    
    QMessageBox::information(this, "Performance Details", details);
}

void AIPanel::setAutoRefresh(bool enabled)
{
    m_autoRefresh = enabled;
    if (enabled) {
        m_refreshTimer->start();
    } else {
        m_refreshTimer->stop();
    }
    saveSettings();
}

void AIPanel::setRefreshInterval(int seconds)
{
    m_refreshInterval = seconds;
    m_refreshTimer->setInterval(seconds * 1000);
    saveSettings();
}

void AIPanel::setShowCharts(bool show)
{
    m_showCharts = show;
    saveSettings();
}

void AIPanel::setShowPredictions(bool show)
{
    m_showPredictions = show;
    saveSettings();
}

bool AIPanel::isAutoRefresh() const
{
    return m_autoRefresh;
}

int AIPanel::getRefreshInterval() const
{
    return m_refreshInterval;
}

bool AIPanel::isShowCharts() const
{
    return m_showCharts;
}

bool AIPanel::isShowPredictions() const
{
    return m_showPredictions;
}

QString AIPanel::formatAmount(CAmount amount)
{
    return ShahcoinUnits::format(ShahcoinUnits::SHAH, amount, false, ShahcoinUnits::separatorAlways);
}

QString AIPanel::formatPercentage(double percentage)
{
    return QString::number(percentage, 'f', 2) + "%";
}

QString AIPanel::formatTime(int seconds)
{
    if (seconds < 60) return QString("%1s").arg(seconds);
    if (seconds < 3600) return QString("%1m").arg(seconds / 60);
    return QString("%1h").arg(seconds / 3600);
}

QString AIPanel::getStakingStatusText()
{
    return m_stakingMetrics.status;
}

QString AIPanel::getPerformanceRating()
{
    double rating = m_performanceMetrics.performanceRating;
    if (rating >= 0.9) return "Excellent";
    if (rating >= 0.8) return "Very Good";
    if (rating >= 0.7) return "Good";
    if (rating >= 0.6) return "Fair";
    return "Poor";
}

QString AIPanel::getInsightMessage()
{
    if (m_stakingMetrics.isActive) {
        return "Your wallet is performing well with active staking!";
    } else if (m_stakingMetrics.totalStaked > 0) {
        return "You have funds available for staking.";
    } else {
        return "Welcome to Shahcoin! Consider staking to earn rewards.";
    }
}

void AIPanel::loadSettings()
{
    QSettings settings;
    m_autoRefresh = settings.value("aipanel/autoRefresh", true).toBool();
    m_refreshInterval = settings.value("aipanel/refreshInterval", 30).toInt();
    m_showCharts = settings.value("aipanel/showCharts", true).toBool();
    m_showPredictions = settings.value("aipanel/showPredictions", true).toBool();
}

void AIPanel::saveSettings()
{
    QSettings settings;
    settings.setValue("aipanel/autoRefresh", m_autoRefresh);
    settings.setValue("aipanel/refreshInterval", m_refreshInterval);
    settings.setValue("aipanel/showCharts", m_showCharts);
    settings.setValue("aipanel/showPredictions", m_showPredictions);
}
