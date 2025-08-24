// Copyright (c) 2011-2022 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/shahcoin-config.h>
#endif

#include <qt/enhancedaddressbook.h>
#include <qt/platformstyle.h>
#include <qt/walletmodel.h>
#include <qt/guiutil.h>
#include <qt/addresstablemodel.h>

#include <QApplication>
#include <QHeaderView>
#include <QTextStream>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QInputDialog>
#include <QStandardPaths>

// Static constants
const QString EnhancedAddressBook::SETTINGS_GROUP = "EnhancedAddressBook";
const QString EnhancedAddressBook::FAVORITES_KEY = "Favorites";
const QString EnhancedAddressBook::RECENT_KEY = "Recent";
const QString EnhancedAddressBook::COLUMN_WIDTHS_KEY = "ColumnWidths";

EnhancedAddressBook::EnhancedAddressBook(const PlatformStyle *_platformStyle, Mode _mode, QWidget *parent)
    : QDialog(parent, GUIUtil::dialog_flags)
    , returnValue("")
    , returnLabel("")
    , mode(_mode)
    , platformStyle(_platformStyle)
    , walletModel(nullptr)
    , m_settings()
{
    setupUI();
    setupTable();
    setupActions();
    loadAddresses();
    
    if (mode == ForSelection) {
        setWindowTitle(tr("Enhanced Address Book - Select Address"));
        m_closeButton->setText(tr("&Select"));
        connect(m_addressTable, &QTableWidget::doubleClicked, this, &QDialog::accept);
    } else if (mode == ForManagement) {
        setWindowTitle(tr("Enhanced Address Book - Manage Receive Addresses"));
        m_closeButton->setText(tr("&Close"));
        // In management mode, focus on receive addresses
        m_filterCombo->setCurrentText(tr("Receive Addresses"));
        onFilterChanged(tr("Receive Addresses"));
    } else {
        setWindowTitle(tr("Enhanced Address Book"));
        m_closeButton->setText(tr("&Close"));
    }
    
    GUIUtil::handleCloseWindowShortcut(this);
}

EnhancedAddressBook::~EnhancedAddressBook()
{
    saveAddresses();
}

void EnhancedAddressBook::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Search and filter group
    m_searchGroup = new QGroupBox(tr("Search & Filter"));
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchGroup);
    
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText(tr("Search by label, address, notes, or tags..."));
    searchLayout->addWidget(m_searchEdit);
    
    m_filterCombo = new QComboBox();
    m_filterCombo->addItem(tr("All Addresses"), "all");
    m_filterCombo->addItem(tr("Favorites"), "favorites");
    m_filterCombo->addItem(tr("Recently Used"), "recent");
    m_filterCombo->addItem(tr("Send Addresses"), "send");
    m_filterCombo->addItem(tr("Receive Addresses"), "receive");
    m_filterCombo->addItem(tr("External Addresses"), "external");
    searchLayout->addWidget(m_filterCombo);
    
    m_mainLayout->addWidget(m_searchGroup);
    
    // Address table
    m_addressTable = new QTableWidget();
    m_mainLayout->addWidget(m_addressTable);
    
    // Stats group
    m_statsGroup = new QGroupBox(tr("Statistics"));
    QHBoxLayout* statsLayout = new QHBoxLayout(m_statsGroup);
    
    m_totalLabel = new QLabel(tr("Total: 0"));
    m_favoritesLabel = new QLabel(tr("Favorites: 0"));
    m_recentLabel = new QLabel(tr("Recent: 0"));
    
    statsLayout->addWidget(m_totalLabel);
    statsLayout->addWidget(m_favoritesLabel);
    statsLayout->addWidget(m_recentLabel);
    statsLayout->addStretch();
    
    m_mainLayout->addWidget(m_statsGroup);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_newButton = new QPushButton(tr("&New"));
    m_editButton = new QPushButton(tr("&Edit"));
    m_deleteButton = new QPushButton(tr("&Delete"));
    m_copyAddressButton = new QPushButton(tr("Copy &Address"));
    m_copyLabelButton = new QPushButton(tr("Copy &Label"));
    m_exportButton = new QPushButton(tr("&Export"));
    m_importButton = new QPushButton(tr("&Import"));
    m_closeButton = new QPushButton(tr("&Close"));
    
    buttonLayout->addWidget(m_newButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_copyAddressButton);
    buttonLayout->addWidget(m_copyLabelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_importButton);
    buttonLayout->addWidget(m_closeButton);
    
    m_mainLayout->addLayout(buttonLayout);
    
    // Set icons
    if (platformStyle->getImagesOnButtons()) {
        m_newButton->setIcon(platformStyle->SingleColorIcon(":/icons/add"));
        m_editButton->setIcon(platformStyle->SingleColorIcon(":/icons/edit"));
        m_deleteButton->setIcon(platformStyle->SingleColorIcon(":/icons/remove"));
        m_copyAddressButton->setIcon(platformStyle->SingleColorIcon(":/icons/editcopy"));
        m_copyLabelButton->setIcon(platformStyle->SingleColorIcon(":/icons/editcopy"));
        m_exportButton->setIcon(platformStyle->SingleColorIcon(":/icons/export"));
        m_importButton->setIcon(platformStyle->SingleColorIcon(":/icons/import"));
    }
    
    // Set window size
    resize(800, 600);
}

