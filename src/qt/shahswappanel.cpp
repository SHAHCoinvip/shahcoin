#include <qt/shahswappanel.h>
#include <qt/walletmodel.h>
#include <qt/platformstyle.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>
#include <interfaces/wallet.h>
#include <util/translation.h>
#include <tokens/token.h> // TODO: Replace with actual token backend includes
#include <dex/dex.h>      // TODO: Replace with actual DEX backend includes
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QScrollBar>
#include <QStyle>
#include <QScreen>
#include <QPainter>
#include <QFontMetrics>
#include <QStandardPaths>
#include <QJsonArray>
#include <QInputDialog>
#include <QToolTip>

ShahSwapPanel::ShahSwapPanel(const PlatformStyle* platformStyle, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(nullptr)
    , m_platformStyle(platformStyle)
    , m_refreshTimer(new QTimer(this))
    , m_priceTimer(new QTimer(this))
    , m_currentSlippage(1.0)
    , m_transactionDeadline(20)
    , m_autoSlippage(true)
    , m_settings(new QSettings("Shahcoin", "ShahSwap", this))
{
    setupUI();
    connectSignals();
    applyTheme();
    
    // Set up refresh timer (every 30 seconds)
    connect(m_refreshTimer, &QTimer::timeout, this, &ShahSwapPanel::refreshData);
    m_refreshTimer->start(30000); // 30 seconds
    
    // Set up price update timer (every 5 seconds)
    connect(m_priceTimer, &QTimer::timeout, this, &ShahSwapPanel::calculateSwap);
    m_priceTimer->start(5000); // 5 seconds
    
    // Load settings
    m_currentSlippage = m_settings->value("slippage", 1.0).toDouble();
    m_transactionDeadline = m_settings->value("deadline", 20).toInt();
    m_autoSlippage = m_settings->value("autoSlippage", true).toBool();
    
    refreshData();
}

ShahSwapPanel::~ShahSwapPanel()
{
    // Save settings
    m_settings->setValue("slippage", m_currentSlippage);
    m_settings->setValue("deadline", m_transactionDeadline);
    m_settings->setValue("autoSlippage", m_autoSlippage);
}

void ShahSwapPanel::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    if (m_walletModel) {
        refreshData();
    }
}

void ShahSwapPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_mainLayout->addWidget(m_tabWidget);
    
    setupSwapInterface();
    setupHistory();
    setupPoolView();
    setupSettings();
}

