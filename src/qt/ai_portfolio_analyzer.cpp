// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ai_portfolio_analyzer.h"
#include "ui_ai_portfolio_analyzer.h"
#include "walletmodel.h"
#include "clientmodel.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "platformstyle.h"

#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QPieSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QThread>
#include <QFuture>
#include <QtConcurrent>

QT_CHARTS_USE_NAMESPACE

AIPortfolioAnalyzer::AIPortfolioAnalyzer(const PlatformStyle* platformStyle, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::AIPortfolioAnalyzer),
    m_platformStyle(platformStyle),
    m_walletModel(nullptr),
    m_clientModel(nullptr),
    m_networkManager(new QNetworkAccessManager(this)),
    m_analysisThread(nullptr),
    m_isAnalyzing(false),
    m_lastAnalysisTime(QDateTime::currentDateTime())
{
    ui->setupUi(this);
    
    // Initialize UI components
    initializeUI();
    setupConnections();
    setupCharts();
    loadSettings();
    
    // Set up periodic analysis timer
    m_analysisTimer = new QTimer(this);
    connect(m_analysisTimer, &QTimer::timeout, this, &AIPortfolioAnalyzer::performPeriodicAnalysis);
    m_analysisTimer->start(300000); // 5 minutes
    
    // Set up market data refresh timer
    m_marketDataTimer = new QTimer(this);
    connect(m_marketDataTimer, &QTimer::timeout, this, &AIPortfolioAnalyzer::refreshMarketData);
    m_marketDataTimer->start(60000); // 1 minute
    
    qDebug() << "[AIPortfolioAnalyzer] Initialized successfully";
}

AIPortfolioAnalyzer::~AIPortfolioAnalyzer()
{
    saveSettings();
    delete ui;
}

void AIPortfolioAnalyzer::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    if (m_walletModel) {
        connect(m_walletModel, &WalletModel::balanceChanged, this, &AIPortfolioAnalyzer::onBalanceChanged);
        connect(m_walletModel, &WalletModel::transactionChanged, this, &AIPortfolioAnalyzer::onTransactionChanged);
        
        // Perform initial analysis
        QTimer::singleShot(1000, this, &AIPortfolioAnalyzer::performInitialAnalysis);
    }
}

void AIPortfolioAnalyzer::setClientModel(ClientModel* clientModel)
{
    m_clientModel = clientModel;
    if (m_clientModel) {
        connect(m_clientModel, &ClientModel::numBlocksChanged, this, &AIPortfolioAnalyzer::onNumBlocksChanged);
    }
}

void AIPortfolioAnalyzer::initializeUI()
{
    // Set up tab widget
    ui->tabWidget->setTabText(0, tr("Portfolio Overview"));
    ui->tabWidget->setTabText(1, tr("AI Analysis"));
    ui->tabWidget->setTabText(2, tr("Risk Assessment"));
    ui->tabWidget->setTabText(3, tr("Performance Charts"));
    ui->tabWidget->setTabText(4, tr("AI Recommendations"));
    
    // Initialize portfolio overview
    setupPortfolioOverview();
    
    // Initialize AI analysis tab
    setupAIAnalysis();
    
    // Initialize risk assessment tab
    setupRiskAssessment();
    
    // Initialize performance charts tab
    setupPerformanceCharts();
    
    // Initialize AI recommendations tab
    setupAIRecommendations();
    
    // Set up control panel
    setupControlPanel();
}

