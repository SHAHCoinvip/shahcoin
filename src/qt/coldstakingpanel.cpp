#include <qt/coldstakingpanel.h>
#include <qt/walletmodel.h>
#include <qt/platformstyle.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>
#include <interfaces/wallet.h>
#include <util/translation.h>
#include <stake/cold_staking.h> // TODO: Replace with actual cold staking backend includes
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
#include <QMenu>
#include <QAction>

ColdStakingPanel::ColdStakingPanel(const PlatformStyle* platformStyle, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(nullptr)
    , m_platformStyle(platformStyle)
    , m_refreshTimer(new QTimer(this))
    , m_stakeAmount(0)
    , m_understandChecked(false)
    , m_settings(new QSettings("Shahcoin", "ColdStaking", this))
{
    setupUI();
    connectSignals();
    applyTheme();
    
    // Set up refresh timer (every 30 seconds)
    connect(m_refreshTimer, &QTimer::timeout, this, &ColdStakingPanel::refreshData);
    m_refreshTimer->start(30000); // 30 seconds
    
    refreshData();
}

ColdStakingPanel::~ColdStakingPanel()
{
    // Save any persistent settings if needed
}

void ColdStakingPanel::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    if (m_walletModel) {
        refreshData();
    }
}

void ColdStakingPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_mainLayout->addWidget(m_tabWidget);
    
    setupColdStakingSetup();
    setupColdStakingList();
    setupManagementTools();
}

void ColdStakingPanel::setupColdStakingSetup()
{
    m_setupTab = new QWidget();
    m_setupLayout = new QVBoxLayout(m_setupTab);
    m_setupLayout->setContentsMargins(0, 0, 0, 0);
    m_setupLayout->setSpacing(20);
    
    // Setup Group
    m_setupGroup = new QGroupBox(tr("🔒 Cold Staking Setup"));
    m_setupForm = new QFormLayout(m_setupGroup);
    m_setupForm->setSpacing(15);
    
    // Hot Wallet Address
    m_hotAddressEdit = new QLineEdit();
    m_hotAddressEdit->setPlaceholderText(tr("Enter hot wallet address (online)"));
    m_hotAddressEdit->setMinimumWidth(400);
    m_setupForm->addRow(tr("Hot Wallet Address:"), m_hotAddressEdit);
    
    // Cold Wallet Address
    m_coldAddressEdit = new QLineEdit();
    m_coldAddressEdit->setPlaceholderText(tr("Enter cold wallet address (offline)"));
    m_coldAddressEdit->setMinimumWidth(400);
    m_setupForm->addRow(tr("Cold Wallet Address:"), m_coldAddressEdit);
    
    // Amount to Stake
    m_amountEdit = new QLineEdit();
    m_amountEdit->setPlaceholderText(tr("Enter amount to stake (minimum 333 SHAH)"));
    m_amountEdit->setMinimumWidth(200);
    m_setupForm->addRow(tr("Stake Amount (SHAH):"), m_amountEdit);
    
    // Understanding Checkbox
    m_understandCheckbox = new QCheckBox(tr("I understand that the cold wallet must remain untouched and offline"));
    m_understandCheckbox->setStyleSheet("QCheckBox { color: #d32f2f; font-weight: bold; }");
    m_setupForm->addRow("", m_understandCheckbox);
    
    m_setupLayout->addWidget(m_setupGroup);
    
    // Action Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    m_startColdStakingButton = new QPushButton(tr("🔒 Start Cold Staking"));
    m_startColdStakingButton->setMinimumHeight(40);
    m_startColdStakingButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; border: none; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #1976D2; } QPushButton:disabled { background-color: #cccccc; }");
    
    m_refreshButton = new QPushButton(tr("🔄 Refresh"));
    m_refreshButton->setMinimumHeight(40);
    
    buttonLayout->addWidget(m_startColdStakingButton);
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();
    
    m_setupLayout->addLayout(buttonLayout);
    m_setupLayout->addStretch();
    
    m_tabWidget->addTab(m_setupTab, tr("🔒 Setup"));
}

