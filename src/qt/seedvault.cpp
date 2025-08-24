#include "seedvault.h"
#include "walletmodel.h"
#include "guiutil.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QSettings>
#include <QApplication>
#include <QStyle>
#include <QFont>
#include <QFontMetrics>

SeedVault::SeedVault(WalletModel* walletModel, QWidget* parent)
    : QDialog(parent)
    , m_walletModel(walletModel)
    , m_seedVisible(false)
    , m_passwordValid(false)
    , m_settings(new QSettings("Shahcoin", "Shahcoin-Qt"))
    , m_passwordAttempts(0)
    , m_lockoutTimer(new QTimer(this))
{
    setWindowTitle(tr("🔐 Seed Vault - Backup Your Wallet"));
    setModal(true);
    setFixedSize(600, 500);
    
    setupUI();
    connectSignals();
    loadSettings();
    
    // Initialize mock seed phrase for demo
    m_seedPhrase = generateMockSeedPhrase();
    
    // Setup lockout timer
    connect(m_lockoutTimer, &QTimer::timeout, this, [this]() {
        m_passwordAttempts = 0;
        m_showSeedButton->setEnabled(true);
        m_passwordEdit->setEnabled(true);
    });
}

SeedVault::~SeedVault()
{
    delete m_settings;
}

void SeedVault::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    QLabel* titleLabel = new QLabel(tr("🔐 Seed Vault"));
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    m_mainLayout->addWidget(titleLabel);
    
    // Description
    QLabel* descLabel = new QLabel(tr("Your seed phrase is the master key to your wallet. Keep it safe and never share it with anyone."));
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setStyleSheet("color: #666666; margin-bottom: 10px;");
    m_mainLayout->addWidget(descLabel);
    
    setupSecurityGroup();
    setupSeedDisplayGroup();
    setupButtonGroup();
}

void SeedVault::setupSecurityGroup()
{
    m_securityGroup = new QGroupBox(tr("🔒 Security Verification"));
    QVBoxLayout* securityLayout = new QVBoxLayout(m_securityGroup);
    
    // Password section
    QHBoxLayout* passwordLayout = new QHBoxLayout();
    m_passwordLabel = new QLabel(tr("Wallet Password:"));
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("Enter your wallet password"));
    
    m_showPasswordCheckbox = new QCheckBox(tr("Show"));
    m_showPasswordCheckbox->setChecked(false);
    
    passwordLayout->addWidget(m_passwordLabel);
    passwordLayout->addWidget(m_passwordEdit);
    passwordLayout->addWidget(m_showPasswordCheckbox);
    
    securityLayout->addLayout(passwordLayout);
    
    // Show seed button
    m_showSeedButton = new QPushButton(tr("🔓 Show Seed Phrase"));
    m_showSeedButton->setEnabled(false);
    m_showSeedButton->setStyleSheet(
        "QPushButton { "
        "background-color: #28a745; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px 16px; "
        "font-weight: bold; "
        "}"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:disabled { background-color: #6c757d; }"
    );
    
    securityLayout->addWidget(m_showSeedButton);
    
    m_mainLayout->addWidget(m_securityGroup);
}

void SeedVault::setupSeedDisplayGroup()
{
    m_seedDisplayGroup = new QGroupBox(tr("🌱 Seed Phrase"));
    m_seedDisplayGroup->setVisible(false);
    QVBoxLayout* seedLayout = new QVBoxLayout(m_seedDisplayGroup);
    
    // Instructions
    m_seedInstructionsLabel = new QLabel(tr("Write down these 12 words in order and store them securely:"));
    m_seedInstructionsLabel->setWordWrap(true);
    m_seedInstructionsLabel->setStyleSheet("color: #dc3545; font-weight: bold; margin-bottom: 10px;");
    seedLayout->addWidget(m_seedInstructionsLabel);
    
    // Seed display
    m_seedDisplayEdit = new QTextEdit();
    m_seedDisplayEdit->setReadOnly(true);
    m_seedDisplayEdit->setMaximumHeight(120);
    m_seedDisplayEdit->setStyleSheet(
        "QTextEdit { "
        "background-color: #f8f9fa; "
        "border: 2px solid #dee2e6; "
        "border-radius: 4px; "
        "padding: 10px; "
        "font-family: 'Courier New', monospace; "
        "font-size: 14px; "
        "font-weight: bold; "
        "}"
    );
    seedLayout->addWidget(m_seedDisplayEdit);
    
    // Seed action buttons
    m_seedButtonLayout = new QHBoxLayout();
    
    m_copySeedButton = new QPushButton(tr("📋 Copy to Clipboard"));
    m_copySeedButton->setStyleSheet(
        "QPushButton { "
        "background-color: #007bff; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #0056b3; }"
    );
    
    m_exportSeedButton = new QPushButton(tr("💾 Export to File"));
    m_exportSeedButton->setStyleSheet(
        "QPushButton { "
        "background-color: #17a2b8; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #138496; }"
    );
    
    m_seedButtonLayout->addWidget(m_copySeedButton);
    m_seedButtonLayout->addWidget(m_exportSeedButton);
    m_seedButtonLayout->addStretch();
    
    seedLayout->addLayout(m_seedButtonLayout);
    
    m_mainLayout->addWidget(m_seedDisplayGroup);
}

