#include "vaultmanager.h"
#include "walletmodel.h"
#include "platformstyle.h"
#include "guiutil.h"
#include "shahcoinunits.h"
#include "interfaces/wallet.h"
#include "util/translation.h"

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QScrollBar>
#include <QStyle>
#include <QScreen>
#include <QPainter>
#include <QFontMetrics>
#include <QStandardPaths>
#include <QJsonArray>
#include <QInputDialog>
#include <QToolTip>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QFormLayout>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QBuffer>
#include <QPainter>
#include <QPen>
#include <QMessageBox>
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

VaultManager::VaultManager(WalletModel* walletModel, QWidget* parent)
    : QDialog(parent)
    , m_walletModel(walletModel)
    , m_platformStyle(nullptr)
    , m_reminderTimer(new QTimer(this))
    , m_passwordValid(false)
    , m_showBackupReminder(true)
    , m_reminderInterval(REMINDER_INTERVAL_DAYS)
{
    setWindowTitle(tr("🧳 Vault Manager"));
    setModal(true);
    resize(700, 600);
    
    setupUI();
    connectSignals();
    applyTheme();
    
    // Setup reminder timer
    connect(m_reminderTimer, &QTimer::timeout, this, &VaultManager::onBackupReminder);
    m_reminderTimer->start(60000); // Check every minute
    
    // Load settings
    QSettings settings;
    m_showBackupReminder = settings.value("VaultManager/ShowBackupReminder", true).toBool();
    m_lastBackupTime = settings.value("VaultManager/LastBackupTime").toDateTime();
    
    // Check if backup reminder should be shown
    if (m_showBackupReminder && !m_lastBackupTime.isValid()) {
        QTimer::singleShot(1000, this, &VaultManager::onBackupReminder);
    }
}

VaultManager::~VaultManager()
{
    // Save settings
    QSettings settings;
    settings.setValue("VaultManager/ShowBackupReminder", m_showBackupReminder);
    settings.setValue("VaultManager/LastBackupTime", m_lastBackupTime);
}

void VaultManager::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_mainLayout->addWidget(m_tabWidget);
    
    setupExportTab();
    setupImportTab();
    setupQRCodeTab();
    setupBackupReminder();
}

void VaultManager::setupExportTab()
{
    m_exportTab = new QWidget();
    m_exportLayout = new QVBoxLayout(m_exportTab);
    m_exportLayout->setContentsMargins(0, 0, 0, 0);
    m_exportLayout->setSpacing(20);
    
    // Export Group
    m_exportGroup = new QGroupBox(tr("🔒 Export Wallet Vault"));
    m_exportForm = new QFormLayout(m_exportGroup);
    m_exportForm->setSpacing(15);
    
    // File path
    QHBoxLayout* pathLayout = new QHBoxLayout();
    m_exportPathEdit = new QLineEdit();
    m_exportPathEdit->setPlaceholderText(tr("Choose vault file location..."));
    m_exportPathEdit->setMinimumWidth(400);
    m_exportBrowseButton = new QPushButton(tr("Browse"));
    pathLayout->addWidget(m_exportPathEdit, 1);
    pathLayout->addWidget(m_exportBrowseButton);
    m_exportForm->addRow(tr("Vault File:"), pathLayout);
    
    // Password
    QHBoxLayout* passLayout = new QHBoxLayout();
    m_exportPasswordEdit = new QLineEdit();
    m_exportPasswordEdit->setEchoMode(QLineEdit::Password);
    m_exportPasswordEdit->setPlaceholderText(tr("Enter strong password for encryption"));
    m_showPasswordCheck = new QCheckBox(tr("Show"));
    passLayout->addWidget(m_exportPasswordEdit, 1);
    passLayout->addWidget(m_showPasswordCheck);
    m_exportForm->addRow(tr("Password:"), passLayout);
    
    // Options
    m_includeQRCheck = new QCheckBox(tr("Include QR code for mobile viewing"));
    m_includeQRCheck->setChecked(true);
    m_includeNFTsCheck = new QCheckBox(tr("Include NFT data"));
    m_includeNFTsCheck->setChecked(true);
    m_includeStakingCheck = new QCheckBox(tr("Include staking configuration"));
    m_includeStakingCheck->setChecked(true);
    
    QVBoxLayout* optionsLayout = new QVBoxLayout();
    optionsLayout->addWidget(m_includeQRCheck);
    optionsLayout->addWidget(m_includeNFTsCheck);
    optionsLayout->addWidget(m_includeStakingCheck);
    m_exportForm->addRow(tr("Include:"), optionsLayout);
    
    // Export button and progress
    m_exportButton = new QPushButton(tr("🔒 Export Vault"));
    m_exportButton->setMinimumHeight(40);
    m_exportProgress = new QProgressBar();
    m_exportProgress->setVisible(false);
    m_exportStatusLabel = new QLabel();
    m_exportStatusLabel->setWordWrap(true);
    
    m_exportLayout->addWidget(m_exportGroup);
    m_exportLayout->addWidget(m_exportButton);
    m_exportLayout->addWidget(m_exportProgress);
    m_exportLayout->addWidget(m_exportStatusLabel);
    m_exportLayout->addStretch();
    
    m_tabWidget->addTab(m_exportTab, tr("📤 Export"));
}

