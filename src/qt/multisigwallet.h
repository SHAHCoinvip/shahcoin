#ifndef SHAHCOIN_QT_MULTISIGWALLET_H
#define SHAHCOIN_QT_MULTISIGWALLET_H

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
class QComboBox;
class QSpinBox;
class QCheckBox;
class WalletModel;

/**
 * @brief Multisignature Wallet Manager for Shahcoin Qt Wallet
 * 
 * Provides functionality for creating and managing multisig wallets:
 * - Create N-of-M multisig addresses
 * - Add/remove cosigners (public keys)
 * - Generate unsigned transactions
 * - Sign/combine transactions
 * - Track transaction status
 * - Import/export multisig configurations
 */
class MultisigWallet : public QWidget
{
    Q_OBJECT

public:
    explicit MultisigWallet(WalletModel* walletModel, QWidget* parent = nullptr);
    ~MultisigWallet();

    // Multisig management
    bool createMultisigAddress(int requiredSignatures, int totalSigners);
    bool addCosigner(const QString& publicKey);
    bool removeCosigner(const QString& publicKey);
    QStringList getCosigners() const;
    QString getMultisigAddress() const;
    
    // Transaction management
    bool createUnsignedTransaction(const QString& recipient, double amount);
    bool signTransaction(const QString& transactionHex);
    bool combineSignatures(const QStringList& signatures);
    QString getTransactionStatus() const;
    
    // Configuration
    bool saveConfiguration(const QString& filePath) const;
    bool loadConfiguration(const QString& filePath);
    QString getConfiguration() const;

public Q_SLOTS:
    void onCreateMultisigClicked();
    void onAddCosignerClicked();
    void onRemoveCosignerClicked();
    void onCreateTransactionClicked();
    void onSignTransactionClicked();
    void onCombineSignaturesClicked();
    void onSaveConfigClicked();
    void onLoadConfigClicked();
    void onRefreshClicked();

Q_SIGNALS:
    void multisigCreated(const QString& address, int required, int total);
    void cosignerAdded(const QString& publicKey);
    void cosignerRemoved(const QString& publicKey);
    void transactionCreated(const QString& txHex);
    void transactionSigned(const QString& signature);
    void signaturesCombined(const QString& finalTx);

private:
    void setupUI();
    void setupCreationGroup();
    void setupCosignersGroup();
    void setupTransactionGroup();
    void setupButtonGroup();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    void refreshData();
    void updateCosignerTable();
    void updateTransactionTable();
    void showWarning(const QString& title, const QString& message);
    void showSuccess(const QString& title, const QString& message);
    void showError(const QString& title, const QString& message);
    bool validatePublicKey(const QString& publicKey) const;
    QString generateMockMultisigAddress() const;
    QStringList generateMockTransactions() const;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Creation Group
    QGroupBox* m_creationGroup;
    QLabel* m_requiredLabel;
    QSpinBox* m_requiredSpinBox;
    QLabel* m_totalLabel;
    QSpinBox* m_totalSpinBox;
    QPushButton* m_createButton;
    QLabel* m_addressLabel;
    QLineEdit* m_addressEdit;
    
    // Cosigners Group
    QGroupBox* m_cosignersGroup;
    QLabel* m_cosignerLabel;
    QLineEdit* m_cosignerEdit;
    QPushButton* m_addCosignerButton;
    QPushButton* m_removeCosignerButton;
    QTableWidget* m_cosignerTable;
    
    // Transaction Group
    QGroupBox* m_transactionGroup;
    QLabel* m_recipientLabel;
    QLineEdit* m_recipientEdit;
    QLabel* m_amountLabel;
    QLineEdit* m_amountEdit;
    QPushButton* m_createTxButton;
    QPushButton* m_signTxButton;
    QPushButton* m_combineButton;
    QTableWidget* m_transactionTable;
    
    // Button Group
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_saveConfigButton;
    QPushButton* m_loadConfigButton;
    QPushButton* m_refreshButton;
    
    // Data
    WalletModel* m_walletModel;
    QString m_multisigAddress;
    int m_requiredSignatures;
    int m_totalSigners;
    QStringList m_cosigners;
    QStringList m_pendingTransactions;
    QStringList m_signedTransactions;
    QString m_currentTransaction;
    
    // Settings
    QSettings* m_settings;
    
    // Update timer
    QTimer* m_updateTimer;
    
    // Constants
    static constexpr const char* SETTINGS_GROUP = "MultisigWallet";
    static constexpr const char* ADDRESS_KEY = "MultisigAddress";
    static constexpr const char* REQUIRED_KEY = "RequiredSignatures";
    static constexpr const char* TOTAL_KEY = "TotalSigners";
    static constexpr const char* COSIGNERS_KEY = "Cosigners";
    static constexpr int UPDATE_INTERVAL_MS = 30000; // 30 seconds
    static constexpr int MAX_COSIGNERS = 15;
    static constexpr int MIN_REQUIRED = 1;
    static constexpr int MAX_REQUIRED = 15;
};

#endif // SHAHCOIN_QT_MULTISIGWALLET_H
