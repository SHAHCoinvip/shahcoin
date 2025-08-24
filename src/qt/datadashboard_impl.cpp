#include "datadashboard.h"
#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QProgressBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QPieSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QDateTimeAxis>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>

// DataDashboard Implementation
DataDashboard::DataDashboard(QObject* parent)
    : QObject(parent)
    , m_settings("Shahcoin", "DataDashboard")
    , m_networkManager(new QNetworkAccessManager(this))
    , m_refreshTimer(new QTimer(this))
    , m_autoRefreshEnabled(true)
    , m_refreshInterval(30000) // 30 seconds
    , m_chartTheme(QChart::ChartThemeLight)
    , m_exportFormat(ExportFormat::CSV)
{
    initialize();
}

DataDashboard::~DataDashboard()
{
    stopRefresh();
}

void DataDashboard::initialize()
{
    loadSettings();
    setupTimers();
    setupCharts();
    setupNetworkMonitoring();
    
    qDebug() << "[DataDashboard] Initialized with settings:";
    qDebug() << "  Auto Refresh:" << m_autoRefreshEnabled;
    qDebug() << "  Refresh Interval:" << m_refreshInterval << "ms";
    qDebug() << "  Chart Theme:" << static_cast<int>(m_chartTheme);
    qDebug() << "  Export Format:" << static_cast<int>(m_exportFormat);
}

void DataDashboard::setupTimers()
{
    connect(m_refreshTimer, &QTimer::timeout, this, &DataDashboard::refreshData);
    m_refreshTimer->setInterval(m_refreshInterval);
    
    if (m_autoRefreshEnabled) {
        m_refreshTimer->start();
    }
}

void DataDashboard::setupCharts()
{
    // Initialize chart themes
    m_chartThemes = {
        {QChart::ChartThemeLight, "Light Theme"},
        {QChart::ChartThemeDark, "Dark Theme"},
        {QChart::ChartThemeBlueCerulean, "Blue Cerulean"},
        {QChart::ChartThemeBrownSand, "Brown Sand"},
        {QChart::ChartThemeBlueNcs, "Blue NCS"},
        {QChart::ChartThemeHighContrast, "High Contrast"},
        {QChart::ChartThemeBlueIcy, "Blue Icy"},
        {QChart::ChartThemeQt, "Qt Theme"}
    };
    
    qDebug() << "[DataDashboard] Chart themes setup complete";
}

void DataDashboard::setupNetworkMonitoring()
{
    // Monitor network connectivity for data fetching
    connect(m_networkManager, &QNetworkAccessManager::networkAccessibleChanged,
            this, &DataDashboard::onNetworkAccessibilityChanged);
    
    qDebug() << "[DataDashboard] Network monitoring setup complete";
}

void DataDashboard::startRefresh()
{
    if (m_refreshTimer->isActive()) return;
    
    m_refreshTimer->start();
    qDebug() << "[DataDashboard] Auto refresh started";
    emit refreshStarted();
}

void DataDashboard::stopRefresh()
{
    m_refreshTimer->stop();
    qDebug() << "[DataDashboard] Auto refresh stopped";
    emit refreshStopped();
}

void DataDashboard::refreshData()
{
    if (!m_autoRefreshEnabled) return;
    
    qDebug() << "[DataDashboard] Refreshing data...";
    
    // Refresh miner stats
    refreshMinerStats();
    
    // Refresh node stats
    refreshNodeStats();
    
    // Refresh network stats
    refreshNetworkStats();
    
    // Refresh staking performance
    refreshStakingPerformance();
    
    emit dataRefreshed();
}

void DataDashboard::refreshMinerStats()
{
    // Simulate fetching miner statistics
    MinerStats stats;
    stats.timestamp = QDateTime::currentDateTime();
    stats.hashRate = 125.5; // MH/s
    stats.acceptedShares = 1024;
    stats.rejectedShares = 5;
    stats.uptime = 3600; // seconds
    stats.temperature = 65.0; // Celsius
    stats.powerConsumption = 850.0; // Watts
    stats.efficiency = 0.85; // 85%
    
    m_minerStats = stats;
    emit minerStatsUpdated(stats);
}

