#include "watchonlywallet.h"
#include "walletmodel.h"
#include "guiutil.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QSettings>
#include <QApplication>
#include <QStyle>
#include <QFont>
#include <QHeaderView>
#include <QClipboard>

WatchOnlyWallet::WatchOnlyWallet(WalletModel* walletModel, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(walletModel)
    , m_totalBalance(0.0)
    , m_transactionCount(0)
    , m_settings(new QSettings("Shahcoin", "Shahcoin-Qt"))
    , m_updateTimer(new QTimer(this))
{
    setWindowTitle(tr("👁 Watch-Only Wallet"));
    
    setupUI();
    connectSignals();
    loadSettings();
    
    // Setup update timer
    connect(m_updateTimer, &QTimer::timeout, this, &WatchOnlyWallet::refreshData);
    m_updateTimer->start(UPDATE_INTERVAL_MS);
    
    // Initial data refresh
    refreshData();
}

WatchOnlyWallet::~WatchOnlyWallet()
{
    delete m_settings;
}

void WatchOnlyWallet::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Title
    QLabel* titleLabel = new QLabel(tr("👁 Watch-Only Wallet"));
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #007bff; margin-bottom: 10px;");
    m_mainLayout->addWidget(titleLabel);
    
    // Description
    QLabel* descLabel = new QLabel(tr("Monitor public addresses without private keys. You can view balances and transactions but cannot send funds."));
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setStyleSheet("color: #666666; margin-bottom: 15px;");
    m_mainLayout->addWidget(descLabel);
    
    setupAddressGroup();
    setupBalanceGroup();
    setupTransactionGroup();
    setupButtonGroup();
}

void WatchOnlyWallet::setupAddressGroup()
{
    m_addressGroup = new QGroupBox(tr("📍 Address Management"));
    QVBoxLayout* addressLayout = new QVBoxLayout(m_addressGroup);
    
    // Address input section
    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_addressLabel = new QLabel(tr("Public Address:"));
    m_addressEdit = new QLineEdit();
    m_addressEdit->setPlaceholderText(tr("Enter SHAH public address to watch"));
    
    m_addAddressButton = new QPushButton(tr("➕ Add"));
    m_addAddressButton->setEnabled(false);
    m_addAddressButton->setStyleSheet(
        "QPushButton { "
        "background-color: #28a745; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:disabled { background-color: #6c757d; }"
    );
    
    m_removeAddressButton = new QPushButton(tr("➖ Remove"));
    m_removeAddressButton->setEnabled(false);
    m_removeAddressButton->setStyleSheet(
        "QPushButton { "
        "background-color: #dc3545; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #c82333; }"
        "QPushButton:disabled { background-color: #6c757d; }"
    );
    
    inputLayout->addWidget(m_addressLabel);
    inputLayout->addWidget(m_addressEdit);
    inputLayout->addWidget(m_addAddressButton);
    inputLayout->addWidget(m_removeAddressButton);
    
    addressLayout->addLayout(inputLayout);
    
    // Address table
    m_addressTable = new QTableWidget();
    m_addressTable->setColumnCount(3);
    m_addressTable->setHorizontalHeaderLabels({tr("Address"), tr("Balance"), tr("Last Activity")});
    m_addressTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_addressTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_addressTable->setAlternatingRowColors(true);
    m_addressTable->horizontalHeader()->setStretchLastSection(true);
    m_addressTable->setMaximumHeight(150);
    
    addressLayout->addWidget(m_addressTable);
    
    m_mainLayout->addWidget(m_addressGroup);
}

void WatchOnlyWallet::setupBalanceGroup()
{
    m_balanceGroup = new QGroupBox(tr("💰 Total Balance"));
    QHBoxLayout* balanceLayout = new QHBoxLayout(m_balanceGroup);
    
    // Total balance
    QVBoxLayout* balanceVLayout = new QVBoxLayout();
    m_totalBalanceLabel = new QLabel(tr("Total Balance:"));
    m_totalBalanceLabel->setStyleSheet("font-weight: bold; color: #28a745;");
    
    m_balanceValueLabel = new QLabel("0.000000 SHAH");
    QFont balanceFont = m_balanceValueLabel->font();
    balanceFont.setPointSize(16);
    balanceFont.setBold(true);
    m_balanceValueLabel->setFont(balanceFont);
    m_balanceValueLabel->setStyleSheet("color: #28a745;");
    
    balanceVLayout->addWidget(m_totalBalanceLabel);
    balanceVLayout->addWidget(m_balanceValueLabel);
    
    // Transaction count
    QVBoxLayout* countVLayout = new QVBoxLayout();
    m_transactionCountLabel = new QLabel(tr("Total Transactions:"));
    m_transactionCountLabel->setStyleSheet("font-weight: bold; color: #007bff;");
    
    m_countValueLabel = new QLabel("0");
    QFont countFont = m_countValueLabel->font();
    countFont.setPointSize(16);
    countFont.setBold(true);
    m_countValueLabel->setFont(countFont);
    m_countValueLabel->setStyleSheet("color: #007bff;");
    
    countVLayout->addWidget(m_transactionCountLabel);
    countVLayout->addWidget(m_countValueLabel);
    
    balanceLayout->addLayout(balanceVLayout);
    balanceLayout->addStretch();
    balanceLayout->addLayout(countVLayout);
    
    m_mainLayout->addWidget(m_balanceGroup);
}

