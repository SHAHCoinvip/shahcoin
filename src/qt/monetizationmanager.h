#ifndef SHAHCOIN_QT_MONETIZATIONMANAGER_H
#define SHAHCOIN_QT_MONETIZATIONMANAGER_H

#include <QObject>
#include <QSettings>
#include <QDateTime>
#include <QMap>
#include <QString>

class QWidget;
class WalletModel;

/**
 * MonetizationManager
 *
 * Manages AI Wallet access levels, premium upgrades, and prepaid credits.
 * Stores state in QSettings keyed under "aiwallet/*" for simplicity.
 */
class MonetizationManager : public QObject
{
    Q_OBJECT

public:
    enum Feature {
        Assistant,
        Insights,
        Troubleshooter,
        FeeOptimizer,
        Multilingual
    };

    enum AccessLevel {
        Locked,
        Basic,      // Free if user holds ≥ $10 SHAH
        Premium     // Purchased or Subscribed (Stripe or on-chain)
    };

    explicit MonetizationManager(QObject* parent = nullptr);

    // Access checks
    AccessLevel getAccessLevel(WalletModel* walletModel) const;
    bool hasBasicAccess(WalletModel* walletModel) const;
    bool hasPremiumAccess() const;
    bool isFeatureAllowed(Feature feature, WalletModel* walletModel) const;

    // Credits
    int getCreditBalance() const;
    void setCreditBalance(int credits);
    bool consumeCredit(int amount = 1);

    // Premium unlock
    void setPremiumUnlocked(bool unlocked, const QString& source = QString());
    bool isPremiumUnlocked() const;
    QString premiumSource() const;
    QDateTime premiumSince() const;

    // Subscription (monthly)
    void setSubscriptionActive(bool active, const QString& source = QString());
    bool isSubscriptionActive() const;
    QString subscriptionSource() const;
    QDateTime subscriptionSince() const;
    QDateTime subscriptionRenewalDue() const;
    void setSubscriptionRenewalDue(const QDateTime& due);
    double getMonthlyPriceUsd() const; // default 4.99
    void setMonthlyPriceUsd(double priceUsd);

    // Pricing and thresholds
    double getSHAHUsdPrice() const; // USD per SHAH
    void setSHAHUsdPrice(double price);
    double getBasicThresholdUsd() const; // default 10.0
    double getPremiumPriceUsd() const;   // default 9.99
    double getRequiredShahForMonthly() const; // computed from monthly price

    // Utility
    double getWalletUsdValue(WalletModel* walletModel) const;
    static QString featureName(Feature f);

signals:
    void entitlementsChanged();
    void creditsChanged(int credits);
    void priceUpdated(double usdPerShah);

private:
    void loadSettings();
    void saveSettings() const;

    // Settings-backed fields
    bool m_premiumUnlocked;
    QString m_premiumSource;
    QDateTime m_premiumSince;
    bool m_subscriptionActive;
    QString m_subscriptionSource;
    QDateTime m_subscriptionSince;
    QDateTime m_subscriptionRenewalDue;
    int m_credits;
    double m_usdPerShah;
    double m_basicThresholdUsd;
    double m_premiumPriceUsd;
    double m_monthlyPriceUsd;
};

#endif // SHAHCOIN_QT_MONETIZATIONMANAGER_H


