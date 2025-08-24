// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/nftcreatorpanel.h>

#include <qt/walletmodel.h>
#include <qt/platformstyle.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>

#include <interfaces/wallet.h>
#include <util/translation.h>
#include <tokens/nft.h>

#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QValidator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QTimer>
#include <QNetworkRequest>
#include <QUrlQuery>

NFTCreatorPanel::NFTCreatorPanel(const PlatformStyle* platformStyle, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(nullptr)
    , m_platformStyle(platformStyle)
    , m_nameValid(false)
    , m_imageValid(false)
    , m_hasSufficientBalance(false)
    , m_creationFee(100 * COIN) // 100 SHAH default fee
    , m_currentBalance(0)
{
    setupUI();
    connectSignals();
    applyTheme();
    refreshData();
}

NFTCreatorPanel::~NFTCreatorPanel()
{
}

void NFTCreatorPanel::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    refreshData();
}

void NFTCreatorPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(12);

    // NFT Details Group
    m_nftDetailsGroup = new QGroupBox(tr("NFT Details"));
    QGridLayout* nftGrid = new QGridLayout(m_nftDetailsGroup);

    // NFT Name
    nftGrid->addWidget(new QLabel(tr("NFT Name:")), 0, 0);
    m_nftNameEdit = new QLineEdit();
    m_nftNameEdit->setPlaceholderText(tr("Enter NFT name (e.g., My Digital Art #1"));
    m_nftNameEdit->setMaxLength(100);
    nftGrid->addWidget(m_nftNameEdit, 0, 1);

    // Description
    nftGrid->addWidget(new QLabel(tr("Description:")), 1, 0);
    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setPlaceholderText(tr("Enter NFT description (optional)"));
    m_descriptionEdit->setMaximumHeight(80);
    nftGrid->addWidget(m_descriptionEdit, 1, 1);

    m_mainLayout->addWidget(m_nftDetailsGroup);

    // Image Group
    m_imageGroup = new QGroupBox(tr("NFT Image"));
    QHBoxLayout* imageLayout = new QHBoxLayout(m_imageGroup);

    m_imagePreview = new QLabel();
    m_imagePreview->setFixedSize(128, 128);
    m_imagePreview->setStyleSheet("border: 2px dashed #ccc; border-radius: 8px;");
    m_imagePreview->setAlignment(Qt::AlignCenter);
    m_imagePreview->setText(tr("No Image"));

    m_imageUploadButton = new QPushButton(tr("Upload Image"));
    m_imageUploadButton->setIcon(QIcon::fromTheme("document-open"));

    m_imageClearButton = new QPushButton(tr("Clear"));
    m_imageClearButton->setIcon(QIcon::fromTheme("edit-clear"));
    m_imageClearButton->setEnabled(false);

    imageLayout->addWidget(m_imagePreview);
    imageLayout->addWidget(m_imageUploadButton);
    imageLayout->addWidget(m_imageClearButton);
    imageLayout->addStretch();

    m_mainLayout->addWidget(m_imageGroup);

    // Attributes Group
    m_attributesGroup = new QGroupBox(tr("Custom Attributes (Optional)"));
    QVBoxLayout* attributesLayout = new QVBoxLayout(m_attributesGroup);

    // Attributes Table
    m_attributesTable = new QTableWidget();
    m_attributesTable->setColumnCount(2);
    m_attributesTable->setHorizontalHeaderLabels({tr("Attribute"), tr("Value")});
    m_attributesTable->horizontalHeader()->setStretchLastSection(true);
    m_attributesTable->setAlternatingRowColors(true);
    m_attributesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_attributesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_attributesTable->setMaximumHeight(150);

    // Attribute Input Controls
    QHBoxLayout* attributeInputLayout = new QHBoxLayout();
    
    m_attributeKeyEdit = new QLineEdit();
    m_attributeKeyEdit->setPlaceholderText(tr("Attribute name (e.g., Rarity)"));
    m_attributeKeyEdit->setMaxLength(50);
    
    m_attributeValueEdit = new QLineEdit();
    m_attributeValueEdit->setPlaceholderText(tr("Attribute value (e.g., Legendary)"));
    m_attributeValueEdit->setMaxLength(100);
    
    m_addAttributeButton = new QPushButton(tr("Add"));
    m_addAttributeButton->setIcon(QIcon::fromTheme("list-add"));
    
    m_removeAttributeButton = new QPushButton(tr("Remove"));
    m_removeAttributeButton->setIcon(QIcon::fromTheme("list-remove"));
    m_removeAttributeButton->setEnabled(false);

    attributeInputLayout->addWidget(new QLabel(tr("Key:")));
    attributeInputLayout->addWidget(m_attributeKeyEdit);
    attributeInputLayout->addWidget(new QLabel(tr("Value:")));
    attributeInputLayout->addWidget(m_attributeValueEdit);
    attributeInputLayout->addWidget(m_addAttributeButton);
    attributeInputLayout->addWidget(m_removeAttributeButton);

    attributesLayout->addWidget(m_attributesTable);
    attributesLayout->addLayout(attributeInputLayout);

    m_mainLayout->addWidget(m_attributesGroup);

    // Tier & Fee Group
    m_feeGroup = new QGroupBox(tr("Creation Tier (Per NFT) & Balance"));
    QVBoxLayout* feeLayout = new QVBoxLayout(m_feeGroup);

    // Tier selection
    QHBoxLayout* tierRow = new QHBoxLayout();
    tierRow->addWidget(new QLabel(tr("Choose Tier (per NFT):")));
    m_tierCombo = new QComboBox();
    m_tierCombo->addItem(tr("Basic — $15 per NFT"), QVariant("Basic"));
    m_tierCombo->addItem(tr("Pro — $25 per NFT"), QVariant("Pro"));
    m_tierCombo->addItem(tr("Premium — $39 per NFT"), QVariant("Premium"));
    tierRow->addWidget(m_tierCombo);
    feeLayout->addLayout(tierRow);

    // Tier benefits
    m_tierBenefits = new QLabel();
    m_tierBenefits->setWordWrap(true);
    feeLayout->addWidget(m_tierBenefits);

    m_feeLabel = new QLabel();
    m_feeLabel->setStyleSheet("font-weight: bold; color: #2c3e50;");

    m_balanceLabel = new QLabel();
    m_balanceLabel->setStyleSheet("color: #34495e;");

    m_balanceProgress = new QProgressBar();
    m_balanceProgress->setRange(0, 100);
    m_balanceProgress->setFormat(tr("Balance: %p%"));

    feeLayout->addWidget(m_feeLabel);
    feeLayout->addWidget(m_balanceLabel);
    feeLayout->addWidget(m_balanceProgress);

    // Payment method row
    m_paymentLayout = new QHBoxLayout();
    m_paymentLayout->addWidget(new QLabel(tr("Payment Method:")));
    m_paymentMethodCombo = new QComboBox();
    m_paymentMethodCombo->addItem(tr("SHAH (native)"), QVariant("shah"));
    m_paymentMethodCombo->addItem(tr("SHAHtoken (ERC-20)"), QVariant("shahtoken"));
    m_paymentMethodCombo->addItem(tr("Stripe (USD)"), QVariant("stripe"));
    m_paymentLayout->addWidget(m_paymentMethodCombo);
    m_paymentStatusLabel = new QLabel(tr("Payment Status: Not Verified"));
    m_paymentLayout->addWidget(m_paymentStatusLabel);
    feeLayout->addLayout(m_paymentLayout);

    m_mainLayout->addWidget(m_feeGroup);

    // Action Buttons
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();

    m_payVerifyButton = new QPushButton(tr("Pay & Verify"));
    m_payVerifyButton->setIcon(QIcon::fromTheme("emblem-ok"));
    m_mintButton = new QPushButton(tr("Mint NFT"));
    m_mintButton->setIcon(QIcon::fromTheme("document-new"));
    m_mintButton->setEnabled(false);

    m_clearButton = new QPushButton(tr("Clear Form"));
    m_clearButton->setIcon(QIcon::fromTheme("edit-clear"));

    m_buttonLayout->addWidget(m_clearButton);
    m_buttonLayout->addWidget(m_payVerifyButton);
    m_buttonLayout->addWidget(m_mintButton);

    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addStretch();
}