void WatchOnlyWallet::setupTransactionGroup()
{
    m_transactionGroup = new QGroupBox(tr("📊 Recent Transactions"));
    QVBoxLayout* transactionLayout = new QVBoxLayout(m_transactionGroup);
    
    m_transactionTable = new QTableWidget();
    m_transactionTable->setColumnCount(5);
    m_transactionTable->setHorizontalHeaderLabels({
        tr("Date"), tr("Address"), tr("Type"), tr("Amount"), tr("Status")
    });
    m_transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_transactionTable->setAlternatingRowColors(true);
    m_transactionTable->horizontalHeader()->setStretchLastSection(true);
    m_transactionTable->setMaximumHeight(200);
    
    transactionLayout->addWidget(m_transactionTable);
    
    m_mainLayout->addWidget(m_transactionGroup);
}

void WatchOnlyWallet::setupButtonGroup()
{
    m_buttonLayout = new QHBoxLayout();
    
    m_importButton = new QPushButton(tr("📁 Import from File"));
    m_importButton->setStyleSheet(
        "QPushButton { "
        "background-color: #17a2b8; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px 16px; "
        "}"
        "QPushButton:hover { background-color: #138496; }"
    );
    
    m_exportButton = new QPushButton(tr("💾 Export to File"));
    m_exportButton->setStyleSheet(
        "QPushButton { "
        "background-color: #6f42c1; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px 16px; "
        "}"
        "QPushButton:hover { background-color: #5a32a3; }"
    );
    
    m_refreshButton = new QPushButton(tr("🔄 Refresh"));
    m_refreshButton->setStyleSheet(
        "QPushButton { "
        "background-color: #ffc107; "
        "color: #212529; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px 16px; "
        "}"
        "QPushButton:hover { background-color: #e0a800; }"
    );
    
    m_buttonLayout->addWidget(m_importButton);
    m_buttonLayout->addWidget(m_exportButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_refreshButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void WatchOnlyWallet::connectSignals()
{
    connect(m_addressEdit, &QLineEdit::textChanged, this, &WatchOnlyWallet::onAddressTextChanged);
    connect(m_addAddressButton, &QPushButton::clicked, this, &WatchOnlyWallet::onAddAddressClicked);
    connect(m_removeAddressButton, &QPushButton::clicked, this, &WatchOnlyWallet::onRemoveAddressClicked);
    connect(m_importButton, &QPushButton::clicked, this, &WatchOnlyWallet::onImportFromFileClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &WatchOnlyWallet::onExportToFileClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &WatchOnlyWallet::onRefreshClicked);
    connect(m_addressTable, &QTableWidget::itemSelectionChanged, this, &WatchOnlyWallet::onTableSelectionChanged);
}

void WatchOnlyWallet::loadSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_watchedAddresses = m_settings->value(ADDRESSES_KEY, QStringList()).toStringList();
    m_settings->endGroup();
    
    populateAddressTable();
}

void WatchOnlyWallet::saveSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_settings->setValue(ADDRESSES_KEY, m_watchedAddresses);
    m_settings->endGroup();
    m_settings->sync();
}

bool WatchOnlyWallet::addWatchedAddress(const QString& address)
{
    if (address.trimmed().isEmpty()) {
        return false;
    }
    
    if (!validateAddress(address)) {
        showError(tr("Invalid Address"), tr("Please enter a valid SHAH address."));
        return false;
    }
    
    if (isAddressWatched(address)) {
        showWarning(tr("Address Already Watched"), tr("This address is already being monitored."));
        return false;
    }
    
    if (m_watchedAddresses.size() >= MAX_WATCHED_ADDRESSES) {
        showError(tr("Limit Reached"), tr("Maximum number of watched addresses reached (%1).").arg(MAX_WATCHED_ADDRESSES));
        return false;
    }
    
    m_watchedAddresses.append(address);
    saveSettings();
    populateAddressTable();
    refreshData();
    
    emit addressAdded(address);
    showSuccess(tr("Address Added"), tr("Address %1 is now being monitored.").arg(address));
    
    return true;
}

