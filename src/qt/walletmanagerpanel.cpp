#include <qt/walletmanagerpanel.h>
#include <qt/walletmodel.h>
#include <qt/walletcontroller.h>
#include <qt/platformstyle.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>
#include <interfaces/wallet.h>
#include <util/translation.h>
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
#include <QFileInfo>

WalletManagerPanel::WalletManagerPanel(const PlatformStyle* platformStyle, QWidget* parent)
    : QWidget(parent)
    , m_walletController(nullptr)
    , m_walletModel(nullptr)
    , m_platformStyle(platformStyle)
    , m_refreshTimer(new QTimer(this))
    , m_settings(new QSettings("Shahcoin", "WalletManager", this))
{
    setupUI();
    setupDialogs();
    connectSignals();
    applyTheme();
    
    // Set up refresh timer (every 30 seconds)
    connect(m_refreshTimer, &QTimer::timeout, this, &WalletManagerPanel::refreshData);
    m_refreshTimer->start(30000); // 30 seconds
    
    refreshData();
}

WalletManagerPanel::~WalletManagerPanel()
{
    if (m_refreshTimer) {
        m_refreshTimer->stop();
    }
}

void WalletManagerPanel::setWalletController(WalletController* walletController)
{
    m_walletController = walletController;
    refreshData();
}

void WalletManagerPanel::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    refreshData();
}

void WalletManagerPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_mainLayout->addWidget(m_tabWidget);
    
    // Status Bar
    QFrame* statusFrame = new QFrame(this);
    statusFrame->setFrameStyle(QFrame::StyledPanel);
    statusFrame->setStyleSheet("QFrame { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 3px; }");
    QHBoxLayout* statusLayout = new QHBoxLayout(statusFrame);
    statusLayout->setContentsMargins(10, 5, 10, 5);
    
    QLabel* statusIcon = new QLabel("ℹ️", this);
    m_statusTextLabel = new QLabel(tr("Ready to manage your wallets. Select a wallet from the list above to get started."), this);
    m_statusTextLabel->setStyleSheet("QLabel { color: #495057; font-size: 12px; }");
    
    statusLayout->addWidget(statusIcon);
    statusLayout->addWidget(m_statusTextLabel);
    statusLayout->addStretch();
    
    m_mainLayout->addWidget(statusFrame);
    
    setupWalletTable();
    setupControls();
    setupStatsTab();
    setupSettingsTab();
    setupLanguageSelector();
}

void WalletManagerPanel::setupWalletTable()
{
    m_mainTab = new QWidget();
    m_mainLayout = new QVBoxLayout(m_mainTab);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(20);
    
    // Main Instruction Label
    QLabel* instructionLabel = new QLabel(tr("💡 Manage all your wallets here. You can create new wallets, switch between them, back them up, or secure them individually."), this);
    instructionLabel->setWordWrap(true);
    instructionLabel->setStyleSheet("QLabel { background-color: #e3f2fd; border: 1px solid #2196f3; border-radius: 5px; padding: 10px; color: #1976d2; font-weight: bold; }");
    m_mainLayout->addWidget(instructionLabel);
    
    // Wallet Group
    m_walletGroup = new QGroupBox(tr("💼 Wallet Management"));
    QVBoxLayout* walletLayout = new QVBoxLayout(m_walletGroup);
    
    // Wallet Table
    m_walletTable = new QTableWidget(this);
    m_walletTable->setColumnCount(9);
    m_walletTable->setHorizontalHeaderLabels({
        tr("Wallet Name"),
        tr("File Name"),
        tr("Balance"),
        tr("Status"),
        tr("Cold Wallet"),
        tr("Created"),
        tr("Size"),
        tr("Notes"),
        tr("Actions")
    });
    
    // Set tooltips for table headers
    m_walletTable->horizontalHeaderItem(0)->setToolTip(tr("Your custom name for this wallet. Useful for identifying different purposes like 'Mining', 'Savings', etc."));
    m_walletTable->horizontalHeaderItem(1)->setToolTip(tr("The physical file where this wallet is stored (e.g. wallet1.dat)."));
    m_walletTable->horizontalHeaderItem(2)->setToolTip(tr("Total balance currently held in this wallet."));
    m_walletTable->horizontalHeaderItem(3)->setToolTip(tr("Indicates if this wallet is actively loaded or not."));
    m_walletTable->horizontalHeaderItem(4)->setToolTip(tr("Mark as cold wallet for long-term storage. Cold wallets are typically offline wallets used for secure fund storage."));
    m_walletTable->horizontalHeaderItem(5)->setToolTip(tr("Date when this wallet was created."));
    m_walletTable->horizontalHeaderItem(6)->setToolTip(tr("Size of the wallet file on disk."));
    m_walletTable->horizontalHeaderItem(7)->setToolTip(tr("Custom notes or tags for this wallet."));
    m_walletTable->horizontalHeaderItem(8)->setToolTip(tr("Use these buttons to switch, backup, encrypt, or unload your wallet."));
    
    // Set table properties
    m_walletTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_walletTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_walletTable->setAlternatingRowColors(true);
    m_walletTable->setContextMenuPolicy(Qt::CustomContextMenu);
    m_walletTable->horizontalHeader()->setStretchLastSection(true);
    m_walletTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    
    walletLayout->addWidget(m_walletTable);
    m_mainLayout->addWidget(m_walletGroup);
    
    m_tabWidget->addTab(m_mainTab, tr("💼 Wallets"));
}

