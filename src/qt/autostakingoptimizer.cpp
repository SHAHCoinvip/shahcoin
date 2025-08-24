#include "autostakingoptimizer.h"
#include "walletmodel.h"
#include "guiutil.h"
#include "platformstyle.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QBuffer>
#include <QPainter>
#include <QPen>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QTimer>
#include <QMessageBox>
#include <QProgressDialog>
#include <QInputDialog>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSlider>
#include <QDateTimeEdit>
#include <QCalendarWidget>

// Constants
const QString AutoStakingOptimizer::SETTINGS_GROUP = "AutoStakingOptimizer";
const QString AutoStakingOptimizer::SETTINGS_STRATEGY = "Strategy";
const QString AutoStakingOptimizer::SETTINGS_FREQUENCY = "Frequency";
const QString AutoStakingOptimizer::SETTINGS_MIN_AMOUNT = "MinAmount";
const QString AutoStakingOptimizer::SETTINGS_MAX_PERCENTAGE = "MaxPercentage";
const QString AutoStakingOptimizer::SETTINGS_AUTO_RESTAKE = "AutoRestake";
const QString AutoStakingOptimizer::SETTINGS_COMPOUND_REWARDS = "CompoundRewards";
const QString AutoStakingOptimizer::SETTINGS_NOTIFY_RESTAKE = "NotifyRestake";
const QString AutoStakingOptimizer::SETTINGS_RESTAKE_THRESHOLD = "RestakeThreshold";

