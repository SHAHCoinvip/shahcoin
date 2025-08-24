#include "portfoliotracker.h"
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
#include <QDateTimeEdit>
#include <QCalendarWidget>

// Constants
const QString PortfolioTracker::SETTINGS_GROUP = "PortfolioTracker";
const QString PortfolioTracker::SETTINGS_TIME_RANGE = "TimeRange";
const QString PortfolioTracker::SETTINGS_CHART_TYPE = "ChartType";
const QString PortfolioTracker::SETTINGS_AUTO_REFRESH = "AutoRefreshEnabled";
const QString PortfolioTracker::SETTINGS_INCLUDE_STAKING = "IncludeStaking";
const QString PortfolioTracker::SETTINGS_INCLUDE_TOKENS = "IncludeTokens";
const QString PortfolioTracker::SETTINGS_INCLUDE_NFTS = "IncludeNFTs";

PortfolioTracker::PortfolioTracker(WalletModel* walletModel, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(walletModel)
    , m_platformStyle(nullptr)
    , m_mainLayout(nullptr)
    , m_overviewGroup(nullptr)
    , m_overviewLayout(nullptr)
    , m_totalValueLabel(nullptr)
    , m_changeLabel(nullptr)
    , m_changePercentLabel(nullptr)
    , m_refreshButton(nullptr)
    , m_controlsGroup(nullptr)
    , m_controlsLayout(nullptr)
    , m_timeRangeLabel(nullptr)
    , m_timeRangeCombo(nullptr)
    , m_chartTypeLabel(nullptr)
    , m_chartTypeCombo(nullptr)
    , m_includeStakingCheck(nullptr)
    , m_includeTokensCheck(nullptr)
    , m_includeNFTsCheck(nullptr)
    , m_autoRefreshCheck(nullptr)
    , m_metricsGroup(nullptr)
    , m_metricsLayout(nullptr)
    , m_totalReturnLabel(nullptr)
    , m_averageReturnLabel(nullptr)
    , m_bestDayLabel(nullptr)
    , m_worstDayLabel(nullptr)
    , m_volatilityLabel(nullptr)
    , m_sharpeRatioLabel(nullptr)
    , m_chartsGroup(nullptr)
    , m_chartsLayout(nullptr)
    , m_chartView(nullptr)
    , m_chart(nullptr)
    , m_balanceSeries(nullptr)
    , m_rewardsSeries(nullptr)
    , m_tokenSeries(nullptr)
    , m_valueAxis(nullptr)
    , m_timeAxis(nullptr)
    , m_historyGroup(nullptr)
    , m_portfolioTable(nullptr)
    , m_exportButton(nullptr)
    , m_currentTimeRange(Month)
    , m_currentChartType(BalanceChart)
    , m_autoRefreshEnabled(false)
    , m_includeStaking(true)
    , m_includeTokens(true)
    , m_includeNFTs(true)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_updateTimer(new QTimer(this))
    , m_animationTimer(new QTimer(this))
    , m_settings(new QSettings("Shahcoin", "PortfolioTracker"))
    , m_valueAnimation(new QPropertyAnimation(this))
    , m_valueOpacityEffect(new QGraphicsOpacityEffect(this))
{
    setWindowTitle(tr("📈 Portfolio Performance Tracker"));
    setupUI();
    connectSignals();
    applyTheme();
    loadSettings();
    setupCharts();

    // Initialize performance metrics
    m_currentMetrics.totalReturn = 0.0;
    m_currentMetrics.averageReturn = 0.0;
    m_currentMetrics.bestDay = 0.0;
    m_currentMetrics.worstDay = 0.0;
    m_currentMetrics.volatility = 0.0;
    m_currentMetrics.sharpeRatio = 0.0;
    m_currentMetrics.daysTracked = 0;

    // Start portfolio tracking
    refreshPortfolioData();
    if (m_autoRefreshEnabled) {
        m_updateTimer->setInterval(UPDATE_INTERVAL_MS);
        m_updateTimer->start();
    }
}

PortfolioTracker::~PortfolioTracker()
{
    saveSettings();
    savePortfolioHistory();
}

void PortfolioTracker::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Portfolio Overview Section
    m_overviewGroup = new QGroupBox(tr("📊 Portfolio Overview"), this);
    m_overviewLayout = new QHBoxLayout(m_overviewGroup);

    m_totalValueLabel = new QLabel(tr("Total Value: Loading..."), this);
    m_totalValueLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2E8B57;");
    
    m_changeLabel = new QLabel(tr("Change: --"), this);
    m_changePercentLabel = new QLabel(tr("(--%)"), this);
    
    m_refreshButton = new QPushButton(tr("🔄 Refresh"), this);

    m_overviewLayout->addWidget(m_totalValueLabel);
    m_overviewLayout->addWidget(m_changeLabel);
    m_overviewLayout->addWidget(m_changePercentLabel);
    m_overviewLayout->addWidget(m_refreshButton);
    m_overviewLayout->addStretch();

    // Controls Section
    m_controlsGroup = new QGroupBox(tr("⚙️ Portfolio Controls"), this);
    m_controlsLayout = new QHBoxLayout(m_controlsGroup);

    m_timeRangeLabel = new QLabel(tr("Time Range:"), this);
    m_timeRangeCombo = new QComboBox(this);
    m_timeRangeCombo->addItem(tr("1 Day"), Day);
    m_timeRangeCombo->addItem(tr("1 Week"), Week);
    m_timeRangeCombo->addItem(tr("1 Month"), Month);
    m_timeRangeCombo->addItem(tr("3 Months"), Quarter);
    m_timeRangeCombo->addItem(tr("1 Year"), Year);
    m_timeRangeCombo->addItem(tr("Custom"), Custom);
    m_timeRangeCombo->setCurrentIndex(2); // 1 Month

    m_chartTypeLabel = new QLabel(tr("Chart Type:"), this);
    m_chartTypeCombo = new QComboBox(this);
    m_chartTypeCombo->addItem(tr("Balance"), BalanceChart);
    m_chartTypeCombo->addItem(tr("Performance"), PerformanceChart);
    m_chartTypeCombo->addItem(tr("Rewards"), RewardsChart);
    m_chartTypeCombo->addItem(tr("Tokens"), TokenChart);

    m_includeStakingCheck = new QCheckBox(tr("Include Staking"), this);
    m_includeStakingCheck->setChecked(true);
    m_includeTokensCheck = new QCheckBox(tr("Include Tokens"), this);
    m_includeTokensCheck->setChecked(true);
    m_includeNFTsCheck = new QCheckBox(tr("Include NFTs"), this);
    m_includeNFTsCheck->setChecked(true);
    m_autoRefreshCheck = new QCheckBox(tr("Auto-refresh"), this);

    m_controlsLayout->addWidget(m_timeRangeLabel);
    m_controlsLayout->addWidget(m_timeRangeCombo);
    m_controlsLayout->addWidget(m_chartTypeLabel);
    m_controlsLayout->addWidget(m_chartTypeCombo);
    m_controlsLayout->addWidget(m_includeStakingCheck);
    m_controlsLayout->addWidget(m_includeTokensCheck);
    m_controlsLayout->addWidget(m_includeNFTsCheck);
    m_controlsLayout->addWidget(m_autoRefreshCheck);
    m_controlsLayout->addStretch();

    // Performance Metrics Section
    m_metricsGroup = new QGroupBox(tr("📈 Performance Metrics"), this);
    m_metricsLayout = new QHBoxLayout(m_metricsGroup);

    m_totalReturnLabel = new QLabel(tr("Total Return: --"), this);
    m_averageReturnLabel = new QLabel(tr("Avg Return: --"), this);
    m_bestDayLabel = new QLabel(tr("Best Day: --"), this);
    m_worstDayLabel = new QLabel(tr("Worst Day: --"), this);
    m_volatilityLabel = new QLabel(tr("Volatility: --"), this);
    m_sharpeRatioLabel = new QLabel(tr("Sharpe Ratio: --"), this);

    m_metricsLayout->addWidget(m_totalReturnLabel);
    m_metricsLayout->addWidget(m_averageReturnLabel);
    m_metricsLayout->addWidget(m_bestDayLabel);
    m_metricsLayout->addWidget(m_worstDayLabel);
    m_metricsLayout->addWidget(m_volatilityLabel);
    m_metricsLayout->addWidget(m_sharpeRatioLabel);

    // Charts Section
    m_chartsGroup = new QGroupBox(tr("📊 Portfolio Charts"), this);
    m_chartsLayout = new QVBoxLayout(m_chartsGroup);

    m_chartView = new QChartView(this);
    m_chartView->setMinimumHeight(300);
    m_chartsLayout->addWidget(m_chartView);

    // Portfolio History Table
    m_historyGroup = new QGroupBox(tr("📋 Portfolio History"), this);
    QVBoxLayout* historyLayout = new QVBoxLayout(m_historyGroup);

    m_portfolioTable = new QTableWidget(this);
    m_portfolioTable->setColumnCount(4);
    m_portfolioTable->setHorizontalHeaderLabels({tr("Date"), tr("Total Value"), tr("Change"), tr("Change %")});
    m_portfolioTable->horizontalHeader()->setStretchLastSection(true);
    m_portfolioTable->setMaximumHeight(200);

    m_exportButton = new QPushButton(tr("📤 Export Data"), this);

    historyLayout->addWidget(m_portfolioTable);
    historyLayout->addWidget(m_exportButton);

    // Add all sections to main layout
    m_mainLayout->addWidget(m_overviewGroup);
    m_mainLayout->addWidget(m_controlsGroup);
    m_mainLayout->addWidget(m_metricsGroup);
    m_mainLayout->addWidget(m_chartsGroup);
    m_mainLayout->addWidget(m_historyGroup);

    // Apply opacity effect for animations
    m_overviewGroup->setGraphicsEffect(m_valueOpacityEffect);
}