void VaultManager::setupImportTab()
{
    m_importTab = new QWidget();
    m_importLayout = new QVBoxLayout(m_importTab);
    m_importLayout->setContentsMargins(0, 0, 0, 0);
    m_importLayout->setSpacing(20);
    
    // Import Group
    m_importGroup = new QGroupBox(tr("📥 Import Wallet Vault"));
    m_importForm = new QFormLayout(m_importGroup);
    m_importForm->setSpacing(15);
    
    // File path
    QHBoxLayout* pathLayout = new QHBoxLayout();
    m_importPathEdit = new QLineEdit();
    m_importPathEdit->setPlaceholderText(tr("Select vault file to import..."));
    m_importPathEdit->setMinimumWidth(400);
    m_importBrowseButton = new QPushButton(tr("Browse"));
    pathLayout->addWidget(m_importPathEdit, 1);
    pathLayout->addWidget(m_importBrowseButton);
    m_importForm->addRow(tr("Vault File:"), pathLayout);
    
    // Password
    m_importPasswordEdit = new QLineEdit();
    m_importPasswordEdit->setEchoMode(QLineEdit::Password);
    m_importPasswordEdit->setPlaceholderText(tr("Enter vault password"));
    m_importForm->addRow(tr("Password:"), m_importPasswordEdit);
    
    // Import button and progress
    m_importButton = new QPushButton(tr("📥 Import Vault"));
    m_importButton->setMinimumHeight(40);
    m_importProgress = new QProgressBar();
    m_importProgress->setVisible(false);
    m_importStatusLabel = new QLabel();
    m_importStatusLabel->setWordWrap(true);
    
    // Preview
    m_importPreview = new QTextEdit();
    m_importPreview->setMaximumHeight(150);
    m_importPreview->setPlaceholderText(tr("Vault contents will be previewed here..."));
    m_importPreview->setReadOnly(true);
    
    m_importLayout->addWidget(m_importGroup);
    m_importLayout->addWidget(m_importButton);
    m_importLayout->addWidget(m_importProgress);
    m_importLayout->addWidget(m_importStatusLabel);
    m_importLayout->addWidget(new QLabel(tr("Preview:")));
    m_importLayout->addWidget(m_importPreview);
    
    m_tabWidget->addTab(m_importTab, tr("📥 Import"));
}

void VaultManager::setupQRCodeTab()
{
    m_qrTab = new QWidget();
    m_qrLayout = new QVBoxLayout(m_qrTab);
    m_qrLayout->setContentsMargins(0, 0, 0, 0);
    m_qrLayout->setSpacing(20);
    
    // QR Group
    m_qrGroup = new QGroupBox(tr("📱 QR Code Export"));
    QVBoxLayout* qrGroupLayout = new QVBoxLayout(m_qrGroup);
    
    QLabel* qrDescription = new QLabel(tr("Generate a QR code containing your vault data for easy mobile viewing."));
    qrDescription->setWordWrap(true);
    qrGroupLayout->addWidget(qrDescription);
    
    m_qrLabel = new QLabel();
    m_qrLabel->setMinimumSize(300, 300);
    m_qrLabel->setAlignment(Qt::AlignCenter);
    m_qrLabel->setStyleSheet("QLabel { background:white; border:1px solid #ddd; }");
    qrGroupLayout->addWidget(m_qrLabel, 0, Qt::AlignCenter);
    
    QHBoxLayout* qrButtonsLayout = new QHBoxLayout();
    m_generateQRButton = new QPushButton(tr("Generate QR"));
    m_saveQRButton = new QPushButton(tr("Save QR"));
    m_saveQRButton->setEnabled(false);
    qrButtonsLayout->addStretch();
    qrButtonsLayout->addWidget(m_generateQRButton);
    qrButtonsLayout->addWidget(m_saveQRButton);
    qrGroupLayout->addLayout(qrButtonsLayout);
    
    m_qrStatusLabel = new QLabel();
    m_qrStatusLabel->setAlignment(Qt::AlignCenter);
    qrGroupLayout->addWidget(m_qrStatusLabel);
    
    m_qrLayout->addWidget(m_qrGroup);
    m_qrLayout->addStretch();
    
    m_tabWidget->addTab(m_qrTab, tr("📱 QR Code"));
}