void SeedVault::setupButtonGroup()
{
    m_buttonLayout = new QHBoxLayout();
    
    m_acceptButton = new QPushButton(tr("✅ Done"));
    m_acceptButton->setStyleSheet(
        "QPushButton { "
        "background-color: #28a745; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px 16px; "
        "font-weight: bold; "
        "}"
        "QPushButton:hover { background-color: #218838; }"
    );
    
    m_cancelButton = new QPushButton(tr("❌ Cancel"));
    m_cancelButton->setStyleSheet(
        "QPushButton { "
        "background-color: #dc3545; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px 16px; "
        "}"
        "QPushButton:hover { background-color: #c82333; }"
    );
    
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_acceptButton);
    m_buttonLayout->addWidget(m_cancelButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void SeedVault::connectSignals()
{
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &SeedVault::onPasswordChanged);
    connect(m_showPasswordCheckbox, &QCheckBox::toggled, this, &SeedVault::onShowPasswordToggled);
    connect(m_showSeedButton, &QPushButton::clicked, this, &SeedVault::onShowSeedClicked);
    connect(m_copySeedButton, &QPushButton::clicked, this, &SeedVault::onCopySeedClicked);
    connect(m_exportSeedButton, &QPushButton::clicked, this, &SeedVault::onExportSeedClicked);
    connect(m_acceptButton, &QPushButton::clicked, this, &SeedVault::onAcceptClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SeedVault::onCancelClicked);
}

void SeedVault::loadSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_passwordAttempts = m_settings->value(PASSWORD_ATTEMPTS_KEY, 0).toInt();
    m_settings->endGroup();
    
    // Check if we're in lockout period
    if (m_passwordAttempts >= MAX_PASSWORD_ATTEMPTS) {
        m_showSeedButton->setEnabled(false);
        m_passwordEdit->setEnabled(false);
        m_lockoutTimer->start(LOCKOUT_DURATION_MS);
    }
}

void SeedVault::saveSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_settings->setValue(PASSWORD_ATTEMPTS_KEY, m_passwordAttempts);
    m_settings->setValue(LAST_ACCESS_KEY, QDateTime::currentDateTime().toString());
    m_settings->endGroup();
    m_settings->sync();
}

bool SeedVault::hasSeedPhrase() const
{
    return !m_seedPhrase.isEmpty();
}

QStringList SeedVault::getSeedPhrase() const
{
    return m_seedPhrase;
}

bool SeedVault::validateSeedPhrase(const QStringList& words) const
{
    // Basic validation - check if we have 12 or 24 words
    if (words.size() != 12 && words.size() != 24) {
        return false;
    }
    
    // Check if all words are non-empty
    for (const QString& word : words) {
        if (word.trimmed().isEmpty()) {
            return false;
        }
    }
    
    return true;
}

bool SeedVault::isPasswordValid(const QString& password) const
{
    return checkWalletPassword(password);
}

void SeedVault::setPassword(const QString& password)
{
    m_walletPassword = password;
}

bool SeedVault::exportSeedToFile(const QString& filePath) const
{
    if (!m_seedVisible) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << generateSeedBackupText();
    file.close();
    
    return true;
}

QString SeedVault::generateSeedBackupText() const
{
    QString text;
    text += "=== SHAHCOIN WALLET SEED PHRASE BACKUP ===\n\n";
    text += "IMPORTANT: Keep this file secure and never share it!\n\n";
    text += "Date: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n";
    text += "Wallet: Shahcoin Qt Wallet\n\n";
    text += "Your 12-word seed phrase:\n";
    text += "========================\n";
    
    for (int i = 0; i < m_seedPhrase.size(); ++i) {
        text += QString("%1. %2").arg(i + 1, 2, 10, QChar('0')).arg(m_seedPhrase[i]);
        if ((i + 1) % 3 == 0) {
            text += "\n";
        } else {
            text += "    ";
        }
    }
    
    text += "\n\n";
    text += "Instructions:\n";
    text += "1. Write down these words on paper\n";
    text += "2. Store in a secure location\n";
    text += "3. Never share with anyone\n";
    text += "4. Use to recover wallet if needed\n\n";
    text += "Security Warning:\n";
    text += "- Anyone with these words can access your funds\n";
    text += "- Keep this file encrypted or delete after backup\n";
    text += "- Consider using a hardware wallet for large amounts\n";
    
    return text;
}

void SeedVault::onShowSeedClicked()
{
    QString password = m_passwordEdit->text();
    
    if (!checkWalletPassword(password)) {
        m_passwordAttempts++;
        saveSettings();
        
        if (m_passwordAttempts >= MAX_PASSWORD_ATTEMPTS) {
            showError(tr("Access Denied"), 
                     tr("Too many failed attempts. Please wait 5 minutes before trying again."));
            m_showSeedButton->setEnabled(false);
            m_passwordEdit->setEnabled(false);
            m_lockoutTimer->start(LOCKOUT_DURATION_MS);
            emit accessDenied();
            return;
        }
        
        showError(tr("Invalid Password"), 
                 tr("Incorrect password. %1 attempts remaining.").arg(MAX_PASSWORD_ATTEMPTS - m_passwordAttempts));
        return;
    }
    
    // Reset password attempts on success
    m_passwordAttempts = 0;
    saveSettings();
    
    showSeedPhrase();
}

