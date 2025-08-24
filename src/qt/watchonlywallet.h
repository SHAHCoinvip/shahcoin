#ifndef SHAHCOIN_QT_WATCHONLYWALLET_H
#define SHAHCOIN_QT_WATCHONLYWALLET_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QTimer>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QTableWidget;
class QTableWidgetItem;
class QGroupBox;
class QMessageBox;
class QFileDialog;
class WalletModel;

/**
 * @brief Watch-Only Wallet Manager for Shahcoin Qt Wallet
 * 
 * Provides functionality to monitor public addresses without private keys:
 * - Import public addresses
 * - Display balances and transactions
 * - Show NFTs owned by watched addresses
 * - Import from file (.txt list)
 * - Export watched addresses
 * - Disable sending/staking functionality
 */
class WatchOnlyWallet : public QWidget
{
    Q_OBJECT

public:
    explicit WatchOnlyWallet(WalletModel* walletModel, QWidget* parent = nullptr);
    ~WatchOnlyWallet();

    // Address management
    bool addWatchedAddress(const QString& address);
    bool removeWatchedAddress(const QString& address);
    bool isAddressWatched(const QString& address) const;
    QStringList getWatchedAddresses() const;
    
    // Balance and transaction tracking
    double getTotalBalance() const;
    int getTransactionCount() const;
    QStringList getRecentTransactions() const;
    
    // Import/Export
    bool importFromFile(const QString& filePath);
    bool exportToFile(const QString& filePath) const;
    bool validateAddress(const QString& address) const;

public Q_SLOTS:
    void onAddAddressClicked();
    void onRemoveAddressClicked();
    void onImportFromFileClicked();
    void onExportToFileClicked();
    void onRefreshClicked();
    void onAddressTextChanged();
    void onTableSelectionChanged();

Q_SIGNALS:
    void addressAdded(const QString& address);
    void addressRemoved(const QString& address);
    void balanceUpdated(double balance);
    void transactionCountChanged(int count);

private:
    void setupUI();
    void setupAddressGroup();
    void setupBalanceGroup();
    void setupTransactionGroup();
    void setupButtonGroup();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    void refreshData();
    void updateBalanceDisplay();
    void updateTransactionTable();
    void populateAddressTable();
    void showWarning(const QString& title, const QString& message);
    void showSuccess(const QString& title, const QString& message);
    void showError(const QString& title, const QString& message);
    QStringList generateMockTransactions() const;
    double generateMockBalance() const;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Address Management Group
    QGroupBox* m_addressGroup;
    QLabel* m_addressLabel;
    QLineEdit* m_addressEdit;
    QPushButton* m_addAddressButton;
    QPushButton* m_removeAddressButton;
    QTableWidget* m_addressTable;
    
    // Balance Group
    QGroupBox* m_balanceGroup;
    QLabel* m_totalBalanceLabel;
    QLabel* m_balanceValueLabel;
    QLabel* m_transactionCountLabel;
    QLabel* m_countValueLabel;
    
    // Transaction Group
    QGroupBox* m_transactionGroup;
    QTableWidget* m_transactionTable;
    
    // Button Group
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_importButton;
    QPushButton* m_exportButton;
    QPushButton* m_refreshButton;
    
    // Data
    WalletModel* m_walletModel;
    QStringList m_watchedAddresses;
    double m_totalBalance;
    int m_transactionCount;
    QStringList m_recentTransactions;
    
    // Settings
    QSettings* m_settings;
    
    // Update timer
    QTimer* m_updateTimer;
    
    // Constants
    static constexpr const char* SETTINGS_GROUP = "WatchOnlyWallet";
    static constexpr const char* ADDRESSES_KEY = "WatchedAddresses";
    static constexpr int UPDATE_INTERVAL_MS = 30000; // 30 seconds
    static constexpr int MAX_WATCHED_ADDRESSES = 100;
};

#endif // SHAHCOIN_QT_WATCHONLYWALLET_H
