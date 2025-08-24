#include "aiaccessgate.h"
#include "monetizationmanager.h"
#include "upgradeaipanel.h"
#include <qt/walletmodel.h>

AIAccessGate::AIAccessGate(MonetizationManager* monetization, QObject* parent)
    : QObject(parent)
    , m_monetization(monetization)
{
}

bool AIAccessGate::ensureAccess(QWidget* parent, WalletModel* walletModel)
{
    if (!m_monetization) return true;
    const auto level = m_monetization->getAccessLevel(walletModel);
    if (level != MonetizationManager::Locked) return true;

    // Show upgrade modal
    UpgradeAIPanel dlg(walletModel, m_monetization, parent);
    return dlg.exec() == QDialog::Accepted;
}