void EnhancedAddressBook::setupTable()
{
    QStringList headers;
    headers << tr("Label") << tr("Address") << tr("Notes") << tr("Tags") << tr("Type") << tr("Created") << tr("Last Used") << tr("Usage");
    
    m_addressTable->setColumnCount(headers.size());
    m_addressTable->setHorizontalHeaderLabels(headers);
    m_addressTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_addressTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_addressTable->setAlternatingRowColors(true);
    m_addressTable->setContextMenuPolicy(Qt::CustomContextMenu);
    m_addressTable->setSortingEnabled(true);
    
    // Set column widths
    m_addressTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); // Label
    m_addressTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Address
    m_addressTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch); // Notes
    m_addressTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Tags
    m_addressTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Type
    m_addressTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents); // Created
    m_addressTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents); // Last Used
    m_addressTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents); // Usage
    
    // Load saved column widths
    m_settings.beginGroup(SETTINGS_GROUP);
    QList<int> savedWidths = m_settings.value(COLUMN_WIDTHS_KEY).value<QList<int>>();
    if (!savedWidths.isEmpty() && savedWidths.size() == headers.size()) {
        for (int i = 0; i < savedWidths.size(); ++i) {
            m_addressTable->setColumnWidth(i, savedWidths[i]);
        }
    }
    m_settings.endGroup();
}

void EnhancedAddressBook::setupActions()
{
    // Connect signals
    connect(m_newButton, &QPushButton::clicked, this, &EnhancedAddressBook::onNewAddressClicked);
    connect(m_editButton, &QPushButton::clicked, this, &EnhancedAddressBook::onEditAddressClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &EnhancedAddressBook::onDeleteAddressClicked);
    connect(m_copyAddressButton, &QPushButton::clicked, this, &EnhancedAddressBook::onCopyAddressClicked);
    connect(m_copyLabelButton, &QPushButton::clicked, this, &EnhancedAddressBook::onCopyLabelClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &EnhancedAddressBook::onExportClicked);
    connect(m_importButton, &QPushButton::clicked, this, &EnhancedAddressBook::onImportClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    connect(m_searchEdit, &QLineEdit::textChanged, this, &EnhancedAddressBook::onSearchTextChanged);
    connect(m_filterCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged), this, &EnhancedAddressBook::onFilterChanged);
    connect(m_addressTable, &QTableWidget::itemSelectionChanged, this, &EnhancedAddressBook::onTableSelectionChanged);
    connect(m_addressTable, &QTableWidget::customContextMenuRequested, this, &EnhancedAddressBook::onContextMenuRequested);
    
    // Setup context menu
    m_contextMenu = new QMenu(this);
    m_sendToAction = m_contextMenu->addAction(tr("Send to this address"));
    m_favoriteAction = m_contextMenu->addAction(tr("Mark as favorite"));
    m_unfavoriteAction = m_contextMenu->addAction(tr("Remove from favorites"));
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(tr("Copy address"), this, &EnhancedAddressBook::onCopyAddressClicked);
    m_contextMenu->addAction(tr("Copy label"), this, &EnhancedAddressBook::onCopyLabelClicked);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(tr("Edit"), this, &EnhancedAddressBook::onEditAddressClicked);
    m_contextMenu->addAction(tr("Delete"), this, &EnhancedAddressBook::onDeleteAddressClicked);
    
    connect(m_sendToAction, &QAction::triggered, this, &EnhancedAddressBook::onSendToAddress);
    connect(m_favoriteAction, &QAction::triggered, this, &EnhancedAddressBook::onMarkAsFavorite);
    connect(m_unfavoriteAction, &QAction::triggered, this, &EnhancedAddressBook::onRemoveFromFavorites);
    
    // Initial button states
    onTableSelectionChanged();
}

