#include "aifeeestimator.h"
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

// Constants
const QString AIFeeEstimator::SETTINGS_GROUP = "AIFeeEstimator";
const QString AIFeeEstimator::SETTINGS_AUTO_REFRESH = "AutoRefreshEnabled";
const QString AIFeeEstimator::SETTINGS_REFRESH_INTERVAL = "RefreshIntervalMs";
const QString AIFeeEstimator::SETTINGS_SHOW_NOTIFICATIONS = "ShowNotifications";
const QString AIFeeEstimator::SETTINGS_OPTIMIZE_AUTO = "OptimizeAutomatically";

AIFeeEstimator::AIFeeEstimator(WalletModel* walletModel, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(walletModel)
    , m_platformStyle(nullptr)
    , m_mainLayout(nullptr)
    , m_feeGroup(nullptr)
    , m_feeInputLayout(nullptr)
    , m_amountLabel(nullptr)
    , m_amountInput(nullptr)
    , m_priorityLabel(nullptr)
    , m_priorityCombo(nullptr)
    , m_refreshButton(nullptr)
    , m_optimizeButton(nullptr)
    , m_feeDisplayGroup(nullptr)
    , m_feeDisplayLayout(nullptr)
    , m_currentFeeLabel(nullptr)
    , m_recommendedFeeLabel(nullptr)
    , m_savingsLabel(nullptr)
    , m_applyFeeButton(nullptr)
    , m_networkGroup(nullptr)
    , m_networkLayout(nullptr)
    , m_networkStatusIcon(nullptr)
    , m_networkStatusLabel(nullptr)
    , m_pendingTxLabel(nullptr)
    , m_avgConfirmationLabel(nullptr)
    , m_networkLoadBar(nullptr)
    , m_chartsGroup(nullptr)
    , m_chartTabs(nullptr)
    , m_feeChartView(nullptr)
    , m_networkChartView(nullptr)
    , m_feeChart(nullptr)
    , m_networkChart(nullptr)
    , m_feeSeries(nullptr)
    , m_networkSeries(nullptr)
    , m_historyGroup(nullptr)
    , m_feeHistoryTable(nullptr)
    , m_exportHistoryButton(nullptr)
    , m_advancedGroup(nullptr)
    , m_autoRefreshCheck(nullptr)
    , m_refreshIntervalSpin(nullptr)
    , m_showNotificationsCheck(nullptr)
    , m_optimizeAutomaticallyCheck(nullptr)
    , m_lastAmount(0.0)
    , m_lastPriority(Normal)
    , m_autoRefreshEnabled(false)
    , m_optimizationInProgress(false)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_updateTimer(new QTimer(this))
    , m_animationTimer(new QTimer(this))
    , m_settings(new QSettings("Shahcoin", "AIFeeEstimator"))
    , m_feeAnimation(new QPropertyAnimation(this))
    , m_feeOpacityEffect(new QGraphicsOpacityEffect(this))
{
    setWindowTitle(tr("💸 AI Fee Estimator"));
    setupUI();
    connectSignals();
    applyTheme();
    loadSettings();
    setupCharts();

    // Initialize network stats
    m_networkStats.condition = NormalCongestion;
    m_networkStats.avgFeePerKB = 0.001;
    m_networkStats.avgConfirmationTime = 10;
    m_networkStats.pendingTransactions = 0;
    m_networkStats.networkLoad = 50.0;
    m_networkStats.lastUpdate = QDateTime::currentDateTime();

    // Start network monitoring
    startNetworkMonitoring();
}

AIFeeEstimator::~AIFeeEstimator()
{
    stopNetworkMonitoring();
    saveSettings();
}

void AIFeeEstimator::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Fee Estimation Section
    m_feeGroup = new QGroupBox(tr("💸 Fee Estimation"), this);
    m_feeInputLayout = new QHBoxLayout(m_feeGroup);

    m_amountLabel = new QLabel(tr("Amount (SHAH):"), this);
    m_amountInput = new QDoubleSpinBox(this);
    m_amountInput->setRange(0.0001, 1000000.0);
    m_amountInput->setDecimals(8);
    m_amountInput->setValue(1.0);
    m_amountInput->setSuffix(" SHAH");

    m_priorityLabel = new QLabel(tr("Priority:"), this);
    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->addItem(tr("Low"), Low);
    m_priorityCombo->addItem(tr("Normal"), Normal);
    m_priorityCombo->addItem(tr("High"), High);
    m_priorityCombo->addItem(tr("Urgent"), Urgent);
    m_priorityCombo->setCurrentIndex(1); // Normal

    m_refreshButton = new QPushButton(tr("🔄 Refresh"), this);
    m_optimizeButton = new QPushButton(tr("🤖 AI Optimize"), this);

    m_feeInputLayout->addWidget(m_amountLabel);
    m_feeInputLayout->addWidget(m_amountInput);
    m_feeInputLayout->addWidget(m_priorityLabel);
    m_feeInputLayout->addWidget(m_priorityCombo);
    m_feeInputLayout->addWidget(m_refreshButton);
    m_feeInputLayout->addWidget(m_optimizeButton);
    m_feeInputLayout->addStretch();

    // Fee Display Section
    m_feeDisplayGroup = new QGroupBox(tr("📊 Fee Analysis"), this);
    m_feeDisplayLayout = new QVBoxLayout(m_feeDisplayGroup);

    m_currentFeeLabel = new QLabel(tr("Current Fee: Calculating..."), this);
    m_recommendedFeeLabel = new QLabel(tr("AI Recommendation: Waiting for analysis..."), this);
    m_savingsLabel = new QLabel(tr("Potential Savings: --"), this);
    m_applyFeeButton = new QPushButton(tr("✅ Apply Recommended Fee"), this);
    m_applyFeeButton->setEnabled(false);

    m_feeDisplayLayout->addWidget(m_currentFeeLabel);
    m_feeDisplayLayout->addWidget(m_recommendedFeeLabel);
    m_feeDisplayLayout->addWidget(m_savingsLabel);
    m_feeDisplayLayout->addWidget(m_applyFeeButton);

    // Network Status Section
    m_networkGroup = new QGroupBox(tr("🌐 Network Status"), this);
    m_networkLayout = new QHBoxLayout(m_networkGroup);

    m_networkStatusIcon = new QLabel(this);
    m_networkStatusIcon->setFixedSize(24, 24);
    m_networkStatusIcon->setPixmap(QPixmap(":/icons/synced.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_networkStatusLabel = new QLabel(tr("Network: Normal"), this);
    m_pendingTxLabel = new QLabel(tr("Pending: 0"), this);
    m_avgConfirmationLabel = new QLabel(tr("Avg Confirm: 10 min"), this);
    m_networkLoadBar = new QProgressBar(this);
    m_networkLoadBar->setRange(0, 100);
    m_networkLoadBar->setValue(50);
    m_networkLoadBar->setFormat(tr("Load: %p%"));

    m_networkLayout->addWidget(m_networkStatusIcon);
    m_networkLayout->addWidget(m_networkStatusLabel);
    m_networkLayout->addWidget(m_pendingTxLabel);
    m_networkLayout->addWidget(m_avgConfirmationLabel);
    m_networkLayout->addWidget(m_networkLoadBar);
    m_networkLayout->addStretch();

    // Charts Section
    m_chartsGroup = new QGroupBox(tr("📈 Fee Trends"), this);
    QVBoxLayout* chartsLayout = new QVBoxLayout(m_chartsGroup);

    m_chartTabs = new QTabWidget(this);
    m_feeChartView = new QChartView(this);
    m_networkChartView = new QChartView(this);
    m_feeChartView->setMinimumHeight(200);
    m_networkChartView->setMinimumHeight(200);

    m_chartTabs->addTab(m_feeChartView, tr("Fee History"));
    m_chartTabs->addTab(m_networkChartView, tr("Network Load"));

    chartsLayout->addWidget(m_chartTabs);

    // Fee History Table
    m_historyGroup = new QGroupBox(tr("📋 Fee History"), this);
    QVBoxLayout* historyLayout = new QVBoxLayout(m_historyGroup);

    m_feeHistoryTable = new QTableWidget(this);
    m_feeHistoryTable->setColumnCount(4);
    m_feeHistoryTable->setHorizontalHeaderLabels({tr("Time"), tr("Fee"), tr("Priority"), tr("Applied")});
    m_feeHistoryTable->horizontalHeader()->setStretchLastSection(true);
    m_feeHistoryTable->setMaximumHeight(150);

    m_exportHistoryButton = new QPushButton(tr("📤 Export History"), this);

    historyLayout->addWidget(m_feeHistoryTable);
    historyLayout->addWidget(m_exportHistoryButton);

    // Advanced Options
    m_advancedGroup = new QGroupBox(tr("⚙️ Advanced Options"), this);
    QVBoxLayout* advancedLayout = new QVBoxLayout(m_advancedGroup);

    m_autoRefreshCheck = new QCheckBox(tr("Auto-refresh every 30 seconds"), this);
    m_refreshIntervalSpin = new QSpinBox(this);
    m_refreshIntervalSpin->setRange(10, 300);
    m_refreshIntervalSpin->setValue(30);
    m_refreshIntervalSpin->setSuffix(tr(" seconds"));

    m_showNotificationsCheck = new QCheckBox(tr("Show optimization notifications"), this);
    m_optimizeAutomaticallyCheck = new QCheckBox(tr("Auto-optimize when significant savings detected"), this);

    QHBoxLayout* refreshLayout = new QHBoxLayout();
    refreshLayout->addWidget(m_autoRefreshCheck);
    refreshLayout->addWidget(m_refreshIntervalSpin);
    refreshLayout->addStretch();

    advancedLayout->addLayout(refreshLayout);
    advancedLayout->addWidget(m_showNotificationsCheck);
    advancedLayout->addWidget(m_optimizeAutomaticallyCheck);

    // Add all sections to main layout
    m_mainLayout->addWidget(m_feeGroup);
    m_mainLayout->addWidget(m_feeDisplayGroup);
    m_mainLayout->addWidget(m_networkGroup);
    m_mainLayout->addWidget(m_chartsGroup);
    m_mainLayout->addWidget(m_historyGroup);
    m_mainLayout->addWidget(m_advancedGroup);

    // Apply opacity effect for animations
    m_feeDisplayGroup->setGraphicsEffect(m_feeOpacityEffect);
}

void AIFeeEstimator::connectSignals()
{
    connect(m_refreshButton, &QPushButton::clicked, this, &AIFeeEstimator::onRefreshClicked);
    connect(m_optimizeButton, &QPushButton::clicked, this, &AIFeeEstimator::onOptimizeClicked);
    connect(m_applyFeeButton, &QPushButton::clicked, this, &AIFeeEstimator::onApplyFeeClicked);
    connect(m_exportHistoryButton, &QPushButton::clicked, this, &AIFeeEstimator::onExportHistoryClicked);

    connect(m_amountInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AIFeeEstimator::onAmountChanged);
    connect(m_priorityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AIFeeEstimator::onPriorityChanged);

    connect(m_autoRefreshCheck, &QCheckBox::toggled, this, &AIFeeEstimator::onAutoRefreshToggled);
    connect(m_refreshIntervalSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (m_autoRefreshEnabled) {
            m_updateTimer->setInterval(value * 1000);
        }
    });

    connect(m_updateTimer, &QTimer::timeout, this, &AIFeeEstimator::onUpdateTimer);
    connect(m_animationTimer, &QTimer::timeout, this, &AIFeeEstimator::animateFeeUpdate);

    connect(m_feeHistoryTable, &QTableWidget::itemClicked, this, &AIFeeEstimator::onFeeHistoryItemClicked);
    connect(m_feeHistoryTable, &QTableWidget::customContextMenuRequested, this, &AIFeeEstimator::onContextMenuRequested);

    // Network manager
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &AIFeeEstimator::onNetworkReplyFinished);
}

