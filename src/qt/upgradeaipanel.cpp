#include "upgradeaipanel.h"
#include "monetizationmanager.h"
#include <qt/walletmodel.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>

#include <QMessageBox>
#include <QTimer>

UpgradeAIPanel::UpgradeAIPanel(WalletModel* walletModel, MonetizationManager* monetization, QWidget* parent)
    : QDialog(parent)
    , m_walletModel(walletModel)
    , m_monetization(monetization)
    , m_mainLayout(new QVBoxLayout(this))
    , m_titleLabel(new QLabel(tr("💳 Upgrade to AI Wallet"), this))
    , m_descLabel(new QLabel(tr("Unlock full AI features (Assistant, Insights, Troubleshooter, Fee Optimizer, Multilingual)."), this))
    , m_statusLabel(new QLabel("", this))
    , m_progress(new QProgressBar(this))
    , m_optionsGroup(new QGroupBox(tr("Payment Options"), this))
    , m_payShahRadio(new QRadioButton(tr("Pay with SHAH (auto-convert $9.99 via ShahSwap)"), this))
    , m_payShahTokenRadio(new QRadioButton(tr("Pay with SHAHtoken (SHI-20)"), this))
    , m_payStripeRadio(new QRadioButton(tr("Pay with Credit Card (Stripe) — $9.99"), this))
    , m_priceLabel(new QLabel("", this))
    , m_purchaseButton(new QPushButton(tr("Purchase"), this))
    , m_buyShahButton(new QPushButton(tr("Buy SHAH with Credit Card"), this))
    , m_cancelButton(new QPushButton(tr("Cancel"), this))
{
    setWindowTitle(tr("Upgrade to AI Wallet"));
    setModal(true);
    setMinimumSize(520, 420);

    setupUI();
    updatePriceLabels();

    connect(m_payShahRadio, &QRadioButton::toggled, this, &UpgradeAIPanel::onOptionChanged);
    connect(m_payShahTokenRadio, &QRadioButton::toggled, this, &UpgradeAIPanel::onOptionChanged);
    connect(m_payStripeRadio, &QRadioButton::toggled, this, &UpgradeAIPanel::onOptionChanged);
    connect(m_purchaseButton, &QPushButton::clicked, this, &UpgradeAIPanel::onPurchaseClicked);
    connect(m_buyShahButton, &QPushButton::clicked, this, &UpgradeAIPanel::onBuyShahClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &UpgradeAIPanel::onCancelClicked);
}

void UpgradeAIPanel::setupUI()
{
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    m_descLabel->setStyleSheet("color: #555;");
    m_statusLabel->setVisible(false);
    m_progress->setVisible(false);
    m_progress->setRange(0, 0);

    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_descLabel);

    QVBoxLayout* optLayout = new QVBoxLayout(m_optionsGroup);
    optLayout->addWidget(m_payShahRadio);
    optLayout->addWidget(m_payShahTokenRadio);
    optLayout->addWidget(m_payStripeRadio);
    m_mainLayout->addWidget(m_optionsGroup);

    m_payShahRadio->setChecked(true);

    m_priceLabel->setStyleSheet("font-weight: bold;");
    m_mainLayout->addWidget(m_priceLabel);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_purchaseButton);
    btnLayout->addStretch();
    btnLayout->addWidget(m_buyShahButton);
    btnLayout->addWidget(m_cancelButton);
    m_mainLayout->addLayout(btnLayout);

    m_mainLayout->addWidget(m_statusLabel);
    m_mainLayout->addWidget(m_progress);
}

void UpgradeAIPanel::updatePriceLabels()
{
    const double usd = m_monetization ? m_monetization->getPremiumPriceUsd() : 9.99;
    const double price = m_monetization ? m_monetization->getSHAHUsdPrice() : 0.10;
    const double shah_needed = price > 0 ? usd / price : 0.0;
    m_priceLabel->setText(tr("Price: $%1 • ~%2 SHAH (live price)")
                          .arg(QString::number(usd, 'f', 2))
                          .arg(QString::number(shah_needed, 'f', 4)));
}

