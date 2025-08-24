#include "autoswapengine.h"
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
#include <QTextBrowser>
#include <QScrollArea>

// Constants
const QString AutoSwapEngine::SETTINGS_GROUP = "AutoSwapEngine";
const QString AutoSwapEngine::SETTINGS_STRATEGIES = "Strategies";
const QString AutoSwapEngine::SETTINGS_SWAP_HISTORY = "SwapHistory";
const QString AutoSwapEngine::SETTINGS_AUTO_EXECUTE = "AutoExecute";
const QString AutoSwapEngine::SETTINGS_NOTIFY_ON_SWAP = "NotifyOnSwap";
const QString AutoSwapEngine::SETTINGS_STOP_ON_ERROR = "StopOnError";
const QString AutoSwapEngine::SETTINGS_MAX_CONCURRENT_SWAPS = "MaxConcurrentSwaps";
const QString AutoSwapEngine::SETTINGS_MAX_SLIPPAGE = "MaxSlippage";

AutoSwapEngine::AutoSwapEngine(WalletModel* walletModel, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(walletModel)
    , m_platformStyle(nullptr)
    , m_mainLayout(nullptr)
    , m_overviewGroup(nullptr)
    , m_overviewLayout(nullptr)
    , m_activeStrategiesLabel(nullptr)
    , m_totalSwapsLabel(nullptr)
    , m_totalVolumeLabel(nullptr)
    , m_totalFeesLabel(nullptr)
    , m_createStrategyButton(nullptr)
    , m_strategiesGroup(nullptr)
    , m_strategiesLayout(nullptr)
    , m_strategiesTable(nullptr)
    , m_pauseStrategyButton(nullptr)
    , m_resumeStrategyButton(nullptr)
    , m_cancelStrategyButton(nullptr)
    , m_editStrategyButton(nullptr)
    , m_duplicateStrategyButton(nullptr)
    , m_exportStrategyButton(nullptr)
    , m_historyGroup(nullptr)
    , m_historyLayout(nullptr)
    , m_swapHistoryTable(nullptr)
    , m_exportHistoryButton(nullptr)
    , m_clearHistoryButton(nullptr)
    , m_analyticsGroup(nullptr)
    , m_analyticsLayout(nullptr)
    , m_performanceChartView(nullptr)
    , m_performanceChart(nullptr)
    , m_swapSeries(nullptr)
    , m_priceSeries(nullptr)
    , m_valueAxis(nullptr)
    , m_timeAxis(nullptr)
    , m_showAnalyticsButton(nullptr)
    , m_settingsGroup(nullptr)
    , m_settingsLayout(nullptr)
    , m_autoExecuteCheck(nullptr)
    , m_notifyOnSwapCheck(nullptr)
    , m_stopOnErrorCheck(nullptr)
    , m_maxConcurrentSwapsSpin(nullptr)
    , m_maxSlippageSpin(nullptr)
    , m_saveSettingsButton(nullptr)
    , m_autoExecuteEnabled(false)
    , m_notifyOnSwap(true)
    , m_stopOnError(true)
    , m_maxConcurrentSwaps(3)
    , m_maxSlippage(2.0)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_updateTimer(new QTimer(this))
    , m_executionTimer(new QTimer(this))
    , m_animationTimer(new QTimer(this))
    , m_settings(new QSettings("Shahcoin", "AutoSwapEngine"))
    , m_strategyAnimation(new QPropertyAnimation(this))
    , m_strategyOpacityEffect(new QGraphicsOpacityEffect(this))
{
    setWindowTitle(tr("🔄 Auto Swap / DCA Engine"));
    setupUI();
    connectSignals();
    applyTheme();
    loadSettings();
    
    // Start timers
    m_updateTimer->setInterval(UPDATE_INTERVAL_MS);
    m_executionTimer->setInterval(EXECUTION_INTERVAL_MS);
    m_updateTimer->start();
    m_executionTimer->start();
}

AutoSwapEngine::~AutoSwapEngine()
{
    saveSettings();
}

void AutoSwapEngine::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Strategy Overview Section
    m_overviewGroup = new QGroupBox(tr("📊 Strategy Overview"), this);
    m_overviewLayout = new QHBoxLayout(m_overviewGroup);

    m_activeStrategiesLabel = new QLabel(tr("Active Strategies: 0"), this);
    m_totalSwapsLabel = new QLabel(tr("Total Swaps: 0"), this);
    m_totalVolumeLabel = new QLabel(tr("Total Volume: 0 SHAH"), this);
    m_totalFeesLabel = new QLabel(tr("Total Fees: 0 SHAH"), this);
    m_createStrategyButton = new QPushButton(tr("➕ Create Strategy"), this);

    m_overviewLayout->addWidget(m_activeStrategiesLabel);
    m_overviewLayout->addWidget(m_totalSwapsLabel);
    m_overviewLayout->addWidget(m_totalVolumeLabel);
    m_overviewLayout->addWidget(m_totalFeesLabel);
    m_overviewLayout->addWidget(m_createStrategyButton);
    m_overviewLayout->addStretch();

    // Strategy List Section
    m_strategiesGroup = new QGroupBox(tr("📋 Active Strategies"), this);
    m_strategiesLayout = new QVBoxLayout(m_strategiesGroup);

    m_strategiesTable = new QTableWidget(this);
    m_strategiesTable->setColumnCount(8);
    m_strategiesTable->setHorizontalHeaderLabels({
        tr("Name"), tr("From"), tr("To"), tr("Amount"), tr("Frequency"), tr("Status"), tr("Next Swap"), tr("Actions")
    });
    m_strategiesTable->horizontalHeader()->setStretchLastSection(true);
    m_strategiesTable->setMaximumHeight(200);

    QHBoxLayout* strategyButtonsLayout = new QHBoxLayout();
    m_pauseStrategyButton = new QPushButton(tr("⏸️ Pause"), this);
    m_resumeStrategyButton = new QPushButton(tr("▶️ Resume"), this);
    m_cancelStrategyButton = new QPushButton(tr("❌ Cancel"), this);
    m_editStrategyButton = new QPushButton(tr("✏️ Edit"), this);
    m_duplicateStrategyButton = new QPushButton(tr("📋 Duplicate"), this);
    m_exportStrategyButton = new QPushButton(tr("📤 Export"), this);

    strategyButtonsLayout->addWidget(m_pauseStrategyButton);
    strategyButtonsLayout->addWidget(m_resumeStrategyButton);
    strategyButtonsLayout->addWidget(m_cancelStrategyButton);
    strategyButtonsLayout->addWidget(m_editStrategyButton);
    strategyButtonsLayout->addWidget(m_duplicateStrategyButton);
    strategyButtonsLayout->addWidget(m_exportStrategyButton);
    strategyButtonsLayout->addStretch();

    m_strategiesLayout->addWidget(m_strategiesTable);
    m_strategiesLayout->addLayout(strategyButtonsLayout);

    // Swap History Section
    m_historyGroup = new QGroupBox(tr("📈 Swap History"), this);
    m_historyLayout = new QVBoxLayout(m_historyGroup);

    m_swapHistoryTable = new QTableWidget(this);
    m_swapHistoryTable->setColumnCount(7);
    m_swapHistoryTable->setHorizontalHeaderLabels({
        tr("Date"), tr("From"), tr("To"), tr("Amount"), tr("Price"), tr("Fees"), tr("Status")
    });
    m_swapHistoryTable->horizontalHeader()->setStretchLastSection(true);
    m_swapHistoryTable->setMaximumHeight(150);

    QHBoxLayout* historyButtonsLayout = new QHBoxLayout();
    m_exportHistoryButton = new QPushButton(tr("📤 Export History"), this);
    m_clearHistoryButton = new QPushButton(tr("🗑️ Clear History"), this);
    m_showAnalyticsButton = new QPushButton(tr("📊 Show Analytics"), this);

    historyButtonsLayout->addWidget(m_exportHistoryButton);
    historyButtonsLayout->addWidget(m_clearHistoryButton);
    historyButtonsLayout->addWidget(m_showAnalyticsButton);
    historyButtonsLayout->addStretch();

    m_historyLayout->addWidget(m_swapHistoryTable);
    m_historyLayout->addLayout(historyButtonsLayout);

    // Performance Analytics Section
    m_analyticsGroup = new QGroupBox(tr("📊 Performance Analytics"), this);
    m_analyticsLayout = new QVBoxLayout(m_analyticsGroup);

    m_performanceChartView = new QChartView(this);
    m_performanceChartView->setMinimumHeight(200);
    m_analyticsLayout->addWidget(m_performanceChartView);

    // Settings Panel
    m_settingsGroup = new QGroupBox(tr("⚙️ Settings"), this);
    m_settingsLayout = new QVBoxLayout(m_settingsGroup);

    m_autoExecuteCheck = new QCheckBox(tr("Auto-execute strategies"), this);
    m_notifyOnSwapCheck = new QCheckBox(tr("Notify on swap completion"), this);
    m_notifyOnSwapCheck->setChecked(true);
    m_stopOnErrorCheck = new QCheckBox(tr("Stop on error"), this);
    m_stopOnErrorCheck->setChecked(true);

    QHBoxLayout* settingsAdvancedLayout = new QHBoxLayout();
    QLabel* maxSwapsLabel = new QLabel(tr("Max concurrent swaps:"), this);
    m_maxConcurrentSwapsSpin = new QSpinBox(this);
    m_maxConcurrentSwapsSpin->setRange(1, 10);
    m_maxConcurrentSwapsSpin->setValue(3);
    
    QLabel* maxSlippageLabel = new QLabel(tr("Max slippage (%):"), this);
    m_maxSlippageSpin = new QDoubleSpinBox(this);
    m_maxSlippageSpin->setRange(0.1, 10.0);
    m_maxSlippageSpin->setValue(2.0);
    m_maxSlippageSpin->setDecimals(1);

    settingsAdvancedLayout->addWidget(maxSwapsLabel);
    settingsAdvancedLayout->addWidget(m_maxConcurrentSwapsSpin);
    settingsAdvancedLayout->addWidget(maxSlippageLabel);
    settingsAdvancedLayout->addWidget(m_maxSlippageSpin);
    settingsAdvancedLayout->addStretch();

    m_saveSettingsButton = new QPushButton(tr("💾 Save Settings"), this);

    m_settingsLayout->addWidget(m_autoExecuteCheck);
    m_settingsLayout->addWidget(m_notifyOnSwapCheck);
    m_settingsLayout->addWidget(m_stopOnErrorCheck);
    m_settingsLayout->addLayout(settingsAdvancedLayout);
    m_settingsLayout->addWidget(m_saveSettingsButton);

    // Add all sections to main layout
    m_mainLayout->addWidget(m_overviewGroup);
    m_mainLayout->addWidget(m_strategiesGroup);
    m_mainLayout->addWidget(m_historyGroup);
    m_mainLayout->addWidget(m_analyticsGroup);
    m_mainLayout->addWidget(m_settingsGroup);

    // Setup chart
    setupChart();
}

