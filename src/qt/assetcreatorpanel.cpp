#include <qt/assetcreatorpanel.h>
#include <qt/walletmodel.h>
#include <qt/platformstyle.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>
#include <interfaces/wallet.h>
#include <util/translation.h>
#include <tokens/token_creation.h> // TODO: Replace with actual token creation backend includes
#include <nfts/nft_creation.h> // TODO: Replace with actual NFT creation backend includes
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

AssetCreatorPanel::AssetCreatorPanel(const PlatformStyle* platformStyle, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(nullptr)
    , m_platformStyle(platformStyle)
    , m_feeCalculationTimer(new QTimer(this))
    , m_tokenSupply(1000000)
    , m_tokenDecimals(8)
    , m_tokenCreationFee(10000000000) // 100 SHAH default
    , m_nftCreationFee(1000000000) // 10 SHAH default
    , m_currentBalance(0)
    , m_shahUsdPrice(1.0) // TODO: Get from price oracle
    , m_settings(new QSettings("Shahcoin", "AssetCreator", this))
{
    setupUI();
    connectSignals();
    applyTheme();
    
    // Set up fee calculation timer (every 60 seconds)
    connect(m_feeCalculationTimer, &QTimer::timeout, this, &AssetCreatorPanel::onFeeCalculationTimer);
    m_feeCalculationTimer->start(60000); // 60 seconds
    
    // Load initial data
    loadCollections();
    refreshData();
}

AssetCreatorPanel::~AssetCreatorPanel()
{
    // Cleanup handled by Qt parent-child system
}

void AssetCreatorPanel::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    refreshData();
}

void AssetCreatorPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);
    m_tabWidget->setDocumentMode(true);
    
    setupTokenMintingTab();
    setupNFTMintingTab();
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Create confirmation dialogs
    setupTokenConfirmationDialog();
    setupNFTConfirmationDialog();
}

void AssetCreatorPanel::setupTokenMintingTab()
{
    m_tokenTab = new QWidget();
    m_tokenLayout = new QVBoxLayout(m_tokenTab);
    m_tokenLayout->setContentsMargins(0, 0, 0, 0);
    m_tokenLayout->setSpacing(15);
    
    // Token creation group
    m_tokenGroup = new QGroupBox(tr("🪙 Create New Token"));
    m_tokenForm = new QFormLayout(m_tokenGroup);
    m_tokenForm->setSpacing(10);
    
    // Token Name
    m_tokenNameEdit = new QLineEdit();
    m_tokenNameEdit->setPlaceholderText(tr("Enter token name (e.g., My Awesome Token)"));
    m_tokenNameEdit->setMaxLength(50);
    m_tokenForm->addRow(tr("Token Name:"), m_tokenNameEdit);
    
    // Token Symbol
    m_tokenSymbolEdit = new QLineEdit();
    m_tokenSymbolEdit->setPlaceholderText(tr("Enter symbol (e.g., MAT)"));
    m_tokenSymbolEdit->setMaxLength(10);
    m_tokenForm->addRow(tr("Token Symbol:"), m_tokenSymbolEdit);
    
    // Total Supply
    m_tokenSupplySpinBox = new QSpinBox();
    m_tokenSupplySpinBox->setRange(1, 999999999999);
    m_tokenSupplySpinBox->setValue(1000000);
    m_tokenSupplySpinBox->setSuffix(tr(" units"));
    m_tokenForm->addRow(tr("Total Supply:"), m_tokenSupplySpinBox);
    
    // Decimal Precision
    m_tokenDecimalsSpinBox = new QSpinBox();
    m_tokenDecimalsSpinBox->setRange(0, 18);
    m_tokenDecimalsSpinBox->setValue(8);
    m_tokenDecimalsSpinBox->setSuffix(tr(" decimals"));
    m_tokenForm->addRow(tr("Decimal Precision:"), m_tokenDecimalsSpinBox);
    
    // Description
    m_tokenDescriptionEdit = new QTextEdit();
    m_tokenDescriptionEdit->setPlaceholderText(tr("Enter token description (optional)"));
    m_tokenDescriptionEdit->setMaximumHeight(80);
    m_tokenForm->addRow(tr("Description:"), m_tokenDescriptionEdit);
    
    // Logo Upload
    QHBoxLayout* logoLayout = new QHBoxLayout();
    m_tokenLogoPathEdit = new QLineEdit();
    m_tokenLogoPathEdit->setPlaceholderText(tr("Select logo image (optional)"));
    m_tokenLogoPathEdit->setReadOnly(true);
    m_tokenLogoUploadButton = new QPushButton(tr("Browse"));
    logoLayout->addWidget(m_tokenLogoPathEdit);
    logoLayout->addWidget(m_tokenLogoUploadButton);
    m_tokenForm->addRow(tr("Logo:"), logoLayout);
    
    // Fee and Balance info
    m_tokenCreationFeeLabel = new QLabel();
    m_tokenCreationFeeLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
    m_tokenForm->addRow(tr("Creation Fee:"), m_tokenCreationFeeLabel);
    
    m_tokenBalanceLabel = new QLabel();
    m_tokenBalanceLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
    m_tokenForm->addRow(tr("Your Balance:"), m_tokenBalanceLabel);
    
    m_tokenLayout->addWidget(m_tokenGroup);
    
    // Buttons
    m_tokenButtonLayout = new QHBoxLayout();
    m_createTokenButton = new QPushButton(tr("🪙 Create Token"));
    m_createTokenButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; padding: 10px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #2980b9; } QPushButton:disabled { background-color: #bdc3c7; }");
    m_clearTokenButton = new QPushButton(tr("Clear Form"));
    m_clearTokenButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; border: none; padding: 10px; border-radius: 5px; } QPushButton:hover { background-color: #7f8c8d; }");
    
    m_tokenButtonLayout->addWidget(m_createTokenButton);
    m_tokenButtonLayout->addWidget(m_clearTokenButton);
    m_tokenButtonLayout->addStretch();
    
    m_tokenLayout->addLayout(m_tokenButtonLayout);
    m_tokenLayout->addStretch();
    
    m_tabWidget->addTab(m_tokenTab, tr("🪙 Token Minting"));
}

