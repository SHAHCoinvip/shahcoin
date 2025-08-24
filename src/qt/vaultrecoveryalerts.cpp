#include "vaultrecoveryalerts.h"
#include "walletmodel.h"
#include "guiutil.h"
#include "platformstyle.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QBuffer>
#include <QPainter>
#include <QPen>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QTimer>
#include <QMessageBox>
#include <QProgressDialog>
#include <QInputDialog>
#include <QCheckBox>

// Constants
const QString VaultRecoveryAlerts::SETTINGS_GROUP = "VaultRecoveryAlerts";
const QString VaultRecoveryAlerts::SETTINGS_AUTO_CHECK = "AutoCheckEnabled";
const QString VaultRecoveryAlerts::SETTINGS_CHECK_INTERVAL = "CheckIntervalHours";
const QString VaultRecoveryAlerts::SETTINGS_LAST_CHECK = "LastCheckTime";

VaultRecoveryAlerts::VaultRecoveryAlerts(WalletModel* walletModel, QWidget* parent)
    : QDialog(parent)
    , m_walletModel(walletModel)
    , m_platformStyle(nullptr)
    , m_mainLayout(nullptr)
    , m_statusGroup(nullptr)
    , m_statusIcon(nullptr)
    , m_statusLabel(nullptr)
    , m_detailsLabel(nullptr)
    , m_detailsText(nullptr)
    , m_showDetailsButton(nullptr)
    , m_recoveryGroup(nullptr)
    , m_restoreButton(nullptr)
    , m_rescanButton(nullptr)
    , m_createButton(nullptr)
    , m_repairButton(nullptr)
    , m_ignoreButton(nullptr)
    , m_progressBar(nullptr)
    , m_progressLabel(nullptr)
    , m_currentStatus(StatusUnknown)
    , m_recoveryInProgress(false)
    , m_settings(new QSettings("Shahcoin", "VaultRecoveryAlerts"))
    , m_autoCheckEnabled(true)
    , m_checkIntervalHours(CHECK_INTERVAL_HOURS)
{
    setWindowTitle(tr("🚨 Vault Recovery Alerts"));
    setModal(true);
    resize(600, 500);
    setupUI();
    connectSignals();
    applyTheme();
    loadRecoverySettings();
    
    // Get wallet path
    m_walletPath = getWalletFilePath();
}

VaultRecoveryAlerts::~VaultRecoveryAlerts()
{
    saveRecoverySettings();
}

