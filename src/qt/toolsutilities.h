#ifndef TOOLSUTILITIES_H
#define TOOLSUTILITIES_H

#include <QWidget>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QTableWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QBuffer>
#include <QImage>
#include <QPixmap>
#include <QClipboard>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QSize>
#include <QPoint>
#include <QRect>
#include <QColor>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QToolTip>
#include <QWhatsThis>
#include <QHelpEvent>
#include <QToolButton>
#include <QLineEdit>
#include <QTextBrowser>
#include <QScrollArea>
#include <QFrame>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolBar>
#include <QStatusBar>
#include <QMainWindow>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QProgressDialog>
#include <QInputDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPageSetupDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPrintEngine>
#include <QPrinterInfo>
#include <QPageSize>
#include <QPageLayout>
#include <QMargins>
#include <QPageRanges>
#include <QPrintRange>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPrintEngine>
#include <QPrinterInfo>
#include <QPageSize>
#include <QPageLayout>
#include <QMargins>
#include <QPageRanges>
#include <QPrintRange>

class WalletModel;

// Token/NFT Builder structure
struct TokenBuilderConfig {
    QString name;
    QString symbol;
    QString description;
    QString website;
    QString logo;
    int decimals;
    double initialSupply;
    bool mintable;
    bool burnable;
    bool pausable;
    bool upgradeable;
    QString owner;
    QString tier;
    QMap<QString, QString> metadata;
    QList<QString> features;
    QString sourceCode;
    QString bytecode;
    QString abi;
    bool verified;
    QDateTime createdAt;
    QString creator;
};

// NFT Builder structure
struct NFTBuilderConfig {
    QString name;
    QString symbol;
    QString description;
    QString image;
    QString externalUrl;
    QString animationUrl;
    QString attributes;
    int totalSupply;
    bool mintable;
    bool burnable;
    bool pausable;
    bool upgradeable;
    QString owner;
    QString tier;
    QMap<QString, QString> metadata;
    QList<QString> features;
    QString sourceCode;
    QString bytecode;
    QString abi;
    bool verified;
    QDateTime createdAt;
    QString creator;
};

// Legacy Backup structure
struct LegacyBackup {
    QString filename;
    QString type; // "qr", "paper", "encrypted"
    QString address;
    QString privateKey;
    QString publicKey;
    QString seedPhrase;
    QString password;
    QString encryptionMethod;
    QDateTime createdAt;
    QDateTime expiresAt;
    bool isEncrypted;
    bool isCompressed;
    QString description;
    QString warnings;
    QByteArray qrCodeData;
    QString paperWalletText;
    QString backupInstructions;
};

// Offline Transaction structure
struct OfflineTransaction {
    QString txId;
    QString type; // "unsigned", "signed", "broadcast"
    QString fromAddress;
    QString toAddress;
    double amount;
    QString currency;
    double fee;
    QString feeCurrency;
    QString data;
    QString signature;
    QString rawTx;
    QDateTime createdAt;
    QDateTime signedAt;
    QDateTime broadcastAt;
    QString status;
    QString error;
    bool isConfirmed;
    int confirmations;
    QString blockHash;
    int blockHeight;
    QString memo;
    QString metadata;
};

// Tools & Utilities - Main class for wallet tools
class ToolsUtilities : public QWidget
{
    Q_OBJECT

public:
    explicit ToolsUtilities(WalletModel* walletModel, QWidget* parent = nullptr);
    ~ToolsUtilities();

    // Core functionality
    void initialize();
    void startTools();
    void stopTools();
    bool isToolsActive() const;

    // Token/NFT Builder
    void createToken(const TokenBuilderConfig& config);
    void createNFT(const NFTBuilderConfig& config);
    void validateTokenConfig(const TokenBuilderConfig& config);
    void validateNFTConfig(const NFTBuilderConfig& config);
    void generateTokenSourceCode(const TokenBuilderConfig& config);
    void generateNFTSourceCode(const NFTBuilderConfig& config);
    void deployToken(const TokenBuilderConfig& config);
    void deployNFT(const NFTBuilderConfig& config);
    void verifyToken(const QString& contractAddress);
    void verifyNFT(const QString& contractAddress);

