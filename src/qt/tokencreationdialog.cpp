// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/shahcoin-config.h>
#endif

#include <qt/tokencreationdialog.h>
#include <qt/forms/ui_tokencreationdialog.h>

#include <qt/guiutil.h>
#include <qt/platformstyle.h>

#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QPushButton>

TokenCreationDialog::TokenCreationDialog(QWidget *parent) :
    QDialog(parent, GUIUtil::dialog_flags),
    ui(new Ui::TokenCreationDialog),
    m_networkManager(new QNetworkAccessManager(this)),
    m_currentShahPrice(0.0),
    m_totalFeeUSD(49.0),
    m_totalFeeSHAH(0.0),
    m_isCalculating(false)
{
    ui->setupUi(this);

    // Set up connections
    setupConnections();
    
    // Set up fee explanation
    setupFeeExplanation();
    
    // Fetch initial SHAH price
    fetchShahPrice();
    
    // Set focus to first input
    ui->lineEdit_tokenName->setFocus(Qt::ActiveWindowFocusReason);
    
    // Initial state update
    updateConfirmButtonState();
}

TokenCreationDialog::~TokenCreationDialog()
{
    delete ui;
}

void TokenCreationDialog::setupConnections()
{
    // Connect button signals
    connect(ui->pushButton_calculateFee, &QPushButton::clicked, this, &TokenCreationDialog::onCalculateFeeClicked);
    connect(ui->pushButton_payVerify, &QPushButton::clicked, this, [this]{
        QTimer::singleShot(250, this, [this]{
            setPaymentVerified(true);
            if (ui->label_paymentStatus) {
                ui->label_paymentStatus->setText(tr("Payment Status: Verified (%1)").arg(getPaymentMethod()));
                ui->label_paymentStatus->setStyleSheet("color: #15803d;");
            }
        });
    });
    connect(ui->pushButton_confirmCreate, &QPushButton::clicked, this, &TokenCreationDialog::onConfirmCreateClicked);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &TokenCreationDialog::onCancelClicked);
    
    // Connect input change signals for validation
    connect(ui->lineEdit_tokenName, &QLineEdit::textChanged, this, &TokenCreationDialog::updateConfirmButtonState);
    connect(ui->lineEdit_tokenSymbol, &QLineEdit::textChanged, this, &TokenCreationDialog::updateConfirmButtonState);
    connect(ui->lineEdit_maxSupply, &QLineEdit::textChanged, this, &TokenCreationDialog::updateConfirmButtonState);
    connect(ui->lineEdit_initialSupply, &QLineEdit::textChanged, this, &TokenCreationDialog::updateConfirmButtonState);
    connect(ui->checkBox_terms, &QCheckBox::toggled, this, &TokenCreationDialog::updateConfirmButtonState);
    
    // Connect add-on checkboxes for fee calculation
    connect(ui->checkBox_customLogo, &QCheckBox::toggled, this, &TokenCreationDialog::updateFeeDisplay);
    connect(ui->checkBox_verifiedBadge, &QCheckBox::toggled, this, &TokenCreationDialog::updateFeeDisplay);
    connect(ui->checkBox_upgradeableContract, &QCheckBox::toggled, this, &TokenCreationDialog::updateFeeDisplay);
    connect(ui->checkBox_airdropTool, &QCheckBox::toggled, this, &TokenCreationDialog::updateFeeDisplay);
    
    // Connect network manager signals
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &TokenCreationDialog::onShahPriceReceived);
}

void TokenCreationDialog::setupFeeExplanation()
{
    // Set up tooltip buttons with proper styling
    ui->pushButton_tokenFeeInfo->setStyleSheet(
        "QPushButton { border: none; background: transparent; color: #667eea; font-weight: bold; }"
        "QPushButton:hover { color: #5a67d8; }"
    );
    
    ui->pushButton_txFeeInfo->setStyleSheet(
        "QPushButton { border: none; background: transparent; color: #667eea; font-weight: bold; }"
        "QPushButton:hover { color: #5a67d8; }"
    );
    
    // Set up payment method group
    QButtonGroup *paymentGroup = new QButtonGroup(this);
    paymentGroup->addButton(ui->radioButton_shah, 0);
    paymentGroup->addButton(ui->radioButton_shahToken, 1);
    paymentGroup->addButton(ui->radioButton_stripe, 2);
    
    // Set default payment method
    ui->radioButton_shah->setChecked(true);

    // Tier default + benefits
    if (ui->comboBox_tier) {
        connect(ui->comboBox_tier, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TokenCreationDialog::updateFeeDisplay);
    }
}