void ShahSwapPanel::setupSwapInterface()
{
    m_swapTab = new QWidget();
    m_swapLayout = new QVBoxLayout(m_swapTab);
    m_swapLayout->setContentsMargins(0, 0, 0, 0);
    m_swapLayout->setSpacing(20);
    
    // Swap Group
    m_swapGroup = new QGroupBox(tr("🔁 Token Swap"));
    m_swapForm = new QFormLayout(m_swapGroup);
    m_swapForm->setSpacing(15);
    
    // Token A Selection
    m_tokenACombo = new QComboBox();
    m_tokenACombo->setMinimumWidth(200);
    m_swapForm->addRow(tr("From Token:"), m_tokenACombo);
    
    // Amount A Input
    m_amountAEdit = new QLineEdit();
    m_amountAEdit->setPlaceholderText(tr("Enter amount to swap"));
    m_amountAEdit->setMinimumWidth(200);
    m_swapForm->addRow(tr("Amount:"), m_amountAEdit);
    
    // Balance A Display
    m_balanceALabel = new QLabel(tr("Balance: 0"));
    m_balanceALabel->setStyleSheet("color: #666; font-size: 12px;");
    m_swapForm->addRow("", m_balanceALabel);
    
    // Token B Selection
    m_tokenBCombo = new QComboBox();
    m_tokenBCombo->setMinimumWidth(200);
    m_swapForm->addRow(tr("To Token:"), m_tokenBCombo);
    
    // Amount B Output
    m_amountBEdit = new QLineEdit();
    m_amountBEdit->setReadOnly(true);
    m_amountBEdit->setPlaceholderText(tr("Estimated output"));
    m_amountBEdit->setMinimumWidth(200);
    m_swapForm->addRow(tr("You Receive:"), m_amountBEdit);
    
    // Balance B Display
    m_balanceBLabel = new QLabel(tr("Balance: 0"));
    m_balanceBLabel->setStyleSheet("color: #666; font-size: 12px;");
    m_swapForm->addRow("", m_balanceBLabel);
    
    m_swapLayout->addWidget(m_swapGroup);
    
    // Swap Details Group
    m_detailsGroup = new QGroupBox(tr("📊 Swap Details"));
    m_detailsLayout = new QVBoxLayout(m_detailsGroup);
    m_detailsLayout->setSpacing(10);
    
    m_priceImpactLabel = new QLabel(tr("Price Impact: --"));
    m_minimumReceivedLabel = new QLabel(tr("Minimum Received: --"));
    m_liquidityFeeLabel = new QLabel(tr("Liquidity Fee: --"));
    m_slippageLabel = new QLabel(tr("Slippage: --"));
    m_routeLabel = new QLabel(tr("Route: --"));
    
    m_detailsLayout->addWidget(m_priceImpactLabel);
    m_detailsLayout->addWidget(m_minimumReceivedLabel);
    m_detailsLayout->addWidget(m_liquidityFeeLabel);
    m_detailsLayout->addWidget(m_slippageLabel);
    m_detailsLayout->addWidget(m_routeLabel);
    
    m_swapLayout->addWidget(m_detailsGroup);
    
    // Action Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    m_swapButton = new QPushButton(tr("🔄 Swap"));
    m_swapButton->setMinimumHeight(40);
    m_swapButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #45a049; } QPushButton:disabled { background-color: #cccccc; }");
    
    m_settingsButton = new QPushButton(tr("⚙️ Settings"));
    m_settingsButton->setMinimumHeight(40);
    
    m_refreshButton = new QPushButton(tr("🔄 Refresh"));
    m_refreshButton->setMinimumHeight(40);
    
    buttonLayout->addWidget(m_swapButton);
    buttonLayout->addWidget(m_settingsButton);
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();
    
    m_swapLayout->addLayout(buttonLayout);
    m_swapLayout->addStretch();
    
    m_tabWidget->addTab(m_swapTab, tr("🔁 Swap"));
}

void ShahSwapPanel::setupSettings()
{
    m_settingsDialog = new QDialog(this);
    m_settingsDialog->setWindowTitle(tr("ShahSwap Settings"));
    m_settingsDialog->setModal(true);
    m_settingsDialog->resize(400, 300);
    
    m_settingsLayout = new QVBoxLayout(m_settingsDialog);
    m_settingsLayout->setSpacing(20);
    
    // Slippage Settings
    m_slippageGroup = new QGroupBox(tr("Slippage Tolerance"));
    m_slippageLayout = new QHBoxLayout(m_slippageGroup);
    
    m_autoSlippageCheck = new QCheckBox(tr("Auto"));
    m_autoSlippageCheck->setChecked(m_autoSlippage);
    
    m_slippageSpinBox = new QDoubleSpinBox();
    m_slippageSpinBox->setRange(0.1, 50.0);
    m_slippageSpinBox->setSuffix("%");
    m_slippageSpinBox->setValue(m_currentSlippage);
    m_slippageSpinBox->setEnabled(!m_autoSlippage);
    
    m_slippageSlider = new QSlider(Qt::Horizontal);
    m_slippageSlider->setRange(1, 500); // 0.1% to 50%
    m_slippageSlider->setValue(static_cast<int>(m_currentSlippage * 10));
    m_slippageSlider->setEnabled(!m_autoSlippage);
    
    m_slippageValueLabel = new QLabel(QString::number(m_currentSlippage, 'f', 1) + "%");
    
    m_slippageLayout->addWidget(m_autoSlippageCheck);
    m_slippageLayout->addWidget(m_slippageSpinBox);
    m_slippageLayout->addWidget(m_slippageSlider);
    m_slippageLayout->addWidget(m_slippageValueLabel);
    
    // Deadline Settings
    m_deadlineGroup = new QGroupBox(tr("Transaction Deadline"));
    m_deadlineLayout = new QHBoxLayout(m_deadlineGroup);
    
    m_deadlineSpinBox = new QSpinBox();
    m_deadlineSpinBox->setRange(1, 1000);
    m_deadlineSpinBox->setValue(m_transactionDeadline);
    
    m_deadlineUnitCombo = new QComboBox();
    m_deadlineUnitCombo->addItems({tr("Blocks"), tr("Minutes")});
    
    m_deadlineLayout->addWidget(m_deadlineSpinBox);
    m_deadlineLayout->addWidget(m_deadlineUnitCombo);
    m_deadlineLayout->addStretch();
    
    // Settings Buttons
    QHBoxLayout* settingsButtonLayout = new QHBoxLayout();
    m_settingsSaveButton = new QPushButton(tr("Save"));
    m_settingsCancelButton = new QPushButton(tr("Cancel"));
    
    settingsButtonLayout->addStretch();
    settingsButtonLayout->addWidget(m_settingsSaveButton);
    settingsButtonLayout->addWidget(m_settingsCancelButton);
    
    m_settingsLayout->addWidget(m_slippageGroup);
    m_settingsLayout->addWidget(m_deadlineGroup);
    m_settingsLayout->addStretch();
    m_settingsLayout->addLayout(settingsButtonLayout);
}