void AssetCreatorPanel::setupNFTMintingTab()
{
    m_nftTab = new QWidget();
    m_nftLayout = new QVBoxLayout(m_nftTab);
    m_nftLayout->setContentsMargins(0, 0, 0, 0);
    m_nftLayout->setSpacing(15);
    
    // NFT creation group
    m_nftGroup = new QGroupBox(tr("🖼️ Mint New NFT"));
    m_nftForm = new QFormLayout(m_nftGroup);
    m_nftForm->setSpacing(10);
    
    // NFT Name
    m_nftNameEdit = new QLineEdit();
    m_nftNameEdit->setPlaceholderText(tr("Enter NFT name"));
    m_nftNameEdit->setMaxLength(100);
    m_nftForm->addRow(tr("NFT Name:"), m_nftNameEdit);
    
    // Description
    m_nftDescriptionEdit = new QTextEdit();
    m_nftDescriptionEdit->setPlaceholderText(tr("Enter NFT description"));
    m_nftDescriptionEdit->setMaximumHeight(80);
    m_nftForm->addRow(tr("Description:"), m_nftDescriptionEdit);
    
    // Image Upload
    QHBoxLayout* imageLayout = new QHBoxLayout();
    m_nftImagePathEdit = new QLineEdit();
    m_nftImagePathEdit->setPlaceholderText(tr("Select image file"));
    m_nftImagePathEdit->setReadOnly(true);
    m_nftImageUploadButton = new QPushButton(tr("Browse"));
    imageLayout->addWidget(m_nftImagePathEdit);
    imageLayout->addWidget(m_nftImageUploadButton);
    m_nftForm->addRow(tr("Image:"), imageLayout);
    
    // Collection
    m_nftCollectionCombo = new QComboBox();
    m_nftCollectionCombo->addItem(tr("No Collection"), "");
    m_nftForm->addRow(tr("Collection:"), m_nftCollectionCombo);
    
    // Attributes
    m_nftAttributesEdit = new QTextEdit();
    m_nftAttributesEdit->setPlaceholderText(tr("Enter attributes as JSON (optional)\nExample: {\"rarity\": \"legendary\", \"power\": 100}"));
    m_nftAttributesEdit->setMaximumHeight(80);
    m_nftForm->addRow(tr("Attributes:"), m_nftAttributesEdit);
    
    // Fee and Balance info
    m_nftCreationFeeLabel = new QLabel();
    m_nftCreationFeeLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
    m_nftForm->addRow(tr("Minting Fee:"), m_nftCreationFeeLabel);
    
    m_nftBalanceLabel = new QLabel();
    m_nftBalanceLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
    m_nftForm->addRow(tr("Your Balance:"), m_nftBalanceLabel);
    
    m_nftLayout->addWidget(m_nftGroup);
    
    // Buttons
    m_nftButtonLayout = new QHBoxLayout();
    m_mintNFTButton = new QPushButton(tr("🖼️ Mint NFT"));
    m_mintNFTButton->setStyleSheet("QPushButton { background-color: #9b59b6; color: white; border: none; padding: 10px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #8e44ad; } QPushButton:disabled { background-color: #bdc3c7; }");
    m_clearNFTButton = new QPushButton(tr("Clear Form"));
    m_clearNFTButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; border: none; padding: 10px; border-radius: 5px; } QPushButton:hover { background-color: #7f8c8d; }");
    
    m_nftButtonLayout->addWidget(m_mintNFTButton);
    m_nftButtonLayout->addWidget(m_clearNFTButton);
    m_nftButtonLayout->addStretch();
    
    m_nftLayout->addLayout(m_nftButtonLayout);
    m_nftLayout->addStretch();
    
    m_tabWidget->addTab(m_nftTab, tr("🖼️ NFT Minting"));
}

