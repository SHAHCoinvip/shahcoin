// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#include "powposdashboard.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "qt/guiutil.h"
#include "qt/shahcoinunits.h"
#include "consensus/hybrid.h"
#include "chain.h"
#include "chainparams.h"
#include "validation.h"
#include "rpc/server.h"
#include "rpc/client.h"
#include "util/time.h"

#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QTimer>
#include <QThread>
#include <QFuture>
#include <QtConcurrent>

PoWPoSDashboard::PoWPoSDashboard(QWidget *parent)
    : QWidget(parent)
    , m_clientModel(nullptr)
    , m_walletModel(nullptr)
    , m_timeRange(1000) // Default to last 1000 blocks
    , m_refreshTimer(new QTimer(this))
{
    setupUI();
    setupCharts();
    setupTables();
    setupControls();
    connectSignals();
    
    // Set up auto-refresh timer
    m_refreshTimer->setInterval(30000); // 30 seconds default
    connect(m_refreshTimer, &QTimer::timeout, this, &PoWPoSDashboard::updateDashboard);
}

PoWPoSDashboard::~PoWPoSDashboard()
{
}

void PoWPoSDashboard::setClientModel(ClientModel *clientModel)
{
    m_clientModel = clientModel;
    if (m_clientModel) {
        connect(m_clientModel, &ClientModel::numBlocksChanged, this, &PoWPoSDashboard::updateDashboard);
        connect(m_clientModel, &ClientModel::strMiscWarningChanged, this, &PoWPoSDashboard::updateNetworkHealth);
    }
}

void PoWPoSDashboard::setWalletModel(WalletModel *walletModel)
{
    m_walletModel = walletModel;
    if (m_walletModel) {
        connect(m_walletModel, &WalletModel::balanceChanged, this, &PoWPoSDashboard::updateStakingStats);
    }
}

