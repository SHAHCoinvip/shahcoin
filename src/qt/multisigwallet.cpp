#include "multisigwallet.h"
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
#include <QSpinBox>
#include <QComboBox>

MultisigWallet::MultisigWallet(WalletModel* walletModel, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(walletModel)
    , m_requiredSignatures(2)
    , m_totalSigners(3)
    , m_settings(new QSettings("Shahcoin", "Shahcoin-Qt"))
    , m_updateTimer(new QTimer(this))
{
    setWindowTitle(tr("🔐 Multisig Wallet"));
    
    setupUI();
    connectSignals();
    loadSettings();
    
    // Setup update timer
    connect(m_updateTimer, &QTimer::timeout, this, &MultisigWallet::refreshData);
    m_updateTimer->start(UPDATE_INTERVAL_MS);
    
    // Initial data refresh
    refreshData();
}

MultisigWallet::~MultisigWallet()
{
    delete m_settings;
}

void MultisigWallet::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Title
    QLabel* titleLabel = new QLabel(tr("🔐 Multisig Wallet"));
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #6f42c1; margin-bottom: 10px;");
    m_mainLayout->addWidget(titleLabel);
    
    // Description
    QLabel* descLabel = new QLabel(tr("Create and manage multisignature wallets requiring multiple signatures for transactions."));
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setStyleSheet("color: #666666; margin-bottom: 15px;");
    m_mainLayout->addWidget(descLabel);
    
    setupCreationGroup();
    setupCosignersGroup();
    setupTransactionGroup();
    setupButtonGroup();
}

void MultisigWallet::setupCreationGroup()
{
    m_creationGroup = new QGroupBox(tr("🏗️ Create Multisig Address"));
    QVBoxLayout* creationLayout = new QVBoxLayout(m_creationGroup);
    
    // Signature requirements
    QHBoxLayout* sigLayout = new QHBoxLayout();
    m_requiredLabel = new QLabel(tr("Required Signatures:"));
    m_requiredSpinBox = new QSpinBox();
    m_requiredSpinBox->setRange(MIN_REQUIRED, MAX_REQUIRED);
    m_requiredSpinBox->setValue(m_requiredSignatures);
    
    m_totalLabel = new QLabel(tr("Total Signers:"));
    m_totalSpinBox = new QSpinBox();
    m_totalSpinBox->setRange(MIN_REQUIRED, MAX_COSIGNERS);
    m_totalSpinBox->setValue(m_totalSigners);
    
    sigLayout->addWidget(m_requiredLabel);
    sigLayout->addWidget(m_requiredSpinBox);
    sigLayout->addWidget(m_totalLabel);
    sigLayout->addWidget(m_totalSpinBox);
    sigLayout->addStretch();
    
    creationLayout->addLayout(sigLayout);
    
    // Create button
    m_createButton = new QPushButton(tr("🔨 Create Multisig Address"));
    m_createButton->setStyleSheet(
        "QPushButton { "
        "background-color: #28a745; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px 16px; "
        "font-weight: bold; "
        "}"
        "QPushButton:hover { background-color: #218838; }"
    );
    
    creationLayout->addWidget(m_createButton);
    
    // Address display
    QHBoxLayout* addrLayout = new QHBoxLayout();
    m_addressLabel = new QLabel(tr("Multisig Address:"));
    m_addressEdit = new QLineEdit();
    m_addressEdit->setReadOnly(true);
    m_addressEdit->setPlaceholderText(tr("Create a multisig address first"));
    
    addrLayout->addWidget(m_addressLabel);
    addrLayout->addWidget(m_addressEdit);
    
    creationLayout->addLayout(addrLayout);
    
    m_mainLayout->addWidget(m_creationGroup);
}