AutoStakingOptimizer::AutoStakingOptimizer(WalletModel* walletModel, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(walletModel)
    , m_platformStyle(nullptr)
    , m_mainLayout(nullptr)
    , m_analysisGroup(nullptr)
    , m_analysisLayout(nullptr)
    , m_currentBalanceLabel(nullptr)
    , m_currentStakedLabel(nullptr)
    , m_currentRewardsLabel(nullptr)
    , m_analyzeButton(nullptr)
    , m_optimizationGroup(nullptr)
    , m_optimizationLayout(nullptr)
    , m_strategyLayout(nullptr)
    , m_strategyLabel(nullptr)
    , m_strategyCombo(nullptr)
    , m_frequencyLabel(nullptr)
    , m_frequencyCombo(nullptr)
    , m_optimizeButton(nullptr)
    , m_amountLayout(nullptr)
    , m_minAmountLabel(nullptr)
    , m_minAmountInput(nullptr)
    , m_maxPercentageLabel(nullptr)
    , m_maxPercentageSlider(nullptr)
    , m_maxPercentageValue(nullptr)
    , m_autoRestakeLayout(nullptr)
    , m_autoRestakeCheck(nullptr)
    , m_compoundRewardsCheck(nullptr)
    , m_notifyRestakeCheck(nullptr)
    , m_restakeThresholdLabel(nullptr)
    , m_restakeThresholdSpin(nullptr)
    , m_recommendationGroup(nullptr)
    , m_recommendationLayout(nullptr)
    , m_recommendedAmountLabel(nullptr)
    , m_potentialRewardLabel(nullptr)
    , m_compoundEffectLabel(nullptr)
    , m_strategyReasoningLabel(nullptr)
    , m_applyRecommendationButton(nullptr)
    , m_performanceGroup(nullptr)
    , m_performanceLayout(nullptr)
    , m_totalStakedLabel(nullptr)
    , m_totalRewardsLabel(nullptr)
    , m_averageAPYLabel(nullptr)
    , m_efficiencyScoreLabel(nullptr)
    , m_lastRestakeLabel(nullptr)
    , m_nextRestakeLabel(nullptr)
    , m_chartsGroup(nullptr)
    , m_chartTabs(nullptr)
    , m_performanceChartView(nullptr)
    , m_projectionChartView(nullptr)
    , m_performanceChart(nullptr)
    , m_projectionChart(nullptr)
    , m_performanceSeries(nullptr)
    , m_projectionSeries(nullptr)
    , m_projectionControlsLayout(nullptr)
    , m_projectionRangeLabel(nullptr)
    , m_projectionRangeCombo(nullptr)
    , m_showProjectionsButton(nullptr)
    , m_exportDataButton(nullptr)
    , m_settingsGroup(nullptr)
    , m_settingsLayout(nullptr)
    , m_resetSettingsButton(nullptr)
    , m_saveSettingsButton(nullptr)
    , m_optimizationInProgress(false)
    , m_autoRestakeEnabled(false)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_updateTimer(new QTimer(this))
    , m_restakeCheckTimer(new QTimer(this))
    , m_animationTimer(new QTimer(this))
    , m_settingsStorage(new QSettings("Shahcoin", "AutoStakingOptimizer"))
    , m_optimizationAnimation(new QPropertyAnimation(this))
    , m_optimizationOpacityEffect(new QGraphicsOpacityEffect(this))
{
    setWindowTitle(tr("♻️ Auto-Staking Optimizer"));
    setupUI();
    connectSignals();
    applyTheme();
    loadSettings();
    loadPerformanceHistory();

    // Initialize default settings
    m_settings.strategy = Balanced;
    m_settings.frequency = Monthly;
    m_settings.minStakeAmount = 1.0;
    m_settings.maxStakePercentage = 80.0;
    m_settings.autoRestake = false;
    m_settings.compoundRewards = true;
    m_settings.notifyOnRestake = true;
    m_settings.restakeThreshold = 10;

    // Initialize performance metrics
    m_currentMetrics.totalStaked = 0.0;
    m_currentMetrics.totalRewards = 0.0;
    m_currentMetrics.averageAPY = 0.0;
    m_currentMetrics.compoundGrowth = 0.0;
    m_currentMetrics.daysStaking = 0;
    m_currentMetrics.efficiencyScore = 0.0;
    m_currentMetrics.lastRestake = QDateTime::currentDateTime();
    m_currentMetrics.nextScheduledRestake = QDateTime::currentDateTime().addDays(30);

    // Start timers
    m_updateTimer->setInterval(UPDATE_INTERVAL_MS);
    m_restakeCheckTimer->setInterval(RESTAKE_CHECK_INTERVAL_MS);
    m_updateTimer->start();
    m_restakeCheckTimer->start();

    // Perform initial analysis
    analyzeCurrentStaking();
}

AutoStakingOptimizer::~AutoStakingOptimizer()
{
    saveSettings();
    savePerformanceHistory();
}

void AutoStakingOptimizer::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Analysis Section
    m_analysisGroup = new QGroupBox(tr("📊 Current Staking Analysis"), this);
    m_analysisLayout = new QHBoxLayout(m_analysisGroup);

    m_currentBalanceLabel = new QLabel(tr("Balance: Analyzing..."), this);
    m_currentStakedLabel = new QLabel(tr("Staked: Analyzing..."), this);
    m_currentRewardsLabel = new QLabel(tr("Rewards: Analyzing..."), this);
    m_analyzeButton = new QPushButton(tr("🔄 Analyze"), this);

    m_analysisLayout->addWidget(m_currentBalanceLabel);
    m_analysisLayout->addWidget(m_currentStakedLabel);
    m_analysisLayout->addWidget(m_currentRewardsLabel);
    m_analysisLayout->addWidget(m_analyzeButton);
    m_analysisLayout->addStretch();

    // Optimization Section
    m_optimizationGroup = new QGroupBox(tr("⚙️ Optimization Settings"), this);
    m_optimizationLayout = new QVBoxLayout(m_optimizationGroup);

    // Strategy Settings
    m_strategyLayout = new QHBoxLayout();
    m_strategyLabel = new QLabel(tr("Strategy:"), this);
    m_strategyCombo = new QComboBox(this);
    m_strategyCombo->addItem(tr("Conservative"), Conservative);
    m_strategyCombo->addItem(tr("Balanced"), Balanced);
    m_strategyCombo->addItem(tr("Aggressive"), Aggressive);
    m_strategyCombo->addItem(tr("Custom"), Custom);
    m_strategyCombo->setCurrentIndex(1); // Balanced

    m_frequencyLabel = new QLabel(tr("Frequency:"), this);
    m_frequencyCombo = new QComboBox(this);
    m_frequencyCombo->addItem(tr("Daily"), Daily);
    m_frequencyCombo->addItem(tr("Weekly"), Weekly);
    m_frequencyCombo->addItem(tr("Monthly"), Monthly);
    m_frequencyCombo->addItem(tr("Quarterly"), Quarterly);
    m_frequencyCombo->addItem(tr("Yearly"), Yearly);
    m_frequencyCombo->setCurrentIndex(2); // Monthly

    m_optimizeButton = new QPushButton(tr("🤖 Optimize"), this);

    m_strategyLayout->addWidget(m_strategyLabel);
    m_strategyLayout->addWidget(m_strategyCombo);
    m_strategyLayout->addWidget(m_frequencyLabel);
    m_strategyLayout->addWidget(m_frequencyCombo);
    m_strategyLayout->addWidget(m_optimizeButton);
    m_strategyLayout->addStretch();

    // Amount Settings
    m_amountLayout = new QHBoxLayout();
    m_minAmountLabel = new QLabel(tr("Min Amount (SHAH):"), this);
    m_minAmountInput = new QDoubleSpinBox(this);
    m_minAmountInput->setRange(0.1, 1000000.0);
    m_minAmountInput->setDecimals(2);
    m_minAmountInput->setValue(1.0);
    m_minAmountInput->setSuffix(" SHAH");

    m_maxPercentageLabel = new QLabel(tr("Max % of Balance:"), this);
    m_maxPercentageSlider = new QSlider(Qt::Horizontal, this);
    m_maxPercentageSlider->setRange(10, 100);
    m_maxPercentageSlider->setValue(80);
    m_maxPercentageValue = new QLabel(tr("80%"), this);

    m_amountLayout->addWidget(m_minAmountLabel);
    m_amountLayout->addWidget(m_minAmountInput);
    m_amountLayout->addWidget(m_maxPercentageLabel);
    m_amountLayout->addWidget(m_maxPercentageSlider);
    m_amountLayout->addWidget(m_maxPercentageValue);
    m_amountLayout->addStretch();

    // Auto-restake Settings
    m_autoRestakeLayout = new QHBoxLayout();
    m_autoRestakeCheck = new QCheckBox(tr("Auto-restake rewards"), this);
    m_compoundRewardsCheck = new QCheckBox(tr("Compound rewards"), this);
    m_compoundRewardsCheck->setChecked(true);
    m_notifyRestakeCheck = new QCheckBox(tr("Notify on restake"), this);
    m_notifyRestakeCheck->setChecked(true);

    m_restakeThresholdLabel = new QLabel(tr("Threshold (SHAH):"), this);
    m_restakeThresholdSpin = new QSpinBox(this);
    m_restakeThresholdSpin->setRange(1, 1000);
    m_restakeThresholdSpin->setValue(10);
    m_restakeThresholdSpin->setSuffix(" SHAH");

    m_autoRestakeLayout->addWidget(m_autoRestakeCheck);
    m_autoRestakeLayout->addWidget(m_compoundRewardsCheck);
    m_autoRestakeLayout->addWidget(m_notifyRestakeCheck);
    m_autoRestakeLayout->addWidget(m_restakeThresholdLabel);
    m_autoRestakeLayout->addWidget(m_restakeThresholdSpin);
    m_autoRestakeLayout->addStretch();

    m_optimizationLayout->addLayout(m_strategyLayout);
    m_optimizationLayout->addLayout(m_amountLayout);
    m_optimizationLayout->addLayout(m_autoRestakeLayout);

    // Recommendation Display
    m_recommendationGroup = new QGroupBox(tr("💡 Optimization Recommendation"), this);
    m_recommendationLayout = new QVBoxLayout(m_recommendationGroup);

    m_recommendedAmountLabel = new QLabel(tr("Recommended Amount: Waiting for analysis..."), this);
    m_potentialRewardLabel = new QLabel(tr("Potential Reward: --"), this);
    m_compoundEffectLabel = new QLabel(tr("Compound Effect: --"), this);
    m_strategyReasoningLabel = new QLabel(tr("Strategy: --"), this);
    m_applyRecommendationButton = new QPushButton(tr("✅ Apply Recommendation"), this);
    m_applyRecommendationButton->setEnabled(false);

    m_recommendationLayout->addWidget(m_recommendedAmountLabel);
    m_recommendationLayout->addWidget(m_potentialRewardLabel);
    m_recommendationLayout->addWidget(m_compoundEffectLabel);
    m_recommendationLayout->addWidget(m_strategyReasoningLabel);
    m_recommendationLayout->addWidget(m_applyRecommendationButton);

    // Performance Metrics
    m_performanceGroup = new QGroupBox(tr("📈 Performance Metrics"), this);
    m_performanceLayout = new QHBoxLayout(m_performanceGroup);

    m_totalStakedLabel = new QLabel(tr("Total Staked: --"), this);
    m_totalRewardsLabel = new QLabel(tr("Total Rewards: --"), this);
    m_averageAPYLabel = new QLabel(tr("Avg APY: --"), this);
    m_efficiencyScoreLabel = new QLabel(tr("Efficiency: --"), this);
    m_lastRestakeLabel = new QLabel(tr("Last Restake: --"), this);
    m_nextRestakeLabel = new QLabel(tr("Next Restake: --"), this);

    m_performanceLayout->addWidget(m_totalStakedLabel);
    m_performanceLayout->addWidget(m_totalRewardsLabel);
    m_performanceLayout->addWidget(m_averageAPYLabel);
    m_performanceLayout->addWidget(m_efficiencyScoreLabel);
    m_performanceLayout->addWidget(m_lastRestakeLabel);
    m_performanceLayout->addWidget(m_nextRestakeLabel);

    // Charts
    m_chartsGroup = new QGroupBox(tr("📊 Performance Charts"), this);
    QVBoxLayout* chartsLayout = new QVBoxLayout(m_chartsGroup);

    m_chartTabs = new QTabWidget(this);
    m_performanceChartView = new QChartView(this);
    m_projectionChartView = new QChartView(this);
    m_performanceChartView->setMinimumHeight(200);
    m_projectionChartView->setMinimumHeight(200);

    m_chartTabs->addTab(m_performanceChartView, tr("Performance History"));
    m_chartTabs->addTab(m_projectionChartView, tr("Future Projections"));

    // Projection Controls
    m_projectionControlsLayout = new QHBoxLayout();
    m_projectionRangeLabel = new QLabel(tr("Projection Range:"), this);
    m_projectionRangeCombo = new QComboBox(this);
    m_projectionRangeCombo->addItem(tr("1 Year"), 1);
    m_projectionRangeCombo->addItem(tr("2 Years"), 2);
    m_projectionRangeCombo->addItem(tr("5 Years"), 5);
    m_projectionRangeCombo->addItem(tr("10 Years"), 10);
    m_projectionRangeCombo->setCurrentIndex(1); // 2 Years

    m_showProjectionsButton = new QPushButton(tr("📈 Show Projections"), this);
    m_exportDataButton = new QPushButton(tr("📤 Export Data"), this);

    m_projectionControlsLayout->addWidget(m_projectionRangeLabel);
    m_projectionControlsLayout->addWidget(m_projectionRangeCombo);
    m_projectionControlsLayout->addWidget(m_showProjectionsButton);
    m_projectionControlsLayout->addWidget(m_exportDataButton);
    m_projectionControlsLayout->addStretch();

    chartsLayout->addWidget(m_chartTabs);
    chartsLayout->addLayout(m_projectionControlsLayout);

    // Settings
    m_settingsGroup = new QGroupBox(tr("⚙️ Settings"), this);
    m_settingsLayout = new QHBoxLayout(m_settingsGroup);

    m_resetSettingsButton = new QPushButton(tr("🔄 Reset to Defaults"), this);
    m_saveSettingsButton = new QPushButton(tr("💾 Save Settings"), this);

    m_settingsLayout->addWidget(m_resetSettingsButton);
    m_settingsLayout->addWidget(m_saveSettingsButton);
    m_settingsLayout->addStretch();

    // Add all sections to main layout
    m_mainLayout->addWidget(m_analysisGroup);
    m_mainLayout->addWidget(m_optimizationGroup);
    m_mainLayout->addWidget(m_recommendationGroup);
    m_mainLayout->addWidget(m_performanceGroup);
    m_mainLayout->addWidget(m_chartsGroup);
    m_mainLayout->addWidget(m_settingsGroup);

    // Apply opacity effect for animations
    m_recommendationGroup->setGraphicsEffect(m_optimizationOpacityEffect);
}

