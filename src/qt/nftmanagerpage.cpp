// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/nftmanagerpage.h>

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

#include "ui_nftmanagerpage.h"

NFTManagerPage::NFTManagerPage(const PlatformStyle* _platformStyle, QWidget* parent)
    : QWidget(parent),
      platformStyle(_platformStyle),
      updateTimer(new QTimer(this))
{
    setupUI();
    connectSignals();

    // Set up timer for periodic updates
    updateTimer->setInterval(UPDATE_INTERVAL_MS);
    connect(updateTimer, &QTimer::timeout, this, &NFTManagerPage::updateNFTList);
    updateTimer->start();
}

NFTManagerPage::~NFTManagerPage()
{
    if (updateTimer) {
        updateTimer->stop();
    }
}

void NFTManagerPage::setupUI()
{
    // Load UI from form
    Ui::NFTManagerPage ui;
    ui.setupUi(this);

    // Store UI component pointers
    lineEdit_NFTName = ui.lineEdit_NFTName;
    textEdit_NFTDescription = ui.textEdit_NFTDescription;
    lineEdit_ImagePath = ui.lineEdit_ImagePath;
    pushButton_BrowseImage = ui.pushButton_BrowseImage;
    lineEdit_Collection = ui.lineEdit_Collection;
    label_NFTPreview = ui.label_NFTPreview;
    label_MintingFee = ui.label_MintingFee;
    pushButton_MintNFT = ui.pushButton_MintNFT;
    pushButton_ClearForm = ui.pushButton_ClearForm;

    tableWidget_MyNFTs = ui.tableWidget_MyNFTs;
    lineEdit_SearchNFTs = ui.lineEdit_SearchNFTs;
    comboBox_FilterNFTs = ui.comboBox_FilterNFTs;
    pushButton_RefreshNFTs = ui.pushButton_RefreshNFTs;

    comboBox_SelectNFT = ui.comboBox_SelectNFT;
    lineEdit_RecipientAddress = ui.lineEdit_RecipientAddress;
    label_TransferFee = ui.label_TransferFee;
    pushButton_TransferNFT = ui.pushButton_TransferNFT;

    // Set up table headers
    tableWidget_MyNFTs->setColumnCount(5);
    tableWidget_MyNFTs->setHorizontalHeaderLabels({
        tr("NFT ID"),
        tr("Name"),
        tr("Collection"),
        tr("Owner"),
        tr("Actions")
    });

    // Set table properties
    tableWidget_MyNFTs->setAlternatingRowColors(true);
    tableWidget_MyNFTs->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget_MyNFTs->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget_MyNFTs->horizontalHeader()->setStretchLastSection(true);

    // Set up filter combo box
    comboBox_FilterNFTs->addItem(tr("All NFTs"), "all");
    comboBox_FilterNFTs->addItem(tr("My NFTs"), "owned");
    comboBox_FilterNFTs->addItem(tr("Created"), "created");

    // Set up fee labels
    label_MintingFee->setText(tr("Minting Fee: 100 SHAH"));
    label_TransferFee->setText(tr("Transfer Fee: 0.001 SHAH"));

    // Initial update
    updateNFTList();
}

void NFTManagerPage::connectSignals()
{
    // Mint NFT tab
    connect(pushButton_BrowseImage, &QPushButton::clicked, this, &NFTManagerPage::onBrowseImage);
    connect(pushButton_MintNFT, &QPushButton::clicked, this, &NFTManagerPage::onMintNFT);
    connect(pushButton_ClearForm, &QPushButton::clicked, this, &NFTManagerPage::onClearForm);
    connect(lineEdit_ImagePath, &QLineEdit::textChanged, this, &NFTManagerPage::onPreviewUpdate);

    // My NFTs tab
    connect(pushButton_RefreshNFTs, &QPushButton::clicked, this, &NFTManagerPage::onRefreshNFTs);
    connect(lineEdit_SearchNFTs, &QLineEdit::textChanged, this, &NFTManagerPage::onSearchChanged);
    connect(comboBox_FilterNFTs, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &NFTManagerPage::onFilterChanged);

    // Transfer NFT tab
    connect(pushButton_TransferNFT, &QPushButton::clicked, this, &NFTManagerPage::onTransferNFT);
}

void NFTManagerPage::setClientModel(ClientModel* _clientModel)
{
    clientModel = _clientModel;
}

void NFTManagerPage::setWalletModel(WalletModel* _walletModel)
{
    walletModel = _walletModel;
    if (walletModel) {
        updateNFTList();
    }
}