void ShahSwapPanel::setupHistory()
{
    m_historyTab = new QWidget();
    m_historyLayout = new QVBoxLayout(m_historyTab);
    m_historyLayout->setContentsMargins(0, 0, 0, 0);
    m_historyLayout->setSpacing(15);
    
    // History Table
    m_historyTable = new QTableWidget();
    m_historyTable->setColumnCount(7);
    m_historyTable->setHorizontalHeaderLabels({
        tr("Date"),
        tr("From"),
        tr("To"),
        tr("Amount"),
        tr("Received"),
        tr("Status"),
        tr("TX Hash")
    });
    
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    m_historyRefreshButton = new QPushButton(tr("🔄 Refresh History"));
    m_historyRefreshButton->setMaximumWidth(150);
    
    QHBoxLayout* historyButtonLayout = new QHBoxLayout();
    historyButtonLayout->addWidget(m_historyRefreshButton);
    historyButtonLayout->addStretch();
    
    m_historyLayout->addLayout(historyButtonLayout);
    m_historyLayout->addWidget(m_historyTable);
    
    m_tabWidget->addTab(m_historyTab, tr("📋 History"));
}

void ShahSwapPanel::setupPoolView()
{
    m_poolTab = new QWidget();
    m_poolLayout = new QVBoxLayout(m_poolTab);
    m_poolLayout->setContentsMargins(0, 0, 0, 0);
    m_poolLayout->setSpacing(15);
    
    // Pool Table
    m_poolTable = new QTableWidget();
    m_poolTable->setColumnCount(6);
    m_poolTable->setHorizontalHeaderLabels({
        tr("Token A"),
        tr("Token B"),
        tr("Reserve A"),
        tr("Reserve B"),
        tr("Total Supply"),
        tr("Fee")
    });
    
    m_poolTable->horizontalHeader()->setStretchLastSection(true);
    m_poolTable->setAlternatingRowColors(true);
    m_poolTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    m_poolRefreshButton = new QPushButton(tr("🔄 Refresh Pools"));
    m_poolRefreshButton->setMaximumWidth(150);
    
    QHBoxLayout* poolButtonLayout = new QHBoxLayout();
    poolButtonLayout->addWidget(m_poolRefreshButton);
    poolButtonLayout->addStretch();
    
    m_poolLayout->addLayout(poolButtonLayout);
    m_poolLayout->addWidget(m_poolTable);
    
    m_tabWidget->addTab(m_poolTab, tr("🏊 Pool View"));
}