    // Legacy Backup System
    void createQRBackup(const QString& address, const QString& privateKey);
    void createPaperWallet(const QString& address, const QString& privateKey);
    void createEncryptedBackup(const QString& data, const QString& password);
    void importBackup(const QString& filename);
    void exportBackup(const QString& filename, const QString& type);
    void validateBackup(const QString& filename);
    void restoreFromBackup(const QString& filename, const QString& password);
    void listBackups();
    void deleteBackup(const QString& filename);
    void encryptBackup(const QString& filename, const QString& password);
    void decryptBackup(const QString& filename, const QString& password);

    // Offline Transaction Signing
    void createUnsignedTransaction(const QString& fromAddress, const QString& toAddress, double amount);
    void signTransaction(const QString& txHex, const QString& privateKey);
    void broadcastTransaction(const QString& signedTxHex);
    void validateTransaction(const QString& txHex);
    void estimateTransactionFee(const QString& txHex);
    void createBatchTransaction(const QList<QPair<QString, double>>& recipients);
    void signBatchTransaction(const QString& batchTxHex, const QString& privateKey);
    void broadcastBatchTransaction(const QString& signedBatchTxHex);
    void createMultiSigTransaction(const QString& fromAddress, const QString& toAddress, double amount, const QStringList& signers);
    void signMultiSigTransaction(const QString& txHex, const QString& privateKey, int signerIndex);
    void broadcastMultiSigTransaction(const QString& signedTxHex);

    // Settings
    void setAutoSave(bool enabled);
    void setBackupEncryption(bool enabled);
    void setOfflineMode(bool enabled);
    void setTransactionConfirmation(bool enabled);
    void setFeeEstimation(bool enabled);
    void setSourceCodeValidation(bool enabled);

    bool isAutoSave() const;
    bool isBackupEncryption() const;
    bool isOfflineMode() const;
    bool isTransactionConfirmation() const;
    bool isFeeEstimation() const;
    bool isSourceCodeValidation() const;

    // Data access
    QList<TokenBuilderConfig> getTokenConfigs() const;
    QList<NFTBuilderConfig> getNFTConfigs() const;
    QList<LegacyBackup> getBackups() const;
    QList<OfflineTransaction> getOfflineTransactions() const;

signals:
    void tokenCreated(const TokenBuilderConfig& config);
    void nftCreated(const NFTBuilderConfig& config);
    void tokenDeployed(const QString& contractAddress);
    void nftDeployed(const QString& contractAddress);
    void backupCreated(const LegacyBackup& backup);
    void backupImported(const LegacyBackup& backup);
    void backupRestored(const QString& address);
    void transactionSigned(const OfflineTransaction& transaction);
    void transactionBroadcast(const OfflineTransaction& transaction);
    void toolsStarted();
    void toolsStopped();
    void operationCompleted(const QString& operation);
    void operationFailed(const QString& operation, const QString& error);

private slots:
    void onTokenBuilderTab();
    void onNFTBuilderTab();
    void onLegacyBackupTab();
    void onOfflineSigningTab();
    void onCreateTokenClicked();
    void onCreateNFTClicked();
    void onCreateBackupClicked();
    void onImportBackupClicked();
    void onCreateTransactionClicked();
    void onSignTransactionClicked();
    void onBroadcastTransactionClicked();
    void onValidateConfigClicked();
    void onGenerateSourceClicked();
    void onDeployContractClicked();
    void onVerifyContractClicked();
    void onExportBackupClicked();
    void onDeleteBackupClicked();
    void onEncryptBackupClicked();
    void onDecryptBackupClicked();
    void onEstimateFeeClicked();
    void onCreateBatchTxClicked();
    void onSignBatchTxClicked();
    void onBroadcastBatchTxClicked();
    void onCreateMultiSigTxClicked();
    void onSignMultiSigTxClicked();
    void onBroadcastMultiSigTxClicked();

private:
    // Core members
    WalletModel* m_walletModel;
    QSettings m_settings;
    bool m_isToolsActive;
    bool m_autoSave;
    bool m_backupEncryption;
    bool m_offlineMode;
    bool m_transactionConfirmation;
    bool m_feeEstimation;
    bool m_sourceCodeValidation;