void NFTCreatorPanel::connectSignals()
{
    connect(m_nftNameEdit, &QLineEdit::textChanged, this, &NFTCreatorPanel::onNFTNameChanged);
    
    connect(m_imageUploadButton, &QPushButton::clicked, this, &NFTCreatorPanel::onImageUploadClicked);
    connect(m_tierCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NFTCreatorPanel::onTierChanged);
    connect(m_payVerifyButton, &QPushButton::clicked, this, &NFTCreatorPanel::onPayVerifyClicked);
    connect(m_imageClearButton, &QPushButton::clicked, [this]() {
        m_imageFilePath.clear();
        m_imagePreview->setPixmap(QPixmap());
        m_imagePreview->setText(tr("No Image"));
        m_imageClearButton->setEnabled(false);
        m_imageValid = false;
        updateMintButton();
    });
    
    connect(m_addAttributeButton, &QPushButton::clicked, this, &NFTCreatorPanel::onAddAttributeClicked);
    connect(m_removeAttributeButton, &QPushButton::clicked, this, &NFTCreatorPanel::onRemoveAttributeClicked);
    
    connect(m_attributeKeyEdit, &QLineEdit::textChanged, this, &NFTCreatorPanel::onAttributeChanged);
    connect(m_attributeValueEdit, &QLineEdit::textChanged, this, &NFTCreatorPanel::onAttributeChanged);
    
    connect(m_attributesTable, &QTableWidget::itemSelectionChanged, [this]() {
        m_removeAttributeButton->setEnabled(m_attributesTable->currentRow() >= 0);
    });

    connect(m_paymentMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NFTCreatorPanel::onPaymentMethodChanged);

    connect(m_mintButton, &QPushButton::clicked, this, &NFTCreatorPanel::onMintNFTClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &NFTCreatorPanel::onClearClicked);
}

