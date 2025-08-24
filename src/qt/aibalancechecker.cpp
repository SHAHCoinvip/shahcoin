#include "aibalancechecker.h"
#include "monetizationmanager.h"
#include <qt/walletmodel.h>

AIBalanceChecker::AIBalanceChecker(QObject* parent)
    : QObject(parent)
    , m_walletModel(nullptr)
    , m_monetization(nullptr)
    , m_lastFreeAccess(false)
{
    QObject::connect(&m_timer, &QTimer::timeout, this, &AIBalanceChecker::refresh);
}

void AIBalanceChecker::setModels(WalletModel* walletModel, MonetizationManager* monetization)
{
    m_walletModel = walletModel;
    m_monetization = monetization;
}

void AIBalanceChecker::start(int intervalMs)
{
    if (intervalMs < 2000) intervalMs = 2000;
    m_timer.start(intervalMs);
    refresh();
}

void AIBalanceChecker::stop()
{
    m_timer.stop();
}

void AIBalanceChecker::refresh()
{
    if (!m_walletModel || !m_monetization) return;
    const double usd = m_monetization->getWalletUsdValue(m_walletModel);
    const bool freeAccess = usd >= m_monetization->getBasicThresholdUsd();
    if (freeAccess != m_lastFreeAccess) {
        m_lastFreeAccess = freeAccess;
        emit freeAccessChanged(freeAccess, usd);
    }
}


