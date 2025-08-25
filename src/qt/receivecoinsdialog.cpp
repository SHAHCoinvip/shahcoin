// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <wallet/wallet.h>

#include <qt/receivecoinsdialog.h>
#include <qt/forms/ui_receivecoinsdialog.h>

#include <qt/addresstablemodel.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/receiverequestdialog.h>
#include <qt/recentrequeststablemodel.h>
#include <qt/walletmodel.h>

#include <QAction>
#include <QCursor>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QTextDocument>
#include <QGroupBox>
#include <QFormLayout>
#include <QStandardPaths>
#include <QUrl>

ReceiveCoinsDialog::ReceiveCoinsDialog(const PlatformStyle *_platformStyle, QWidget *parent) :
    QDialog(parent, GUIUtil::dialog_flags),
    ui(new Ui::ReceiveCoinsDialog),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);

    if (!_platformStyle->getImagesOnButtons()) {
        ui->clearButton->setIcon(QIcon());
        ui->receiveButton->setIcon(QIcon());
        ui->showRequestButton->setIcon(QIcon());
        ui->removeRequestButton->setIcon(QIcon());
    } else {
        ui->clearButton->setIcon(_platformStyle->SingleColorIcon(":/icons/remove"));
        ui->receiveButton->setIcon(_platformStyle->SingleColorIcon(":/icons/receiving_addresses"));
        ui->showRequestButton->setIcon(_platformStyle->SingleColorIcon(":/icons/eye"));
        ui->removeRequestButton->setIcon(_platformStyle->SingleColorIcon(":/icons/remove"));
    }

    // context menu
    contextMenu = new QMenu(this);
    contextMenu->addAction(tr("Copy &URI"), this, &ReceiveCoinsDialog::copyURI);
    contextMenu->addAction(tr("&Copy address"), this, &ReceiveCoinsDialog::copyAddress);
    copyLabelAction = contextMenu->addAction(tr("Copy &label"), this, &ReceiveCoinsDialog::copyLabel);
    copyMessageAction = contextMenu->addAction(tr("Copy &message"), this, &ReceiveCoinsDialog::copyMessage);
    copyAmountAction = contextMenu->addAction(tr("Copy &amount"), this, &ReceiveCoinsDialog::copyAmount);
    connect(ui->recentRequestsView, &QWidget::customContextMenuRequested, this, &ReceiveCoinsDialog::showMenu);

    connect(ui->clearButton, &QPushButton::clicked, this, &ReceiveCoinsDialog::clear);

    // Add Address Book button to the horizontal layout
    QPushButton* addressBookButton = new QPushButton(tr("📒 Address Book"), this);
    addressBookButton->setToolTip(tr("Manage your address book - add labels to your addresses"));
    if (_platformStyle->getImagesOnButtons()) {
        addressBookButton->setIcon(_platformStyle->SingleColorIcon(":/icons/address-book"));
    }
    connect(addressBookButton, &QPushButton::clicked, this, &ReceiveCoinsDialog::on_manageAddressBookClicked);
    
    // Insert the address book button into the horizontal layout after the clear button
    QHBoxLayout* buttonLayout = qobject_cast<QHBoxLayout*>(ui->clearButton->parentWidget()->layout());
    if (buttonLayout) {
        buttonLayout->insertWidget(buttonLayout->indexOf(ui->clearButton) + 1, addressBookButton);
    }
    
    // Setup QR Code section
    setupQRCodeSection();

    QTableView* tableView = ui->recentRequestsView;
    tableView->verticalHeader()->hide();
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);

    QSettings settings;
    if (!tableView->horizontalHeader()->restoreState(settings.value("RecentRequestsViewHeaderState").toByteArray())) {
        tableView->setColumnWidth(RecentRequestsTableModel::Date, DATE_COLUMN_WIDTH);
        tableView->setColumnWidth(RecentRequestsTableModel::Label, LABEL_COLUMN_WIDTH);
        tableView->setColumnWidth(RecentRequestsTableModel::Amount, AMOUNT_MINIMUM_COLUMN_WIDTH);
        tableView->horizontalHeader()->setMinimumSectionSize(MINIMUM_COLUMN_WIDTH);
        tableView->horizontalHeader()->setStretchLastSection(true);
    }
}