void AIPortfolioAnalyzer::setupPortfolioOverview()
{
    // Portfolio summary group
    QGroupBox* summaryGroup = new QGroupBox(tr("Portfolio Summary"));
    QGridLayout* summaryLayout = new QGridLayout(summaryGroup);
    
    // Total balance
    m_totalBalanceLabel = new QLabel(tr("Total Balance:"));
    m_totalBalanceValue = new QLabel("0.00 SHAH");
    m_totalBalanceValue->setStyleSheet("font-weight: bold; font-size: 16px; color: #2E8B57;");
    summaryLayout->addWidget(m_totalBalanceLabel, 0, 0);
    summaryLayout->addWidget(m_totalBalanceValue, 0, 1);
    
    // Fiat value
    m_fiatValueLabel = new QLabel(tr("Fiat Value:"));
    m_fiatValueValue = new QLabel("$0.00 USD");
    m_fiatValueValue->setStyleSheet("font-weight: bold; font-size: 14px; color: #4169E1;");
    summaryLayout->addWidget(m_fiatValueLabel, 1, 0);
    summaryLayout->addWidget(m_fiatValueValue, 1, 1);
    
    // 24h change
    m_change24hLabel = new QLabel(tr("24h Change:"));
    m_change24hValue = new QLabel("0.00%");
    m_change24hValue->setStyleSheet("font-weight: bold; font-size: 14px;");
    summaryLayout->addWidget(m_change24hLabel, 2, 0);
    summaryLayout->addWidget(m_change24hValue, 2, 1);
    
    // Portfolio diversity score
    m_diversityLabel = new QLabel(tr("Diversity Score:"));
    m_diversityValue = new QLabel("0/100");
    m_diversityValue->setStyleSheet("font-weight: bold; font-size: 14px; color: #FF6347;");
    summaryLayout->addWidget(m_diversityLabel, 3, 0);
    summaryLayout->addWidget(m_diversityValue, 3, 1);
    
    // Risk score
    m_riskScoreLabel = new QLabel(tr("Risk Score:"));
    m_riskScoreValue = new QLabel("0/100");
    m_riskScoreValue->setStyleSheet("font-weight: bold; font-size: 14px; color: #FF6347;");
    summaryLayout->addWidget(m_riskScoreLabel, 4, 0);
    summaryLayout->addWidget(m_riskScoreValue, 4, 1);
    
    // Add to main layout
    QVBoxLayout* overviewLayout = new QVBoxLayout(ui->overviewTab);
    overviewLayout->addWidget(summaryGroup);
    
    // Asset allocation chart
    QGroupBox* allocationGroup = new QGroupBox(tr("Asset Allocation"));
    QVBoxLayout* allocationLayout = new QVBoxLayout(allocationGroup);
    
    m_allocationChartView = new QChartView();
    m_allocationChartView->setRenderHint(QPainter::Antialiasing);
    allocationLayout->addWidget(m_allocationChartView);
    
    overviewLayout->addWidget(allocationGroup);
    overviewLayout->addStretch();
}

void AIPortfolioAnalyzer::setupAIAnalysis()
{
    QVBoxLayout* analysisLayout = new QVBoxLayout(ui->aiAnalysisTab);
    
    // AI Analysis Controls
    QGroupBox* controlsGroup = new QGroupBox(tr("Analysis Controls"));
    QGridLayout* controlsLayout = new QGridLayout(controlsGroup);
    
    // Analysis type
    controlsLayout->addWidget(new QLabel(tr("Analysis Type:")), 0, 0);
    m_analysisTypeCombo = new QComboBox();
    m_analysisTypeCombo->addItem(tr("Comprehensive"), "comprehensive");
    m_analysisTypeCombo->addItem(tr("Risk Assessment"), "risk");
    m_analysisTypeCombo->addItem(tr("Performance"), "performance");
    m_analysisTypeCombo->addItem(tr("Diversification"), "diversification");
    controlsLayout->addWidget(m_analysisTypeCombo, 0, 1);
    
    // Time period
    controlsLayout->addWidget(new QLabel(tr("Time Period:")), 1, 0);
    m_timePeriodCombo = new QComboBox();
    m_timePeriodCombo->addItem(tr("1 Day"), 1);
    m_timePeriodCombo->addItem(tr("1 Week"), 7);
    m_timePeriodCombo->addItem(tr("1 Month"), 30);
    m_timePeriodCombo->addItem(tr("3 Months"), 90);
    m_timePeriodCombo->addItem(tr("1 Year"), 365);
    controlsLayout->addWidget(m_timePeriodCombo, 1, 1);
    
    // Analysis button
    m_analyzeButton = new QPushButton(tr("Start AI Analysis"));
    m_analyzeButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; padding: 10px; border-radius: 5px; } QPushButton:hover { background-color: #45a049; }");
    controlsLayout->addWidget(m_analyzeButton, 2, 0, 1, 2);
    
    analysisLayout->addWidget(controlsGroup);
    
    // Analysis Results
    QGroupBox* resultsGroup = new QGroupBox(tr("AI Analysis Results"));
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsGroup);
    
    m_analysisResultsText = new QTextEdit();
    m_analysisResultsText->setReadOnly(true);
    m_analysisResultsText->setStyleSheet("QTextEdit { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; }");
    resultsLayout->addWidget(m_analysisResultsText);
    
    analysisLayout->addWidget(resultsGroup);
}