void AutoStakingOptimizer::connectSignals()
{
    // Button connections
    connect(m_analyzeButton, &QPushButton::clicked, this, &AutoStakingOptimizer::onAnalyzeClicked);
    connect(m_optimizeButton, &QPushButton::clicked, this, &AutoStakingOptimizer::onOptimizeClicked);
    connect(m_applyRecommendationButton, &QPushButton::clicked, this, &AutoStakingOptimizer::onApplyRecommendationClicked);
    connect(m_showProjectionsButton, &QPushButton::clicked, this, &AutoStakingOptimizer::onShowProjectionsClicked);
    connect(m_exportDataButton, &QPushButton::clicked, this, &AutoStakingOptimizer::onExportDataClicked);
    connect(m_resetSettingsButton, &QPushButton::clicked, this, &AutoStakingOptimizer::onResetSettingsClicked);
    connect(m_saveSettingsButton, &QPushButton::clicked, this, &AutoStakingOptimizer::saveSettings);

    // Settings connections
    connect(m_autoRestakeCheck, &QCheckBox::toggled, this, &AutoStakingOptimizer::onAutoRestakeToggled);
    connect(m_compoundRewardsCheck, &QCheckBox::toggled, this, &AutoStakingOptimizer::onCompoundRewardsToggled);
    connect(m_notifyRestakeCheck, &QCheckBox::toggled, this, &AutoStakingOptimizer::onNotifyRestakeToggled);
    connect(m_strategyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AutoStakingOptimizer::onStrategyChanged);
    connect(m_frequencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AutoStakingOptimizer::onFrequencyChanged);
    connect(m_minAmountInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AutoStakingOptimizer::onMinAmountChanged);
    connect(m_maxPercentageSlider, &QSlider::valueChanged, this, &AutoStakingOptimizer::onMaxPercentageChanged);
    connect(m_restakeThresholdSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &AutoStakingOptimizer::onRestakeThresholdChanged);

    // Timer connections
    connect(m_updateTimer, &QTimer::timeout, this, &AutoStakingOptimizer::onUpdateTimer);
    connect(m_restakeCheckTimer, &QTimer::timeout, this, &AutoStakingOptimizer::checkRestakeThreshold);

    // Chart connections
    connect(m_projectionRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AutoStakingOptimizer::onProjectionRangeChanged);

    // Wallet model connections (if available)
    if (m_walletModel) {
        // Connect to wallet balance changes
        // Note: This would need to be implemented based on the actual WalletModel interface
    }
}

void AutoStakingOptimizer::applyTheme()
{
    // Apply Shahcoin styling
    QString styleSheet = R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #2c3e50;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #3498db;
        }
        QPushButton {
            background-color: #3498db;
            border: none;
            color: white;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
        QPushButton:pressed {
            background-color: #21618c;
        }
        QPushButton:disabled {
            background-color: #bdc3c7;
            color: #7f8c8d;
        }
        QLabel {
            color: #2c3e50;
        }
        QComboBox, QSpinBox, QDoubleSpinBox {
            border: 2px solid #bdc3c7;
            border-radius: 4px;
            padding: 4px;
            background-color: white;
        }
        QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border-color: #3498db;
        }
        QCheckBox {
            color: #2c3e50;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
        }
        QSlider::groove:horizontal {
            border: 1px solid #bdc3c7;
            height: 8px;
            background: #ecf0f1;
            border-radius: 4px;
        }
        QSlider::handle:horizontal {
            background: #3498db;
            border: 1px solid #2980b9;
            width: 18px;
            margin: -2px 0;
            border-radius: 9px;
        }
        QTabWidget::pane {
            border: 1px solid #bdc3c7;
            border-radius: 4px;
        }
        QTabBar::tab {
            background-color: #ecf0f1;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #3498db;
            color: white;
        }
    )";

    setStyleSheet(styleSheet);
}

void AutoStakingOptimizer::analyzeCurrentStaking()
{
    if (m_optimizationInProgress) return;
    m_optimizationInProgress = true;

    // Update current metrics
    m_currentMetrics = calculateCurrentMetrics();
    updatePerformanceDisplay();

    // Update analysis labels
    if (m_walletModel) {
        // TODO: Get actual wallet data from WalletModel
        double balance = 1000.0; // Mock data
        double staked = 500.0;   // Mock data
        double rewards = 25.0;   // Mock data

        m_currentBalanceLabel->setText(tr("Balance: %1 SHAH").arg(balance, 0, 'f', 2));
        m_currentStakedLabel->setText(tr("Staked: %1 SHAH").arg(staked, 0, 'f', 2));
        m_currentRewardsLabel->setText(tr("Rewards: %1 SHAH").arg(rewards, 0, 'f', 2));
    } else {
        m_currentBalanceLabel->setText(tr("Balance: No wallet available"));
        m_currentStakedLabel->setText(tr("Staked: No wallet available"));
        m_currentRewardsLabel->setText(tr("Rewards: No wallet available"));
    }

    m_lastAnalysis = QDateTime::currentDateTime();
    m_optimizationInProgress = false;
}