void AutoSwapEngine::connectSignals()
{
    connect(m_createStrategyButton, &QPushButton::clicked, this, &AutoSwapEngine::onCreateStrategyClicked);
    connect(m_pauseStrategyButton, &QPushButton::clicked, this, &AutoSwapEngine::onPauseStrategyClicked);
    connect(m_resumeStrategyButton, &QPushButton::clicked, this, &AutoSwapEngine::onResumeStrategyClicked);
    connect(m_cancelStrategyButton, &QPushButton::clicked, this, &AutoSwapEngine::onCancelStrategyClicked);
    connect(m_editStrategyButton, &QPushButton::clicked, this, &AutoSwapEngine::onEditStrategyClicked);
    connect(m_duplicateStrategyButton, &QPushButton::clicked, this, &AutoSwapEngine::onDuplicateStrategyClicked);
    connect(m_exportStrategyButton, &QPushButton::clicked, this, &AutoSwapEngine::onExportStrategyClicked);
    
    connect(m_exportHistoryButton, &QPushButton::clicked, this, &AutoSwapEngine::onExportStrategyClicked);
    connect(m_clearHistoryButton, &QPushButton::clicked, this, [this]() {
        m_swapHistory.clear();
        updateSwapHistory();
    });
    connect(m_showAnalyticsButton, &QPushButton::clicked, this, &AutoSwapEngine::onShowAnalyticsClicked);
    
    connect(m_autoExecuteCheck, &QCheckBox::toggled, this, &AutoSwapEngine::onAutoExecuteToggled);
    connect(m_saveSettingsButton, &QPushButton::clicked, this, [this]() {
        saveSettings();
        showNotification(tr("Settings saved successfully"), "success");
    });
    
    connect(m_updateTimer, &QTimer::timeout, this, &AutoSwapEngine::onUpdateTimer);
    connect(m_executionTimer, &QTimer::timeout, this, [this]() {
        if (m_autoExecuteEnabled) {
            checkAndExecuteStrategies();
        }
    });
    
    connect(m_strategiesTable, &QTableWidget::itemClicked, this, &AutoSwapEngine::onStrategyItemClicked);
    connect(m_strategiesTable, &QTableWidget::customContextMenuRequested, this, &AutoSwapEngine::onContextMenuRequested);
}