void MultisigWallet::setupCosignersGroup()
{
    m_cosignersGroup = new QGroupBox(tr("👥 Cosigners"));
    QVBoxLayout* cosignerLayout = new QVBoxLayout(m_cosignersGroup);
    
    // Cosigner input
    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_cosignerLabel = new QLabel(tr("Public Key:"));
    m_cosignerEdit = new QLineEdit();
    m_cosignerEdit->setPlaceholderText(tr("Enter cosigner public key"));
    
    m_addCosignerButton = new QPushButton(tr("➕ Add"));
    m_addCosignerButton->setEnabled(false);
    m_addCosignerButton->setStyleSheet(
        "QPushButton { "
        "background-color: #17a2b8; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #138496; }"
        "QPushButton:disabled { background-color: #6c757d; }"
    );
    
    m_removeCosignerButton = new QPushButton(tr("➖ Remove"));
    m_removeCosignerButton->setEnabled(false);
    m_removeCosignerButton->setStyleSheet(
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
    
    inputLayout->addWidget(m_cosignerLabel);
    inputLayout->addWidget(m_cosignerEdit);
    inputLayout->addWidget(m_addCosignerButton);
    inputLayout->addWidget(m_removeCosignerButton);
    
    cosignerLayout->addLayout(inputLayout);
    
    // Cosigner table
    m_cosignerTable = new QTableWidget();
    m_cosignerTable->setColumnCount(3);
    m_cosignerTable->setHorizontalHeaderLabels({tr("Public Key"), tr("Status"), tr("Added")});
    m_cosignerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_cosignerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_cosignerTable->setAlternatingRowColors(true);
    m_cosignerTable->horizontalHeader()->setStretchLastSection(true);
    m_cosignerTable->setMaximumHeight(120);
    
    cosignerLayout->addWidget(m_cosignerTable);
    
    m_mainLayout->addWidget(m_cosignersGroup);
}

void MultisigWallet::setupTransactionGroup()
{
    m_transactionGroup = new QGroupBox(tr("💸 Transaction Management"));
    QVBoxLayout* txLayout = new QVBoxLayout(m_transactionGroup);
    
    // Transaction creation
    QHBoxLayout* txInputLayout = new QHBoxLayout();
    m_recipientLabel = new QLabel(tr("Recipient:"));
    m_recipientEdit = new QLineEdit();
    m_recipientEdit->setPlaceholderText(tr("Enter recipient address"));
    
    m_amountLabel = new QLabel(tr("Amount (SHAH):"));
    m_amountEdit = new QLineEdit();
    m_amountEdit->setPlaceholderText(tr("0.000000"));
    
    txInputLayout->addWidget(m_recipientLabel);
    txInputLayout->addWidget(m_recipientEdit);
    txInputLayout->addWidget(m_amountLabel);
    txInputLayout->addWidget(m_amountEdit);
    
    txLayout->addLayout(txInputLayout);
    
    // Transaction buttons
    QHBoxLayout* txButtonLayout = new QHBoxLayout();
    m_createTxButton = new QPushButton(tr("📝 Create Transaction"));
    m_createTxButton->setStyleSheet(
        "QPushButton { "
        "background-color: #ffc107; "
        "color: #212529; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #e0a800; }"
    );
    
    m_signTxButton = new QPushButton(tr("✍️ Sign Transaction"));
    m_signTxButton->setStyleSheet(
        "QPushButton { "
        "background-color: #6f42c1; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #5a32a3; }"
    );
    
    m_combineButton = new QPushButton(tr("🔗 Combine Signatures"));
    m_combineButton->setStyleSheet(
        "QPushButton { "
        "background-color: #28a745; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #218838; }"
    );
    
    txButtonLayout->addWidget(m_createTxButton);
    txButtonLayout->addWidget(m_signTxButton);
    txButtonLayout->addWidget(m_combineButton);
    txButtonLayout->addStretch();
    
    txLayout->addLayout(txButtonLayout);
    
    // Transaction table
    m_transactionTable = new QTableWidget();
    m_transactionTable->setColumnCount(5);
    m_transactionTable->setHorizontalHeaderLabels({
        tr("Date"), tr("Recipient"), tr("Amount"), tr("Signatures"), tr("Status")
    });
    m_transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_transactionTable->setAlternatingRowColors(true);
    m_transactionTable->horizontalHeader()->setStretchLastSection(true);
    m_transactionTable->setMaximumHeight(150);
    
    txLayout->addWidget(m_transactionTable);
    
    m_mainLayout->addWidget(m_transactionGroup);
}

void MultisigWallet::setupButtonGroup()
{
    m_buttonLayout = new QHBoxLayout();
    
    m_saveConfigButton = new QPushButton(tr("💾 Save Config"));
    m_saveConfigButton->setStyleSheet(
        "QPushButton { "
        "background-color: #17a2b8; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px 16px; "
        "}"
        "QPushButton:hover { background-color: #138496; }"
    );
    
    m_loadConfigButton = new QPushButton(tr("📁 Load Config"));
    m_loadConfigButton->setStyleSheet(
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
    
    m_buttonLayout->addWidget(m_saveConfigButton);
    m_buttonLayout->addWidget(m_loadConfigButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_refreshButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void MultisigWallet::connectSignals()
{
    connect(m_requiredSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        m_totalSpinBox->setMinimum(value);
    });
    
    connect(m_totalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        m_requiredSpinBox->setMaximum(value);
    });
    
    connect(m_createButton, &QPushButton::clicked, this, &MultisigWallet::onCreateMultisigClicked);
    connect(m_cosignerEdit, &QLineEdit::textChanged, this, [this]() {
        m_addCosignerButton->setEnabled(!m_cosignerEdit->text().trimmed().isEmpty());
    });
    connect(m_addCosignerButton, &QPushButton::clicked, this, &MultisigWallet::onAddCosignerClicked);
    connect(m_removeCosignerButton, &QPushButton::clicked, this, &MultisigWallet::onRemoveCosignerClicked);
    connect(m_createTxButton, &QPushButton::clicked, this, &MultisigWallet::onCreateTransactionClicked);
    connect(m_signTxButton, &QPushButton::clicked, this, &MultisigWallet::onSignTransactionClicked);
    connect(m_combineButton, &QPushButton::clicked, this, &MultisigWallet::onCombineSignaturesClicked);
    connect(m_saveConfigButton, &QPushButton::clicked, this, &MultisigWallet::onSaveConfigClicked);
    connect(m_loadConfigButton, &QPushButton::clicked, this, &MultisigWallet::onLoadConfigClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &MultisigWallet::onRefreshClicked);
    connect(m_cosignerTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_removeCosignerButton->setEnabled(!m_cosignerTable->selectedItems().isEmpty());
    });
}

void MultisigWallet::loadSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_multisigAddress = m_settings->value(ADDRESS_KEY, "").toString();
    m_requiredSignatures = m_settings->value(REQUIRED_KEY, 2).toInt();
    m_totalSigners = m_settings->value(TOTAL_KEY, 3).toInt();
    m_cosigners = m_settings->value(COSIGNERS_KEY, QStringList()).toStringList();
    m_settings->endGroup();
    
    // Update UI
    m_requiredSpinBox->setValue(m_requiredSignatures);
    m_totalSpinBox->setValue(m_totalSigners);
    m_addressEdit->setText(m_multisigAddress);
    
    updateCosignerTable();
}