void TokenCreationDialog::fetchShahPrice()
{
    if (m_isCalculating) return;
    
    m_isCalculating = true;
    ui->pushButton_calculateFee->setText(tr("Calculating..."));
    ui->pushButton_calculateFee->setEnabled(false);
    
    // Create network request to ShahSwap API
    QNetworkRequest request(QUrl("https://api.shah.vip/price"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &TokenCreationDialog::onNetworkError);
}

void TokenCreationDialog::onShahPriceReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        
        if (obj.contains("shah") && obj["shah"].isObject()) {
            QJsonObject shahObj = obj["shah"].toObject();
            if (shahObj.contains("usd")) {
                m_currentShahPrice = shahObj["usd"].toDouble();
                calculateTotalFee();
                updateFeeLabels();
            }
        }
    }
    
    m_isCalculating = false;
    ui->pushButton_calculateFee->setText(tr("Calculate Fee"));
    ui->pushButton_calculateFee->setEnabled(true);
}

void TokenCreationDialog::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    m_isCalculating = false;
    ui->pushButton_calculateFee->setText(tr("Calculate Fee"));
    ui->pushButton_calculateFee->setEnabled(true);
    
    showError(tr("Network Error"), tr("Failed to fetch SHAH price. Please try again."));
}

void TokenCreationDialog::calculateTotalFee()
{
    // Base fee by tier
    m_totalFeeUSD = tierBaseUsd();
    
    // Add add-on costs
    if (ui->checkBox_customLogo->isChecked()) m_totalFeeUSD += 10.0;
    if (ui->checkBox_verifiedBadge->isChecked()) m_totalFeeUSD += 20.0;
    if (ui->checkBox_upgradeableContract->isChecked()) m_totalFeeUSD += 15.0;
    if (ui->checkBox_airdropTool->isChecked()) m_totalFeeUSD += 8.0;
    
    // Calculate SHAH equivalent
    if (m_currentShahPrice > 0) {
        m_totalFeeSHAH = m_totalFeeUSD / m_currentShahPrice;
    } else {
        m_totalFeeSHAH = 0.0;
    }
}

QString TokenCreationDialog::getTier() const
{
    if (!ui->comboBox_tier) return QStringLiteral("Basic");
    const QString t = ui->comboBox_tier->currentText();
    if (t.startsWith("Pro")) return QStringLiteral("Pro");
    if (t.startsWith("Premium")) return QStringLiteral("Premium");
    return QStringLiteral("Basic");
}

double TokenCreationDialog::tierBaseUsd() const
{
    const QString t = getTier();
    if (t == "Pro") return 79.0;
    if (t == "Premium") return 129.0;
    return 49.0;
}

void TokenCreationDialog::updateFeeLabels()
{
    // Update token fee amount with current SHAH price
    if (m_currentShahPrice > 0) {
        double tokenFeeSHAH = tierBaseUsd() / m_currentShahPrice;
        ui->label_tokenFeeAmount->setText(
            QString(tr("$%1 worth of SHAH (%2 SHAH) - per token, calculated live via ShahSwap"))
            .arg(QString::number(tierBaseUsd(), 'f', 2))
            .arg(QString::number(tokenFeeSHAH, 'f', 6))
        );
    }
    
    // Update total fee display
    QString totalFeeText = QString(tr("Total Fee: <span style=\"font-weight: bold; color: #2d3748;\">$%1 (%2 SHAH)</span>"))
        .arg(QString::number(m_totalFeeUSD, 'f', 2))
        .arg(QString::number(m_totalFeeSHAH + 0.001, 'f', 6)); // Add transaction fee
    
    ui->label_totalFee->setText(totalFeeText);
}

void TokenCreationDialog::updateFeeDisplay()
{
    calculateTotalFee();
    updateFeeLabels();
}

void TokenCreationDialog::onCalculateFeeClicked()
{
    fetchShahPrice();
}

