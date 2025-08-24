#ifndef SHAHCOIN_QT_PRICEORACLE_H
#define SHAHCOIN_QT_PRICEORACLE_H

#include <QObject>
#include <QTimer>

/**
 * PriceOracle
 *
 * Lightweight SHAH/USD price oracle for local UI logic.
 * Currently reads a value from QSettings (key: "shahswap/usdPerShah")
 * and emits updates periodically. Can be extended to read pool data.
 */
class PriceOracle : public QObject
{
    Q_OBJECT

public:
    explicit PriceOracle(QObject* parent = nullptr);

    double currentUsdPerShah() const { return m_usdPerShah; }
    void setManualPrice(double usdPerShah);

signals:
    void priceUpdated(double usdPerShah);

public slots:
    void start(int intervalMs = 30000); // default 30s
    void stop();
    void refresh();

private:
    void loadFromSettings();

    QTimer m_timer;
    double m_usdPerShah;
};

#endif // SHAHCOIN_QT_PRICEORACLE_H