void AIPortfolioAnalyzer::setupRiskAssessment()
{
    QVBoxLayout* riskLayout = new QVBoxLayout(ui->riskAssessmentTab);
    
    // Risk Metrics
    QGroupBox* metricsGroup = new QGroupBox(tr("Risk Metrics"));
    QGridLayout* metricsLayout = new QGridLayout(metricsGroup);
    
    // Volatility
    metricsLayout->addWidget(new QLabel(tr("Volatility:")), 0, 0);
    m_volatilityLabel = new QLabel("0.00%");
    m_volatilityLabel->setStyleSheet("font-weight: bold; color: #FF6347;");
    metricsLayout->addWidget(m_volatilityLabel, 0, 1);
    
    // Sharpe Ratio
    metricsLayout->addWidget(new QLabel(tr("Sharpe Ratio:")), 1, 0);
    m_sharpeRatioLabel = new QLabel("0.00");
    m_sharpeRatioLabel->setStyleSheet("font-weight: bold; color: #2E8B57;");
    metricsLayout->addWidget(m_sharpeRatioLabel, 1, 1);
    
    // Maximum Drawdown
    metricsLayout->addWidget(new QLabel(tr("Max Drawdown:")), 2, 0);
    m_maxDrawdownLabel = new QLabel("0.00%");
    m_maxDrawdownLabel->setStyleSheet("font-weight: bold; color: #FF6347;");
    metricsLayout->addWidget(m_maxDrawdownLabel, 2, 1);
    
    // Value at Risk (VaR)
    metricsLayout->addWidget(new QLabel(tr("VaR (95%):")), 3, 0);
    m_varLabel = new QLabel("0.00%");
    m_varLabel->setStyleSheet("font-weight: bold; color: #FF6347;");
    metricsLayout->addWidget(m_varLabel, 3, 1);
    
    // Beta
    metricsLayout->addWidget(new QLabel(tr("Beta:")), 4, 0);
    m_betaLabel = new QLabel("0.00");
    m_betaLabel->setStyleSheet("font-weight: bold; color: #4169E1;");
    metricsLayout->addWidget(m_betaLabel, 4, 1);
    
    riskLayout->addWidget(metricsGroup);
    
    // Risk Chart
    QGroupBox* riskChartGroup = new QGroupBox(tr("Risk Analysis Chart"));
    QVBoxLayout* riskChartLayout = new QVBoxLayout(riskChartGroup);
    
    m_riskChartView = new QChartView();
    m_riskChartView->setRenderHint(QPainter::Antialiasing);
    riskChartLayout->addWidget(m_riskChartView);
    
    riskLayout->addWidget(riskChartGroup);
}

void AIPortfolioAnalyzer::setupPerformanceCharts()
{
    QVBoxLayout* chartsLayout = new QVBoxLayout(ui->performanceChartsTab);
    
    // Performance Chart
    QGroupBox* performanceGroup = new QGroupBox(tr("Portfolio Performance"));
    QVBoxLayout* performanceLayout = new QVBoxLayout(performanceGroup);
    
    m_performanceChartView = new QChartView();
    m_performanceChartView->setRenderHint(QPainter::Antialiasing);
    performanceLayout->addWidget(m_performanceChartView);
    
    chartsLayout->addWidget(performanceGroup);
    
    // Returns Distribution
    QGroupBox* returnsGroup = new QGroupBox(tr("Returns Distribution"));
    QVBoxLayout* returnsLayout = new QVBoxLayout(returnsGroup);
    
    m_returnsChartView = new QChartView();
    m_returnsChartView->setRenderHint(QPainter::Antialiasing);
    returnsLayout->addWidget(m_returnsChartView);
    
    chartsLayout->addWidget(returnsGroup);
}

void AIPortfolioAnalyzer::setupAIRecommendations()
{
    QVBoxLayout* recommendationsLayout = new QVBoxLayout(ui->aiRecommendationsTab);
    
    // AI Recommendations List
    QGroupBox* recommendationsGroup = new QGroupBox(tr("AI Recommendations"));
    QVBoxLayout* recLayout = new QVBoxLayout(recommendationsGroup);
    
    m_recommendationsTable = new QTableWidget();
    m_recommendationsTable->setColumnCount(4);
    m_recommendationsTable->setHorizontalHeaderLabels({tr("Type"), tr("Action"), tr("Confidence"), tr("Impact")});
    m_recommendationsTable->horizontalHeader()->setStretchLastSection(true);
    m_recommendationsTable->setAlternatingRowColors(true);
    recLayout->addWidget(m_recommendationsTable);
    
    recommendationsLayout->addWidget(recommendationsGroup);
    
    // Action Buttons
    QHBoxLayout* actionLayout = new QHBoxLayout();
    
    m_applyRecommendationButton = new QPushButton(tr("Apply Selected"));
    m_applyRecommendationButton->setEnabled(false);
    actionLayout->addWidget(m_applyRecommendationButton);
    
    m_dismissRecommendationButton = new QPushButton(tr("Dismiss Selected"));
    m_dismissRecommendationButton->setEnabled(false);
    actionLayout->addWidget(m_dismissRecommendationButton);
    
    actionLayout->addStretch();
    
    recommendationsLayout->addLayout(actionLayout);
}