void NFTManagerPage::updateDisplayUnit()
{
    if (walletModel && walletModel->getOptionsModel()) {
        displayUnit = walletModel->getOptionsModel()->getDisplayUnit();
    }
}

void NFTManagerPage::updateNFTList()
{
    if (!walletModel) return;

    tableWidget_MyNFTs->setRowCount(0);
    
    // Get owned NFTs from wallet model
    QStringList ownedNFTs = walletModel->getOwnedNFTs();
    
    for (int i = 0; i < ownedNFTs.size(); ++i) {
        // TODO: Parse NFT data and populate table
        // For now, show empty table
    }

    // Update transfer combo box
    comboBox_SelectNFT->clear();
    for (const QString& nftId : ownedNFTs) {
        comboBox_SelectNFT->addItem(nftId, nftId);
    }
}

void NFTManagerPage::onMintNFT()
{
    if (!walletModel) {
        showNFTError(tr("Wallet not available"));
        return;
    }

    // Validate form inputs
    QString name = lineEdit_NFTName->text().trimmed();
    QString description = textEdit_NFTDescription->toPlainText().trimmed();
    QString imagePath = lineEdit_ImagePath->text().trimmed();
    QString collection = lineEdit_Collection->text().trimmed();

    if (name.isEmpty()) {
        showNFTError(tr("NFT name is required"));
        return;
    }

    if (description.isEmpty()) {
        showNFTError(tr("NFT description is required"));
        return;
    }

    if (imagePath.isEmpty()) {
        showNFTError(tr("Image path is required"));
        return;
    }

    // Check if file exists
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists()) {
        showNFTError(tr("Image file does not exist"));
        return;
    }

    QString error;
    if (walletModel->mintNFT(name, description, imagePath, collection, error)) {
        showNFTSuccess(tr("NFT minted successfully"));
        onClearForm();
        updateNFTList();
    } else {
        showNFTError(error);
    }
}

void NFTManagerPage::onTransferNFT()
{
    if (!walletModel) {
        showNFTError(tr("Wallet not available"));
        return;
    }

    // Get selected NFT and recipient
    QString nftId = comboBox_SelectNFT->currentData().toString();
    QString recipient = lineEdit_RecipientAddress->text().trimmed();

    if (nftId.isEmpty()) {
        showNFTError(tr("Please select an NFT to transfer"));
        return;
    }

    if (recipient.isEmpty()) {
        showNFTError(tr("Recipient address is required"));
        return;
    }

    // Validate address format
    if (!walletModel->validateAddress(recipient)) {
        showNFTError(tr("Invalid recipient address"));
        return;
    }

    QString error;
    if (walletModel->transferNFT(nftId, recipient, error)) {
        showNFTSuccess(tr("NFT transferred successfully"));
        lineEdit_RecipientAddress->clear();
        updateNFTList();
    } else {
        showNFTError(error);
    }
}

void NFTManagerPage::onBrowseImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select NFT Image"), "",
        tr("Image Files (*.png *.jpg *.jpeg *.gif *.bmp);;All Files (*)"));

    if (!fileName.isEmpty()) {
        lineEdit_ImagePath->setText(fileName);
        onPreviewUpdate();
    }
}

void NFTManagerPage::onClearForm()
{
    lineEdit_NFTName->clear();
    textEdit_NFTDescription->clear();
    lineEdit_ImagePath->clear();
    lineEdit_Collection->clear();
    label_NFTPreview->clear();
    label_NFTPreview->setText(tr("NFT Preview"));
}

void NFTManagerPage::onRefreshNFTs()
{
    updateNFTList();
    showNFTSuccess(tr("NFT list refreshed"));
}

void NFTManagerPage::onPreviewUpdate()
{
    QString imagePath = lineEdit_ImagePath->text();
    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            // Scale to fit preview area
            pixmap = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            label_NFTPreview->setPixmap(pixmap);
        } else {
            label_NFTPreview->setText(tr("Invalid image"));
        }
    } else {
        label_NFTPreview->setText(tr("NFT Preview"));
    }
}

void NFTManagerPage::onFilterChanged()
{
    // TODO: Implement filtering logic
    updateNFTList();
}

void NFTManagerPage::onSearchChanged()
{
    // TODO: Implement search logic
    updateNFTList();
}

void NFTManagerPage::showNFTError(const QString& message)
{
    Q_EMIT this->message(tr("NFT Error"), message, CClientUIInterface::MSG_ERROR);
}

void NFTManagerPage::showNFTSuccess(const QString& message)
{
    Q_EMIT this->message(tr("NFT Success"), message, CClientUIInterface::MSG_INFORMATION);
}