void AutoSwapEngine::applyTheme()
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
        QPushButton:disabled {
            background-color: #808080;
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
        QCheckBox {
            color: #2E8B57;
        }
        QSpinBox, QDoubleSpinBox {
            border: 1px solid #2E8B57;
            border-radius: 3px;
            padding: 5px;
        }
    )");
}

void AutoSwapEngine::setupChart()
{
    m_performanceChart = new QChart();
    m_performanceChart->setTitle(tr("Swap Performance"));
    m_performanceChart->setAnimationOptions(QChart::SeriesAnimations);

    m_swapSeries = new QLineSeries();
    m_swapSeries->setName(tr("Swap Volume"));
    m_swapSeries->setColor(QColor("#2E8B57"));

    m_priceSeries = new QLineSeries();
    m_priceSeries->setName(tr("Average Price"));
    m_priceSeries->setColor(QColor("#FFD700"));

    m_performanceChart->addSeries(m_swapSeries);
    m_performanceChart->addSeries(m_priceSeries);

    m_valueAxis = new QValueAxis();
    m_valueAxis->setTitleText(tr("Value"));
    m_valueAxis->setLabelFormat("%.2f");

    m_timeAxis = new QDateTimeAxis();
    m_timeAxis->setTitleText(tr("Time"));
    m_timeAxis->setFormat("MMM dd");

    m_performanceChart->addAxis(m_valueAxis, Qt::AlignLeft);
    m_performanceChart->addAxis(m_timeAxis, Qt::AlignBottom);

    m_swapSeries->attachAxis(m_valueAxis);
    m_swapSeries->attachAxis(m_timeAxis);
    m_priceSeries->attachAxis(m_valueAxis);
    m_priceSeries->attachAxis(m_timeAxis);

    m_performanceChartView->setChart(m_performanceChart);
    m_performanceChartView->setRenderHint(QPainter::Antialiasing);
}