void ShahSwapPanel::connectSignals()
{
    // Token selection
    connect(m_tokenACombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &ShahSwapPanel::onTokenASelectionChanged);
    connect(m_tokenBCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &ShahSwapPanel::onTokenBSelectionChanged);
    
    // Amount input
    connect(m_amountAEdit, &QLineEdit::textChanged,
            this, &ShahSwapPanel::onAmountAChanged);
    
    // Buttons
    connect(m_swapButton, &QPushButton::clicked,
            this, &ShahSwapPanel::onSwapClicked);
    connect(m_settingsButton, &QPushButton::clicked,
            this, &ShahSwapPanel::onSettingsClicked);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &ShahSwapPanel::onRefreshClicked);
    connect(m_historyRefreshButton, &QPushButton::clicked,
            this, &ShahSwapPanel::onHistoryClicked);
    connect(m_poolRefreshButton, &QPushButton::clicked,
            this, &ShahSwapPanel::onPoolViewClicked);
    
    // Settings dialog
    connect(m_autoSlippageCheck, &QCheckBox::toggled,
            this, &ShahSwapPanel::onAutoSlippageToggled);
    connect(m_slippageSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ShahSwapPanel::onSlippageChanged);
    connect(m_slippageSlider, &QSlider::valueChanged,
            this, &ShahSwapPanel::onSlippageChanged);
    connect(m_deadlineSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ShahSwapPanel::onDeadlineChanged);
    connect(m_settingsSaveButton, &QPushButton::clicked,
            m_settingsDialog, &QDialog::accept);
    connect(m_settingsCancelButton, &QPushButton::clicked,
            m_settingsDialog, &QDialog::reject);
}

void ShahSwapPanel::applyTheme()
{
    QString style = R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QLineEdit {
            padding: 8px;
            border: 1px solid #cccccc;
            border-radius: 4px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #4CAF50;
        }
        QComboBox {
            padding: 8px;
            border: 1px solid #cccccc;
            border-radius: 4px;
            background-color: white;
        }
        QComboBox:focus {
            border-color: #4CAF50;
        }
        QPushButton {
            padding: 8px 16px;
            border: 1px solid #cccccc;
            border-radius: 4px;
            background-color: #f8f9fa;
        }
        QPushButton:hover {
            background-color: #e9ecef;
        }
        QTableWidget {
            border: 1px solid #cccccc;
            gridline-color: #e9ecef;
        }
        QTableWidget::item {
            padding: 8px;
        }
        QHeaderView::section {
            background-color: #f8f9fa;
            padding: 8px;
            border: none;
            border-bottom: 1px solid #dee2e6;
        }
    )";
    
    setStyleSheet(style);
}

void ShahSwapPanel::refreshData()
{
    loadTokenList();
    updateTokenBalances();
    refreshSwapHistory();
    refreshPoolData();
}

void ShahSwapPanel::loadTokenList()
{
    // TODO: Replace with actual backend call to get token list
    m_tokenList.clear();
    
    // Simulated token data
    TokenInfo shahToken;
    shahToken.id = "SHAH";
    shahToken.name = "Shahcoin";
    shahToken.symbol = "SHAH";
    shahToken.decimals = 8;
    shahToken.balance = 1000000000; // 10 SHAH
    shahToken.address = "SHAH_NATIVE";
    m_tokenList.append(shahToken);
    
    TokenInfo testToken1;
    testToken1.id = "TEST1";
    testToken1.name = "Test Token 1";
    testToken1.symbol = "TST1";
    testToken1.decimals = 6;
    testToken1.balance = 50000000; // 50 TST1
    testToken1.address = "TEST1_ADDRESS";
    m_tokenList.append(testToken1);
    
    TokenInfo testToken2;
    testToken2.id = "TEST2";
    testToken2.name = "Test Token 2";
    testToken2.symbol = "TST2";
    testToken2.decimals = 8;
    testToken2.balance = 100000000; // 1 TST2
    testToken2.address = "TEST2_ADDRESS";
    m_tokenList.append(testToken2);
    
    // Update combo boxes
    m_tokenACombo->clear();
    m_tokenBCombo->clear();
    
    for (const TokenInfo& token : m_tokenList) {
        QString displayText = QString("%1 (%2)").arg(token.symbol, token.name);
        m_tokenACombo->addItem(displayText, token.id);
        m_tokenBCombo->addItem(displayText, token.id);
    }
    
    // Set default selections
    if (m_tokenACombo->count() > 0) {
        m_tokenACombo->setCurrentIndex(0);
    }
    if (m_tokenBCombo->count() > 1) {
        m_tokenBCombo->setCurrentIndex(1);
    }
}

