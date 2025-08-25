// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/tokencreatorpage.h>

#include <qt/clientmodel.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/shahcoinamountfield.h>
#include <qt/shahcoinunits.h>
#include <qt/walletmodel.h>

#include <interfaces/node.h>
#include <interfaces/wallet.h>
#include <node/interface_ui.h>
#include <util/strencodings.h>
#include <validation.h>

#include <QApplication>
#include <QDateTime>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

#include "ui_tokencreatorpage.h"

TokenCreatorPage::TokenCreatorPage(const PlatformStyle* _platformStyle, QWidget* parent)
    : QWidget(parent),
      platformStyle(_platformStyle),
      updateTimer(new QTimer(this))
{
    setupUI();
    connectSignals();

    // Set up timer for periodic updates
    updateTimer->setInterval(UPDATE_INTERVAL_MS);
    connect(updateTimer, &QTimer::timeout, this, &TokenCreatorPage::updateTokenList);
    updateTimer->start();
}

TokenCreatorPage::~TokenCreatorPage()
{
    if (updateTimer) {
        updateTimer->stop();
    }
}

void TokenCreatorPage::setupUI()
{
    // Load UI from form
    Ui::TokenCreatorPage ui;
    ui.setupUi(this);

    // Store UI component pointers
    lineEdit_TokenName = ui.lineEdit_TokenName;
    lineEdit_TokenSymbol = ui.lineEdit_TokenSymbol;
    amountField_TotalSupply = ui.amountField_TotalSupply;
    spinBox_Decimals = ui.spinBox_Decimals;
    textEdit_TokenDescription = ui.textEdit_TokenDescription;
    label_CreationFee = ui.label_CreationFee;
    pushButton_CreateToken = ui.pushButton_CreateToken;
    pushButton_ClearForm = ui.pushButton_ClearForm;

    tableWidget_MyTokens = ui.tableWidget_MyTokens;
    lineEdit_SearchTokens = ui.lineEdit_SearchTokens;
    pushButton_RefreshTokens = ui.pushButton_RefreshTokens;

    comboBox_SelectToken = ui.comboBox_SelectToken;
    label_TokenBalance = ui.label_TokenBalance;
    amountField_TransferAmount = ui.amountField_TransferAmount;
    lineEdit_RecipientAddress = ui.lineEdit_RecipientAddress;
    label_TransferFee = ui.label_TransferFee;
    pushButton_TransferToken = ui.pushButton_TransferToken;

    // Set up table headers
    tableWidget_MyTokens->setColumnCount(6);
    tableWidget_MyTokens->setHorizontalHeaderLabels({
        tr("Token ID"),
        tr("Name"),
        tr("Symbol"),
        tr("Supply"),
        tr("Balance"),
        tr("Actions")
    });

    // Set table properties
    tableWidget_MyTokens->setAlternatingRowColors(true);
    tableWidget_MyTokens->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget_MyTokens->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget_MyTokens->horizontalHeader()->setStretchLastSection(true);

    // Set up amount fields
    amountField_TotalSupply->setDisplayUnit(ShahcoinUnits::SHAH);
    amountField_TotalSupply->setMinimum(1 * COIN);
    amountField_TotalSupply->setMaximum(1000000000 * COIN); // 1 billion SHAH max

    amountField_TransferAmount->setDisplayUnit(ShahcoinUnits::SHAH);
    amountField_TransferAmount->setMinimum(0);
    amountField_TransferAmount->setMaximum(1000000000 * COIN);

    // Set up decimals spin box
    spinBox_Decimals->setMinimum(0);
    spinBox_Decimals->setMaximum(18);
    spinBox_Decimals->setValue(8);

    // Set up fee labels
    label_CreationFee->setText(tr("Creation Fee: 500 SHAH"));
    label_TransferFee->setText(tr("Transfer Fee: 0.001 SHAH"));

    // Initial update
    updateTokenList();
}

void TokenCreatorPage::connectSignals()
{
    // Create Token tab
    connect(pushButton_CreateToken, &QPushButton::clicked, this, &TokenCreatorPage::onCreateToken);
    connect(pushButton_ClearForm, &QPushButton::clicked, this, &TokenCreatorPage::onClearForm);

    // My Tokens tab
    connect(pushButton_RefreshTokens, &QPushButton::clicked, this, &TokenCreatorPage::onRefreshTokens);
    connect(lineEdit_SearchTokens, &QLineEdit::textChanged, this, &TokenCreatorPage::onSearchChanged);

    // Transfer Tokens tab
    connect(pushButton_TransferToken, &QPushButton::clicked, this, &TokenCreatorPage::onTransferToken);
    connect(comboBox_SelectToken, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &TokenCreatorPage::onTokenSelected);
}

void TokenCreatorPage::setClientModel(ClientModel* _clientModel)
{
    clientModel = _clientModel;
}

void TokenCreatorPage::setWalletModel(WalletModel* _walletModel)
{
    walletModel = _walletModel;
    if (walletModel) {
        updateTokenList();
    }
}