void PoWPoSDashboard::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Control Panel
    QGroupBox *controlGroup = new QGroupBox(tr("Dashboard Controls"));
    m_controlLayout = new QHBoxLayout(controlGroup);
    
    m_refreshButton = new QPushButton(tr("Refresh"));
    m_timeRangeCombo = new QComboBox();
    m_timeRangeCombo->addItem(tr("Last 100 blocks"), 100);
    m_timeRangeCombo->addItem(tr("Last 500 blocks"), 500);
    m_timeRangeCombo->addItem(tr("Last 1000 blocks"), 1000);
    m_timeRangeCombo->addItem(tr("Last 5000 blocks"), 5000);
    m_timeRangeCombo->setCurrentIndex(2); // Default to 1000 blocks
    
    m_autoRefreshCheck = new QCheckBox(tr("Auto-refresh"));
    m_refreshIntervalSpin = new QSpinBox();
    m_refreshIntervalSpin->setRange(10, 300);
    m_refreshIntervalSpin->setValue(30);
    m_refreshIntervalSpin->setSuffix(tr(" seconds"));
    
    m_controlLayout->addWidget(m_refreshButton);
    m_controlLayout->addWidget(new QLabel(tr("Time Range:")));
    m_controlLayout->addWidget(m_timeRangeCombo);
    m_controlLayout->addWidget(m_autoRefreshCheck);
    m_controlLayout->addWidget(new QLabel(tr("Interval:")));
    m_controlLayout->addWidget(m_refreshIntervalSpin);
    m_controlLayout->addStretch();
    
    m_mainLayout->addWidget(controlGroup);
    
    // Statistics Cards
    QGroupBox *statsGroup = new QGroupBox(tr("Network Statistics"));
    m_statsLayout = new QGridLayout(statsGroup);
    
    // Create stat cards
    m_totalBlocksLabel = new QLabel(tr("Total Blocks: 0"));
    m_powBlocksLabel = new QLabel(tr("PoW Blocks: 0"));
    m_posBlocksLabel = new QLabel(tr("PoS Blocks: 0"));
    m_currentAlgoLabel = new QLabel(tr("Current Algo: SHA256d"));
    m_nextAlgoLabel = new QLabel(tr("Next Algo: Scrypt"));
    m_avgBlockTimeLabel = new QLabel(tr("Avg Block Time: 0s"));
    m_networkHashrateLabel = new QLabel(tr("Network Hashrate: 0 H/s"));
    m_totalStakeLabel = new QLabel(tr("Total Stake: 0 SHAH"));
    m_stakingRewardsLabel = new QLabel(tr("Staking Rewards: 0 SHAH"));
    
    // Style the labels
    QList<QLabel*> statLabels = {m_totalBlocksLabel, m_powBlocksLabel, m_posBlocksLabel,
                                m_currentAlgoLabel, m_nextAlgoLabel, m_avgBlockTimeLabel,
                                m_networkHashrateLabel, m_totalStakeLabel, m_stakingRewardsLabel};
    
    for (QLabel *label : statLabels) {
        label->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 8px; border-radius: 4px; }");
        label->setAlignment(Qt::AlignCenter);
    }
    
    // Add to grid
    m_statsLayout->addWidget(m_totalBlocksLabel, 0, 0);
    m_statsLayout->addWidget(m_powBlocksLabel, 0, 1);
    m_statsLayout->addWidget(m_posBlocksLabel, 0, 2);
    m_statsLayout->addWidget(m_currentAlgoLabel, 1, 0);
    m_statsLayout->addWidget(m_nextAlgoLabel, 1, 1);
    m_statsLayout->addWidget(m_avgBlockTimeLabel, 1, 2);
    m_statsLayout->addWidget(m_networkHashrateLabel, 2, 0);
    m_statsLayout->addWidget(m_totalStakeLabel, 2, 1);
    m_statsLayout->addWidget(m_stakingRewardsLabel, 2, 2);
    
    m_mainLayout->addWidget(statsGroup);
    
    // Charts Section
    QHBoxLayout *chartsLayout = new QHBoxLayout();
    
    // Block Distribution Chart
    QGroupBox *blockDistGroup = new QGroupBox(tr("Block Distribution"));
    QVBoxLayout *blockDistLayout = new QVBoxLayout(blockDistGroup);
    m_blockDistributionChart = new QChartView();
    m_blockDistributionChart->setMinimumHeight(200);
    blockDistLayout->addWidget(m_blockDistributionChart);
    chartsLayout->addWidget(blockDistGroup);
    
    // Algorithm Usage Chart
    QGroupBox *algoGroup = new QGroupBox(tr("Algorithm Usage"));
    QVBoxLayout *algoLayout = new QVBoxLayout(algoGroup);
    m_algorithmUsageChart = new QChartView();
    m_algorithmUsageChart->setMinimumHeight(200);
    algoLayout->addWidget(m_algorithmUsageChart);
    chartsLayout->addWidget(algoGroup);
    
    m_mainLayout->addLayout(chartsLayout);
    
    // Difficulty and Staking Charts
    QHBoxLayout *chartsLayout2 = new QHBoxLayout();
    
    // Difficulty Chart
    QGroupBox *diffGroup = new QGroupBox(tr("Difficulty Trends"));
    QVBoxLayout *diffLayout = new QVBoxLayout(diffGroup);
    m_difficultyChart = new QChartView();
    m_difficultyChart->setMinimumHeight(200);
    diffLayout->addWidget(m_difficultyChart);
    chartsLayout2->addWidget(diffGroup);
    
    // Staking Chart
    QGroupBox *stakingGroup = new QGroupBox(tr("Staking Statistics"));
    QVBoxLayout *stakingLayout = new QVBoxLayout(stakingGroup);
    m_stakingChart = new QChartView();
    m_stakingChart->setMinimumHeight(200);
    stakingLayout->addWidget(m_stakingChart);
    chartsLayout2->addWidget(stakingGroup);
    
    m_mainLayout->addLayout(chartsLayout2);
    
    // Tables Section
    QHBoxLayout *tablesLayout = new QHBoxLayout();
    
    // Recent Blocks Table
    QGroupBox *blocksGroup = new QGroupBox(tr("Recent Blocks"));
    QVBoxLayout *blocksLayout = new QVBoxLayout(blocksGroup);
    m_recentBlocksTable = new QTableWidget();
    m_recentBlocksTable->setColumnCount(6);
    m_recentBlocksTable->setHorizontalHeaderLabels({tr("Height"), tr("Hash"), tr("Type"), tr("Algo"), tr("Time"), tr("Reward")});
    m_recentBlocksTable->setMaximumHeight(150);
    blocksLayout->addWidget(m_recentBlocksTable);
    tablesLayout->addWidget(blocksGroup);
    
    // Algorithm Stats Table
    QGroupBox *algoStatsGroup = new QGroupBox(tr("Algorithm Statistics"));
    QVBoxLayout *algoStatsLayout = new QVBoxLayout(algoStatsGroup);
    m_algorithmStatsTable = new QTableWidget();
    m_algorithmStatsTable->setColumnCount(4);
    m_algorithmStatsTable->setHorizontalHeaderLabels({tr("Algorithm"), tr("Blocks"), tr("Difficulty"), tr("Hashrate")});
    m_algorithmStatsTable->setMaximumHeight(150);
    algoStatsLayout->addWidget(m_algorithmStatsTable);
    tablesLayout->addWidget(algoStatsGroup);
    
    m_mainLayout->addLayout(tablesLayout);
    
    // Sync Progress
    QGroupBox *syncGroup = new QGroupBox(tr("Synchronization"));
    QVBoxLayout *syncLayout = new QVBoxLayout(syncGroup);
    m_syncProgressBar = new QProgressBar();
    m_syncStatusLabel = new QLabel(tr("Ready"));
    syncLayout->addWidget(m_syncProgressBar);
    syncLayout->addWidget(m_syncStatusLabel);
    m_mainLayout->addWidget(syncGroup);
}

