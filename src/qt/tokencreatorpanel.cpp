// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/tokencreatorpanel.h>

#include <qt/walletmodel.h>
#include <qt/platformstyle.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>

#include <interfaces/wallet.h>
#include <util/translation.h>
#include <tokens/token.h>

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
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QValidator>

TokenCreatorPanel::TokenCreatorPanel(const PlatformStyle* platformStyle, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(nullptr)
    , m_platformStyle(platformStyle)
    , m_nameValid(false)
    , m_symbolValid(false)
    , m_supplyValid(false)
    , m_decimalsValid(false)
    , m_hasSufficientBalance(false)
    , m_creationFee(100 * COIN) // 100 SHAH default fee
    , m_currentBalance(0)
{
    setupUI();
    connectSignals();
    applyTheme();
    refreshData();
}

TokenCreatorPanel::~TokenCreatorPanel()
{
}

void TokenCreatorPanel::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    refreshData();
}

void TokenCreatorPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(12);

    // Token Details Group
    m_tokenDetailsGroup = new QGroupBox(tr("Token Details"));
    QGridLayout* tokenGrid = new QGridLayout(m_tokenDetailsGroup);

    // Token Name
    tokenGrid->addWidget(new QLabel(tr("Token Name:")), 0, 0);
    m_tokenNameEdit = new QLineEdit();
    m_tokenNameEdit->setPlaceholderText(tr("Enter token name (e.g., Radix Token)"));
    m_tokenNameEdit->setMaxLength(50);
    tokenGrid->addWidget(m_tokenNameEdit, 0, 1);

    // Token Symbol
    tokenGrid->addWidget(new QLabel(tr("Token Symbol:")), 1, 0);
    m_tokenSymbolEdit = new QLineEdit();
    m_tokenSymbolEdit->setPlaceholderText(tr("Enter symbol (e.g., RDX)"));
    m_tokenSymbolEdit->setMaxLength(10);
    tokenGrid->addWidget(m_tokenSymbolEdit, 1, 1);

    // Total Supply
    tokenGrid->addWidget(new QLabel(tr("Total Supply:")), 2, 0);
    m_supplySpinBox = new QSpinBox();
    m_supplySpinBox->setRange(1, 999999999);
    m_supplySpinBox->setValue(1000000);
    m_supplySpinBox->setSuffix(tr(" units"));
    tokenGrid->addWidget(m_supplySpinBox, 2, 1);

    // Decimal Places
    tokenGrid->addWidget(new QLabel(tr("Decimal Places:")), 3, 0);
    m_decimalsSpinBox = new QSpinBox();
    m_decimalsSpinBox->setRange(0, 18);
    m_decimalsSpinBox->setValue(8);
    m_decimalsSpinBox->setSuffix(tr(" decimals"));
    tokenGrid->addWidget(m_decimalsSpinBox, 3, 1);

    // Description
    tokenGrid->addWidget(new QLabel(tr("Description:")), 4, 0);
    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setPlaceholderText(tr("Enter token description (optional)"));
    m_descriptionEdit->setMaximumHeight(80);
    tokenGrid->addWidget(m_descriptionEdit, 4, 1);

    m_mainLayout->addWidget(m_tokenDetailsGroup);

    // Logo Group
    m_logoGroup = new QGroupBox(tr("Token Logo (Optional)"));
    QHBoxLayout* logoLayout = new QHBoxLayout(m_logoGroup);

    m_logoPreview = new QLabel();
    m_logoPreview->setFixedSize(64, 64);
    m_logoPreview->setStyleSheet("border: 2px dashed #ccc; border-radius: 8px;");
    m_logoPreview->setAlignment(Qt::AlignCenter);
    m_logoPreview->setText(tr("No Logo"));

    m_logoUploadButton = new QPushButton(tr("Upload Logo"));
    m_logoUploadButton->setIcon(QIcon::fromTheme("document-open"));

    m_logoClearButton = new QPushButton(tr("Clear"));
    m_logoClearButton->setIcon(QIcon::fromTheme("edit-clear"));
    m_logoClearButton->setEnabled(false);

    logoLayout->addWidget(m_logoPreview);
    logoLayout->addWidget(m_logoUploadButton);
    logoLayout->addWidget(m_logoClearButton);
    logoLayout->addStretch();

    m_mainLayout->addWidget(m_logoGroup);

    // Fee and Balance Group
    m_feeGroup = new QGroupBox(tr("Creation Fee & Balance"));
    QVBoxLayout* feeLayout = new QVBoxLayout(m_feeGroup);

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

    m_mainLayout->addWidget(m_feeGroup);

    // Action Buttons
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();

    m_createButton = new QPushButton(tr("Create Token"));
    m_createButton->setIcon(QIcon::fromTheme("document-new"));
    m_createButton->setEnabled(false);

    m_clearButton = new QPushButton(tr("Clear Form"));
    m_clearButton->setIcon(QIcon::fromTheme("edit-clear"));

    m_buttonLayout->addWidget(m_clearButton);
    m_buttonLayout->addWidget(m_createButton);

    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addStretch();
}