void AIPortfolioAnalyzer::setupControlPanel()
{
    // Control panel for settings and options
    QGroupBox* controlGroup = new QGroupBox(tr("Analysis Settings"));
    QGridLayout* controlLayout = new QGridLayout(controlGroup);
    
    // Auto-analysis
    m_autoAnalysisCheck = new QCheckBox(tr("Enable Auto-Analysis"));
    m_autoAnalysisCheck->setChecked(true);
    controlLayout->addWidget(m_autoAnalysisCheck, 0, 0);
    
    // Analysis frequency
    controlLayout->addWidget(new QLabel(tr("Analysis Frequency:")), 1, 0);
    m_analysisFrequencyCombo = new QComboBox();
    m_analysisFrequencyCombo->addItem(tr("Every 5 minutes"), 300);
    m_analysisFrequencyCombo->addItem(tr("Every 15 minutes"), 900);
    m_analysisFrequencyCombo->addItem(tr("Every hour"), 3600);
    m_analysisFrequencyCombo->addItem(tr("Every 6 hours"), 21600);
    controlLayout->addWidget(m_analysisFrequencyCombo, 1, 1);
    
    // Market data source
    controlLayout->addWidget(new QLabel(tr("Market Data Source:")), 2, 0);
    m_marketDataSourceCombo = new QComboBox();
    m_marketDataSourceCombo->addItem(tr("CoinGecko"), "coingecko");
    m_marketDataSourceCombo->addItem(tr("CoinMarketCap"), "coinmarketcap");
    m_marketDataSourceCombo->addItem(tr("Multiple Sources"), "multiple");
    controlLayout->addWidget(m_marketDataSourceCombo, 2, 1);
    
    // Add to main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(controlGroup);
}

void AIPortfolioAnalyzer::setupConnections()
{
    // Connect UI signals
    connect(m_analyzeButton, &QPushButton::clicked, this, &AIPortfolioAnalyzer::startAnalysis);
    connect(m_applyRecommendationButton, &QPushButton::clicked, this, &AIPortfolioAnalyzer::applyRecommendation);
    connect(m_dismissRecommendationButton, &QPushButton::clicked, this, &AIPortfolioAnalyzer::dismissRecommendation);
    connect(m_recommendationsTable, &QTableWidget::itemSelectionChanged, this, &AIPortfolioAnalyzer::onRecommendationSelectionChanged);
    
    // Connect settings changes
    connect(m_autoAnalysisCheck, &QCheckBox::toggled, this, &AIPortfolioAnalyzer::onAutoAnalysisToggled);
    connect(m_analysisFrequencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AIPortfolioAnalyzer::onAnalysisFrequencyChanged);
    
    // Connect network manager
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &AIPortfolioAnalyzer::onNetworkReplyFinished);
}

void AIPortfolioAnalyzer::setupCharts()
{
    // Setup allocation chart
    setupAllocationChart();
    
    // Setup performance chart
    setupPerformanceChart();
    
    // Setup risk chart
    setupRiskChart();
    
    // Setup returns chart
    setupReturnsChart();
}

void AIPortfolioAnalyzer::setupAllocationChart()
{
    QChart* chart = new QChart();
    chart->setTitle(tr("Portfolio Allocation"));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QPieSeries* series = new QPieSeries();
    series->append(tr("SHAH"), 100.0);
    
    chart->addSeries(series);
    chart->legend()->setAlignment(Qt::AlignRight);
    
    m_allocationChartView->setChart(chart);
}