void NFTCreatorPanel::applyTheme()
{
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #dee2e6;
            border-radius: 6px;
            margin-top: 12px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #495057;
        }
        QLineEdit, QTextEdit {
            border: 1px solid #ced4da;
            border-radius: 4px;
            padding: 6px;
            background-color: #ffffff;
        }
        QLineEdit:focus, QTextEdit:focus {
            border-color: #007bff;
            outline: none;
        }
        QLineEdit:invalid {
            border-color: #dc3545;
            background-color: #fff5f5;
        }
        QPushButton {
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: 500;
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
        QPushButton#clearButton {
            background-color: #6c757d;
        }
        QPushButton#clearButton:hover {
            background-color: #545b62;
        }
        QTableWidget {
            border: 1px solid #dee2e6;
            gridline-color: #dee2e6;
        }
        QTableWidget::item {
            padding: 4px;
        }
        QProgressBar {
            border: 1px solid #dee2e6;
            border-radius: 3px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #28a745;
            border-radius: 2px;
        }
    )");

    m_clearButton->setObjectName("clearButton");
}

void NFTCreatorPanel::refreshData()
{
    if (m_walletModel) {
        m_currentBalance = m_walletModel->getBalance();
        updateFeeDisplay();
        updateTierBenefits();
        updateMintButton();
    }
}

void NFTCreatorPanel::updateFeeDisplay()
{
    QString feeText = ShahcoinUnits::formatWithUnit(
        m_walletModel ? m_walletModel->getOptionsModel()->getDisplayUnit() : ShahcoinUnit::SHAH,
        m_creationFee,
        false,
        ShahcoinUnits::SeparatorStyle::STANDARD
    );
    m_feeLabel->setText(tr("Creation Fee (chain tx): %1").arg(feeText));

    QString balanceText = ShahcoinUnits::formatWithUnit(
        m_walletModel ? m_walletModel->getOptionsModel()->getDisplayUnit() : ShahcoinUnit::SHAH,
        m_currentBalance,
        false,
        ShahcoinUnits::SeparatorStyle::STANDARD
    );
    m_balanceLabel->setText(tr("Your Balance: %1").arg(balanceText));

    // Update balance progress
    m_hasSufficientBalance = m_currentBalance >= m_creationFee;
    if (m_currentBalance > 0) {
        int progress = qMin(100, (int)((double)m_currentBalance / m_creationFee * 100));
        m_balanceProgress->setValue(progress);
        
        if (m_hasSufficientBalance) {
            m_balanceProgress->setStyleSheet("QProgressBar::chunk { background-color: #28a745; }");
        } else {
            m_balanceProgress->setStyleSheet("QProgressBar::chunk { background-color: #dc3545; }");
        }
    } else {
        m_balanceProgress->setValue(0);
        m_balanceProgress->setStyleSheet("QProgressBar::chunk { background-color: #dc3545; }");
    }
}