void TokenCreatorPanel::connectSignals()
{
    connect(m_tokenNameEdit, &QLineEdit::textChanged, this, &TokenCreatorPanel::onTokenNameChanged);
    connect(m_tokenSymbolEdit, &QLineEdit::textChanged, this, &TokenCreatorPanel::onTokenSymbolChanged);
    connect(m_supplySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TokenCreatorPanel::onSupplyChanged);
    connect(m_decimalsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TokenCreatorPanel::onDecimalsChanged);
    
    connect(m_logoUploadButton, &QPushButton::clicked, this, &TokenCreatorPanel::onLogoUploadClicked);
    connect(m_logoClearButton, &QPushButton::clicked, [this]() {
        m_logoFilePath.clear();
        m_logoPreview->setPixmap(QPixmap());
        m_logoPreview->setText(tr("No Logo"));
        m_logoClearButton->setEnabled(false);
    });
    
    connect(m_createButton, &QPushButton::clicked, this, &TokenCreatorPanel::onCreateTokenClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &TokenCreatorPanel::onClearClicked);
}

void TokenCreatorPanel::applyTheme()
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
        QLineEdit, QSpinBox, QTextEdit {
            border: 1px solid #ced4da;
            border-radius: 4px;
            padding: 6px;
            background-color: #ffffff;
        }
        QLineEdit:focus, QSpinBox:focus, QTextEdit:focus {
            border-color: #007bff;
            outline: none;
        }
        QLineEdit:invalid, QSpinBox:invalid {
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

void TokenCreatorPanel::refreshData()
{
    if (m_walletModel) {
        m_currentBalance = m_walletModel->getBalance();
        updateFeeDisplay();
        updateCreateButton();
    }
}

void TokenCreatorPanel::updateFeeDisplay()
{
    QString feeText = ShahcoinUnits::formatWithUnit(
        m_walletModel ? m_walletModel->getOptionsModel()->getDisplayUnit() : ShahcoinUnit::SHAH,
        m_creationFee,
        false,
        ShahcoinUnits::SeparatorStyle::STANDARD
    );
    m_feeLabel->setText(tr("Creation Fee: %1").arg(feeText));

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

void TokenCreatorPanel::updateCreateButton()
{
    bool canCreate = m_nameValid && m_symbolValid && m_supplyValid && 
                    m_decimalsValid && m_hasSufficientBalance;
    m_createButton->setEnabled(canCreate);
}

void TokenCreatorPanel::onTokenNameChanged()
{
    QString name = m_tokenNameEdit->text().trimmed();
    m_nameValid = name.length() >= 3 && name.length() <= 50;
    
    if (m_nameValid) {
        m_tokenNameEdit->setStyleSheet("");
    } else {
        m_tokenNameEdit->setStyleSheet("border-color: #dc3545; background-color: #fff5f5;");
    }
    
    updateCreateButton();
}

void TokenCreatorPanel::onTokenSymbolChanged()
{
    QString symbol = m_tokenSymbolEdit->text().trimmed().toUpper();
    m_symbolValid = symbol.length() >= 2 && symbol.length() <= 10 && 
                   symbol.matches(QRegularExpression("^[A-Z0-9]+$"));
    
    if (m_symbolValid) {
        m_tokenSymbolEdit->setStyleSheet("");
    } else {
        m_tokenSymbolEdit->setStyleSheet("border-color: #dc3545; background-color: #fff5f5;");
    }
    
    updateCreateButton();
}

void TokenCreatorPanel::onSupplyChanged()
{
    int supply = m_supplySpinBox->value();
    m_supplyValid = supply > 0 && supply <= 999999999;
    updateCreateButton();
}

void TokenCreatorPanel::onDecimalsChanged()
{
    int decimals = m_decimalsSpinBox->value();
    m_decimalsValid = decimals >= 0 && decimals <= 18;
    updateCreateButton();
}

void TokenCreatorPanel::onLogoUploadClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Token Logo"),
        QDir::homePath(),
        tr("Image Files (*.png *.jpg *.jpeg *.gif *.bmp);;All Files (*)")
    );

    if (!filePath.isEmpty()) {
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            // Scale to 64x64 for preview
            QPixmap scaledPixmap = pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_logoPreview->setPixmap(scaledPixmap);
            m_logoFilePath = filePath;
            m_logoClearButton->setEnabled(true);
        } else {
            QMessageBox::warning(this, tr("Invalid Image"), tr("The selected file is not a valid image."));
        }
    }
}

void TokenCreatorPanel::onCreateTokenClicked()
{
    if (!validateInputs()) {
        return;
    }

    showConfirmationDialog();
}