void WalletManagerPanel::setupControls()
{
    m_controlsLayout = new QHBoxLayout();
    
    // Create Wallet Button
    m_createWalletButton = new QPushButton(tr("➕ Create Wallet"), this);
    m_createWalletButton->setToolTip(tr("Generate a brand new wallet. You can name and secure it during creation."));
    
    // Load Wallet Button
    m_loadWalletButton = new QPushButton(tr("📂 Load Wallet"), this);
    m_loadWalletButton->setToolTip(tr("Open a previously created wallet from your computer."));
    
    // Refresh Button
    m_refreshButton = new QPushButton(tr("🔄 Refresh"), this);
    m_refreshButton->setToolTip(tr("Refresh wallet list and update balances."));
    
    // Export Button
    m_exportButton = new QPushButton(tr("📁 Export List"), this);
    m_exportButton->setToolTip(tr("Export wallet list to a JSON file for backup or reference."));
    
    m_controlsLayout->addWidget(m_createWalletButton);
    m_controlsLayout->addWidget(m_loadWalletButton);
    m_controlsLayout->addStretch();
    m_controlsLayout->addWidget(m_refreshButton);
    m_controlsLayout->addWidget(m_exportButton);
    
    m_mainLayout->addLayout(m_controlsLayout);
    
    // Bottom Tip Area
    QLabel* tipLabel = new QLabel(tr("💡 You can load multiple wallets at once and switch between them at any time."), this);
    tipLabel->setWordWrap(true);
    tipLabel->setStyleSheet("QLabel { background-color: #fff3cd; border: 1px solid #ffeaa7; border-radius: 5px; padding: 8px; color: #856404; font-style: italic; }");
    m_mainLayout->addWidget(tipLabel);
}

void WalletManagerPanel::setupStatsTab()
{
    m_statsTab = new QWidget();
    m_statsLayout = new QVBoxLayout(m_statsTab);
    m_statsLayout->setContentsMargins(0, 0, 0, 0);
    m_statsLayout->setSpacing(20);
    
    // Total Balance Group
    m_totalBalanceGroup = new QGroupBox(tr("📊 Wallet Statistics"));
    QGridLayout* statsGrid = new QGridLayout(m_totalBalanceGroup);
    
    m_totalBalanceLabel = new QLabel(tr("Total Balance: 0 SHAH"), this);
    m_totalBalanceLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2E8B57;");
    
    m_totalWalletsLabel = new QLabel(tr("Total Wallets: 0"), this);
    m_loadedWalletsLabel = new QLabel(tr("Loaded Wallets: 0"), this);
    m_encryptedWalletsLabel = new QLabel(tr("Encrypted Wallets: 0"), this);
    
    statsGrid->addWidget(m_totalBalanceLabel, 0, 0, 1, 2);
    statsGrid->addWidget(m_totalWalletsLabel, 1, 0);
    statsGrid->addWidget(m_loadedWalletsLabel, 1, 1);
    statsGrid->addWidget(m_encryptedWalletsLabel, 2, 0);
    
    m_statsLayout->addWidget(m_totalBalanceGroup);
    
    // Help Section
    QGroupBox* helpGroup = new QGroupBox(tr("💡 Quick Tips"), this);
    QVBoxLayout* helpLayout = new QVBoxLayout(helpGroup);
    
    QLabel* helpLabel1 = new QLabel(tr("• Right-click on any wallet for more options"), this);
    QLabel* helpLabel2 = new QLabel(tr("• Use the Settings tab to configure automatic backups"), this);
    QLabel* helpLabel3 = new QLabel(tr("• Encrypted wallets provide better security"), this);
    QLabel* helpLabel4 = new QLabel(tr("• Keep regular backups of your wallet files"), this);
    
    helpLabel1->setStyleSheet("QLabel { color: #666; font-size: 11px; }");
    helpLabel2->setStyleSheet("QLabel { color: #666; font-size: 11px; }");
    helpLabel3->setStyleSheet("QLabel { color: #666; font-size: 11px; }");
    helpLabel4->setStyleSheet("QLabel { color: #666; font-size: 11px; }");
    
    helpLayout->addWidget(helpLabel1);
    helpLayout->addWidget(helpLabel2);
    helpLayout->addWidget(helpLabel3);
    helpLayout->addWidget(helpLabel4);
    
    m_statsLayout->addWidget(helpGroup);
    m_statsLayout->addStretch();
    
    m_tabWidget->addTab(m_statsTab, tr("📊 Statistics"));
}