void PortfolioTracker::connectSignals()
{
    connect(m_refreshButton, &QPushButton::clicked, this, &PortfolioTracker::onRefreshClicked);
    connect(m_timeRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PortfolioTracker::onTimeRangeChanged);
    connect(m_chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PortfolioTracker::onChartTypeChanged);
    connect(m_exportButton, &QPushButton::clicked, this, &PortfolioTracker::onExportDataClicked);
    connect(m_autoRefreshCheck, &QCheckBox::toggled, this, &PortfolioTracker::onAutoRefreshToggled);
    connect(m_includeStakingCheck, &QCheckBox::toggled, this, &PortfolioTracker::onIncludeStakingToggled);
    connect(m_includeTokensCheck, &QCheckBox::toggled, this, &PortfolioTracker::onIncludeTokensToggled);
    connect(m_includeNFTsCheck, &QCheckBox::toggled, this, &PortfolioTracker::onIncludeNFTsToggled);
    connect(m_updateTimer, &QTimer::timeout, this, &PortfolioTracker::onUpdateTimer);
    connect(m_portfolioTable, &QTableWidget::itemClicked, this, &PortfolioTracker::onPortfolioItemClicked);
    connect(m_portfolioTable, &QTableWidget::customContextMenuRequested, this, &PortfolioTracker::onContextMenuRequested);
}