void DataDashboard::refreshNodeStats()
{
    // Simulate fetching node statistics
    NodeStats stats;
    stats.timestamp = QDateTime::currentDateTime();
    stats.peerCount = 12;
    stats.syncPercentage = 98.5;
    stats.blockHeight = 1234567;
    stats.connections = 8;
    stats.uptime = 7200; // seconds
    stats.memoryUsage = 512.0; // MB
    stats.cpuUsage = 15.0; // %
    stats.diskUsage = 2.5; // GB
    
    m_nodeStats = stats;
    emit nodeStatsUpdated(stats);
}

void DataDashboard::refreshNetworkStats()
{
    // Simulate fetching network statistics
    NetworkStats stats;
    stats.timestamp = QDateTime::currentDateTime();
    stats.totalNodes = 1250;
    stats.activeNodes = 1180;
    stats.networkHashRate = 125000.0; // MH/s
    stats.difficulty = 1.25;
    stats.blockTime = 120; // seconds
    stats.mempoolSize = 45;
    stats.transactionCount = 1250;
    stats.networkLatency = 45.0; // ms
    
    m_networkStats = stats;
    emit networkStatsUpdated(stats);
}

void DataDashboard::refreshStakingPerformance()
{
    // Simulate fetching staking performance data
    StakingPerformance stats;
    stats.timestamp = QDateTime::currentDateTime();
    stats.totalStaked = 50000.0; // SHAH
    stats.rewardsEarned = 1250.0; // SHAH
    stats.currentAPY = 12.5; // %
    stats.stakingDuration = 30; // days
    stats.rewardHistory = {100.0, 95.0, 110.0, 105.0, 115.0}; // Last 5 rewards
    stats.nftBoost = 1.15; // 15% boost
    stats.estimatedNextReward = 120.0; // SHAH
    
    m_stakingPerformance = stats;
    emit stakingPerformanceUpdated(stats);
}

// Chart Management
QChart* DataDashboard::createHashRateChart()
{
    QChart* chart = new QChart();
    chart->setTitle("Hash Rate Over Time");
    chart->setTheme(m_chartTheme);
    
    QLineSeries* series = new QLineSeries();
    series->setName("Hash Rate (MH/s)");
    
    // Add sample data points
    QDateTime now = QDateTime::currentDateTime();
    for (int i = 0; i < 24; ++i) {
        QDateTime time = now.addSecs(-i * 3600);
        double hashRate = 120.0 + (qrand() % 20 - 10); // Random variation
        series->append(time.toMSecsSinceEpoch(), hashRate);
    }
    
    chart->addSeries(series);
    
    // Create axes
    QDateTimeAxis* axisX = new QDateTimeAxis;
    axisX->setTickCount(6);
    axisX->setFormat("hh:mm");
    axisX->setTitleText("Time");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis;
    axisY->setLabelFormat("%.1f");
    axisY->setTitleText("Hash Rate (MH/s)");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    return chart;
}

QChart* DataDashboard::createStakingRewardsChart()
{
    QChart* chart = new QChart();
    chart->setTitle("Staking Rewards History");
    chart->setTheme(m_chartTheme);
    
    QBarSeries* series = new QBarSeries();
    QBarSet* rewardsSet = new QBarSet("Rewards (SHAH)");
    
    // Add sample reward data
    *rewardsSet << 100.0 << 95.0 << 110.0 << 105.0 << 115.0 << 120.0 << 118.0 << 125.0;
    series->append(rewardsSet);
    
    chart->addSeries(series);
    
    // Create axes
    QBarCategoryAxis* axisX = new QBarCategoryAxis;
    QStringList categories = {"Day 1", "Day 2", "Day 3", "Day 4", "Day 5", "Day 6", "Day 7", "Day 8"};
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis;
    axisY->setLabelFormat("%.1f");
    axisY->setTitleText("Rewards (SHAH)");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    return chart;
}