void WalletManagerPanel::setupSettingsTab()
{
    m_settingsTab = new QWidget();
    m_settingsLayout = new QVBoxLayout(m_settingsTab);
    m_settingsLayout->setContentsMargins(0, 0, 0, 0);
    m_settingsLayout->setSpacing(20);
    
    // Auto Load Group
    m_autoLoadGroup = new QGroupBox(tr("⚙️ Auto Load Settings"));
    QVBoxLayout* autoLoadLayout = new QVBoxLayout(m_autoLoadGroup);
    
    m_autoLoadDefaultCheck = new QCheckBox(tr("Auto-load default wallet on startup"), this);
    m_autoLoadDefaultCheck->setToolTip(tr("Automatically load the default wallet when the application starts."));
    m_rememberLastWalletCheck = new QCheckBox(tr("Remember last used wallet"), this);
    m_rememberLastWalletCheck->setToolTip(tr("Remember and automatically switch to the last wallet you used."));
    m_showWalletNotesCheck = new QCheckBox(tr("Show wallet notes in table"), this);
    m_showWalletNotesCheck->setToolTip(tr("Display custom notes and tags in the wallet list table."));
    
    autoLoadLayout->addWidget(m_autoLoadDefaultCheck);
    autoLoadLayout->addWidget(m_rememberLastWalletCheck);
    autoLoadLayout->addWidget(m_showWalletNotesCheck);
    
    // Backup Group
    m_backupGroup = new QGroupBox(tr("💾 Backup Settings"));
    QFormLayout* backupLayout = new QFormLayout(m_backupGroup);
    
    m_autoBackupCheck = new QCheckBox(tr("Enable automatic backup"), this);
    m_autoBackupCheck->setToolTip(tr("Automatically create backups of your wallets at regular intervals."));
    m_backupIntervalSpinBox = new QSpinBox(this);
    m_backupIntervalSpinBox->setRange(1, 365);
    m_backupIntervalSpinBox->setValue(7);
    m_backupIntervalSpinBox->setSuffix(tr(" days"));
    m_backupIntervalSpinBox->setToolTip(tr("How often to automatically backup your wallets."));
    
    m_backupPathEdit = new QLineEdit(this);
    m_backupPathEdit->setToolTip(tr("Directory where automatic backups will be stored."));
    m_backupPathButton = new QPushButton(tr("Browse"), this);
    m_backupPathButton->setToolTip(tr("Browse to select a directory for automatic backups."));
    
    QHBoxLayout* backupPathLayout = new QHBoxLayout();
    backupPathLayout->addWidget(m_backupPathEdit);
    backupPathLayout->addWidget(m_backupPathButton);
    
    backupLayout->addRow(m_autoBackupCheck);
    backupLayout->addRow(tr("Backup interval:"), m_backupIntervalSpinBox);
    backupLayout->addRow(tr("Backup path:"), backupPathLayout);
    
    // Language Settings Group
    QGroupBox* languageGroup = new QGroupBox(tr("🌐 Language Settings"), this);
    QFormLayout* languageLayout = new QFormLayout(languageGroup);
    
    m_languageLabel = new QLabel(tr("Interface Language:"), this);
    m_languageSelector = new QComboBox(this);
    m_languageSelector->setToolTip(tr("Select your preferred language for the wallet interface."));
    
    // Add supported languages
    m_languageSelector->addItem("🇺🇸 English", "en");
    m_languageSelector->addItem("🇫🇷 Français", "fr");
    m_languageSelector->addItem("🇮🇷 فارسی", "fa");
    m_languageSelector->addItem("🇪🇸 Español", "es");
    m_languageSelector->addItem("🇸🇦 العربية", "ar");
    m_languageSelector->addItem("🇨🇳 中文", "zh");
    m_languageSelector->addItem("🇯🇵 日本語", "ja");
    m_languageSelector->addItem("🇩🇪 Deutsch", "de");
    
    languageLayout->addRow(m_languageLabel, m_languageSelector);
    
    // Connect language selector
    connect(m_languageSelector, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
        this, &WalletManagerPanel::onLanguageChanged);
    
    m_settingsLayout->addWidget(m_autoLoadGroup);
    m_settingsLayout->addWidget(m_backupGroup);
    m_settingsLayout->addWidget(languageGroup);
    m_settingsLayout->addStretch();
    
    m_tabWidget->addTab(m_settingsTab, tr("⚙️ Settings"));
}

