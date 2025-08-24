#ifndef SHAHCOIN_QT_AIACCESSGATE_H
#define SHAHCOIN_QT_AIACCESSGATE_H

#include <QObject>

class QWidget;
class WalletModel;
class MonetizationManager;

/**
 * AIAccessGate
 *
 * Centralized helper to enforce AI Wallet access controls.
 * If access is insufficient, shows UpgradeAIPanel.
 */
class AIAccessGate : public QObject
{
    Q_OBJECT
public:
    explicit AIAccessGate(MonetizationManager* monetization, QObject* parent = nullptr);

    bool ensureAccess(QWidget* parent, WalletModel* walletModel);

private:
    MonetizationManager* m_monetization;
};

#endif // SHAHCOIN_QT_AIACCESSGATE_H