void AutoSwapEngine::onCreateStrategyClicked()
{
    showCreateStrategyDialog();
}

void AutoSwapEngine::showCreateStrategyDialog()
{
    // Create a simple strategy creation dialog
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Create Auto Swap Strategy"));
    dialog.setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    // Strategy name
    QLabel* nameLabel = new QLabel(tr("Strategy Name:"), &dialog);
    QLineEdit* nameInput = new QLineEdit(&dialog);
    nameInput->setPlaceholderText(tr("e.g., Daily SHAH to TokenA"));
    
    // From token
    QLabel* fromLabel = new QLabel(tr("From Token:"), &dialog);
    QComboBox* fromCombo = new QComboBox(&dialog);
    fromCombo->addItem("SHAH");
    fromCombo->addItem("TokenA");
    fromCombo->addItem("TokenB");
    
    // To token
    QLabel* toLabel = new QLabel(tr("To Token:"), &dialog);
    QComboBox* toCombo = new QComboBox(&dialog);
    toCombo->addItem("TokenA");
    toCombo->addItem("TokenB");
    toCombo->addItem("SHAH");
    
    // Amount
    QLabel* amountLabel = new QLabel(tr("Amount:"), &dialog);
    QDoubleSpinBox* amountInput = new QDoubleSpinBox(&dialog);
    amountInput->setRange(0.1, 10000.0);
    amountInput->setValue(10.0);
    amountInput->setDecimals(2);
    
    // Frequency
    QLabel* freqLabel = new QLabel(tr("Frequency:"), &dialog);
    QComboBox* freqCombo = new QComboBox(&dialog);
    freqCombo->addItem(tr("Daily"), Daily);
    freqCombo->addItem(tr("Weekly"), Weekly);
    freqCombo->addItem(tr("Monthly"), Monthly);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* createButton = new QPushButton(tr("Create"), &dialog);
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), &dialog);
    
    buttonLayout->addWidget(createButton);
    buttonLayout->addWidget(cancelButton);
    
    // Add widgets to layout
    layout->addWidget(nameLabel);
    layout->addWidget(nameInput);
    layout->addWidget(fromLabel);
    layout->addWidget(fromCombo);
    layout->addWidget(toLabel);
    layout->addWidget(toCombo);
    layout->addWidget(amountLabel);
    layout->addWidget(amountInput);
    layout->addWidget(freqLabel);
    layout->addWidget(freqCombo);
    layout->addLayout(buttonLayout);
    
    // Connect signals
    connect(createButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Create strategy
        SwapStrategy strategy;
        strategy.id = "strategy_" + QString::number(QDateTime::currentMSecsSinceEpoch());
        strategy.name = nameInput->text();
        strategy.fromToken = fromCombo->currentText();
        strategy.toToken = toCombo->currentText();
        strategy.amount = amountInput->value();
        strategy.frequency = static_cast<SwapFrequency>(freqCombo->currentData().toInt());
        strategy.triggerType = TimeBased;
        strategy.status = Active;
        strategy.startDate = QDateTime::currentDateTime();
        strategy.endDate = QDateTime::currentDateTime().addYears(1);
        strategy.totalSwaps = 0;
        strategy.totalAmount = 0.0;
        strategy.averagePrice = 0.0;
        strategy.totalFees = 0.0;
        strategy.nextSwap = QDateTime::currentDateTime().addDays(1);
        
        createSwapStrategy(strategy);
    }
}

