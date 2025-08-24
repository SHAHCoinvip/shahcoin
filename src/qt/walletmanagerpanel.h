#ifndef SHAHCOIN_QT_WALLETMANAGERPANEL_H
#define SHAHCOIN_QT_WALLETMANAGERPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QProgressBar>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QTimer>
#include <QMessageBox>
#include <QDialog>
#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QToolTip>
#include <QStatusBar>
#include <QFrame>
#include <QScrollArea>
#include <QTabWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QCalendarWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class WalletModel;
class WalletController;
class PlatformStyle;

/**
 * Multi-Wallet Manager Panel
 * 
 * Provides a comprehensive interface for managing multiple wallets
 * within the Shahcoin Qt GUI. Users can create, load, switch, backup,
 * encrypt, and manage multiple wallet files.
 */
class WalletManagerPanel : public QWidget
{
    Q_OBJECT
public:
    explicit WalletManagerPanel(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~WalletManagerPanel();
    void setWalletController(WalletController* walletController);
    void setWalletModel(WalletModel* walletModel);

public Q_SLOTS:
    void refreshData();
    void updateWalletList();

private Q_SLOTS:
    void onCreateWalletClicked();
    void onLoadWalletClicked();
    void onSwitchWalletClicked();
    void onBackupWalletClicked();
    void onEncryptWalletClicked();
    void onUnloadWalletClicked();
    void onDeleteWalletClicked();
    void onRefreshClicked();
    void onExportClicked();
    void onWalletRowClicked(int row, int column);
    void onWalletContextMenu(const QPoint& pos);
    void onViewWalletDetails();
    void onRenameWallet();
    void onAddWalletNote();
    void onShowWalletStats();
    void onConfirmWalletAction();
    void onCancelWalletAction();

private:
    void setupUI();
    void setupWalletTable();
    void setupControls();
    void setupDialogs();
    void connectSignals();
    void applyTheme();
    void loadWalletData();
    void populateWalletTable();
    void updateWalletStatus();
    void showWalletDetailsDialog(const QString& walletName);
    void showCreateWalletDialog();
    void showLoadWalletDialog();
    void showBackupWalletDialog(const QString& walletName);
    void showEncryptWalletDialog(const QString& walletName);
    void showDeleteWalletDialog(const QString& walletName);
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void refreshWalletBalances();
    QString formatBalance(qint64 balance);
    QString formatWalletSize(qint64 size);
    QString getWalletStatus(const QString& walletName);
    bool isWalletLoaded(const QString& walletName);
    bool isWalletEncrypted(const QString& walletName);
                    void exportWalletList();
                void importWalletList();
                void updateStatusMessage(const QString& message);
                QString calculatePasswordStrength(const QString& password);
                void updatePasswordStrengthMeter(const QString& password);
                void setupLanguageSelector();
                void onLanguageChanged(const QString& language);
                void loadLanguageSettings();
                void saveLanguageSettings(const QString& language);
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Main Tab
    QWidget* m_mainTab;
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_walletGroup;
    QTableWidget* m_walletTable;
    QHBoxLayout* m_controlsLayout;
    QPushButton* m_createWalletButton;
    QPushButton* m_loadWalletButton;
    QPushButton* m_refreshButton;
    QPushButton* m_exportButton;
    
    // Stats Tab
    QWidget* m_statsTab;
    QVBoxLayout* m_statsLayout;
    QGroupBox* m_totalBalanceGroup;
    QLabel* m_totalBalanceLabel;
    QLabel* m_totalWalletsLabel;
    QLabel* m_loadedWalletsLabel;
    QLabel* m_encryptedWalletsLabel;
    QProgressBar* m_syncProgressBar;
    
    // Settings Tab
    QWidget* m_settingsTab;
    QVBoxLayout* m_settingsLayout;
    QGroupBox* m_autoLoadGroup;
    QCheckBox* m_autoLoadDefaultCheck;
    QCheckBox* m_rememberLastWalletCheck;
    QCheckBox* m_showWalletNotesCheck;
    QGroupBox* m_backupGroup;
    QCheckBox* m_autoBackupCheck;
    QSpinBox* m_backupIntervalSpinBox;
    QLineEdit* m_backupPathEdit;
    QPushButton* m_backupPathButton;
    
    // Create Wallet Dialog
    QDialog* m_createWalletDialog;
    QVBoxLayout* m_createWalletLayout;
    QFormLayout* m_createWalletForm;
    QLineEdit* m_walletNameEdit;
    QLineEdit* m_walletDescriptionEdit;
    QCheckBox* m_encryptWalletCheck;
    QLineEdit* m_walletPasswordEdit;
    QLineEdit* m_walletPasswordConfirmEdit;
    QProgressBar* m_passwordStrengthMeter;
    QLabel* m_passwordStrengthLabel;
    QLabel* m_passwordStrengthTip;
    QHBoxLayout* m_createWalletButtonLayout;
    QPushButton* m_createWalletConfirmButton;
    QPushButton* m_createWalletCancelButton;
    
    // Load Wallet Dialog
    QDialog* m_loadWalletDialog;
    QVBoxLayout* m_loadWalletLayout;
    QFormLayout* m_loadWalletForm;
    QLineEdit* m_loadWalletPathEdit;
    QPushButton* m_loadWalletBrowseButton;
    QCheckBox* m_loadWalletEncryptedCheck;
    QLineEdit* m_loadWalletPasswordEdit;
    QHBoxLayout* m_loadWalletButtonLayout;
    QPushButton* m_loadWalletConfirmButton;
    QPushButton* m_loadWalletCancelButton;
    
    // Backup Wallet Dialog
    QDialog* m_backupWalletDialog;
    QVBoxLayout* m_backupWalletLayout;
    QFormLayout* m_backupWalletForm;
    QLineEdit* m_backupWalletPathEdit;
    QPushButton* m_backupWalletBrowseButton;
    QCheckBox* m_backupWalletEncryptedCheck;
    QLineEdit* m_backupWalletPasswordEdit;
    QHBoxLayout* m_backupWalletButtonLayout;
    QPushButton* m_backupWalletConfirmButton;
    QPushButton* m_backupWalletCancelButton;
    
    // Encrypt Wallet Dialog
    QDialog* m_encryptWalletDialog;
    QVBoxLayout* m_encryptWalletLayout;
    QFormLayout* m_encryptWalletForm;
    QLineEdit* m_encryptWalletPasswordEdit;
    QLineEdit* m_encryptWalletPasswordConfirmEdit;
    QHBoxLayout* m_encryptWalletButtonLayout;
    QPushButton* m_encryptWalletConfirmButton;
    QPushButton* m_encryptWalletCancelButton;
    
    // Delete Wallet Dialog
    QDialog* m_deleteWalletDialog;
    QVBoxLayout* m_deleteWalletLayout;
    QLabel* m_deleteWalletWarningLabel;
    QCheckBox* m_deleteWalletConfirmCheck;
    QHBoxLayout* m_deleteWalletButtonLayout;
    QPushButton* m_deleteWalletConfirmButton;
    QPushButton* m_deleteWalletCancelButton;
    
    // Wallet Details Dialog
    QDialog* m_walletDetailsDialog;
    QVBoxLayout* m_walletDetailsLayout;
    QFormLayout* m_walletDetailsForm;
    QLabel* m_walletDetailsNameLabel;
    QLabel* m_walletDetailsPathLabel;
    QLabel* m_walletDetailsBalanceLabel;
    QLabel* m_walletDetailsStatusLabel;
    QLabel* m_walletDetailsCreatedLabel;
    QLabel* m_walletDetailsSizeLabel;
    QTextEdit* m_walletDetailsNotesEdit;
    QHBoxLayout* m_walletDetailsButtonLayout;
    QPushButton* m_walletDetailsSaveButton;
    QPushButton* m_walletDetailsCloseButton;
    
                    // Data
                WalletController* m_walletController;
                WalletModel* m_walletModel;
                const PlatformStyle* m_platformStyle;
                QTimer* m_refreshTimer;
                QLabel* m_statusTextLabel;
    
    // Language Selector
    QComboBox* m_languageSelector;
    QLabel* m_languageLabel;
    
    // Wallet Data
    struct WalletInfo {
        QString name;
        QString path;
        QString description;
        qint64 balance;
        QString status;
        bool isLoaded;
        bool isEncrypted;
        bool isColdWallet;
        QDateTime createdDate;
        qint64 fileSize;
        QString notes;
        int transactionCount;
        QString lastUsed;
    };
    QList<WalletInfo> m_walletList;
    
    // Current Selection
    QString m_selectedWalletName;
    QString m_currentWalletName;
    
    // Settings
    QSettings* m_settings;
    
    // Context Menu
    QMenu* m_walletContextMenu;
    QAction* m_viewDetailsAction;
    QAction* m_switchAction;
    QAction* m_backupAction;
    QAction* m_encryptAction;
    QAction* m_unloadAction;
    QAction* m_renameAction;
    QAction* m_deleteAction;
};

/**
 * Wallet Statistics Widget
 * 
 * Shows detailed statistics for all wallets
 */
class WalletStatsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WalletStatsWidget(QWidget* parent = nullptr);

public Q_SLOTS:
    void updateStats(const QList<WalletManagerPanel::WalletInfo>& wallets);

private:
    void setupUI();
    void applyTheme();
    
    QVBoxLayout* m_layout;
    QGroupBox* m_summaryGroup;
    QGridLayout* m_summaryLayout;
    QLabel* m_totalBalanceLabel;
    QLabel* m_totalWalletsLabel;
    QLabel* m_loadedWalletsLabel;
    QLabel* m_encryptedWalletsLabel;
    QLabel* m_totalTransactionsLabel;
    QLabel* m_oldestWalletLabel;
    QLabel* m_newestWalletLabel;
    
    QGroupBox* m_chartGroup;
    QLabel* m_balanceChartLabel;
    QLabel* m_walletTypeChartLabel;
};

#endif // SHAHCOIN_QT_WALLETMANAGERPANEL_H