void ColdStakingPanel::setupColdStakingList()
{
    m_listTab = new QWidget();
    m_listLayout = new QVBoxLayout(m_listTab);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(15);
    
    // Cold Staking Table
    m_coldStakingTable = new QTableWidget();
    m_coldStakingTable->setColumnCount(8);
    m_coldStakingTable->setHorizontalHeaderLabels({
        tr("ID"),
        tr("Hot Address"),
        tr("Cold Address"),
        tr("Stake Amount"),
        tr("Rewards Earned"),
        tr("Start Time"),
        tr("Status"),
        tr("Confirmations")
    });
    
    m_coldStakingTable->horizontalHeader()->setStretchLastSection(true);
    m_coldStakingTable->setAlternatingRowColors(true);
    m_coldStakingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_coldStakingTable->setContextMenuPolicy(Qt::CustomContextMenu);
    
    m_listRefreshButton = new QPushButton(tr("🔄 Refresh List"));
    m_listRefreshButton->setMaximumWidth(150);
    
    QHBoxLayout* listButtonLayout = new QHBoxLayout();
    listButtonLayout->addWidget(m_listRefreshButton);
    listButtonLayout->addStretch();
    
    m_listLayout->addLayout(listButtonLayout);
    m_listLayout->addWidget(m_coldStakingTable);
    
    m_tabWidget->addTab(m_listTab, tr("📋 Cold Staking List"));
}

void ColdStakingPanel::setupManagementTools()
{
    m_managementTab = new QWidget();
    m_managementLayout = new QVBoxLayout(m_managementTab);
    m_managementLayout->setContentsMargins(0, 0, 0, 0);
    m_managementLayout->setSpacing(20);
    
    // Management Group
    m_managementGroup = new QGroupBox(tr("🛠️ Management Tools"));
    m_managementForm = new QVBoxLayout(m_managementGroup);
    m_managementForm->setSpacing(15);
    
    m_unstakeButton = new QPushButton(tr("❌ Unstake"));
    m_unstakeButton->setMinimumHeight(40);
    m_unstakeButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #d32f2f; }");
    
    m_changeDelegateButton = new QPushButton(tr("🔄 Change Delegate"));
    m_changeDelegateButton->setMinimumHeight(40);
    m_changeDelegateButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; border: none; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #F57C00; }");
    
    m_exportConfigButton = new QPushButton(tr("📤 Export Config"));
    m_exportConfigButton->setMinimumHeight(40);
    
    m_managementForm->addWidget(m_unstakeButton);
    m_managementForm->addWidget(m_changeDelegateButton);
    m_managementForm->addWidget(m_exportConfigButton);
    m_managementForm->addStretch();
    
    m_managementLayout->addWidget(m_managementGroup);
    m_managementLayout->addStretch();
    
    m_tabWidget->addTab(m_managementTab, tr("🛠️ Management"));
}

void ColdStakingPanel::connectSignals()
{
    // Setup signals
    connect(m_hotAddressEdit, &QLineEdit::textChanged,
            this, &ColdStakingPanel::onHotAddressChanged);
    connect(m_coldAddressEdit, &QLineEdit::textChanged,
            this, &ColdStakingPanel::onColdAddressChanged);
    connect(m_amountEdit, &QLineEdit::textChanged,
            this, &ColdStakingPanel::onAmountChanged);
    connect(m_understandCheckbox, &QCheckBox::toggled,
            this, &ColdStakingPanel::onUnderstandCheckboxToggled);
    
    // Button signals
    connect(m_startColdStakingButton, &QPushButton::clicked,
            this, &ColdStakingPanel::onStartColdStakingClicked);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &ColdStakingPanel::onRefreshClicked);
    connect(m_listRefreshButton, &QPushButton::clicked,
            this, &ColdStakingPanel::onRefreshClicked);
    
    // Management signals
    connect(m_unstakeButton, &QPushButton::clicked,
            this, &ColdStakingPanel::onUnstakeClicked);
    connect(m_changeDelegateButton, &QPushButton::clicked,
            this, &ColdStakingPanel::onChangeDelegateClicked);
    connect(m_exportConfigButton, &QPushButton::clicked,
            this, &ColdStakingPanel::onExportConfigClicked);
    
    // Table signals
    connect(m_coldStakingTable, &QTableWidget::cellClicked,
            this, &ColdStakingPanel::onColdStakingRowClicked);
    connect(m_coldStakingTable, &QTableWidget::customContextMenuRequested,
            this, &ColdStakingPanel::onColdStakingContextMenu);
}

void ColdStakingPanel::applyTheme()
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
            border-color: #2196F3;
        }
        QLineEdit:invalid {
            border-color: #f44336;
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
        QCheckBox {
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
        }
    )";
    
    setStyleSheet(style);
}

