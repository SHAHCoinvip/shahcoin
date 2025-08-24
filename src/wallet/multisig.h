#ifndef SHAHCOIN_WALLET_MULTISIG_H
#define SHAHCOIN_WALLET_MULTISIG_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QMutex>

#include <script/script.h>
#include <script/standard.h>
#include <primitives/transaction.h>
#include <key.h>
#include <pubkey.h>
#include <amount.h>

class WalletModel;
class CWallet;
class CTransaction;

/**
 * Multisig Wallet Support for Shahcoin
 * 
 * Provides multi-signature wallet capabilities including:
 * - M-of-N signature schemes
 * - Shared transaction signing
 * - Redeem script generation
 * - Transaction export/import
 * - Co-signer management
 */
class MultisigWallet : public QObject
{
    Q_OBJECT

public:
    enum MultisigType {
        TwoOfThree,    // 2-of-3 multisig
        ThreeOfFive,   // 3-of-5 multisig
        Custom         // Custom M-of-N
    };

    enum TransactionStatus {
        Unsigned,
        PartiallySigned,
        FullySigned,
        Broadcast,
        Confirmed,
        Failed
    };

    struct MultisigConfig {
        MultisigType type;
        int requiredSignatures;
        int totalSignatures;
        QStringList publicKeys;
        QString redeemScript;
        QString address;
        QString description;
        bool isActive;
    };

    struct MultisigTransaction {
        QString transactionId;
        QString rawTransaction;
        QStringList signatures;
        QStringList signers;
        TransactionStatus status;
        CAmount amount;
        QString recipientAddress;
        QString changeAddress;
        int confirmations;
        QDateTime timestamp;
        QString memo;
    };

    struct CoSigner {
        QString name;
        QString publicKey;
        QString address;
        bool isOnline;
        bool isTrusted;
        QString contactInfo;
    };

    explicit MultisigWallet(QObject* parent = nullptr);
    ~MultisigWallet();

    // Configuration
    void setWalletModel(WalletModel* model);
    void createMultisigWallet(const MultisigConfig& config);
    void loadMultisigWallet(const QString& walletId);
    void saveMultisigWallet(const QString& walletId);
    void deleteMultisigWallet(const QString& walletId);

    // Wallet management
    QList<MultisigConfig> getMultisigWallets() const;
    MultisigConfig getMultisigWallet(const QString& walletId) const;
    bool isMultisigWallet(const QString& address) const;
    QString getRedeemScript(const QString& address) const;

    // Transaction management
    QString createMultisigTransaction(const QString& recipient, CAmount amount, 
                                     const QString& changeAddress = "", const QString& memo = "");
    bool signTransaction(const QString& transactionId, const QString& signature);
    bool verifyTransaction(const QString& transactionId);
    bool broadcastTransaction(const QString& transactionId);
    QList<MultisigTransaction> getMultisigTransactions() const;
    MultisigTransaction getMultisigTransaction(const QString& transactionId) const;

    // Co-signer management
    void addCoSigner(const QString& walletId, const CoSigner& signer);
    void removeCoSigner(const QString& walletId, const QString& publicKey);
    void updateCoSigner(const QString& walletId, const CoSigner& signer);
    QList<CoSigner> getCoSigners(const QString& walletId) const;
    bool isCoSigner(const QString& walletId, const QString& publicKey) const;

    // Export/Import
    QString exportTransaction(const QString& transactionId);
    bool importTransaction(const QString& rawTransaction);
    QString exportWallet(const QString& walletId);
    bool importWallet(const QString& walletData);

    // Validation
    bool validateMultisigConfig(const MultisigConfig& config);
    bool validateTransaction(const QString& transactionId);
    bool validateSignature(const QString& transactionId, const QString& signature);

    // Settings
    void setAutoSign(bool autoSign);
    void setRequireAllSignatures(bool requireAll);
    void setAllowPartialSignatures(bool allowPartial);
    void setDefaultTimeout(int timeout);

    bool isAutoSign() const;
    bool isRequireAllSignatures() const;
    bool isAllowPartialSignatures() const;
    int getDefaultTimeout() const;

public Q_SLOTS:
    void refreshWallets();
    void refreshTransactions();
    void checkForNewTransactions();
    void validateAllTransactions();

private Q_SLOTS:
    void onWalletTransactionChanged();
    void onCoSignerStatusChanged();

private:
    // Core functionality
    void generateRedeemScript(MultisigConfig& config);
    void generateMultisigAddress(MultisigConfig& config);
    bool validatePublicKeys(const QStringList& publicKeys);
    bool validateRedeemScript(const QString& redeemScript);