void UpgradeAIPanel::showProcessing(const QString& text)
{
    m_statusLabel->setText(text);
    m_statusLabel->setVisible(true);
    m_progress->setVisible(true);
}

void UpgradeAIPanel::hideProcessing()
{
    m_progress->setVisible(false);
}

void UpgradeAIPanel::onOptionChanged()
{
    updatePriceLabels();
}

void UpgradeAIPanel::onPurchaseClicked()
{
    showProcessing(tr("Processing payment…"));
    const double usd = m_monetization ? m_monetization->getPremiumPriceUsd() : 9.99;
    QString receiptOrTx;
    bool ok = false;

    if (m_payStripeRadio->isChecked()) {
        ok = processStripePayment(usd, &receiptOrTx);
    } else if (m_payShahTokenRadio->isChecked()) {
        ok = processShahTokenPayment(usd, &receiptOrTx);
    } else {
        ok = processShahPayment(usd, &receiptOrTx);
    }

    hideProcessing();
    if (ok) {
        if (m_monetization) m_monetization->setPremiumUnlocked(true, m_payStripeRadio->isChecked() ? "Stripe" : (m_payShahTokenRadio->isChecked() ? "SHAHtoken" : "SHAH"));
        QMessageBox::information(this, tr("Success"), tr("AI Wallet unlocked! Receipt/TX: %1").arg(receiptOrTx));
        // Emit paymentRecorded for history
        const QString method = m_payStripeRadio->isChecked() ? "Credit Card" : (m_payShahTokenRadio->isChecked() ? "SHAHtoken" : "SHAH");
        const QString plan = "One-Time";
        const QString invoiceUrl = m_payStripeRadio->isChecked() ? QString("https://billing.stripe.com/invoice/%1").arg(receiptOrTx) : QString();
        Q_EMIT paymentRecorded(method, usd, receiptOrTx, plan, invoiceUrl);
        emit premiumUnlocked();
        accept();
    } else {
        QMessageBox::warning(this, tr("Payment Failed"), tr("We couldn't complete your payment. Please try another method."));
    }
}

void UpgradeAIPanel::onBuyShahClicked()
{
    showProcessing(tr("Buying SHAH via Stripe → Bridge → ShahSwap…"));
    QString shahTxid;
    const double usd = 25.00; // default quick-buy amount, can be made user-selectable
    const bool ok = bridgeStripeToShah(usd, &shahTxid);
    hideProcessing();
    if (ok) {
        QMessageBox::information(this, tr("Purchase Complete"), tr("Bought SHAH and deposited to your wallet. TX: %1").arg(shahTxid));
        updatePriceLabels();
    } else {
        QMessageBox::warning(this, tr("Purchase Failed"), tr("We couldn't complete the purchase. Please try again."));
    }
}

void UpgradeAIPanel::onCancelClicked()
{
    reject();
}

bool UpgradeAIPanel::processStripePayment(double amountUsd, QString* receiptOut)
{
    Q_UNUSED(amountUsd);
    // Placeholder for local Stripe checkout (requires external component/API)
    if (receiptOut) *receiptOut = "stripe_receipt_demo_123";
    return true;
}

bool UpgradeAIPanel::processShahPayment(double amountUsd, QString* txidOut)
{
    Q_UNUSED(amountUsd);
    // Placeholder: Use ShahSwap to compute exact SHAH and send to unlock address/contract
    if (txidOut) *txidOut = "shahswap_tx_demo_123";
    return true;
}

bool UpgradeAIPanel::processShahTokenPayment(double amountUsd, QString* txidOut)
{
    Q_UNUSED(amountUsd);
    // Placeholder for token payment path
    if (txidOut) *txidOut = "shahtoken_tx_demo_123";
    return true;
}

bool UpgradeAIPanel::bridgeStripeToShah(double amountUsd, QString* shahTxidOut)
{
    Q_UNUSED(amountUsd);
    // Placeholder: Stripe capture → bridge service → ShahSwap → send SHAH to wallet address
    if (shahTxidOut) *shahTxidOut = "bridge_shah_tx_demo_123";
    return true;
}