void AIFeeEstimator::applyTheme()
{
    // Apply Shahcoin styling
    setStyleSheet(R"(
        QWidget {
            background-color: #f8f9fa;
            color: #212529;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #dee2e6;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #495057;
        }
        QPushButton {
            background-color: #007bff;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QPushButton:pressed {
            background-color: #004085;
        }
        QPushButton:disabled {
            background-color: #6c757d;
            color: #adb5bd;
        }
        QComboBox {
            border: 1px solid #ced4da;
            border-radius: 4px;
            padding: 4px 8px;
            background-color: white;
        }
        QSpinBox, QDoubleSpinBox {
            border: 1px solid #ced4da;
            border-radius: 4px;
            padding: 4px 8px;
            background-color: white;
        }
        QProgressBar {
            border: 1px solid #ced4da;
            border-radius: 4px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #28a745;
            border-radius: 3px;
        }
        QTableWidget {
            border: 1px solid #ced4da;
            border-radius: 4px;
            background-color: white;
            gridline-color: #dee2e6;
        }
        QTableWidget::item {
            padding: 4px;
        }
        QTableWidget::item:selected {
            background-color: #007bff;
            color: white;
        }
        QTabWidget::pane {
            border: 1px solid #ced4da;
            border-radius: 4px;
        }
        QTabBar::tab {
            background-color: #e9ecef;
            border: 1px solid #ced4da;
            padding: 8px 16px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: white;
            border-bottom: 2px solid #007bff;
        }
    )");
}