    // Network
    QNetworkAccessManager* m_networkManager;

    // Data
    QList<TokenBuilderConfig> m_tokenConfigs;
    QList<NFTBuilderConfig> m_nftConfigs;
    QList<LegacyBackup> m_backups;
    QList<OfflineTransaction> m_offlineTransactions;
    QMutex m_dataMutex;

    // UI components
    QTabWidget* m_tabWidget;
    
    // Token Builder Tab
    QWidget* m_tokenBuilderTab;
    QLineEdit* m_tokenNameEdit;
    QLineEdit* m_tokenSymbolEdit;
    QTextEdit* m_tokenDescriptionEdit;
    QLineEdit* m_tokenWebsiteEdit;
    QLineEdit* m_tokenLogoEdit;
    QSpinBox* m_tokenDecimalsSpin;
    QDoubleSpinBox* m_tokenSupplySpin;
    QCheckBox* m_tokenMintableCheck;
    QCheckBox* m_tokenBurnableCheck;
    QCheckBox* m_tokenPausableCheck;
    QCheckBox* m_tokenUpgradeableCheck;
    QLineEdit* m_tokenOwnerEdit;
    QComboBox* m_tokenTierCombo;
    QTextEdit* m_tokenMetadataEdit;
    QListWidget* m_tokenFeaturesList;
    QPushButton* m_createTokenButton;
    QPushButton* m_validateTokenButton;
    QPushButton* m_generateTokenSourceButton;
    QPushButton* m_deployTokenButton;
    QPushButton* m_verifyTokenButton;
    QTextEdit* m_tokenSourceCodeEdit;
    QTextEdit* m_tokenBytecodeEdit;
    QTextEdit* m_tokenAbiEdit;

    // NFT Builder Tab
    QWidget* m_nftBuilderTab;
    QLineEdit* m_nftNameEdit;
    QLineEdit* m_nftSymbolEdit;
    QTextEdit* m_nftDescriptionEdit;
    QLineEdit* m_nftImageEdit;
    QLineEdit* m_nftExternalUrlEdit;
    QLineEdit* m_nftAnimationUrlEdit;
    QTextEdit* m_nftAttributesEdit;
    QSpinBox* m_nftSupplySpin;
    QCheckBox* m_nftMintableCheck;
    QCheckBox* m_nftBurnableCheck;
    QCheckBox* m_nftPausableCheck;
    QCheckBox* m_nftUpgradeableCheck;
    QLineEdit* m_nftOwnerEdit;
    QComboBox* m_nftTierCombo;
    QTextEdit* m_nftMetadataEdit;
    QListWidget* m_nftFeaturesList;
    QPushButton* m_createNFTButton;
    QPushButton* m_validateNFTButton;
    QPushButton* m_generateNFTSourceButton;
    QPushButton* m_deployNFTButton;
    QPushButton* m_verifyNFTButton;
    QTextEdit* m_nftSourceCodeEdit;
    QTextEdit* m_nftBytecodeEdit;
    QTextEdit* m_nftAbiEdit;

    // Legacy Backup Tab
    QWidget* m_legacyBackupTab;
    QTableWidget* m_backupsTable;
    QPushButton* m_createBackupButton;
    QPushButton* m_importBackupButton;
    QPushButton* m_exportBackupButton;
    QPushButton* m_deleteBackupButton;
    QPushButton* m_encryptBackupButton;
    QPushButton* m_decryptBackupButton;
    QComboBox* m_backupTypeCombo;
    QLineEdit* m_backupPasswordEdit;
    QTextEdit* m_backupInstructionsEdit;
    QLabel* m_backupStatusLabel;
    QProgressBar* m_backupProgressBar;