void VaultManager::setupBackupReminder()
{
    m_reminderDialog = new QDialog(this);
    m_reminderDialog->setWindowTitle(tr("🔒 Backup Reminder"));
    m_reminderDialog->setModal(true);
    m_reminderDialog->setFixedSize(400, 250);
    
    m_reminderLayout = new QVBoxLayout(m_reminderDialog);
    m_reminderLayout->setContentsMargins(20, 20, 20, 20);
    m_reminderLayout->setSpacing(15);
    
    m_reminderTitle = new QLabel(tr("🔒 Secure Your Wallet"));
    m_reminderTitle->setStyleSheet("font-size: 16px; font-weight: bold;");
    m_reminderLayout->addWidget(m_reminderTitle);
    
    m_reminderMessage = new QLabel(tr("It's recommended to create a secure backup of your wallet. This will encrypt all your wallet data including addresses, settings, and staking configuration."));
    m_reminderMessage->setWordWrap(true);
    m_reminderLayout->addWidget(m_reminderMessage);
    
    m_reminderButtons = new QHBoxLayout();
    m_reminderNowButton = new QPushButton(tr("Backup Now"));
    m_reminderLaterButton = new QPushButton(tr("Later"));
    m_reminderDontShowCheck = new QCheckBox(tr("Don't show this again"));
    
    m_reminderButtons->addWidget(m_reminderDontShowCheck);
    m_reminderButtons->addStretch();
    m_reminderButtons->addWidget(m_reminderLaterButton);
    m_reminderButtons->addWidget(m_reminderNowButton);
    
    m_reminderLayout->addLayout(m_reminderButtons);
}

void VaultManager::connectSignals()
{
    // Export tab
    connect(m_exportBrowseButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getSaveFileName(this, tr("Save Vault File"),
            generateVaultFileName(), VAULT_FILE_FILTER);
        if (!filePath.isEmpty()) {
            m_exportPathEdit->setText(filePath);
            m_lastExportPath = filePath;
        }
    });
    
    connect(m_exportPasswordEdit, &QLineEdit::textChanged, this, &VaultManager::onPasswordChanged);
    connect(m_showPasswordCheck, &QCheckBox::toggled, this, &VaultManager::onShowPasswordToggled);
    connect(m_exportButton, &QPushButton::clicked, this, &VaultManager::onExportVaultClicked);
    
    // Import tab
    connect(m_importBrowseButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, tr("Open Vault File"),
            m_lastImportPath, VAULT_FILE_FILTER);
        if (!filePath.isEmpty()) {
            m_importPathEdit->setText(filePath);
            m_lastImportPath = filePath;
            // Try to preview the vault
            QByteArray data = readVaultFile(filePath);
            if (!data.isEmpty()) {
                m_importPreview->setPlainText(tr("Vault file loaded. Enter password to preview contents."));
            }
        }
    });
    
    connect(m_importPasswordEdit, &QLineEdit::textChanged, this, &VaultManager::onPasswordChanged);
    connect(m_importButton, &QPushButton::clicked, this, &VaultManager::onImportVaultClicked);
    
    // QR tab
    connect(m_generateQRButton, &QPushButton::clicked, this, &VaultManager::onGenerateQRClicked);
    connect(m_saveQRButton, &QPushButton::clicked, this, &VaultManager::onSaveQRClicked);
    
    // Reminder dialog
    connect(m_reminderNowButton, &QPushButton::clicked, this, [this]() {
        m_reminderDialog->accept();
        m_tabWidget->setCurrentIndex(0); // Switch to export tab
    });
    
    connect(m_reminderLaterButton, &QPushButton::clicked, this, [this]() {
        m_reminderDialog->reject();
    });
    
    connect(m_reminderDontShowCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_showBackupReminder = !checked;
    });
}