void AIFeeEstimator::setupCharts()
{
    // Setup fee chart
    m_feeChart = new QChart();
    m_feeSeries = new QLineSeries();
    m_feeSeries->setName(tr("Fee per KB"));
    
    m_feeChart->addSeries(m_feeSeries);
    m_feeChart->setTitle(tr("Fee History"));
    m_feeChart->createDefaultAxes();
    
    QDateTimeAxis* feeAxisX = new QDateTimeAxis;
    feeAxisX->setTickCount(10);
    feeAxisX->setFormat("hh:mm");
    feeAxisX->setTitleText(tr("Time"));
    m_feeChart->setAxisX(feeAxisX, m_feeSeries);
    
    QValueAxis* feeAxisY = new QValueAxis;
    feeAxisY->setLabelFormat("%.6f");
    feeAxisY->setTitleText(tr("Fee (SHAH/KB)"));
    m_feeChart->setAxisY(feeAxisY, m_feeSeries);
    
    m_feeChartView->setChart(m_feeChart);
    m_feeChartView->setRenderHint(QPainter::Antialiasing);

    // Setup network chart
    m_networkChart = new QChart();
    m_networkSeries = new QLineSeries();
    m_networkSeries->setName(tr("Network Load"));
    
    m_networkChart->addSeries(m_networkSeries);
    m_networkChart->setTitle(tr("Network Load"));
    m_networkChart->createDefaultAxes();
    
    QDateTimeAxis* networkAxisX = new QDateTimeAxis;
    networkAxisX->setTickCount(10);
    networkAxisX->setFormat("hh:mm");
    networkAxisX->setTitleText(tr("Time"));
    m_networkChart->setAxisX(networkAxisX, m_networkSeries);
    
    QValueAxis* networkAxisY = new QValueAxis;
    networkAxisY->setRange(0, 100);
    networkAxisY->setLabelFormat("%.0f%%");
    networkAxisY->setTitleText(tr("Load (%)"));
    m_networkChart->setAxisY(networkAxisY, m_networkSeries);
    
    m_networkChartView->setChart(m_networkChart);
    m_networkChartView->setRenderHint(QPainter::Antialiasing);
}