void AssetCreatorPanel::setupTokenConfirmationDialog()
{
    m_tokenConfirmDialog = new QDialog(this);
    m_tokenConfirmDialog->setWindowTitle(tr("Confirm Token Creation"));
    m_tokenConfirmDialog->setModal(true);
    m_tokenConfirmDialog->setFixedSize(500, 400);
    
    m_tokenConfirmLayout = new QVBoxLayout(m_tokenConfirmDialog);
    m_tokenConfirmLayout->setSpacing(15);
    
    m_tokenConfirmTitleLabel = new QLabel(tr("🪙 Confirm Token Creation"));
    m_tokenConfirmTitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    m_tokenConfirmLayout->addWidget(m_tokenConfirmTitleLabel);
    
    m_tokenConfirmDetailsLabel = new QLabel(tr("Please review the token details:"));
    m_tokenConfirmLayout->addWidget(m_tokenConfirmDetailsLabel);
    
    m_tokenConfirmDetailsText = new QTextEdit();
    m_tokenConfirmDetailsText->setReadOnly(true);
    m_tokenConfirmLayout->addWidget(m_tokenConfirmDetailsText);
    
    m_tokenConfirmButtonLayout = new QHBoxLayout();
    m_tokenConfirmButton = new QPushButton(tr("✅ Create Token"));
    m_tokenConfirmButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; border: none; padding: 10px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #229954; }");
    m_tokenCancelButton = new QPushButton(tr("❌ Cancel"));
    m_tokenCancelButton->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; border: none; padding: 10px; border-radius: 5px; } QPushButton:hover { background-color: #c0392b; }");
    
    m_tokenConfirmButtonLayout->addWidget(m_tokenConfirmButton);
    m_tokenConfirmButtonLayout->addWidget(m_tokenCancelButton);
    m_tokenConfirmLayout->addLayout(m_tokenConfirmButtonLayout);
}