void PortfolioTracker::applyTheme()
{
    // Apply Shahcoin styling
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #2E8B57;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #2E8B57;
        }
        QPushButton {
            background-color: #2E8B57;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #3CB371;
        }
        QPushButton:pressed {
            background-color: #228B22;
        }
        QComboBox {
            border: 1px solid #2E8B57;
            border-radius: 3px;
            padding: 5px;
        }
        QCheckBox {
            color: #2E8B57;
        }
        QTableWidget {
            gridline-color: #2E8B57;
            selection-background-color: #3CB371;
        }
        QHeaderView::section {
            background-color: #2E8B57;
            color: white;
            padding: 5px;
            border: 1px solid #228B22;
        }
    )");
}

void PortfolioTracker::setupCharts()
{
    m_chart = new QChart();
    m_chart->setTitle(tr("Portfolio Performance"));
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    m_balanceSeries = new QLineSeries();
    m_balanceSeries->setName(tr("Total Balance"));
    m_balanceSeries->setColor(QColor("#2E8B57"));

    m_rewardsSeries = new QLineSeries();
    m_rewardsSeries->setName(tr("Staking Rewards"));
    m_rewardsSeries->setColor(QColor("#FFD700"));

    m_tokenSeries = new QLineSeries();
    m_tokenSeries->setName(tr("Token Value"));
    m_tokenSeries->setColor(QColor("#4169E1"));

    m_chart->addSeries(m_balanceSeries);
    m_chart->addSeries(m_rewardsSeries);
    m_chart->addSeries(m_tokenSeries);

    m_valueAxis = new QValueAxis();
    m_valueAxis->setTitleText(tr("Value (USD)"));
    m_valueAxis->setLabelFormat("%.2f");

    m_timeAxis = new QDateTimeAxis();
    m_timeAxis->setTitleText(tr("Time"));
    m_timeAxis->setFormat("MMM dd");

    m_chart->addAxis(m_valueAxis, Qt::AlignLeft);
    m_chart->addAxis(m_timeAxis, Qt::AlignBottom);

    m_balanceSeries->attachAxis(m_valueAxis);
    m_balanceSeries->attachAxis(m_timeAxis);
    m_rewardsSeries->attachAxis(m_valueAxis);
    m_rewardsSeries->attachAxis(m_timeAxis);
    m_tokenSeries->attachAxis(m_valueAxis);
    m_tokenSeries->attachAxis(m_timeAxis);

    m_chartView->setChart(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

void PortfolioTracker::refreshPortfolioData()
{
    // Fetch portfolio data for current time range
    m_portfolioHistory = fetchPortfolioData(m_currentTimeRange);
    
    // Calculate current portfolio snapshot
    PortfolioData currentSnapshot = calculatePortfolioSnapshot();
    m_portfolioHistory.append(currentSnapshot);
    
    // Calculate performance metrics
    m_currentMetrics = calculateMetrics(m_portfolioHistory);
    
    // Update UI
    updatePortfolioDisplay();
    updateMetricsDisplay();
    updateBalanceChart();
    
    // Save to history
    savePortfolioHistory();
    
    emit portfolioUpdated(m_portfolioHistory);
    emit performanceCalculated(m_currentMetrics);
}

PortfolioTracker::PortfolioData PortfolioTracker::calculatePortfolioSnapshot()
{
    PortfolioData snapshot;
    snapshot.timestamp = QDateTime::currentDateTime();
    
    // Mock data for development - replace with actual wallet data
    snapshot.shahBalance = 1000.0; // TODO: Get from WalletModel
    snapshot.shahValue = snapshot.shahBalance * fetchShahPrice(snapshot.timestamp);
    snapshot.stakingRewards = m_includeStaking ? 25.0 : 0.0; // TODO: Get from staking data
    snapshot.tokenValue = m_includeTokens ? 150.0 : 0.0; // TODO: Get from token data
    snapshot.nftValue = m_includeNFTs ? 75.0 : 0.0; // TODO: Get from NFT data
    snapshot.totalValue = snapshot.shahValue + snapshot.stakingRewards + snapshot.tokenValue + snapshot.nftValue;
    
    // Calculate change from previous data point
    if (!m_portfolioHistory.isEmpty()) {
        double previousValue = m_portfolioHistory.last().totalValue;
        snapshot.changePercent = ((snapshot.totalValue - previousValue) / previousValue) * 100.0;
    } else {
        snapshot.changePercent = 0.0;
    }
    
    return snapshot;
}

QList<PortfolioTracker::PortfolioData> PortfolioTracker::fetchPortfolioData(TimeRange range)
{
    QList<PortfolioData> data;
    QDateTime endTime = QDateTime::currentDateTime();
    QDateTime startTime;
    
    switch (range) {
        case Day:
            startTime = endTime.addDays(-1);
            break;
        case Week:
            startTime = endTime.addDays(-7);
            break;
        case Month:
            startTime = endTime.addMonths(-1);
            break;
        case Quarter:
            startTime = endTime.addMonths(-3);
            break;
        case Year:
            startTime = endTime.addYears(-1);
            break;
        case Custom:
            startTime = endTime.addDays(-30); // Default to 30 days
            break;
    }
    
    // Generate mock historical data for development
    QDateTime current = startTime;
    double baseValue = 1000.0;
    
    while (current <= endTime) {
        PortfolioData point;
        point.timestamp = current;
        point.shahBalance = baseValue + (qrand() % 100 - 50); // Random variation
        point.shahValue = point.shahBalance * fetchShahPrice(current);
        point.stakingRewards = m_includeStaking ? (qrand() % 50) : 0.0;
        point.tokenValue = m_includeTokens ? (qrand() % 200) : 0.0;
        point.nftValue = m_includeNFTs ? (qrand() % 100) : 0.0;
        point.totalValue = point.shahValue + point.stakingRewards + point.tokenValue + point.nftValue;
        point.changePercent = 0.0; // Will be calculated later
        
        data.append(point);
        current = current.addDays(1);
        baseValue = point.totalValue;
    }
    
    return data;
}

PortfolioTracker::PerformanceMetrics PortfolioTracker::calculateMetrics(const QList<PortfolioData>& data)
{
    PerformanceMetrics metrics;
    
    if (data.size() < 2) {
        return metrics;
    }
    
    double initialValue = data.first().totalValue;
    double finalValue = data.last().totalValue;
    
    metrics.totalReturn = ((finalValue - initialValue) / initialValue) * 100.0;
    metrics.daysTracked = data.size();
    metrics.averageReturn = metrics.totalReturn / metrics.daysTracked;
    
    // Calculate daily returns and find best/worst days
    QList<double> dailyReturns;
    double bestDay = -999999.0;
    double worstDay = 999999.0;
    
    for (int i = 1; i < data.size(); ++i) {
        double dailyReturn = ((data[i].totalValue - data[i-1].totalValue) / data[i-1].totalValue) * 100.0;
        dailyReturns.append(dailyReturn);
        
        if (dailyReturn > bestDay) bestDay = dailyReturn;
        if (dailyReturn < worstDay) worstDay = dailyReturn;
    }
    
    metrics.bestDay = bestDay;
    metrics.worstDay = worstDay;
    
    // Calculate volatility (standard deviation of daily returns)
    if (!dailyReturns.isEmpty()) {
        double mean = 0.0;
        for (double ret : dailyReturns) {
            mean += ret;
        }
        mean /= dailyReturns.size();
        
        double variance = 0.0;
        for (double ret : dailyReturns) {
            variance += (ret - mean) * (ret - mean);
        }
        variance /= dailyReturns.size();
        
        metrics.volatility = sqrt(variance);
        
        // Calculate Sharpe ratio (assuming risk-free rate of 2%)
        double riskFreeRate = 2.0 / 365.0; // Daily risk-free rate
        metrics.sharpeRatio = (metrics.averageReturn - riskFreeRate) / metrics.volatility;
    }
    
    return metrics;
}

double PortfolioTracker::fetchShahPrice(const QDateTime& timestamp)
{
    // Mock price data - replace with actual API call
    Q_UNUSED(timestamp);
    return 0.50 + (qrand() % 100) / 1000.0; // Random price between 0.50 and 0.60
}

double PortfolioTracker::fetchTokenValue(const QString& tokenId)
{
    // Mock token value - replace with actual token data
    Q_UNUSED(tokenId);
    return qrand() % 100;
}

double PortfolioTracker::fetchNFTValue(const QString& nftId)
{
    // Mock NFT value - replace with actual NFT data
    Q_UNUSED(nftId);
    return qrand() % 50;
}

void PortfolioTracker::updatePortfolioDisplay()
{
    if (m_portfolioHistory.isEmpty()) return;
    
    PortfolioData latest = m_portfolioHistory.last();
    
    m_totalValueLabel->setText(tr("Total Value: $%1").arg(latest.totalValue, 0, 'f', 2));
    
    QString changeText = tr("Change: $%1").arg(latest.totalValue - (m_portfolioHistory.size() > 1 ? m_portfolioHistory[m_portfolioHistory.size()-2].totalValue : latest.totalValue), 0, 'f', 2);
    m_changeLabel->setText(changeText);
    
    QString changePercentText = tr("(%1%)").arg(latest.changePercent, 0, 'f', 2);
    m_changePercentLabel->setText(changePercentText);
    
    // Color coding for positive/negative change
    if (latest.changePercent >= 0) {
        m_changeLabel->setStyleSheet("color: #2E8B57; font-weight: bold;");
        m_changePercentLabel->setStyleSheet("color: #2E8B57; font-weight: bold;");
    } else {
        m_changeLabel->setStyleSheet("color: #DC143C; font-weight: bold;");
        m_changePercentLabel->setStyleSheet("color: #DC143C; font-weight: bold;");
    }
    
    animateValueUpdate();
}

void PortfolioTracker::updateMetricsDisplay()
{
    m_totalReturnLabel->setText(tr("Total Return: %1%").arg(m_currentMetrics.totalReturn, 0, 'f', 2));
    m_averageReturnLabel->setText(tr("Avg Return: %1%").arg(m_currentMetrics.averageReturn, 0, 'f', 2));
    m_bestDayLabel->setText(tr("Best Day: %1%").arg(m_currentMetrics.bestDay, 0, 'f', 2));
    m_worstDayLabel->setText(tr("Worst Day: %1%").arg(m_currentMetrics.worstDay, 0, 'f', 2));
    m_volatilityLabel->setText(tr("Volatility: %1%").arg(m_currentMetrics.volatility, 0, 'f', 2));
    m_sharpeRatioLabel->setText(tr("Sharpe Ratio: %1").arg(m_currentMetrics.sharpeRatio, 0, 'f', 2));
    
    // Color coding for metrics
    if (m_currentMetrics.totalReturn >= 0) {
        m_totalReturnLabel->setStyleSheet("color: #2E8B57; font-weight: bold;");
    } else {
        m_totalReturnLabel->setStyleSheet("color: #DC143C; font-weight: bold;");
    }
}

void PortfolioTracker::updateBalanceChart()
{
    m_balanceSeries->clear();
    m_rewardsSeries->clear();
    m_tokenSeries->clear();
    
    for (const PortfolioData& data : m_portfolioHistory) {
        qint64 timestamp = data.timestamp.toMSecsSinceEpoch();
        m_balanceSeries->append(timestamp, data.shahValue);
        m_rewardsSeries->append(timestamp, data.stakingRewards);
        m_tokenSeries->append(timestamp, data.tokenValue + data.nftValue);
    }
    
    updateChartAxes();
}

void PortfolioTracker::updateChartAxes()
{
    if (m_portfolioHistory.isEmpty()) return;
    
    QDateTime startTime = m_portfolioHistory.first().timestamp;
    QDateTime endTime = m_portfolioHistory.last().timestamp;
    
    m_timeAxis->setRange(startTime, endTime);
    
    double minValue = 0.0;
    double maxValue = 0.0;
    
    for (const PortfolioData& data : m_portfolioHistory) {
        double totalValue = data.totalValue;
        if (totalValue < minValue || minValue == 0.0) minValue = totalValue;
        if (totalValue > maxValue) maxValue = totalValue;
    }
    
    // Add some padding to the chart
    double padding = (maxValue - minValue) * 0.1;
    m_valueAxis->setRange(minValue - padding, maxValue + padding);
}

void PortfolioTracker::animateValueUpdate()
{
    m_valueAnimation->setTargetObject(m_valueOpacityEffect);
    m_valueAnimation->setPropertyName("opacity");
    m_valueAnimation->setDuration(ANIMATION_DURATION_MS);
    m_valueAnimation->setStartValue(0.3);
    m_valueAnimation->setEndValue(1.0);
    m_valueAnimation->start();
}

void PortfolioTracker::onRefreshClicked()
{
    refreshPortfolioData();
    showNotification(tr("Portfolio data refreshed"), "info");
}

void PortfolioTracker::onTimeRangeChanged(int index)
{
    m_currentTimeRange = static_cast<TimeRange>(m_timeRangeCombo->itemData(index).toInt());
    refreshPortfolioData();
}

void PortfolioTracker::onChartTypeChanged(int index)
{
    m_currentChartType = static_cast<ChartType>(m_chartTypeCombo->itemData(index).toInt());
    updateBalanceChart(); // For now, all chart types use the same data
}

void PortfolioTracker::onExportDataClicked()
{
    showExportDialog();
}

void PortfolioTracker::onAutoRefreshToggled(bool enabled)
{
    m_autoRefreshEnabled = enabled;
    if (enabled) {
        m_updateTimer->start();
    } else {
        m_updateTimer->stop();
    }
}

void PortfolioTracker::onIncludeStakingToggled(bool enabled)
{
    m_includeStaking = enabled;
    refreshPortfolioData();
}

void PortfolioTracker::onIncludeTokensToggled(bool enabled)
{
    m_includeTokens = enabled;
    refreshPortfolioData();
}

void PortfolioTracker::onIncludeNFTsToggled(bool enabled)
{
    m_includeNFTs = enabled;
    refreshPortfolioData();
}

void PortfolioTracker::onUpdateTimer()
{
    refreshPortfolioData();
}

void PortfolioTracker::onPortfolioItemClicked(QTableWidgetItem* item)
{
    Q_UNUSED(item);
    // TODO: Show detailed view for selected portfolio entry
}

void PortfolioTracker::onContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);
    // TODO: Implement context menu for portfolio table
}