void VaultManager::applyTheme()
{
    // Apply consistent styling
    QString style = R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:disabled {
            background-color: #cccccc;
        }
        QProgressBar {
            border: 2px solid grey;
            border-radius: 5px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #4CAF50;
            border-radius: 3px;
        }
    )";
    
    setStyleSheet(style);
}

void VaultManager::onExportVaultClicked()
{
    QString filePath = m_exportPathEdit->text().trimmed();
    QString password = m_exportPasswordEdit->text();
    
    if (filePath.isEmpty()) {
        showError(tr("Please select a file location for the vault."));
        return;
    }
    
    if (password.isEmpty()) {
        showError(tr("Please enter a password for encryption."));
        return;
    }
    
    if (password.length() < 8) {
        showWarning(tr("Password should be at least 8 characters long for security."));
    }
    
    // Show progress
    m_exportProgress->setVisible(true);
    m_exportProgress->setRange(0, 100);
    m_exportProgress->setValue(0);
    m_exportStatusLabel->setText(tr("Preparing vault data..."));
    m_exportButton->setEnabled(false);
    
    // Perform export in background
    QTimer::singleShot(100, this, [this, filePath, password]() {
        bool success = exportVault(filePath, password);
        
        m_exportProgress->setVisible(false);
        m_exportButton->setEnabled(true);
        
        if (success) {
            showSuccess(tr("Vault exported successfully to %1").arg(filePath));
            m_lastBackupTime = QDateTime::currentDateTime();
            m_exportStatusLabel->setText(tr("✅ Export completed successfully"));
        } else {
            showError(tr("Failed to export vault. Please check the file path and try again."));
            m_exportStatusLabel->setText(tr("❌ Export failed"));
        }
    });
}

void VaultManager::onImportVaultClicked()
{
    QString filePath = m_importPathEdit->text().trimmed();
    QString password = m_importPasswordEdit->text();
    
    if (filePath.isEmpty()) {
        showError(tr("Please select a vault file to import."));
        return;
    }
    
    if (password.isEmpty()) {
        showError(tr("Please enter the vault password."));
        return;
    }
    
    // Show progress
    m_importProgress->setVisible(true);
    m_importProgress->setRange(0, 100);
    m_importProgress->setValue(0);
    m_importStatusLabel->setText(tr("Reading vault file..."));
    m_importButton->setEnabled(false);
    
    // Perform import in background
    QTimer::singleShot(100, this, [this, filePath, password]() {
        bool success = importVault(filePath, password);
        
        m_importProgress->setVisible(false);
        m_importButton->setEnabled(true);
        
        if (success) {
            showSuccess(tr("Vault imported successfully!"));
            m_importStatusLabel->setText(tr("✅ Import completed successfully"));
        } else {
            showError(tr("Failed to import vault. Please check the password and try again."));
            m_importStatusLabel->setText(tr("❌ Import failed"));
        }
    });
}

void VaultManager::onGenerateQRClicked()
{
    if (m_currentVaultData.isEmpty()) {
        // Generate vault data first
        m_currentVaultData = generateVaultData();
    }
    
    if (m_currentVaultData.isEmpty()) {
        showError(tr("No vault data available. Please export a vault first."));
        return;
    }
    
    m_qrStatusLabel->setText(tr("Generating QR code..."));
    m_generateQRButton->setEnabled(false);
    
    // Generate QR in background
    QTimer::singleShot(100, this, [this]() {
        m_currentQRPixmap = generateVaultQR(m_currentVaultData);
        
        if (!m_currentQRPixmap.isNull()) {
            m_qrLabel->setPixmap(m_currentQRPixmap);
            m_saveQRButton->setEnabled(true);
            m_qrStatusLabel->setText(tr("✅ QR code generated successfully"));
        } else {
            showError(tr("Failed to generate QR code."));
            m_qrStatusLabel->setText(tr("❌ QR generation failed"));
        }
        
        m_generateQRButton->setEnabled(true);
    });
}

void VaultManager::onSaveQRClicked()
{
    if (m_currentQRPixmap.isNull()) {
        showError(tr("No QR code to save. Please generate one first."));
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save QR Code"),
        "shahcoin_vault_qr.png", tr("PNG Files (*.png);;All Files (*)"));
    
    if (!filePath.isEmpty()) {
        saveQRCode(m_currentQRPixmap, filePath);
        showSuccess(tr("QR code saved to %1").arg(filePath));
    }
}