void AIFeeEstimator::refreshFeeEstimates()
{
    double amount = m_amountInput->value();
    FeePriority priority = static_cast<FeePriority>(m_priorityCombo->currentData().toInt());
    
    m_currentEstimates = calculateFeeEstimates(amount);
    updateFeeDisplay();
    
    // Get AI recommendation
    m_aiRecommendation = generateAIRecommendation(amount, priority);
    
    // Add to history
    if (!m_currentEstimates.isEmpty()) {
        FeeEstimate current = m_currentEstimates.first();
        addFeeDataPoint(current.feePerKB, QDateTime::currentDateTime());
        
        // Add to history table
        int row = m_feeHistoryTable->rowCount();
        m_feeHistoryTable->insertRow(row);
        m_feeHistoryTable->setItem(row, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
        m_feeHistoryTable->setItem(row, 1, new QTableWidgetItem(QString::number(current.feePerKB, 'f', 6)));
        m_feeHistoryTable->setItem(row, 2, new QTableWidgetItem(m_priorityCombo->currentText()));
        m_feeHistoryTable->setItem(row, 3, new QTableWidgetItem(""));
    }
    
    emit feeEstimateUpdated(m_currentEstimates);
}

QList<AIFeeEstimator::FeeEstimate> AIFeeEstimator::calculateFeeEstimates(double amount)
{
    QList<FeeEstimate> estimates;
    
    // Base fee calculation based on network conditions
    double baseFee = m_networkStats.avgFeePerKB;
    double networkMultiplier = 1.0;
    
    switch (m_networkStats.condition) {
        case LowCongestion:
            networkMultiplier = 0.8;
            break;
        case NormalCongestion:
            networkMultiplier = 1.0;
            break;
        case HighCongestion:
            networkMultiplier = 1.5;
            break;
        case ExtremeCongestion:
            networkMultiplier = 2.5;
            break;
    }
    
    // Calculate fees for different priorities
    for (int i = 0; i <= Urgent; ++i) {
        FeeEstimate estimate;
        estimate.priority = static_cast<FeePriority>(i);
        
        // Fee multipliers based on priority
        double priorityMultiplier = 1.0;
        switch (estimate.priority) {
            case Low:
                priorityMultiplier = 0.7;
                estimate.confirmationTime = 30;
                estimate.description = tr("Low priority - may take longer to confirm");
                break;
            case Normal:
                priorityMultiplier = 1.0;
                estimate.confirmationTime = 10;
                estimate.description = tr("Standard priority - normal confirmation time");
                break;
            case High:
                priorityMultiplier = 1.3;
                estimate.confirmationTime = 5;
                estimate.description = tr("High priority - faster confirmation");
                break;
            case Urgent:
                priorityMultiplier = 2.0;
                estimate.confirmationTime = 2;
                estimate.description = tr("Urgent - fastest confirmation");
                break;
        }
        
        estimate.feePerKB = baseFee * networkMultiplier * priorityMultiplier;
        estimate.totalFee = estimate.feePerKB * (amount / 1000.0); // Simplified calculation
        estimate.recommended = (estimate.priority == Normal);
        
        estimates.append(estimate);
    }
    
    return estimates;
}

AIFeeEstimator::FeeEstimate AIFeeEstimator::generateAIRecommendation(double amount, FeePriority urgency)
{
    FeeEstimate recommendation;
    
    // AI logic for fee optimization
    if (urgency == Urgent) {
        // For urgent transactions, recommend high fee
        recommendation = calculateFeeEstimates(amount).last();
        recommendation.recommended = true;
        recommendation.description = tr("AI: Urgent transaction - using high fee for fastest confirmation");
    } else if (m_networkStats.condition == LowCongestion) {
        // Low congestion - can use lower fees
        recommendation = calculateFeeEstimates(amount).first();
        recommendation.recommended = true;
        recommendation.description = tr("AI: Low network congestion - using lower fee to save costs");
    } else if (m_networkStats.condition == ExtremeCongestion) {
        // High congestion - recommend higher fees
        recommendation = calculateFeeEstimates(amount).at(2); // High priority
        recommendation.recommended = true;
        recommendation.description = tr("AI: High network congestion - using higher fee to ensure confirmation");
    } else {
        // Normal conditions - use standard fee
        recommendation = calculateFeeEstimates(amount).at(1); // Normal priority
        recommendation.recommended = true;
        recommendation.description = tr("AI: Normal network conditions - using standard fee");
    }
    
    return recommendation;
}

void AIFeeEstimator::updateFeeDisplay()
{
    if (m_currentEstimates.isEmpty()) {
        m_currentFeeLabel->setText(tr("Current Fee: No estimates available"));
        m_recommendedFeeLabel->setText(tr("AI Recommendation: Waiting for analysis..."));
        m_savingsLabel->setText(tr("Potential Savings: --"));
        m_applyFeeButton->setEnabled(false);
        return;
    }
    
    // Get current fee (based on selected priority)
    FeePriority currentPriority = static_cast<FeePriority>(m_priorityCombo->currentData().toInt());
    FeeEstimate currentFee;
    
    for (const auto& estimate : m_currentEstimates) {
        if (estimate.priority == currentPriority) {
            currentFee = estimate;
            break;
        }
    }
    
    // Update current fee display
    m_currentFeeLabel->setText(tr("Current Fee: %1 SHAH/KB (%2 min)")
                              .arg(currentFee.feePerKB, 0, 'f', 6)
                              .arg(currentFee.confirmationTime));
    
    // Update AI recommendation
    if (m_aiRecommendation.recommended) {
        m_recommendedFeeLabel->setText(tr("AI Recommendation: %1 SHAH/KB (%2 min) - %3")
                                     .arg(m_aiRecommendation.feePerKB, 0, 'f', 6)
                                     .arg(m_aiRecommendation.confirmationTime)
                                     .arg(m_aiRecommendation.description));
        
        // Calculate potential savings
        double savings = calculateFeeSavings(currentFee, m_aiRecommendation);
        if (savings > 0) {
            m_savingsLabel->setText(tr("Potential Savings: %1 SHAH").arg(savings, 0, 'f', 6));
            m_savingsLabel->setStyleSheet("color: #28a745; font-weight: bold;");
        } else if (savings < 0) {
            m_savingsLabel->setText(tr("Additional Cost: %1 SHAH").arg(-savings, 0, 'f', 6));
            m_savingsLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
        } else {
            m_savingsLabel->setText(tr("No savings available"));
            m_savingsLabel->setStyleSheet("");
        }
        
        m_applyFeeButton->setEnabled(true);
    } else {
        m_recommendedFeeLabel->setText(tr("AI Recommendation: Analysis in progress..."));
        m_savingsLabel->setText(tr("Potential Savings: --"));
        m_applyFeeButton->setEnabled(false);
    }
    
    // Animate the update
    animateFeeUpdate();
}

double AIFeeEstimator::calculateFeeSavings(const FeeEstimate& current, const FeeEstimate& optimized)
{
    double currentTotal = current.totalFee;
    double optimizedTotal = optimized.totalFee;
    return currentTotal - optimizedTotal;
}

void AIFeeEstimator::startNetworkMonitoring()
{
    m_updateTimer->setInterval(UPDATE_INTERVAL_MS);
    m_updateTimer->start();
    
    // Initial update
    updateNetworkStats();
}

void AIFeeEstimator::stopNetworkMonitoring()
{
    m_updateTimer->stop();
}

void AIFeeEstimator::updateNetworkStats()
{
    fetchNetworkData();
}

void AIFeeEstimator::fetchNetworkData()
{
    // In a real implementation, this would fetch data from a network API
    // For now, we'll simulate network data with some randomness
    
    // Simulate network response
    QJsonObject networkData;
    networkData["avgFeePerKB"] = 0.001 + (QRandomGenerator::global()->bounded(100) / 1000000.0);
    networkData["pendingTransactions"] = QRandomGenerator::global()->bounded(1000, 5000);
    networkData["avgConfirmationTime"] = QRandomGenerator::global()->bounded(5, 30);
    networkData["networkLoad"] = QRandomGenerator::global()->bounded(20, 90);
    
    QJsonDocument doc(networkData);
    parseNetworkResponse(doc.toJson());
}

void AIFeeEstimator::parseNetworkResponse(const QByteArray& data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    
    m_networkStats.avgFeePerKB = obj["avgFeePerKB"].toDouble();
    m_networkStats.pendingTransactions = obj["pendingTransactions"].toInt();
    m_networkStats.avgConfirmationTime = obj["avgConfirmationTime"].toInt();
    m_networkStats.networkLoad = obj["networkLoad"].toDouble();
    m_networkStats.lastUpdate = QDateTime::currentDateTime();
    
    // Determine network condition
    m_networkStats.condition = determineNetworkCondition(m_networkStats.avgFeePerKB, m_networkStats.pendingTransactions);
    
    // Update UI
    updateNetworkDisplay();
    updateNetworkIndicators();
    
    // Add data points to charts
    addFeeDataPoint(m_networkStats.avgFeePerKB, m_networkStats.lastUpdate);
    addNetworkDataPoint(m_networkStats.networkLoad, m_networkStats.lastUpdate);
    
    emit networkStatsUpdated(m_networkStats);
}

AIFeeEstimator::NetworkCondition AIFeeEstimator::determineNetworkCondition(double avgFee, int pendingTx)
{
    if (avgFee > 0.002 && pendingTx > 4000) {
        return ExtremeCongestion;
    } else if (avgFee > 0.0015 && pendingTx > 3000) {
        return HighCongestion;
    } else if (avgFee < 0.0008 && pendingTx < 2000) {
        return LowCongestion;
    } else {
        return NormalCongestion;
    }
}

void AIFeeEstimator::updateNetworkDisplay()
{
    QString conditionText;
    QString iconPath;
    QColor loadColor;
    
    switch (m_networkStats.condition) {
        case LowCongestion:
            conditionText = tr("Low Congestion");
            iconPath = ":/icons/synced.png";
            loadColor = QColor("#28a745");
            break;
        case NormalCongestion:
            conditionText = tr("Normal");
            iconPath = ":/icons/synced.png";
            loadColor = QColor("#17a2b8");
            break;
        case HighCongestion:
            conditionText = tr("High Congestion");
            iconPath = ":/icons/warning.png";
            loadColor = QColor("#ffc107");
            break;
        case ExtremeCongestion:
            conditionText = tr("Extreme Congestion");
            iconPath = ":/icons/warning.png";
            loadColor = QColor("#dc3545");
            break;
    }
    
    m_networkStatusLabel->setText(tr("Network: %1").arg(conditionText));
    m_networkStatusIcon->setPixmap(QPixmap(iconPath).scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    m_pendingTxLabel->setText(tr("Pending: %1").arg(m_networkStats.pendingTransactions));
    m_avgConfirmationLabel->setText(tr("Avg Confirm: %1 min").arg(m_networkStats.avgConfirmationTime));
    
    m_networkLoadBar->setValue(static_cast<int>(m_networkStats.networkLoad));
    
    // Update progress bar color based on load
    QString styleSheet = QString("QProgressBar::chunk { background-color: %1; }").arg(loadColor.name());
    m_networkLoadBar->setStyleSheet(styleSheet);
}

void AIFeeEstimator::updateNetworkIndicators()
{
    // Update chart data
    updateFeeChart();
    updateNetworkLoadChart();
}

void AIFeeEstimator::addFeeDataPoint(double fee, const QDateTime& timestamp)
{
    m_feeHistory.append({timestamp, fee});
    
    // Keep only recent data points
    if (m_feeHistory.size() > MAX_HISTORY_POINTS) {
        m_feeHistory.removeFirst();
    }
    
    // Add to chart
    m_feeSeries->append(timestamp.toMSecsSinceEpoch(), fee);
    
    // Remove old points from chart
    if (m_feeSeries->count() > MAX_HISTORY_POINTS) {
        QList<QPointF> points = m_feeSeries->points();
        points.removeFirst();
        m_feeSeries->replace(points);
    }
}

void AIFeeEstimator::addNetworkDataPoint(double load, const QDateTime& timestamp)
{
    m_networkHistory.append({timestamp, load});
    
    // Keep only recent data points
    if (m_networkHistory.size() > MAX_HISTORY_POINTS) {
        m_networkHistory.removeFirst();
    }
    
    // Add to chart
    m_networkSeries->append(timestamp.toMSecsSinceEpoch(), load);
    
    // Remove old points from chart
    if (m_networkSeries->count() > MAX_HISTORY_POINTS) {
        QList<QPointF> points = m_networkSeries->points();
        points.removeFirst();
        m_networkSeries->replace(points);
    }
}

void AIFeeEstimator::updateFeeChart()
{
    // Chart is updated automatically when data points are added
    // This method can be used for additional chart customization
}

void AIFeeEstimator::updateNetworkLoadChart()
{
    // Chart is updated automatically when data points are added
    // This method can be used for additional chart customization
}

void AIFeeEstimator::animateFeeUpdate()
{
    // Create a brief animation to highlight the fee update
    m_feeAnimation->setTargetObject(m_feeOpacityEffect);
    m_feeAnimation->setPropertyName("opacity");
    m_feeAnimation->setDuration(ANIMATION_DURATION_MS);
    m_feeAnimation->setStartValue(0.5);
    m_feeAnimation->setEndValue(1.0);
    m_feeAnimation->start();
}

// Public slots
void AIFeeEstimator::onRefreshClicked()
{
    refreshFeeEstimates();
    updateNetworkStats();
}

void AIFeeEstimator::onAmountChanged()
{
    double amount = m_amountInput->value();
    if (amount != m_lastAmount) {
        m_lastAmount = amount;
        refreshFeeEstimates();
    }
}

void AIFeeEstimator::onPriorityChanged(int index)
{
    FeePriority priority = static_cast<FeePriority>(m_priorityCombo->itemData(index).toInt());
    if (priority != m_lastPriority) {
        m_lastPriority = priority;
        updateFeeDisplay();
    }
}

void AIFeeEstimator::onApplyFeeClicked()
{
    if (m_aiRecommendation.recommended) {
        emit feeApplied(m_aiRecommendation.feePerKB, m_aiRecommendation.priority);
        
        // Update history table
        int row = m_feeHistoryTable->rowCount() - 1;
        if (row >= 0) {
            m_feeHistoryTable->setItem(row, 3, new QTableWidgetItem("✅ Applied"));
        }
        
        showNotification(tr("Fee applied successfully!"), "success");
    }
}

void AIFeeEstimator::onOptimizeClicked()
{
    if (m_optimizationInProgress) {
        showNotification(tr("Optimization already in progress..."), "warning");
        return;
    }
    
    m_optimizationInProgress = true;
    m_optimizeButton->setEnabled(false);
    m_optimizeButton->setText(tr("🤖 Analyzing..."));
    
    // Simulate AI analysis
    QTimer::singleShot(2000, this, [this]() {
        double amount = m_amountInput->value();
        FeePriority priority = static_cast<FeePriority>(m_priorityCombo->currentData().toInt());
        
        FeeEstimate current = m_currentEstimates.isEmpty() ? FeeEstimate{} : m_currentEstimates.first();
        FeeEstimate optimized = generateAIRecommendation(amount, priority);
        
        if (shouldRecommendOptimization(current, optimized)) {
            showOptimizationDialog(optimized);
        } else {
            showNotification(tr("No significant optimization found"), "info");
        }
        
        m_optimizationInProgress = false;
        m_optimizeButton->setEnabled(true);
        m_optimizeButton->setText(tr("🤖 AI Optimize"));
    });
}

void AIFeeEstimator::onExportHistoryClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export Fee History"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/fee_history.csv",
        tr("CSV Files (*.csv);;All Files (*.*)"));
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Export Error"), tr("Could not open file for writing"));
        return;
    }
    
    QTextStream out(&file);
    out << "Time,Fee,Priority,Applied\n";
    
    for (int row = 0; row < m_feeHistoryTable->rowCount(); ++row) {
        QString time = m_feeHistoryTable->item(row, 0)->text();
        QString fee = m_feeHistoryTable->item(row, 1)->text();
        QString priority = m_feeHistoryTable->item(row, 2)->text();
        QString applied = m_feeHistoryTable->item(row, 3)->text();
        
        out << QString("%1,%2,%3,%4\n").arg(time, fee, priority, applied);
    }
    
    file.close();
    showNotification(tr("Fee history exported successfully!"), "success");
}

