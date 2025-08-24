// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/diagnosticspanel.h>
#include <qt/walletmodel.h>
#include <qt/platformstyle.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>
#include <interfaces/wallet.h>
#include <interfaces/node.h>
#include <util/translation.h>
#include <validation.h>
#include <miner.h>
#include <stake/stake.h>
#include <net.h>
#include <net_processing.h>
#include <util/system.h>
#include <util/strencodings.h>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QScrollBar>
#include <QStyle>
#include <QScreen>

DiagnosticsPanel::DiagnosticsPanel(const PlatformStyle* platformStyle, QWidget* parent)
    : QDialog(parent)
    , m_walletModel(nullptr)
    , m_platformStyle(platformStyle)
    , m_isUpdating(false)
    , m_currentTab(0)
    , m_logFilter("All")
    , m_updateTimer(new QTimer(this))
{
    setupUI();
    connectSignals();
    applyTheme();
    
    // Set window properties
    setWindowTitle(tr("Shahcoin Diagnostics"));
    setModal(true);
    resize(800, 600);
    
    // Center on screen
    if (parent) {
        move(parent->frameGeometry().center() - frameGeometry().center());
    }
    
    startUpdateTimer();
    refreshData();
}

DiagnosticsPanel::~DiagnosticsPanel()
{
    stopUpdateTimer();
}

void DiagnosticsPanel::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    refreshData();
}

void DiagnosticsPanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);
    
    // Setup individual tabs
    setupMiningTab();
    setupStakingTab();
    setupChainSyncTab();
    setupNetworkTab();
    setupWalletTab();
    setupLogsTab();
    
    mainLayout->addWidget(m_tabWidget);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_refreshButton = new QPushButton(tr("🔄 Refresh"), this);
    m_refreshButton->setToolTip(tr("Refresh all diagnostic data"));
    
    m_closeButton = new QPushButton(tr("✕ Close"), this);
    m_closeButton->setToolTip(tr("Close diagnostics panel"));
    
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addWidget(m_closeButton);
    
    mainLayout->addLayout(buttonLayout);
}