void ShahSwapPanel::onTokenASelectionChanged()
{
    m_selectedTokenA = m_tokenACombo->currentData().toString();
    updateTokenBalances();
    calculateSwap();
}

void ShahSwapPanel::onTokenBSelectionChanged()
{
    m_selectedTokenB = m_tokenBCombo->currentData().toString();
    updateTokenBalances();
    calculateSwap();
}

void ShahSwapPanel::onAmountAChanged()
{
    QString amountText = m_amountAEdit->text();
    bool ok;
    double amount = amountText.toDouble(&ok);
    
    if (ok && amount > 0) {
        // Find token A info
        for (const TokenInfo& token : m_tokenList) {
            if (token.id == m_selectedTokenA) {
                m_inputAmount = static_cast<qint64>(amount * pow(10, token.decimals));
                break;
            }
        }
    } else {
        m_inputAmount = 0;
    }
    
    calculateSwap();
}

void ShahSwapPanel::calculateSwap()
{
    if (m_selectedTokenA.isEmpty() || m_selectedTokenB.isEmpty() || m_inputAmount <= 0) {
        m_amountBEdit->clear();
        updateSwapDetails();
        return;
    }
    
    // TODO: Replace with actual AMM calculation using constant product formula
    // For now, use a simple 1:1 ratio with 0.3% fee
    
    // Find token info
    TokenInfo tokenA, tokenB;
    for (const TokenInfo& token : m_tokenList) {
        if (token.id == m_selectedTokenA) tokenA = token;
        if (token.id == m_selectedTokenB) tokenB = token;
    }
    
    // Simple swap calculation (replace with actual AMM logic)
    double fee = 0.003; // 0.3%
    double inputValue = static_cast<double>(m_inputAmount) / pow(10, tokenA.decimals);
    double outputValue = inputValue * (1.0 - fee);
    m_outputAmount = static_cast<qint64>(outputValue * pow(10, tokenB.decimals));
    
    // Update UI
    m_amountBEdit->setText(formatTokenAmount(m_outputAmount, tokenB.decimals));
    updateSwapDetails();
}

void ShahSwapPanel::updateTokenBalances()
{
    // Update balance labels
    for (const TokenInfo& token : m_tokenList) {
        if (token.id == m_selectedTokenA) {
            m_balanceALabel->setText(tr("Balance: %1").arg(formatTokenAmount(token.balance, token.decimals)));
        }
        if (token.id == m_selectedTokenB) {
            m_balanceBLabel->setText(tr("Balance: %1").arg(formatTokenAmount(token.balance, token.decimals)));
        }
    }
}

void ShahSwapPanel::updateSwapDetails()
{
    if (m_inputAmount <= 0 || m_outputAmount <= 0) {
        m_priceImpactLabel->setText(tr("Price Impact: --"));
        m_minimumReceivedLabel->setText(tr("Minimum Received: --"));
        m_liquidityFeeLabel->setText(tr("Liquidity Fee: --"));
        m_slippageLabel->setText(tr("Slippage: %1%").arg(m_currentSlippage));
        m_routeLabel->setText(tr("Route: %1 → %2").arg(m_selectedTokenA, m_selectedTokenB));
        return;
    }
    
    // Calculate price impact (simplified)
    double priceImpact = calculatePriceImpact(m_inputAmount, m_outputAmount);
    m_priceImpactLabel->setText(tr("Price Impact: %1%").arg(QString::number(priceImpact, 'f', 2)));
    
    // Calculate minimum received
    qint64 minimumReceived = calculateMinimumReceived(m_outputAmount, m_currentSlippage);
    m_minimumReceivedLabel->setText(tr("Minimum Received: %1").arg(formatTokenAmount(minimumReceived, 8)));
    
    // Liquidity fee
    m_liquidityFeeLabel->setText(tr("Liquidity Fee: 0.3%"));
    
    // Slippage
    m_slippageLabel->setText(tr("Slippage: %1%").arg(QString::number(m_currentSlippage, 'f', 1)));
    
    // Route
    m_routeLabel->setText(tr("Route: %1 → %2").arg(m_selectedTokenA, m_selectedTokenB));
}