void AIFeeEstimator::onAutoRefreshToggled(bool enabled)
{
    m_autoRefreshEnabled = enabled;
    if (enabled) {
        m_updateTimer->setInterval(m_refreshIntervalSpin->value() * 1000);
        m_updateTimer->start();
    } else {
        m_updateTimer->stop();
    }
}

void AIFeeEstimator::onUpdateTimer()
{
    updateNetworkStats();
    if (m_autoRefreshEnabled) {
        refreshFeeEstimates();
    }
}

void AIFeeEstimator::onNetworkReplyFinished()
{
    // Handle network reply if needed
}

void AIFeeEstimator::onFeeHistoryItemClicked(QTableWidgetItem* item)
{
    // Handle fee history item clicks
}

void AIFeeEstimator::onContextMenuRequested(const QPoint& pos)
{
    // Handle context menu for fee history table
}

void AIFeeEstimator::showOptimizationDialog(const FeeEstimate& recommendation)
{
    FeeEstimate current = m_currentEstimates.isEmpty() ? FeeEstimate{} : m_currentEstimates.first();
    FeeOptimizationDialog dialog(current, recommendation, this);
    
    connect(&dialog, &FeeOptimizationDialog::optimizationAccepted, this, [this](const FeeEstimate& fee) {
        emit feeApplied(fee.feePerKB, fee.priority);
        emit optimizationCompleted(calculateFeeSavings(m_currentEstimates.first(), fee));
        showNotification(tr("Optimization applied!"), "success");
    });
    
    dialog.exec();
}