bool WatchOnlyWallet::removeWatchedAddress(const QString& address)
{
    if (m_watchedAddresses.removeOne(address)) {
        saveSettings();
        populateAddressTable();
        refreshData();
        
        emit addressRemoved(address);
        showSuccess(tr("Address Removed"), tr("Address %1 is no longer being monitored.").arg(address));
        return true;
    }
    return false;
}

bool WatchOnlyWallet::isAddressWatched(const QString& address) const
{
    return m_watchedAddresses.contains(address, Qt::CaseInsensitive);
}

QStringList WatchOnlyWallet::getWatchedAddresses() const
{
    return m_watchedAddresses;
}

double WatchOnlyWallet::getTotalBalance() const
{
    return m_totalBalance;
}

int WatchOnlyWallet::getTransactionCount() const
{
    return m_transactionCount;
}

QStringList WatchOnlyWallet::getRecentTransactions() const
{
    return m_recentTransactions;
}

bool WatchOnlyWallet::importFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    QStringList newAddresses;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty() && validateAddress(line)) {
            newAddresses.append(line);
        }
    }
    
    file.close();
    
    if (newAddresses.isEmpty()) {
        showWarning(tr("No Valid Addresses"), tr("No valid SHAH addresses found in the file."));
        return false;
    }
    
    // Add new addresses (avoiding duplicates)
    int addedCount = 0;
    for (const QString& address : newAddresses) {
        if (!isAddressWatched(address) && m_watchedAddresses.size() < MAX_WATCHED_ADDRESSES) {
            m_watchedAddresses.append(address);
            addedCount++;
        }
    }
    
    if (addedCount > 0) {
        saveSettings();
        populateAddressTable();
        refreshData();
        showSuccess(tr("Import Complete"), tr("Successfully imported %1 new addresses.").arg(addedCount));
    } else {
        showWarning(tr("Import Complete"), tr("All addresses in the file are already being watched."));
    }
    
    return true;
}

bool WatchOnlyWallet::exportToFile(const QString& filePath) const
{
    if (m_watchedAddresses.isEmpty()) {
        showWarning(tr("No Addresses"), tr("No addresses to export."));
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << "# Shahcoin Watch-Only Wallet Addresses\n";
    out << "# Exported on: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    out << "# Total addresses: " << m_watchedAddresses.size() << "\n\n";
    
    for (const QString& address : m_watchedAddresses) {
        out << address << "\n";
    }
    
    file.close();
    return true;
}

bool WatchOnlyWallet::validateAddress(const QString& address) const
{
    // Basic SHAH address validation (simplified for demo)
    // In a real implementation, this would use proper address validation
    return address.length() >= 26 && address.length() <= 35 && 
           (address.startsWith("S") || address.startsWith("s"));
}

void WatchOnlyWallet::onAddAddressClicked()
{
    QString address = m_addressEdit->text().trimmed();
    if (addWatchedAddress(address)) {
        m_addressEdit->clear();
    }
}

void WatchOnlyWallet::onRemoveAddressClicked()
{
    QList<QTableWidgetItem*> selectedItems = m_addressTable->selectedItems();
    if (selectedItems.isEmpty()) {
        showWarning(tr("No Selection"), tr("Please select an address to remove."));
        return;
    }
    
    int row = selectedItems.first()->row();
    QString address = m_addressTable->item(row, 0)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        tr("Remove Address"), 
        tr("Are you sure you want to stop watching address %1?").arg(address),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        removeWatchedAddress(address);
    }
}

void WatchOnlyWallet::onImportFromFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, 
                                                   tr("Import Addresses"), 
                                                   "", 
                                                   tr("Text Files (*.txt);;All Files (*)"));
    
    if (!filePath.isEmpty()) {
        if (!importFromFile(filePath)) {
            showError(tr("Import Failed"), tr("Could not import addresses from file."));
        }
    }
}

void WatchOnlyWallet::onExportToFileClicked()
{
    if (m_watchedAddresses.isEmpty()) {
        showWarning(tr("No Addresses"), tr("No addresses to export."));
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, 
                                                   tr("Export Addresses"), 
                                                   "shahcoin_watched_addresses.txt", 
                                                   tr("Text Files (*.txt);;All Files (*)"));
    
    if (!filePath.isEmpty()) {
        if (exportToFile(filePath)) {
            showSuccess(tr("Export Complete"), tr("Addresses exported to: %1").arg(filePath));
        } else {
            showError(tr("Export Failed"), tr("Could not export addresses to file."));
        }
    }
}