void MultisigWallet::saveSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_settings->setValue(ADDRESS_KEY, m_multisigAddress);
    m_settings->setValue(REQUIRED_KEY, m_requiredSignatures);
    m_settings->setValue(TOTAL_KEY, m_totalSigners);
    m_settings->setValue(COSIGNERS_KEY, m_cosigners);
    m_settings->endGroup();
    m_settings->sync();
}

bool MultisigWallet::createMultisigAddress(int requiredSignatures, int totalSigners)
{
    if (requiredSignatures > totalSigners) {
        showError(tr("Invalid Configuration"), tr("Required signatures cannot exceed total signers."));
        return false;
    }
    
    if (totalSigners > MAX_COSIGNERS) {
        showError(tr("Too Many Signers"), tr("Maximum number of signers is %1.").arg(MAX_COSIGNERS));
        return false;
    }
    
    m_requiredSignatures = requiredSignatures;
    m_totalSigners = totalSigners;
    m_multisigAddress = generateMockMultisigAddress();
    
    m_addressEdit->setText(m_multisigAddress);
    saveSettings();
    
    emit multisigCreated(m_multisigAddress, m_requiredSignatures, m_totalSigners);
    showSuccess(tr("Multisig Created"), 
               tr("Multisig address created: %1-of-%2").arg(m_requiredSignatures).arg(m_totalSigners));
    
    return true;
}