QChart* DataDashboard::createNetworkActivityChart()
{
    QChart* chart = new QChart();
    chart->setTitle("Network Activity");
    chart->setTheme(m_chartTheme);
    
    QLineSeries* txSeries = new QLineSeries();
    txSeries->setName("Transactions");
    
    QLineSeries* blockSeries = new QLineSeries();
    blockSeries->setName("Blocks");
    
    // Add sample data
    QDateTime now = QDateTime::currentDateTime();
    for (int i = 0; i < 12; ++i) {
        QDateTime time = now.addSecs(-i * 600); // 10-minute intervals
        int txCount = 1000 + (qrand() % 500);
        int blockCount = 5 + (qrand() % 3);
        
        txSeries->append(time.toMSecsSinceEpoch(), txCount);
        blockSeries->append(time.toMSecsSinceEpoch(), blockCount);
    }
    
    chart->addSeries(txSeries);
    chart->addSeries(blockSeries);
    
    // Create axes
    QDateTimeAxis* axisX = new QDateTimeAxis;
    axisX->setTickCount(6);
    axisX->setFormat("hh:mm");
    axisX->setTitleText("Time");
    chart->addAxis(axisX, Qt::AlignBottom);
    txSeries->attachAxis(axisX);
    blockSeries->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis;
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Count");
    chart->addAxis(axisY, Qt::AlignLeft);
    txSeries->attachAxis(axisY);
    blockSeries->attachAxis(axisY);
    
    return chart;
}

QChart* DataDashboard::createPerformancePieChart()
{
    QChart* chart = new QChart();
    chart->setTitle("System Performance Distribution");
    chart->setTheme(m_chartTheme);
    
    QPieSeries* series = new QPieSeries();
    
    // Add performance metrics
    series->append("Mining", 45.0);
    series->append("Staking", 30.0);
    series->append("Network", 15.0);
    series->append("Other", 10.0);
    
    chart->addSeries(series);
    
    return chart;
}

// Export functionality
void DataDashboard::exportData(ExportFormat format, const QString& filePath)
{
    QString actualPath = filePath;
    if (actualPath.isEmpty()) {
        actualPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/shahcoin_dashboard_export";
    }
    
    switch (format) {
        case ExportFormat::CSV:
            exportToCSV(actualPath + ".csv");
            break;
        case ExportFormat::JSON:
            exportToJSON(actualPath + ".json");
            break;
        case ExportFormat::PDF:
            exportToPDF(actualPath + ".pdf");
            break;
        case ExportFormat::EXCEL:
            exportToExcel(actualPath + ".xlsx");
            break;
    }
    
    emit dataExported(format, actualPath);
}

void DataDashboard::exportToCSV(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "[DataDashboard] Failed to open CSV file for writing:" << filePath;
        return;
    }
    
    QTextStream out(&file);
    
    // Export miner stats
    out << "Miner Statistics\n";
    out << "Timestamp,Hash Rate (MH/s),Accepted Shares,Rejected Shares,Uptime (s),Temperature (°C),Power (W),Efficiency (%)\n";
    out << m_minerStats.timestamp.toString(Qt::ISODate) << ","
        << m_minerStats.hashRate << ","
        << m_minerStats.acceptedShares << ","
        << m_minerStats.rejectedShares << ","
        << m_minerStats.uptime << ","
        << m_minerStats.temperature << ","
        << m_minerStats.powerConsumption << ","
        << (m_minerStats.efficiency * 100) << "\n\n";
    
    // Export node stats
    out << "Node Statistics\n";
    out << "Timestamp,Peer Count,Sync %,Block Height,Connections,Uptime (s),Memory (MB),CPU (%),Disk (GB)\n";
    out << m_nodeStats.timestamp.toString(Qt::ISODate) << ","
        << m_nodeStats.peerCount << ","
        << m_nodeStats.syncPercentage << ","
        << m_nodeStats.blockHeight << ","
        << m_nodeStats.connections << ","
        << m_nodeStats.uptime << ","
        << m_nodeStats.memoryUsage << ","
        << m_nodeStats.cpuUsage << ","
        << m_nodeStats.diskUsage << "\n\n";
    
    // Export network stats
    out << "Network Statistics\n";
    out << "Timestamp,Total Nodes,Active Nodes,Network Hash Rate (MH/s),Difficulty,Block Time (s),Mempool Size,Tx Count,Latency (ms)\n";
    out << m_networkStats.timestamp.toString(Qt::ISODate) << ","
        << m_networkStats.totalNodes << ","
        << m_networkStats.activeNodes << ","
        << m_networkStats.networkHashRate << ","
        << m_networkStats.difficulty << ","
        << m_networkStats.blockTime << ","
        << m_networkStats.mempoolSize << ","
        << m_networkStats.transactionCount << ","
        << m_networkStats.networkLatency << "\n\n";
    
    // Export staking performance
    out << "Staking Performance\n";
    out << "Timestamp,Total Staked (SHAH),Rewards Earned (SHAH),Current APY (%),Staking Duration (days),NFT Boost,Estimated Next Reward (SHAH)\n";
    out << m_stakingPerformance.timestamp.toString(Qt::ISODate) << ","
        << m_stakingPerformance.totalStaked << ","
        << m_stakingPerformance.rewardsEarned << ","
        << m_stakingPerformance.currentAPY << ","
        << m_stakingPerformance.stakingDuration << ","
        << m_stakingPerformance.nftBoost << ","
        << m_stakingPerformance.estimatedNextReward << "\n";
    
    file.close();
    qDebug() << "[DataDashboard] Data exported to CSV:" << filePath;
}