void AutoSwapEngine::createSwapStrategy(const SwapStrategy& strategy)
{
    m_strategies.append(strategy);
    addStrategyToTable(strategy);
    updatePerformanceMetrics();
    saveStrategies();
    
    emit strategyCreated(strategy);
    showNotification(tr("Strategy created successfully"), "success");
}

void AutoSwapEngine::addStrategyToTable(const SwapStrategy& strategy)
{
    int row = m_strategiesTable->rowCount();
    m_strategiesTable->insertRow(row);
    
    m_strategiesTable->setItem(row, 0, new QTableWidgetItem(strategy.name));
    m_strategiesTable->setItem(row, 1, new QTableWidgetItem(strategy.fromToken));
    m_strategiesTable->setItem(row, 2, new QTableWidgetItem(strategy.toToken));
    m_strategiesTable->setItem(row, 3, new QTableWidgetItem(QString("%1").arg(strategy.amount, 0, 'f', 2)));
    
    QString frequencyText;
    switch (strategy.frequency) {
        case Daily: frequencyText = tr("Daily"); break;
        case Weekly: frequencyText = tr("Weekly"); break;
        case Monthly: frequencyText = tr("Monthly"); break;
        case Custom: frequencyText = tr("Custom"); break;
    }
    m_strategiesTable->setItem(row, 4, new QTableWidgetItem(frequencyText));
    
    QString statusText;
    switch (strategy.status) {
        case Active: statusText = tr("Active"); break;
        case Paused: statusText = tr("Paused"); break;
        case Completed: statusText = tr("Completed"); break;
        case Cancelled: statusText = tr("Cancelled"); break;
        case Error: statusText = tr("Error"); break;
    }
    m_strategiesTable->setItem(row, 5, new QTableWidgetItem(statusText));
    
    m_strategiesTable->setItem(row, 6, new QTableWidgetItem(strategy.nextSwap.toString("MMM dd, hh:mm")));
    m_strategiesTable->setItem(row, 7, new QTableWidgetItem(tr("Actions")));
    
    // Store strategy ID in the first column for reference
    m_strategiesTable->item(row, 0)->setData(Qt::UserRole, strategy.id);
}