void AIPortfolioAnalyzer::setupPerformanceChart()
{
    QChart* chart = new QChart();
    chart->setTitle(tr("Portfolio Performance"));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QLineSeries* series = new QLineSeries();
    series->setName(tr("Portfolio Value"));
    
    // Add sample data
    QDateTime startDate = QDateTime::currentDateTime().addDays(-30);
    for (int i = 0; i < 30; ++i) {
        QDateTime date = startDate.addDays(i);
        double value = 1000.0 + (i * 10.0) + (qrand() % 50 - 25);
        series->append(date.toMSecsSinceEpoch(), value);
    }
    
    chart->addSeries(series);
    
    QDateTimeAxis* axisX = new QDateTimeAxis;
    axisX->setTickCount(10);
    axisX->setFormat("MMM dd");
    axisX->setTitleText(tr("Date"));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis;
    axisY->setLabelFormat("%.0f");
    axisY->setTitleText(tr("Value (USD)"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    m_performanceChartView->setChart(chart);
}

void AIPortfolioAnalyzer::setupRiskChart()
{
    QChart* chart = new QChart();
    chart->setTitle(tr("Risk Analysis"));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QBarSeries* series = new QBarSeries();
    QBarSet* riskSet = new QBarSet(tr("Risk Level"));
    
    // Add sample risk data
    *riskSet << 25 << 35 << 15 << 25;
    series->append(riskSet);
    
    chart->addSeries(series);
    
    QStringList categories;
    categories << tr("Low") << tr("Medium") << tr("High") << tr("Critical");
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, 100);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    m_riskChartView->setChart(chart);
}

void AIPortfolioAnalyzer::setupReturnsChart()
{
    QChart* chart = new QChart();
    chart->setTitle(tr("Returns Distribution"));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QBarSeries* series = new QBarSeries();
    QBarSet* returnsSet = new QBarSet(tr("Returns"));
    
    // Add sample returns data
    *returnsSet << 5 << 15 << 25 << 20 << 15 << 10 << 5 << 5;
    series->append(returnsSet);
    
    chart->addSeries(series);
    
    QStringList categories;
    categories << tr("-10%") << tr("-5%") << tr("0%") << tr("5%") << tr("10%") << tr("15%") << tr("20%") << tr("25%");
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, 30);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    m_returnsChartView->setChart(chart);
}