void AutoStakingOptimizer::generateRecommendations()
{
    if (m_optimizationInProgress) return;
    m_optimizationInProgress = true;

    // Generate optimal recommendation
    m_currentRecommendation = generateOptimalRecommendation();
    updateRecommendationDisplay();

    // Enable apply button
    m_applyRecommendationButton->setEnabled(true);

    // Emit signal
    emit optimizationCompleted(m_currentRecommendation);

    m_optimizationInProgress = false;
}

AutoStakingOptimizer::StakingRecommendation AutoStakingOptimizer::analyzeStakingPerformance()
{
    StakingRecommendation recommendation;
    
    // TODO: Analyze actual staking performance from wallet data
    // For now, use mock data
    recommendation.currentStaked = 500.0;
    recommendation.availableBalance = 1000.0;
    recommendation.recommendedAmount = 0.0;
    recommendation.potentialReward = 0.0;
    recommendation.compoundEffect = 0.0;
    recommendation.optimalFrequency = Monthly;
    recommendation.strategy = "Balanced";
    recommendation.reasoning = "Analysis pending...";
    recommendation.autoRestake = false;
    recommendation.nextRestake = QDateTime::currentDateTime().addDays(30);

    return recommendation;
}

AutoStakingOptimizer::StakingRecommendation AutoStakingOptimizer::generateOptimalRecommendation()
{
    StakingRecommendation recommendation = analyzeStakingPerformance();
    
    // Calculate optimal amount based on strategy
    double balance = recommendation.availableBalance + recommendation.currentStaked;
    recommendation.recommendedAmount = calculateOptimalStakeAmount(balance, m_settings.strategy);
    
    // Calculate potential rewards
    int days = 365; // 1 year projection
    recommendation.potentialReward = calculatePotentialReward(recommendation.recommendedAmount, days);
    
    // Calculate compound effect
    double apy = 5.0; // Mock APY
    recommendation.compoundEffect = calculateCompoundEffect(recommendation.recommendedAmount, apy / 100.0, 12);
    
    // Determine optimal frequency
    recommendation.optimalFrequency = determineOptimalFrequency(recommendation.recommendedAmount, m_settings.strategy);
    
    // Generate strategy reasoning
    recommendation.strategy = m_strategyCombo->currentText();
    recommendation.reasoning = generateStrategyReasoning(recommendation);
    
    // Set auto-restake based on settings
    recommendation.autoRestake = m_settings.autoRestake;
    recommendation.nextRestake = QDateTime::currentDateTime().addDays(30);
    
    return recommendation;
}

double AutoStakingOptimizer::calculateOptimalStakeAmount(double balance, OptimizationStrategy strategy)
{
    double percentage = 0.0;
    
    switch (strategy) {
        case Conservative:
            percentage = 0.3; // 30% of balance
            break;
        case Balanced:
            percentage = 0.6; // 60% of balance
            break;
        case Aggressive:
            percentage = 0.85; // 85% of balance
            break;
        case Custom:
            percentage = m_settings.maxStakePercentage / 100.0;
            break;
    }
    
    double recommended = balance * percentage;
    
    // Ensure minimum amount
    if (recommended < m_settings.minStakeAmount) {
        recommended = m_settings.minStakeAmount;
    }
    
    // Ensure maximum percentage
    double maxAmount = balance * (m_settings.maxStakePercentage / 100.0);
    if (recommended > maxAmount) {
        recommended = maxAmount;
    }
    
    return recommended;
}

double AutoStakingOptimizer::calculatePotentialReward(double amount, int days)
{
    // Mock calculation - would use actual staking APY
    double apy = 5.0; // 5% APY
    return amount * (apy / 100.0) * (days / 365.0);
}

double AutoStakingOptimizer::calculateCompoundEffect(double principal, double rate, int periods)
{
    return principal * pow(1 + rate, periods) - principal;
}

int AutoStakingOptimizer::determineOptimalFrequency(double amount, OptimizationStrategy strategy)
{
    switch (strategy) {
        case Conservative:
            return Monthly;
        case Balanced:
            return Weekly;
        case Aggressive:
            return Daily;
        case Custom:
            return m_settings.frequency;
    }
    return Monthly;
}

QString AutoStakingOptimizer::generateStrategyReasoning(const StakingRecommendation& recommendation)
{
    QString reasoning = tr("Based on your %1 strategy: ").arg(recommendation.strategy);
    
    if (recommendation.recommendedAmount > recommendation.currentStaked) {
        reasoning += tr("Increase staking by %1 SHAH to maximize rewards. ").arg(recommendation.recommendedAmount - recommendation.currentStaked, 0, 'f', 2);
    } else if (recommendation.recommendedAmount < recommendation.currentStaked) {
        reasoning += tr("Consider reducing staking by %1 SHAH for better balance. ").arg(recommendation.currentStaked - recommendation.recommendedAmount, 0, 'f', 2);
    } else {
        reasoning += tr("Current staking amount is optimal. ");
    }
    
    reasoning += tr("Expected annual reward: %1 SHAH with compound effect of %2 SHAH.").arg(recommendation.potentialReward, 0, 'f', 2).arg(recommendation.compoundEffect, 0, 'f', 2);
    
    return reasoning;
}

AutoStakingOptimizer::PerformanceMetrics AutoStakingOptimizer::calculateCurrentMetrics()
{
    PerformanceMetrics metrics;
    
    // TODO: Calculate actual metrics from wallet data
    // For now, use mock data
    metrics.totalStaked = 500.0;
    metrics.totalRewards = 25.0;
    metrics.averageAPY = 5.0;
    metrics.compoundGrowth = 2.5;
    metrics.daysStaking = 180;
    metrics.efficiencyScore = 85.0;
    metrics.lastRestake = QDateTime::currentDateTime().addDays(-7);
    metrics.nextScheduledRestake = QDateTime::currentDateTime().addDays(23);
    
    return metrics;
}

void AutoStakingOptimizer::updateRecommendationDisplay()
{
    if (m_currentRecommendation.recommendedAmount > 0) {
        m_recommendedAmountLabel->setText(tr("Recommended Amount: %1 SHAH").arg(m_currentRecommendation.recommendedAmount, 0, 'f', 2));
        m_potentialRewardLabel->setText(tr("Potential Reward: %1 SHAH/year").arg(m_currentRecommendation.potentialReward, 0, 'f', 2));
        m_compoundEffectLabel->setText(tr("Compound Effect: %1 SHAH/year").arg(m_currentRecommendation.compoundEffect, 0, 'f', 2));
        m_strategyReasoningLabel->setText(tr("Strategy: %1").arg(m_currentRecommendation.reasoning));
    } else {
        m_recommendedAmountLabel->setText(tr("Recommended Amount: Waiting for analysis..."));
        m_potentialRewardLabel->setText(tr("Potential Reward: --"));
        m_compoundEffectLabel->setText(tr("Compound Effect: --"));
        m_strategyReasoningLabel->setText(tr("Strategy: --"));
    }
}

