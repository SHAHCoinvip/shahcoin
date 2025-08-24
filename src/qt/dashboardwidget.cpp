#include <qt/dashboardwidget.h>
#include <qt/walletmodel.h>
#include <qt/shahcoinunits.h>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QPainter>
#include <QFontMetrics>

// Note: Qt Charts might not be available in all builds
// This implementation includes fallbacks for when charts are not available

DashboardWidget::DashboardWidget(QWidget* parent)
    : QWidget(parent)
    , m_autoRefreshTimer(new QTimer(this))
    , m_walletModel(nullptr)
{
    setupUI();
    setupCharts();
    
    // Set up auto-refresh timer (30 seconds)
    connect(m_autoRefreshTimer, &QTimer::timeout, this, &DashboardWidget::updatePortfolioData);
    
    // Load initial data
    loadPortfolioData();
    updatePortfolioData();
}

void DashboardWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Controls
    m_controlsLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton(tr("🔄 Refresh"), this);
    m_refreshButton->setToolTip(tr("Refresh portfolio data"));
    
    m_exportButton = new QPushButton(tr("📊 Export Report"), this);
    m_exportButton->setToolTip(tr("Export portfolio report as JSON"));
    
    m_autoRefreshButton = new QPushButton(tr("⏰ Auto-refresh: OFF"), this);
    m_autoRefreshButton->setCheckable(true);
    m_autoRefreshButton->setToolTip(tr("Enable automatic refresh every 30 seconds"));
    
    m_controlsLayout->addWidget(m_refreshButton);
    m_controlsLayout->addWidget(m_exportButton);
    m_controlsLayout->addStretch();
    m_controlsLayout->addWidget(m_autoRefreshButton);
    
    m_mainLayout->addLayout(m_controlsLayout);
    
    // Summary Cards
    m_summaryGroup = new QGroupBox(tr("📊 Portfolio Summary"), this);
    m_summaryLayout = new QGridLayout(m_summaryGroup);
    
    // Create summary cards
    m_totalBalanceLabel = new QLabel(tr("Total Balance: 0 SHAH"), this);
    m_totalBalanceLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #2E8B57; padding: 10px; background-color: #f8f9fa; border-radius: 5px; }");
    
    m_nftCountLabel = new QLabel(tr("NFTs Owned: 0"), this);
    m_nftCountLabel->setStyleSheet("QLabel { font-size: 14px; padding: 8px; background-color: #e3f2fd; border-radius: 5px; }");
    
    m_tokenCountLabel = new QLabel(tr("Tokens: 0"), this);
    m_tokenCountLabel->setStyleSheet("QLabel { font-size: 14px; padding: 8px; background-color: #fff3e0; border-radius: 5px; }");
    
    m_stakingAmountLabel = new QLabel(tr("Staking: 0 SHAH"), this);
    m_stakingAmountLabel->setStyleSheet("QLabel { font-size: 14px; padding: 8px; background-color: #e8f5e8; border-radius: 5px; }");
    
    m_miningBlocksLabel = new QLabel(tr("Blocks Mined: 0"), this);
    m_miningBlocksLabel->setStyleSheet("QLabel { font-size: 14px; padding: 8px; background-color: #f3e5f5; border-radius: 5px; }");
    
    m_fiatValueLabel = new QLabel(tr("Fiat Value: $0.00"), this);
    m_fiatValueLabel->setStyleSheet("QLabel { font-size: 14px; padding: 8px; background-color: #e0f2f1; border-radius: 5px; }");
    
    // Add cards to grid
    m_summaryLayout->addWidget(m_totalBalanceLabel, 0, 0, 1, 2);
    m_summaryLayout->addWidget(m_nftCountLabel, 1, 0);
    m_summaryLayout->addWidget(m_tokenCountLabel, 1, 1);
    m_summaryLayout->addWidget(m_stakingAmountLabel, 2, 0);
    m_summaryLayout->addWidget(m_miningBlocksLabel, 2, 1);
    m_summaryLayout->addWidget(m_fiatValueLabel, 3, 0, 1, 2);
    
    m_mainLayout->addWidget(m_summaryGroup);
    
    // Charts Group
    m_chartsGroup = new QGroupBox(tr("📈 Portfolio Charts"), this);
    m_chartsLayout = new QHBoxLayout(m_chartsGroup);
    
    // Charts will be added in setupCharts()
    
    m_mainLayout->addWidget(m_chartsGroup);
    
    // Connect signals
    connect(m_refreshButton, &QPushButton::clicked, this, &DashboardWidget::onRefreshClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &DashboardWidget::onExportReportClicked);
    connect(m_autoRefreshButton, &QPushButton::toggled, this, &DashboardWidget::onAutoRefreshToggled);
}