void NFTCreatorPanel::updateMintButton()
{
    bool canMint = m_nameValid && m_imageValid && m_hasSufficientBalance && m_paymentVerified;
    m_mintButton->setEnabled(canMint);
}

void NFTCreatorPanel::onNFTNameChanged()
{
    QString name = m_nftNameEdit->text().trimmed();
    m_nameValid = name.length() >= 3 && name.length() <= 100;
    
    if (m_nameValid) {
        m_nftNameEdit->setStyleSheet("");
    } else {
        m_nftNameEdit->setStyleSheet("border-color: #dc3545; background-color: #fff5f5;");
    }
    
    updateMintButton();
}

void NFTCreatorPanel::onImageUploadClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select NFT Image"),
        QDir::homePath(),
        tr("Image Files (*.png *.jpg *.jpeg *.gif *.bmp);;All Files (*)")
    );

    if (!filePath.isEmpty()) {
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            // Scale to 128x128 for preview
            QPixmap scaledPixmap = pixmap.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_imagePreview->setPixmap(scaledPixmap);
            m_imageFilePath = filePath;
            m_imageClearButton->setEnabled(true);
            m_imageValid = true;
            updateMintButton();
        } else {
            QMessageBox::warning(this, tr("Invalid Image"), tr("The selected file is not a valid image."));
        }
    }
}

void NFTCreatorPanel::onAddAttributeClicked()
{
    QString key = m_attributeKeyEdit->text().trimmed();
    QString value = m_attributeValueEdit->text().trimmed();
    
    if (key.isEmpty() || value.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Attribute"), 
                           tr("Both attribute key and value must be provided."));
        return;
    }
    
    if (m_attributes.contains(key)) {
        QMessageBox::warning(this, tr("Duplicate Attribute"), 
                           tr("An attribute with this key already exists."));
        return;
    }
    
    m_attributes[key] = value;
    updateAttributesTable();
    
    // Clear input fields
    m_attributeKeyEdit->clear();
    m_attributeValueEdit->clear();
    m_attributeKeyEdit->setFocus();
}

void NFTCreatorPanel::onRemoveAttributeClicked()
{
    int currentRow = m_attributesTable->currentRow();
    if (currentRow >= 0) {
        QString key = m_attributesTable->item(currentRow, 0)->text();
        m_attributes.remove(key);
        updateAttributesTable();
    }
}

void NFTCreatorPanel::onAttributeChanged()
{
    QString key = m_attributeKeyEdit->text().trimmed();
    QString value = m_attributeValueEdit->text().trimmed();
    m_addAttributeButton->setEnabled(!key.isEmpty() && !value.isEmpty());
}

void NFTCreatorPanel::onTierChanged()
{
    updateTierBenefits();
}

void NFTCreatorPanel::setPaymentVerified(bool ok)
{
    m_paymentVerified = ok;
    m_paymentStatusLabel->setText(ok ? tr("Payment Status: Verified ✅") : tr("Payment Status: Not Verified"));
    updateMintButton();
}

void NFTCreatorPanel::onPayVerifyClicked()
{
    startPaymentVerification();
}

void NFTCreatorPanel::onPaymentMethodChanged(int)
{
    // Could update tooltips or hints based on selected method
}