void TokenCreatorPanel::onClearClicked()
{
    clearForm();
}

bool TokenCreatorPanel::validateInputs()
{
    if (!m_nameValid) {
        QMessageBox::warning(this, tr("Invalid Token Name"), 
                           tr("Token name must be 3-50 characters long."));
        return false;
    }

    if (!m_symbolValid) {
        QMessageBox::warning(this, tr("Invalid Token Symbol"), 
                           tr("Token symbol must be 2-10 uppercase letters/numbers."));
        return false;
    }

    if (!m_supplyValid) {
        QMessageBox::warning(this, tr("Invalid Supply"), 
                           tr("Total supply must be between 1 and 999,999,999."));
        return false;
    }

    if (!m_decimalsValid) {
        QMessageBox::warning(this, tr("Invalid Decimals"), 
                           tr("Decimal places must be between 0 and 18."));
        return false;
    }

    if (!m_hasSufficientBalance) {
        QMessageBox::warning(this, tr("Insufficient Balance"), 
                           tr("You don't have enough SHAH to pay the creation fee."));
        return false;
    }

    return true;
}

void TokenCreatorPanel::showConfirmationDialog()
{
    QString tokenName = m_tokenNameEdit->text().trimmed();
    QString tokenSymbol = m_tokenSymbolEdit->text().trimmed().toUpper();
    int supply = m_supplySpinBox->value();
    int decimals = m_decimalsSpinBox->value();
    QString description = m_descriptionEdit->toPlainText().trimmed();

    QString feeText = ShahcoinUnits::formatWithUnit(
        m_walletModel ? m_walletModel->getOptionsModel()->getDisplayUnit() : ShahcoinUnit::SHAH,
        m_creationFee,
        false,
        ShahcoinUnits::SeparatorStyle::STANDARD
    );

    QString message = tr(
        "Please confirm token creation:\n\n"
        "Token Name: %1\n"
        "Token Symbol: %2\n"
        "Total Supply: %3\n"
        "Decimal Places: %4\n"
        "Description: %5\n"
        "Creation Fee: %6\n\n"
        "This action cannot be undone. Continue?"
    ).arg(tokenName, tokenSymbol, QString::number(supply), 
          QString::number(decimals), description.isEmpty() ? tr("None") : description, feeText);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Confirm Token Creation"), message,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        createToken();
    }
}

void TokenCreatorPanel::createToken()
{
    if (!m_walletModel) {
        showErrorDialog(tr("Wallet not available"));
        return;
    }

    QString tokenName = m_tokenNameEdit->text().trimmed();
    QString tokenSymbol = m_tokenSymbolEdit->text().trimmed().toUpper();
    int supply = m_supplySpinBox->value();
    int decimals = m_decimalsSpinBox->value();
    QString description = m_descriptionEdit->toPlainText().trimmed();
    QString logoBase64 = m_logoFilePath.isEmpty() ? "" : encodeLogoToBase64(m_logoFilePath);

    try {
        // Call the token creation function from WalletModel
        QString tokenId = m_walletModel->createToken(
            tokenName, tokenSymbol, supply, decimals, description, logoBase64
        );

        if (!tokenId.isEmpty()) {
            showSuccessDialog(tokenId);
            clearForm();
        } else {
            showErrorDialog(tr("Failed to create token. Please try again."));
        }
    } catch (const std::exception& e) {
        showErrorDialog(tr("Error creating token: %1").arg(QString::fromStdString(e.what())));
    }
}

void TokenCreatorPanel::showSuccessDialog(const QString& tokenId)
{
    QString message = tr(
        "Token created successfully!\n\n"
        "Token ID: %1\n\n"
        "Your new token has been created and is now available in your wallet. "
        "You can view it in the Token Explorer panel."
    ).arg(tokenId);

    QMessageBox::information(this, tr("Token Created"), message);
}

void TokenCreatorPanel::showErrorDialog(const QString& error)
{
    QMessageBox::critical(this, tr("Token Creation Failed"), error);
}

QString TokenCreatorPanel::encodeLogoToBase64(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return "";
    }

    QByteArray imageData = file.readAll();
    return imageData.toBase64();
}

void TokenCreatorPanel::clearForm()
{
    m_tokenNameEdit->clear();
    m_tokenSymbolEdit->clear();
    m_supplySpinBox->setValue(1000000);
    m_decimalsSpinBox->setValue(8);
    m_descriptionEdit->clear();
    
    m_logoFilePath.clear();
    m_logoPreview->setPixmap(QPixmap());
    m_logoPreview->setText(tr("No Logo"));
    m_logoClearButton->setEnabled(false);

    // Reset validation state
    m_nameValid = false;
    m_symbolValid = false;
    m_supplyValid = true;
    m_decimalsValid = true;
    
    updateCreateButton();
}