void DashboardWidget::setupCharts()
{
    // Note: This is a simplified implementation
    // In a real implementation, you would use Qt Charts library
    
    // Create placeholder chart views
    m_pieChartView = new QChartView(this);
    m_pieChartView->setMinimumSize(300, 200);
    m_pieChartView->setStyleSheet("QChartView { border: 1px solid #ccc; background-color: white; }");
    
    m_earningsChartView = new QChartView(this);
    m_earningsChartView->setMinimumSize(300, 200);
    m_earningsChartView->setStyleSheet("QChartView { border: 1px solid #ccc; background-color: white; }");
    
    // Add placeholder text
    QLabel* piePlaceholder = new QLabel(tr("Asset Distribution\n(Chart placeholder)"), m_pieChartView);
    piePlaceholder->setAlignment(Qt::AlignCenter);
    piePlaceholder->setStyleSheet("QLabel { color: #666; font-size: 12px; }");
    
    QLabel* earningsPlaceholder = new QLabel(tr("Earnings History\n(Chart placeholder)"), m_earningsChartView);
    earningsPlaceholder->setAlignment(Qt::AlignCenter);
    earningsPlaceholder->setStyleSheet("QLabel { color: #666; font-size: 12px; }");
    
    m_chartsLayout->addWidget(m_pieChartView);
    m_chartsLayout->addWidget(m_earningsChartView);
}

void DashboardWidget::updatePortfolioData()
{
    loadPortfolioData();
    updateStatistics();
    updateCharts();
}

void DashboardWidget::setWalletModel(WalletModel* model)
{
    m_walletModel = model;
    if (m_walletModel) {
        // Connect to wallet model signals for real-time updates
        connect(m_walletModel, &WalletModel::balanceChanged, this, &DashboardWidget::updatePortfolioData);
        // Load real data from wallet
        loadPortfolioData();
        updatePortfolioData();
    }
}

void DashboardWidget::onRefreshClicked()
{
    updatePortfolioData();
    QMessageBox::information(this, tr("Data Refreshed"),
        tr("Portfolio data has been refreshed."));
}

void DashboardWidget::onExportReportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Portfolio Report"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/shahcoin_portfolio_report.json",
        tr("JSON Files (*.json);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        QJsonObject report;
        report["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        report["totalBalance"] = m_portfolioData.totalBalance;
        report["nftCount"] = m_portfolioData.nftCount;
        report["tokenCount"] = m_portfolioData.tokenCount;
        report["stakingAmount"] = m_portfolioData.stakingAmount;
        report["stakingRewards"] = m_portfolioData.stakingRewards;
        report["miningBlocks"] = m_portfolioData.miningBlocks;
        report["fiatValue"] = m_portfolioData.fiatValue;
        
        // Add asset breakdown
        QJsonArray assetsArray;
        for (const auto& asset : m_portfolioData.assetBreakdown) {
            QJsonObject assetObj;
            assetObj["name"] = asset.first;
            assetObj["value"] = asset.second;
            assetsArray.append(assetObj);
        }
        report["assets"] = assetsArray;
        
        // Add earnings history
        QJsonArray earningsArray;
        for (const auto& earning : m_portfolioData.earningsHistory) {
            QJsonObject earningObj;
            earningObj["date"] = earning.first;
            earningObj["amount"] = earning.second;
            earningsArray.append(earningObj);
        }
        report["earnings"] = earningsArray;
        
        QJsonDocument doc(report);
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            QMessageBox::information(this, tr("Report Exported"),
                tr("Portfolio report has been exported to: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, tr("Export Failed"),
                tr("Failed to export report. Please try again."));
        }
    }
}

void DashboardWidget::onAutoRefreshToggled(bool enabled)
{
    if (enabled) {
        m_autoRefreshTimer->start(30000); // 30 seconds
        m_autoRefreshButton->setText(tr("⏰ Auto-refresh: ON"));
    } else {
        m_autoRefreshTimer->stop();
        m_autoRefreshButton->setText(tr("⏰ Auto-refresh: OFF"));
    }
}