void ShahSwapPanel::onSwapClicked()
{
    if (!validateSwap()) {
        return;
    }
    
    showSwapConfirmation();
}

void ShahSwapPanel::onSettingsClicked()
{
    // Update settings dialog with current values
    m_autoSlippageCheck->setChecked(m_autoSlippage);
    m_slippageSpinBox->setValue(m_currentSlippage);
    m_slippageSlider->setValue(static_cast<int>(m_currentSlippage * 10));
    m_deadlineSpinBox->setValue(m_transactionDeadline);
    
    if (m_settingsDialog->exec() == QDialog::Accepted) {
        // Settings were saved
        m_currentSlippage = m_slippageSpinBox->value();
        m_transactionDeadline = m_deadlineSpinBox->value();
        m_autoSlippage = m_autoSlippageCheck->isChecked();
        
        // Save to settings
        m_settings->setValue("slippage", m_currentSlippage);
        m_settings->setValue("deadline", m_transactionDeadline);
        m_settings->setValue("autoSlippage", m_autoSlippage);
        
        updateSwapDetails();
    }
}

void ShahSwapPanel::onRefreshClicked()
{
    refreshData();
    showSuccess(tr("Data refreshed successfully"));
}

void ShahSwapPanel::onHistoryClicked()
{
    refreshSwapHistory();
}

void ShahSwapPanel::onPoolViewClicked()
{
    refreshPoolData();
}

void ShahSwapPanel::onSlippageChanged()
{
    if (m_autoSlippage) return;
    
    double slippage = m_slippageSpinBox->value();
    m_slippageSlider->setValue(static_cast<int>(slippage * 10));
    m_slippageValueLabel->setText(QString::number(slippage, 'f', 1) + "%");
    
    m_currentSlippage = slippage;
    updateSwapDetails();
}

void ShahSwapPanel::onDeadlineChanged()
{
    m_transactionDeadline = m_deadlineSpinBox->value();
}

void ShahSwapPanel::onAutoSlippageToggled()
{
    bool autoSlippage = m_autoSlippageCheck->isChecked();
    m_slippageSpinBox->setEnabled(!autoSlippage);
    m_slippageSlider->setEnabled(!autoSlippage);
    
    if (autoSlippage) {
        // Auto-calculate slippage based on pool liquidity
        m_currentSlippage = 0.5; // TODO: Calculate based on actual pool data
        m_slippageSpinBox->setValue(m_currentSlippage);
        m_slippageSlider->setValue(static_cast<int>(m_currentSlippage * 10));
        m_slippageValueLabel->setText(QString::number(m_currentSlippage, 'f', 1) + "%");
    }
    
    m_autoSlippage = autoSlippage;
    updateSwapDetails();
}

bool ShahSwapPanel::validateSwap()
{
    if (m_selectedTokenA.isEmpty() || m_selectedTokenB.isEmpty()) {
        showError(tr("Please select both tokens"));
        return false;
    }
    
    if (m_selectedTokenA == m_selectedTokenB) {
        showError(tr("Cannot swap the same token"));
        return false;
    }
    
    if (m_inputAmount <= 0) {
        showError(tr("Please enter a valid amount"));
        return false;
    }
    
    // Check balance
    for (const TokenInfo& token : m_tokenList) {
        if (token.id == m_selectedTokenA && m_inputAmount > token.balance) {
            showError(tr("Insufficient balance"));
            return false;
        }
    }
    
    return true;
}