void EnhancedAddressBook::setModel(WalletModel *model)
{
    walletModel = model;
    loadAddresses();
}

void EnhancedAddressBook::loadAddresses()
{
    m_addresses.clear();
    m_addressTable->setRowCount(0);
    
    // Load from wallet model if available
    if (walletModel && walletModel->getAddressTableModel()) {
        AddressTableModel* addressModel = walletModel->getAddressTableModel();
        for (int row = 0; row < addressModel->rowCount(); ++row) {
            QModelIndex labelIndex = addressModel->index(row, AddressTableModel::Label);
            QModelIndex addressIndex = addressModel->index(row, AddressTableModel::Address);
            
            QString label = addressModel->data(labelIndex).toString();
            QString address = addressModel->data(addressIndex).toString();
            QString type = addressModel->data(labelIndex, AddressTableModel::TypeRole).toString();
            
            EnhancedAddressEntry entry(label, address, type);
            m_addresses.append(entry);
        }
    }
    
    // Load enhanced data from settings
    m_settings.beginGroup(SETTINGS_GROUP);
    QJsonArray enhancedData = QJsonDocument::fromJson(m_settings.value("EnhancedData").toByteArray()).array();
    m_settings.endGroup();
    
    for (const QJsonValue& value : enhancedData) {
        QJsonObject obj = value.toObject();
        QString address = obj["address"].toString();
        
        // Find existing entry and update with enhanced data
        for (int i = 0; i < m_addresses.size(); ++i) {
            if (m_addresses[i].address == address) {
                m_addresses[i].notes = obj["notes"].toString();
                m_addresses[i].tags = obj["tags"].toString();
                m_addresses[i].createdDate = QDateTime::fromString(obj["createdDate"].toString(), Qt::ISODate);
                m_addresses[i].lastUsedDate = QDateTime::fromString(obj["lastUsedDate"].toString(), Qt::ISODate);
                m_addresses[i].usageCount = obj["usageCount"].toInt();
                break;
            }
        }
    }
    
    // Populate table
    for (const EnhancedAddressEntry& entry : m_addresses) {
        addAddressEntry(entry);
    }
    
    updateStats();
}

void EnhancedAddressBook::saveAddresses()
{
    // Save column widths
    m_settings.beginGroup(SETTINGS_GROUP);
    QList<int> columnWidths;
    for (int i = 0; i < m_addressTable->columnCount(); ++i) {
        columnWidths.append(m_addressTable->columnWidth(i));
    }
    m_settings.setValue(COLUMN_WIDTHS_KEY, QVariant::fromValue(columnWidths));
    
    // Save enhanced data
    QJsonArray enhancedData;
    for (const EnhancedAddressEntry& entry : m_addresses) {
        enhancedData.append(entry.toJson());
    }
    m_settings.setValue("EnhancedData", QJsonDocument(enhancedData).toJson());
    m_settings.endGroup();
}

void EnhancedAddressBook::addAddressEntry(const EnhancedAddressEntry& entry)
{
    int row = m_addressTable->rowCount();
    m_addressTable->insertRow(row);
    
    m_addressTable->setItem(row, 0, new QTableWidgetItem(entry.label));
    m_addressTable->setItem(row, 1, new QTableWidgetItem(entry.address));
    m_addressTable->setItem(row, 2, new QTableWidgetItem(entry.notes));
    m_addressTable->setItem(row, 3, new QTableWidgetItem(entry.tags));
    m_addressTable->setItem(row, 4, new QTableWidgetItem(entry.type));
    m_addressTable->setItem(row, 5, new QTableWidgetItem(entry.createdDate.toString("yyyy-MM-dd hh:mm")));
    m_addressTable->setItem(row, 6, new QTableWidgetItem(entry.lastUsedDate.toString("yyyy-MM-dd hh:mm")));
    m_addressTable->setItem(row, 7, new QTableWidgetItem(QString::number(entry.usageCount)));
    
    // Store the address in the first column for easy access
    m_addressTable->item(row, 0)->setData(Qt::UserRole, entry.address);
}