    // Transaction processing
    void processMultisigTransaction(const QString& transactionId);
    void updateTransactionStatus(const QString& transactionId, TransactionStatus status);
    void addSignatureToTransaction(const QString& transactionId, const QString& signature, const QString& signer);
    bool verifyTransactionSignatures(const QString& transactionId);

    // Co-signer management
    void updateCoSignerStatus(const QString& walletId, const QString& publicKey, bool isOnline);
    void validateCoSignerTrust(const QString& walletId, const QString& publicKey);

    // Storage
    void saveMultisigConfigs();
    void loadMultisigConfigs();
    void saveMultisigTransactions();
    void loadMultisigTransactions();
    void saveCoSigners();
    void loadCoSigners();

    // Utility functions
    QString generateWalletId() const;
    QString formatTransactionData(const MultisigTransaction& transaction);
    MultisigTransaction parseTransactionData(const QString& data);
    void logMultisigEvent(const QString& event, const QString& details);
    void saveSettings();
    void loadSettings();

    // Data members
    WalletModel* m_walletModel;
    QMap<QString, MultisigConfig> m_multisigWallets;
    QMap<QString, MultisigTransaction> m_multisigTransactions;
    QMap<QString, QList<CoSigner>> m_coSigners;
    
    // Settings
    bool m_autoSign;
    bool m_requireAllSignatures;
    bool m_allowPartialSignatures;
    int m_defaultTimeout;
    
    // State
    bool m_isInitialized;
    QMutex m_dataMutex;
};

/**
 * Multisig Transaction Builder
 * 
 * Helps build and manage multisig transactions
 */
class MultisigTransactionBuilder
{
public:
    struct TransactionInput {
        QString txid;
        int vout;
        CAmount amount;
        QString scriptPubKey;
    };

    struct TransactionOutput {
        QString address;
        CAmount amount;
        QString scriptPubKey;
    };

    explicit MultisigTransactionBuilder(const MultisigConfig& config);

    // Transaction building
    void addInput(const TransactionInput& input);
    void addOutput(const TransactionOutput& output);
    void setChangeAddress(const QString& address);
    void setFee(CAmount fee);
    
    // Transaction creation
    QString buildTransaction();
    QString buildUnsignedTransaction();
    bool signTransaction(const QString& privateKey);
    bool verifyTransaction();

    // Getters
    QString getTransactionId() const;
    QString getRawTransaction() const;
    QList<QString> getSignatures() const;
    bool isFullySigned() const;

private:
    MultisigConfig m_config;
    QList<TransactionInput> m_inputs;
    QList<TransactionOutput> m_outputs;
    QString m_changeAddress;
    CAmount m_fee;
    QString m_transactionId;
    QString m_rawTransaction;
    QList<QString> m_signatures;
    
    // Internal methods
    void calculateTransactionId();
    bool validateInputs();
    bool validateOutputs();
    CAmount calculateTotalInput() const;
    CAmount calculateTotalOutput() const;
};

/**
 * Multisig Signature Manager
 * 
 * Manages signatures for multisig transactions
 */
class MultisigSignatureManager
{
public:
    struct Signature {
        QString publicKey;
        QString signature;
        QString transactionId;
        QDateTime timestamp;
        bool isValid;
    };

    explicit MultisigSignatureManager(const MultisigConfig& config);

    // Signature management
    bool addSignature(const QString& publicKey, const QString& signature, const QString& transactionId);
    bool removeSignature(const QString& publicKey, const QString& transactionId);
    bool verifySignature(const QString& publicKey, const QString& signature, const QString& transactionId);
    bool isSignatureValid(const QString& publicKey, const QString& signature) const;

    // Signature collection
    QList<Signature> getSignatures(const QString& transactionId) const;
    int getSignatureCount(const QString& transactionId) const;
    bool hasEnoughSignatures(const QString& transactionId) const;
    bool isFullySigned(const QString& transactionId) const;

    // Validation
    bool validateAllSignatures(const QString& transactionId);
    bool validatePublicKey(const QString& publicKey) const;

private:
    MultisigConfig m_config;
    QMap<QString, QList<Signature>> m_signatures;
    
    // Internal methods
    bool verifySignatureWithPublicKey(const QString& publicKey, const QString& signature, const QString& transactionId);
    bool isPublicKeyInConfig(const QString& publicKey) const;
    void cleanupInvalidSignatures();
};

#endif // SHAHCOIN_WALLET_MULTISIG_H