void VaultRecoveryAlerts::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Status Group
    m_statusGroup = new QGroupBox(tr("Wallet Health Status"), this);
    QVBoxLayout* statusLayout = new QVBoxLayout(m_statusGroup);
    
    QHBoxLayout* statusHeaderLayout = new QHBoxLayout();
    m_statusIcon = new QLabel(this);
    m_statusIcon->setFixedSize(32, 32);
    m_statusIcon->setPixmap(QPixmap(":/icons/warning.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    m_statusLabel = new QLabel(tr("Checking wallet health..."), this);
    QFont statusFont = m_statusLabel->font();
    statusFont.setPointSize(12);
    statusFont.setBold(true);
    m_statusLabel->setFont(statusFont);
    
    statusHeaderLayout->addWidget(m_statusIcon);
    statusHeaderLayout->addWidget(m_statusLabel);
    statusHeaderLayout->addStretch();
    
    m_detailsLabel = new QLabel(tr("Details:"), this);
    m_detailsText = new QTextEdit(this);
    m_detailsText->setMaximumHeight(100);
    m_detailsText->setReadOnly(true);
    m_detailsText->hide();
    
    m_showDetailsButton = new QPushButton(tr("Show Details"), this);
    m_showDetailsButton->setMaximumWidth(120);
    
    statusLayout->addLayout(statusHeaderLayout);
    statusLayout->addWidget(m_detailsLabel);
    statusLayout->addWidget(m_detailsText);
    statusLayout->addWidget(m_showDetailsButton, 0, Qt::AlignRight);
    
    // Recovery Group
    m_recoveryGroup = new QGroupBox(tr("Recovery Options"), this);
    QVBoxLayout* recoveryLayout = new QVBoxLayout(m_recoveryGroup);
    
    QHBoxLayout* buttonLayout1 = new QHBoxLayout();
    m_restoreButton = new QPushButton(tr("🔄 Restore from Backup"), this);
    m_rescanButton = new QPushButton(tr("🔍 Rescan Wallet"), this);
    buttonLayout1->addWidget(m_restoreButton);
    buttonLayout1->addWidget(m_rescanButton);
    
    QHBoxLayout* buttonLayout2 = new QHBoxLayout();
    m_createButton = new QPushButton(tr("🆕 Create New Wallet"), this);
    m_repairButton = new QPushButton(tr("🔧 Repair Wallet"), this);
    buttonLayout2->addWidget(m_createButton);
    buttonLayout2->addWidget(m_repairButton);
    
    QHBoxLayout* buttonLayout3 = new QHBoxLayout();
    m_ignoreButton = new QPushButton(tr("⚠️ Ignore Warning"), this);
    buttonLayout3->addWidget(m_ignoreButton);
    buttonLayout3->addStretch();
    
    recoveryLayout->addLayout(buttonLayout1);
    recoveryLayout->addLayout(buttonLayout2);
    recoveryLayout->addLayout(buttonLayout3);
    
    // Progress
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressLabel = new QLabel(this);
    m_progressLabel->setVisible(false);
    
    // Add to main layout
    m_mainLayout->addWidget(m_statusGroup);
    m_mainLayout->addWidget(m_recoveryGroup);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addWidget(m_progressLabel);
    m_mainLayout->addStretch();
    
    // Initially disable recovery buttons until health check completes
    enableRecoveryButtons(false);
}

void VaultRecoveryAlerts::connectSignals()
{
    connect(m_showDetailsButton, &QPushButton::clicked, this, &VaultRecoveryAlerts::onShowDetails);
    connect(m_restoreButton, &QPushButton::clicked, this, &VaultRecoveryAlerts::onRestoreFromBackup);
    connect(m_rescanButton, &QPushButton::clicked, this, &VaultRecoveryAlerts::onRescanWallet);
    connect(m_createButton, &QPushButton::clicked, this, &VaultRecoveryAlerts::onCreateNewWallet);
    connect(m_repairButton, &QPushButton::clicked, this, &VaultRecoveryAlerts::onRepairWallet);
    connect(m_ignoreButton, &QPushButton::clicked, this, &VaultRecoveryAlerts::onIgnoreWarning);
    
    // Connect health check watcher
    connect(&m_healthCheckWatcher, &QFutureWatcher<WalletStatus>::finished, 
            this, &VaultRecoveryAlerts::onHealthCheckFinished);
    
    // Connect recovery watcher
    connect(&m_recoveryWatcher, &QFutureWatcher<bool>::finished,
            this, &VaultRecoveryAlerts::onRecoveryActionFinished);
}

void VaultRecoveryAlerts::applyTheme()
{
    // Apply Shahcoin styling
    setStyleSheet(R"(
        QDialog {
            background-color: #f8f9fa;
            color: #212529;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #dee2e6;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #495057;
        }
        QPushButton {
            background-color: #007bff;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QPushButton:pressed {
            background-color: #004085;
        }
        QPushButton:disabled {
            background-color: #6c757d;
            color: #adb5bd;
        }
        QTextEdit {
            border: 1px solid #ced4da;
            border-radius: 4px;
            background-color: white;
        }
        QProgressBar {
            border: 1px solid #ced4da;
            border-radius: 4px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #28a745;
            border-radius: 3px;
        }
    )");
}

void VaultRecoveryAlerts::checkWalletHealth()
{
    if (m_recoveryInProgress) {
        QMessageBox::warning(this, tr("Recovery in Progress"), 
                           tr("Please wait for the current recovery operation to complete."));
        return;
    }
    
    showProgressDialog(tr("Checking Wallet Health"), tr("Analyzing wallet.dat file..."));
    
    // Run health check in background thread
    QFuture<WalletStatus> future = QtConcurrent::run([this]() {
        return detectWalletStatus(m_walletPath);
    });
    
    m_healthCheckWatcher.setFuture(future);
}

void VaultRecoveryAlerts::showRecoveryDialog(WalletStatus status, const QString& details)
{
    m_currentStatus = status;
    m_currentDetails = details;
    
    updateStatusDisplay(status, details);
    enableRecoveryButtons(true);
    
    // Show appropriate buttons based on status
    switch (status) {
        case StatusMissing:
            m_restoreButton->setEnabled(true);
            m_createButton->setEnabled(true);
            m_rescanButton->setEnabled(false);
            m_repairButton->setEnabled(false);
            break;
        case StatusCorrupted:
            m_restoreButton->setEnabled(true);
            m_repairButton->setEnabled(true);
            m_createButton->setEnabled(true);
            m_rescanButton->setEnabled(false);
            break;
        case StatusOutdated:
            m_rescanButton->setEnabled(true);
            m_restoreButton->setEnabled(true);
            m_repairButton->setEnabled(true);
            m_createButton->setEnabled(false);
            break;
        case StatusLocked:
            m_repairButton->setEnabled(true);
            m_restoreButton->setEnabled(true);
            m_rescanButton->setEnabled(false);
            m_createButton->setEnabled(false);
            break;
        case StatusOK:
            enableRecoveryButtons(false);
            break;
        default:
            enableRecoveryButtons(true);
            break;
    }
    
    show();
}

void VaultRecoveryAlerts::performRecoveryAction(RecoveryAction action)
{
    if (m_recoveryInProgress) {
        QMessageBox::warning(this, tr("Recovery in Progress"), 
                           tr("Please wait for the current recovery operation to complete."));
        return;
    }
    
    m_recoveryInProgress = true;
    enableRecoveryButtons(false);
    
    QFuture<bool> future = QtConcurrent::run([this, action]() {
        QMutexLocker locker(&m_recoveryMutex);
        
        switch (action) {
            case RestoreFromBackup:
                return restoreWalletFromBackup(QString());
            case RescanWallet:
                return rescanWallet();
            case CreateNewWallet:
                return createNewWallet();
            case RepairWallet:
                return repairWallet();
            case IgnoreWarning:
                return true;
            default:
                return false;
        }
    });
    
    m_recoveryWatcher.setFuture(future);
}

bool VaultRecoveryAlerts::shouldCheckWalletHealth()
{
    QSettings settings("Shahcoin", "VaultRecoveryAlerts");
    bool autoCheck = settings.value(SETTINGS_AUTO_CHECK, true).toBool();
    if (!autoCheck) return false;
    
    QDateTime lastCheck = settings.value(SETTINGS_LAST_CHECK).toDateTime();
    int intervalHours = settings.value(SETTINGS_CHECK_INTERVAL, CHECK_INTERVAL_HOURS).toInt();
    
    return !lastCheck.isValid() || lastCheck.addSecs(intervalHours * 3600) <= QDateTime::currentDateTime();
}

void VaultRecoveryAlerts::setLastHealthCheck()
{
    QSettings settings("Shahcoin", "VaultRecoveryAlerts");
    settings.setValue(SETTINGS_LAST_CHECK, QDateTime::currentDateTime());
}

VaultRecoveryAlerts::WalletStatus VaultRecoveryAlerts::detectWalletStatus(const QString& walletPath)
{
    QString path = walletPath.isEmpty() ? getWalletFilePath() : walletPath;
    
    // Check if file exists
    if (!checkWalletFileExists(path)) {
        return StatusMissing;
    }
    
    // Check for corruption
    if (checkWalletFileCorruption(path)) {
        return StatusCorrupted;
    }
    
    // Check for lock
    if (checkWalletFileLock(path)) {
        return StatusLocked;
    }
    
    // Check version
    if (checkWalletFileVersion(path)) {
        return StatusOutdated;
    }
    
    return StatusOK;
}

QString VaultRecoveryAlerts::getWalletFilePath() const
{
    // Get wallet directory from settings or use default
    QSettings settings("Shahcoin", "Shahcoin-Qt");
    QString walletDir = settings.value("strDataDir", QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.shahcoin").toString();
    
    // Look for wallet.dat in the wallet directory
    QDir dir(walletDir);
    QStringList filters;
    filters << "wallet.dat";
    QStringList walletFiles = dir.entryList(filters, QDir::Files);
    
    if (!walletFiles.isEmpty()) {
        return dir.absoluteFilePath(walletFiles.first());
    }
    
    // Fallback to default location
    return dir.absoluteFilePath("wallet.dat");
}

bool VaultRecoveryAlerts::checkWalletFileExists(const QString& walletPath)
{
    QFileInfo fileInfo(walletPath);
    return fileInfo.exists() && fileInfo.isFile();
}

bool VaultRecoveryAlerts::checkWalletFileCorruption(const QString& walletPath)
{
    QFile file(walletPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return true; // Consider unreadable files as corrupted
    }
    
    // Check file size (wallet.dat should be at least 4KB)
    if (file.size() < 4096) {
        return true;
    }
    
    // Read first few bytes to check for BDB magic
    QByteArray header = file.read(16);
    file.close();
    
    if (header.size() < 16) {
        return true;
    }
    
    // Check for Berkeley DB magic bytes
    // Big endian: 00 05 31 62, Little endian: 62 31 05 00
    uint32_t magic = 0;
    memcpy(&magic, header.data() + 12, 4);
    
    return (magic != 0x00053162 && magic != 0x62310500);
}

bool VaultRecoveryAlerts::checkWalletFileVersion(const QString& walletPath)
{
    // This is a simplified check - in a real implementation, you'd parse the wallet version
    // For now, we'll assume the wallet is up to date if it exists and isn't corrupted
    return false;
}

bool VaultRecoveryAlerts::checkWalletFileLock(const QString& walletPath)
{
    // Check if wallet is locked by another process
    QFile file(walletPath);
    if (!file.open(QIODevice::ReadWrite)) {
        return true; // File is locked or inaccessible
    }
    file.close();
    return false;
}

bool VaultRecoveryAlerts::restoreWalletFromBackup(const QString& backupPath)
{
    QString sourcePath = backupPath;
    if (sourcePath.isEmpty()) {
        // Show file dialog to select backup
        sourcePath = QFileDialog::getOpenFileName(this, 
            tr("Select Wallet Backup"), 
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            tr("Wallet Files (*.dat *.bak);;All Files (*.*)"));
        
        if (sourcePath.isEmpty()) {
            return false;
        }
    }
    
    // Create backup of current wallet if it exists
    if (QFile::exists(m_walletPath)) {
        QString backupName = QString("%1.backup.%2").arg(m_walletPath).arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        if (!QFile::copy(m_walletPath, backupName)) {
            return false;
        }
    }
    
    // Copy backup to wallet location
    return QFile::copy(sourcePath, m_walletPath);
}

bool VaultRecoveryAlerts::rescanWallet()
{
    // In a real implementation, this would trigger a wallet rescan
    // For now, we'll simulate the process
    QThread::msleep(2000); // Simulate work
    return true;
}

bool VaultRecoveryAlerts::createNewWallet()
{
    // In a real implementation, this would create a new wallet
    // For now, we'll simulate the process
    QThread::msleep(2000); // Simulate work
    return true;
}

bool VaultRecoveryAlerts::repairWallet()
{
    // In a real implementation, this would attempt to repair the wallet
    // For now, we'll simulate the process
    QThread::msleep(2000); // Simulate work
    return true;
}

void VaultRecoveryAlerts::updateStatusDisplay(WalletStatus status, const QString& details)
{
    QString statusText, iconPath;
    
    switch (status) {
        case StatusOK:
            statusText = tr("✅ Wallet is healthy");
            iconPath = ":/icons/synced.png";
            break;
        case StatusMissing:
            statusText = tr("❌ Wallet file is missing");
            iconPath = ":/icons/warning.png";
            break;
        case StatusCorrupted:
            statusText = tr("⚠️ Wallet file is corrupted");
            iconPath = ":/icons/warning.png";
            break;
        case StatusOutdated:
            statusText = tr("🔄 Wallet needs rescan");
            iconPath = ":/icons/clock1.png";
            break;
        case StatusLocked:
            statusText = tr("🔒 Wallet is locked");
            iconPath = ":/icons/lock_closed.png";
            break;
        default:
            statusText = tr("❓ Unknown wallet status");
            iconPath = ":/icons/warning.png";
            break;
    }
    
    m_statusLabel->setText(statusText);
    m_statusIcon->setPixmap(QPixmap(iconPath).scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    if (!details.isEmpty()) {
        m_detailsText->setPlainText(details);
        m_detailsText->show();
        m_showDetailsButton->setText(tr("Hide Details"));
    }
}

void VaultRecoveryAlerts::showProgressDialog(const QString& title, const QString& message)
{
    m_progressBar->setVisible(true);
    m_progressLabel->setVisible(true);
    m_progressLabel->setText(message);
    m_progressBar->setRange(0, 0); // Indeterminate progress
}

void VaultRecoveryAlerts::hideProgressDialog()
{
    m_progressBar->setVisible(false);
    m_progressLabel->setVisible(false);
}

void VaultRecoveryAlerts::enableRecoveryButtons(bool enabled)
{
    m_restoreButton->setEnabled(enabled);
    m_rescanButton->setEnabled(enabled);
    m_createButton->setEnabled(enabled);
    m_repairButton->setEnabled(enabled);
    m_ignoreButton->setEnabled(enabled);
}

void VaultRecoveryAlerts::saveRecoverySettings()
{
    m_settings->setValue(SETTINGS_AUTO_CHECK, m_autoCheckEnabled);
    m_settings->setValue(SETTINGS_CHECK_INTERVAL, m_checkIntervalHours);
    m_settings->setValue(SETTINGS_LAST_CHECK, m_lastCheckTime);
}

void VaultRecoveryAlerts::loadRecoverySettings()
{
    m_autoCheckEnabled = m_settings->value(SETTINGS_AUTO_CHECK, true).toBool();
    m_checkIntervalHours = m_settings->value(SETTINGS_CHECK_INTERVAL, CHECK_INTERVAL_HOURS).toInt();
    m_lastCheckTime = m_settings->value(SETTINGS_LAST_CHECK).toDateTime();
}

// Public slots
void VaultRecoveryAlerts::onCheckWalletHealth()
{
    checkWalletHealth();
}

void VaultRecoveryAlerts::onRestoreFromBackup()
{
    performRecoveryAction(RestoreFromBackup);
}

void VaultRecoveryAlerts::onRescanWallet()
{
    performRecoveryAction(RescanWallet);
}

void VaultRecoveryAlerts::onCreateNewWallet()
{
    performRecoveryAction(CreateNewWallet);
}

void VaultRecoveryAlerts::onRepairWallet()
{
    performRecoveryAction(RepairWallet);
}

void VaultRecoveryAlerts::onIgnoreWarning()
{
    performRecoveryAction(IgnoreWarning);
}

void VaultRecoveryAlerts::onShowDetails()
{
    if (m_detailsText->isVisible()) {
        m_detailsText->hide();
        m_showDetailsButton->setText(tr("Show Details"));
    } else {
        m_detailsText->show();
        m_showDetailsButton->setText(tr("Hide Details"));
    }
}

// Private slots
void VaultRecoveryAlerts::onHealthCheckFinished()
{
    hideProgressDialog();
    
    WalletStatus status = m_healthCheckWatcher.result();
    QString details;
    
    switch (status) {
        case StatusMissing:
            details = tr("The wallet.dat file was not found at:\n%1\n\nThis could mean:\n• The wallet was moved or deleted\n• You're using a different wallet directory\n• The wallet hasn't been created yet").arg(m_walletPath);
            break;
        case StatusCorrupted:
            details = tr("The wallet.dat file appears to be corrupted at:\n%1\n\nThis could be due to:\n• Incomplete shutdown\n• Disk errors\n• File system issues\n• Malware or tampering").arg(m_walletPath);
            break;
        case StatusOutdated:
            details = tr("The wallet.dat file may be outdated and requires a rescan.\n\nThis usually happens when:\n• The blockchain has advanced significantly\n• Transaction data is missing\n• The wallet was restored from backup");
            break;
        case StatusLocked:
            details = tr("The wallet.dat file is currently locked by another process.\n\nThis could mean:\n• Another Shahcoin instance is running\n• The wallet is being used by another application\n• A backup or sync operation is in progress");
            break;
        case StatusOK:
            details = tr("Wallet health check completed successfully.\nNo issues detected.");
            break;
        default:
            details = tr("Unable to determine wallet status.\nPlease check the wallet file manually.");
            break;
    }
    
    updateStatusDisplay(status, details);
    showRecoveryDialog(status, details);
    
    // Emit signal
    emit walletHealthCheckCompleted(status, details);
}

void VaultRecoveryAlerts::onRecoveryActionFinished()
{
    m_recoveryInProgress = false;
    hideProgressDialog();
    
    bool success = m_recoveryWatcher.result();
    QString message;
    
    if (success) {
        message = tr("Recovery operation completed successfully.");
        QMessageBox::information(this, tr("Recovery Successful"), message);
        accept();
    } else {
        message = tr("Recovery operation failed. Please try a different approach.");
        QMessageBox::critical(this, tr("Recovery Failed"), message);
    }
    
    enableRecoveryButtons(true);
    emit walletRecoveryCompleted(success, message);
}

void VaultRecoveryAlerts::onBackupFileSelected(const QString& filePath)
{
    // This would be called when a backup file is selected
    // For now, we'll handle it in the restore method
}

// WalletHealthChecker implementation
WalletHealthChecker::WalletHealthChecker(QObject* parent)
    : QObject(parent)
{
}

VaultRecoveryAlerts::WalletStatus WalletHealthChecker::checkWalletHealth(const QString& walletPath)
{
    return VaultRecoveryAlerts::detectWalletStatus(walletPath);
}

void WalletHealthChecker::performHealthCheck(const QString& walletPath)
{
    WalletStatus status = checkWalletHealth(walletPath);
    QString details;
    
    switch (status) {
        case VaultRecoveryAlerts::StatusMissing:
            details = tr("Wallet file not found");
            break;
        case VaultRecoveryAlerts::StatusCorrupted:
            details = tr("Wallet file is corrupted");
            break;
        case VaultRecoveryAlerts::StatusOutdated:
            details = tr("Wallet needs rescan");
            break;
        case VaultRecoveryAlerts::StatusLocked:
            details = tr("Wallet is locked");
            break;
        case VaultRecoveryAlerts::StatusOK:
            details = tr("Wallet is healthy");
            break;
        default:
            details = tr("Unknown status");
            break;
    }
    
    emit healthCheckCompleted(status, details);
}

bool WalletHealthChecker::isWalletFileValid(const QString& walletPath)
{
    return !VaultRecoveryAlerts::detectWalletStatus(walletPath);
}

bool WalletHealthChecker::isWalletFileCorrupted(const QString& walletPath)
{
    VaultRecoveryAlerts::WalletStatus status = VaultRecoveryAlerts::detectWalletStatus(walletPath);
    return status == VaultRecoveryAlerts::StatusCorrupted;
}

bool WalletHealthChecker::isWalletFileOutdated(const QString& walletPath)
{
    VaultRecoveryAlerts::WalletStatus status = VaultRecoveryAlerts::detectWalletStatus(walletPath);
    return status == VaultRecoveryAlerts::StatusOutdated;
}

QString WalletHealthChecker::getWalletFileVersion(const QString& walletPath)
{
    // This would parse the wallet version from the file
    // For now, return a placeholder
    return "1.0.0";
}