void EnhancedAddressBook::updateAddressEntry(int row, const EnhancedAddressEntry& entry)
{
    if (row >= 0 && row < m_addressTable->rowCount()) {
        m_addressTable->setItem(row, 0, new QTableWidgetItem(entry.label));
        m_addressTable->setItem(row, 1, new QTableWidgetItem(entry.address));
        m_addressTable->setItem(row, 2, new QTableWidgetItem(entry.notes));
        m_addressTable->setItem(row, 3, new QTableWidgetItem(entry.tags));
        m_addressTable->setItem(row, 4, new QTableWidgetItem(entry.type));
        m_addressTable->setItem(row, 5, new QTableWidgetItem(entry.createdDate.toString("yyyy-MM-dd hh:mm")));
        m_addressTable->setItem(row, 6, new QTableWidgetItem(entry.lastUsedDate.toString("yyyy-MM-dd hh:mm")));
        m_addressTable->setItem(row, 7, new QTableWidgetItem(QString::number(entry.usageCount)));
        
        m_addressTable->item(row, 0)->setData(Qt::UserRole, entry.address);
    }
}

void EnhancedAddressBook::removeAddressEntry(int row)
{
    if (row >= 0 && row < m_addressTable->rowCount()) {
        QString address = m_addressTable->item(row, 0)->data(Qt::UserRole).toString();
        
        // Remove from addresses list
        for (int i = 0; i < m_addresses.size(); ++i) {
            if (m_addresses[i].address == address) {
                m_addresses.removeAt(i);
                break;
            }
        }
        
        // Remove from table
        m_addressTable->removeRow(row);
        updateStats();
    }
}

void EnhancedAddressBook::filterAddresses()
{
    QString searchText = m_searchEdit->text().toLower();
    QString filterType = m_filterCombo->currentData().toString();
    
    for (int row = 0; row < m_addressTable->rowCount(); ++row) {
        bool showRow = true;
        
        // Apply search filter
        if (!searchText.isEmpty()) {
            QString label = m_addressTable->item(row, 0)->text().toLower();
            QString address = m_addressTable->item(row, 1)->text().toLower();
            QString notes = m_addressTable->item(row, 2)->text().toLower();
            QString tags = m_addressTable->item(row, 3)->text().toLower();
            
            if (!label.contains(searchText) && !address.contains(searchText) && 
                !notes.contains(searchText) && !tags.contains(searchText)) {
                showRow = false;
            }
        }
        
        // Apply type filter
        if (showRow && filterType != "all") {
            QString type = m_addressTable->item(row, 4)->text().toLower();
            if (filterType == "favorites") {
                // Check if address is in favorites
                m_settings.beginGroup(SETTINGS_GROUP);
                QStringList favorites = m_settings.value(FAVORITES_KEY).toStringList();
                m_settings.endGroup();
                showRow = favorites.contains(m_addressTable->item(row, 1)->text());
            } else if (filterType == "recent") {
                // Check if address was used recently (last 30 days)
                QDateTime lastUsed = QDateTime::fromString(m_addressTable->item(row, 6)->text(), "yyyy-MM-dd hh:mm");
                showRow = lastUsed.daysTo(QDateTime::currentDateTime()) <= 30;
            } else if (filterType == "send" && type != "send") {
                showRow = false;
            } else if (filterType == "receive" && type != "receive") {
                showRow = false;
            } else if (filterType == "external" && type != "external") {
                showRow = false;
            }
        }
        
        m_addressTable->setRowHidden(row, !showRow);
    }
}