void AutoStakingOptimizer::updatePerformanceDisplay()
{
    m_totalStakedLabel->setText(tr("Total Staked: %1 SHAH").arg(m_currentMetrics.totalStaked, 0, 'f', 2));
    m_totalRewardsLabel->setText(tr("Total Rewards: %1 SHAH").arg(m_currentMetrics.totalRewards, 0, 'f', 2));
    m_averageAPYLabel->setText(tr("Avg APY: %1%").arg(m_currentMetrics.averageAPY, 0, 'f', 2));
    m_efficiencyScoreLabel->setText(tr("Efficiency: %1%").arg(m_currentMetrics.efficiencyScore, 0, 'f', 1));
    m_lastRestakeLabel->setText(tr("Last Restake: %1").arg(m_currentMetrics.lastRestake.toString("MMM dd, yyyy")));
    m_nextRestakeLabel->setText(tr("Next Restake: %1").arg(m_currentMetrics.nextScheduledRestake.toString("MMM dd, yyyy")));
}

void AutoStakingOptimizer::updatePerformanceChart()
{
    if (!m_performanceChart) {
        m_performanceChart = new QChart();
        m_performanceSeries = new QLineSeries();
        m_performanceChart->addSeries(m_performanceSeries);
        m_performanceChart->setTitle(tr("Staking Performance History"));
        m_performanceChart->createDefaultAxes();
        m_performanceChart->axes(Qt::Horizontal).first()->setTitleText(tr("Date"));
        m_performanceChart->axes(Qt::Vertical).first()->setTitleText(tr("Total Value (SHAH)"));
        m_performanceChartView->setChart(m_performanceChart);
    }

    // Clear existing data
    m_performanceSeries->clear();

    // Add historical data points
    for (const auto& point : m_performanceHistory) {
        m_performanceSeries->append(point.first.toMSecsSinceEpoch(), point.second);
    }

    // Update chart
    m_performanceChart->axes(Qt::Horizontal).first()->setRange(
        QDateTime::currentDateTime().addDays(-30).toMSecsSinceEpoch(),
        QDateTime::currentDateTime().toMSecsSinceEpoch()
    );
}

void AutoStakingOptimizer::updateProjectionChart()
{
    if (!m_projectionChart) {
        m_projectionChart = new QChart();
        m_projectionSeries = new QLineSeries();
        m_projectionChart->addSeries(m_projectionSeries);
        m_projectionChart->setTitle(tr("Future Staking Projections"));
        m_projectionChart->createDefaultAxes();
        m_projectionChart->axes(Qt::Horizontal).first()->setTitleText(tr("Date"));
        m_projectionChart->axes(Qt::Vertical).first()->setTitleText(tr("Projected Value (SHAH)"));
        m_projectionChartView->setChart(m_projectionChart);
    }

    // Clear existing data
    m_projectionSeries->clear();

    // Add projection data points
    for (const auto& point : m_projectionData) {
        m_projectionSeries->append(point.first.toMSecsSinceEpoch(), point.second);
    }

    // Update chart
    if (!m_projectionData.isEmpty()) {
        m_projectionChart->axes(Qt::Horizontal).first()->setRange(
            QDateTime::currentDateTime().toMSecsSinceEpoch(),
            m_projectionData.last().first.toMSecsSinceEpoch()
        );
    }
}

void AutoStakingOptimizer::addPerformanceDataPoint(const QDateTime& timestamp, double value)
{
    m_performanceHistory.append(qMakePair(timestamp, value));
    
    // Keep only recent data points
    while (m_performanceHistory.size() > MAX_HISTORY_POINTS) {
        m_performanceHistory.removeFirst();
    }
}

void AutoStakingOptimizer::addProjectionDataPoint(const QDateTime& timestamp, double value)
{
    m_projectionData.append(qMakePair(timestamp, value));
}

void AutoStakingOptimizer::calculateProjections()
{
    // Clear existing projections
    m_projectionData.clear();

    // Get projection range
    int years = m_projectionRangeCombo->currentData().toInt();
    
    // Calculate projections based on current recommendation
    if (m_currentRecommendation.recommendedAmount > 0) {
        double currentValue = m_currentRecommendation.recommendedAmount;
        double apy = m_currentMetrics.averageAPY / 100.0;
        
        QDateTime currentDate = QDateTime::currentDateTime();
        
        for (int month = 0; month <= years * 12; month++) {
            QDateTime projectionDate = currentDate.addMonths(month);
            double projectedValue = currentValue * pow(1 + apy / 12.0, month);
            addProjectionDataPoint(projectionDate, projectedValue);
        }
    }
    
    updateProjectionChart();
}

// Slot implementations
void AutoStakingOptimizer::onAnalyzeClicked()
{
    analyzeCurrentStaking();
    showNotification(tr("Staking analysis completed!"), "success");
}

void AutoStakingOptimizer::onOptimizeClicked()
{
    generateRecommendations();
    showNotification(tr("Optimization recommendations generated!"), "success");
}

void AutoStakingOptimizer::onApplyRecommendationClicked()
{
    if (m_currentRecommendation.recommendedAmount > 0) {
        showOptimizationDialog(m_currentRecommendation);
    } else {
        showNotification(tr("No recommendation available. Please run optimization first."), "warning");
    }
}

void AutoStakingOptimizer::onAutoRestakeToggled(bool enabled)
{
    m_settings.autoRestake = enabled;
    m_autoRestakeEnabled = enabled;
    
    if (enabled) {
        scheduleAutoRestake();
        showNotification(tr("Auto-restake enabled. Next restake scheduled."), "info");
    } else {
        showNotification(tr("Auto-restake disabled."), "info");
    }
}

void AutoStakingOptimizer::onStrategyChanged(int index)
{
    m_settings.strategy = static_cast<OptimizationStrategy>(index);
    
    // Regenerate recommendations if we have current data
    if (m_currentRecommendation.recommendedAmount > 0) {
        generateRecommendations();
    }
}

void AutoStakingOptimizer::onFrequencyChanged(int index)
{
    m_settings.frequency = static_cast<StakingFrequency>(index);
    
    // Regenerate recommendations if we have current data
    if (m_currentRecommendation.recommendedAmount > 0) {
        generateRecommendations();
    }
}

void AutoStakingOptimizer::onMinAmountChanged()
{
    m_settings.minStakeAmount = m_minAmountInput->value();
    
    // Regenerate recommendations if we have current data
    if (m_currentRecommendation.recommendedAmount > 0) {
        generateRecommendations();
    }
}

void AutoStakingOptimizer::onMaxPercentageChanged()
{
    int value = m_maxPercentageSlider->value();
    m_settings.maxStakePercentage = value;
    m_maxPercentageValue->setText(tr("%1%").arg(value));
    
    // Regenerate recommendations if we have current data
    if (m_currentRecommendation.recommendedAmount > 0) {
        generateRecommendations();
    }
}