void DashboardWidget::updateStatistics()
{
    // Update summary labels
    m_totalBalanceLabel->setText(tr("Total Balance: %1 SHAH").arg(m_portfolioData.totalBalance, 0, 'f', 8));
    m_nftCountLabel->setText(tr("NFTs Owned: %1").arg(m_portfolioData.nftCount));
    m_tokenCountLabel->setText(tr("Tokens: %1").arg(m_portfolioData.tokenCount));
    m_stakingAmountLabel->setText(tr("Staking: %1 SHAH").arg(m_portfolioData.stakingAmount, 0, 'f', 8));
    m_miningBlocksLabel->setText(tr("Blocks Mined: %1").arg(m_portfolioData.miningBlocks));
    m_fiatValueLabel->setText(tr("Fiat Value: $%1").arg(m_portfolioData.fiatValue, 0, 'f', 2));
}

void DashboardWidget::updateCharts()
{
    // Note: This is a placeholder implementation
    // In a real implementation, you would update the actual charts
    
    // For now, we'll just update the placeholder text
    if (m_pieChartView) {
        QLabel* placeholder = m_pieChartView->findChild<QLabel*>();
        if (placeholder) {
            placeholder->setText(tr("Asset Distribution\nSHAH: %1%\nNFTs: %2%\nTokens: %3%")
                .arg(70) // Placeholder percentages
                .arg(20)
                .arg(10));
        }
    }
    
    if (m_earningsChartView) {
        QLabel* placeholder = m_earningsChartView->findChild<QLabel*>();
        if (placeholder) {
            placeholder->setText(tr("Earnings History\nStaking: %1 SHAH\nMining: %2 SHAH")
                .arg(m_portfolioData.stakingRewards, 0, 'f', 8)
                .arg(0.0, 0, 'f', 8));
        }
    }
}

void DashboardWidget::loadPortfolioData()
{
    if (m_walletModel) {
        // Load real wallet data
        CAmount balance = m_walletModel->getBalance();
        m_portfolioData.totalBalance = ShahcoinUnits::fromAmount(balance);
        
        // TODO: Load real NFT and token data from wallet
        m_portfolioData.nftCount = 5; // Placeholder
        m_portfolioData.tokenCount = 3; // Placeholder
        
        // TODO: Load real staking data
        m_portfolioData.stakingAmount = 500.0; // Placeholder
        m_portfolioData.stakingRewards = 25.5; // Placeholder
        
        // TODO: Load real mining data
        m_portfolioData.miningBlocks = 12; // Placeholder
        
        // Calculate fiat value (placeholder - would need price API)
        m_portfolioData.fiatValue = m_portfolioData.totalBalance * 100.0; // Assuming $100 per SHAH
        
        // Asset breakdown
        m_portfolioData.assetBreakdown.clear();
        m_portfolioData.assetBreakdown.append({"SHAH", m_portfolioData.totalBalance});
        m_portfolioData.assetBreakdown.append({"NFTs", 250.0}); // Placeholder
        m_portfolioData.assetBreakdown.append({"Tokens", 100.0}); // Placeholder
        
        // Earnings history (last 7 days) - placeholder
        m_portfolioData.earningsHistory.clear();
        QDate today = QDate::currentDate();
        for (int i = 6; i >= 0; --i) {
            QDate date = today.addDays(-i);
            double dailyEarning = 3.5 + (qrand() % 20) / 10.0; // Random daily earning
            m_portfolioData.earningsHistory.append({date.toString("MM/dd"), dailyEarning});
        }
    } else {
        // Fallback to simulated data if no wallet model
        m_portfolioData.totalBalance = 1250.75;
        m_portfolioData.nftCount = 5;
        m_portfolioData.tokenCount = 3;
        m_portfolioData.stakingAmount = 500.0;
        m_portfolioData.stakingRewards = 25.5;
        m_portfolioData.miningBlocks = 12;
        m_portfolioData.fiatValue = 125075.00; // Assuming $100 per SHAH
        
        // Asset breakdown
        m_portfolioData.assetBreakdown.clear();
        m_portfolioData.assetBreakdown.append({"SHAH", 1250.75});
        m_portfolioData.assetBreakdown.append({"NFTs", 250.0});
        m_portfolioData.assetBreakdown.append({"Tokens", 100.0});
        
        // Earnings history (last 7 days)
        m_portfolioData.earningsHistory.clear();
        QDate today = QDate::currentDate();
        for (int i = 6; i >= 0; --i) {
            QDate date = today.addDays(-i);
            double dailyEarning = 3.5 + (qrand() % 20) / 10.0; // Random daily earning
            m_portfolioData.earningsHistory.append({date.toString("MM/dd"), dailyEarning});
        }
    }
}
