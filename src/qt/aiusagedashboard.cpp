#include "aiusagedashboard.h"
#include "monetizationmanager.h"
#include <qt/walletmodel.h>

#include <QDateTime>
#include <QSettings>

AIUsageDashboard::AIUsageDashboard(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void AIUsageDashboard::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_statusLabel = new QLabel(this);
    m_timeRemainingLabel = new QLabel(this);
    m_messagesLabel = new QLabel(this);
    m_chatsLabel = new QLabel(this);
    m_notificationsLabel = new QLabel(this);
    m_languageLabel = new QLabel(this);
    m_savingsLabel = new QLabel(this);
    m_upgradeButton = new QPushButton(tr("Upgrade Plan"), this);
    m_extendButton = new QPushButton(tr("Extend Subscription"), this);

    QHBoxLayout* header = new QHBoxLayout();
    header->addWidget(m_statusLabel);
    header->addStretch();
    header->addWidget(m_upgradeButton);
    header->addWidget(m_extendButton);
    m_mainLayout->addLayout(header);

    QGroupBox* stats = new QGroupBox(tr("Usage"), this);
    QVBoxLayout* statsLayout = new QVBoxLayout(stats);
    statsLayout->addWidget(m_timeRemainingLabel);
    statsLayout->addWidget(m_messagesLabel);
    statsLayout->addWidget(m_chatsLabel);
    statsLayout->addWidget(m_notificationsLabel);
    statsLayout->addWidget(m_languageLabel);
    statsLayout->addWidget(m_savingsLabel);
    m_mainLayout->addWidget(stats);

    // Feature usage chart
    QPieSeries* series = new QPieSeries();
    series->append("Insights", 0);
    series->append("Chat", 0);
    series->append("Alerts", 0);
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(tr("Feature Usage"));
    m_chartView = new QChartView(chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_mainLayout->addWidget(m_chartView);

    // Overlay for expired state
    m_overlay = new QFrame(this);
    m_overlay->setStyleSheet("QFrame { background: rgba(0,0,0,0.5); } QLabel { color: white; font-weight: bold; }");
    m_overlay->setVisible(false);
    QVBoxLayout* overlayLayout = new QVBoxLayout(m_overlay);
    overlayLayout->addStretch();
    m_overlayText = new QLabel(tr("Renew to unlock insights"), m_overlay);
    m_overlayText->setAlignment(Qt::AlignCenter);
    overlayLayout->addWidget(m_overlayText);
    m_overlayRenewButton = new QPushButton(tr("Renew Now"), m_overlay);
    overlayLayout->addWidget(m_overlayRenewButton, 0, Qt::AlignCenter);
    overlayLayout->addStretch();
    m_mainLayout->addWidget(m_overlay);

    connect(m_extendButton, &QPushButton::clicked, this, &AIUsageDashboard::onRenewClicked);
    connect(m_upgradeButton, &QPushButton::clicked, this, &AIUsageDashboard::onUpgradeClicked);
    connect(m_overlayRenewButton, &QPushButton::clicked, this, &AIUsageDashboard::onRenewClicked);
}

void AIUsageDashboard::setWalletModel(WalletModel* model)
{
    m_walletModel = model;
}

void AIUsageDashboard::setMonetization(MonetizationManager* monetization)
{
    m_monetization = monetization;
}

QString AIUsageDashboard::formatDuration(qint64 secs) const
{
    if (secs < 0) secs = 0;
    qint64 days = secs / 86400; secs %= 86400;
    qint64 hours = secs / 3600; secs %= 3600;
    qint64 minutes = secs / 60;
    return QString("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
}

void AIUsageDashboard::updateChart(int insightsCount, int chatCount, int alertsCount)
{
    QChart* chart = m_chartView->chart();
    auto* series = qobject_cast<QPieSeries*>(chart->series().isEmpty() ? nullptr : chart->series().first());
    if (!series) return;
    series->clear();
    series->append("Insights", insightsCount);
    series->append("Chat", chatCount);
    series->append("Alerts", alertsCount);
}

void AIUsageDashboard::refresh()
{
    // Subscription status
    const bool subActive = m_monetization && m_monetization->isSubscriptionActive();
    m_statusLabel->setText(subActive ? tr("Subscription Status: Active") : tr("Subscription Status: Inactive"));

    // Time remaining
    if (m_monetization) {
        QDateTime now = QDateTime::currentDateTime();
        QDateTime due = m_monetization->subscriptionRenewalDue();
        qint64 secs = due.isValid() ? now.secsTo(due) : 0;
        m_timeRemainingLabel->setText(tr("Time Remaining: %1").arg(formatDuration(secs)));
    }

    // Read usage metrics from QSettings (or replace with real counters)
    QSettings s;
    int aiMessages = s.value("aiwallet/metrics/messages", 0).toInt();
    int chats = s.value("aiwallet/metrics/chats", 0).toInt();
    int alerts = s.value("aiwallet/metrics/alerts", 0).toInt();
    QString lang = s.value("aiassistant/language", "en").toString();
    double savings = s.value("aiwallet/metrics/savings", 0.0).toDouble();

    m_messagesLabel->setText(tr("AI Messages Received: %1").arg(aiMessages));
    m_chatsLabel->setText(tr("Smart Chats Used: %1").arg(chats));
    m_notificationsLabel->setText(tr("Notifications Triggered: %1").arg(alerts));
    m_languageLabel->setText(tr("Language Used: %1").arg(lang));
    m_savingsLabel->setText(tr("Estimated SHAH Saved: %1").arg(QString::number(savings, 'f', 4)));

    updateChart(aiMessages, chats, alerts);
    updateOverlay();
}

void AIUsageDashboard::updateOverlay()
{
    const bool expired = m_monetization && m_monetization->isSubscriptionActive() == false;
    m_overlay->setVisible(expired);
}

void AIUsageDashboard::onRenewClicked()
{
    if (!m_monetization) return;
    m_monetization->setSubscriptionRenewalDue(QDateTime::currentDateTime().addDays(30));
    refresh();
}

void AIUsageDashboard::onUpgradeClicked()
{
    // Placeholder: open Upgrade modal
    QMessageBox::information(this, tr("Upgrade"), tr("Open Upgrade/Subscribe flow…"));
}