void PoWPoSDashboard::setupCharts()
{
    // Block Distribution Pie Chart
    m_blockDistributionChartObj = new QChart();
    m_blockDistributionSeries = new QPieSeries();
    m_blockDistributionChartObj->addSeries(m_blockDistributionSeries);
    m_blockDistributionChartObj->setTitle(tr("Block Distribution (PoW vs PoS)"));
    m_blockDistributionChartObj->legend()->setAlignment(Qt::AlignBottom);
    m_blockDistributionChart->setChart(m_blockDistributionChartObj);
    
    // Algorithm Usage Bar Chart
    m_algorithmUsageChartObj = new QChart();
    m_algorithmUsageSeries = new QBarSeries();
    m_algorithmUsageChartObj->addSeries(m_algorithmUsageSeries);
    m_algorithmUsageChartObj->setTitle(tr("Algorithm Usage"));
    m_algorithmUsageChartObj->createDefaultAxes();
    m_algorithmUsageChart->setChart(m_algorithmUsageChartObj);
    
    // Difficulty Line Chart
    m_difficultyChartObj = new QChart();
    m_difficultySeries = new QLineSeries();
    m_difficultyChartObj->addSeries(m_difficultySeries);
    m_difficultyChartObj->setTitle(tr("Difficulty Trends"));
    m_difficultyChartObj->createDefaultAxes();
    m_difficultyChart->setChart(m_difficultyChartObj);
    
    // Staking Line Chart
    m_stakingChartObj = new QChart();
    m_stakingSeries = new QLineSeries();
    m_stakingChartObj->addSeries(m_stakingSeries);
    m_stakingChartObj->setTitle(tr("Staking Statistics"));
    m_stakingChartObj->createDefaultAxes();
    m_stakingChart->setChart(m_stakingChartObj);
}

void PoWPoSDashboard::setupTables()
{
    // Recent Blocks Table
    m_recentBlocksTable->horizontalHeader()->setStretchLastSection(true);
    m_recentBlocksTable->setAlternatingRowColors(true);
    m_recentBlocksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Algorithm Stats Table
    m_algorithmStatsTable->horizontalHeader()->setStretchLastSection(true);
    m_algorithmStatsTable->setAlternatingRowColors(true);
    m_algorithmStatsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void PoWPoSDashboard::setupControls()
{
    // Control setup is done in setupUI()
}

void PoWPoSDashboard::connectSignals()
{
    connect(m_refreshButton, &QPushButton::clicked, this, &PoWPoSDashboard::onRefreshClicked);
    connect(m_timeRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PoWPoSDashboard::onTimeRangeChanged);
    connect(m_autoRefreshCheck, &QCheckBox::toggled, this, &PoWPoSDashboard::onAutoRefreshToggled);
    connect(m_refreshIntervalSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        if (m_autoRefreshCheck->isChecked()) {
            m_refreshTimer->setInterval(value * 1000);
        }
    });
}