void AssetCreatorPanel::setupNFTConfirmationDialog()
{
    m_nftConfirmDialog = new QDialog(this);
    m_nftConfirmDialog->setWindowTitle(tr("Confirm NFT Minting"));
    m_nftConfirmDialog->setModal(true);
    m_nftConfirmDialog->setFixedSize(500, 400);
    
    m_nftConfirmLayout = new QVBoxLayout(m_nftConfirmDialog);
    m_nftConfirmLayout->setSpacing(15);
    
    m_nftConfirmTitleLabel = new QLabel(tr("🖼️ Confirm NFT Minting"));
    m_nftConfirmTitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    m_nftConfirmLayout->addWidget(m_nftConfirmTitleLabel);
    
    m_nftConfirmDetailsLabel = new QLabel(tr("Please review the NFT details:"));
    m_nftConfirmLayout->addWidget(m_nftConfirmDetailsLabel);
    
    m_nftConfirmDetailsText = new QTextEdit();
    m_nftConfirmDetailsText->setReadOnly(true);
    m_nftConfirmLayout->addWidget(m_nftConfirmDetailsText);
    
    m_nftConfirmButtonLayout = new QHBoxLayout();
    m_nftConfirmButton = new QPushButton(tr("✅ Mint NFT"));
    m_nftConfirmButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; border: none; padding: 10px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #229954; }");
    m_nftCancelButton = new QPushButton(tr("❌ Cancel"));
    m_nftCancelButton->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; border: none; padding: 10px; border-radius: 5px; } QPushButton:hover { background-color: #c0392b; }");
    
    m_nftConfirmButtonLayout->addWidget(m_nftConfirmButton);
    m_nftConfirmButtonLayout->addWidget(m_nftCancelButton);
    m_nftConfirmLayout->addLayout(m_nftConfirmButtonLayout);
}

void AssetCreatorPanel::connectSignals()
{
    // Token creation signals
    connect(m_tokenNameEdit, &QLineEdit::textChanged, this, &AssetCreatorPanel::onTokenNameChanged);
    connect(m_tokenSymbolEdit, &QLineEdit::textChanged, this, &AssetCreatorPanel::onTokenSymbolChanged);
    connect(m_tokenSupplySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AssetCreatorPanel::onTokenSupplyChanged);
    connect(m_tokenDecimalsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AssetCreatorPanel::onTokenDecimalsChanged);
    connect(m_tokenDescriptionEdit, &QTextEdit::textChanged, this, &AssetCreatorPanel::onTokenDescriptionChanged);
    connect(m_tokenLogoUploadButton, &QPushButton::clicked, this, &AssetCreatorPanel::onTokenLogoUploadClicked);
    connect(m_createTokenButton, &QPushButton::clicked, this, &AssetCreatorPanel::onCreateTokenClicked);
    connect(m_clearTokenButton, &QPushButton::clicked, this, &AssetCreatorPanel::onClearTokenFormClicked);
    
    // NFT creation signals
    connect(m_nftNameEdit, &QLineEdit::textChanged, this, &AssetCreatorPanel::onNFTNameChanged);
    connect(m_nftDescriptionEdit, &QTextEdit::textChanged, this, &AssetCreatorPanel::onNFTDescriptionChanged);
    connect(m_nftImageUploadButton, &QPushButton::clicked, this, &AssetCreatorPanel::onNFTImageUploadClicked);
    connect(m_nftCollectionCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged), this, &AssetCreatorPanel::onNFTCollectionChanged);
    connect(m_nftAttributesEdit, &QTextEdit::textChanged, this, &AssetCreatorPanel::onNFTAttributesChanged);
    connect(m_mintNFTButton, &QPushButton::clicked, this, &AssetCreatorPanel::onMintNFTClicked);
    connect(m_clearNFTButton, &QPushButton::clicked, this, &AssetCreatorPanel::onClearNFTFormClicked);
    
    // Shared signals
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &AssetCreatorPanel::onTabChanged);
    connect(m_tokenConfirmButton, &QPushButton::clicked, this, &AssetCreatorPanel::onConfirmTokenCreation);
    connect(m_tokenCancelButton, &QPushButton::clicked, this, &AssetCreatorPanel::onCancelTokenCreation);
    connect(m_nftConfirmButton, &QPushButton::clicked, this, &AssetCreatorPanel::onConfirmNFTCreation);
    connect(m_nftCancelButton, &QPushButton::clicked, this, &AssetCreatorPanel::onCancelNFTCreation);
}