void ShahSwapPanel::showSwapConfirmation()
{
    // Create confirmation dialog
    m_confirmDialog = new QDialog(this);
    m_confirmDialog->setWindowTitle(tr("Confirm Swap"));
    m_confirmDialog->setModal(true);
    m_confirmDialog->resize(500, 400);
    
    m_confirmLayout = new QVBoxLayout(m_confirmDialog);
    m_confirmLayout->setSpacing(20);
    
    m_confirmTitleLabel = new QLabel(tr("🔄 Confirm Token Swap"));
    m_confirmTitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #4CAF50;");
    
    m_confirmDetailsLabel = new QLabel(tr("Swap Details:"));
    m_confirmDetailsLabel->setStyleSheet("font-weight: bold;");
    
    m_confirmDetailsText = new QTextEdit();
    m_confirmDetailsText->setReadOnly(true);
    
    // Build details text
    QString details = tr("From: %1 %2\n").arg(formatTokenAmount(m_inputAmount, 8), m_selectedTokenA);
    details += tr("To: %1 %2\n").arg(formatTokenAmount(m_outputAmount, 8), m_selectedTokenB);
    details += tr("Price Impact: %1%\n").arg(QString::number(calculatePriceImpact(m_inputAmount, m_outputAmount), 'f', 2));
    details += tr("Slippage: %1%\n").arg(QString::number(m_currentSlippage, 'f', 1));
    details += tr("Minimum Received: %1 %2\n").arg(formatTokenAmount(calculateMinimumReceived(m_outputAmount, m_currentSlippage), 8), m_selectedTokenB);
    details += tr("Liquidity Fee: 0.3%\n");
    details += tr("Transaction Deadline: %1 blocks").arg(m_transactionDeadline);
    
    m_confirmDetailsText->setText(details);
    
    m_confirmButtonLayout = new QHBoxLayout();
    m_confirmButton = new QPushButton(tr("✅ Confirm Swap"));
    m_confirmButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 5px; font-weight: bold; padding: 10px; } QPushButton:hover { background-color: #45a049; }");
    
    m_cancelButton = new QPushButton(tr("❌ Cancel"));
    m_cancelButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 5px; font-weight: bold; padding: 10px; } QPushButton:hover { background-color: #da190b; }");
    
    m_confirmButtonLayout->addStretch();
    m_confirmButtonLayout->addWidget(m_cancelButton);
    m_confirmButtonLayout->addWidget(m_confirmButton);
    
    m_confirmLayout->addWidget(m_confirmTitleLabel);
    m_confirmLayout->addWidget(m_confirmDetailsLabel);
    m_confirmLayout->addWidget(m_confirmDetailsText);
    m_confirmLayout->addLayout(m_confirmButtonLayout);
    
    connect(m_confirmButton, &QPushButton::clicked, this, &ShahSwapPanel::onConfirmSwap);
    connect(m_cancelButton, &QPushButton::clicked, this, &ShahSwapPanel::onCancelSwap);
    
    m_confirmDialog->exec();
}

void ShahSwapPanel::onConfirmSwap()
{
    m_confirmDialog->accept();
    executeSwap();
}

void ShahSwapPanel::onCancelSwap()
{
    m_confirmDialog->reject();
}

void ShahSwapPanel::executeSwap()
{
    // TODO: Replace with actual backend swap execution
    // This should create and broadcast a swap transaction
    
    // Simulate swap execution
    QMessageBox::information(this, tr("Swap Executed"), 
        tr("Swap transaction has been submitted to the network.\n\n"
           "Transaction ID: %1\n"
           "Status: Pending confirmation").arg("SWAP_TX_HASH_123456"));
    
    // Add to history
    SwapHistory history;
    history.txHash = "SWAP_TX_HASH_123456";
    history.tokenA = m_selectedTokenA;
    history.tokenB = m_selectedTokenB;
    history.amountA = m_inputAmount;
    history.amountB = m_outputAmount;
    history.timestamp = QDateTime::currentSecsSinceEpoch();
    history.status = tr("Pending");
    
    m_swapHistory.prepend(history);
    refreshSwapHistory();
    
    // Clear input
    m_amountAEdit->clear();
    m_inputAmount = 0;
    m_outputAmount = 0;
    m_amountBEdit->clear();
    updateSwapDetails();
    
    showSuccess(tr("Swap executed successfully"));
}