void PortfolioTracker::showExportDialog()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Portfolio Data"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/portfolio_data.json",
        tr("JSON Files (*.json);;CSV Files (*.csv)"));
    
    if (filename.isEmpty()) return;
    
    // Export data
    QJsonObject exportData;
    QJsonArray portfolioArray;
    
    for (const PortfolioData& data : m_portfolioHistory) {
        QJsonObject entry;
        entry["timestamp"] = data.timestamp.toString(Qt::ISODate);
        entry["shahBalance"] = data.shahBalance;
        entry["shahValue"] = data.shahValue;
        entry["stakingRewards"] = data.stakingRewards;
        entry["tokenValue"] = data.tokenValue;
        entry["nftValue"] = data.nftValue;
        entry["totalValue"] = data.totalValue;
        entry["changePercent"] = data.changePercent;
        portfolioArray.append(entry);
    }
    
    exportData["portfolio"] = portfolioArray;
    exportData["metrics"] = QJsonObject{
        {"totalReturn", m_currentMetrics.totalReturn},
        {"averageReturn", m_currentMetrics.averageReturn},
        {"bestDay", m_currentMetrics.bestDay},
        {"worstDay", m_currentMetrics.worstDay},
        {"volatility", m_currentMetrics.volatility},
        {"sharpeRatio", m_currentMetrics.sharpeRatio},
        {"daysTracked", m_currentMetrics.daysTracked}
    };
    
    QJsonDocument doc(exportData);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        showNotification(tr("Portfolio data exported successfully"), "success");
        emit dataExported(filename);
    } else {
        showNotification(tr("Failed to export portfolio data"), "error");
    }
}