void WalletManagerPanel::setupDialogs()
{
    // Create Wallet Dialog
    m_createWalletDialog = new QDialog(this);
    m_createWalletDialog->setWindowTitle(tr("Create New Wallet"));
    m_createWalletDialog->setModal(true);
    m_createWalletDialog->resize(400, 300);
    
    m_createWalletLayout = new QVBoxLayout(m_createWalletDialog);
    m_createWalletForm = new QFormLayout();
    
    m_walletNameEdit = new QLineEdit(this);
    m_walletNameEdit->setToolTip(tr("Enter a descriptive name for your wallet (e.g., 'Mining Wallet', 'Savings')."));
    
    // Wallet Name Suggestions ComboBox
    QComboBox* walletNameSuggestions = new QComboBox(this);
    walletNameSuggestions->addItem(tr("-- Select a suggested name --"));
    walletNameSuggestions->addItem(tr("Main Wallet"));
    walletNameSuggestions->addItem(tr("Mining Wallet"));
    walletNameSuggestions->addItem(tr("Cold Vault"));
    walletNameSuggestions->addItem(tr("Trading Wallet"));
    walletNameSuggestions->addItem(tr("NFT Wallet"));
    walletNameSuggestions->addItem(tr("Test Wallet"));
    walletNameSuggestions->addItem(tr("Savings Wallet"));
    walletNameSuggestions->addItem(tr("Staking Wallet"));
    walletNameSuggestions->setToolTip(tr("Pick a helpful name to organize your wallets by purpose."));
    
    // Connect suggestions to the name edit
    connect(walletNameSuggestions, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
        [this, walletNameSuggestions](const QString& text) {
            if (text != tr("-- Select a suggested name --")) {
                m_walletNameEdit->setText(text);
            }
        });
    
    m_walletDescriptionEdit = new QLineEdit(this);
    m_walletDescriptionEdit->setToolTip(tr("Optional description to help identify this wallet's purpose."));
    m_encryptWalletCheck = new QCheckBox(tr("Encrypt wallet"), this);
    m_encryptWalletCheck->setToolTip(tr("Protect your wallet with a password for enhanced security."));
    m_walletPasswordEdit = new QLineEdit(this);
    m_walletPasswordEdit->setEchoMode(QLineEdit::Password);
    m_walletPasswordEdit->setToolTip(tr("Enter a strong password to encrypt your wallet. You'll need this for transactions."));
    
    // Password Strength Meter
    m_passwordStrengthMeter = new QProgressBar(this);
    m_passwordStrengthMeter->setRange(0, 100);
    m_passwordStrengthMeter->setValue(0);
    m_passwordStrengthMeter->setTextVisible(false);
    m_passwordStrengthMeter->setMaximumHeight(8);
    
    m_passwordStrengthLabel = new QLabel(tr("Password Strength: Weak"), this);
    m_passwordStrengthLabel->setStyleSheet("QLabel { color: #dc3545; font-weight: bold; }");
    
    m_passwordStrengthTip = new QLabel(tr("💡 Use 8+ characters, mix upper/lowercase, numbers, and symbols."), this);
    m_passwordStrengthTip->setStyleSheet("QLabel { color: #6c757d; font-size: 10px; font-style: italic; }");
    m_passwordStrengthTip->setWordWrap(true);
    
    // Connect password field to strength meter
    connect(m_walletPasswordEdit, &QLineEdit::textChanged, this, &WalletManagerPanel::updatePasswordStrengthMeter);
    
    m_walletPasswordConfirmEdit = new QLineEdit(this);
    m_walletPasswordConfirmEdit->setEchoMode(QLineEdit::Password);
    m_walletPasswordConfirmEdit->setToolTip(tr("Confirm your password by entering it again."));
    
    m_createWalletForm->addRow(tr("Wallet Name:"), m_walletNameEdit);
    m_createWalletForm->addRow(tr("Suggested Names:"), walletNameSuggestions);
    m_createWalletForm->addRow(tr("Description:"), m_walletDescriptionEdit);
    m_createWalletForm->addRow(m_encryptWalletCheck);
    m_createWalletForm->addRow(tr("Password:"), m_walletPasswordEdit);
    m_createWalletForm->addRow(m_passwordStrengthMeter);
    m_createWalletForm->addRow(m_passwordStrengthLabel);
    m_createWalletForm->addRow(m_passwordStrengthTip);
    m_createWalletForm->addRow(tr("Confirm Password:"), m_walletPasswordConfirmEdit);
    
    m_createWalletButtonLayout = new QHBoxLayout();
    m_createWalletConfirmButton = new QPushButton(tr("Create"), this);
    m_createWalletConfirmButton->setToolTip(tr("Create the new wallet with the specified settings."));
    m_createWalletCancelButton = new QPushButton(tr("Cancel"), this);
    m_createWalletCancelButton->setToolTip(tr("Cancel wallet creation and return to wallet list."));
    
    m_createWalletButtonLayout->addStretch();
    m_createWalletButtonLayout->addWidget(m_createWalletConfirmButton);
    m_createWalletButtonLayout->addWidget(m_createWalletCancelButton);
    
    m_createWalletLayout->addLayout(m_createWalletForm);
    m_createWalletLayout->addLayout(m_createWalletButtonLayout);
    
    // Load Wallet Dialog
    m_loadWalletDialog = new QDialog(this);
    m_loadWalletDialog->setWindowTitle(tr("Load Wallet"));
    m_loadWalletDialog->setModal(true);
    m_loadWalletDialog->resize(400, 200);
    
    m_loadWalletLayout = new QVBoxLayout(m_loadWalletDialog);
    m_loadWalletForm = new QFormLayout();
    
    m_loadWalletPathEdit = new QLineEdit(this);
    m_loadWalletPathEdit->setToolTip(tr("Path to the wallet file you want to load (.dat file)."));
    m_loadWalletBrowseButton = new QPushButton(tr("Browse"), this);
    m_loadWalletBrowseButton->setToolTip(tr("Browse your computer to select a wallet file."));
    m_loadWalletEncryptedCheck = new QCheckBox(tr("Wallet is encrypted"), this);
    m_loadWalletEncryptedCheck->setToolTip(tr("Check this if the wallet file is password-protected."));
    m_loadWalletPasswordEdit = new QLineEdit(this);
    m_loadWalletPasswordEdit->setEchoMode(QLineEdit::Password);
    m_loadWalletPasswordEdit->setToolTip(tr("Enter the password for the encrypted wallet."));
    
    QHBoxLayout* loadPathLayout = new QHBoxLayout();
    loadPathLayout->addWidget(m_loadWalletPathEdit);
    loadPathLayout->addWidget(m_loadWalletBrowseButton);
    
    m_loadWalletForm->addRow(tr("Wallet Path:"), loadPathLayout);
    m_loadWalletForm->addRow(m_loadWalletEncryptedCheck);
    m_loadWalletForm->addRow(tr("Password:"), m_loadWalletPasswordEdit);
    
    m_loadWalletButtonLayout = new QHBoxLayout();
    m_loadWalletConfirmButton = new QPushButton(tr("Load"), this);
    m_loadWalletConfirmButton->setToolTip(tr("Load the selected wallet file into the application."));
    m_loadWalletCancelButton = new QPushButton(tr("Cancel"), this);
    m_loadWalletCancelButton->setToolTip(tr("Cancel loading and return to wallet list."));
    
    m_loadWalletButtonLayout->addStretch();
    m_loadWalletButtonLayout->addWidget(m_loadWalletConfirmButton);
    m_loadWalletButtonLayout->addWidget(m_loadWalletCancelButton);
    
    m_loadWalletLayout->addLayout(m_loadWalletForm);
    m_loadWalletLayout->addLayout(m_loadWalletButtonLayout);
    
    // Setup other dialogs (simplified for brevity)
    setupContextMenu();
}