void DataDashboard::exportToJSON(const QString& filePath)
{
    QJsonObject root;
    
    // Miner stats
    QJsonObject minerStats;
    minerStats["timestamp"] = m_minerStats.timestamp.toString(Qt::ISODate);
    minerStats["hashRate"] = m_minerStats.hashRate;
    minerStats["acceptedShares"] = m_minerStats.acceptedShares;
    minerStats["rejectedShares"] = m_minerStats.rejectedShares;
    minerStats["uptime"] = m_minerStats.uptime;
    minerStats["temperature"] = m_minerStats.temperature;
    minerStats["powerConsumption"] = m_minerStats.powerConsumption;
    minerStats["efficiency"] = m_minerStats.efficiency;
    root["minerStats"] = minerStats;
    
    // Node stats
    QJsonObject nodeStats;
    nodeStats["timestamp"] = m_nodeStats.timestamp.toString(Qt::ISODate);
    nodeStats["peerCount"] = m_nodeStats.peerCount;
    nodeStats["syncPercentage"] = m_nodeStats.syncPercentage;
    nodeStats["blockHeight"] = m_nodeStats.blockHeight;
    nodeStats["connections"] = m_nodeStats.connections;
    nodeStats["uptime"] = m_nodeStats.uptime;
    nodeStats["memoryUsage"] = m_nodeStats.memoryUsage;
    nodeStats["cpuUsage"] = m_nodeStats.cpuUsage;
    nodeStats["diskUsage"] = m_nodeStats.diskUsage;
    root["nodeStats"] = nodeStats;
    
    // Network stats
    QJsonObject networkStats;
    networkStats["timestamp"] = m_networkStats.timestamp.toString(Qt::ISODate);
    networkStats["totalNodes"] = m_networkStats.totalNodes;
    networkStats["activeNodes"] = m_networkStats.activeNodes;
    networkStats["networkHashRate"] = m_networkStats.networkHashRate;
    networkStats["difficulty"] = m_networkStats.difficulty;
    networkStats["blockTime"] = m_networkStats.blockTime;
    networkStats["mempoolSize"] = m_networkStats.mempoolSize;
    networkStats["transactionCount"] = m_networkStats.transactionCount;
    networkStats["networkLatency"] = m_networkStats.networkLatency;
    root["networkStats"] = networkStats;
    
    // Staking performance
    QJsonObject stakingPerformance;
    stakingPerformance["timestamp"] = m_stakingPerformance.timestamp.toString(Qt::ISODate);
    stakingPerformance["totalStaked"] = m_stakingPerformance.totalStaked;
    stakingPerformance["rewardsEarned"] = m_stakingPerformance.rewardsEarned;
    stakingPerformance["currentAPY"] = m_stakingPerformance.currentAPY;
    stakingPerformance["stakingDuration"] = m_stakingPerformance.stakingDuration;
    stakingPerformance["nftBoost"] = m_stakingPerformance.nftBoost;
    stakingPerformance["estimatedNextReward"] = m_stakingPerformance.estimatedNextReward;
    
    QJsonArray rewardHistory;
    for (double reward : m_stakingPerformance.rewardHistory) {
        rewardHistory.append(reward);
    }
    stakingPerformance["rewardHistory"] = rewardHistory;
    root["stakingPerformance"] = stakingPerformance;
    
    // Write to file
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "[DataDashboard] Data exported to JSON:" << filePath;
    } else {
        qDebug() << "[DataDashboard] Failed to open JSON file for writing:" << filePath;
    }
}