void ReceiveCoinsDialog::setModel(WalletModel *_model)
{
    this->model = _model;

    if(_model && _model->getOptionsModel())
    {
        _model->getRecentRequestsTableModel()->sort(RecentRequestsTableModel::Date, Qt::DescendingOrder);
        connect(_model->getOptionsModel(), &OptionsModel::displayUnitChanged, this, &ReceiveCoinsDialog::updateDisplayUnit);
        updateDisplayUnit();

        QTableView* tableView = ui->recentRequestsView;
        tableView->setModel(_model->getRecentRequestsTableModel());
        tableView->sortByColumn(RecentRequestsTableModel::Date, Qt::DescendingOrder);

        connect(tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged, this,
            &ReceiveCoinsDialog::recentRequestsView_selectionChanged);

        // Populate address type dropdown and select default
        auto add_address_type = [&](OutputType type, const QString& text, const QString& tooltip) {
            const auto index = ui->addressType->count();
            ui->addressType->addItem(text, (int) type);
            ui->addressType->setItemData(index, tooltip, Qt::ToolTipRole);
            if (model->wallet().getDefaultAddressType() == type) ui->addressType->setCurrentIndex(index);
        };
        add_address_type(OutputType::LEGACY, tr("Base58 (Legacy)"), tr("Not recommended due to higher fees and less protection against typos."));
        add_address_type(OutputType::P2SH_SEGWIT, tr("Base58 (P2SH-SegWit)"), tr("Generates an address compatible with older wallets."));
        add_address_type(OutputType::BECH32, tr("Bech32 (SegWit)"), tr("Generates a native segwit address (BIP-173). Some old wallets don't support it."));
        if (model->wallet().taprootEnabled()) {
            add_address_type(OutputType::BECH32M, tr("Bech32m (Taproot)"), tr("Bech32m (BIP-350) is an upgrade to Bech32, wallet support is still limited."));
        }

        // Set the button to be enabled or disabled based on whether the wallet can give out new addresses.
        ui->receiveButton->setEnabled(model->wallet().canGetAddresses());

        // Enable/disable the receive button if the wallet is now able/unable to give out new addresses.
        connect(model, &WalletModel::canGetAddressesChanged, [this] {
            ui->receiveButton->setEnabled(model->wallet().canGetAddresses());
        });
        
        // Update QR code when model is set
        updateQRCode();
    }
}

void ReceiveCoinsDialog::setupQRCodeSection()
{
    // Create QR Code section
    QGroupBox* qrGroup = new QGroupBox(tr("📱 QR Code & Payment Request"), this);
    qrGroup->setToolTip(tr("Generate QR codes for receiving SHAH payments"));
    
    QVBoxLayout* qrLayout = new QVBoxLayout(qrGroup);
    
    // QR Code display
    m_qrCodeLabel = new QLabel(this);
    m_qrCodeLabel->setMinimumSize(200, 200);
    m_qrCodeLabel->setMaximumSize(300, 300);
    m_qrCodeLabel->setAlignment(Qt::AlignCenter);
    m_qrCodeLabel->setStyleSheet("QLabel { border: 2px solid #cccccc; border-radius: 8px; background-color: white; }");
    m_qrCodeLabel->setText(tr("QR Code will appear here"));
    m_qrCodeLabel->setToolTip(tr("Scan this QR code with a mobile wallet to send SHAH"));
    
    // Payment details form
    QFormLayout* formLayout = new QFormLayout();
    
    m_qrAmountEdit = new QLineEdit(this);
    m_qrAmountEdit->setPlaceholderText(tr("0.00"));
    m_qrAmountEdit->setToolTip(tr("Optional: Enter amount to receive"));
    connect(m_qrAmountEdit, &QLineEdit::textChanged, this, &ReceiveCoinsDialog::onQRAmountChanged);
    
    m_qrLabelEdit = new QLineEdit(this);
    m_qrLabelEdit->setPlaceholderText(tr("Optional label"));
    m_qrLabelEdit->setToolTip(tr("Optional: Label for this payment request"));
    connect(m_qrLabelEdit, &QLineEdit::textChanged, this, &ReceiveCoinsDialog::onQRLabelChanged);
    
    m_qrMessageEdit = new QLineEdit(this);
    m_qrMessageEdit->setPlaceholderText(tr("Optional message"));
    m_qrMessageEdit->setToolTip(tr("Optional: Message to include with payment"));
    connect(m_qrMessageEdit, &QLineEdit::textChanged, this, &ReceiveCoinsDialog::onQRMessageChanged);
    
    formLayout->addRow(tr("Amount (SHAH):"), m_qrAmountEdit);
    formLayout->addRow(tr("Label:"), m_qrLabelEdit);
    formLayout->addRow(tr("Message:"), m_qrMessageEdit);
    
    // Action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_copyAddressButton = new QPushButton(tr("📋 Copy Address"), this);
    m_copyAddressButton->setToolTip(tr("Copy wallet address to clipboard"));
    connect(m_copyAddressButton, &QPushButton::clicked, this, &ReceiveCoinsDialog::onCopyAddressClicked);
    
    m_copyURIButton = new QPushButton(tr("🔗 Copy URI"), this);
    m_copyURIButton->setToolTip(tr("Copy payment URI to clipboard"));
    connect(m_copyURIButton, &QPushButton::clicked, this, &ReceiveCoinsDialog::onCopyURIClicked);
    
    m_saveQRButton = new QPushButton(tr("💾 Save QR"), this);
    m_saveQRButton->setToolTip(tr("Save QR code as PNG image"));
    connect(m_saveQRButton, &QPushButton::clicked, this, &ReceiveCoinsDialog::onSaveQRClicked);
    
    buttonLayout->addWidget(m_copyAddressButton);
    buttonLayout->addWidget(m_copyURIButton);
    buttonLayout->addWidget(m_saveQRButton);
    
    // Add all components to QR layout
    qrLayout->addWidget(m_qrCodeLabel, 0, Qt::AlignCenter);
    qrLayout->addLayout(formLayout);
    qrLayout->addLayout(buttonLayout);
    
    // Add QR section to main layout (find the main layout and insert after the form)
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        // Find the form widget and insert QR section after it
        for (int i = 0; i < mainLayout->count(); ++i) {
            QLayoutItem* item = mainLayout->itemAt(i);
            if (item->widget() && item->widget()->objectName().contains("form")) {
                mainLayout->insertWidget(i + 1, qrGroup);
                break;
            }
        }
        // If we couldn't find the form, add at the end
        if (!qrGroup->parent()) {
            mainLayout->addWidget(qrGroup);
        }
    }
    
    // Generate initial QR code
    generateQRCode();
}