void AutoStakingOptimizer::onCompoundRewardsToggled(bool enabled)
{
    m_settings.compoundRewards = enabled;
}

void AutoStakingOptimizer::onNotifyRestakeToggled(bool enabled)
{
    m_settings.notifyOnRestake = enabled;
}

void AutoStakingOptimizer::onRestakeThresholdChanged()
{
    m_settings.restakeThreshold = m_restakeThresholdSpin->value();
}

void AutoStakingOptimizer::onShowProjectionsClicked()
{
    calculateProjections();
    showNotification(tr("Projections calculated and displayed!"), "success");
}

void AutoStakingOptimizer::onExportDataClicked()
{
    exportOptimizationData();
}

void AutoStakingOptimizer::onResetSettingsClicked()
{
    resetToDefaults();
    showNotification(tr("Settings reset to defaults!"), "info");
}

void AutoStakingOptimizer::onUpdateTimer()
{
    // Update performance metrics periodically
    updatePerformanceMetrics();
}

void AutoStakingOptimizer::onBalanceChanged()
{
    // Triggered when wallet balance changes
    analyzeCurrentStaking();
}

void AutoStakingOptimizer::onStakingStatusChanged()
{
    // Triggered when staking status changes
    analyzeCurrentStaking();
}

void AutoStakingOptimizer::onProjectionRangeChanged()
{
    // Recalculate projections with new range
    if (!m_projectionData.isEmpty()) {
        calculateProjections();
    }
}

void AutoStakingOptimizer::onChartTypeChanged()
{
    // Update chart display
    updatePerformanceChart();
    updateProjectionChart();
}

void AutoStakingOptimizer::onContextMenuRequested(const QPoint& pos)
{
    // Context menu for charts
    QMenu contextMenu(this);
    QAction* copyAction = contextMenu.addAction(tr("Copy Data"));
    QAction* exportAction = contextMenu.addAction(tr("Export Chart"));
    
    QAction* selectedAction = contextMenu.exec(mapToGlobal(pos));
    
    if (selectedAction == copyAction) {
        // Copy chart data to clipboard
        QApplication::clipboard()->setText(tr("Staking Performance Data"));
    } else if (selectedAction == exportAction) {
        // Export chart as image
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Chart"), 
            QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/staking_chart.png",
            tr("PNG Files (*.png);;All Files (*)"));
        
        if (!fileName.isEmpty()) {
            // TODO: Implement chart export
            showNotification(tr("Chart export not implemented yet."), "info");
        }
    }
}

void AutoStakingOptimizer::updatePerformanceMetrics()
{
    // Update current metrics
    m_currentMetrics = calculateCurrentMetrics();
    updatePerformanceDisplay();
    
    // Add current data point to history
    double totalValue = m_currentMetrics.totalStaked + m_currentMetrics.totalRewards;
    addPerformanceDataPoint(QDateTime::currentDateTime(), totalValue);
    
    // Update performance chart
    updatePerformanceChart();
    
    // Emit signal
    emit performanceUpdated(m_currentMetrics);
}

void AutoStakingOptimizer::applyOptimization(const StakingRecommendation& recommendation)
{
    // TODO: Apply the recommendation to actual staking
    // This would integrate with the wallet's staking functionality
    
    showNotification(tr("Optimization applied successfully!"), "success");
    
    // Update metrics after applying optimization
    analyzeCurrentStaking();
}

void AutoStakingOptimizer::scheduleAutoRestake()
{
    if (!m_settings.autoRestake) return;
    
    // Schedule next restake based on frequency
    QDateTime nextRestake = QDateTime::currentDateTime();
    
    switch (m_settings.frequency) {
        case Daily:
            nextRestake = nextRestake.addDays(1);
            break;
        case Weekly:
            nextRestake = nextRestake.addDays(7);
            break;
        case Monthly:
            nextRestake = nextRestake.addMonths(1);
            break;
        case Quarterly:
            nextRestake = nextRestake.addMonths(3);
            break;
        case Yearly:
            nextRestake = nextRestake.addYears(1);
            break;
    }
    
    m_currentMetrics.nextScheduledRestake = nextRestake;
    updatePerformanceDisplay();
    
    emit autoRestakeScheduled(nextRestake);
}

void AutoStakingOptimizer::scheduleNextRestake(const StakingRecommendation& recommendation)
{
    m_currentMetrics.nextScheduledRestake = recommendation.nextRestake;
    updatePerformanceDisplay();
}

void AutoStakingOptimizer::checkRestakeThreshold()
{
    if (!m_settings.autoRestake) return;
    
    if (shouldAutoRestake()) {
        executeAutoRestake();
    }
}

bool AutoStakingOptimizer::shouldAutoRestake()
{
    // Check if we have enough rewards to restake
    return m_currentMetrics.totalRewards >= m_settings.restakeThreshold;
}

void AutoStakingOptimizer::executeAutoRestake()
{
    // TODO: Execute actual restake transaction
    // This would integrate with the wallet's staking functionality
    
    if (m_settings.notifyOnRestake) {
        showNotification(tr("Auto-restake executed! %1 SHAH restaked.").arg(m_currentMetrics.totalRewards, 0, 'f', 2), "success");
    }
    
    // Update metrics
    m_currentMetrics.lastRestake = QDateTime::currentDateTime();
    m_currentMetrics.totalStaked += m_currentMetrics.totalRewards;
    m_currentMetrics.totalRewards = 0.0;
    
    updatePerformanceDisplay();
    
    // Schedule next restake
    scheduleAutoRestake();
}

void AutoStakingOptimizer::showOptimizationDialog(const StakingRecommendation& recommendation)
{
    OptimizationStrategyDialog dialog(recommendation, this);
    
    connect(&dialog, &OptimizationStrategyDialog::strategyAccepted, this, [this](const StakingRecommendation& rec) {
        applyOptimization(rec);
    });
    
    dialog.exec();
}

void AutoStakingOptimizer::animateOptimizationUpdate()
{
    m_optimizationAnimation->setTargetObject(m_optimizationOpacityEffect);
    m_optimizationAnimation->setPropertyName("opacity");
    m_optimizationAnimation->setDuration(ANIMATION_DURATION_MS);
    m_optimizationAnimation->setStartValue(0.3);
    m_optimizationAnimation->setEndValue(1.0);
    m_optimizationAnimation->start();
}

void AutoStakingOptimizer::showNotification(const QString& message, const QString& type)
{
    // Simple notification - could be enhanced with a proper notification system
    QString title = tr("Auto-Staking Optimizer");
    
    if (type == "success") {
        QMessageBox::information(this, title, message);
    } else if (type == "warning") {
        QMessageBox::warning(this, title, message);
    } else if (type == "error") {
        QMessageBox::critical(this, title, message);
    } else {
        QMessageBox::information(this, title, message);
    }
}