void AutoSwapEngine::onPauseStrategyClicked()
{
    showNotification(tr("Strategy pause not yet implemented"), "info");
}

void AutoSwapEngine::onResumeStrategyClicked()
{
    showNotification(tr("Strategy resume not yet implemented"), "info");
}

void AutoSwapEngine::onCancelStrategyClicked()
{
    showNotification(tr("Strategy cancellation not yet implemented"), "info");
}

void AutoSwapEngine::onEditStrategyClicked()
{
    showNotification(tr("Strategy editing not yet implemented"), "info");
}

void AutoSwapEngine::onDuplicateStrategyClicked()
{
    showNotification(tr("Strategy duplication not yet implemented"), "info");
}

void AutoSwapEngine::onExportStrategyClicked()
{
    showNotification(tr("Strategy export not yet implemented"), "info");
}

void AutoSwapEngine::onShowAnalyticsClicked()
{
    showNotification(tr("Analytics dialog not yet implemented"), "info");
}

void AutoSwapEngine::onAutoExecuteToggled(bool enabled)
{
    m_autoExecuteEnabled = enabled;
    if (enabled) {
        m_executionTimer->start();
    } else {
        m_executionTimer->stop();
    }
}

void AutoSwapEngine::onUpdateTimer()
{
    updatePerformanceMetrics();
    updateSwapHistory();
}

void AutoSwapEngine::checkAndExecuteStrategies()
{
    // TODO: Implement strategy execution logic
}

void AutoSwapEngine::updatePerformanceMetrics()
{
    int activeStrategies = 0;
    int totalSwaps = 0;
    double totalVolume = 0.0;
    double totalFees = 0.0;
    
    for (const SwapStrategy& strategy : m_strategies) {
        if (strategy.status == Active) {
            activeStrategies++;
        }
        totalSwaps += strategy.totalSwaps;
        totalVolume += strategy.totalAmount;
        totalFees += strategy.totalFees;
    }
    
    m_activeStrategiesLabel->setText(tr("Active Strategies: %1").arg(activeStrategies));
    m_totalSwapsLabel->setText(tr("Total Swaps: %1").arg(totalSwaps));
    m_totalVolumeLabel->setText(tr("Total Volume: %1 SHAH").arg(totalVolume, 0, 'f', 2));
    m_totalFeesLabel->setText(tr("Total Fees: %1 SHAH").arg(totalFees, 0, 'f', 2));
}

void AutoSwapEngine::updateSwapHistory()
{
    m_swapHistoryTable->setRowCount(m_swapHistory.size());
    
    for (int i = 0; i < m_swapHistory.size(); ++i) {
        const SwapHistory& history = m_swapHistory[i];
        
        m_swapHistoryTable->setItem(i, 0, new QTableWidgetItem(history.timestamp.toString("MMM dd, hh:mm")));
        m_swapHistoryTable->setItem(i, 1, new QTableWidgetItem(history.fromToken));
        m_swapHistoryTable->setItem(i, 2, new QTableWidgetItem(history.toToken));
        m_swapHistoryTable->setItem(i, 3, new QTableWidgetItem(QString("%1").arg(history.amount, 0, 'f', 2)));
        m_swapHistoryTable->setItem(i, 4, new QTableWidgetItem(QString("%1").arg(history.price, 0, 'f', 4)));
        m_swapHistoryTable->setItem(i, 5, new QTableWidgetItem(QString("%1").arg(history.fees, 0, 'f', 4)));
        m_swapHistoryTable->setItem(i, 6, new QTableWidgetItem(history.success ? tr("Success") : tr("Failed")));
    }
}