void PortfolioTracker::showNotification(const QString& message, const QString& type)
{
    Q_UNUSED(type);
    // TODO: Implement proper notification system
    qDebug() << "Portfolio Tracker:" << message;
}

void PortfolioTracker::saveSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_settings->setValue(SETTINGS_TIME_RANGE, m_currentTimeRange);
    m_settings->setValue(SETTINGS_CHART_TYPE, m_currentChartType);
    m_settings->setValue(SETTINGS_AUTO_REFRESH, m_autoRefreshEnabled);
    m_settings->setValue(SETTINGS_INCLUDE_STAKING, m_includeStaking);
    m_settings->setValue(SETTINGS_INCLUDE_TOKENS, m_includeTokens);
    m_settings->setValue(SETTINGS_INCLUDE_NFTS, m_includeNFTs);
    m_settings->endGroup();
}

void PortfolioTracker::loadSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_currentTimeRange = static_cast<TimeRange>(m_settings->value(SETTINGS_TIME_RANGE, Month).toInt());
    m_currentChartType = static_cast<ChartType>(m_settings->value(SETTINGS_CHART_TYPE, BalanceChart).toInt());
    m_autoRefreshEnabled = m_settings->value(SETTINGS_AUTO_REFRESH, false).toBool();
    m_includeStaking = m_settings->value(SETTINGS_INCLUDE_STAKING, true).toBool();
    m_includeTokens = m_settings->value(SETTINGS_INCLUDE_TOKENS, true).toBool();
    m_includeNFTs = m_settings->value(SETTINGS_INCLUDE_NFTS, true).toBool();
    m_settings->endGroup();
    
    // Update UI to reflect loaded settings
    m_timeRangeCombo->setCurrentIndex(m_currentTimeRange);
    m_chartTypeCombo->setCurrentIndex(m_currentChartType);
    m_autoRefreshCheck->setChecked(m_autoRefreshEnabled);
    m_includeStakingCheck->setChecked(m_includeStaking);
    m_includeTokensCheck->setChecked(m_includeTokens);
    m_includeNFTsCheck->setChecked(m_includeNFTs);
}