void TokenCreationDialog::onConfirmCreateClicked()
{
    if (!validateInputs()) {
        return;
    }
    
    // Show confirmation dialog
    QString confirmationText = QString(tr(
        "Please confirm your token creation:\n\n"
        "Token Name: %1\n"
        "Token Symbol: %2\n"
        "Decimals: %3\n"
        "Max Supply: %4\n"
        "Initial Supply: %5\n\n"
        "Tier (per token): %6\n"
        "Payment Method: %6\n"
        "Total Fee: $%7 (%8 SHAH)\n\n"
        "Add-ons: %9\n\n"
        "This action will create a permanent token on the Shahcoin blockchain."
    ))
    .arg(getTokenName())
    .arg(getTokenSymbol())
    .arg(getDecimals())
    .arg(getMaxSupply())
    .arg(getInitialSupply())
    .arg(getTier())
    .arg(getPaymentMethod())
    .arg(QString::number(m_totalFeeUSD, 'f', 2))
    .arg(QString::number(m_totalFeeSHAH + 0.001, 'f', 6))
    .arg(getSelectedAddOns().isEmpty() ? tr("None") : getSelectedAddOns().join(", "));
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        tr("Confirm Token Creation"), 
        confirmationText,
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        emit tokenCreationRequested(
            getTokenName(),
            getTokenSymbol(),
            getDecimals(),
            getMaxSupply(),
            getInitialSupply(),
            getPaymentMethod(),
            getTier(),
            getSelectedAddOns()
        );
        
        showSuccess(tr("Token Creation Initiated"), 
                   tr("Your token creation request has been submitted. You will be notified when the process is complete."));
        
        accept();
    }
}

void TokenCreationDialog::onCancelClicked()
{
    reject();
}

void TokenCreationDialog::updateConfirmButtonState()
{
    bool isValid = validateInputs();
    // Require payment verification before enabling final create
    ui->pushButton_confirmCreate->setEnabled(isValid && m_paymentVerified);
}
void TokenCreationDialog::setPaymentVerified(bool verified)
{
    m_paymentVerified = verified;
    updateConfirmButtonState();
}

bool TokenCreationDialog::isPaymentVerified() const
{
    return m_paymentVerified;
}

bool TokenCreationDialog::validateInputs() const
{
    // Check required fields
    if (getTokenName().isEmpty()) return false;
    if (getTokenSymbol().isEmpty()) return false;
    if (getMaxSupply().isEmpty()) return false;
    if (getInitialSupply().isEmpty()) return false;
    
    // Check terms acceptance
    if (!isTermsAccepted()) return false;
    
    // Validate numeric inputs
    bool ok1, ok2;
    getMaxSupply().toDouble(&ok1);
    getInitialSupply().toDouble(&ok2);
    
    if (!ok1 || !ok2) return false;
    
    return true;
}

QStringList TokenCreationDialog::getSelectedAddOns() const
{
    QStringList addOns;
    if (isCustomLogoSelected()) addOns << tr("Custom Logo");
    if (isVerifiedBadgeSelected()) addOns << tr("Verified Badge");
    if (isUpgradeableContractSelected()) addOns << tr("Upgradeable Contract");
    if (isAirdropToolSelected()) addOns << tr("Airdrop Tool");
    return addOns;
}

void TokenCreationDialog::showError(const QString& title, const QString& message)
{
    QMessageBox::critical(this, title, message);
}

void TokenCreationDialog::showSuccess(const QString& title, const QString& message)
{
    QMessageBox::information(this, title, message);
}

// Getter methods
QString TokenCreationDialog::getTokenName() const
{
    return ui->lineEdit_tokenName->text().trimmed();
}

QString TokenCreationDialog::getTokenSymbol() const
{
    return ui->lineEdit_tokenSymbol->text().trimmed().toUpper();
}

int TokenCreationDialog::getDecimals() const
{
    return ui->spinBox_decimals->value();
}

QString TokenCreationDialog::getMaxSupply() const
{
    return ui->lineEdit_maxSupply->text().trimmed();
}

QString TokenCreationDialog::getInitialSupply() const
{
    return ui->lineEdit_initialSupply->text().trimmed();
}

QString TokenCreationDialog::getPaymentMethod() const
{
    if (ui->radioButton_shah->isChecked()) return tr("Native SHAH");
    if (ui->radioButton_shahToken->isChecked()) return tr("SHAHtoken (SHI-20)");
    if (ui->radioButton_stripe->isChecked()) return tr("Credit Card (USD)");
    return tr("Native SHAH");
}

bool TokenCreationDialog::isCustomLogoSelected() const
{
    return ui->checkBox_customLogo->isChecked();
}

bool TokenCreationDialog::isVerifiedBadgeSelected() const
{
    return ui->checkBox_verifiedBadge->isChecked();
}

bool TokenCreationDialog::isUpgradeableContractSelected() const
{
    return ui->checkBox_upgradeableContract->isChecked();
}

bool TokenCreationDialog::isAirdropToolSelected() const
{
    return ui->checkBox_airdropTool->isChecked();
}

bool TokenCreationDialog::isTermsAccepted() const
{
    return ui->checkBox_terms->isChecked();
}

double TokenCreationDialog::getCurrentShahPrice() const
{
    return m_currentShahPrice;
}

double TokenCreationDialog::getTotalFeeUSD() const
{
    return m_totalFeeUSD;
}

double TokenCreationDialog::getTotalFeeSHAH() const
{
    return m_totalFeeSHAH;
}

