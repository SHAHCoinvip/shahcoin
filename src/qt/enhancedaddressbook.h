// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_ENHANCEDADDRESSBOOK_H
#define SHAHCOIN_QT_ENHANCEDADDRESSBOOK_H

#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QClipboard>

class PlatformStyle;
class WalletModel;

QT_BEGIN_NAMESPACE
class QTableWidgetItem;
QT_END_NAMESPACE

/**
 * Enhanced Address Book Entry Structure
 */
struct EnhancedAddressEntry
{
    QString label;
    QString address;
    QString notes;
    QString tags;
    QString type; // "Send", "Receive", "External"
    QDateTime createdDate;
    QDateTime lastUsedDate;
    int usageCount;
    
    EnhancedAddressEntry() : usageCount(0) {}
    
    EnhancedAddressEntry(const QString& _label, const QString& _address, const QString& _type = "Send")
        : label(_label), address(_address), type(_type), usageCount(0)
    {
        createdDate = QDateTime::currentDateTime();
        lastUsedDate = QDateTime::currentDateTime();
    }
    
    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["label"] = label;
        obj["address"] = address;
        obj["notes"] = notes;
        obj["tags"] = tags;
        obj["type"] = type;
        obj["createdDate"] = createdDate.toString(Qt::ISODate);
        obj["lastUsedDate"] = lastUsedDate.toString(Qt::ISODate);
        obj["usageCount"] = usageCount;
        return obj;
    }
    
    static EnhancedAddressEntry fromJson(const QJsonObject& obj)
    {
        EnhancedAddressEntry entry;
        entry.label = obj["label"].toString();
        entry.address = obj["address"].toString();
        entry.notes = obj["notes"].toString();
        entry.tags = obj["tags"].toString();
        entry.type = obj["type"].toString();
        entry.createdDate = QDateTime::fromString(obj["createdDate"].toString(), Qt::ISODate);
        entry.lastUsedDate = QDateTime::fromString(obj["lastUsedDate"].toString(), Qt::ISODate);
        entry.usageCount = obj["usageCount"].toInt();
        return entry;
    }
};

/**
 * Enhanced Address Book Dialog
 */
class EnhancedAddressBook : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        ForSelection, /**< Open address book to pick address */
        ForEditing,   /**< Open address book for editing */
        ForManagement /**< Open address book for managing receive addresses */
    };

    explicit EnhancedAddressBook(const PlatformStyle *platformStyle, Mode mode, QWidget *parent = nullptr);
    ~EnhancedAddressBook();

    void setModel(WalletModel *model);
    const QString &getReturnValue() const { return returnValue; }
    const QString &getReturnLabel() const { return returnLabel; }

public Q_SLOTS:
    void done(int retval) override;

private Q_SLOTS:
    void onNewAddressClicked();
    void onEditAddressClicked();
    void onDeleteAddressClicked();
    void onCopyAddressClicked();
    void onCopyLabelClicked();
    void onExportClicked();
    void onImportClicked();
    void onSearchTextChanged(const QString& text);
    void onFilterChanged(const QString& filter);
    void onTableSelectionChanged();
    void onTableDoubleClicked(int row, int column);
    void onContextMenuRequested(const QPoint& pos);
    void onSendToAddress();
    void onMarkAsFavorite();
    void onRemoveFromFavorites();

private:
    void setupUI();
    void setupTable();
    void setupActions();
    void loadAddresses();
    void saveAddresses();
    void addAddressEntry(const EnhancedAddressEntry& entry);
    void updateAddressEntry(int row, const EnhancedAddressEntry& entry);
    void removeAddressEntry(int row);
    void filterAddresses();
    void updateStats();
    void showEditDialog(const EnhancedAddressEntry& entry = EnhancedAddressEntry(), int row = -1);
    void exportToCSV(const QString& filename);
    void importFromCSV(const QString& filename);
    void exportToJSON(const QString& filename);
    void importFromJSON(const QString& filename);
    bool validateAddress(const QString& address);
    QStringList parseTags(const QString& tagsString);
    QString formatTags(const QStringList& tags);
    void incrementUsageCount(const QString& address);
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_searchGroup;
    QLineEdit* m_searchEdit;
    QComboBox* m_filterCombo;
    QTableWidget* m_addressTable;
    QGroupBox* m_statsGroup;
    QLabel* m_totalLabel;
    QLabel* m_favoritesLabel;
    QLabel* m_recentLabel;
    
    // Action buttons
    QPushButton* m_newButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_copyAddressButton;
    QPushButton* m_copyLabelButton;
    QPushButton* m_exportButton;
    QPushButton* m_importButton;
    QPushButton* m_closeButton;
    
    // Context menu
    QMenu* m_contextMenu;
    QAction* m_sendToAction;
    QAction* m_favoriteAction;
    QAction* m_unfavoriteAction;
    
    // Data
    QList<EnhancedAddressEntry> m_addresses;
    QString returnValue;
    QString returnLabel;
    Mode mode;
    const PlatformStyle* platformStyle;
    WalletModel* walletModel;
    
    // Settings
    QSettings m_settings;
    
    // Constants
    static const QString SETTINGS_GROUP;
    static const QString FAVORITES_KEY;
    static const QString RECENT_KEY;
    static const QString COLUMN_WIDTHS_KEY;
};

#endif // SHAHCOIN_QT_ENHANCEDADDRESSBOOK_H