void DiagnosticsPanel::setupMiningTab()
{
    QWidget* miningWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(miningWidget);
    
    QGroupBox* miningGroup = new QGroupBox(tr("⛏️ Mining Metrics"));
    QGridLayout* grid = new QGridLayout(miningGroup);
    
    // Algorithm
    grid->addWidget(new QLabel(tr("Algorithm:")), 0, 0);
    m_miningAlgorithmLabel = new QLabel("SHA256d");
    m_miningAlgorithmLabel->setStyleSheet("font-weight: bold; color: #2E8B57;");
    grid->addWidget(m_miningAlgorithmLabel, 0, 1);
    
    // Hashrate
    grid->addWidget(new QLabel(tr("Hashrate:")), 1, 0);
    m_hashrateLabel = new QLabel("0 H/s");
    m_hashrateLabel->setStyleSheet("font-weight: bold;");
    grid->addWidget(m_hashrateLabel, 1, 1);
    
    // Threads
    grid->addWidget(new QLabel(tr("Active Threads:")), 2, 0);
    m_threadsLabel = new QLabel("0");
    grid->addWidget(m_threadsLabel, 2, 1);
    
    // Accepted shares
    grid->addWidget(new QLabel(tr("Accepted Shares:")), 3, 0);
    m_acceptedSharesLabel = new QLabel("0");
    m_acceptedSharesLabel->setStyleSheet("color: #2E8B57;");
    grid->addWidget(m_acceptedSharesLabel, 3, 1);
    
    // Rejected shares
    grid->addWidget(new QLabel(tr("Rejected Shares:")), 4, 0);
    m_rejectedSharesLabel = new QLabel("0");
    m_rejectedSharesLabel->setStyleSheet("color: #DC143C;");
    grid->addWidget(m_rejectedSharesLabel, 4, 1);
    
    // Mining errors
    grid->addWidget(new QLabel(tr("Errors:")), 5, 0);
    m_miningErrorsLabel = new QLabel("None");
    m_miningErrorsLabel->setStyleSheet("color: #DC143C;");
    grid->addWidget(m_miningErrorsLabel, 5, 1);
    
    // Progress bar
    m_miningProgressBar = new QProgressBar();
    m_miningProgressBar->setRange(0, 100);
    m_miningProgressBar->setValue(0);
    m_miningProgressBar->setFormat(tr("Mining Progress: %p%"));
    grid->addWidget(m_miningProgressBar, 6, 0, 1, 2);
    
    layout->addWidget(miningGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(miningWidget, tr("⛏️ Mining"));
}

void DiagnosticsPanel::setupStakingTab()
{
    QWidget* stakingWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(stakingWidget);
    
    QGroupBox* stakingGroup = new QGroupBox(tr("🌱 Staking Metrics"));
    QGridLayout* grid = new QGridLayout(stakingGroup);
    
    // Staking status
    grid->addWidget(new QLabel(tr("Status:")), 0, 0);
    m_stakingStatusLabel = new QLabel(tr("Inactive"));
    m_stakingStatusLabel->setStyleSheet("font-weight: bold; color: #DC143C;");
    grid->addWidget(m_stakingStatusLabel, 0, 1);
    
    // Staked amount
    grid->addWidget(new QLabel(tr("Staked Amount:")), 1, 0);
    m_stakedAmountLabel = new QLabel("0 SHAH");
    m_stakedAmountLabel->setStyleSheet("font-weight: bold;");
    grid->addWidget(m_stakedAmountLabel, 1, 1);
    
    // Expected reward time
    grid->addWidget(new QLabel(tr("Expected Reward:")), 2, 0);
    m_expectedRewardTimeLabel = new QLabel(tr("Not staking"));
    grid->addWidget(m_expectedRewardTimeLabel, 2, 1);
    
    // Wallet unlock status
    grid->addWidget(new QLabel(tr("Wallet Unlocked:")), 3, 0);
    m_walletUnlockStatusLabel = new QLabel(tr("No"));
    m_walletUnlockStatusLabel->setStyleSheet("color: #DC143C;");
    grid->addWidget(m_walletUnlockStatusLabel, 3, 1);
    
    // Progress bar
    m_stakingProgressBar = new QProgressBar();
    m_stakingProgressBar->setRange(0, 100);
    m_stakingProgressBar->setValue(0);
    m_stakingProgressBar->setFormat(tr("Staking Progress: %p%"));
    grid->addWidget(m_stakingProgressBar, 4, 0, 1, 2);
    
    layout->addWidget(stakingGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(stakingWidget, tr("🌱 Staking"));
}

void DiagnosticsPanel::setupChainSyncTab()
{
    QWidget* chainWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(chainWidget);
    
    QGroupBox* chainGroup = new QGroupBox(tr("⛓️ Chain Sync"));
    QGridLayout* grid = new QGridLayout(chainGroup);
    
    // Block height
    grid->addWidget(new QLabel(tr("Block Height:")), 0, 0);
    m_blockHeightLabel = new QLabel("0");
    m_blockHeightLabel->setStyleSheet("font-weight: bold;");
    grid->addWidget(m_blockHeightLabel, 0, 1);
    
    // Headers height
    grid->addWidget(new QLabel(tr("Headers Height:")), 1, 0);
    m_headersHeightLabel = new QLabel("0");
    grid->addWidget(m_headersHeightLabel, 1, 1);
    
    // Sync progress
    grid->addWidget(new QLabel(tr("Sync Progress:")), 2, 0);
    m_syncProgressLabel = new QLabel("0%");
    m_syncProgressLabel->setStyleSheet("font-weight: bold;");
    grid->addWidget(m_syncProgressLabel, 2, 1);
    
    // Reorg count
    grid->addWidget(new QLabel(tr("Reorg Count:")), 3, 0);
    m_reorgCountLabel = new QLabel("0");
    grid->addWidget(m_reorgCountLabel, 3, 1);
    
    // Last block time
    grid->addWidget(new QLabel(tr("Last Block:")), 4, 0);
    m_lastBlockTimeLabel = new QLabel(tr("Unknown"));
    grid->addWidget(m_lastBlockTimeLabel, 4, 1);
    
    // Progress bar
    m_syncProgressBar = new QProgressBar();
    m_syncProgressBar->setRange(0, 100);
    m_syncProgressBar->setValue(0);
    m_syncProgressBar->setFormat(tr("Sync Progress: %p%"));
    grid->addWidget(m_syncProgressBar, 5, 0, 1, 2);
    
    layout->addWidget(chainGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(chainWidget, tr("⛓️ Chain"));
}

void DiagnosticsPanel::setupNetworkTab()
{
    QWidget* networkWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(networkWidget);
    
    QGroupBox* networkGroup = new QGroupBox(tr("📡 Network Peers"));
    QVBoxLayout* networkLayout = new QVBoxLayout(networkGroup);
    
    // Summary info
    QHBoxLayout* summaryLayout = new QHBoxLayout();
    summaryLayout->addWidget(new QLabel(tr("Total Peers:")));
    m_totalPeersLabel = new QLabel("0");
    m_totalPeersLabel->setStyleSheet("font-weight: bold;");
    summaryLayout->addWidget(m_totalPeersLabel);
    summaryLayout->addStretch();
    
    summaryLayout->addWidget(new QLabel(tr("Avg Ping:")));
    m_pingTimeLabel = new QLabel("0ms");
    summaryLayout->addWidget(m_pingTimeLabel);
    
    networkLayout->addLayout(summaryLayout);
    
    // Peers table
    m_peersTable = new QTableWidget();
    m_peersTable->setColumnCount(5);
    m_peersTable->setHorizontalHeaderLabels({
        tr("Address"), tr("Type"), tr("Ping"), tr("Country"), tr("Version")
    });
    m_peersTable->horizontalHeader()->setStretchLastSection(true);
    m_peersTable->setAlternatingRowColors(true);
    m_peersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_peersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    networkLayout->addWidget(m_peersTable);
    
    layout->addWidget(networkGroup);
    
    m_tabWidget->addTab(networkWidget, tr("📡 Network"));
}

void DiagnosticsPanel::setupWalletTab()
{
    QWidget* walletWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(walletWidget);
    
    QGroupBox* walletGroup = new QGroupBox(tr("💼 Wallet Health"));
    QGridLayout* grid = new QGridLayout(walletGroup);
    
    // Confirmed balance
    grid->addWidget(new QLabel(tr("Confirmed Balance:")), 0, 0);
    m_confirmedBalanceLabel = new QLabel("0 SHAH");
    m_confirmedBalanceLabel->setStyleSheet("font-weight: bold; color: #2E8B57;");
    grid->addWidget(m_confirmedBalanceLabel, 0, 1);
    
    // Unconfirmed balance
    grid->addWidget(new QLabel(tr("Unconfirmed Balance:")), 1, 0);
    m_unconfirmedBalanceLabel = new QLabel("0 SHAH");
    m_unconfirmedBalanceLabel->setStyleSheet("color: #FF8C00;");
    grid->addWidget(m_unconfirmedBalanceLabel, 1, 1);
    
    // Staked balance
    grid->addWidget(new QLabel(tr("Staked Balance:")), 2, 0);
    m_stakedBalanceLabel = new QLabel("0 SHAH");
    m_stakedBalanceLabel->setStyleSheet("color: #4169E1;");
    grid->addWidget(m_stakedBalanceLabel, 2, 1);
    
    // Transaction count
    grid->addWidget(new QLabel(tr("Transactions:")), 3, 0);
    m_transactionCountLabel = new QLabel("0");
    grid->addWidget(m_transactionCountLabel, 3, 1);
    
    // Disk space
    grid->addWidget(new QLabel(tr("Disk Space:")), 4, 0);
    m_diskSpaceLabel = new QLabel("Unknown");
    grid->addWidget(m_diskSpaceLabel, 4, 1);
    
    // DB errors
    grid->addWidget(new QLabel(tr("DB Errors:")), 5, 0);
    m_dbErrorsLabel = new QLabel(tr("None"));
    m_dbErrorsLabel->setStyleSheet("color: #2E8B57;");
    grid->addWidget(m_dbErrorsLabel, 5, 1);
    
    // Disk space progress bar
    m_diskSpaceProgressBar = new QProgressBar();
    m_diskSpaceProgressBar->setRange(0, 100);
    m_diskSpaceProgressBar->setValue(0);
    m_diskSpaceProgressBar->setFormat(tr("Disk Usage: %p%"));
    grid->addWidget(m_diskSpaceProgressBar, 6, 0, 1, 2);
    
    layout->addWidget(walletGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(walletWidget, tr("💼 Wallet"));
}

void DiagnosticsPanel::setupLogsTab()
{
    QWidget* logsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(logsWidget);
    
    QGroupBox* logsGroup = new QGroupBox(tr("📋 System Logs"));
    QVBoxLayout* logsLayout = new QVBoxLayout(logsGroup);
    
    // Controls
    QHBoxLayout* controlsLayout = new QHBoxLayout();
    
    controlsLayout->addWidget(new QLabel(tr("Filter:")));
    m_logFilterCombo = new QComboBox();
    m_logFilterCombo->addItems({tr("All"), tr("Error"), tr("Warning"), tr("Info"), tr("Debug")});
    controlsLayout->addWidget(m_logFilterCombo);
    
    controlsLayout->addStretch();
    
    m_openLogFileButton = new QPushButton(tr("📁 Open Log File"));
    m_openLogFileButton->setToolTip(tr("Open debug.log in system default application"));
    controlsLayout->addWidget(m_openLogFileButton);
    
    m_clearLogsButton = new QPushButton(tr("🗑️ Clear"));
    m_clearLogsButton->setToolTip(tr("Clear displayed logs"));
    controlsLayout->addWidget(m_clearLogsButton);
    
    logsLayout->addLayout(controlsLayout);
    
    // Logs text area
    m_logsTextEdit = new QTextEdit();
    m_logsTextEdit->setReadOnly(true);
    m_logsTextEdit->setFont(QFont("Consolas", 9));
    m_logsTextEdit->setMaximumBlockCount(1000); // Limit to prevent memory issues
    logsLayout->addWidget(m_logsTextEdit);
    
    layout->addWidget(logsGroup);
    
    m_tabWidget->addTab(logsWidget, tr("📋 Logs"));
}

void DiagnosticsPanel::connectSignals()
{
    connect(m_refreshButton, &QPushButton::clicked, this, &DiagnosticsPanel::onRefreshClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_openLogFileButton, &QPushButton::clicked, this, &DiagnosticsPanel::onOpenLogFileClicked);
    connect(m_clearLogsButton, &QPushButton::clicked, m_logsTextEdit, &QTextEdit::clear);
    connect(m_logFilterCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &DiagnosticsPanel::onLogFilterChanged);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &DiagnosticsPanel::onTabChanged);
    
    // Update timer
    connect(m_updateTimer, &QTimer::timeout, this, &DiagnosticsPanel::refreshData);
}

void DiagnosticsPanel::applyTheme()
{
    // Apply consistent styling
    QString style = R"(
        QDialog {
            background-color: #f8f9fa;
        }
        QTabWidget::pane {
            border: 1px solid #dee2e6;
            background-color: white;
        }
        QTabBar::tab {
            background-color: #e9ecef;
            padding: 8px 16px;
            margin-right: 2px;
            border: 1px solid #dee2e6;
            border-bottom: none;
        }
        QTabBar::tab:selected {
            background-color: white;
            border-bottom: 1px solid white;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #dee2e6;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
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
        QTableWidget {
            gridline-color: #dee2e6;
            alternate-background-color: #f8f9fa;
        }
        QTableWidget::item {
            padding: 4px;
        }
        QProgressBar {
            border: 1px solid #dee2e6;
            border-radius: 3px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #007bff;
            border-radius: 2px;
        }
    )";
    
    setStyleSheet(style);
}

void DiagnosticsPanel::startUpdateTimer()
{
    m_updateTimer->start(5000); // Update every 5 seconds
}

void DiagnosticsPanel::stopUpdateTimer()
{
    m_updateTimer->stop();
}

void DiagnosticsPanel::refreshData()
{
    if (m_isUpdating) return;
    m_isUpdating = true;
    
    updateMiningMetrics();
    updateStakingMetrics();
    updateChainSyncMetrics();
    updateNetworkMetrics();
    updateWalletMetrics();
    updateLogs();
    
    m_isUpdating = false;
}

void DiagnosticsPanel::updateMiningMetrics()
{
    // Get mining data from backend
    std::string algorithm = "SHA256d"; // Default
    double hashrate = 0.0;
    int threads = 0;
    int acceptedShares = 0;
    int rejectedShares = 0;
    QString errors = tr("None");
    
    // Try to get actual mining data from backend
    try {
        // TODO: Connect to actual mining backend when available
        // This would typically call:
        // algorithm = g_miner->GetCurrentAlgorithm();
        // hashrate = g_miner->GetCurrentHashrate();
        // threads = g_miner->GetActiveThreads();
        // acceptedShares = g_miner->GetAcceptedShares();
        // rejectedShares = g_miner->GetRejectedShares();
        // errors = g_miner->GetLastError();
        
        // For now, simulate some data for demonstration
        if (m_walletModel && m_walletModel->isWalletUnlocked()) {
            hashrate = 1500000.0; // 1.5 MH/s
            threads = 4;
            acceptedShares = 12;
            rejectedShares = 1;
        }
    } catch (...) {
        // Handle any backend errors gracefully
        errors = tr("Backend error");
    }
    
    m_miningAlgorithmLabel->setText(QString::fromStdString(algorithm));
    m_hashrateLabel->setText(formatHashrate(hashrate));
    m_threadsLabel->setText(QString::number(threads));
    m_acceptedSharesLabel->setText(QString::number(acceptedShares));
    m_rejectedSharesLabel->setText(QString::number(rejectedShares));
    m_miningErrorsLabel->setText(errors);
    
    // Update progress bar based on hashrate
    int progress = hashrate > 0 ? qMin(100, static_cast<int>(hashrate / 1000000.0)) : 0;
    m_miningProgressBar->setValue(progress);
    
    // Update colors based on status
    if (hashrate > 0) {
        m_miningAlgorithmLabel->setStyleSheet("font-weight: bold; color: #2E8B57;");
        m_miningProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #2E8B57; }");
    } else {
        m_miningAlgorithmLabel->setStyleSheet("font-weight: bold; color: #DC143C;");
        m_miningProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #DC143C; }");
    }
}

void DiagnosticsPanel::updateStakingMetrics()
{
    // Get staking data from backend
    bool stakingActive = false;
    CAmount stakedAmount = 0;
    int expectedRewardTime = 0;
    bool walletUnlocked = false;
    
    if (m_walletModel) {
        walletUnlocked = m_walletModel->isWalletUnlocked();
        
        // TODO: Connect to actual staking backend when available
        // This would typically call:
        // stakingActive = g_stakingManager->IsStakingActive();
        // stakedAmount = g_stakingManager->GetTotalStakedAmount();
        // expectedRewardTime = g_stakingManager->GetExpectedRewardTime();
        
        // For now, simulate staking data based on wallet balance
        CAmount balance = m_walletModel->getBalance();
        if (balance >= 333 * COIN) { // Minimum staking amount
            stakingActive = walletUnlocked;
            stakedAmount = balance * 0.8; // Assume 80% is staked
            expectedRewardTime = 3600; // 1 hour placeholder
        }
    }
    
    // Update UI
    if (stakingActive) {
        m_stakingStatusLabel->setText(tr("Active"));
        m_stakingStatusLabel->setStyleSheet("font-weight: bold; color: #2E8B57;");
    } else {
        m_stakingStatusLabel->setText(tr("Inactive"));
        m_stakingStatusLabel->setStyleSheet("font-weight: bold; color: #DC143C;");
    }
    
    m_stakedAmountLabel->setText(ShahcoinUnits::format(ShahcoinUnits::SHAH, stakedAmount, false, ShahcoinUnits::separatorAlways));
    m_expectedRewardTimeLabel->setText(expectedRewardTime > 0 ? formatTime(expectedRewardTime) : tr("Not staking"));
    
    if (walletUnlocked) {
        m_walletUnlockStatusLabel->setText(tr("Yes"));
        m_walletUnlockStatusLabel->setStyleSheet("color: #2E8B57;");
    } else {
        m_walletUnlockStatusLabel->setText(tr("No"));
        m_walletUnlockStatusLabel->setStyleSheet("color: #DC143C;");
    }
    
    // Update progress bar based on staking status
    int progress = stakingActive ? 75 : 0;
    m_stakingProgressBar->setValue(progress);
    
    if (stakingActive) {
        m_stakingProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #2E8B57; }");
    } else {
        m_stakingProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #DC143C; }");
    }
}

void DiagnosticsPanel::updateChainSyncMetrics()
{
    // Get chain sync data from backend
    int blockHeight = 0;
    int headersHeight = 0;
    double syncProgress = 0.0;
    int reorgCount = 0;
    int64_t lastBlockTime = 0;
    
    // TODO: Connect to actual chain backend when available
    // This would typically call:
    // blockHeight = chainActive.Height();
    // headersHeight = pindexBestHeader->nHeight;
    // syncProgress = (double)blockHeight / headersHeight * 100.0;
    // reorgCount = g_chainman->GetReorgCount();
    // lastBlockTime = chainActive.Tip()->GetBlockTime();
    
    // For now, simulate some data for demonstration
    blockHeight = 12345;
    headersHeight = 12350;
    syncProgress = (double)blockHeight / headersHeight * 100.0;
    reorgCount = 2;
    lastBlockTime = QDateTime::currentSecsSinceEpoch() - 600; // 10 minutes ago
    
    m_blockHeightLabel->setText(QString::number(blockHeight));
    m_headersHeightLabel->setText(QString::number(headersHeight));
    m_syncProgressLabel->setText(QString::number(syncProgress, 'f', 1) + "%");
    m_reorgCountLabel->setText(QString::number(reorgCount));
    
    if (lastBlockTime > 0) {
        QDateTime blockTime = QDateTime::fromSecsSinceEpoch(lastBlockTime);
        m_lastBlockTimeLabel->setText(blockTime.toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        m_lastBlockTimeLabel->setText(tr("Unknown"));
    }
    
    // Update progress bar
    m_syncProgressBar->setValue(static_cast<int>(syncProgress));
    
    if (syncProgress >= 100.0) {
        m_syncProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #2E8B57; }");
    } else {
        m_syncProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #FF8C00; }");
    }
}

void DiagnosticsPanel::updateNetworkMetrics()
{
    // Get network data from backend
    int totalPeers = 0;
    double avgPing = 0.0;
    
    // TODO: Connect to actual network backend when available
    // This would typically call:
    // totalPeers = g_connman->GetNodeCount();
    // avgPing = g_connman->GetAveragePing();
    
    // For now, simulate some data for demonstration
    totalPeers = 8;
    avgPing = 45.2;
    
    m_totalPeersLabel->setText(QString::number(totalPeers));
    m_pingTimeLabel->setText(QString::number(avgPing, 'f', 1) + "ms");
    
    // Update peers table with sample data
    m_peersTable->setRowCount(totalPeers);
    for (int i = 0; i < totalPeers; ++i) {
        m_peersTable->setItem(i, 0, new QTableWidgetItem(QString("192.168.1.%1").arg(i + 100)));
        m_peersTable->setItem(i, 1, new QTableWidgetItem(i % 2 == 0 ? tr("Inbound") : tr("Outbound")));
        m_peersTable->setItem(i, 2, new QTableWidgetItem(QString("%1 ms").arg(30 + (i * 5))));
        m_peersTable->setItem(i, 3, new QTableWidgetItem(tr("Unknown")));
        m_peersTable->setItem(i, 4, new QTableWidgetItem(QString("v0.21.0")));
    }
}

void DiagnosticsPanel::updateWalletMetrics()
{
    if (!m_walletModel) return;
    
    // Get wallet data
    CAmount confirmedBalance = 0;
    CAmount unconfirmedBalance = 0;
    CAmount stakedBalance = 0;
    int transactionCount = 0;
    qint64 diskSpace = 0;
    QString dbErrors = tr("None");
    
    // Get balances from wallet model
    confirmedBalance = m_walletModel->getBalance();
    unconfirmedBalance = m_walletModel->getUnconfirmedBalance();
    
    // TODO: Get staked balance and transaction count from wallet
    // This would call wallet methods for staking info
    
    // Check disk space
    QDir dataDir = QDir::home();
    diskSpace = dataDir.availableSpace();
    
    // Update UI
    m_confirmedBalanceLabel->setText(ShahcoinUnits::format(ShahcoinUnits::SHAH, confirmedBalance, false, ShahcoinUnits::separatorAlways));
    m_unconfirmedBalanceLabel->setText(ShahcoinUnits::format(ShahcoinUnits::SHAH, unconfirmedBalance, false, ShahcoinUnits::separatorAlways));
    m_stakedBalanceLabel->setText(ShahcoinUnits::format(ShahcoinUnits::SHAH, stakedBalance, false, ShahcoinUnits::separatorAlways));
    m_transactionCountLabel->setText(QString::number(transactionCount));
    
    // Format disk space
    QString diskSpaceText;
    if (diskSpace > 0) {
        if (diskSpace > 1024 * 1024 * 1024) {
            diskSpaceText = QString::number(diskSpace / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GB";
        } else if (diskSpace > 1024 * 1024) {
            diskSpaceText = QString::number(diskSpace / (1024.0 * 1024.0), 'f', 1) + " MB";
        } else {
            diskSpaceText = QString::number(diskSpace / 1024.0, 'f', 1) + " KB";
        }
    } else {
        diskSpaceText = tr("Unknown");
    }
    m_diskSpaceLabel->setText(diskSpaceText);
    
    m_dbErrorsLabel->setText(dbErrors);
    if (dbErrors == tr("None")) {
        m_dbErrorsLabel->setStyleSheet("color: #2E8B57;");
    } else {
        m_dbErrorsLabel->setStyleSheet("color: #DC143C;");
    }
    
    // Update disk space progress bar
    qint64 totalSpace = dataDir.totalSpace();
    if (totalSpace > 0) {
        int usagePercent = static_cast<int>((totalSpace - diskSpace) * 100 / totalSpace);
        m_diskSpaceProgressBar->setValue(usagePercent);
        
        if (usagePercent > 90) {
            m_diskSpaceProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #DC143C; }");
        } else if (usagePercent > 75) {
            m_diskSpaceProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #FF8C00; }");
        } else {
            m_diskSpaceProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #2E8B57; }");
        }
    }
}

void DiagnosticsPanel::updateLogs()
{
    // Read recent log entries
    QString logFilePath = QDir::home().filePath(".shahcoin/debug.log");
    QFile logFile(logFilePath);
    
    if (!logFile.exists()) {
        m_logsTextEdit->setPlainText(tr("Log file not found: %1").arg(logFilePath));
        return;
    }
    
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_logsTextEdit->setPlainText(tr("Cannot open log file: %1").arg(logFile.errorString()));
        return;
    }
    
    QStringList logLines;
    QTextStream in(&logFile);
    
    // Read last 50 lines
    while (!in.atEnd()) {
        logLines << in.readLine();
        if (logLines.size() > 50) {
            logLines.removeFirst();
        }
    }
    
    logFile.close();
    
    // Filter logs based on selected level
    QString filteredLogs;
    for (const QString& line : logLines) {
        if (m_logFilter == tr("All") || line.contains(m_logFilter, Qt::CaseInsensitive)) {
            QString coloredLine = line;
            if (line.contains("ERROR", Qt::CaseInsensitive)) {
                coloredLine = QString("<span style='color: #DC143C;'>%1</span>").arg(line);
            } else if (line.contains("WARNING", Qt::CaseInsensitive)) {
                coloredLine = QString("<span style='color: #FF8C00;'>%1</span>").arg(line);
            } else if (line.contains("INFO", Qt::CaseInsensitive)) {
                coloredLine = QString("<span style='color: #2E8B57;'>%1</span>").arg(line);
            }
            filteredLogs += coloredLine + "<br>";
        }
    }
    
    m_logsTextEdit->setHtml(filteredLogs);
    
    // Scroll to bottom
    QScrollBar* scrollBar = m_logsTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void DiagnosticsPanel::onRefreshClicked()
{
    refreshData();
}

void DiagnosticsPanel::onOpenLogFileClicked()
{
    QString logFilePath = QDir::home().filePath(".shahcoin/debug.log");
    QFileInfo logFile(logFilePath);
    
    if (!logFile.exists()) {
        QMessageBox::warning(this, tr("Log File Not Found"),
                           tr("Debug log file not found at:\n%1").arg(logFilePath));
        return;
    }
    
    QDesktopServices::openUrl(QUrl::fromLocalFile(logFilePath));
}

void DiagnosticsPanel::onLogFilterChanged()
{
    m_logFilter = m_logFilterCombo->currentText();
    updateLogs();
}

void DiagnosticsPanel::onTabChanged(int index)
{
    m_currentTab = index;
    // Could implement tab-specific refresh logic here
}

QString DiagnosticsPanel::formatHashrate(double hashrate)
{
    if (hashrate >= 1e12) {
        return QString::number(hashrate / 1e12, 'f', 2) + " TH/s";
    } else if (hashrate >= 1e9) {
        return QString::number(hashrate / 1e9, 'f', 2) + " GH/s";
    } else if (hashrate >= 1e6) {
        return QString::number(hashrate / 1e6, 'f', 2) + " MH/s";
    } else if (hashrate >= 1e3) {
        return QString::number(hashrate / 1e3, 'f', 2) + " KH/s";
    } else {
        return QString::number(hashrate, 'f', 2) + " H/s";
    }
}

QString DiagnosticsPanel::formatTime(int seconds)
{
    if (seconds < 60) {
        return tr("%1 seconds").arg(seconds);
    } else if (seconds < 3600) {
        return tr("%1 minutes").arg(seconds / 60);
    } else if (seconds < 86400) {
        return tr("%1 hours").arg(seconds / 3600);
    } else {
        return tr("%1 days").arg(seconds / 86400);
    }
}

QString DiagnosticsPanel::getLogLevelColor(const QString& level)
{
    if (level.toUpper() == "ERROR") {
        return "#DC143C";
    } else if (level.toUpper() == "WARNING") {
        return "#FF8C00";
    } else if (level.toUpper() == "INFO") {
        return "#2E8B57";
    } else {
        return "#000000";
    }
}