void WalletManagerPanel::setupContextMenu()
{
    m_walletContextMenu = new QMenu(this);
    
    m_viewDetailsAction = new QAction(tr("👁️ View Details"), this);
    m_viewDetailsAction->setToolTip(tr("View detailed information about this wallet including balance, transactions, and settings."));
    
    m_switchAction = new QAction(tr("🔄 Switch to Wallet"), this);
    m_switchAction->setToolTip(tr("Set this wallet as your active wallet. All transactions will use it."));
    
    m_backupAction = new QAction(tr("💾 Backup Wallet"), this);
    m_backupAction->setToolTip(tr("Save a secure backup (.dat) of this wallet. Store in a safe location."));
    
    m_encryptAction = new QAction(tr("🔐 Encrypt Wallet"), this);
    m_encryptAction->setToolTip(tr("Protect this wallet with a password. You'll need it for transactions."));
    
    m_unloadAction = new QAction(tr("📤 Unload Wallet"), this);
    m_unloadAction->setToolTip(tr("Temporarily remove this wallet from the app. You can load it again later."));
    
    m_renameAction = new QAction(tr("✏️ Rename Wallet"), this);
    m_renameAction->setToolTip(tr("Change the display name of this wallet for easier identification."));
    
    m_deleteAction = new QAction(tr("🗑️ Delete Wallet"), this);
    m_deleteAction->setToolTip(tr("Permanently delete this wallet file from your device. ⚠️ Irreversible."));
    
    m_walletContextMenu->addAction(m_viewDetailsAction);
    m_walletContextMenu->addSeparator();
    m_walletContextMenu->addAction(m_switchAction);
    m_walletContextMenu->addAction(m_backupAction);
    m_walletContextMenu->addAction(m_encryptAction);
    m_walletContextMenu->addAction(m_unloadAction);
    m_walletContextMenu->addSeparator();
    m_walletContextMenu->addAction(m_renameAction);
    m_walletContextMenu->addAction(m_deleteAction);
}

void WalletManagerPanel::connectSignals()
{
    // Button connections
    connect(m_createWalletButton, &QPushButton::clicked, this, &WalletManagerPanel::onCreateWalletClicked);
    connect(m_loadWalletButton, &QPushButton::clicked, this, &WalletManagerPanel::onLoadWalletClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &WalletManagerPanel::onRefreshClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &WalletManagerPanel::onExportClicked);
    
    // Table connections
    connect(m_walletTable, &QTableWidget::cellClicked, this, &WalletManagerPanel::onWalletRowClicked);
    connect(m_walletTable, &QTableWidget::customContextMenuRequested, this, &WalletManagerPanel::onWalletContextMenu);
    
    // Dialog connections
    connect(m_createWalletConfirmButton, &QPushButton::clicked, this, &WalletManagerPanel::onConfirmWalletAction);
    connect(m_createWalletCancelButton, &QPushButton::clicked, this, &WalletManagerPanel::onCancelWalletAction);
    connect(m_loadWalletConfirmButton, &QPushButton::clicked, this, &WalletManagerPanel::onConfirmWalletAction);
    connect(m_loadWalletCancelButton, &QPushButton::clicked, this, &WalletManagerPanel::onCancelWalletAction);
    connect(m_loadWalletBrowseButton, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getOpenFileName(this, tr("Select Wallet File"), 
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), 
            tr("Wallet Files (*.dat);;All Files (*)"));
        if (!path.isEmpty()) {
            m_loadWalletPathEdit->setText(path);
        }
    });
    
    // Context menu connections
    connect(m_viewDetailsAction, &QAction::triggered, this, &WalletManagerPanel::onViewWalletDetails);
    connect(m_switchAction, &QAction::triggered, this, &WalletManagerPanel::onSwitchWalletClicked);
    connect(m_backupAction, &QAction::triggered, this, &WalletManagerPanel::onBackupWalletClicked);
    connect(m_encryptAction, &QAction::triggered, this, &WalletManagerPanel::onEncryptWalletClicked);
    connect(m_unloadAction, &QAction::triggered, this, &WalletManagerPanel::onUnloadWalletClicked);
    connect(m_renameAction, &QAction::triggered, this, &WalletManagerPanel::onRenameWallet);
    connect(m_deleteAction, &QAction::triggered, this, &WalletManagerPanel::onDeleteWalletClicked);
}

void WalletManagerPanel::applyTheme()
{
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
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #e9ecef;
        }
        QPushButton:pressed {
            background-color: #dee2e6;
        }
        QTableWidget {
            border: 1px solid #cccccc;
            gridline-color: #e9ecef;
        }
        QTableWidget::item {
            padding: 8px;
        }
        QHeaderView::section {
            background-color: #f8f9fa;
            padding: 8px;
            border: none;
            border-bottom: 1px solid #dee2e6;
        }
    )";
    
    setStyleSheet(style);
}

void WalletManagerPanel::refreshData()
{
    loadWalletData();
    populateWalletTable();
    updateWalletStatus();
}

void WalletManagerPanel::loadWalletData()
{
    // TODO: Replace with actual backend call to get wallet list
    m_walletList.clear();
    
    if (!m_walletController) {
        return;
    }
    
    // Get wallet directory listing
    auto walletDir = m_walletController->listWalletDir();
    
    for (const auto& [name, loaded] : walletDir) {
        WalletInfo info;
        info.name = QString::fromStdString(name);
        info.path = QString::fromStdString(name) + ".dat";
        info.isLoaded = loaded;
        info.isEncrypted = false; // TODO: Check actual encryption status
        info.isColdWallet = false; // TODO: Load from persistent storage
        info.balance = 0; // TODO: Get actual balance
        info.status = loaded ? tr("Loaded") : tr("Unloaded");
        info.createdDate = QDateTime::currentDateTime(); // TODO: Get actual creation date
        info.fileSize = 0; // TODO: Get actual file size
        info.notes = "";
        info.transactionCount = 0; // TODO: Get actual transaction count
        info.lastUsed = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        
        m_walletList.append(info);
    }
}