void NFTCreatorPanel::startPaymentVerification()
{
    const QString tier = m_tierCombo ? m_tierCombo->currentData().toString() : QStringLiteral("Basic");
    const QString method = m_paymentMethodCombo ? m_paymentMethodCombo->currentData().toString() : QStringLiteral("shah");

    if (method == "stripe") {
        startStripePayment(tier);
        return;
    }

    // SHAH / SHAHtoken
    requestShahInvoice(tier, method);
}

void NFTCreatorPanel::startStripePayment(const QString& tier)
{
    Q_UNUSED(tier);
    // Placeholder: open Stripe Checkout via Dev Portal
    QMessageBox::information(this, tr("Stripe"), tr("Opening Stripe Checkout in browser... (stub)"));
    // Simulate async verification success
    QTimer::singleShot(800, this, [this](){ setPaymentVerified(true); });
}

void NFTCreatorPanel::requestShahInvoice(const QString& tier, const QString& method)
{
    Q_UNUSED(tier); Q_UNUSED(method);
    // Placeholder: call backend to create invoice for selected method
    // On success, store invoice id and start polling
    m_invoiceId = QString("inv_%1").arg(QDateTime::currentSecsSinceEpoch());
    startInvoicePolling();
}

void NFTCreatorPanel::startInvoicePolling()
{
    connect(&m_invoicePollTimer, &QTimer::timeout, this, &NFTCreatorPanel::pollInvoiceStatus);
    m_invoicePollTimer.start(2000);
}

void NFTCreatorPanel::pollInvoiceStatus()
{
    // Placeholder: query backend for invoice status
    static int ticks = 0; ticks++;
    if (ticks >= 2) {
        m_invoicePollTimer.stop();
        setPaymentVerified(true);
        ticks = 0;
    }
}

void NFTCreatorPanel::updateTierBenefits()
{
    if (!m_tierBenefits || !m_tierCombo) return;
    const QString tier = m_tierCombo->currentData().toString();
    if (tier == "Premium") {
        m_tierBenefits->setText(tr("Premium: includes Pro + smart trait filters, unlockable content, bundle mint, spotlight."));
    } else if (tier == "Pro") {
        m_tierBenefits->setText(tr("Pro: includes Basic + animation/multimedia, verified creator, premium metadata, higher search rank."));
    } else {
        m_tierBenefits->setText(tr("Basic: image upload, name/description, standard mint, visible in explorer/wallet."));
    }
}

void NFTCreatorPanel::onMintNFTClicked()
{
    if (!validateInputs()) {
        return;
    }
    if (!m_paymentVerified) {
        QMessageBox::warning(this, tr("Payment Required"), tr("Please Pay & Verify before minting."));
        return;
    }

    showConfirmationDialog();
}

void NFTCreatorPanel::onClearClicked()
{
    clearForm();
}

bool NFTCreatorPanel::validateInputs()
{
    if (!m_nameValid) {
        QMessageBox::warning(this, tr("Invalid NFT Name"), 
                           tr("NFT name must be 3-100 characters long."));
        return false;
    }

    if (!m_imageValid) {
        QMessageBox::warning(this, tr("No Image"), 
                           tr("Please upload an image for your NFT."));
        return false;
    }

    if (!m_hasSufficientBalance) {
        QMessageBox::warning(this, tr("Insufficient Balance"), 
                           tr("You don't have enough SHAH to pay the creation fee."));
        return false;
    }

    return true;
}

void NFTCreatorPanel::showConfirmationDialog()
{
    QString nftName = m_nftNameEdit->text().trimmed();
    QString description = m_descriptionEdit->toPlainText().trimmed();
    int attributeCount = m_attributes.size();
    QString tier = m_tierCombo ? m_tierCombo->currentData().toString() : QStringLiteral("Basic");

    QString feeText = ShahcoinUnits::formatWithUnit(
        m_walletModel ? m_walletModel->getOptionsModel()->getDisplayUnit() : ShahcoinUnit::SHAH,
        m_creationFee,
        false,
        ShahcoinUnits::SeparatorStyle::STANDARD
    );

    QString message = tr(
        "Please confirm NFT minting:\n\n"
        "NFT Name: %1\n"
        "Description: %2\n"
        "Tier: %3\n"
        "Image: %4\n"
        "Attributes: %5\n"
        "Blockchain TX Fee: %6\n\n"
        "This action cannot be undone. Continue?"
    ).arg(nftName, 
          description.isEmpty() ? tr("None") : description,
          tier,
          m_imageFilePath.isEmpty() ? tr("None") : QFileInfo(m_imageFilePath).fileName(),
          attributeCount == 0 ? tr("None") : tr("%1 attributes").arg(attributeCount),
          feeText);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Confirm NFT Minting"), message,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        mintNFT();
    }
}