void PortfolioTracker::savePortfolioHistory()
{
    // TODO: Implement portfolio history persistence
}

void PortfolioTracker::loadPortfolioHistory()
{
    // TODO: Implement portfolio history loading
}

void PortfolioTracker::cleanupOldHistory()
{
    // TODO: Implement cleanup of old portfolio data
}

// PortfolioHistoryTable implementation
PortfolioHistoryTable::PortfolioHistoryTable(QWidget* parent)
    : QTableWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTableWidget::customContextMenuRequested, this, &PortfolioHistoryTable::contextMenuEvent);
}

void PortfolioHistoryTable::addPortfolioRecord(const QDateTime& timestamp, double totalValue, double changePercent, bool positive)
{
    int row = rowCount();
    insertRow(row);
    
    setItem(row, 0, new QTableWidgetItem(timestamp.toString("MMM dd, yyyy")));
    setItem(row, 1, new QTableWidgetItem(QString("$%1").arg(totalValue, 0, 'f', 2)));
    setItem(row, 2, new QTableWidgetItem(QString("%1%").arg(changePercent, 0, 'f', 2)));
    setItem(row, 3, new QTableWidgetItem(positive ? "↗" : "↘"));
    
    // Color coding
    if (positive) {
        item(row, 2)->setForeground(QColor("#2E8B57"));
        item(row, 3)->setForeground(QColor("#2E8B57"));
    } else {
        item(row, 2)->setForeground(QColor("#DC143C"));
        item(row, 3)->setForeground(QColor("#DC143C"));
    }
}