void AutoStakingOptimizer::savePerformanceHistory()
{
    m_settingsStorage->beginGroup("PerformanceHistory");
    
    m_settingsStorage->setValue("Count", m_performanceHistory.size());
    
    for (int i = 0; i < m_performanceHistory.size(); ++i) {
        const auto& point = m_performanceHistory.at(i);
        m_settingsStorage->setValue(QString("Timestamp_%1").arg(i), point.first);
        m_settingsStorage->setValue(QString("Value_%1").arg(i), point.second);
    }
    
    m_settingsStorage->endGroup();
}

void AutoStakingOptimizer::loadPerformanceHistory()
{
    m_settingsStorage->beginGroup("PerformanceHistory");
    
    int count = m_settingsStorage->value("Count", 0).toInt();
    
    for (int i = 0; i < count; ++i) {
        QDateTime timestamp = m_settingsStorage->value(QString("Timestamp_%1").arg(i)).toDateTime();
        double value = m_settingsStorage->value(QString("Value_%1").arg(i)).toDouble();
        
        if (timestamp.isValid()) {
            m_performanceHistory.append(qMakePair(timestamp, value));
        }
    }
    
    m_settingsStorage->endGroup();
}

void AutoStakingOptimizer::exportOptimizationData()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Optimization Data"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/staking_optimization_data.json",
        tr("JSON Files (*.json);;All Files (*)"));
    
    if (fileName.isEmpty()) return;
    
    QJsonObject data;
    
    // Export current recommendation
    QJsonObject recommendation;
    recommendation["recommendedAmount"] = m_currentRecommendation.recommendedAmount;
    recommendation["potentialReward"] = m_currentRecommendation.potentialReward;
    recommendation["compoundEffect"] = m_currentRecommendation.compoundEffect;
    recommendation["strategy"] = m_currentRecommendation.strategy;
    recommendation["reasoning"] = m_currentRecommendation.reasoning;
    data["recommendation"] = recommendation;
    
    // Export performance metrics
    QJsonObject metrics;
    metrics["totalStaked"] = m_currentMetrics.totalStaked;
    metrics["totalRewards"] = m_currentMetrics.totalRewards;
    metrics["averageAPY"] = m_currentMetrics.averageAPY;
    metrics["efficiencyScore"] = m_currentMetrics.efficiencyScore;
    data["metrics"] = metrics;
    
    // Export settings
    QJsonObject settings;
    settings["strategy"] = m_settings.strategy;
    settings["frequency"] = m_settings.frequency;
    settings["minStakeAmount"] = m_settings.minStakeAmount;
    settings["maxStakePercentage"] = m_settings.maxStakePercentage;
    settings["autoRestake"] = m_settings.autoRestake;
    settings["compoundRewards"] = m_settings.compoundRewards;
    data["settings"] = settings;
    
    // Export performance history
    QJsonArray history;
    for (const auto& point : m_performanceHistory) {
        QJsonObject historyPoint;
        historyPoint["timestamp"] = point.first.toString(Qt::ISODate);
        historyPoint["value"] = point.second;
        history.append(historyPoint);
    }
    data["performanceHistory"] = history;
    
    QJsonDocument doc(data);
    QFile file(fileName);
    
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        showNotification(tr("Data exported successfully to %1").arg(fileName), "success");
    } else {
        showNotification(tr("Failed to export data: %1").arg(file.errorString()), "error");
    }
}

void AutoStakingOptimizer::saveSettings()
{
    m_settingsStorage->beginGroup(SETTINGS_GROUP);
    
    m_settingsStorage->setValue(SETTINGS_STRATEGY, m_settings.strategy);
    m_settingsStorage->setValue(SETTINGS_FREQUENCY, m_settings.frequency);
    m_settingsStorage->setValue(SETTINGS_MIN_AMOUNT, m_settings.minStakeAmount);
    m_settingsStorage->setValue(SETTINGS_MAX_PERCENTAGE, m_settings.maxStakePercentage);
    m_settingsStorage->setValue(SETTINGS_AUTO_RESTAKE, m_settings.autoRestake);
    m_settingsStorage->setValue(SETTINGS_COMPOUND_REWARDS, m_settings.compoundRewards);
    m_settingsStorage->setValue(SETTINGS_NOTIFY_RESTAKE, m_settings.notifyOnRestake);
    m_settingsStorage->setValue(SETTINGS_RESTAKE_THRESHOLD, m_settings.restakeThreshold);
    
    m_settingsStorage->endGroup();
    
    emit settingsChanged(m_settings);
}

void AutoStakingOptimizer::loadSettings()
{
    m_settingsStorage->beginGroup(SETTINGS_GROUP);
    
    m_settings.strategy = static_cast<OptimizationStrategy>(m_settingsStorage->value(SETTINGS_STRATEGY, Balanced).toInt());
    m_settings.frequency = static_cast<StakingFrequency>(m_settingsStorage->value(SETTINGS_FREQUENCY, Monthly).toInt());
    m_settings.minStakeAmount = m_settingsStorage->value(SETTINGS_MIN_AMOUNT, 1.0).toDouble();
    m_settings.maxStakePercentage = m_settingsStorage->value(SETTINGS_MAX_PERCENTAGE, 80.0).toDouble();
    m_settings.autoRestake = m_settingsStorage->value(SETTINGS_AUTO_RESTAKE, false).toBool();
    m_settings.compoundRewards = m_settingsStorage->value(SETTINGS_COMPOUND_REWARDS, true).toBool();
    m_settings.notifyOnRestake = m_settingsStorage->value(SETTINGS_NOTIFY_RESTAKE, true).toBool();
    m_settings.restakeThreshold = m_settingsStorage->value(SETTINGS_RESTAKE_THRESHOLD, 10).toInt();
    
    m_settingsStorage->endGroup();
    
    // Apply loaded settings to UI
    m_strategyCombo->setCurrentIndex(m_settings.strategy);
    m_frequencyCombo->setCurrentIndex(m_settings.frequency);
    m_minAmountInput->setValue(m_settings.minStakeAmount);
    m_maxPercentageSlider->setValue(static_cast<int>(m_settings.maxStakePercentage));
    m_maxPercentageValue->setText(tr("%1%").arg(static_cast<int>(m_settings.maxStakePercentage)));
    m_autoRestakeCheck->setChecked(m_settings.autoRestake);
    m_compoundRewardsCheck->setChecked(m_settings.compoundRewards);
    m_notifyRestakeCheck->setChecked(m_settings.notifyOnRestake);
    m_restakeThresholdSpin->setValue(m_settings.restakeThreshold);
}

void AutoStakingOptimizer::resetToDefaults()
{
    // Reset settings to defaults
    m_settings.strategy = Balanced;
    m_settings.frequency = Monthly;
    m_settings.minStakeAmount = 1.0;
    m_settings.maxStakePercentage = 80.0;
    m_settings.autoRestake = false;
    m_settings.compoundRewards = true;
    m_settings.notifyOnRestake = true;
    m_settings.restakeThreshold = 10;
    
    // Update UI
    m_strategyCombo->setCurrentIndex(m_settings.strategy);
    m_frequencyCombo->setCurrentIndex(m_settings.frequency);
    m_minAmountInput->setValue(m_settings.minStakeAmount);
    m_maxPercentageSlider->setValue(static_cast<int>(m_settings.maxStakePercentage));
    m_maxPercentageValue->setText(tr("%1%").arg(static_cast<int>(m_settings.maxStakePercentage)));
    m_autoRestakeCheck->setChecked(m_settings.autoRestake);
    m_compoundRewardsCheck->setChecked(m_settings.compoundRewards);
    m_notifyRestakeCheck->setChecked(m_settings.notifyOnRestake);
    m_restakeThresholdSpin->setValue(m_settings.restakeThreshold);
    
    // Save settings
    saveSettings();
}