void AutoSwapEngine::onStrategyItemClicked(QTableWidgetItem* item)
{
    Q_UNUSED(item);
    // TODO: Show strategy details
}

void AutoSwapEngine::onContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);
    // TODO: Implement context menu
}

void AutoSwapEngine::showNotification(const QString& message, const QString& type)
{
    Q_UNUSED(type);
    // TODO: Implement proper notification system
    qDebug() << "Auto Swap Engine:" << message;
}

void AutoSwapEngine::saveSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    saveStrategies();
    saveSwapHistory();
    m_settings->setValue(SETTINGS_AUTO_EXECUTE, m_autoExecuteEnabled);
    m_settings->setValue(SETTINGS_NOTIFY_ON_SWAP, m_notifyOnSwap);
    m_settings->setValue(SETTINGS_STOP_ON_ERROR, m_stopOnError);
    m_settings->setValue(SETTINGS_MAX_CONCURRENT_SWAPS, m_maxConcurrentSwaps);
    m_settings->setValue(SETTINGS_MAX_SLIPPAGE, m_maxSlippage);
    m_settings->endGroup();
}

void AutoSwapEngine::loadSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    loadStrategies();
    loadSwapHistory();
    m_autoExecuteEnabled = m_settings->value(SETTINGS_AUTO_EXECUTE, false).toBool();
    m_notifyOnSwap = m_settings->value(SETTINGS_NOTIFY_ON_SWAP, true).toBool();
    m_stopOnError = m_settings->value(SETTINGS_STOP_ON_ERROR, true).toBool();
    m_maxConcurrentSwaps = m_settings->value(SETTINGS_MAX_CONCURRENT_SWAPS, 3).toInt();
    m_maxSlippage = m_settings->value(SETTINGS_MAX_SLIPPAGE, 2.0).toDouble();
    m_settings->endGroup();
    
    // Update UI
    m_autoExecuteCheck->setChecked(m_autoExecuteEnabled);
    m_notifyOnSwapCheck->setChecked(m_notifyOnSwap);
    m_stopOnErrorCheck->setChecked(m_stopOnError);
    m_maxConcurrentSwapsSpin->setValue(m_maxConcurrentSwaps);
    m_maxSlippageSpin->setValue(m_maxSlippage);
    
    refreshStrategyList();
    updateSwapHistory();
    updatePerformanceMetrics();
}

void AutoSwapEngine::saveStrategies()
{
    QJsonArray strategiesArray;
    for (const SwapStrategy& strategy : m_strategies) {
        QJsonObject strategyObj;
        strategyObj["id"] = strategy.id;
        strategyObj["name"] = strategy.name;
        strategyObj["from_token"] = strategy.fromToken;
        strategyObj["to_token"] = strategy.toToken;
        strategyObj["amount"] = strategy.amount;
        strategyObj["frequency"] = static_cast<int>(strategy.frequency);
        strategyObj["status"] = static_cast<int>(strategy.status);
        strategyObj["total_swaps"] = strategy.totalSwaps;
        strategyObj["total_amount"] = strategy.totalAmount;
        strategyObj["average_price"] = strategy.averagePrice;
        strategyObj["total_fees"] = strategy.totalFees;
        strategiesArray.append(strategyObj);
    }
    
    m_settings->setValue(SETTINGS_STRATEGIES, strategiesArray);
}

void AutoSwapEngine::loadStrategies()
{
    m_strategies.clear();
    QJsonArray strategiesArray = m_settings->value(SETTINGS_STRATEGIES).toJsonArray();
    
    for (const QJsonValue& value : strategiesArray) {
        QJsonObject obj = value.toObject();
        SwapStrategy strategy;
        strategy.id = obj["id"].toString();
        strategy.name = obj["name"].toString();
        strategy.fromToken = obj["from_token"].toString();
        strategy.toToken = obj["to_token"].toString();
        strategy.amount = obj["amount"].toDouble();
        strategy.frequency = static_cast<SwapFrequency>(obj["frequency"].toInt());
        strategy.status = static_cast<SwapStatus>(obj["status"].toInt());
        strategy.totalSwaps = obj["total_swaps"].toInt();
        strategy.totalAmount = obj["total_amount"].toDouble();
        strategy.averagePrice = obj["average_price"].toDouble();
        strategy.totalFees = obj["total_fees"].toDouble();
        m_strategies.append(strategy);
    }
}