bool MultisigWallet::addCosigner(const QString& publicKey)
{
    if (publicKey.trimmed().isEmpty()) {
        return false;
    }
    
    if (!validatePublicKey(publicKey)) {
        showError(tr("Invalid Public Key"), tr("Please enter a valid public key."));
        return false;
    }
    
    if (m_cosigners.contains(publicKey, Qt::CaseInsensitive)) {
        showWarning(tr("Cosigner Exists"), tr("This public key is already added as a cosigner."));
        return false;
    }
    
    if (m_cosigners.size() >= m_totalSigners) {
        showError(tr("Limit Reached"), tr("Maximum number of cosigners reached (%1).").arg(m_totalSigners));
        return false;
    }
    
    m_cosigners.append(publicKey);
    saveSettings();
    updateCosignerTable();
    
    emit cosignerAdded(publicKey);
    showSuccess(tr("Cosigner Added"), tr("Public key added as cosigner."));
    
    return true;
}

bool MultisigWallet::removeCosigner(const QString& publicKey)
{
    if (m_cosigners.removeOne(publicKey)) {
        saveSettings();
        updateCosignerTable();
        
        emit cosignerRemoved(publicKey);
        showSuccess(tr("Cosigner Removed"), tr("Public key removed from cosigners."));
        return true;
    }
    return false;
}

QStringList MultisigWallet::getCosigners() const
{
    return m_cosigners;
}

QString MultisigWallet::getMultisigAddress() const
{
    return m_multisigAddress;
}

bool MultisigWallet::createUnsignedTransaction(const QString& recipient, double amount)
{
    if (recipient.trimmed().isEmpty()) {
        showError(tr("Invalid Recipient"), tr("Please enter a valid recipient address."));
        return false;
    }
    
    if (amount <= 0) {
        showError(tr("Invalid Amount"), tr("Please enter a valid amount greater than 0."));
        return false;
    }
    
    if (m_multisigAddress.isEmpty()) {
        showError(tr("No Multisig Address"), tr("Please create a multisig address first."));
        return false;
    }
    
    // Generate mock transaction hex
    QString txHex = QString("mock_tx_%1_%2_%3").arg(recipient).arg(amount).arg(QDateTime::currentDateTime().toSecsSinceEpoch());
    m_currentTransaction = txHex;
    m_pendingTransactions.append(txHex);
    
    updateTransactionTable();
    
    emit transactionCreated(txHex);
    showSuccess(tr("Transaction Created"), tr("Unsigned transaction created successfully."));
    
    return true;
}

bool MultisigWallet::signTransaction(const QString& transactionHex)
{
    if (transactionHex.isEmpty()) {
        showError(tr("No Transaction"), tr("Please create a transaction first."));
        return false;
    }
    
    // Generate mock signature
    QString signature = QString("mock_sig_%1_%2").arg(transactionHex).arg(QDateTime::currentDateTime().toSecsSinceEpoch());
    m_signedTransactions.append(signature);
    
    updateTransactionTable();
    
    emit transactionSigned(signature);
    showSuccess(tr("Transaction Signed"), tr("Transaction signed successfully."));
    
    return true;
}

bool MultisigWallet::combineSignatures(const QStringList& signatures)
{
    if (signatures.isEmpty()) {
        showError(tr("No Signatures"), tr("No signatures to combine."));
        return false;
    }
    
    if (signatures.size() < m_requiredSignatures) {
        showError(tr("Insufficient Signatures"), 
                 tr("Need at least %1 signatures, but only have %2.").arg(m_requiredSignatures).arg(signatures.size()));
        return false;
    }
    
    // Generate mock final transaction
    QString finalTx = QString("mock_final_tx_%1").arg(QDateTime::currentDateTime().toSecsSinceEpoch());
    
    updateTransactionTable();
    
    emit signaturesCombined(finalTx);
    showSuccess(tr("Signatures Combined"), tr("Transaction ready for broadcast with %1 signatures.").arg(signatures.size()));
    
    return true;
}

QString MultisigWallet::getTransactionStatus() const
{
    if (m_signedTransactions.size() >= m_requiredSignatures) {
        return tr("Ready to Broadcast");
    } else if (m_signedTransactions.size() > 0) {
        return tr("Partially Signed (%1/%2)").arg(m_signedTransactions.size()).arg(m_requiredSignatures);
    } else {
        return tr("Pending");
    }
}