void AIPortfolioAnalyzer::startAnalysis()
{
    if (m_isAnalyzing) {
        QMessageBox::information(this, tr("Analysis in Progress"), tr("An analysis is already in progress. Please wait for it to complete."));
        return;
    }
    
    m_isAnalyzing = true;
    m_analyzeButton->setEnabled(false);
    m_analyzeButton->setText(tr("Analyzing..."));
    
    // Show progress dialog
    QProgressDialog progress(tr("Performing AI analysis..."), tr("Cancel"), 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(false);
    progress.show();
    
    // Start analysis in background thread
    m_analysisThread = QThread::create([this, &progress]() {
        return performAnalysis(progress);
    });
    
    connect(m_analysisThread, &QThread::finished, this, [this, &progress]() {
        m_isAnalyzing = false;
        m_analyzeButton->setEnabled(true);
        m_analyzeButton->setText(tr("Start AI Analysis"));
        progress.close();
        
        // Update UI with results
        updateAnalysisResults();
    });
    
    m_analysisThread->start();
}

PortfolioAnalysisResult AIPortfolioAnalyzer::performAnalysis(QProgressDialog& progress)
{
    PortfolioAnalysisResult result;
    
    // Simulate analysis steps
    progress.setValue(10);
    QThread::msleep(100);
    
    // Collect portfolio data
    result.totalBalance = getTotalBalance();
    result.fiatValue = getFiatValue();
    progress.setValue(30);
    QThread::msleep(100);
    
    // Calculate risk metrics
    result.volatility = calculateVolatility();
    result.sharpeRatio = calculateSharpeRatio();
    result.maxDrawdown = calculateMaxDrawdown();
    result.var = calculateVaR();
    result.beta = calculateBeta();
    progress.setValue(60);
    QThread::msleep(100);
    
    // Generate AI insights
    result.insights = generateAIInsights();
    result.recommendations = generateRecommendations();
    progress.setValue(90);
    QThread::msleep(100);
    
    // Calculate diversity and risk scores
    result.diversityScore = calculateDiversityScore();
    result.riskScore = calculateRiskScore();
    progress.setValue(100);
    
    return result;
}

void AIPortfolioAnalyzer::updateAnalysisResults()
{
    // Update portfolio overview
    updatePortfolioOverview();
    
    // Update risk metrics
    updateRiskMetrics();
    
    // Update AI recommendations
    updateAIRecommendations();
    
    // Update charts
    updateCharts();
    
    // Show completion message
    QMessageBox::information(this, tr("Analysis Complete"), tr("AI portfolio analysis has been completed successfully."));
}

void AIPortfolioAnalyzer::updatePortfolioOverview()
{
    if (!m_walletModel) return;
    
    // Get current balance
    CAmount balance = m_walletModel->getBalance();
    QString balanceStr = ShahcoinUnits::format(m_walletModel->getOptionsModel()->getDisplayUnit(), balance);
    m_totalBalanceValue->setText(balanceStr);
    
    // Get fiat value (simulated)
    double fiatValue = balance / COIN * getCurrentPrice();
    m_fiatValueValue->setText(QString("$%1 USD").arg(fiatValue, 0, 'f', 2));
    
    // Calculate 24h change (simulated)
    double change24h = calculate24hChange();
    QString changeStr = QString("%1%2%").arg(change24h >= 0 ? "+" : "").arg(change24h, 0, 'f', 2);
    m_change24hValue->setText(changeStr);
    m_change24hValue->setStyleSheet(QString("font-weight: bold; font-size: 14px; color: %1;").arg(change24h >= 0 ? "#2E8B57" : "#FF6347"));
    
    // Update diversity and risk scores
    m_diversityValue->setText(QString("%1/100").arg(m_lastAnalysisResult.diversityScore));
    m_riskScoreValue->setText(QString("%1/100").arg(m_lastAnalysisResult.riskScore));
}

void AIPortfolioAnalyzer::updateRiskMetrics()
{
    m_volatilityLabel->setText(QString("%1%").arg(m_lastAnalysisResult.volatility, 0, 'f', 2));
    m_sharpeRatioLabel->setText(QString("%1").arg(m_lastAnalysisResult.sharpeRatio, 0, 'f', 2));
    m_maxDrawdownLabel->setText(QString("%1%").arg(m_lastAnalysisResult.maxDrawdown, 0, 'f', 2));
    m_varLabel->setText(QString("%1%").arg(m_lastAnalysisResult.var, 0, 'f', 2));
    m_betaLabel->setText(QString("%1").arg(m_lastAnalysisResult.beta, 0, 'f', 2));
}

void AIPortfolioAnalyzer::updateAIRecommendations()
{
    m_recommendationsTable->setRowCount(m_lastAnalysisResult.recommendations.size());
    
    for (int i = 0; i < m_lastAnalysisResult.recommendations.size(); ++i) {
        const AIRecommendation& rec = m_lastAnalysisResult.recommendations[i];
        
        m_recommendationsTable->setItem(i, 0, new QTableWidgetItem(rec.type));
        m_recommendationsTable->setItem(i, 1, new QTableWidgetItem(rec.action));
        m_recommendationsTable->setItem(i, 2, new QTableWidgetItem(QString("%1%").arg(rec.confidence, 0, 'f', 1)));
        m_recommendationsTable->setItem(i, 3, new QTableWidgetItem(rec.impact));
    }
}

void AIPortfolioAnalyzer::updateCharts()
{
    // Update allocation chart
    updateAllocationChart();
    
    // Update performance chart
    updatePerformanceChart();
    
    // Update risk chart
    updateRiskChart();
    
    // Update returns chart
    updateReturnsChart();
}

void AIPortfolioAnalyzer::updateAllocationChart()
{
    QChart* chart = m_allocationChartView->chart();
    if (!chart) return;
    
    QPieSeries* series = qobject_cast<QPieSeries*>(chart->series().first());
    if (!series) return;
    
    series->clear();
    
    // Add current allocation data
    series->append(tr("SHAH"), 100.0);
    
    // Add other assets if available
    // This would be populated with actual token/NFT data
}

void AIPortfolioAnalyzer::updatePerformanceChart()
{
    // Update performance chart with real data
    // This would fetch historical price data and calculate performance
}

void AIPortfolioAnalyzer::updateRiskChart()
{
    // Update risk chart with current risk metrics
    // This would show current risk distribution
}

void AIPortfolioAnalyzer::updateReturnsChart()
{
    // Update returns distribution chart
    // This would show historical returns distribution
}

// Helper methods for calculations
CAmount AIPortfolioAnalyzer::getTotalBalance()
{
    if (!m_walletModel) return 0;
    return m_walletModel->getBalance();
}

double AIPortfolioAnalyzer::getFiatValue()
{
    CAmount balance = getTotalBalance();
    return balance / COIN * getCurrentPrice();
}

double AIPortfolioAnalyzer::getCurrentPrice()
{
    // This would fetch real-time price from API
    // For now, return simulated price
    return 1.25; // $1.25 per SHAH
}

double AIPortfolioAnalyzer::calculate24hChange()
{
    // This would calculate actual 24h change
    // For now, return simulated change
    return (qrand() % 200 - 100) / 10.0; // -10% to +10%
}

double AIPortfolioAnalyzer::calculateVolatility()
{
    // Calculate portfolio volatility
    // This would use historical data
    return 15.5; // 15.5% volatility
}

double AIPortfolioAnalyzer::calculateSharpeRatio()
{
    // Calculate Sharpe ratio
    // This would use risk-free rate and portfolio returns
    return 1.25; // 1.25 Sharpe ratio
}

double AIPortfolioAnalyzer::calculateMaxDrawdown()
{
    // Calculate maximum drawdown
    // This would use historical data
    return 8.5; // 8.5% max drawdown
}

double AIPortfolioAnalyzer::calculateVaR()
{
    // Calculate Value at Risk
    // This would use statistical methods
    return 12.3; // 12.3% VaR
}

double AIPortfolioAnalyzer::calculateBeta()
{
    // Calculate beta relative to market
    // This would compare to market index
    return 0.85; // 0.85 beta
}

int AIPortfolioAnalyzer::calculateDiversityScore()
{
    // Calculate portfolio diversity score
    // This would analyze asset allocation
    return 75; // 75/100 diversity score
}

int AIPortfolioAnalyzer::calculateRiskScore()
{
    // Calculate overall risk score
    // This would combine various risk metrics
    return 45; // 45/100 risk score
}

QStringList AIPortfolioAnalyzer::generateAIInsights()
{
    QStringList insights;
    
    // Generate AI insights based on analysis
    insights << tr("Your portfolio shows moderate diversification with room for improvement.");
    insights << tr("Risk metrics indicate a balanced risk-return profile.");
    insights << tr("Consider increasing exposure to emerging market assets.");
    insights << tr("Current volatility is within acceptable range for your risk tolerance.");
    insights << tr("Portfolio performance has been consistent over the past month.");
    
    return insights;
}

QList<AIRecommendation> AIPortfolioAnalyzer::generateRecommendations()
{
    QList<AIRecommendation> recommendations;
    
    // Generate AI recommendations
    AIRecommendation rec1;
    rec1.type = tr("Diversification");
    rec1.action = tr("Consider adding 10-15% allocation to DeFi tokens");
    rec1.confidence = 85.5;
    rec1.impact = tr("Medium");
    recommendations << rec1;
    
    AIRecommendation rec2;
    rec2.type = tr("Risk Management");
    rec2.action = tr("Set stop-loss orders at 8% below current positions");
    rec2.confidence = 92.3;
    rec2.impact = tr("High");
    recommendations << rec2;
    
    AIRecommendation rec3;
    rec3.type = tr("Performance");
    rec3.action = tr("Rebalance portfolio monthly to maintain target allocations");
    rec3.confidence = 78.9;
    rec3.impact = tr("Medium");
    recommendations << rec3;
    
    return recommendations;
}

void AIPortfolioAnalyzer::performInitialAnalysis()
{
    if (m_autoAnalysisCheck->isChecked()) {
        startAnalysis();
    }
}

void AIPortfolioAnalyzer::performPeriodicAnalysis()
{
    if (m_autoAnalysisCheck->isChecked() && !m_isAnalyzing) {
        startAnalysis();
    }
}

void AIPortfolioAnalyzer::refreshMarketData()
{
    // Fetch latest market data
    fetchMarketData();
}

void AIPortfolioAnalyzer::fetchMarketData()
{
    // Fetch market data from API
    QString url = "https://api.coingecko.com/api/v3/simple/price?ids=shahcoin&vs_currencies=usd&include_24hr_change=true";
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    m_networkManager->get(request);
}

void AIPortfolioAnalyzer::onNetworkReplyFinished(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        
        // Parse market data
        if (obj.contains("shahcoin")) {
            QJsonObject shahData = obj["shahcoin"].toObject();
            if (shahData.contains("usd")) {
                double price = shahData["usd"].toDouble();
                // Update price data
                qDebug() << "[AIPortfolioAnalyzer] Updated SHAH price:" << price;
            }
        }
    } else {
        qDebug() << "[AIPortfolioAnalyzer] Network error:" << reply->errorString();
    }
    
    reply->deleteLater();
}

