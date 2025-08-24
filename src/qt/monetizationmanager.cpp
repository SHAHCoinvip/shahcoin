#include "monetizationmanager.h"
#include <qt/walletmodel.h>
#include <qt/shahcoinunits.h>
#include <QSettings>
#include <QDateTime>

MonetizationManager::MonetizationManager(QObject* parent)
    : QObject(parent)
    , m_premiumUnlocked(false)
    , m_premiumSource("")
    , m_subscriptionActive(false)
    , m_credits(0)
    , m_usdPerShah(0.10) // default fallback $0.10 per SHAH
    , m_basicThresholdUsd(10.0)
    , m_premiumPriceUsd(9.99)
    , m_monthlyPriceUsd(4.99)
{
    loadSettings();
}

void MonetizationManager::loadSettings()
{
    QSettings s;
    m_premiumUnlocked = s.value("aiwallet/premiumUnlocked", false).toBool();
    m_premiumSource = s.value("aiwallet/premiumSource", "").toString();
    m_premiumSince = s.value("aiwallet/premiumSince").toDateTime();
    m_subscriptionActive = s.value("aiwallet/subActive", false).toBool();
    m_subscriptionSource = s.value("aiwallet/subSource", "").toString();
    m_subscriptionSince = s.value("aiwallet/subSince").toDateTime();
    m_subscriptionRenewalDue = s.value("aiwallet/subRenewalDue").toDateTime();
    m_credits = s.value("aiwallet/credits", 0).toInt();
    m_usdPerShah = s.value("aiwallet/usdPerShah", m_usdPerShah).toDouble();
    m_basicThresholdUsd = s.value("aiwallet/basicThresholdUsd", m_basicThresholdUsd).toDouble();
    m_premiumPriceUsd = s.value("aiwallet/premiumPriceUsd", m_premiumPriceUsd).toDouble();
    m_monthlyPriceUsd = s.value("aiwallet/monthlyPriceUsd", m_monthlyPriceUsd).toDouble();
}

void MonetizationManager::saveSettings() const
{
    QSettings s;
    s.setValue("aiwallet/premiumUnlocked", m_premiumUnlocked);
    s.setValue("aiwallet/premiumSource", m_premiumSource);
    s.setValue("aiwallet/premiumSince", m_premiumSince);
    s.setValue("aiwallet/subActive", m_subscriptionActive);
    s.setValue("aiwallet/subSource", m_subscriptionSource);
    s.setValue("aiwallet/subSince", m_subscriptionSince);
    s.setValue("aiwallet/subRenewalDue", m_subscriptionRenewalDue);
    s.setValue("aiwallet/credits", m_credits);
    s.setValue("aiwallet/usdPerShah", m_usdPerShah);
    s.setValue("aiwallet/basicThresholdUsd", m_basicThresholdUsd);
    s.setValue("aiwallet/premiumPriceUsd", m_premiumPriceUsd);
    s.setValue("aiwallet/monthlyPriceUsd", m_monthlyPriceUsd);
}

MonetizationManager::AccessLevel MonetizationManager::getAccessLevel(WalletModel* walletModel) const
{
    if (m_subscriptionActive) return Premium;
    if (m_premiumUnlocked) return Premium;
    if (hasBasicAccess(walletModel)) return Basic;
    return Locked;
}

bool MonetizationManager::hasBasicAccess(WalletModel* walletModel) const
{
    if (!walletModel) return false;
    const double usd = getWalletUsdValue(walletModel);
    return usd >= m_basicThresholdUsd;
}

bool MonetizationManager::hasPremiumAccess() const
{
    return m_premiumUnlocked || isSubscriptionActive();
}

bool MonetizationManager::isFeatureAllowed(Feature feature, WalletModel* walletModel) const
{
    Q_UNUSED(feature);
    if (m_subscriptionActive) return true;
    return getAccessLevel(walletModel) != Locked;
}

int MonetizationManager::getCreditBalance() const
{
    return m_credits;
}