void ColdStakingPanel::refreshData()
{
    loadColdStakingData();
    refreshColdStakingList();
}

void ColdStakingPanel::loadColdStakingData()
{
    // TODO: Replace with actual backend call to get cold staking data
    m_coldStakingList.clear();
    
    // Simulated cold staking data
    ColdStakingInfo staking1;
    staking1.id = "CS001";
    staking1.hotAddress = "S1HotWalletAddress123456789";
    staking1.coldAddress = "S1ColdWalletAddress987654321";
    staking1.stakeAmount = 100000000000; // 1000 SHAH
    staking1.rewardsEarned = 5000000000; // 50 SHAH
    staking1.startTime = QDateTime::currentSecsSinceEpoch() - 86400 * 30; // 30 days ago
    staking1.status = tr("Active");
    staking1.confirmations = 100;
    m_coldStakingList.append(staking1);
    
    ColdStakingInfo staking2;
    staking2.id = "CS002";
    staking2.hotAddress = "S1HotWalletAddress456789123";
    staking2.coldAddress = "S1ColdWalletAddress321654987";
    staking2.stakeAmount = 50000000000; // 500 SHAH
    staking2.rewardsEarned = 2500000000; // 25 SHAH
    staking2.startTime = QDateTime::currentSecsSinceEpoch() - 86400 * 15; // 15 days ago
    staking2.status = tr("Active");
    staking2.confirmations = 50;
    m_coldStakingList.append(staking2);
}

void ColdStakingPanel::onHotAddressChanged()
{
    m_hotAddress = m_hotAddressEdit->text().trimmed();
    validateInputs();
}

void ColdStakingPanel::onColdAddressChanged()
{
    m_coldAddress = m_coldAddressEdit->text().trimmed();
    validateInputs();
}

void ColdStakingPanel::onAmountChanged()
{
    QString amountText = m_amountEdit->text();
    bool ok;
    double amount = amountText.toDouble(&ok);
    
    if (ok && amount > 0) {
        m_stakeAmount = static_cast<qint64>(amount * 100000000); // Convert to satoshis
    } else {
        m_stakeAmount = 0;
    }
    
    validateInputs();
}

void ColdStakingPanel::onUnderstandCheckboxToggled()
{
    m_understandChecked = m_understandCheckbox->isChecked();
    validateInputs();
}

void ColdStakingPanel::validateInputs()
{
    bool isValid = true;
    
    // Validate hot address
    if (m_hotAddress.isEmpty()) {
        m_hotAddressEdit->setStyleSheet("QLineEdit { border-color: #f44336; }");
        isValid = false;
    } else if (!isValidShahcoinAddress(m_hotAddress)) {
        m_hotAddressEdit->setStyleSheet("QLineEdit { border-color: #f44336; }");
        isValid = false;
    } else {
        m_hotAddressEdit->setStyleSheet("");
    }
    
    // Validate cold address
    if (m_coldAddress.isEmpty()) {
        m_coldAddressEdit->setStyleSheet("QLineEdit { border-color: #f44336; }");
        isValid = false;
    } else if (!isValidShahcoinAddress(m_coldAddress)) {
        m_coldAddressEdit->setStyleSheet("QLineEdit { border-color: #f44336; }");
        isValid = false;
    } else {
        m_coldAddressEdit->setStyleSheet("");
    }
    
    // Validate amount
    if (m_stakeAmount < 33300000000) { // Minimum 333 SHAH
        m_amountEdit->setStyleSheet("QLineEdit { border-color: #f44336; }");
        isValid = false;
    } else {
        m_amountEdit->setStyleSheet("");
    }
    
    // Validate understanding checkbox
    if (!m_understandChecked) {
        isValid = false;
    }
    
    // Check if addresses are different
    if (m_hotAddress == m_coldAddress && !m_hotAddress.isEmpty()) {
        showError(tr("Hot and cold addresses must be different"));
        isValid = false;
    }
    
    m_startColdStakingButton->setEnabled(isValid);
}

void ColdStakingPanel::onStartColdStakingClicked()
{
    if (!validateInputs()) {
        showError(tr("Please fix the validation errors before proceeding"));
        return;
    }
    
    showColdStakingConfirmation();
}