void VaultManager::onPasswordChanged()
{
    QString password = m_exportPasswordEdit->text();
    m_passwordValid = password.length() >= 8;
    updateButtonStates();
}

void VaultManager::onShowPasswordToggled(bool checked)
{
    QLineEdit::EchoMode mode = checked ? QLineEdit::Normal : QLineEdit::Password;
    m_exportPasswordEdit->setEchoMode(mode);
    m_importPasswordEdit->setEchoMode(mode);
}

void VaultManager::onBackupReminder()
{
    if (!m_showBackupReminder) return;
    
    // Check if backup is needed
    if (!m_lastBackupTime.isValid() || 
        m_lastBackupTime.daysTo(QDateTime::currentDateTime()) >= m_reminderInterval) {
        
        m_reminderDialog->show();
        m_reminderDialog->raise();
        m_reminderDialog->activateWindow();
    }
}

void VaultManager::onValidateBackup()
{
    // This could be called to validate existing backups
    showInfo(tr("Backup validation feature coming soon."));
}

bool VaultManager::exportVault(const QString& filePath, const QString& password)
{
    try {
        // Generate vault data
        QString vaultData = generateVaultData();
        if (vaultData.isEmpty()) {
            return false;
        }
        
        // Generate salt and derive key
        m_currentSalt = generateSalt();
        QString key = deriveKeyFromPassword(password, m_currentSalt);
        
        // Encrypt data
        QByteArray data = vaultData.toUtf8();
        QByteArray encryptedData = encryptData(data, key);
        
        // Create vault file structure
        QJsonObject vaultFile;
        vaultFile["version"] = VAULT_VERSION;
        vaultFile["created"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        vaultFile["salt"] = QString::fromUtf8(m_currentSalt.toBase64());
        vaultFile["data"] = QString::fromUtf8(encryptedData.toBase64());
        vaultFile["checksum"] = generateBackupHash();
        
        QJsonDocument doc(vaultFile);
        QByteArray fileData = doc.toJson(QJsonDocument::Compact);
        
        // Write to file
        return writeVaultFile(filePath, fileData);
        
    } catch (const std::exception& e) {
        qWarning() << "Vault export error:" << e.what();
        return false;
    }
}

bool VaultManager::importVault(const QString& filePath, const QString& password)
{
    try {
        // Read vault file
        QByteArray fileData = readVaultFile(filePath);
        if (fileData.isEmpty()) {
            return false;
        }
        
        // Parse JSON
        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        if (!doc.isObject()) {
            return false;
        }
        
        QJsonObject vaultFile = doc.object();
        
        // Validate version
        if (vaultFile["version"].toInt() != VAULT_VERSION) {
            showError(tr("Unsupported vault version. Please use a newer version of Shahcoin."));
            return false;
        }
        
        // Extract data
        QByteArray salt = QByteArray::fromBase64(vaultFile["salt"].toString().toUtf8());
        QByteArray encryptedData = QByteArray::fromBase64(vaultFile["data"].toString().toUtf8());
        
        // Derive key and decrypt
        QString key = deriveKeyFromPassword(password, salt);
        QByteArray decryptedData = decryptData(encryptedData, key);
        
        if (decryptedData.isEmpty()) {
            return false;
        }
        
        QString vaultData = QString::fromUtf8(decryptedData);
        
        // Validate and restore
        if (!validateVaultData(vaultData)) {
            showError(tr("Invalid vault data. The file may be corrupted."));
            return false;
        }
        
        // Parse and restore data
        QJsonDocument dataDoc = QJsonDocument::fromJson(vaultData.toUtf8());
        if (!dataDoc.isObject()) {
            return false;
        }
        
        QJsonObject data = dataDoc.object();
        
        // Restore wallet data
        bool success = true;
        success &= restoreWalletData(data["wallet"].toObject());
        success &= restoreAddressData(data["addresses"].toObject());
        success &= restoreStakingData(data["staking"].toObject());
        success &= restoreSettingsData(data["settings"].toObject());
        
        if (data.contains("nfts")) {
            success &= restoreNFTData(data["nfts"].toObject());
        }
        
        return success;
        
    } catch (const std::exception& e) {
        qWarning() << "Vault import error:" << e.what();
        return false;
    }
}

QString VaultManager::generateVaultData() const
{
    QJsonObject vaultData;
    
    // Collect all wallet data
    vaultData["wallet"] = collectWalletData();
    vaultData["addresses"] = collectAddressData();
    vaultData["staking"] = collectStakingData();
    vaultData["settings"] = collectSettingsData();
    
    if (m_includeNFTsCheck && m_includeNFTsCheck->isChecked()) {
        vaultData["nfts"] = collectNFTData();
    }
    
    // Add metadata
    vaultData["exported"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    vaultData["version"] = VAULT_VERSION;
    vaultData["checksum"] = generateBackupHash();
    
    QJsonDocument doc(vaultData);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

bool VaultManager::validateVaultData(const QString& data) const
{
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) {
        return false;
    }
    
    QJsonObject obj = doc.object();
    
    // Check required fields
    if (!obj.contains("wallet") || !obj.contains("addresses") || 
        !obj.contains("staking") || !obj.contains("settings")) {
        return false;
    }
    
    // Validate checksum
    QString storedChecksum = obj["checksum"].toString();
    QString calculatedChecksum = generateBackupHash();
    
    return storedChecksum == calculatedChecksum;
}

QString VaultManager::deriveKeyFromPassword(const QString& password, const QByteArray& salt) const
{
    // Use PBKDF2-like key derivation (simplified for MVP)
    QByteArray keyData = password.toUtf8() + salt;
    QByteArray hash = QCryptographicHash::hash(keyData, QCryptographicHash::Sha256);
    
    // Multiple iterations for security
    for (int i = 0; i < 10000; ++i) {
        hash = QCryptographicHash::hash(hash + keyData, QCryptographicHash::Sha256);
    }
    
    return QString::fromUtf8(hash.toHex());
}

QByteArray VaultManager::encryptData(const QByteArray& data, const QString& key) const
{
    // MVP: Simple XOR encryption (replace with AES in production)
    QByteArray keyBytes = QByteArray::fromHex(key.toUtf8());
    QByteArray encrypted = data;
    
    for (int i = 0; i < encrypted.size(); ++i) {
        encrypted[i] = encrypted[i] ^ keyBytes.at(i % keyBytes.size());
    }
    
    return encrypted;
}

QByteArray VaultManager::decryptData(const QByteArray& encryptedData, const QString& key) const
{
    // MVP: Simple XOR decryption (replace with AES in production)
    return encryptData(encryptedData, key); // XOR is symmetric
}

QPixmap VaultManager::generateVaultQR(const QString& vaultData) const
{
    // MVP: Generate placeholder QR pattern
    // In production, use a proper QR library like qrencode
    
    QPixmap pixmap(300, 300);
    pixmap.fill(Qt::white);
    
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 2));
    
    // Generate a pattern based on the data hash
    QByteArray hash = QCryptographicHash::hash(vaultData.toUtf8(), QCryptographicHash::Sha256);
    
    for (int i = 0; i < 25; ++i) {
        for (int j = 0; j < 25; ++j) {
            int index = (i * 7 + j * 11) % hash.size();
            if (hash.at(index) % 3 == 0) {
                painter.fillRect(i * 12, j * 12, 10, 10, Qt::black);
            }
        }
    }
    
    painter.end();
    return pixmap;
}