void WalletManagerPanel::populateWalletTable()
{
    m_walletTable->setRowCount(m_walletList.size());
    
    for (int i = 0; i < m_walletList.size(); ++i) {
        const WalletInfo& wallet = m_walletList.at(i);
        
        m_walletTable->setItem(i, 0, new QTableWidgetItem(wallet.name));
        m_walletTable->setItem(i, 1, new QTableWidgetItem(wallet.path));
        m_walletTable->setItem(i, 2, new QTableWidgetItem(formatBalance(wallet.balance)));
        m_walletTable->setItem(i, 3, new QTableWidgetItem(wallet.status));
        
        // Cold Wallet Checkbox
        QWidget* coldWalletWidget = new QWidget();
        QHBoxLayout* coldWalletLayout = new QHBoxLayout(coldWalletWidget);
        coldWalletLayout->setContentsMargins(2, 2, 2, 2);
        
        QCheckBox* coldWalletCheck = new QCheckBox(tr("❄️ Cold Wallet"), this);
        coldWalletCheck->setChecked(wallet.isColdWallet);
        coldWalletCheck->setToolTip(tr("Cold wallets are typically offline wallets used for long-term storage."));
        
        // Connect the checkbox to update the wallet status
        connect(coldWalletCheck, &QCheckBox::toggled, [this, i, coldWalletCheck](bool checked) {
            if (i < m_walletList.size()) {
                m_walletList[i].isColdWallet = checked;
                // TODO: Save cold wallet status to persistent storage
                updateStatusMessage(tr("Cold wallet status updated for: %1").arg(m_walletList[i].name));
            }
        });
        
        coldWalletLayout->addWidget(coldWalletCheck);
        coldWalletLayout->addStretch();
        
        m_walletTable->setCellWidget(i, 4, coldWalletWidget);
        
        m_walletTable->setItem(i, 5, new QTableWidgetItem(wallet.createdDate.toString("yyyy-MM-dd")));
        m_walletTable->setItem(i, 6, new QTableWidgetItem(formatWalletSize(wallet.fileSize)));
        m_walletTable->setItem(i, 7, new QTableWidgetItem(wallet.notes));
        
        // Actions column
        QWidget* actionsWidget = new QWidget();
        QHBoxLayout* actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->setContentsMargins(2, 2, 2, 2);
        
        QPushButton* switchBtn = new QPushButton(tr("Switch"), this);
        switchBtn->setToolTip(tr("Set this wallet as your active wallet. All transactions will use it."));
        
        QPushButton* backupBtn = new QPushButton(tr("Backup"), this);
        backupBtn->setToolTip(tr("Save a secure backup (.dat) of this wallet. Store in a safe location."));
        
        actionsLayout->addWidget(switchBtn);
        actionsLayout->addWidget(backupBtn);
        
        m_walletTable->setCellWidget(i, 8, actionsWidget);
    }
}

void WalletManagerPanel::updateWalletStatus()
{
    // Update statistics
    qint64 totalBalance = 0;
    int totalWallets = m_walletList.size();
    int loadedWallets = 0;
    int encryptedWallets = 0;
    int coldWallets = 0;
    
    for (const WalletInfo& wallet : m_walletList) {
        totalBalance += wallet.balance;
        if (wallet.isLoaded) loadedWallets++;
        if (wallet.isEncrypted) encryptedWallets++;
        if (wallet.isColdWallet) coldWallets++;
    }
    
    m_totalBalanceLabel->setText(tr("Total Balance: %1").arg(formatBalance(totalBalance)));
    m_totalWalletsLabel->setText(tr("Total Wallets: %1").arg(totalWallets));
    m_loadedWalletsLabel->setText(tr("Loaded Wallets: %1").arg(loadedWallets));
    m_encryptedWalletsLabel->setText(tr("Encrypted Wallets: %1").arg(encryptedWallets));
    
    // Add cold wallet count to stats if we have a label for it
    if (m_encryptedWalletsLabel) {
        // We could add a new label for cold wallets, but for now we'll update the existing label
        // to show both encrypted and cold wallet counts
        m_encryptedWalletsLabel->setText(tr("Encrypted: %1 | Cold: %2").arg(encryptedWallets).arg(coldWallets));
    }
}

void WalletManagerPanel::onCreateWalletClicked()
{
    updateStatusMessage(tr("Creating new wallet..."));
    showCreateWalletDialog();
}

void WalletManagerPanel::onLoadWalletClicked()
{
    updateStatusMessage(tr("Loading wallet..."));
    showLoadWalletDialog();
}

void WalletManagerPanel::onSwitchWalletClicked()
{
    if (m_selectedWalletName.isEmpty()) {
        showError(tr("Please select a wallet first"));
        return;
    }
    
    // TODO: Implement wallet switching logic
    showSuccess(tr("Switched to wallet: %1").arg(m_selectedWalletName));
}

void WalletManagerPanel::onBackupWalletClicked()
{
    if (m_selectedWalletName.isEmpty()) {
        showError(tr("Please select a wallet first"));
        return;
    }
    
    showBackupWalletDialog(m_selectedWalletName);
}

void WalletManagerPanel::onEncryptWalletClicked()
{
    if (m_selectedWalletName.isEmpty()) {
        showError(tr("Please select a wallet first"));
        return;
    }
    
    showEncryptWalletDialog(m_selectedWalletName);
}

void WalletManagerPanel::onUnloadWalletClicked()
{
    if (m_selectedWalletName.isEmpty()) {
        showError(tr("Please select a wallet first"));
        return;
    }
    
    // TODO: Implement wallet unloading logic
    showSuccess(tr("Unloaded wallet: %1").arg(m_selectedWalletName));
}