void AIFeeEstimator::showNotification(const QString& message, const QString& type)
{
    if (m_showNotificationsCheck->isChecked()) {
        QString title = tr("Fee Estimator");
        QMessageBox::Icon icon = QMessageBox::Information;
        
        if (type == "success") {
            icon = QMessageBox::Information;
        } else if (type == "warning") {
            icon = QMessageBox::Warning;
        } else if (type == "error") {
            icon = QMessageBox::Critical;
        }
        
        QMessageBox msgBox(icon, title, message, QMessageBox::Ok, this);
        msgBox.exec();
    }
}

bool AIFeeEstimator::shouldRecommendOptimization(const FeeEstimate& current, const FeeEstimate& optimized)
{
    double savings = calculateFeeSavings(current, optimized);
    return qAbs(savings) > 0.0001; // Significant savings threshold
}

void AIFeeEstimator::saveSettings()
{
    m_settings->setValue(SETTINGS_AUTO_REFRESH, m_autoRefreshEnabled);
    m_settings->setValue(SETTINGS_REFRESH_INTERVAL, m_refreshIntervalSpin->value() * 1000);
    m_settings->setValue(SETTINGS_SHOW_NOTIFICATIONS, m_showNotificationsCheck->isChecked());
    m_settings->setValue(SETTINGS_OPTIMIZE_AUTO, m_optimizeAutomaticallyCheck->isChecked());
}