void PoWPoSDashboard::updateDashboard()
{
    if (!m_clientModel) return;
    
    // Update all dashboard components
    updateBlockDistribution();
    updateAlgorithmUsage();
    updateDifficultyChart();
    updateStakingStats();
    updateNetworkHealth();
    
    // Update sync progress
    int numBlocks = m_clientModel->getNumBlocks();
    int headers = m_clientModel->getNumHeaders();
    
    if (headers > 0) {
        int progress = (numBlocks * 100) / headers;
        m_syncProgressBar->setValue(progress);
        m_syncStatusLabel->setText(tr("Synced %1 of %2 blocks (%3%)").arg(numBlocks).arg(headers).arg(progress));
    } else {
        m_syncProgressBar->setValue(0);
        m_syncStatusLabel->setText(tr("No headers available"));
    }
}

void PoWPoSDashboard::updateBlockDistribution()
{
    if (!m_clientModel) return;
    
    // Fetch block data
    fetchBlockData();
    
    // Update pie chart
    updatePieChart(m_blockDistributionSeries, m_blockDistributionData);
    
    // Update statistics labels
    double totalBlocks = m_blockDistributionData.value("PoW", 0) + m_blockDistributionData.value("PoS", 0);
    double powPercentage = totalBlocks > 0 ? (m_blockDistributionData.value("PoW", 0) / totalBlocks) * 100 : 0;
    double posPercentage = totalBlocks > 0 ? (m_blockDistributionData.value("PoS", 0) / totalBlocks) * 100 : 0;
    
    m_totalBlocksLabel->setText(tr("Total Blocks: %1").arg(static_cast<int>(totalBlocks)));
    m_powBlocksLabel->setText(tr("PoW Blocks: %1 (%2%)").arg(static_cast<int>(m_blockDistributionData.value("PoW", 0))).arg(powPercentage, 0, 'f', 1));
    m_posBlocksLabel->setText(tr("PoS Blocks: %1 (%2%)").arg(static_cast<int>(m_blockDistributionData.value("PoS", 0))).arg(posPercentage, 0, 'f', 1));
}

void PoWPoSDashboard::updateAlgorithmUsage()
{
    if (!m_clientModel) return;
    
    // Update algorithm usage data
    m_algorithmUsageData.clear();
    m_algorithmUsageData["SHA256d"] = 0;
    m_algorithmUsageData["Scrypt"] = 0;
    m_algorithmUsageData["Groestl"] = 0;
    
    // Simulate algorithm usage data (in real implementation, fetch from blockchain)
    m_algorithmUsageData["SHA256d"] = 40;
    m_algorithmUsageData["Scrypt"] = 35;
    m_algorithmUsageData["Groestl"] = 25;
    
    // Update bar chart
    updateBarChart(m_algorithmUsageSeries, m_algorithmUsageData);
    
    // Update current/next algorithm labels
    int currentHeight = m_clientModel->getNumBlocks();
    AlgoType currentAlgo = SelectNextAlgo(currentHeight);
    AlgoType nextAlgo = SelectNextAlgo(currentHeight + 1);
    
    m_currentAlgoLabel->setText(tr("Current Algo: %1").arg(QString::fromStdString(AlgoName(currentAlgo))));
    m_nextAlgoLabel->setText(tr("Next Algo: %1").arg(QString::fromStdString(AlgoName(nextAlgo))));
}

void PoWPoSDashboard::updateDifficultyChart()
{
    if (!m_clientModel) return;
    
    // Fetch difficulty data
    fetchNetworkData();
    
    // Update line chart
    updateLineChart(m_difficultySeries, m_difficultyData);
}