void DataDashboard::exportToPDF(const QString& filePath)
{
    // This would integrate with a PDF library like QPdfWriter
    // For now, create a simple text-based report
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "[DataDashboard] Failed to open PDF file for writing:" << filePath;
        return;
    }
    
    QTextStream out(&file);
    out << "Shahcoin Data Dashboard Report\n";
    out << "Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n\n";
    
    out << "Miner Statistics:\n";
    out << "Hash Rate: " << m_minerStats.hashRate << " MH/s\n";
    out << "Accepted Shares: " << m_minerStats.acceptedShares << "\n";
    out << "Rejected Shares: " << m_minerStats.rejectedShares << "\n";
    out << "Uptime: " << m_minerStats.uptime << " seconds\n";
    out << "Temperature: " << m_minerStats.temperature << "°C\n";
    out << "Power Consumption: " << m_minerStats.powerConsumption << "W\n";
    out << "Efficiency: " << (m_minerStats.efficiency * 100) << "%\n\n";
    
    out << "Node Statistics:\n";
    out << "Peer Count: " << m_nodeStats.peerCount << "\n";
    out << "Sync Percentage: " << m_nodeStats.syncPercentage << "%\n";
    out << "Block Height: " << m_nodeStats.blockHeight << "\n";
    out << "Connections: " << m_nodeStats.connections << "\n";
    out << "Uptime: " << m_nodeStats.uptime << " seconds\n";
    out << "Memory Usage: " << m_nodeStats.memoryUsage << "MB\n";
    out << "CPU Usage: " << m_nodeStats.cpuUsage << "%\n";
    out << "Disk Usage: " << m_nodeStats.diskUsage << "GB\n\n";
    
    out << "Network Statistics:\n";
    out << "Total Nodes: " << m_networkStats.totalNodes << "\n";
    out << "Active Nodes: " << m_networkStats.activeNodes << "\n";
    out << "Network Hash Rate: " << m_networkStats.networkHashRate << " MH/s\n";
    out << "Difficulty: " << m_networkStats.difficulty << "\n";
    out << "Block Time: " << m_networkStats.blockTime << " seconds\n";
    out << "Mempool Size: " << m_networkStats.mempoolSize << "\n";
    out << "Transaction Count: " << m_networkStats.transactionCount << "\n";
    out << "Network Latency: " << m_networkStats.networkLatency << "ms\n\n";
    
    out << "Staking Performance:\n";
    out << "Total Staked: " << m_stakingPerformance.totalStaked << " SHAH\n";
    out << "Rewards Earned: " << m_stakingPerformance.rewardsEarned << " SHAH\n";
    out << "Current APY: " << m_stakingPerformance.currentAPY << "%\n";
    out << "Staking Duration: " << m_stakingPerformance.stakingDuration << " days\n";
    out << "NFT Boost: " << m_stakingPerformance.nftBoost << "x\n";
    out << "Estimated Next Reward: " << m_stakingPerformance.estimatedNextReward << " SHAH\n";
    
    file.close();
    qDebug() << "[DataDashboard] Data exported to PDF:" << filePath;
}

void DataDashboard::exportToExcel(const QString& filePath)
{
    // This would integrate with a library like QXlsx
    // For now, create a CSV file with .xlsx extension
    exportToCSV(filePath);
    qDebug() << "[DataDashboard] Data exported to Excel format:" << filePath;
}

// Settings Management
void DataDashboard::loadSettings()
{
    m_autoRefreshEnabled = m_settings.value("autoRefreshEnabled", true).toBool();
    m_refreshInterval = m_settings.value("refreshInterval", 30000).toInt();
    m_chartTheme = static_cast<QChart::ChartTheme>(m_settings.value("chartTheme", QChart::ChartThemeLight).toInt());
    m_exportFormat = static_cast<ExportFormat>(m_settings.value("exportFormat", ExportFormat::CSV).toInt());
}