void AIFeeEstimator::loadSettings()
{
    m_autoRefreshEnabled = m_settings->value(SETTINGS_AUTO_REFRESH, false).toBool();
    m_autoRefreshCheck->setChecked(m_autoRefreshEnabled);
    
    int interval = m_settings->value(SETTINGS_REFRESH_INTERVAL, 30000).toInt();
    m_refreshIntervalSpin->setValue(interval / 1000);
    
    m_showNotificationsCheck->setChecked(m_settings->value(SETTINGS_SHOW_NOTIFICATIONS, true).toBool());
    m_optimizeAutomaticallyCheck->setChecked(m_settings->value(SETTINGS_OPTIMIZE_AUTO, false).toBool());
}

// FeeHistoryTable implementation
FeeHistoryTable::FeeHistoryTable(QWidget* parent)
    : QTableWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void FeeHistoryTable::addFeeRecord(const QDateTime& timestamp, double fee, AIFeeEstimator::FeePriority priority, bool applied)
{
    int row = rowCount();
    insertRow(row);
    
    setItem(row, 0, new QTableWidgetItem(timestamp.toString("hh:mm:ss")));
    setItem(row, 1, new QTableWidgetItem(QString::number(fee, 'f', 6)));
    
    QString priorityText;
    switch (priority) {
        case AIFeeEstimator::Low: priorityText = tr("Low"); break;
        case AIFeeEstimator::Normal: priorityText = tr("Normal"); break;
        case AIFeeEstimator::High: priorityText = tr("High"); break;
        case AIFeeEstimator::Urgent: priorityText = tr("Urgent"); break;
    }
    setItem(row, 2, new QTableWidgetItem(priorityText));
    setItem(row, 3, new QTableWidgetItem(applied ? "✅" : ""));
}

void FeeHistoryTable::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    
    QAction* copyFee = menu.addAction(tr("Copy Fee"));
    QAction* copyTimestamp = menu.addAction(tr("Copy Timestamp"));
    menu.addSeparator();
    QAction* exportSelected = menu.addAction(tr("Export Selected"));
    
    connect(copyFee, &QAction::triggered, this, &FeeHistoryTable::onCopyFee);
    connect(copyTimestamp, &QAction::triggered, this, &FeeHistoryTable::onCopyTimestamp);
    connect(exportSelected, &QAction::triggered, this, &FeeHistoryTable::onExportSelected);
    
    menu.exec(event->globalPos());
}