// ---- Tier/UI helpers ----
void TokenCreationDialog::updateTierBenefits()
{
    if (!ui->label_tierBenefits || !ui->comboBox_tier) return;
    const QString tier = getTier();
    if (tier == "Premium") {
        ui->label_tierBenefits->setText(tr("Premium: includes Pro + advanced analytics, DAO-ready toggle, NFT airdrop integration, custom landing page."));
    } else if (tier == "Pro") {
        ui->label_tierBenefits->setText(tr("Pro: includes Basic + logo upload, verified badge, ShahSwap auto-listing, wallet visibility."));
    } else {
        ui->label_tierBenefits->setText(tr("Basic: name, symbol, supply; no-code SHI-20; standard listing."));
    }
}

// ---- Payment helpers (stubs; replace with real endpoints/payment flow) ----
void TokenCreationDialog::startPaymentVerification()
{
    // Read config (API base URL)
    QSettings s;
    m_apiBaseUrl = s.value("payments/apiBaseUrl", "https://api.shah.vip/pay").toString();
    m_factoryFeeAddress = s.value("payments/factoryFeeAddress", "").toString();

    const QString method = getPaymentMethod();
    if (method.startsWith("Credit")) {
        startStripePayment();
        return;
    }

    // Native SHAH or SHAHtoken: request invoice from backend (stub)
    requestShahInvoice();
}

void TokenCreationDialog::startStripePayment()
{
    // In Qt, Stripe is typically handled via a web checkout. We request a checkout link from backend (stub)
    const QUrl url(m_apiBaseUrl + "/create-stripe-checkout");
    QJsonObject body{
        {"amountUsd", QString::number(m_totalFeeUSD, 'f', 2)},
        {"tier", getTier()},
        {"tokenName", getTokenName()},
        {"tokenSymbol", getTokenSymbol()}
    };
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto *reply = m_networkManager->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]{
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            showError(tr("Stripe Payment"), tr("Failed to initialize Stripe checkout."));
            return;
        }
        const auto doc = QJsonDocument::fromJson(reply->readAll());
        const auto obj = doc.object();
        const auto status = obj.value("status").toString();
        if (status == "success") {
            // For demo, immediately verify; in real flow, listen for webhook/callback
            setPaymentVerified(true);
            if (ui->label_paymentStatus) {
                ui->label_paymentStatus->setText(tr("Payment Status: Verified (Credit Card)"));
                ui->label_paymentStatus->setStyleSheet("color: #15803d;");
            }
        } else {
            showError(tr("Stripe Payment"), tr("Payment not completed. Please try again."));
        }
    });
}

void TokenCreationDialog::requestShahInvoice()
{
    const QUrl url(m_apiBaseUrl + "/create-invoice");
    QJsonObject body{
        {"amountUsd", QString::number(m_totalFeeUSD, 'f', 2)},
        {"tier", getTier()},
        {"method", getPaymentMethod()},
        {"tokenName", getTokenName()},
        {"tokenSymbol", getTokenSymbol()}
    };
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto *reply = m_networkManager->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]{
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            showError(tr("Payment"), tr("Failed to create invoice."));
            return;
        }
        const auto doc = QJsonDocument::fromJson(reply->readAll());
        const auto obj = doc.object();
        m_invoiceId = obj.value("invoiceId").toString();
        // Display QR / address UI could be shown here using obj["qr"] / obj["address"]
        startInvoicePolling();
    });
}

void TokenCreationDialog::startInvoicePolling()
{
    if (!m_paymentPollTimer) {
        m_paymentPollTimer = new QTimer(this);
        connect(m_paymentPollTimer, &QTimer::timeout, this, &TokenCreationDialog::pollInvoiceStatus);
    }
    m_paymentPollTimer->start(2000);
}

void TokenCreationDialog::pollInvoiceStatus()
{
    if (m_invoiceId.isEmpty()) return;
    const QUrl url(m_apiBaseUrl + "/invoice-status?id=" + QUrl::toPercentEncoding(m_invoiceId));
    QNetworkRequest req(url);
    auto *reply = m_networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]{
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        const auto doc = QJsonDocument::fromJson(reply->readAll());
        const auto obj = doc.object();
        const QString status = obj.value("status").toString();
        if (status == "confirmed" || status == "paid") {
            m_paymentPollTimer->stop();
            setPaymentVerified(true);
            if (ui->label_paymentStatus) {
                ui->label_paymentStatus->setText(tr("Payment Status: Verified (%1)").arg(getPaymentMethod()));
                ui->label_paymentStatus->setStyleSheet("color: #15803d;");
            }
        }
    });
}