void DataDashboard::saveSettings()
{
    m_settings.setValue("autoRefreshEnabled", m_autoRefreshEnabled);
    m_settings.setValue("refreshInterval", m_refreshInterval);
    m_settings.setValue("chartTheme", static_cast<int>(m_chartTheme));
    m_settings.setValue("exportFormat", static_cast<int>(m_exportFormat));
    m_settings.sync();
}

// Getters and Setters
bool DataDashboard::isAutoRefreshEnabled() const { return m_autoRefreshEnabled; }
int DataDashboard::getRefreshInterval() const { return m_refreshInterval; }
QChart::ChartTheme DataDashboard::getChartTheme() const { return m_chartTheme; }
ExportFormat DataDashboard::getExportFormat() const { return m_exportFormat; }

MinerStats DataDashboard::getMinerStats() const { return m_minerStats; }
NodeStats DataDashboard::getNodeStats() const { return m_nodeStats; }
NetworkStats DataDashboard::getNetworkStats() const { return m_networkStats; }
StakingPerformance DataDashboard::getStakingPerformance() const { return m_stakingPerformance; }

void DataDashboard::setAutoRefreshEnabled(bool enabled)
{
    if (m_autoRefreshEnabled != enabled) {
        m_autoRefreshEnabled = enabled;
        if (enabled) {
            startRefresh();
        } else {
            stopRefresh();
        }
        saveSettings();
        emit autoRefreshEnabledChanged(enabled);
    }
}

void DataDashboard::setRefreshInterval(int interval)
{
    if (m_refreshInterval != interval && interval > 0) {
        m_refreshInterval = interval;
        m_refreshTimer->setInterval(interval);
        saveSettings();
        emit refreshIntervalChanged(interval);
    }
}

void DataDashboard::setChartTheme(QChart::ChartTheme theme)
{
    if (m_chartTheme != theme) {
        m_chartTheme = theme;
        saveSettings();
        emit chartThemeChanged(theme);
    }
}

void DataDashboard::setExportFormat(ExportFormat format)
{
    if (m_exportFormat != format) {
        m_exportFormat = format;
        saveSettings();
        emit exportFormatChanged(format);
    }
}

// Network event handlers
void DataDashboard::onNetworkAccessibilityChanged(QNetworkAccessManager::NetworkAccessibility accessibility)
{
    if (accessibility == QNetworkAccessManager::NotAccessible) {
        qDebug() << "[DataDashboard] Network connectivity lost - data refresh may be affected";
        emit networkConnectivityLost();
    } else {
        qDebug() << "[DataDashboard] Network connectivity restored";
        emit networkConnectivityRestored();
    }
}

// Data Dashboard Settings Dialog Implementation
DataDashboardSettingsDialog::DataDashboardSettingsDialog(DataDashboard* dashboard, QWidget* parent)
    : QDialog(parent)
    , m_dashboard(dashboard)
{
    setWindowTitle("Data Dashboard Settings");
    setModal(true);
    resize(500, 400);
    
    setupUI();
    loadCurrentSettings();
}

