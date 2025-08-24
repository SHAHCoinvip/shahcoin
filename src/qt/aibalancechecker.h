#ifndef SHAHCOIN_QT_AIBALANCECHECKER_H
#define SHAHCOIN_QT_AIBALANCECHECKER_H

#include <QObject>
#include <QTimer>

class WalletModel;
class MonetizationManager;

/**
 * AIBalanceChecker
 *
 * Periodically checks if wallet meets free access threshold (≥ $10 SHAH).
 */
class AIBalanceChecker : public QObject
{
    Q_OBJECT

public:
    explicit AIBalanceChecker(QObject* parent = nullptr);

    void setModels(WalletModel* walletModel, MonetizationManager* monetization);
    void start(int intervalMs = 15000);
    void stop();

signals:
    void freeAccessChanged(bool hasFreeAccess, double walletUsd);

public slots:
    void refresh();

private:
    WalletModel* m_walletModel;
    MonetizationManager* m_monetization;
    QTimer m_timer;
    bool m_lastFreeAccess;
};

#endif // SHAHCOIN_QT_AIBALANCECHECKER_H