bool MultisigWallet::saveConfiguration(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << getConfiguration();
    file.close();
    
    return true;
}

bool MultisigWallet::loadConfiguration(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    QString config = in.readAll();
    file.close();
    
    // Parse configuration (simplified for demo)
    QStringList lines = config.split('\n');
    for (const QString& line : lines) {
        if (line.startsWith("Address:")) {
            m_multisigAddress = line.mid(8).trimmed();
        } else if (line.startsWith("Required:")) {
            m_requiredSignatures = line.mid(9).trimmed().toInt();
        } else if (line.startsWith("Total:")) {
            m_totalSigners = line.mid(6).trimmed().toInt();
        } else if (line.startsWith("Cosigner:")) {
            QString cosigner = line.mid(9).trimmed();
            if (!cosigner.isEmpty() && !m_cosigners.contains(cosigner)) {
                m_cosigners.append(cosigner);
            }
        }
    }
    
    saveSettings();
    updateCosignerTable();
    m_addressEdit->setText(m_multisigAddress);
    m_requiredSpinBox->setValue(m_requiredSignatures);
    m_totalSpinBox->setValue(m_totalSigners);
    
    return true;
}

QString MultisigWallet::getConfiguration() const
{
    QString config;
    config += "# Shahcoin Multisig Wallet Configuration\n";
    config += "# Generated on: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n\n";
    config += "Address: " + m_multisigAddress + "\n";
    config += "Required: " + QString::number(m_requiredSignatures) + "\n";
    config += "Total: " + QString::number(m_totalSigners) + "\n\n";
    config += "# Cosigners:\n";
    for (const QString& cosigner : m_cosigners) {
        config += "Cosigner: " + cosigner + "\n";
    }
    config += "\n# Instructions:\n";
    config += "# 1. Share this configuration with all cosigners\n";
    config += "# 2. Each cosigner should import this configuration\n";
    config += "# 3. Create transactions and collect signatures\n";
    config += "# 4. Combine signatures to broadcast transactions\n";
    
    return config;
}

void MultisigWallet::onCreateMultisigClicked()
{
    int required = m_requiredSpinBox->value();
    int total = m_totalSpinBox->value();
    createMultisigAddress(required, total);
}

void MultisigWallet::onAddCosignerClicked()
{
    QString publicKey = m_cosignerEdit->text().trimmed();
    if (addCosigner(publicKey)) {
        m_cosignerEdit->clear();
    }
}

void MultisigWallet::onRemoveCosignerClicked()
{
    QList<QTableWidgetItem*> selectedItems = m_cosignerTable->selectedItems();
    if (selectedItems.isEmpty()) {
        showWarning(tr("No Selection"), tr("Please select a cosigner to remove."));
        return;
    }
    
    int row = selectedItems.first()->row();
    QString publicKey = m_cosignerTable->item(row, 0)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        tr("Remove Cosigner"), 
        tr("Are you sure you want to remove this cosigner?"),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        removeCosigner(publicKey);
    }
}

void MultisigWallet::onCreateTransactionClicked()
{
    QString recipient = m_recipientEdit->text().trimmed();
    double amount = m_amountEdit->text().toDouble();
    createUnsignedTransaction(recipient, amount);
}

void MultisigWallet::onSignTransactionClicked()
{
    if (m_currentTransaction.isEmpty()) {
        showWarning(tr("No Transaction"), tr("Please create a transaction first."));
        return;
    }
    signTransaction(m_currentTransaction);
}

void MultisigWallet::onCombineSignaturesClicked()
{
    if (m_signedTransactions.isEmpty()) {
        showWarning(tr("No Signatures"), tr("No signatures to combine."));
        return;
    }
    combineSignatures(m_signedTransactions);
}

void MultisigWallet::onSaveConfigClicked()
{
    if (m_multisigAddress.isEmpty()) {
        showWarning(tr("No Configuration"), tr("Please create a multisig address first."));
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, 
                                                   tr("Save Configuration"), 
                                                   "shahcoin_multisig_config.txt", 
                                                   tr("Text Files (*.txt);;All Files (*)"));
    
    if (!filePath.isEmpty()) {
        if (saveConfiguration(filePath)) {
            showSuccess(tr("Configuration Saved"), tr("Configuration saved to: %1").arg(filePath));
        } else {
            showError(tr("Save Failed"), tr("Could not save configuration to file."));
        }
    }
}