// Supporting class implementations

PerformanceHistoryTable::PerformanceHistoryTable(QWidget* parent)
    : QTableWidget(parent)
{
    setColumnCount(4);
    setHorizontalHeaderLabels({tr("Date"), tr("Staked"), tr("Rewards"), tr("APY")});
    horizontalHeader()->setStretchLastSection(true);
    setMaximumHeight(150);
}

void PerformanceHistoryTable::addPerformanceRecord(const QDateTime& timestamp, double staked, double rewards, double apy)
{
    int row = rowCount();
    insertRow(row);
    
    setItem(row, 0, new QTableWidgetItem(timestamp.toString("MMM dd, yyyy")));
    setItem(row, 1, new QTableWidgetItem(QString::number(staked, 'f', 2)));
    setItem(row, 2, new QTableWidgetItem(QString::number(rewards, 'f', 2)));
    setItem(row, 3, new QTableWidgetItem(QString::number(apy, 'f', 2) + "%"));
}

void PerformanceHistoryTable::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu contextMenu(this);
    QAction* copyAction = contextMenu.addAction(tr("Copy Value"));
    QAction* copyTimestampAction = contextMenu.addAction(tr("Copy Timestamp"));
    QAction* exportAction = contextMenu.addAction(tr("Export Selected"));
    
    QAction* selectedAction = contextMenu.exec(event->globalPos());
    
    if (selectedAction == copyAction) {
        QTableWidgetItem* item = itemAt(event->pos());
        if (item) {
            QApplication::clipboard()->setText(item->text());
        }
    } else if (selectedAction == copyTimestampAction) {
        QTableWidgetItem* item = itemAt(event->pos());
        if (item && item->column() == 0) {
            QApplication::clipboard()->setText(item->text());
        }
    } else if (selectedAction == exportAction) {
        // TODO: Implement export functionality
    }
}

OptimizationStrategyDialog::OptimizationStrategyDialog(const AutoStakingOptimizer::StakingRecommendation& recommendation, QWidget* parent)
    : QDialog(parent)
    , m_recommendation(recommendation)
{
    setWindowTitle(tr("Apply Optimization Strategy"));
    setModal(true);
    setupUI();
    applyTheme();
}

void OptimizationStrategyDialog::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Recommendation summary
    QGroupBox* summaryGroup = new QGroupBox(tr("Optimization Summary"), this);
    QVBoxLayout* summaryLayout = new QVBoxLayout(summaryGroup);
    
    QLabel* amountLabel = new QLabel(tr("Recommended Amount: %1 SHAH").arg(m_recommendation.recommendedAmount, 0, 'f', 2), this);
    QLabel* rewardLabel = new QLabel(tr("Potential Annual Reward: %1 SHAH").arg(m_recommendation.potentialReward, 0, 'f', 2), this);
    QLabel* compoundLabel = new QLabel(tr("Compound Effect: %1 SHAH/year").arg(m_recommendation.compoundEffect, 0, 'f', 2), this);
    QLabel* strategyLabel = new QLabel(tr("Strategy: %1").arg(m_recommendation.strategy), this);
    QLabel* reasoningLabel = new QLabel(tr("Reasoning: %1").arg(m_recommendation.reasoning), this);
    reasoningLabel->setWordWrap(true);
    
    summaryLayout->addWidget(amountLabel);
    summaryLayout->addWidget(rewardLabel);
    summaryLayout->addWidget(compoundLabel);
    summaryLayout->addWidget(strategyLabel);
    summaryLayout->addWidget(reasoningLabel);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* acceptButton = new QPushButton(tr("✅ Apply Strategy"), this);
    QPushButton* rejectButton = new QPushButton(tr("❌ Cancel"), this);
    
    connect(acceptButton, &QPushButton::clicked, this, &OptimizationStrategyDialog::onAcceptStrategy);
    connect(rejectButton, &QPushButton::clicked, this, &OptimizationStrategyDialog::onRejectStrategy);
    
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(rejectButton);
    
    layout->addWidget(summaryGroup);
    layout->addLayout(buttonLayout);
}

void OptimizationStrategyDialog::applyTheme()
{
    QString styleSheet = R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #2c3e50;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #3498db;
        }
        QPushButton {
            background-color: #3498db;
            border: none;
            color: white;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
        QLabel {
            color: #2c3e50;
        }
    )";
    
    setStyleSheet(styleSheet);
}

void OptimizationStrategyDialog::onAcceptStrategy()
{
    emit strategyAccepted(m_recommendation);
    accept();
}

void OptimizationStrategyDialog::onRejectStrategy()
{
    emit strategyRejected();
    reject();
}

ProjectionCalculator::ProjectionCalculator(QWidget* parent)
    : QWidget(parent)
    , m_projectionChartView(new QChartView(this))
    , m_projectionChart(new QChart())
    , m_projectionSeries(new QLineSeries())
{
    setupUI();
}

void ProjectionCalculator::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    m_projectionChart->addSeries(m_projectionSeries);
    m_projectionChart->setTitle(tr("Staking Projections"));
    m_projectionChart->createDefaultAxes();
    m_projectionChart->axes(Qt::Horizontal).first()->setTitleText(tr("Date"));
    m_projectionChart->axes(Qt::Vertical).first()->setTitleText(tr("Projected Value (SHAH)"));
    
    m_projectionChartView->setChart(m_projectionChart);
    m_projectionChartView->setMinimumHeight(200);
    
    layout->addWidget(m_projectionChartView);
}

void ProjectionCalculator::calculateProjection(double initialAmount, double apy, int years)
{
    m_projectionData.clear();
    m_projectionSeries->clear();
    
    QDateTime currentDate = QDateTime::currentDateTime();
    
    for (int month = 0; month <= years * 12; month++) {
        QDateTime projectionDate = currentDate.addMonths(month);
        double projectedValue = initialAmount * pow(1 + apy / 12.0, month);
        m_projectionData.append(qMakePair(projectionDate, projectedValue));
        m_projectionSeries->append(projectionDate.toMSecsSinceEpoch(), projectedValue);
    }
    
    updateProjectionChart();
    emit projectionCalculated(m_projectionData);
}

void ProjectionCalculator::updateProjectionChart()
{
    if (!m_projectionData.isEmpty()) {
        m_projectionChart->axes(Qt::Horizontal).first()->setRange(
            QDateTime::currentDateTime().toMSecsSinceEpoch(),
            m_projectionData.last().first.toMSecsSinceEpoch()
        );
    }
}

            m_projectionData.last().first.toMSecsSinceEpoch()