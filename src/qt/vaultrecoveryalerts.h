#ifndef SHAHCOIN_QT_VAULTRECOVERYALERTS_H
#define SHAHCOIN_QT_VAULTRECOVERYALERTS_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QProgressBar>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

class WalletModel;
class PlatformStyle;

/**
 * @brief Vault Recovery Alerts System
 * 
 * Detects wallet.dat issues and provides recovery options:
 * - Missing wallet.dat
 * - Corrupted wallet.dat
 * - Outdated wallet.dat
 * - Recovery from backup
 * - Rescan options
 */
class VaultRecoveryAlerts : public QDialog
{
    Q_OBJECT

public:
    enum RecoveryAction {
        RestoreFromBackup,
        RescanWallet,
        CreateNewWallet,
        RepairWallet,
        IgnoreWarning
    };

    enum WalletStatus {
        StatusOK,
        StatusMissing,
        StatusCorrupted,
        StatusOutdated,
        StatusLocked,
        StatusUnknown
    };

    explicit VaultRecoveryAlerts(WalletModel* walletModel, QWidget* parent = nullptr);
    ~VaultRecoveryAlerts();

    // Main detection and recovery methods
    void checkWalletHealth();
    void showRecoveryDialog(WalletStatus status, const QString& details = QString());
    void performRecoveryAction(RecoveryAction action);
    
    // Static methods for integration
    static bool shouldCheckWalletHealth();
    static void setLastHealthCheck();
    static WalletStatus detectWalletStatus(const QString& walletPath = QString());

signals:
    void walletRecoveryCompleted(bool success, const QString& message);
    void walletHealthCheckCompleted(WalletStatus status, const QString& details);

public Q_SLOTS:
    void onCheckWalletHealth();
    void onRestoreFromBackup();
    void onRescanWallet();
    void onCreateNewWallet();
    void onRepairWallet();
    void onIgnoreWarning();
    void onShowDetails();

private Q_SLOTS:
    void onHealthCheckFinished();
    void onRecoveryActionFinished();
    void onBackupFileSelected(const QString& filePath);

private:
    void setupUI();
    void connectSignals();
    void applyTheme();
    
    // Detection methods
    bool checkWalletFileExists(const QString& walletPath);
    bool checkWalletFileCorruption(const QString& walletPath);
    bool checkWalletFileVersion(const QString& walletPath);
    bool checkWalletFileLock(const QString& walletPath);
    QString getWalletFilePath() const;
    
    // Recovery methods
    bool restoreWalletFromBackup(const QString& backupPath);
    bool rescanWallet();
    bool createNewWallet();
    bool repairWallet();
    
    // UI helpers
    void updateStatusDisplay(WalletStatus status, const QString& details);
    void showProgressDialog(const QString& title, const QString& message);
    void hideProgressDialog();
    void enableRecoveryButtons(bool enabled);
    
    // Settings
    void saveRecoverySettings();
    void loadRecoverySettings();

    // Members
    WalletModel* m_walletModel;
    PlatformStyle* m_platformStyle;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_statusGroup;
    QLabel* m_statusIcon;
    QLabel* m_statusLabel;
    QLabel* m_detailsLabel;
    QTextEdit* m_detailsText;
    QPushButton* m_showDetailsButton;
    
    QGroupBox* m_recoveryGroup;
    QPushButton* m_restoreButton;
    QPushButton* m_rescanButton;
    QPushButton* m_createButton;
    QPushButton* m_repairButton;
    QPushButton* m_ignoreButton;
    
    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;
    
    // State
    WalletStatus m_currentStatus;
    QString m_currentDetails;
    QString m_walletPath;
    bool m_recoveryInProgress;
    
    // Threading
    QFutureWatcher<WalletStatus> m_healthCheckWatcher;
    QFutureWatcher<bool> m_recoveryWatcher;
    QMutex m_recoveryMutex;
    
    // Settings
    QSettings* m_settings;
    bool m_autoCheckEnabled;
    int m_checkIntervalHours;
    QDateTime m_lastCheckTime;
    
    // Constants
    static const int CHECK_INTERVAL_HOURS = 24;
    static const int PROGRESS_TIMEOUT_MS = 30000;
    static const QString SETTINGS_GROUP;
    static const QString SETTINGS_AUTO_CHECK;
    static const QString SETTINGS_CHECK_INTERVAL;
    static const QString SETTINGS_LAST_CHECK;
};

/**
 * @brief Background worker for wallet health checks
 */
class WalletHealthChecker : public QObject
{
    Q_OBJECT

public:
    explicit WalletHealthChecker(QObject* parent = nullptr);
    
    static VaultRecoveryAlerts::WalletStatus checkWalletHealth(const QString& walletPath);

signals:
    void healthCheckCompleted(VaultRecoveryAlerts::WalletStatus status, const QString& details);

public Q_SLOTS:
    void performHealthCheck(const QString& walletPath);

private:
    static bool isWalletFileValid(const QString& walletPath);
    static bool isWalletFileCorrupted(const QString& walletPath);
    static bool isWalletFileOutdated(const QString& walletPath);
    static QString getWalletFileVersion(const QString& walletPath);
};

#endif // SHAHCOIN_QT_VAULTRECOVERYALERTS_H