void EnhancedAddressBook::updateStats()
{
    int total = m_addresses.size();
    int favorites = 0;
    int recent = 0;
    
    m_settings.beginGroup(SETTINGS_GROUP);
    QStringList favoritesList = m_settings.value(FAVORITES_KEY).toStringList();
    m_settings.endGroup();
    
    favorites = favoritesList.size();
    
    QDateTime thirtyDaysAgo = QDateTime::currentDateTime().addDays(-30);
    for (const EnhancedAddressEntry& entry : m_addresses) {
        if (entry.lastUsedDate >= thirtyDaysAgo) {
            recent++;
        }
    }
    
    m_totalLabel->setText(tr("Total: %1").arg(total));
    m_favoritesLabel->setText(tr("Favorites: %1").arg(favorites));
    m_recentLabel->setText(tr("Recent: %1").arg(recent));
}

void EnhancedAddressBook::showEditDialog(const EnhancedAddressEntry& entry, int row)
{
    QDialog dialog(this);
    dialog.setWindowTitle(row >= 0 ? tr("Edit Address") : tr("New Address"));
    dialog.setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    // Form layout
    QFormLayout* formLayout = new QFormLayout();
    
    QLineEdit* labelEdit = new QLineEdit(entry.label);
    QLineEdit* addressEdit = new QLineEdit(entry.address);
    QTextEdit* notesEdit = new QTextEdit(entry.notes);
    QLineEdit* tagsEdit = new QLineEdit(entry.tags);
    QComboBox* typeCombo = new QComboBox();
    typeCombo->addItems(QStringList() << "Send" << "Receive" << "External");
    typeCombo->setCurrentText(entry.type);
    
    formLayout->addRow(tr("Label:"), labelEdit);
    formLayout->addRow(tr("Address:"), addressEdit);
    formLayout->addRow(tr("Notes:"), notesEdit);
    formLayout->addRow(tr("Tags:"), tagsEdit);
    formLayout->addRow(tr("Type:"), typeCombo);
    
    layout->addLayout(formLayout);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton(tr("OK"));
    QPushButton* cancelButton = new QPushButton(tr("Cancel"));
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addLayout(buttonLayout);
    
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        EnhancedAddressEntry newEntry;
        newEntry.label = labelEdit->text();
        newEntry.address = addressEdit->text();
        newEntry.notes = notesEdit->toPlainText();
        newEntry.tags = tagsEdit->text();
        newEntry.type = typeCombo->currentText();
        
        if (row >= 0) {
            // Update existing entry
            newEntry.createdDate = entry.createdDate;
            newEntry.lastUsedDate = entry.lastUsedDate;
            newEntry.usageCount = entry.usageCount;
            
            m_addresses[row] = newEntry;
            updateAddressEntry(row, newEntry);
        } else {
            // Add new entry
            newEntry.createdDate = QDateTime::currentDateTime();
            newEntry.lastUsedDate = QDateTime::currentDateTime();
            newEntry.usageCount = 0;
            
            m_addresses.append(newEntry);
            addAddressEntry(newEntry);
        }
        
        updateStats();
        saveAddresses();
    }
}

void EnhancedAddressBook::onNewAddressClicked()
{
    showEditDialog();
}

void EnhancedAddressBook::onEditAddressClicked()
{
    int currentRow = m_addressTable->currentRow();
    if (currentRow >= 0) {
        QString address = m_addressTable->item(currentRow, 0)->data(Qt::UserRole).toString();
        
        // Find the entry
        for (int i = 0; i < m_addresses.size(); ++i) {
            if (m_addresses[i].address == address) {
                showEditDialog(m_addresses[i], i);
                break;
            }
        }
    }
}

void EnhancedAddressBook::onDeleteAddressClicked()
{
    int currentRow = m_addressTable->currentRow();
    if (currentRow >= 0) {
        QString address = m_addressTable->item(currentRow, 1)->text();
        QString label = m_addressTable->item(currentRow, 0)->text();
        
        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Delete Address"),
            tr("Are you sure you want to delete the address '%1' (%2)?").arg(label, address),
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::Yes) {
            removeAddressEntry(currentRow);
            saveAddresses();
        }
    }
}

void EnhancedAddressBook::onCopyAddressClicked()
{
    int currentRow = m_addressTable->currentRow();
    if (currentRow >= 0) {
        QString address = m_addressTable->item(currentRow, 1)->text();
        QApplication::clipboard()->setText(address);
        incrementUsageCount(address);
    }
}

void EnhancedAddressBook::onCopyLabelClicked()
{
    int currentRow = m_addressTable->currentRow();
    if (currentRow >= 0) {
        QString label = m_addressTable->item(currentRow, 0)->text();
        QApplication::clipboard()->setText(label);
    }
}

