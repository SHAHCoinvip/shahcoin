#ifndef SHAHCOIN_QT_FIATVALUEDISPLAY_H
#define SHAHCOIN_QT_FIATVALUEDISPLAY_H

#include <QObject>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class WalletModel;
class QWidget;

/**
 * @brief Fiat Value Display for SHAH
 * 
 * Shows wallet balance in SHAH and estimated USD value.
 * Pulls price from ShahSwap or external API (mock for now).
 * Provides UI toggle to show/hide fiat values.
 */
class FiatValueDisplay : public QObject
{
    Q_OBJECT

public:
    explicit FiatValueDisplay(QObject* parent = nullptr);
    ~FiatValueDisplay();

    void setWalletModel(WalletModel* walletModel);
    void setBalanceLabel(QLabel* balanceLabel);
    void setFiatToggle(QCheckBox* fiatToggle);
    
    QString getFormattedBalance() const;
    QString getFormattedFiatValue() const;
    double getShahPrice() const { return m_shahPrice; }
    bool isFiatEnabled() const { return m_fiatEnabled; }

public Q_SLOTS:
    void onFiatToggleChanged(bool enabled);
    void onBalanceChanged();
    void onPriceUpdated();
    void refreshPrice();

Q_SIGNALS:
    void priceUpdated(double price);
    void fiatValueChanged(const QString& fiatValue);
    void displayFormatChanged();

private Q_SLOTS:
    void onNetworkReplyFinished(QNetworkReply* reply);
    void onPriceUpdateTimeout();

private:
    void setupPriceUpdate();
    void loadSettings();
    void saveSettings();
    void updateDisplay();
    void fetchPriceFromAPI();
    void updatePriceFromMock();
    QString formatShahAmount(qint64 amount) const;
    QString formatFiatAmount(double amount) const;
    double calculateFiatValue(qint64 shahAmount) const;

    WalletModel* m_walletModel{nullptr};
    QLabel* m_balanceLabel{nullptr};
    QCheckBox* m_fiatToggle{nullptr};
    
    QNetworkAccessManager* m_networkManager{nullptr};
    QTimer* m_priceUpdateTimer{nullptr};
    
    double m_shahPrice{0.0};
    qint64 m_currentBalance{0};
    bool m_fiatEnabled{false};
    bool m_priceUpdateInProgress{false};
    
    // Constants
    static constexpr int PRICE_UPDATE_INTERVAL_MS = 60000; // 1 minute
    static constexpr double MOCK_SHAH_PRICE = 0.25; // Mock price for demo
    static constexpr double PRICE_VOLATILITY = 0.05; // 5% volatility for mock updates
};

/**
 * @brief Enhanced Balance Label with Fiat Value
 */
class FiatBalanceLabel : public QLabel
{
    Q_OBJECT

public:
    explicit FiatBalanceLabel(QWidget* parent = nullptr);
    
    void setShahAmount(qint64 amount);
    void setFiatEnabled(bool enabled);
    void setShahPrice(double price);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void updateDisplay();
    QString formatShahAmount() const;
    QString formatFiatAmount() const;

    qint64 m_shahAmount{0};
    double m_shahPrice{0.0};
    bool m_fiatEnabled{false};
    QString m_shahFormatted;
    QString m_fiatFormatted;
};

/**
 * @brief Fiat Value Toggle Widget
 */
class FiatValueToggle : public QCheckBox
{
    Q_OBJECT

public:
    explicit FiatValueToggle(QWidget* parent = nullptr);

public Q_SLOTS:
    void onToggleChanged(bool checked);

Q_SIGNALS:
    void fiatDisplayToggled(bool enabled);

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
};

#endif // SHAHCOIN_QT_FIATVALUEDISPLAY_H