void NFTCreatorPanel::mintNFT()
{
    if (!m_walletModel) {
        showErrorDialog(tr("Wallet not available"));
        return;
    }

    QString nftName = m_nftNameEdit->text().trimmed();
    QString description = m_descriptionEdit->toPlainText().trimmed();
    QString imageBase64 = m_imageFilePath.isEmpty() ? "" : encodeImageToBase64(m_imageFilePath);
    QString attributesJSON = generateAttributesJSON();

    try {
        // Call the NFT minting function from WalletModel
        QString nftId = m_walletModel->mintNFT(
            nftName, description, imageBase64, attributesJSON
        );

        if (!nftId.isEmpty()) {
            showSuccessDialog(nftId);
            clearForm();
        } else {
            showErrorDialog(tr("Failed to mint NFT. Please try again."));
        }
    } catch (const std::exception& e) {
        showErrorDialog(tr("Error minting NFT: %1").arg(QString::fromStdString(e.what())));
    }
}

void NFTCreatorPanel::showSuccessDialog(const QString& nftId)
{
    QString message = tr(
        "NFT minted successfully!\n\n"
        "NFT ID: %1\n\n"
        "Your new NFT has been minted and is now available in your wallet. "
        "You can view it in the NFT Explorer panel."
    ).arg(nftId);

    QMessageBox::information(this, tr("NFT Minted"), message);
}

void NFTCreatorPanel::showErrorDialog(const QString& error)
{
    QMessageBox::critical(this, tr("NFT Minting Failed"), error);
}

QString NFTCreatorPanel::encodeImageToBase64(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return "";
    }

    QByteArray imageData = file.readAll();
    return imageData.toBase64();
}

QString NFTCreatorPanel::generateAttributesJSON()
{
    QJsonObject jsonObj;

    // Include tier at top-level metadata for SHI-30
    const QString tier = m_tierCombo ? m_tierCombo->currentData().toString() : QStringLiteral("Basic");
    jsonObj[QStringLiteral("tier")] = tier;

    // User-defined attributes under attributes.{key: value}
    QJsonObject attrs;
    for (auto it = m_attributes.begin(); it != m_attributes.end(); ++it) {
        attrs[it.key()] = it.value();
    }
    if (!attrs.isEmpty()) {
        jsonObj[QStringLiteral("attributes")] = attrs;
    }

    QJsonDocument doc(jsonObj);
    return doc.toJson(QJsonDocument::Compact);
}

void NFTCreatorPanel::updateAttributesTable()
{
    m_attributesTable->setRowCount(m_attributes.size());
    
    int row = 0;
    for (auto it = m_attributes.begin(); it != m_attributes.end(); ++it) {
        m_attributesTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        m_attributesTable->setItem(row, 1, new QTableWidgetItem(it.value()));
        row++;
    }
}

void NFTCreatorPanel::clearForm()
{
    m_nftNameEdit->clear();
    m_descriptionEdit->clear();
    
    m_imageFilePath.clear();
    m_imagePreview->setPixmap(QPixmap());
    m_imagePreview->setText(tr("No Image"));
    m_imageClearButton->setEnabled(false);
    
    m_attributes.clear();
    updateAttributesTable();
    
    m_attributeKeyEdit->clear();
    m_attributeValueEdit->clear();
    m_addAttributeButton->setEnabled(false);
    m_removeAttributeButton->setEnabled(false);

    // Reset validation state
    m_nameValid = false;
    m_imageValid = false;
    m_paymentVerified = false;
    
    updateMintButton();
}