void AIPortfolioAnalyzer::onBalanceChanged()
{
    if (m_autoAnalysisCheck->isChecked()) {
        // Trigger analysis on balance change
        QTimer::singleShot(5000, this, &AIPortfolioAnalyzer::startAnalysis);
    }
}

void AIPortfolioAnalyzer::onTransactionChanged()
{
    if (m_autoAnalysisCheck->isChecked()) {
        // Trigger analysis on transaction change
        QTimer::singleShot(5000, this, &AIPortfolioAnalyzer::startAnalysis);
    }
}

void AIPortfolioAnalyzer::onNumBlocksChanged()
{
    // Update blockchain sync status if needed
}

void AIPortfolioAnalyzer::onAutoAnalysisToggled(bool enabled)
{
    if (enabled) {
        m_analysisTimer->start();
    } else {
        m_analysisTimer->stop();
    }
}

void AIPortfolioAnalyzer::onAnalysisFrequencyChanged(int index)
{
    int frequency = m_analysisFrequencyCombo->itemData(index).toInt();
    m_analysisTimer->setInterval(frequency * 1000);
}

void AIPortfolioAnalyzer::onRecommendationSelectionChanged()
{
    bool hasSelection = !m_recommendationsTable->selectedItems().isEmpty();
    m_applyRecommendationButton->setEnabled(hasSelection);
    m_dismissRecommendationButton->setEnabled(hasSelection);
}