void DataDashboardSettingsDialog::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Auto Refresh Settings
    QGroupBox* refreshGroup = new QGroupBox("Auto Refresh Settings");
    QVBoxLayout* refreshLayout = new QVBoxLayout(refreshGroup);
    
    QCheckBox* autoRefreshCheck = new QCheckBox("Enable Auto Refresh");
    autoRefreshCheck->setChecked(m_dashboard->isAutoRefreshEnabled());
    connect(autoRefreshCheck, &QCheckBox::toggled, this, &DataDashboardSettingsDialog::onAutoRefreshToggled);
    refreshLayout->addWidget(autoRefreshCheck);
    
    QHBoxLayout* intervalLayout = new QHBoxLayout();
    QLabel* intervalLabel = new QLabel("Refresh Interval (seconds):");
    intervalLayout->addWidget(intervalLabel);
    
    QSpinBox* intervalSpin = new QSpinBox();
    intervalSpin->setRange(10, 3600);
    intervalSpin->setValue(m_dashboard->getRefreshInterval() / 1000);
    connect(intervalSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &DataDashboardSettingsDialog::onIntervalChanged);
    intervalLayout->addWidget(intervalSpin);
    intervalLayout->addStretch();
    refreshLayout->addLayout(intervalLayout);
    
    layout->addWidget(refreshGroup);
    
    // Chart Settings
    QGroupBox* chartGroup = new QGroupBox("Chart Settings");
    QVBoxLayout* chartLayout = new QVBoxLayout(chartGroup);
    
    QHBoxLayout* themeLayout = new QHBoxLayout();
    QLabel* themeLabel = new QLabel("Chart Theme:");
    themeLayout->addWidget(themeLabel);
    
    QComboBox* themeCombo = new QComboBox();
    themeCombo->addItem("Light Theme", QChart::ChartThemeLight);
    themeCombo->addItem("Dark Theme", QChart::ChartThemeDark);
    themeCombo->addItem("Blue Cerulean", QChart::ChartThemeBlueCerulean);
    themeCombo->addItem("Brown Sand", QChart::ChartThemeBrownSand);
    themeCombo->addItem("Blue NCS", QChart::ChartThemeBlueNcs);
    themeCombo->addItem("High Contrast", QChart::ChartThemeHighContrast);
    themeCombo->addItem("Blue Icy", QChart::ChartThemeBlueIcy);
    themeCombo->addItem("Qt Theme", QChart::ChartThemeQt);
    
    int currentThemeIndex = themeCombo->findData(m_dashboard->getChartTheme());
    if (currentThemeIndex >= 0) {
        themeCombo->setCurrentIndex(currentThemeIndex);
    }
    
    connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DataDashboardSettingsDialog::onThemeChanged);
    themeLayout->addWidget(themeCombo);
    themeLayout->addStretch();
    chartLayout->addLayout(themeLayout);
    
    layout->addWidget(chartGroup);
    
    // Export Settings
    QGroupBox* exportGroup = new QGroupBox("Export Settings");
    QVBoxLayout* exportLayout = new QVBoxLayout(exportGroup);
    
    QHBoxLayout* formatLayout = new QHBoxLayout();
    QLabel* formatLabel = new QLabel("Default Export Format:");
    formatLayout->addWidget(formatLabel);
    
    QComboBox* formatCombo = new QComboBox();
    formatCombo->addItem("CSV", ExportFormat::CSV);
    formatCombo->addItem("JSON", ExportFormat::JSON);
    formatCombo->addItem("PDF", ExportFormat::PDF);
    formatCombo->addItem("Excel", ExportFormat::EXCEL);
    
    int currentFormatIndex = formatCombo->findData(m_dashboard->getExportFormat());
    if (currentFormatIndex >= 0) {
        formatCombo->setCurrentIndex(currentFormatIndex);
    }
    
    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DataDashboardSettingsDialog::onFormatChanged);
    formatLayout->addWidget(formatCombo);
    formatLayout->addStretch();
    exportLayout->addLayout(formatLayout);
    
    layout->addWidget(exportGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* okBtn = new QPushButton("OK");
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(okBtn);
    
    QPushButton* cancelBtn = new QPushButton("Cancel");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelBtn);
    
    layout->addLayout(buttonLayout);
}

void DataDashboardSettingsDialog::loadCurrentSettings()
{
    // Settings are loaded in setupUI()
}

void DataDashboardSettingsDialog::onAutoRefreshToggled(bool enabled)
{
    m_dashboard->setAutoRefreshEnabled(enabled);
}

void DataDashboardSettingsDialog::onIntervalChanged(int seconds)
{
    m_dashboard->setRefreshInterval(seconds * 1000);
}

void DataDashboardSettingsDialog::onThemeChanged(int index)
{
    QComboBox* combo = qobject_cast<QComboBox*>(sender());
    if (combo) {
        QChart::ChartTheme theme = static_cast<QChart::ChartTheme>(combo->itemData(index).toInt());
        m_dashboard->setChartTheme(theme);
    }
}

void DataDashboardSettingsDialog::onFormatChanged(int index)
{
    QComboBox* combo = qobject_cast<QComboBox*>(sender());
    if (combo) {
        ExportFormat format = static_cast<ExportFormat>(combo->itemData(index).toInt());
        m_dashboard->setExportFormat(format);
    }
}

// Data Dashboard Export Dialog Implementation
DataDashboardExportDialog::DataDashboardExportDialog(DataDashboard* dashboard, QWidget* parent)
    : QDialog(parent)
    , m_dashboard(dashboard)
{
    setWindowTitle("Export Data Dashboard");
    setModal(true);
    resize(400, 300);
    
    setupUI();
}