void FeeHistoryTable::onCopyFee()
{
    QTableWidgetItem* item = currentItem();
    if (item && item->column() == 1) {
        QApplication::clipboard()->setText(item->text());
    }
}

void FeeHistoryTable::onCopyTimestamp()
{
    QTableWidgetItem* item = currentItem();
    if (item && item->column() == 0) {
        QApplication::clipboard()->setText(item->text());
    }
}

void FeeHistoryTable::onExportSelected()
{
    // Implementation for exporting selected rows
}

// NetworkStatusIndicator implementation
NetworkStatusIndicator::NetworkStatusIndicator(QWidget* parent)
    : QWidget(parent)
    , m_condition(AIFeeEstimator::NormalCongestion)
    , m_statusColor(Qt::blue)
{
    setFixedSize(16, 16);
}

void NetworkStatusIndicator::updateStatus(AIFeeEstimator::NetworkCondition condition, const QString& description)
{
    m_condition = condition;
    m_description = description;
    
    switch (condition) {
        case AIFeeEstimator::LowCongestion:
            m_statusColor = QColor("#28a745");
            break;
        case AIFeeEstimator::NormalCongestion:
            m_statusColor = QColor("#17a2b8");
            break;
        case AIFeeEstimator::HighCongestion:
            m_statusColor = QColor("#ffc107");
            break;
        case AIFeeEstimator::ExtremeCongestion:
            m_statusColor = QColor("#dc3545");
            break;
    }
    
    update();
}

void NetworkStatusIndicator::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    painter.setBrush(m_statusColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(rect().adjusted(2, 2, -2, -2));
}

// FeeOptimizationDialog implementation
FeeOptimizationDialog::FeeOptimizationDialog(const AIFeeEstimator::FeeEstimate& current,
                                           const AIFeeEstimator::FeeEstimate& optimized,
                                           QWidget* parent)
    : QDialog(parent)
    , m_currentFee(current)
    , m_optimizedFee(optimized)
    , m_savings(0.0)
{
    setWindowTitle(tr("🤖 Fee Optimization"));
    setModal(true);
    resize(500, 300);
    
    m_savings = m_currentFee.totalFee - m_optimizedFee.totalFee;
    
    setupUI();
    applyTheme();
}

void FeeOptimizationDialog::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    QLabel* titleLabel = new QLabel(tr("AI Fee Optimization Recommendation"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    
    QGroupBox* comparisonGroup = new QGroupBox(tr("Fee Comparison"), this);
    QVBoxLayout* comparisonLayout = new QVBoxLayout(comparisonGroup);
    
    QLabel* currentLabel = new QLabel(tr("Current Fee: %1 SHAH/KB (%2 min)")
                                    .arg(m_currentFee.feePerKB, 0, 'f', 6)
                                    .arg(m_currentFee.confirmationTime), this);
    
    QLabel* optimizedLabel = new QLabel(tr("Optimized Fee: %1 SHAH/KB (%2 min)")
                                      .arg(m_optimizedFee.feePerKB, 0, 'f', 6)
                                      .arg(m_optimizedFee.confirmationTime), this);
    
    QString savingsText;
    if (m_savings > 0) {
        savingsText = tr("Potential Savings: %1 SHAH").arg(m_savings, 0, 'f', 6);
        optimizedLabel->setStyleSheet("color: #28a745; font-weight: bold;");
    } else {
        savingsText = tr("Additional Cost: %1 SHAH").arg(-m_savings, 0, 'f', 6);
        optimizedLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
    }
    
    QLabel* savingsLabel = new QLabel(savingsText, this);
    savingsLabel->setAlignment(Qt::AlignCenter);
    
    comparisonLayout->addWidget(currentLabel);
    comparisonLayout->addWidget(optimizedLabel);
    comparisonLayout->addWidget(savingsLabel);
    
    QLabel* reasonLabel = new QLabel(m_optimizedFee.description, this);
    reasonLabel->setWordWrap(true);
    reasonLabel->setAlignment(Qt::AlignCenter);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* acceptButton = new QPushButton(tr("✅ Apply Optimization"), this);
    QPushButton* rejectButton = new QPushButton(tr("❌ Keep Current"), this);
    
    connect(acceptButton, &QPushButton::clicked, this, &FeeOptimizationDialog::onAcceptOptimization);
    connect(rejectButton, &QPushButton::clicked, this, &FeeOptimizationDialog::onRejectOptimization);
    
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(rejectButton);
    
    layout->addWidget(titleLabel);
    layout->addWidget(comparisonGroup);
    layout->addWidget(reasonLabel);
    layout->addLayout(buttonLayout);
}

void FeeOptimizationDialog::applyTheme()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #f8f9fa;
            color: #212529;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #dee2e6;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #495057;
        }
        QPushButton {
            background-color: #007bff;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QPushButton:pressed {
            background-color: #004085;
        }
    )");
}

void FeeOptimizationDialog::onAcceptOptimization()
{
    emit optimizationAccepted(m_optimizedFee);
    accept();
}

void FeeOptimizationDialog::onRejectOptimization()
{
    emit optimizationRejected();
    reject();
}
