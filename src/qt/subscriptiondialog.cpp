#include "subscriptiondialog.h"
#include "monetizationmanager.h"
#include <qt/walletmodel.h>

#include <QDateTime>
#include <QMessageBox>

SubscriptionDialog::SubscriptionDialog(MonetizationManager* monetization, WalletModel* walletModel, QWidget* parent)
    : QDialog(parent)
    , m_monetization(monetization)
    , m_walletModel(walletModel)
    , m_layout(new QVBoxLayout(this))
    , m_statusLabel(new QLabel(this))
    , m_priceLabel(new QLabel(this))
    , m_dueLabel(new QLabel(this))
    , m_methodGroup(new QGroupBox(tr("Payment Method"), this))
    , m_shahRadio(new QRadioButton(tr("SHAH (native)"), this))
    , m_shahTokenRadio(new QRadioButton(tr("SHAHtoken (SHI-20)"), this))
    , m_stripeRadio(new QRadioButton(tr("Stripe (Credit Card)"), this))
    , m_subscribeButton(new QPushButton(tr("Subscribe"), this))
    , m_cancelButton(new QPushButton(tr("Cancel Subscription"), this))
    , m_renewButton(new QPushButton(tr("Renew Now"), this))
{
    setWindowTitle(tr("AI Wallet Subscription"));
    setMinimumSize(480, 300);
    setupUI();
    updateStatus();
}

void SubscriptionDialog::setupUI()
{
    m_statusLabel->setStyleSheet("font-weight: bold;");
    m_layout->addWidget(m_statusLabel);
    m_layout->addWidget(m_priceLabel);
    m_layout->addWidget(m_dueLabel);

    QVBoxLayout* methods = new QVBoxLayout(m_methodGroup);
    m_shahRadio->setChecked(true);
    methods->addWidget(m_shahRadio);
    methods->addWidget(m_shahTokenRadio);
    methods->addWidget(m_stripeRadio);
    m_layout->addWidget(m_methodGroup);

    QHBoxLayout* actions = new QHBoxLayout();
    actions->addWidget(m_subscribeButton);
    actions->addWidget(m_renewButton);
    actions->addStretch();
    actions->addWidget(m_cancelButton);
    m_layout->addLayout(actions);

    connect(m_subscribeButton, &QPushButton::clicked, this, &SubscriptionDialog::onSubscribeClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SubscriptionDialog::onCancelSubscriptionClicked);
    connect(m_renewButton, &QPushButton::clicked, this, &SubscriptionDialog::onRenewNowClicked);
    connect(m_shahRadio, &QRadioButton::toggled, this, &SubscriptionDialog::onMethodChanged);
    connect(m_shahTokenRadio, &QRadioButton::toggled, this, &SubscriptionDialog::onMethodChanged);
    connect(m_stripeRadio, &QRadioButton::toggled, this, &SubscriptionDialog::onMethodChanged);
}

void SubscriptionDialog::updateStatus()
{
    if (!m_monetization) return;
    const bool active = m_monetization->isSubscriptionActive();
    const double monthly = m_monetization->getMonthlyPriceUsd();
    m_statusLabel->setText(active ? tr("Status: Active") : tr("Status: Not Active"));
    m_priceLabel->setText(tr("Monthly Price: $%1").arg(QString::number(monthly, 'f', 2)));
    const QDateTime due = m_monetization->subscriptionRenewalDue();
    m_dueLabel->setText(due.isValid() ? tr("Renewal Due: %1").arg(due.toString("yyyy-MM-dd")) : tr("Renewal Due: N/A"));
}

void SubscriptionDialog::onMethodChanged()
{
    // no-op placeholder (could change hints)
}

void SubscriptionDialog::onSubscribeClicked()
{
    if (!m_monetization) return;
    // Placeholder flows: in production, open wallet modal for SHAH/SHHtoken or Stripe Checkout
    if (m_stripeRadio->isChecked()) {
        QMessageBox::information(this, tr("Stripe"), tr("Opening Stripe checkout…"));
        // After webhook success:
        m_monetization->setSubscriptionActive(true, "Stripe");
        Q_EMIT paymentRecorded("Credit Card", m_monetization->getMonthlyPriceUsd(), "stripe_sub_demo_123", "Monthly", "");
    } else if (m_shahRadio->isChecked()) {
        const double required = m_monetization->getRequiredShahForMonthly();
        QMessageBox::information(this, tr("SHAH"), tr("Requesting %1 SHAH subscription payment…").arg(QString::number(required, 'f', 4)));
        m_monetization->setSubscriptionActive(true, "SHAH");
        Q_EMIT paymentRecorded("SHAH", m_monetization->getMonthlyPriceUsd(), "shah_sub_demo_123", "Monthly", "");
    } else {
        const double required = m_monetization->getRequiredShahForMonthly();
        QMessageBox::information(this, tr("SHAHtoken"), tr("Requesting %1 SHAHtoken subscription payment…").arg(QString::number(required, 'f', 4)));
        m_monetization->setSubscriptionActive(true, "SHAHtoken");
        Q_EMIT paymentRecorded("SHAHtoken", m_monetization->getMonthlyPriceUsd(), "shahtoken_sub_demo_123", "Monthly", "");
    }
    updateStatus();
}

void SubscriptionDialog::onCancelSubscriptionClicked()
{
    if (!m_monetization) return;
    m_monetization->setSubscriptionActive(false, "");
    QMessageBox::information(this, tr("Subscription"), tr("Subscription cancelled."));
    updateStatus();
}

void SubscriptionDialog::onRenewNowClicked()
{
    if (!m_monetization) return;
    if (!m_monetization->isSubscriptionActive()) {
        QMessageBox::warning(this, tr("Renew"), tr("No active subscription."));
        return;
    }
    m_monetization->setSubscriptionRenewalDue(QDateTime::currentDateTime().addDays(30));
    QMessageBox::information(this, tr("Renew"), tr("Subscription renewed for 30 days."));
    updateStatus();
}