void PoWPoSDashboard::updateStakingStats()
{
    if (!m_clientModel || !m_walletModel) return;
    
    // Fetch staking data
    fetchStakingData();
    
    // Update staking chart
    updateLineChart(m_stakingSeries, m_stakingData);
    
    // Update staking statistics labels
    double totalStake = 0;
    double totalRewards = 0;
    
    for (const auto &pair : m_stakingData) {
        totalStake += pair.second;
        totalRewards += pair.second * 0.05; // 5% staking reward
    }
    
    m_totalStakeLabel->setText(tr("Total Stake: %1 SHAH").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, totalStake)));
    m_stakingRewardsLabel->setText(tr("Staking Rewards: %1 SHAH").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, totalRewards)));
}

void PoWPoSDashboard::updateNetworkHealth()
{
    if (!m_clientModel) return;
    
    // Calculate average block time
    int numBlocks = m_clientModel->getNumBlocks();
    if (numBlocks > 1) {
        // Simulate average block time calculation
        double avgBlockTime = 150.0; // Target is 150 seconds
        m_avgBlockTimeLabel->setText(tr("Avg Block Time: %1s").arg(avgBlockTime, 0, 'f', 1));
    }
    
    // Calculate network hashrate (simplified)
    double networkHashrate = 1000000.0; // 1 MH/s
    m_networkHashrateLabel->setText(tr("Network Hashrate: %1 H/s").arg(GUIUtil::formatHashrate(networkHashrate)));
}

void PoWPoSDashboard::onRefreshClicked()
{
    updateDashboard();
}

void PoWPoSDashboard::onTimeRangeChanged(int index)
{
    m_timeRange = m_timeRangeCombo->itemData(index).toInt();
    updateDashboard();
}

void PoWPoSDashboard::onAutoRefreshToggled(bool checked)
{
    if (checked) {
        m_refreshTimer->start();
    } else {
        m_refreshTimer->stop();
    }
}

void PoWPoSDashboard::fetchBlockData()
{
    // Simulate fetching block distribution data
    m_blockDistributionData.clear();
    m_blockDistributionData["PoW"] = 90; // 90% PoW blocks
    m_blockDistributionData["PoS"] = 10; // 10% PoS blocks
}

void PoWPoSDashboard::fetchStakingData()
{
    // Simulate fetching staking data
    m_stakingData.clear();
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    
    for (int i = 0; i < 100; i++) {
        qint64 time = currentTime - (100 - i) * 150; // 150 second intervals
        double stake = 1000000.0 + (i * 1000.0); // Simulated stake amount
        m_stakingData.append({time, stake});
    }
}

void PoWPoSDashboard::fetchNetworkData()
{
    // Simulate fetching difficulty data
    m_difficultyData.clear();
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    
    for (int i = 0; i < 100; i++) {
        qint64 time = currentTime - (100 - i) * 150; // 150 second intervals
        double difficulty = 1000000.0 + (i * 10000.0); // Simulated difficulty
        m_difficultyData.append({time, difficulty});
    }
}

void PoWPoSDashboard::updatePieChart(QPieSeries *series, const QMap<QString, double> &data)
{
    series->clear();
    
    for (auto it = data.begin(); it != data.end(); ++it) {
        QPieSlice *slice = series->append(it.key(), it.value());
        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelOutside);
    }
}

void PoWPoSDashboard::updateBarChart(QBarSeries *series, const QMap<QString, double> &data)
{
    series->clear();
    
    QBarSet *set = new QBarSet("Usage");
    QStringList categories;
    
    for (auto it = data.begin(); it != data.end(); ++it) {
        *set << it.value();
        categories << it.key();
    }
    
    series->append(set);
    
    QBarCategoryAxis *axis = qobject_cast<QBarCategoryAxis*>(m_algorithmUsageChartObj->axes(Qt::Horizontal).first());
    if (axis) {
        axis->setCategories(categories);
    }
}

void PoWPoSDashboard::updateLineChart(QLineSeries *series, const QVector<QPair<qint64, double>> &data)
{
    series->clear();
    
    for (const auto &pair : data) {
        series->append(pair.first * 1000, pair.second); // Convert to milliseconds for Qt
    }
}