void ColdStakingPanel::showColdStakingConfirmation()
{
    // Create confirmation dialog
    m_confirmDialog = new QDialog(this);
    m_confirmDialog->setWindowTitle(tr("Confirm Cold Staking"));
    m_confirmDialog->setModal(true);
    m_confirmDialog->resize(600, 500);
    
    m_confirmLayout = new QVBoxLayout(m_confirmDialog);
    m_confirmLayout->setSpacing(20);
    
    m_confirmTitleLabel = new QLabel(tr("🔒 Confirm Cold Staking Setup"));
    m_confirmTitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2196F3;");
    
    m_confirmDetailsLabel = new QLabel(tr("Cold Staking Details:"));
    m_confirmDetailsLabel->setStyleSheet("font-weight: bold;");
    
    m_confirmDetailsText = new QTextEdit();
    m_confirmDetailsText->setReadOnly(true);
    
    // Build details text
    QString details = tr("Hot Wallet Address: %1\n").arg(formatAddress(m_hotAddress));
    details += tr("Cold Wallet Address: %1\n").arg(formatAddress(m_coldAddress));
    details += tr("Stake Amount: %1 SHAH\n").arg(formatStakeAmount(m_stakeAmount));
    details += tr("\n⚠️ IMPORTANT WARNINGS:\n");
    details += tr("• The cold wallet must remain OFFLINE and untouched\n");
    details += tr("• Only the hot wallet should be used for staking operations\n");
    details += tr("• Never expose the cold wallet private keys\n");
    details += tr("• The cold wallet will receive staking rewards\n");
    details += tr("• Minimum stake period: 30 days\n");
    details += tr("• Minimum stake amount: 333 SHAH\n");
    
    m_confirmDetailsText->setText(details);
    
    m_confirmButtonLayout = new QHBoxLayout();
    m_confirmButton = new QPushButton(tr("✅ Confirm Cold Staking"));
    m_confirmButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; border: none; border-radius: 5px; font-weight: bold; padding: 10px; } QPushButton:hover { background-color: #1976D2; }");
    
    m_cancelButton = new QPushButton(tr("❌ Cancel"));
    m_cancelButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 5px; font-weight: bold; padding: 10px; } QPushButton:hover { background-color: #d32f2f; }");
    
    m_confirmButtonLayout->addStretch();
    m_confirmButtonLayout->addWidget(m_cancelButton);
    m_confirmButtonLayout->addWidget(m_confirmButton);
    
    m_confirmLayout->addWidget(m_confirmTitleLabel);
    m_confirmLayout->addWidget(m_confirmDetailsLabel);
    m_confirmLayout->addWidget(m_confirmDetailsText);
    m_confirmLayout->addLayout(m_confirmButtonLayout);
    
    connect(m_confirmButton, &QPushButton::clicked, this, &ColdStakingPanel::onConfirmColdStaking);
    connect(m_cancelButton, &QPushButton::clicked, this, &ColdStakingPanel::onCancelColdStaking);
    
    m_confirmDialog->exec();
}

void ColdStakingPanel::onConfirmColdStaking()
{
    m_confirmDialog->accept();
    executeColdStaking();
}

void ColdStakingPanel::onCancelColdStaking()
{
    m_confirmDialog->reject();
}

void ColdStakingPanel::executeColdStaking()
{
    // TODO: Replace with actual backend cold staking execution
    // This should create and broadcast a cold staking transaction
    
    // Simulate cold staking execution
    QMessageBox::information(this, tr("Cold Staking Setup"), 
        tr("Cold staking setup has been submitted to the network.\n\n"
           "Transaction ID: %1\n"
           "Status: Pending confirmation\n\n"
           "⚠️ IMPORTANT: Keep your cold wallet offline and secure!").arg("COLD_STAKE_TX_HASH_123456"));
    
    // Add to cold staking list
    ColdStakingInfo newStaking;
    newStaking.id = QString("CS%1").arg(m_coldStakingList.size() + 1, 3, 10, QChar('0'));
    newStaking.hotAddress = m_hotAddress;
    newStaking.coldAddress = m_coldAddress;
    newStaking.stakeAmount = m_stakeAmount;
    newStaking.rewardsEarned = 0;
    newStaking.startTime = QDateTime::currentSecsSinceEpoch();
    newStaking.status = tr("Pending");
    newStaking.confirmations = 0;
    
    m_coldStakingList.append(newStaking);
    refreshColdStakingList();
    
    // Clear inputs
    m_hotAddressEdit->clear();
    m_coldAddressEdit->clear();
    m_amountEdit->clear();
    m_understandCheckbox->setChecked(false);
    m_hotAddress.clear();
    m_coldAddress.clear();
    m_stakeAmount = 0;
    m_understandChecked = false;
    
    validateInputs();
    showSuccess(tr("Cold staking setup initiated successfully"));
}