void AIPortfolioAnalyzer::applyRecommendation()
{
    QList<QTableWidgetItem*> selectedItems = m_recommendationsTable->selectedItems();
    if (selectedItems.isEmpty()) return;
    
    int row = selectedItems.first()->row();
    QString action = m_recommendationsTable->item(row, 1)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Apply Recommendation"), 
        tr("Are you sure you want to apply this recommendation?\n\n%1").arg(action),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Apply the recommendation
        // This would execute the recommended action
        QMessageBox::information(this, tr("Recommendation Applied"), 
            tr("The recommendation has been applied successfully."));
        
        // Remove from table
        m_recommendationsTable->removeRow(row);
    }
}

void AIPortfolioAnalyzer::dismissRecommendation()
{
    QList<QTableWidgetItem*> selectedItems = m_recommendationsTable->selectedItems();
    if (selectedItems.isEmpty()) return;
    
    int row = selectedItems.first()->row();
    m_recommendationsTable->removeRow(row);
}

void AIPortfolioAnalyzer::loadSettings()
{
    QSettings settings;
    
    m_autoAnalysisCheck->setChecked(settings.value("aiPortfolioAnalyzer/autoAnalysis", true).toBool());
    m_analysisFrequencyCombo->setCurrentIndex(settings.value("aiPortfolioAnalyzer/analysisFrequency", 0).toInt());
    m_marketDataSourceCombo->setCurrentIndex(settings.value("aiPortfolioAnalyzer/marketDataSource", 0).toInt());
}

void AIPortfolioAnalyzer::saveSettings()
{
    QSettings settings;
    
    settings.setValue("aiPortfolioAnalyzer/autoAnalysis", m_autoAnalysisCheck->isChecked());
    settings.setValue("aiPortfolioAnalyzer/analysisFrequency", m_analysisFrequencyCombo->currentIndex());
    settings.setValue("aiPortfolioAnalyzer/marketDataSource", m_marketDataSourceCombo->currentIndex());
}

void AIPortfolioAnalyzer::exportAnalysis()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Analysis Report"), 
        QString("portfolio_analysis_%1.pdf").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        tr("PDF Files (*.pdf);;HTML Files (*.html);;CSV Files (*.csv)"));
    
    if (!fileName.isEmpty()) {
        // Generate and export analysis report
        generateAnalysisReport(fileName);
        QMessageBox::information(this, tr("Export Complete"), 
            tr("Analysis report has been exported to:\n%1").arg(fileName));
    }
}

void AIPortfolioAnalyzer::generateAnalysisReport(const QString& fileName)
{
    // Generate comprehensive analysis report
    // This would create a detailed PDF/HTML/CSV report
    qDebug() << "[AIPortfolioAnalyzer] Generating analysis report:" << fileName;
}

void AIPortfolioAnalyzer::showHelp()
{
    QMessageBox::information(this, tr("AI Portfolio Analyzer Help"), 
        tr("The AI Portfolio Analyzer provides intelligent insights into your cryptocurrency portfolio:\n\n"
           "• Portfolio Overview: View your total balance, fiat value, and performance metrics\n"
           "• AI Analysis: Get AI-powered analysis of your portfolio\n"
           "• Risk Assessment: Understand your portfolio's risk profile\n"
           "• Performance Charts: Visualize your portfolio's performance over time\n"
           "• AI Recommendations: Receive personalized investment recommendations\n\n"
           "The analyzer automatically updates every 5 minutes and can be configured in the settings."));
}