void ShahSwapPanel::refreshSwapHistory()
{
    // TODO: Replace with actual backend call to get swap history
    
    m_historyTable->setRowCount(m_swapHistory.size());
    
    for (int i = 0; i < m_swapHistory.size(); ++i) {
        const SwapHistory& history = m_swapHistory.at(i);
        
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(history.timestamp);
        
        m_historyTable->setItem(i, 0, new QTableWidgetItem(dateTime.toString("yyyy-MM-dd hh:mm:ss")));
        m_historyTable->setItem(i, 1, new QTableWidgetItem(history.tokenA));
        m_historyTable->setItem(i, 2, new QTableWidgetItem(history.tokenB));
        m_historyTable->setItem(i, 3, new QTableWidgetItem(formatTokenAmount(history.amountA, 8)));
        m_historyTable->setItem(i, 4, new QTableWidgetItem(formatTokenAmount(history.amountB, 8)));
        m_historyTable->setItem(i, 5, new QTableWidgetItem(history.status));
        m_historyTable->setItem(i, 6, new QTableWidgetItem(history.txHash));
    }
}

void ShahSwapPanel::refreshPoolData()
{
    // TODO: Replace with actual backend call to get pool data
    
    m_poolList.clear();
    
    // Simulated pool data
    PoolInfo pool1;
    pool1.tokenA = "SHAH";
    pool1.tokenB = "TEST1";
    pool1.reserveA = 100000000000; // 1000 SHAH
    pool1.reserveB = 1000000000;   // 10000 TEST1
    pool1.totalSupply = 1000000000;
    pool1.fee = 0.003;
    m_poolList.append(pool1);
    
    PoolInfo pool2;
    pool2.tokenA = "SHAH";
    pool2.tokenB = "TEST2";
    pool2.reserveA = 50000000000;  // 500 SHAH
    pool2.reserveB = 500000000;    // 5 TEST2
    pool2.totalSupply = 500000000;
    pool2.fee = 0.003;
    m_poolList.append(pool2);
    
    m_poolTable->setRowCount(m_poolList.size());
    
    for (int i = 0; i < m_poolList.size(); ++i) {
        const PoolInfo& pool = m_poolList.at(i);
        
        m_poolTable->setItem(i, 0, new QTableWidgetItem(pool.tokenA));
        m_poolTable->setItem(i, 1, new QTableWidgetItem(pool.tokenB));
        m_poolTable->setItem(i, 2, new QTableWidgetItem(formatTokenAmount(pool.reserveA, 8)));
        m_poolTable->setItem(i, 3, new QTableWidgetItem(formatTokenAmount(pool.reserveB, 8)));
        m_poolTable->setItem(i, 4, new QTableWidgetItem(formatTokenAmount(pool.totalSupply, 8)));
        m_poolTable->setItem(i, 5, new QTableWidgetItem(QString::number(pool.fee * 100, 'f', 1) + "%"));
    }
}

void ShahSwapPanel::showError(const QString& message)
{
    QMessageBox::warning(this, tr("Swap Error"), message);
}

void ShahSwapPanel::showSuccess(const QString& message)
{
    QMessageBox::information(this, tr("Success"), message);
}

QString ShahSwapPanel::formatTokenAmount(qint64 amount, int decimals)
{
    double value = static_cast<double>(amount) / pow(10, decimals);
    return QString::number(value, 'f', decimals);
}

QString ShahSwapPanel::formatPrice(qint64 price)
{
    return QString::number(static_cast<double>(price) / 100000000.0, 'f', 8);
}

qint64 ShahSwapPanel::calculateMinimumReceived(qint64 inputAmount, double slippage)
{
    return static_cast<qint64>(inputAmount * (1.0 - slippage / 100.0));
}

double ShahSwapPanel::calculatePriceImpact(qint64 inputAmount, qint64 outputAmount)
{
    // Simplified price impact calculation
    // TODO: Replace with actual AMM price impact calculation
    return 0.1; // 0.1% for demo
}