void MultisigWallet::onLoadConfigClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, 
                                                   tr("Load Configuration"), 
                                                   "", 
                                                   tr("Text Files (*.txt);;All Files (*)"));
    
    if (!filePath.isEmpty()) {
        if (loadConfiguration(filePath)) {
            showSuccess(tr("Configuration Loaded"), tr("Configuration loaded from: %1").arg(filePath));
        } else {
            showError(tr("Load Failed"), tr("Could not load configuration from file."));
        }
    }
}

void MultisigWallet::onRefreshClicked()
{
    refreshData();
    showSuccess(tr("Refreshed"), tr("Data has been updated."));
}

void MultisigWallet::refreshData()
{
    updateCosignerTable();
    updateTransactionTable();
}

void MultisigWallet::updateCosignerTable()
{
    m_cosignerTable->setRowCount(0);
    
    for (const QString& cosigner : m_cosigners) {
        int row = m_cosignerTable->rowCount();
        m_cosignerTable->insertRow(row);
        
        m_cosignerTable->setItem(row, 0, new QTableWidgetItem(cosigner));
        m_cosignerTable->setItem(row, 1, new QTableWidgetItem(tr("Active")));
        m_cosignerTable->setItem(row, 2, new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm")));
    }
}

void MultisigWallet::updateTransactionTable()
{
    m_transactionTable->setRowCount(0);
    
    // Add pending transactions
    for (const QString& tx : m_pendingTransactions) {
        int row = m_transactionTable->rowCount();
        m_transactionTable->insertRow(row);
        
        m_transactionTable->setItem(row, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm")));
        m_transactionTable->setItem(row, 1, new QTableWidgetItem("S..."));
        m_transactionTable->setItem(row, 2, new QTableWidgetItem("0.100000 SHAH"));
        m_transactionTable->setItem(row, 3, new QTableWidgetItem(QString("%1/%2").arg(m_signedTransactions.size()).arg(m_requiredSignatures)));
        m_transactionTable->setItem(row, 4, new QTableWidgetItem(getTransactionStatus()));
    }
}

void MultisigWallet::showWarning(const QString& title, const QString& message)
{
    QMessageBox::warning(this, title, message);
}

void MultisigWallet::showSuccess(const QString& title, const QString& message)
{
    QMessageBox::information(this, title, message);
}

void MultisigWallet::showError(const QString& title, const QString& message)
{
    QMessageBox::critical(this, title, message);
}

bool MultisigWallet::validatePublicKey(const QString& publicKey) const
{
    // Basic public key validation (simplified for demo)
    // In a real implementation, this would use proper public key validation
    return publicKey.length() >= 64 && publicKey.length() <= 130 && 
           (publicKey.startsWith("04") || publicKey.startsWith("02") || publicKey.startsWith("03"));
}

QString MultisigWallet::generateMockMultisigAddress() const
{
    // Generate a mock multisig address
    QDateTime now = QDateTime::currentDateTime();
    qint64 seed = now.toSecsSinceEpoch();
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    
    return QString("S%1%2%3").arg(m_requiredSignatures).arg(m_totalSigners).arg(seed, 0, 16);
}

QStringList MultisigWallet::generateMockTransactions() const
{
    QStringList transactions;
    for (int i = 0; i < 5; ++i) {
        QString date = QDateTime::currentDateTime().addSecs(-i * 3600).toString("yyyy-MM-dd hh:mm");
        QString recipient = "S...";
        QString amount = QString("%1 SHAH").arg((i + 1) * 0.1, 0, 'f', 6);
        QString signatures = QString("%1/%2").arg(qMin(i + 1, m_requiredSignatures)).arg(m_requiredSignatures);
        QString status = (i + 1 >= m_requiredSignatures) ? tr("Ready") : tr("Pending");
        
        transactions.append(QString("%1|%2|%3|%4|%5").arg(date, recipient, amount, signatures, status));
    }
    return transactions;
}