void SeedVault::onCopySeedClicked()
{
    if (!m_seedVisible) {
        showError(tr("Access Denied"), tr("Please verify your password first."));
        return;
    }
    
    QString seedText = m_seedPhrase.join(" ");
    QApplication::clipboard()->setText(seedText);
    
    showSuccess(tr("Copied"), tr("Seed phrase copied to clipboard."));
    emit seedCopied();
}

void SeedVault::onExportSeedClicked()
{
    if (!m_seedVisible) {
        showError(tr("Access Denied"), tr("Please verify your password first."));
        return;
    }
    
    QString fileName = QString("shahcoin_seed_backup_%1.txt")
                      .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    
    QString filePath = QFileDialog::getSaveFileName(this, 
                                                   tr("Export Seed Phrase"), 
                                                   fileName, 
                                                   tr("Text Files (*.txt);;All Files (*)"));
    
    if (!filePath.isEmpty()) {
        if (exportSeedToFile(filePath)) {
            showSuccess(tr("Exported"), tr("Seed phrase exported to: %1").arg(filePath));
            emit seedExported(filePath);
        } else {
            showError(tr("Export Failed"), tr("Could not write to file: %1").arg(filePath));
        }
    }
}

void SeedVault::onPasswordChanged()
{
    QString password = m_passwordEdit->text();
    m_showSeedButton->setEnabled(!password.isEmpty());
}

void SeedVault::onShowPasswordToggled(bool checked)
{
    m_passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

void SeedVault::onAcceptClicked()
{
    if (m_seedVisible) {
        showSuccess(tr("Backup Complete"), 
                   tr("Remember to store your seed phrase securely and never share it!"));
    }
    accept();
}

void SeedVault::onCancelClicked()
{
    reject();
}

void SeedVault::showSeedPhrase()
{
    if (!hasSeedPhrase()) {
        showWarning(tr("No Seed Phrase"), 
                   tr("This wallet was created without a seed phrase. Consider creating a new wallet with seed backup."));
        return;
    }
    
    // Display seed phrase in formatted text
    QString seedText;
    for (int i = 0; i < m_seedPhrase.size(); ++i) {
        seedText += QString("%1. %2").arg(i + 1, 2, 10, QChar('0')).arg(m_seedPhrase[i]);
        if ((i + 1) % 3 == 0) {
            seedText += "\n";
        } else {
            seedText += "    ";
        }
    }
    
    m_seedDisplayEdit->setText(seedText);
    m_seedDisplayGroup->setVisible(true);
    m_seedVisible = true;
    
    // Auto-hide after 30 seconds for security
    QTimer::singleShot(30000, this, &SeedVault::hideSeedPhrase);
}

void SeedVault::hideSeedPhrase()
{
    m_seedDisplayGroup->setVisible(false);
    m_seedDisplayEdit->clear();
    m_seedVisible = false;
}

void SeedVault::validateInputs()
{
    // Additional validation if needed
}

void SeedVault::showWarning(const QString& title, const QString& message)
{
    QMessageBox::warning(this, title, message);
}

void SeedVault::showSuccess(const QString& title, const QString& message)
{
    QMessageBox::information(this, title, message);
}

void SeedVault::showError(const QString& title, const QString& message)
{
    QMessageBox::critical(this, title, message);
}

QString SeedVault::getWalletPassword() const
{
    return m_walletPassword;
}

bool SeedVault::checkWalletPassword(const QString& password) const
{
    // In a real implementation, this would check against the actual wallet password
    // For demo purposes, we'll use a simple check
    if (m_walletModel) {
        // This would typically call a method on the wallet model
        // return m_walletModel->checkPassword(password);
    }
    
    // Demo: accept any non-empty password
    return !password.trimmed().isEmpty();
}

QStringList SeedVault::generateMockSeedPhrase() const
{
    // BIP39 word list (first 12 words for demo)
    QStringList bip39Words = {
        "abandon", "ability", "able", "about", "above", "absent", "absorb", "abstract", "absurd", "abuse",
        "access", "accident", "account", "accuse", "achieve", "acid", "acoustic", "acquire", "across", "act",
        "action", "actor", "actual", "adapt", "add", "addict", "address", "adjust", "admit", "adult",
        "advance", "advice", "aerobic", "affair", "afford", "afraid", "again", "age", "agent", "agree"
    };
    
    // Generate a deterministic but seemingly random seed phrase
    QStringList seedPhrase;
    QDateTime now = QDateTime::currentDateTime();
    qint64 seed = now.toSecsSinceEpoch();
    
    for (int i = 0; i < 12; ++i) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        int index = seed % bip39Words.size();
        seedPhrase.append(bip39Words[index]);
    }
    
    return seedPhrase;
}