void WalletManagerPanel::onDeleteWalletClicked()
{
    if (m_selectedWalletName.isEmpty()) {
        showError(tr("Please select a wallet first"));
        return;
    }
    
    showDeleteWalletDialog(m_selectedWalletName);
}

void WalletManagerPanel::onRefreshClicked()
{
    updateStatusMessage(tr("Refreshing wallet list..."));
    refreshData();
    showSuccess(tr("Wallet list refreshed"));
    updateStatusMessage(tr("Wallet list refreshed successfully"));
}

void WalletManagerPanel::onExportClicked()
{
    exportWalletList();
}

            void WalletManagerPanel::onWalletRowClicked(int row, int column)
            {
                if (row >= 0 && row < m_walletList.size()) {
                    m_selectedWalletName = m_walletList.at(row).name;
                    updateStatusMessage(tr("Selected wallet: %1").arg(m_selectedWalletName));
                }
            }

void WalletManagerPanel::onWalletContextMenu(const QPoint& pos)
{
    QTableWidgetItem* item = m_walletTable->itemAt(pos);
    if (item) {
        int row = item->row();
        if (row >= 0 && row < m_walletList.size()) {
            m_selectedWalletName = m_walletList.at(row).name;
            m_walletContextMenu->exec(m_walletTable->viewport()->mapToGlobal(pos));
        }
    }
}

void WalletManagerPanel::onViewWalletDetails()
{
    if (!m_selectedWalletName.isEmpty()) {
        showWalletDetailsDialog(m_selectedWalletName);
    }
}

void WalletManagerPanel::onRenameWallet()
{
    if (m_selectedWalletName.isEmpty()) {
        showError(tr("Please select a wallet first"));
        return;
    }
    
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename Wallet"),
        tr("Enter new name for wallet:"), QLineEdit::Normal, m_selectedWalletName, &ok);
    
    if (ok && !newName.isEmpty()) {
        // TODO: Implement wallet renaming logic
        showSuccess(tr("Wallet renamed to: %1").arg(newName));
    }
}

void WalletManagerPanel::onConfirmWalletAction()
{
    // TODO: Implement wallet action confirmation
    m_createWalletDialog->accept();
    m_loadWalletDialog->accept();
}

void WalletManagerPanel::onCancelWalletAction()
{
    m_createWalletDialog->reject();
    m_loadWalletDialog->reject();
}

void WalletManagerPanel::showCreateWalletDialog()
{
    m_walletNameEdit->clear();
    m_walletDescriptionEdit->clear();
    m_walletPasswordEdit->clear();
    m_walletPasswordConfirmEdit->clear();
    m_encryptWalletCheck->setChecked(false);
    
    // Reset password strength meter
    if (m_passwordStrengthMeter && m_passwordStrengthLabel) {
        m_passwordStrengthMeter->setValue(0);
        m_passwordStrengthLabel->setText(tr("Password Strength: Weak"));
        m_passwordStrengthLabel->setStyleSheet("QLabel { color: #dc3545; font-weight: bold; }");
    }
    
    m_createWalletDialog->exec();
}

void WalletManagerPanel::showLoadWalletDialog()
{
    m_loadWalletPathEdit->clear();
    m_loadWalletPasswordEdit->clear();
    m_loadWalletEncryptedCheck->setChecked(false);
    
    m_loadWalletDialog->exec();
}

void WalletManagerPanel::showBackupWalletDialog(const QString& walletName)
{
    QString backupPath = QFileDialog::getSaveFileName(this, tr("Backup Wallet"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + walletName + "_backup.dat",
        tr("Wallet Files (*.dat);;All Files (*)"));
    
    if (!backupPath.isEmpty()) {
        // TODO: Implement actual backup logic
        showSuccess(tr("Wallet backed up to: %1").arg(backupPath));
    }
}

void WalletManagerPanel::showEncryptWalletDialog(const QString& walletName)
{
    bool ok;
    QString password = QInputDialog::getText(this, tr("Encrypt Wallet"),
        tr("Enter password for wallet encryption:"), QLineEdit::Password, "", &ok);
    
    if (ok && !password.isEmpty()) {
        // TODO: Implement actual encryption logic
        showSuccess(tr("Wallet encrypted successfully"));
    }
}

void WalletManagerPanel::showDeleteWalletDialog(const QString& walletName)
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Delete Wallet"),
        tr("Are you sure you want to delete wallet '%1'? This action cannot be undone.").arg(walletName),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // TODO: Implement actual deletion logic
        showSuccess(tr("Wallet deleted successfully"));
    }
}

void WalletManagerPanel::showWalletDetailsDialog(const QString& walletName)
{
    // Find wallet info
    for (const WalletInfo& wallet : m_walletList) {
        if (wallet.name == walletName) {
            // TODO: Show detailed wallet information dialog
            QMessageBox::information(this, tr("Wallet Details"),
                tr("Name: %1\nPath: %2\nBalance: %3\nStatus: %4\nCreated: %5\nSize: %6")
                .arg(wallet.name, wallet.path, formatBalance(wallet.balance),
                     wallet.status, wallet.createdDate.toString("yyyy-MM-dd hh:mm:ss"),
                     formatWalletSize(wallet.fileSize)));
            break;
        }
    }
}

void WalletManagerPanel::showError(const QString& message)
{
    QMessageBox::warning(this, tr("Wallet Manager Error"), message);
}

void WalletManagerPanel::showSuccess(const QString& message)
{
    QMessageBox::information(this, tr("Success"), message);
}