void AssetCreatorPanel::applyTheme()
{
    // Apply consistent styling
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #bdc3c7;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #2c3e50;
        }
        QLineEdit, QTextEdit, QSpinBox, QComboBox {
            border: 2px solid #bdc3c7;
            border-radius: 5px;
            padding: 8px;
            background-color: white;
        }
        QLineEdit:focus, QTextEdit:focus, QSpinBox:focus, QComboBox:focus {
            border-color: #3498db;
        }
        QTabWidget::pane {
            border: 1px solid #bdc3c7;
            border-radius: 5px;
        }
        QTabBar::tab {
            background-color: #ecf0f1;
            padding: 10px 20px;
            margin-right: 2px;
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
        }
        QTabBar::tab:selected {
            background-color: #3498db;
            color: white;
        }
    )");
}

void AssetCreatorPanel::refreshData()
{
    refreshBalances();
    calculateTokenCreationFee();
    calculateNFTCreationFee();
}

void AssetCreatorPanel::refreshBalances()
{
    if (!m_walletModel) {
        m_currentBalance = 0;
        m_tokenBalanceLabel->setText(tr("Wallet not connected"));
        m_nftBalanceLabel->setText(tr("Wallet not connected"));
        return;
    }
    
    // TODO: Get actual balance from wallet model
    m_currentBalance = 100000000000; // 1000 SHAH simulated
    
    QString balanceText = formatAmount(m_currentBalance);
    m_tokenBalanceLabel->setText(balanceText);
    m_nftBalanceLabel->setText(balanceText);
}

void AssetCreatorPanel::loadCollections()
{
    // TODO: Load actual collections from backend
    m_collections.clear();
    m_collections << "Art Collection" << "Gaming NFTs" << "Music NFTs" << "Sports NFTs" << "Custom Collection";
    
    m_nftCollectionCombo->clear();
    m_nftCollectionCombo->addItem(tr("No Collection"), "");
    for (const QString& collection : m_collections) {
        m_nftCollectionCombo->addItem(collection, collection);
    }
}

void AssetCreatorPanel::calculateTokenCreationFee()
{
    // TODO: Get dynamic fee from backend based on SHAH/USD price
    m_tokenCreationFee = 10000000000; // 100 SHAH default
    
    QString feeText = formatAmount(m_tokenCreationFee);
    m_tokenCreationFeeLabel->setText(feeText);
}

void AssetCreatorPanel::calculateNFTCreationFee()
{
    // TODO: Get dynamic fee from backend based on SHAH/USD price
    m_nftCreationFee = 1000000000; // 10 SHAH default
    
    QString feeText = formatAmount(m_nftCreationFee);
    m_nftCreationFeeLabel->setText(feeText);
}

void AssetCreatorPanel::validateTokenInputs()
{
    bool isValid = true;
    
    // Validate name
    if (!isValidTokenName(m_tokenName)) {
        m_tokenNameEdit->setStyleSheet("border: 2px solid #e74c3c;");
        isValid = false;
    } else {
        m_tokenNameEdit->setStyleSheet("");
    }
    
    // Validate symbol
    if (!isValidTokenSymbol(m_tokenSymbol)) {
        m_tokenSymbolEdit->setStyleSheet("border: 2px solid #e74c3c;");
        isValid = false;
    } else {
        m_tokenSymbolEdit->setStyleSheet("");
    }
    
    // Check balance
    if (m_currentBalance < m_tokenCreationFee) {
        showError(tr("Insufficient balance for token creation fee"));
        isValid = false;
    }
    
    m_createTokenButton->setEnabled(isValid);
}

void AssetCreatorPanel::validateNFTInputs()
{
    bool isValid = true;
    
    // Validate name
    if (!isValidNFTName(m_nftName)) {
        m_nftNameEdit->setStyleSheet("border: 2px solid #e74c3c;");
        isValid = false;
    } else {
        m_nftNameEdit->setStyleSheet("");
    }
    
    // Check if image is selected
    if (m_nftImagePath.isEmpty()) {
        m_nftImagePathEdit->setStyleSheet("border: 2px solid #e74c3c;");
        isValid = false;
    } else {
        m_nftImagePathEdit->setStyleSheet("");
    }
    
    // Check balance
    if (m_currentBalance < m_nftCreationFee) {
        showError(tr("Insufficient balance for NFT minting fee"));
        isValid = false;
    }
    
    m_mintNFTButton->setEnabled(isValid);
}