void EnhancedAddressBook::onExportClicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Address Book"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/addressbook.json",
        tr("JSON files (*.json);;CSV files (*.csv)"));
        
    if (!filename.isEmpty()) {
        if (filename.endsWith(".json")) {
            exportToJSON(filename);
        } else if (filename.endsWith(".csv")) {
            exportToCSV(filename);
        }
    }
}

void EnhancedAddressBook::onImportClicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Address Book"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("JSON files (*.json);;CSV files (*.csv)"));
        
    if (!filename.isEmpty()) {
        if (filename.endsWith(".json")) {
            importFromJSON(filename);
        } else if (filename.endsWith(".csv")) {
            importFromCSV(filename);
        }
    }
}

void EnhancedAddressBook::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text);
    filterAddresses();
}

void EnhancedAddressBook::onFilterChanged(const QString& filter)
{
    Q_UNUSED(filter);
    filterAddresses();
}

void EnhancedAddressBook::onTableSelectionChanged()
{
    bool hasSelection = m_addressTable->currentRow() >= 0;
    
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_copyAddressButton->setEnabled(hasSelection);
    m_copyLabelButton->setEnabled(hasSelection);
}

void EnhancedAddressBook::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (mode == ForSelection && row >= 0) {
        returnValue = m_addressTable->item(row, 1)->text();
        returnLabel = m_addressTable->item(row, 0)->text();
        incrementUsageCount(returnValue);
        accept();
    }
}

void EnhancedAddressBook::onContextMenuRequested(const QPoint& pos)
{
    int row = m_addressTable->rowAt(pos.y());
    if (row >= 0) {
        QString address = m_addressTable->item(row, 1)->text();
        
        m_settings.beginGroup(SETTINGS_GROUP);
        QStringList favorites = m_settings.value(FAVORITES_KEY).toStringList();
        m_settings.endGroup();
        
        bool isFavorite = favorites.contains(address);
        m_favoriteAction->setVisible(!isFavorite);
        m_unfavoriteAction->setVisible(isFavorite);
        
        m_contextMenu->exec(m_addressTable->mapToGlobal(pos));
    }
}

void EnhancedAddressBook::onSendToAddress()
{
    int currentRow = m_addressTable->currentRow();
    if (currentRow >= 0) {
        returnValue = m_addressTable->item(currentRow, 1)->text();
        returnLabel = m_addressTable->item(currentRow, 0)->text();
        incrementUsageCount(returnValue);
        accept();
    }
}

void EnhancedAddressBook::onMarkAsFavorite()
{
    int currentRow = m_addressTable->currentRow();
    if (currentRow >= 0) {
        QString address = m_addressTable->item(currentRow, 1)->text();
        
        m_settings.beginGroup(SETTINGS_GROUP);
        QStringList favorites = m_settings.value(FAVORITES_KEY).toStringList();
        if (!favorites.contains(address)) {
            favorites.append(address);
            m_settings.setValue(FAVORITES_KEY, favorites);
        }
        m_settings.endGroup();
        
        updateStats();
    }
}

void EnhancedAddressBook::onRemoveFromFavorites()
{
    int currentRow = m_addressTable->currentRow();
    if (currentRow >= 0) {
        QString address = m_addressTable->item(currentRow, 1)->text();
        
        m_settings.beginGroup(SETTINGS_GROUP);
        QStringList favorites = m_settings.value(FAVORITES_KEY).toStringList();
        favorites.removeAll(address);
        m_settings.setValue(FAVORITES_KEY, favorites);
        m_settings.endGroup();
        
        updateStats();
    }
}

void EnhancedAddressBook::done(int retval)
{
    if (retval == QDialog::Accepted && mode == ForSelection) {
        int currentRow = m_addressTable->currentRow();
        if (currentRow >= 0) {
            returnValue = m_addressTable->item(currentRow, 1)->text();
            returnLabel = m_addressTable->item(currentRow, 0)->text();
            incrementUsageCount(returnValue);
        }
    }
    
    QDialog::done(retval);
}