void PortfolioHistoryTable::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    QAction* copyValue = menu.addAction(tr("Copy Value"));
    QAction* copyTimestamp = menu.addAction(tr("Copy Timestamp"));
    menu.addSeparator();
    QAction* exportSelected = menu.addAction(tr("Export Selected"));
    
    QAction* selectedAction = menu.exec(event->globalPos());
    
    if (selectedAction == copyValue) {
        onCopyValue();
    } else if (selectedAction == copyTimestamp) {
        onCopyTimestamp();
    } else if (selectedAction == exportSelected) {
        onExportSelected();
    }
}

void PortfolioHistoryTable::onCopyValue()
{
    // TODO: Implement copy value functionality
}

void PortfolioHistoryTable::onCopyTimestamp()
{
    // TODO: Implement copy timestamp functionality
}

void PortfolioHistoryTable::onExportSelected()
{
    // TODO: Implement export selected functionality
}

// PerformanceMetricsWidget implementation
PerformanceMetricsWidget::PerformanceMetricsWidget(QWidget* parent)
    : QWidget(parent)
    , m_positiveColor(QColor("#2E8B57"))
    , m_negativeColor(QColor("#DC143C"))
{
    setMinimumHeight(100);
}

void PerformanceMetricsWidget::updateMetrics(const PortfolioTracker::PerformanceMetrics& metrics)
{
    m_metrics = metrics;
    update();
}

void PerformanceMetricsWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw performance metrics visualization
    // TODO: Implement custom painting for metrics display
}