void VaultManager::saveQRCode(const QPixmap& qrPixmap, const QString& filePath) const
{
    qrPixmap.save(filePath, "PNG");
}

QJsonObject VaultManager::collectWalletData() const
{
    QJsonObject walletData;
    
    if (m_walletModel) {
        walletData["balance"] = QString::number(m_walletModel->getBalance());
        walletData["unconfirmed_balance"] = QString::number(m_walletModel->getUnconfirmedBalance());
        walletData["immature_balance"] = QString::number(m_walletModel->getImmatureBalance());
        walletData["stake"] = QString::number(m_walletModel->getStake());
        walletData["encrypted"] = m_walletModel->isEncrypted();
        walletData["hd_enabled"] = m_walletModel->isHDEnabled();
    }
    
    return walletData;
}

QJsonObject VaultManager::collectAddressData() const
{
    QJsonObject addressData;
    QJsonArray addresses;
    
    if (m_walletModel && m_walletModel->getAddressTableModel()) {
        QStringList addressList = m_walletModel->getAddressTableModel()->getAddressList();
        
        for (const QString& address : addressList) {
            QJsonObject addrObj;
            addrObj["address"] = address;
            addrObj["label"] = m_walletModel->getAddressTableModel()->labelForAddress(address);
            addresses.append(addrObj);
        }
    }
    
    addressData["addresses"] = addresses;
    return addressData;
}