void WatchOnlyWallet::onRefreshClicked()
{
    refreshData();
    showSuccess(tr("Refreshed"), tr("Data has been updated."));
}

void WatchOnlyWallet::onAddressTextChanged()
{
    QString address = m_addressEdit->text().trimmed();
    m_addAddressButton->setEnabled(!address.isEmpty() && validateAddress(address));
}

void WatchOnlyWallet::onTableSelectionChanged()
{
    bool hasSelection = !m_addressTable->selectedItems().isEmpty();
    m_removeAddressButton->setEnabled(hasSelection);
}

void WatchOnlyWallet::refreshData()
{
    // Generate mock data for demo purposes
    m_totalBalance = generateMockBalance();
    m_transactionCount = m_watchedAddresses.size() * 3; // Mock transaction count
    m_recentTransactions = generateMockTransactions();
    
    updateBalanceDisplay();
    updateTransactionTable();
    
    emit balanceUpdated(m_totalBalance);
    emit transactionCountChanged(m_transactionCount);
}

void WatchOnlyWallet::updateBalanceDisplay()
{
    m_balanceValueLabel->setText(QString("%1 SHAH").arg(m_totalBalance, 0, 'f', 6));
    m_countValueLabel->setText(QString::number(m_transactionCount));
}

void WatchOnlyWallet::updateTransactionTable()
{
    m_transactionTable->setRowCount(0);
    
    for (int i = 0; i < qMin(m_recentTransactions.size(), 10); ++i) {
        QStringList parts = m_recentTransactions[i].split("|");
        if (parts.size() >= 5) {
            int row = m_transactionTable->rowCount();
            m_transactionTable->insertRow(row);
            
            m_transactionTable->setItem(row, 0, new QTableWidgetItem(parts[0]));
            m_transactionTable->setItem(row, 1, new QTableWidgetItem(parts[1]));
            m_transactionTable->setItem(row, 2, new QTableWidgetItem(parts[2]));
            m_transactionTable->setItem(row, 3, new QTableWidgetItem(parts[3]));
            m_transactionTable->setItem(row, 4, new QTableWidgetItem(parts[4]));
        }
    }
}

void WatchOnlyWallet::populateAddressTable()
{
    m_addressTable->setRowCount(0);
    
    for (const QString& address : m_watchedAddresses) {
        int row = m_addressTable->rowCount();
        m_addressTable->insertRow(row);
        
        m_addressTable->setItem(row, 0, new QTableWidgetItem(address));
        m_addressTable->setItem(row, 1, new QTableWidgetItem(QString("%1 SHAH").arg(generateMockBalance() / m_watchedAddresses.size(), 0, 'f', 6)));
        m_addressTable->setItem(row, 2, new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm")));
    }
}

void WatchOnlyWallet::showWarning(const QString& title, const QString& message)
{
    QMessageBox::warning(this, title, message);
}

void WatchOnlyWallet::showSuccess(const QString& title, const QString& message)
{
    QMessageBox::information(this, title, message);
}

void WatchOnlyWallet::showError(const QString& title, const QString& message)
{
    QMessageBox::critical(this, title, message);
}

QStringList WatchOnlyWallet::generateMockTransactions() const
{
    QStringList transactions;
    QStringList types = {"Receive", "Send"};
    QStringList statuses = {"Confirmed", "Pending", "Failed"};
    
    for (int i = 0; i < 10; ++i) {
        QString date = QDateTime::currentDateTime().addSecs(-i * 3600).toString("yyyy-MM-dd hh:mm");
        QString address = m_watchedAddresses.isEmpty() ? "S..." : m_watchedAddresses.first();
        QString type = types[i % types.size()];
        QString amount = QString("%1 SHAH").arg((i + 1) * 0.1, 0, 'f', 6);
        QString status = statuses[i % statuses.size()];
        
        transactions.append(QString("%1|%2|%3|%4|%5").arg(date, address, type, amount, status));
    }
    
    return transactions;
}

double WatchOnlyWallet::generateMockBalance() const
{
    // Generate a mock total balance based on number of watched addresses
    double baseBalance = 100.0; // Base balance
    double perAddress = 25.0;   // Additional balance per address
    return baseBalance + (m_watchedAddresses.size() * perAddress);
}