void EnhancedAddressBook::exportToJSON(const QString& filename)
{
    QJsonArray array;
    for (const EnhancedAddressEntry& entry : m_addresses) {
        array.append(entry.toJson());
    }
    
    QJsonDocument doc(array);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        QMessageBox::information(this, tr("Export Successful"), tr("Address book exported successfully."));
    } else {
        QMessageBox::critical(this, tr("Export Failed"), tr("Failed to export address book."));
    }
}

void EnhancedAddressBook::importFromJSON(const QString& filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray array = doc.array();
        
        for (const QJsonValue& value : array) {
            QJsonObject obj = value.toObject();
            EnhancedAddressEntry entry = EnhancedAddressEntry::fromJson(obj);
            m_addresses.append(entry);
            addAddressEntry(entry);
        }
        
        updateStats();
        saveAddresses();
        QMessageBox::information(this, tr("Import Successful"), tr("Address book imported successfully."));
    } else {
        QMessageBox::critical(this, tr("Import Failed"), tr("Failed to import address book."));
    }
}

void EnhancedAddressBook::exportToCSV(const QString& filename)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        
        // Write header
        stream << "Label,Address,Notes,Tags,Type,Created,Last Used,Usage Count\n";
        
        // Write data
        for (const EnhancedAddressEntry& entry : m_addresses) {
            stream << QString("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",\"%6\",\"%7\",%8\n")
                .arg(entry.label)
                .arg(entry.address)
                .arg(entry.notes)
                .arg(entry.tags)
                .arg(entry.type)
                .arg(entry.createdDate.toString("yyyy-MM-dd hh:mm"))
                .arg(entry.lastUsedDate.toString("yyyy-MM-dd hh:mm"))
                .arg(entry.usageCount);
        }
        
        QMessageBox::information(this, tr("Export Successful"), tr("Address book exported successfully."));
    } else {
        QMessageBox::critical(this, tr("Export Failed"), tr("Failed to export address book."));
    }
}

void EnhancedAddressBook::importFromCSV(const QString& filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        
        // Skip header
        QString header = stream.readLine();
        
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            QStringList fields = line.split(",");
            
            if (fields.size() >= 8) {
                EnhancedAddressEntry entry;
                entry.label = fields[0].remove("\"");
                entry.address = fields[1].remove("\"");
                entry.notes = fields[2].remove("\"");
                entry.tags = fields[3].remove("\"");
                entry.type = fields[4].remove("\"");
                entry.createdDate = QDateTime::fromString(fields[5].remove("\""), "yyyy-MM-dd hh:mm");
                entry.lastUsedDate = QDateTime::fromString(fields[6].remove("\""), "yyyy-MM-dd hh:mm");
                entry.usageCount = fields[7].toInt();
                
                m_addresses.append(entry);
                addAddressEntry(entry);
            }
        }
        
        updateStats();
        saveAddresses();
        QMessageBox::information(this, tr("Import Successful"), tr("Address book imported successfully."));
    } else {
        QMessageBox::critical(this, tr("Import Failed"), tr("Failed to import address book."));
    }
}

bool EnhancedAddressBook::validateAddress(const QString& address)
{
    if (walletModel) {
        return walletModel->validateAddress(address);
    }
    return true; // Assume valid if no wallet model
}

QStringList EnhancedAddressBook::parseTags(const QString& tagsString)
{
    return tagsString.split(",", Qt::SkipEmptyParts);
}

QString EnhancedAddressBook::formatTags(const QStringList& tags)
{
    return tags.join(", ");
}

void EnhancedAddressBook::incrementUsageCount(const QString& address)
{
    for (int i = 0; i < m_addresses.size(); ++i) {
        if (m_addresses[i].address == address) {
            m_addresses[i].usageCount++;
            m_addresses[i].lastUsedDate = QDateTime::currentDateTime();
            
            // Update in table
            for (int row = 0; row < m_addressTable->rowCount(); ++row) {
                if (m_addressTable->item(row, 0)->data(Qt::UserRole).toString() == address) {
                    m_addressTable->setItem(row, 6, new QTableWidgetItem(m_addresses[i].lastUsedDate.toString("yyyy-MM-dd hh:mm")));
                    m_addressTable->setItem(row, 7, new QTableWidgetItem(QString::number(m_addresses[i].usageCount)));
                    break;
                }
            }
            
            saveAddresses();
            break;
        }
    }
}