    // Offline Signing Tab
    QWidget* m_offlineSigningTab;
    QTableWidget* m_transactionsTable;
    QLineEdit* m_fromAddressEdit;
    QLineEdit* m_toAddressEdit;
    QDoubleSpinBox* m_amountSpin;
    QDoubleSpinBox* m_feeSpin;
    QTextEdit* m_transactionDataEdit;
    QTextEdit* m_signatureEdit;
    QTextEdit* m_rawTransactionEdit;
    QPushButton* m_createTransactionButton;
    QPushButton* m_signTransactionButton;
    QPushButton* m_broadcastTransactionButton;
    QPushButton* m_validateTransactionButton;
    QPushButton* m_estimateFeeButton;
    QPushButton* m_createBatchTxButton;
    QPushButton* m_signBatchTxButton;
    QPushButton* m_broadcastBatchTxButton;
    QPushButton* m_createMultiSigTxButton;
    QPushButton* m_signMultiSigTxButton;
    QPushButton* m_broadcastMultiSigTxButton;
    QLabel* m_transactionStatusLabel;
    QProgressBar* m_transactionProgressBar;

    // Helper methods
    void setupUI();
    void setupTokenBuilderTab();
    void setupNFTBuilderTab();
    void setupLegacyBackupTab();
    void setupOfflineSigningTab();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    void updateTokenBuilderUI();
    void updateNFTBuilderUI();
    void updateBackupsTable();
    void updateTransactionsTable();
    void generateTokenSourceCode();
    void generateNFTSourceCode();
    void validateTokenConfig();
    void validateNFTConfig();
    void createTokenConfig();
    void createNFTConfig();
    void deployTokenContract();
    void deployNFTContract();
    void verifyTokenContract();
    void verifyNFTContract();
    void createBackup();
    void importBackup();
    void exportBackup();
    void deleteBackup();
    void encryptBackup();
    void decryptBackup();
    void createTransaction();
    void signTransaction();
    void broadcastTransaction();
    void validateTransaction();
    void estimateFee();
    void createBatchTransaction();
    void signBatchTransaction();
    void broadcastBatchTransaction();
    void createMultiSigTransaction();
    void signMultiSigTransaction();
    void broadcastMultiSigTransaction();
    void generateQRCode(const QString& data);
    void generatePaperWallet(const QString& address, const QString& privateKey);
    void encryptData(const QString& data, const QString& password);
    void decryptData(const QString& encryptedData, const QString& password);
    void validateAddress(const QString& address);
    void validatePrivateKey(const QString& privateKey);
    void validateSeedPhrase(const QString& seedPhrase);
    void generateKeyPair();
    void deriveAddress(const QString& privateKey);
    void derivePrivateKey(const QString& seedPhrase);
    void deriveSeedPhrase(const QString& entropy);
    void calculateTransactionFee(const QString& txHex);
    void validateTransactionSignature(const QString& txHex, const QString& signature);
    void broadcastTransactionToNetwork(const QString& signedTxHex);
    void monitorTransactionStatus(const QString& txId);
    void handleNetworkError(const QString& error);
    void handleApiResponse(const QJsonDocument& response);
    void cleanupOldData();
    void exportDataToFile(const QString& filename, const QJsonDocument& data);
    QString generateTokenId() const;
    QString generateNFTId() const;
    QString generateBackupId() const;
    QString generateTransactionId() const;
    QString formatAddress(const QString& address) const;
    QString formatAmount(double amount) const;
    QString formatFee(double fee) const;
    QString formatDateTime(const QDateTime& dateTime) const;
    QColor getStatusColor(const QString& status) const;
    void showCreateTokenDialog();
    void showCreateNFTDialog();
    void showCreateBackupDialog();
    void showImportBackupDialog();
    void showCreateTransactionDialog();
    void showSignTransactionDialog();
    void showBroadcastTransactionDialog();
    void showValidationDialog(const QString& type, const QString& result);
    void showErrorDialog(const QString& error);
    void showSuccessDialog(const QString& message);
    void showWarningDialog(const QString& warning);
    void showInfoDialog(const QString& info);
    void logTokenCreation(const TokenBuilderConfig& config);
    void logNFTCreation(const NFTBuilderConfig& config);
    void logBackupCreation(const LegacyBackup& backup);
    void logTransactionCreation(const OfflineTransaction& transaction);
    void logOperation(const QString& operation, const QString& result);
    void logError(const QString& error);
    void logInfo(const QString& message);
    void logDebug(const QString& message);
};

#endif // TOOLSUTILITIES_H