QJsonObject VaultManager::collectStakingData() const
{
    QJsonObject stakingData;
    
    // Collect cold staking data if available
    // This would integrate with the cold staking panel
    stakingData["cold_staking_enabled"] = false; // Placeholder
    stakingData["stake_amount"] = "0"; // Placeholder
    
    return stakingData;
}

QJsonObject VaultManager::collectSettingsData() const
{
    QJsonObject settingsData;
    
    // Collect application settings
    QSettings settings;
    settingsData["theme"] = settings.value("Display/Theme", "Light").toString();
    settingsData["language"] = settings.value("Display/Language", "en").toString();
    settingsData["fiat_enabled"] = settings.value("Display/FiatEnabled", false).toBool();
    
    return settingsData;
}

QJsonObject VaultManager::collectNFTData() const
{
    QJsonObject nftData;
    QJsonArray nfts;
    
    // Collect NFT data if available
    // This would integrate with the NFT gallery
    nftData["nfts"] = nfts;
    
    return nftData;
}

bool VaultManager::restoreWalletData(const QJsonObject& data)
{
    // Restore wallet data
    // This would integrate with the wallet model
    return true; // Placeholder
}

bool VaultManager::restoreAddressData(const QJsonObject& data)
{
    // Restore address data
    // This would integrate with the address table model
    return true; // Placeholder
}

bool VaultManager::restoreStakingData(const QJsonObject& data)
{
    // Restore staking data
    // This would integrate with the cold staking panel
    return true; // Placeholder
}

bool VaultManager::restoreSettingsData(const QJsonObject& data)
{
    // Restore application settings
    QSettings settings;
    
    if (data.contains("theme")) {
        settings.setValue("Display/Theme", data["theme"].toString());
    }
    
    if (data.contains("language")) {
        settings.setValue("Display/Language", data["language"].toString());
    }
    
    if (data.contains("fiat_enabled")) {
        settings.setValue("Display/FiatEnabled", data["fiat_enabled"].toBool());
    }
    
    return true;
}

bool VaultManager::restoreNFTData(const QJsonObject& data)
{
    // Restore NFT data
    // This would integrate with the NFT gallery
    return true; // Placeholder
}

bool VaultManager::writeVaultFile(const QString& filePath, const QByteArray& encryptedData)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    qint64 bytesWritten = file.write(encryptedData);
    file.close();
    
    return bytesWritten == encryptedData.size();
}

QByteArray VaultManager::readVaultFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    return data;
}

QString VaultManager::generateVaultFileName() const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    return QString("shahcoin_vault_%1.shahvault").arg(timestamp);
}

QByteArray VaultManager::generateSalt() const
{
    QByteArray salt;
    salt.resize(SALT_SIZE);
    
    // Generate random salt
    for (int i = 0; i < SALT_SIZE; ++i) {
        salt[i] = QRandomGenerator::global()->bounded(256);
    }
    
    return salt;
}

bool VaultManager::verifyPassword(const QString& password) const
{
    return password.length() >= 8;
}

QString VaultManager::generateBackupHash() const
{
    QString data = QString::number(QDateTime::currentMSecsSinceEpoch());
    QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromUtf8(hash.toHex());
}

void VaultManager::validateExportInputs()
{
    bool valid = !m_exportPathEdit->text().trimmed().isEmpty() && 
                 !m_exportPasswordEdit->text().isEmpty() &&
                 m_passwordValid;
    
    m_exportButton->setEnabled(valid);
}

void VaultManager::validateImportInputs()
{
    bool valid = !m_importPathEdit->text().trimmed().isEmpty() && 
                 !m_importPasswordEdit->text().isEmpty();
    
    m_importButton->setEnabled(valid);
}

void VaultManager::updateButtonStates()
{
    validateExportInputs();
    validateImportInputs();
}

void VaultManager::showError(const QString& message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

void VaultManager::showSuccess(const QString& message)
{
    QMessageBox::information(this, tr("Success"), message);
}

void VaultManager::showWarning(const QString& message)
{
    QMessageBox::warning(this, tr("Warning"), message);
}

void VaultManager::showInfo(const QString& message)
{
    QMessageBox::information(this, tr("Information"), message);
}