void DataDashboardExportDialog::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Export Format Selection
    QGroupBox* formatGroup = new QGroupBox("Export Format");
    QVBoxLayout* formatLayout = new QVBoxLayout(formatGroup);
    
    m_csvRadio = new QRadioButton("CSV (.csv)");
    m_csvRadio->setChecked(true);
    formatLayout->addWidget(m_csvRadio);
    
    m_jsonRadio = new QRadioButton("JSON (.json)");
    formatLayout->addWidget(m_jsonRadio);
    
    m_pdfRadio = new QRadioButton("PDF (.pdf)");
    formatLayout->addWidget(m_pdfRadio);
    
    m_excelRadio = new QRadioButton("Excel (.xlsx)");
    formatLayout->addWidget(m_excelRadio);
    
    layout->addWidget(formatGroup);
    
    // Data Selection
    QGroupBox* dataGroup = new QGroupBox("Data to Export");
    QVBoxLayout* dataLayout = new QVBoxLayout(dataGroup);
    
    m_includeMinerStats = new QCheckBox("Miner Statistics");
    m_includeMinerStats->setChecked(true);
    dataLayout->addWidget(m_includeMinerStats);
    
    m_includeNodeStats = new QCheckBox("Node Statistics");
    m_includeNodeStats->setChecked(true);
    dataLayout->addWidget(m_includeNodeStats);
    
    m_includeNetworkStats = new QCheckBox("Network Statistics");
    m_includeNetworkStats->setChecked(true);
    dataLayout->addWidget(m_includeNetworkStats);
    
    m_includeStakingStats = new QCheckBox("Staking Performance");
    m_includeStakingStats->setChecked(true);
    dataLayout->addWidget(m_includeStakingStats);
    
    layout->addWidget(dataGroup);
    
    // File Path
    QGroupBox* fileGroup = new QGroupBox("Export Location");
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);
    
    QHBoxLayout* pathLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit();
    m_pathEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/shahcoin_dashboard_export");
    pathLayout->addWidget(m_pathEdit);
    
    QPushButton* browseBtn = new QPushButton("Browse...");
    connect(browseBtn, &QPushButton::clicked, this, &DataDashboardExportDialog::onBrowseClicked);
    pathLayout->addWidget(browseBtn);
    
    fileLayout->addLayout(pathLayout);
    layout->addWidget(fileGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* exportBtn = new QPushButton("Export");
    connect(exportBtn, &QPushButton::clicked, this, &DataDashboardExportDialog::onExportClicked);
    buttonLayout->addWidget(exportBtn);
    
    QPushButton* cancelBtn = new QPushButton("Cancel");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelBtn);
    
    layout->addLayout(buttonLayout);
}

void DataDashboardExportDialog::onBrowseClicked()
{
    QString defaultPath = m_pathEdit->text();
    QString filePath = QFileDialog::getSaveFileName(this, "Save Export File", defaultPath, "All Files (*)");
    
    if (!filePath.isEmpty()) {
        m_pathEdit->setText(filePath);
    }
}

void DataDashboardExportDialog::onExportClicked()
{
    // Determine export format
    ExportFormat format = ExportFormat::CSV;
    if (m_jsonRadio->isChecked()) format = ExportFormat::JSON;
    else if (m_pdfRadio->isChecked()) format = ExportFormat::PDF;
    else if (m_excelRadio->isChecked()) format = ExportFormat::EXCEL;
    
    QString filePath = m_pathEdit->text();
    
    // Add appropriate extension if not present
    QString extension;
    switch (format) {
        case ExportFormat::CSV: extension = ".csv"; break;
        case ExportFormat::JSON: extension = ".json"; break;
        case ExportFormat::PDF: extension = ".pdf"; break;
        case ExportFormat::EXCEL: extension = ".xlsx"; break;
    }
    
    if (!filePath.endsWith(extension)) {
        filePath += extension;
    }
    
    // Export the data
    m_dashboard->exportData(format, filePath);
    
    QMessageBox::information(this, "Export Complete", 
                           QString("Data exported successfully to:\n%1").arg(filePath));
    
    accept();
}