void MonetizationManager::setCreditBalance(int credits)
{
    if (m_credits == credits) return;
    m_credits = qMax(0, credits);
    saveSettings();
    emit creditsChanged(m_credits);
}

bool MonetizationManager::consumeCredit(int amount)
{
    if (amount <= 0) amount = 1;
    if (m_credits < amount) return false;
    m_credits -= amount;
    saveSettings();
    emit creditsChanged(m_credits);
    return true;
}

void MonetizationManager::setPremiumUnlocked(bool unlocked, const QString& source)
{
    if (m_premiumUnlocked == unlocked && (unlocked ? (m_premiumSource == source) : true)) return;
    m_premiumUnlocked = unlocked;
    m_premiumSource = unlocked ? source : QString();
    m_premiumSince = unlocked ? QDateTime::currentDateTime() : QDateTime();
    saveSettings();
    emit entitlementsChanged();
}

bool MonetizationManager::isPremiumUnlocked() const
{
    return m_premiumUnlocked;
}

QString MonetizationManager::premiumSource() const
{
    return m_premiumSource;
}

QDateTime MonetizationManager::premiumSince() const
{
    return m_premiumSince;
}

void MonetizationManager::setSubscriptionActive(bool active, const QString& source)
{
    if (m_subscriptionActive == active && (active ? (m_subscriptionSource == source) : true)) return;
    m_subscriptionActive = active;
    m_subscriptionSource = active ? source : QString();
    m_subscriptionSince = active ? QDateTime::currentDateTime() : QDateTime();
    if (active && m_subscriptionRenewalDue.isNull()) {
        m_subscriptionRenewalDue = QDateTime::currentDateTime().addDays(30);
    }
    saveSettings();
    emit entitlementsChanged();
}

bool MonetizationManager::isSubscriptionActive() const
{
    if (m_subscriptionActive && !m_subscriptionRenewalDue.isNull() && QDateTime::currentDateTime() > m_subscriptionRenewalDue) {
        return false;
    }
    return m_subscriptionActive;
}

QString MonetizationManager::subscriptionSource() const { return m_subscriptionSource; }
QDateTime MonetizationManager::subscriptionSince() const { return m_subscriptionSince; }
QDateTime MonetizationManager::subscriptionRenewalDue() const { return m_subscriptionRenewalDue; }
void MonetizationManager::setSubscriptionRenewalDue(const QDateTime& due) { m_subscriptionRenewalDue = due; saveSettings(); }

double MonetizationManager::getMonthlyPriceUsd() const { return m_monthlyPriceUsd; }
void MonetizationManager::setMonthlyPriceUsd(double priceUsd) { if (priceUsd>0) { m_monthlyPriceUsd = priceUsd; saveSettings(); } }

double MonetizationManager::getSHAHUsdPrice() const
{
    return m_usdPerShah;
}

void MonetizationManager::setSHAHUsdPrice(double price)
{
    if (price <= 0) return;
    if (qFuzzyCompare(m_usdPerShah, price)) return;
    m_usdPerShah = price;
    saveSettings();
    emit priceUpdated(m_usdPerShah);
}

double MonetizationManager::getBasicThresholdUsd() const
{
    return m_basicThresholdUsd;
}

double MonetizationManager::getPremiumPriceUsd() const
{
    return m_premiumPriceUsd;
}

double MonetizationManager::getRequiredShahForMonthly() const
{
    if (m_usdPerShah <= 0) return 0.0;
    return m_monthlyPriceUsd / m_usdPerShah;
}

double MonetizationManager::getWalletUsdValue(WalletModel* walletModel) const
{
    if (!walletModel) return 0.0;
    const CAmount balance = walletModel->getBalance();
    const double shah = static_cast<double>(balance) / static_cast<double>(COIN);
    return shah * m_usdPerShah;
}

QString MonetizationManager::featureName(Feature f)
{
    switch (f) {
        case Assistant: return "AI Assistant";
        case Insights: return "AI Insights";
        case Troubleshooter: return "AI Troubleshooter";
        case FeeOptimizer: return "Fee Optimizer";
        case Multilingual: return "Multilingual Assistant";
    }
    return "Feature";
}