void ColdStakingPanel::onRefreshClicked()
{
    refreshData();
    showSuccess(tr("Data refreshed successfully"));
}

void ColdStakingPanel::onUnstakeClicked()
{
    if (m_coldStakingList.isEmpty()) {
        showError(tr("No cold staking positions to unstake"));
        return;
    }
    
    // TODO: Implement unstaking logic
    QMessageBox::information(this, tr("Unstake"), 
        tr("Unstaking functionality will be implemented.\n"
           "This will require signing with the cold wallet offline."));
}

void ColdStakingPanel::onChangeDelegateClicked()
{
    if (m_coldStakingList.isEmpty()) {
        showError(tr("No cold staking positions to modify"));
        return;
    }
    
    // TODO: Implement delegate change logic
    QMessageBox::information(this, tr("Change Delegate"), 
        tr("Delegate change functionality will be implemented.\n"
           "This will require signing with the cold wallet offline."));
}

void ColdStakingPanel::onExportConfigClicked()
{
    if (m_coldStakingList.isEmpty()) {
        showError(tr("No cold staking configurations to export"));
        return;
    }
    
    exportColdStakingConfig();
}

void ColdStakingPanel::onColdStakingRowClicked(int row, int column)
{
    if (row >= 0 && row < m_coldStakingList.size()) {
        m_selectedColdStakingId = m_coldStakingList.at(row).id;
    }
}

void ColdStakingPanel::onColdStakingContextMenu(const QPoint& pos)
{
    if (m_coldStakingList.isEmpty()) return;
    
    // Create context menu
    if (!m_coldStakingContextMenu) {
        m_coldStakingContextMenu = new QMenu(this);
        
        QAction* viewDetailsAction = new QAction(tr("View Details"), this);
        QAction* exportDataAction = new QAction(tr("Export Data"), this);
        
        connect(viewDetailsAction, &QAction::triggered, this, &ColdStakingPanel::onViewColdStakingDetails);
        connect(exportDataAction, &QAction::triggered, this, &ColdStakingPanel::onExportColdStakingData);
        
        m_coldStakingContextMenu->addAction(viewDetailsAction);
        m_coldStakingContextMenu->addAction(exportDataAction);
    }
    
    m_coldStakingContextMenu->exec(m_coldStakingTable->mapToGlobal(pos));
}

void ColdStakingPanel::onViewColdStakingDetails()
{
    if (m_selectedColdStakingId.isEmpty()) return;
    
    // Find the selected cold staking info
    for (const ColdStakingInfo& staking : m_coldStakingList) {
        if (staking.id == m_selectedColdStakingId) {
            QString details = tr("Cold Staking Details:\n\n");
            details += tr("ID: %1\n").arg(staking.id);
            details += tr("Hot Address: %1\n").arg(formatAddress(staking.hotAddress));
            details += tr("Cold Address: %1\n").arg(formatAddress(staking.coldAddress));
            details += tr("Stake Amount: %1 SHAH\n").arg(formatStakeAmount(staking.stakeAmount));
            details += tr("Rewards Earned: %1 SHAH\n").arg(formatStakeAmount(staking.rewardsEarned));
            details += tr("Start Time: %1\n").arg(QDateTime::fromSecsSinceEpoch(staking.startTime).toString("yyyy-MM-dd hh:mm:ss"));
            details += tr("Status: %1\n").arg(staking.status);
            details += tr("Confirmations: %1").arg(staking.confirmations);
            
            QMessageBox::information(this, tr("Cold Staking Details"), details);
            break;
        }
    }
}