void TokenCreatorPage::updateDisplayUnit()
{
    if (walletModel && walletModel->getOptionsModel()) {
        displayUnit = walletModel->getOptionsModel()->getDisplayUnit();
    }
}

void TokenCreatorPage::updateTokenList()
{
    if (!walletModel) return;

    tableWidget_MyTokens->setRowCount(0);
    
    // Get owned tokens from wallet model
    QStringList ownedTokens = walletModel->getOwnedTokens();
    
    for (int i = 0; i < ownedTokens.size(); ++i) {
        // TODO: Parse token data and populate table
        // For now, show empty table
    }

    // Update transfer combo box
    comboBox_SelectToken->clear();
    for (const QString& tokenId : ownedTokens) {
        comboBox_SelectToken->addItem(tokenId, tokenId);
    }
}

void TokenCreatorPage::onCreateToken()
{
    if (!walletModel) {
        showTokenError(tr("Wallet not available"));
        return;
    }

    // Validate form inputs
    QString name = lineEdit_TokenName->text().trimmed();
    QString symbol = lineEdit_TokenSymbol->text().trimmed();
    CAmount totalSupply = amountField_TotalSupply->value();
    int decimals = spinBox_Decimals->value();
    QString description = textEdit_TokenDescription->toPlainText().trimmed();

    if (name.isEmpty()) {
        showTokenError(tr("Token name is required"));
        return;
    }

    if (symbol.isEmpty()) {
        showTokenError(tr("Token symbol is required"));
        return;
    }

    if (totalSupply <= 0) {
        showTokenError(tr("Total supply must be greater than 0"));
        return;
    }

    if (decimals < 0 || decimals > 18) {
        showTokenError(tr("Decimals must be between 0 and 18"));
        return;
    }

    QString error;
    if (walletModel->createToken(name, symbol, totalSupply, decimals, description, error)) {
        showTokenSuccess(tr("Token created successfully"));
        onClearForm();
        updateTokenList();
    } else {
        showTokenError(error);
    }
}

void TokenCreatorPage::onTransferToken()
{
    if (!walletModel) {
        showTokenError(tr("Wallet not available"));
        return;
    }

    // Get selected token and transfer details
    QString tokenId = comboBox_SelectToken->currentData().toString();
    CAmount amount = amountField_TransferAmount->value();
    QString recipient = lineEdit_RecipientAddress->text().trimmed();

    if (tokenId.isEmpty()) {
        showTokenError(tr("Please select a token to transfer"));
        return;
    }

    if (amount <= 0) {
        showTokenError(tr("Transfer amount must be greater than 0"));
        return;
    }

    if (recipient.isEmpty()) {
        showTokenError(tr("Recipient address is required"));
        return;
    }

    // Validate address format
    if (!walletModel->validateAddress(recipient)) {
        showTokenError(tr("Invalid recipient address"));
        return;
    }

    // Check token balance
    CAmount balance = walletModel->getTokenBalance(tokenId);
    if (amount > balance) {
        showTokenError(tr("Insufficient token balance"));
        return;
    }

    QString error;
    if (walletModel->transferToken(tokenId, recipient, amount, error)) {
        showTokenSuccess(tr("Token transferred successfully"));
        lineEdit_RecipientAddress->clear();
        amountField_TransferAmount->setValue(0);
        updateTokenList();
    } else {
        showTokenError(error);
    }
}

void TokenCreatorPage::onClearForm()
{
    lineEdit_TokenName->clear();
    lineEdit_TokenSymbol->clear();
    amountField_TotalSupply->setValue(1000000 * COIN); // Default 1M supply
    spinBox_Decimals->setValue(8);
    textEdit_TokenDescription->clear();
}

void TokenCreatorPage::onRefreshTokens()
{
    updateTokenList();
    showTokenSuccess(tr("Token list refreshed"));
}

void TokenCreatorPage::onSearchChanged()
{
    // TODO: Implement search logic
    updateTokenList();
}

void TokenCreatorPage::onTokenSelected()
{
    if (!walletModel) return;

    QString tokenId = comboBox_SelectToken->currentData().toString();
    if (!tokenId.isEmpty()) {
        CAmount balance = walletModel->getTokenBalance(tokenId);
        label_TokenBalance->setText(tr("Balance: %1").arg(
            ShahcoinUnits::formatWithUnit(displayUnit, balance, false, ShahcoinUnits::SeparatorStyle::STANDARD)));
        
        // Set maximum transfer amount
        amountField_TransferAmount->setMaximum(balance);
    } else {
        label_TokenBalance->setText(tr("Balance: 0"));
        amountField_TransferAmount->setMaximum(0);
    }
}

void TokenCreatorPage::showTokenError(const QString& message)
{
    Q_EMIT this->message(tr("Token Error"), message, CClientUIInterface::MSG_ERROR);
}

void TokenCreatorPage::showTokenSuccess(const QString& message)
{
    Q_EMIT this->message(tr("Token Success"), message, CClientUIInterface::MSG_INFORMATION);
}