void AssetCreatorPanel::showTokenCreationConfirmation()
{
    QString details = QString(
        "Token Name: %1\n"
        "Token Symbol: %2\n"
        "Total Supply: %3\n"
        "Decimal Precision: %4\n"
        "Description: %5\n"
        "Creation Fee: %6\n\n"
        "Are you sure you want to create this token?"
    ).arg(
        m_tokenName,
        m_tokenSymbol,
        QString::number(m_tokenSupply),
        QString::number(m_tokenDecimals),
        m_tokenDescription.isEmpty() ? tr("None") : m_tokenDescription,
        formatAmount(m_tokenCreationFee)
    );
    
    m_tokenConfirmDetailsText->setText(details);
    m_tokenConfirmDialog->exec();
}

void AssetCreatorPanel::showNFTCreationConfirmation()
{
    QString details = QString(
        "NFT Name: %1\n"
        "Description: %2\n"
        "Image: %3\n"
        "Collection: %4\n"
        "Attributes: %5\n"
        "Minting Fee: %6\n\n"
        "Are you sure you want to mint this NFT?"
    ).arg(
        m_nftName,
        m_nftDescription.isEmpty() ? tr("None") : m_nftDescription,
        QFileInfo(m_nftImagePath).fileName(),
        m_nftCollection.isEmpty() ? tr("No Collection") : m_nftCollection,
        m_nftAttributes.isEmpty() ? tr("None") : m_nftAttributes,
        formatAmount(m_nftCreationFee)
    );
    
    m_nftConfirmDetailsText->setText(details);
    m_nftConfirmDialog->exec();
}

void AssetCreatorPanel::executeTokenCreation()
{
    // TODO: Call actual token creation backend
    showSuccess(tr("Token '%1' created successfully! Transaction ID: %2")
        .arg(m_tokenName)
        .arg("abc123def456..."));
    
    // Clear form
    onClearTokenFormClicked();
}

void AssetCreatorPanel::executeNFTCreation()
{
    // TODO: Call actual NFT creation backend
    showSuccess(tr("NFT '%1' minted successfully! Transaction ID: %2")
        .arg(m_nftName)
        .arg("xyz789uvw012..."));
    
    // Clear form
    onClearNFTFormClicked();
}

void AssetCreatorPanel::showError(const QString& message)
{
    QMessageBox::warning(this, tr("Asset Creator - Error"), message);
}

void AssetCreatorPanel::showSuccess(const QString& message)
{
    QMessageBox::information(this, tr("Asset Creator - Success"), message);
}

QString AssetCreatorPanel::encodeImageToBase64(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QByteArray imageData = file.readAll();
    return imageData.toBase64();
}

bool AssetCreatorPanel::isValidTokenName(const QString& name)
{
    return !name.trimmed().isEmpty() && name.length() <= 50;
}

bool AssetCreatorPanel::isValidTokenSymbol(const QString& symbol)
{
    return !symbol.trimmed().isEmpty() && symbol.length() <= 10 && symbol.matches(QRegularExpression("^[A-Z0-9]+$"));
}

bool AssetCreatorPanel::isValidNFTName(const QString& name)
{
    return !name.trimmed().isEmpty() && name.length() <= 100;
}

QString AssetCreatorPanel::formatAmount(qint64 amount)
{
    return ShahcoinUnits::format(ShahcoinUnits::SHAH, amount, false, ShahcoinUnits::separatorAlways);
}

// Token Creation Slots
void AssetCreatorPanel::onTokenNameChanged()
{
    m_tokenName = m_tokenNameEdit->text().trimmed();
    validateTokenInputs();
}

void AssetCreatorPanel::onTokenSymbolChanged()
{
    m_tokenSymbol = m_tokenSymbolEdit->text().trimmed().toUpper();
    validateTokenInputs();
}