void ColdStakingPanel::onExportColdStakingData()
{
    if (m_selectedColdStakingId.isEmpty()) return;
    
    // Find the selected cold staking info
    for (const ColdStakingInfo& staking : m_coldStakingList) {
        if (staking.id == m_selectedColdStakingId) {
            QJsonObject stakingData;
            stakingData["id"] = staking.id;
            stakingData["hotAddress"] = staking.hotAddress;
            stakingData["coldAddress"] = staking.coldAddress;
            stakingData["stakeAmount"] = QString::number(staking.stakeAmount);
            stakingData["rewardsEarned"] = QString::number(staking.rewardsEarned);
            stakingData["startTime"] = staking.startTime;
            stakingData["status"] = staking.status;
            stakingData["confirmations"] = staking.confirmations;
            
            QString fileName = QString("cold_staking_%1.json").arg(staking.id);
            QString filePath = QFileDialog::getSaveFileName(this, tr("Export Cold Staking Data"), fileName, "JSON Files (*.json)");
            
            if (!filePath.isEmpty()) {
                QFile file(filePath);
                if (file.open(QIODevice::WriteOnly)) {
                    QJsonDocument doc(stakingData);
                    file.write(doc.toJson());
                    file.close();
                    showSuccess(tr("Cold staking data exported successfully"));
                } else {
                    showError(tr("Failed to export cold staking data"));
                }
            }
            break;
        }
    }
}

void ColdStakingPanel::refreshColdStakingList()
{
    m_coldStakingTable->setRowCount(m_coldStakingList.size());
    
    for (int i = 0; i < m_coldStakingList.size(); ++i) {
        const ColdStakingInfo& staking = m_coldStakingList.at(i);
        
        m_coldStakingTable->setItem(i, 0, new QTableWidgetItem(staking.id));
        m_coldStakingTable->setItem(i, 1, new QTableWidgetItem(formatAddress(staking.hotAddress)));
        m_coldStakingTable->setItem(i, 2, new QTableWidgetItem(formatAddress(staking.coldAddress)));
        m_coldStakingTable->setItem(i, 3, new QTableWidgetItem(formatStakeAmount(staking.stakeAmount)));
        m_coldStakingTable->setItem(i, 4, new QTableWidgetItem(formatStakeAmount(staking.rewardsEarned)));
        m_coldStakingTable->setItem(i, 5, new QTableWidgetItem(QDateTime::fromSecsSinceEpoch(staking.startTime).toString("yyyy-MM-dd hh:mm:ss")));
        m_coldStakingTable->setItem(i, 6, new QTableWidgetItem(staking.status));
        m_coldStakingTable->setItem(i, 7, new QTableWidgetItem(QString::number(staking.confirmations)));
    }
}

void ColdStakingPanel::exportColdStakingConfig()
{
    if (m_coldStakingList.isEmpty()) {
        showError(tr("No cold staking configurations to export"));
        return;
    }
    
    QJsonArray stakingArray;
    for (const ColdStakingInfo& staking : m_coldStakingList) {
        QJsonObject stakingObj;
        stakingObj["id"] = staking.id;
        stakingObj["hotAddress"] = staking.hotAddress;
        stakingObj["coldAddress"] = staking.coldAddress;
        stakingObj["stakeAmount"] = QString::number(staking.stakeAmount);
        stakingObj["rewardsEarned"] = QString::number(staking.rewardsEarned);
        stakingObj["startTime"] = staking.startTime;
        stakingObj["status"] = staking.status;
        stakingObj["confirmations"] = staking.confirmations;
        stakingArray.append(stakingObj);
    }
    
    QJsonObject config;
    config["coldStakingConfigurations"] = stakingArray;
    config["exportDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    config["version"] = "1.0";
    
    QString fileName = QString("cold_staking_config_%1.json").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export Cold Staking Config"), fileName, "JSON Files (*.json)");
    
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(config);
            file.write(doc.toJson());
            file.close();
            showSuccess(tr("Cold staking configuration exported successfully"));
        } else {
            showError(tr("Failed to export cold staking configuration"));
        }
    }
}

void ColdStakingPanel::showError(const QString& message)
{
    QMessageBox::warning(this, tr("Cold Staking Error"), message);
}

void ColdStakingPanel::showSuccess(const QString& message)
{
    QMessageBox::information(this, tr("Success"), message);
}

QString ColdStakingPanel::formatStakeAmount(qint64 amount)
{
    double value = static_cast<double>(amount) / 100000000.0;
    return QString::number(value, 'f', 8);
}

QString ColdStakingPanel::formatAddress(const QString& address)
{
    if (address.length() > 20) {
        return address.left(10) + "..." + address.right(10);
    }
    return address;
}

bool ColdStakingPanel::isValidShahcoinAddress(const QString& address)
{
    // TODO: Replace with actual Shahcoin address validation
    // For now, just check basic format
    return address.startsWith("S1") && address.length() >= 26 && address.length() <= 35;
}