ReceiveCoinsDialog::~ReceiveCoinsDialog()
{
    QSettings settings;
    settings.setValue("RecentRequestsViewHeaderState", ui->recentRequestsView->horizontalHeader()->saveState());
    delete ui;
}

void ReceiveCoinsDialog::clear()
{
    ui->reqAmount->clear();
    ui->reqLabel->setText("");
    ui->reqMessage->setText("");
    updateDisplayUnit();
    
    // Clear QR code fields
    if (m_qrAmountEdit) m_qrAmountEdit->clear();
    if (m_qrLabelEdit) m_qrLabelEdit->clear();
    if (m_qrMessageEdit) m_qrMessageEdit->clear();
    
    // Regenerate QR code
    generateQRCode();
}

void ReceiveCoinsDialog::reject()
{
    clear();
}

void ReceiveCoinsDialog::accept()
{
    clear();
}

void ReceiveCoinsDialog::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        ui->reqAmount->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
    }
}

void ReceiveCoinsDialog::on_receiveButton_clicked()
{
    if(!model || !model->getOptionsModel() || !model->getAddressTableModel() || !model->getRecentRequestsTableModel())
        return;

    QString address;
    QString label = ui->reqLabel->text();
    /* Generate new receiving address */
    const OutputType address_type = (OutputType)ui->addressType->currentData().toInt();
    address = model->getAddressTableModel()->addRow(AddressTableModel::Receive, label, "", address_type);

    switch(model->getAddressTableModel()->getEditStatus())
    {
    case AddressTableModel::EditStatus::OK: {
        // Success
        SendCoinsRecipient info(address, label,
            ui->reqAmount->value(), ui->reqMessage->text());
        ReceiveRequestDialog *dialog = new ReceiveRequestDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setModel(model);
        dialog->setInfo(info);
        dialog->show();

        /* Store request for later reference */
        model->getRecentRequestsTableModel()->addNewRequest(info);
        break;
    }
    case AddressTableModel::EditStatus::WALLET_UNLOCK_FAILURE:
        QMessageBox::critical(this, windowTitle(),
            tr("Could not unlock wallet."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case AddressTableModel::EditStatus::KEY_GENERATION_FAILURE:
        QMessageBox::critical(this, windowTitle(),
            tr("Could not generate new %1 address").arg(QString::fromStdString(FormatOutputType(address_type))),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    // These aren't valid return values for our action
    case AddressTableModel::EditStatus::INVALID_ADDRESS:
    case AddressTableModel::EditStatus::DUPLICATE_ADDRESS:
    case AddressTableModel::EditStatus::NO_CHANGES:
        assert(false);
    }
    clear();
}

void ReceiveCoinsDialog::on_recentRequestsView_doubleClicked(const QModelIndex &index)
{
    const RecentRequestsTableModel *submodel = model->getRecentRequestsTableModel();
    ReceiveRequestDialog *dialog = new ReceiveRequestDialog(this);
    dialog->setModel(model);
    dialog->setInfo(submodel->entry(index.row()).recipient);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ReceiveCoinsDialog::recentRequestsView_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // Enable Show/Remove buttons only if anything is selected.
    bool enable = !ui->recentRequestsView->selectionModel()->selectedRows().isEmpty();
    ui->showRequestButton->setEnabled(enable);
    ui->removeRequestButton->setEnabled(enable);
}

void ReceiveCoinsDialog::on_showRequestButton_clicked()
{
    if(!model || !model->getRecentRequestsTableModel() || !ui->recentRequestsView->selectionModel())
        return;
    QModelIndexList selection = ui->recentRequestsView->selectionModel()->selectedRows();

    for (const QModelIndex& index : selection) {
        on_recentRequestsView_doubleClicked(index);
    }
}

void ReceiveCoinsDialog::on_removeRequestButton_clicked()
{
    if(!model || !model->getRecentRequestsTableModel() || !ui->recentRequestsView->selectionModel())
        return;
    QModelIndexList selection = ui->recentRequestsView->selectionModel()->selectedRows();
    if(selection.empty())
        return;
    // correct for selection mode ContiguousSelection
    QModelIndex firstIndex = selection.at(0);
    model->getRecentRequestsTableModel()->removeRows(firstIndex.row(), selection.length(), firstIndex.parent());
}

QModelIndex ReceiveCoinsDialog::selectedRow()
{
    if(!model || !model->getRecentRequestsTableModel() || !ui->recentRequestsView->selectionModel())
        return QModelIndex();
    QModelIndexList selection = ui->recentRequestsView->selectionModel()->selectedRows();
    if(selection.empty())
        return QModelIndex();
    // correct for selection mode ContiguousSelection
    QModelIndex firstIndex = selection.at(0);
    return firstIndex;
}

// copy column of selected row to clipboard
void ReceiveCoinsDialog::copyColumnToClipboard(int column)
{
    QModelIndex firstIndex = selectedRow();
    if (!firstIndex.isValid()) {
        return;
    }
    GUIUtil::setClipboard(model->getRecentRequestsTableModel()->index(firstIndex.row(), column).data(Qt::EditRole).toString());
}

// context menu
void ReceiveCoinsDialog::showMenu(const QPoint &point)
{
    const QModelIndex sel = selectedRow();
    if (!sel.isValid()) {
        return;
    }

    // disable context menu actions when appropriate
    const RecentRequestsTableModel* const submodel = model->getRecentRequestsTableModel();
    const RecentRequestEntry& req = submodel->entry(sel.row());
    copyLabelAction->setDisabled(req.recipient.label.isEmpty());
    copyMessageAction->setDisabled(req.recipient.message.isEmpty());
    copyAmountAction->setDisabled(req.recipient.amount == 0);

    contextMenu->exec(QCursor::pos());
}

// context menu action: copy URI
void ReceiveCoinsDialog::copyURI()
{
    QModelIndex sel = selectedRow();
    if (!sel.isValid()) {
        return;
    }

    const RecentRequestsTableModel * const submodel = model->getRecentRequestsTableModel();
    const QString uri = GUIUtil::formatShahcoinURI(submodel->entry(sel.row()).recipient);
    GUIUtil::setClipboard(uri);
}

// context menu action: copy address
void ReceiveCoinsDialog::copyAddress()
{
    const QModelIndex sel = selectedRow();
    if (!sel.isValid()) {
        return;
    }

    const RecentRequestsTableModel* const submodel = model->getRecentRequestsTableModel();
    const QString address = submodel->entry(sel.row()).recipient.address;
    GUIUtil::setClipboard(address);
}

// context menu action: copy label
void ReceiveCoinsDialog::copyLabel()
{
    copyColumnToClipboard(RecentRequestsTableModel::Label);
}

// context menu action: copy message
void ReceiveCoinsDialog::copyMessage()
{
    copyColumnToClipboard(RecentRequestsTableModel::Message);
}

// context menu action: copy amount
void ReceiveCoinsDialog::copyAmount()
{
    copyColumnToClipboard(RecentRequestsTableModel::Amount);
}

void ReceiveCoinsDialog::on_manageAddressBookClicked()
{
    if (!model) {
        QMessageBox::warning(this, tr("No Wallet"), tr("Please load a wallet first."));
        return;
    }

    // Open enhanced address book for managing receive addresses
    EnhancedAddressBook dlg(platformStyle, EnhancedAddressBook::ForManagement, this);
    dlg.setModel(model);
    
    if (dlg.exec()) {
        // Refresh the address table model to show any updated labels
        if (model->getAddressTableModel()) {
            model->getAddressTableModel()->emitDataChanged();
        }
        
        // Update the label field if an address was selected
        QString selectedAddress = dlg.getReturnValue();
        QString selectedLabel = dlg.getReturnLabel();
        
        if (!selectedAddress.isEmpty()) {
            // Find if this address is in our receive addresses and update the label
            AddressTableModel* addressModel = model->getAddressTableModel();
            for (int row = 0; row < addressModel->rowCount(); ++row) {
                QModelIndex addressIndex = addressModel->index(row, AddressTableModel::Address);
                QModelIndex labelIndex = addressModel->index(row, AddressTableModel::Label);
                QModelIndex typeIndex = addressModel->index(row, AddressTableModel::Type);
                
                QString address = addressModel->data(addressIndex).toString();
                QString type = addressModel->data(typeIndex).toString();
                
                if (address == selectedAddress && type == "Receive") {
                    // Update the label in the address table
                    addressModel->setData(labelIndex, selectedLabel, Qt::EditRole);
                    break;
                }
            }
        }
    }
}

void ReceiveCoinsDialog::updateQRCode()
{
    if (!model || !model->getAddressTableModel()) {
        m_qrCodeLabel->setText(tr("No wallet loaded"));
        return;
    }
    
    // Get the first receiving address
    QStringList addresses = model->getAddressTableModel()->getAddressList();
    QString address;
    for (const QString& addr : addresses) {
        // Find a receive address
        for (int row = 0; row < model->getAddressTableModel()->rowCount(); ++row) {
            QModelIndex addressIndex = model->getAddressTableModel()->index(row, AddressTableModel::Address);
            QModelIndex typeIndex = model->getAddressTableModel()->index(row, AddressTableModel::Type);
            if (model->getAddressTableModel()->data(addressIndex).toString() == addr &&
                model->getAddressTableModel()->data(typeIndex).toString() == "Receive") {
                address = addr;
                break;
            }
        }
        if (!address.isEmpty()) break;
    }
    
    if (address.isEmpty()) {
        m_qrCodeLabel->setText(tr("No receive address available"));
        return;
    }
    
    generateQRCode();
}

void ReceiveCoinsDialog::generateQRCode()
{
    QString qrString = generateQRString();
    if (qrString.isEmpty()) {
        m_qrCodeLabel->setText(tr("No address available"));
        return;
    }
    
    // Create a simple QR code representation (placeholder)
    // In a real implementation, you would use a QR code library like libqrencode
    QPixmap qrPixmap(250, 250);
    qrPixmap.fill(Qt::white);
    
    QPainter painter(&qrPixmap);
    painter.setPen(QPen(Qt::black, 2));
    
    // Draw a simple pattern to represent QR code
    int cellSize = 10;
    for (int i = 0; i < 25; ++i) {
        for (int j = 0; j < 25; ++j) {
            // Simple pattern based on string content
            int charIndex = (i * 25 + j) % qrString.length();
            if (qrString[charIndex].unicode() % 2 == 0) {
                painter.fillRect(i * cellSize, j * cellSize, cellSize, cellSize, Qt::black);
            }
        }
    }
    
    // Add text overlay
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 8));
    painter.drawText(qrPixmap.rect(), Qt::AlignCenter, tr("QR Code\n(Placeholder)"));
    
    m_qrPixmap = qrPixmap;
    m_qrCodeLabel->setPixmap(qrPixmap);
}

QString ReceiveCoinsDialog::generateQRString() const
{
    if (!model || !model->getAddressTableModel()) {
        return QString();
    }
    
    // Get the first receiving address
    QStringList addresses = model->getAddressTableModel()->getAddressList();
    QString address;
    for (const QString& addr : addresses) {
        // Find a receive address
        for (int row = 0; row < model->getAddressTableModel()->rowCount(); ++row) {
            QModelIndex addressIndex = model->getAddressTableModel()->index(row, AddressTableModel::Address);
            QModelIndex typeIndex = model->getAddressTableModel()->index(row, AddressTableModel::Type);
            if (model->getAddressTableModel()->data(addressIndex).toString() == addr &&
                model->getAddressTableModel()->data(typeIndex).toString() == "Receive") {
                address = addr;
                break;
            }
        }
        if (!address.isEmpty()) break;
    }
    
    if (address.isEmpty()) {
        return QString();
    }
    
    // Generate BIP21-style URI
    QString uri = QString("shah:%1").arg(address);
    
    // Add optional parameters
    QStringList params;
    
    if (!m_qrAmountEdit->text().isEmpty()) {
        bool ok;
        double amount = m_qrAmountEdit->text().toDouble(&ok);
        if (ok && amount > 0) {
            params.append(QString("amount=%1").arg(amount));
        }
    }
    
    if (!m_qrLabelEdit->text().isEmpty()) {
        params.append(QString("label=%1").arg(QUrl::toPercentEncoding(m_qrLabelEdit->text())));
    }
    
    if (!m_qrMessageEdit->text().isEmpty()) {
        params.append(QString("message=%1").arg(QUrl::toPercentEncoding(m_qrMessageEdit->text())));
    }
    
    if (!params.isEmpty()) {
        uri += "?" + params.join("&");
    }
    
    return uri;
}

void ReceiveCoinsDialog::onQRAmountChanged()
{
    generateQRCode();
}

void ReceiveCoinsDialog::onQRLabelChanged()
{
    generateQRCode();
}

void ReceiveCoinsDialog::onQRMessageChanged()
{
    generateQRCode();
}

void ReceiveCoinsDialog::onCopyAddressClicked()
{
    if (!model || !model->getAddressTableModel()) {
        QMessageBox::warning(this, tr("No Wallet"), tr("Please load a wallet first."));
        return;
    }
    
    // Get the first receiving address
    QStringList addresses = model->getAddressTableModel()->getAddressList();
    QString address;
    for (const QString& addr : addresses) {
        // Find a receive address
        for (int row = 0; row < model->getAddressTableModel()->rowCount(); ++row) {
            QModelIndex addressIndex = model->getAddressTableModel()->index(row, AddressTableModel::Address);
            QModelIndex typeIndex = model->getAddressTableModel()->index(row, AddressTableModel::Type);
            if (model->getAddressTableModel()->data(addressIndex).toString() == addr &&
                model->getAddressTableModel()->data(typeIndex).toString() == "Receive") {
                address = addr;
                break;
            }
        }
        if (!address.isEmpty()) break;
    }
    
    if (!address.isEmpty()) {
        GUIUtil::setClipboard(address);
        QMessageBox::information(this, tr("Address Copied"), tr("Wallet address has been copied to clipboard."));
    } else {
        QMessageBox::warning(this, tr("No Address"), tr("No wallet address available to copy."));
    }
}

void ReceiveCoinsDialog::onCopyURIClicked()
{
    QString uri = generateQRString();
    if (!uri.isEmpty()) {
        GUIUtil::setClipboard(uri);
        QMessageBox::information(this, tr("URI Copied"), tr("Payment URI has been copied to clipboard."));
    } else {
        QMessageBox::warning(this, tr("No URI"), tr("No payment URI available to copy."));
    }
}

void ReceiveCoinsDialog::onSaveQRClicked()
{
    if (m_qrPixmap.isNull()) {
        QMessageBox::warning(this, tr("No QR Code"), tr("No QR code available to save."));
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save QR Code"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/shahcoin_qr.png",
        tr("PNG Files (*.png)"));
    
    if (!fileName.isEmpty()) {
        if (m_qrPixmap.save(fileName, "PNG")) {
            QMessageBox::information(this, tr("QR Code Saved"), 
                tr("QR code has been saved to:\n%1").arg(fileName));
        } else {
            QMessageBox::critical(this, tr("Save Failed"), 
                tr("Failed to save QR code to:\n%1").arg(fileName));
        }
    }
}