void AssetCreatorPanel::onTokenSupplyChanged()
{
    m_tokenSupply = m_tokenSupplySpinBox->value();
}

void AssetCreatorPanel::onTokenDecimalsChanged()
{
    m_tokenDecimals = m_tokenDecimalsSpinBox->value();
}

void AssetCreatorPanel::onTokenDescriptionChanged()
{
    m_tokenDescription = m_tokenDescriptionEdit->toPlainText().trimmed();
}

void AssetCreatorPanel::onTokenLogoUploadClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Token Logo"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        tr("Image Files (*.png *.jpg *.jpeg *.gif *.bmp)")
    );
    
    if (!filePath.isEmpty()) {
        m_tokenLogoPath = filePath;
        m_tokenLogoPathEdit->setText(QFileInfo(filePath).fileName());
    }
}

void AssetCreatorPanel::onCreateTokenClicked()
{
    if (validateTokenInputs()) {
        showTokenCreationConfirmation();
    }
}

void AssetCreatorPanel::onClearTokenFormClicked()
{
    m_tokenNameEdit->clear();
    m_tokenSymbolEdit->clear();
    m_tokenSupplySpinBox->setValue(1000000);
    m_tokenDecimalsSpinBox->setValue(8);
    m_tokenDescriptionEdit->clear();
    m_tokenLogoPathEdit->clear();
    m_tokenLogoPath.clear();
    
    m_tokenName.clear();
    m_tokenSymbol.clear();
    m_tokenSupply = 1000000;
    m_tokenDecimals = 8;
    m_tokenDescription.clear();
    
    validateTokenInputs();
}

// NFT Creation Slots
void AssetCreatorPanel::onNFTNameChanged()
{
    m_nftName = m_nftNameEdit->text().trimmed();
    validateNFTInputs();
}

void AssetCreatorPanel::onNFTDescriptionChanged()
{
    m_nftDescription = m_nftDescriptionEdit->toPlainText().trimmed();
}

void AssetCreatorPanel::onNFTImageUploadClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select NFT Image"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        tr("Image Files (*.png *.jpg *.jpeg *.gif *.bmp)")
    );
    
    if (!filePath.isEmpty()) {
        m_nftImagePath = filePath;
        m_nftImagePathEdit->setText(QFileInfo(filePath).fileName());
        validateNFTInputs();
    }
}

void AssetCreatorPanel::onNFTCollectionChanged()
{
    m_nftCollection = m_nftCollectionCombo->currentData().toString();
}

void AssetCreatorPanel::onNFTAttributesChanged()
{
    m_nftAttributes = m_nftAttributesEdit->toPlainText().trimmed();
}

void AssetCreatorPanel::onMintNFTClicked()
{
    if (validateNFTInputs()) {
        showNFTCreationConfirmation();
    }
}

void AssetCreatorPanel::onClearNFTFormClicked()
{
    m_nftNameEdit->clear();
    m_nftDescriptionEdit->clear();
    m_nftImagePathEdit->clear();
    m_nftCollectionCombo->setCurrentIndex(0);
    m_nftAttributesEdit->clear();
    m_nftImagePath.clear();
    
    m_nftName.clear();
    m_nftDescription.clear();
    m_nftCollection.clear();
    m_nftAttributes.clear();
    
    validateNFTInputs();
}

// Shared Slots
void AssetCreatorPanel::onTabChanged(int index)
{
    // Refresh data when switching tabs
    refreshData();
}

void AssetCreatorPanel::onConfirmTokenCreation()
{
    m_tokenConfirmDialog->accept();
    executeTokenCreation();
}

void AssetCreatorPanel::onCancelTokenCreation()
{
    m_tokenConfirmDialog->reject();
}

void AssetCreatorPanel::onConfirmNFTCreation()
{
    m_nftConfirmDialog->accept();
    executeNFTCreation();
}

void AssetCreatorPanel::onCancelNFTCreation()
{
    m_nftConfirmDialog->reject();
}

void AssetCreatorPanel::onFeeCalculationTimer()
{
    calculateTokenCreationFee();
    calculateNFTCreationFee();
}