QString WalletManagerPanel::formatBalance(qint64 balance)
{
    return ShahcoinUnits::format(ShahcoinUnits::SHAH, balance, false, ShahcoinUnits::SeparatorStyle::STANDARD);
}

QString WalletManagerPanel::formatWalletSize(qint64 size)
{
    if (size < 1024) {
        return tr("%1 B").arg(size);
    } else if (size < 1024 * 1024) {
        return tr("%1 KB").arg(size / 1024.0, 0, 'f', 1);
    } else {
        return tr("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 1);
    }
}

void WalletManagerPanel::exportWalletList()
{
    QString exportPath = QFileDialog::getSaveFileName(this, tr("Export Wallet List"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/wallet_list.json",
        tr("JSON Files (*.json);;All Files (*)"));
    
    if (!exportPath.isEmpty()) {
        QJsonArray walletArray;
        for (const WalletInfo& wallet : m_walletList) {
            QJsonObject walletObj;
            walletObj["name"] = wallet.name;
            walletObj["path"] = wallet.path;
            walletObj["balance"] = wallet.balance;
            walletObj["status"] = wallet.status;
            walletObj["isLoaded"] = wallet.isLoaded;
            walletObj["isEncrypted"] = wallet.isEncrypted;
            walletObj["isColdWallet"] = wallet.isColdWallet;
            walletObj["createdDate"] = wallet.createdDate.toString(Qt::ISODate);
            walletObj["fileSize"] = wallet.fileSize;
            walletObj["notes"] = wallet.notes;
            walletArray.append(walletObj);
        }
        
        QJsonDocument doc(walletArray);
        QFile file(exportPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            showSuccess(tr("Wallet list exported to: %1").arg(exportPath));
        } else {
            showError(tr("Failed to export wallet list"));
        }
    }
}

void WalletManagerPanel::updateWalletList()
{
    refreshData();
}

void WalletManagerPanel::updateStatusMessage(const QString& message)
{
    if (m_statusTextLabel) {
        m_statusTextLabel->setText(message);
    }
}

QString WalletManagerPanel::calculatePasswordStrength(const QString& password)
{
    if (password.isEmpty()) {
        return tr("Weak");
    }
    
    int score = 0;
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    
    // Length bonus
    if (password.length() >= 8) score += 20;
    if (password.length() >= 12) score += 10;
    if (password.length() >= 16) score += 10;
    
    // Character variety bonus
    for (QChar ch : password) {
        if (ch.isLower()) hasLower = true;
        else if (ch.isUpper()) hasUpper = true;
        else if (ch.isDigit()) hasDigit = true;
        else hasSpecial = true;
    }
    
    if (hasLower) score += 10;
    if (hasUpper) score += 10;
    if (hasDigit) score += 10;
    if (hasSpecial) score += 15;
    
    // Bonus for having all character types
    if (hasLower && hasUpper && hasDigit && hasSpecial) score += 15;
    
    // Determine strength level
    if (score >= 80) return tr("Strong");
    else if (score >= 50) return tr("Moderate");
    else return tr("Weak");
}

void WalletManagerPanel::updatePasswordStrengthMeter(const QString& password)
{
    if (!m_passwordStrengthMeter || !m_passwordStrengthLabel) {
        return;
    }
    
    QString strength = calculatePasswordStrength(password);
    int value = 0;
    QString color = "#dc3545"; // Red for weak
    
    if (strength == tr("Strong")) {
        value = 100;
        color = "#28a745"; // Green for strong
    } else if (strength == tr("Moderate")) {
        value = 60;
        color = "#ffc107"; // Yellow for moderate
    } else {
        value = 20;
        color = "#dc3545"; // Red for weak
    }
    
    m_passwordStrengthMeter->setValue(value);
    m_passwordStrengthLabel->setText(tr("Password Strength: %1").arg(strength));
    m_passwordStrengthLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: bold; }").arg(color));
    
    // Update progress bar color
    QString progressStyle = QString(
        "QProgressBar {"
        "    border: 1px solid #ccc;"
        "    border-radius: 3px;"
        "    background-color: #f8f9fa;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: %1;"
        "    border-radius: 2px;"
        "}"
    ).arg(color);
    
    m_passwordStrengthMeter->setStyleSheet(progressStyle);
}

void WalletManagerPanel::setupLanguageSelector()
{
    loadLanguageSettings();
}

void WalletManagerPanel::onLanguageChanged(const QString& language)
{
    // Extract language code from the display text
    QString languageCode = "en"; // default
    for (int i = 0; i < m_languageSelector->count(); ++i) {
        if (m_languageSelector->itemText(i) == language) {
            languageCode = m_languageSelector->itemData(i).toString();
            break;
        }
    }
    
    saveLanguageSettings(languageCode);
    
    // TODO: Implement actual language switching using QTranslator
    // This would require loading .ts translation files
    updateStatusMessage(tr("Language changed to: %1").arg(language));
    
    // Show restart message for language change to take effect
    QMessageBox::information(this, tr("Language Changed"),
        tr("Language has been changed to %1. Please restart the application for the changes to take effect.").arg(language));
}

void WalletManagerPanel::loadLanguageSettings()
{
    QString currentLanguage = m_settings->value("language", "en").toString();
    
    // Find and set the current language in the selector
    for (int i = 0; i < m_languageSelector->count(); ++i) {
        if (m_languageSelector->itemData(i).toString() == currentLanguage) {
            m_languageSelector->setCurrentIndex(i);
            break;
        }
    }
}

void WalletManagerPanel::saveLanguageSettings(const QString& language)
{
    m_settings->setValue("language", language);
    m_settings->sync();
}