void AutoSwapEngine::saveSwapHistory()
{
    QJsonArray historyArray;
    for (const SwapHistory& history : m_swapHistory) {
        QJsonObject historyObj;
        historyObj["timestamp"] = history.timestamp.toString(Qt::ISODate);
        historyObj["from_token"] = history.fromToken;
        historyObj["to_token"] = history.toToken;
        historyObj["amount"] = history.amount;
        historyObj["price"] = history.price;
        historyObj["fees"] = history.fees;
        historyObj["transaction_id"] = history.transactionId;
        historyObj["success"] = history.success;
        historyArray.append(historyObj);
    }
    
    m_settings->setValue(SETTINGS_SWAP_HISTORY, historyArray);
}

void AutoSwapEngine::loadSwapHistory()
{
    m_swapHistory.clear();
    QJsonArray historyArray = m_settings->value(SETTINGS_SWAP_HISTORY).toJsonArray();
    
    for (const QJsonValue& value : historyArray) {
        QJsonObject obj = value.toObject();
        SwapHistory history;
        history.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        history.fromToken = obj["from_token"].toString();
        history.toToken = obj["to_token"].toString();
        history.amount = obj["amount"].toDouble();
        history.price = obj["price"].toDouble();
        history.fees = obj["fees"].toDouble();
        history.transactionId = obj["transaction_id"].toString();
        history.success = obj["success"].toBool();
        m_swapHistory.append(history);
    }
}

void AutoSwapEngine::refreshStrategyList()
{
    m_strategiesTable->setRowCount(0);
    for (const SwapStrategy& strategy : m_strategies) {
        addStrategyToTable(strategy);
    }
}

// SwapHistoryTable implementation
SwapHistoryTable::SwapHistoryTable(QWidget* parent)
    : QTableWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void SwapHistoryTable::addSwapRecord(const AutoSwapEngine::SwapHistory& history)
{
    int row = rowCount();
    insertRow(row);
    
    setItem(row, 0, new QTableWidgetItem(history.timestamp.toString("MMM dd, hh:mm")));
    setItem(row, 1, new QTableWidgetItem(history.fromToken));
    setItem(row, 2, new QTableWidgetItem(history.toToken));
    setItem(row, 3, new QTableWidgetItem(QString("%1").arg(history.amount, 0, 'f', 2)));
    setItem(row, 4, new QTableWidgetItem(QString("%1").arg(history.price, 0, 'f', 4)));
    setItem(row, 5, new QTableWidgetItem(QString("%1").arg(history.fees, 0, 'f', 4)));
    setItem(row, 6, new QTableWidgetItem(history.success ? tr("Success") : tr("Failed")));
}

void SwapHistoryTable::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    QAction* copyTransactionId = menu.addAction(tr("Copy Transaction ID"));
    QAction* copyAmount = menu.addAction(tr("Copy Amount"));
    menu.addSeparator();
    QAction* exportSwap = menu.addAction(tr("Export Swap"));
    
    QAction* selectedAction = menu.exec(event->globalPos());
    
    if (selectedAction == copyTransactionId) {
        onCopyTransactionId();
    } else if (selectedAction == copyAmount) {
        onCopyAmount();
    } else if (selectedAction == exportSwap) {
        onExportSwap();
    }
}

void SwapHistoryTable::onCopyTransactionId()
{
    // TODO: Implement copy transaction ID functionality
}

void SwapHistoryTable::onCopyAmount()
{
    // TODO: Implement copy amount functionality
}

void SwapHistoryTable::onExportSwap()
{
    // TODO: Implement export swap functionality
}
