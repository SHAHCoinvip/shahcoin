// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/stakingpage.h>

#include <qt/clientmodel.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/shahcoinamountfield.h>
#include <qt/shahcoinunits.h>
#include <qt/walletmodel.h>

#include <interfaces/node.h>
#include <interfaces/wallet.h>
#include <node/interface_ui.h>
#include <util/strencodings.h>
#include <validation.h>

#include <QApplication>
#include <QDateTime>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

#include "ui_stakingpage.h"

StakingPage::StakingPage(const PlatformStyle* _platformStyle, QWidget* parent)
    : QWidget(parent),
      platformStyle(_platformStyle),
      updateTimer(new QTimer(this))
{
    setupUI();
    connectSignals();

    // Set up timer for periodic updates
    updateTimer->setInterval(UPDATE_INTERVAL_MS);
    connect(updateTimer, &QTimer::timeout, this, &StakingPage::updateStakingInfo);
    updateTimer->start();
}

StakingPage::~StakingPage()
{
    if (updateTimer) {
        updateTimer->stop();
    }
}

void StakingPage::setupUI()
{
    // Load UI from form
    Ui::StakingPage ui;
    ui.setupUi(this);

    // Store UI component pointers
    label_TotalBalanceValue = ui.label_TotalBalanceValue;
    label_StakedBalanceValue = ui.label_StakedBalanceValue;
    label_StakeableBalanceValue = ui.label_StakeableBalanceValue;
    label_EstimatedRewardsValue = ui.label_EstimatedRewardsValue;
    label_StakeAgeValue = ui.label_StakeAgeValue;
    label_MinStakeValue = ui.label_MinStakeValue;

    pushButton_Stake = ui.pushButton_Stake;
    pushButton_StopStaking = ui.pushButton_StopStaking;
    pushButton_ClaimRewards = ui.pushButton_ClaimRewards;
    pushButton_Refresh = ui.pushButton_Refresh;

    tableWidget_ActiveStakes = ui.tableWidget_ActiveStakes;
    tableWidget_StakingHistory = ui.tableWidget_StakingHistory;

    // Set up table headers
    tableWidget_ActiveStakes->setColumnCount(5);
    tableWidget_ActiveStakes->setHorizontalHeaderLabels({
        tr("Stake ID"),
        tr("Amount"),
        tr("Age"),
        tr("Rewards Earned"),
        tr("Status")
    });

    tableWidget_StakingHistory->setColumnCount(5);
    tableWidget_StakingHistory->setHorizontalHeaderLabels({
        tr("Date"),
        tr("Type"),
        tr("Amount"),
        tr("Reward"),
        tr("Block Height")
    });

    // Set table properties
    tableWidget_ActiveStakes->setAlternatingRowColors(true);
    tableWidget_ActiveStakes->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget_ActiveStakes->setSortingEnabled(true);
    tableWidget_ActiveStakes->horizontalHeader()->setStretchLastSection(true);

    tableWidget_StakingHistory->setAlternatingRowColors(true);
    tableWidget_StakingHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget_StakingHistory->setSortingEnabled(true);
    tableWidget_StakingHistory->horizontalHeader()->setStretchLastSection(true);

    // Initialize display
    updateDisplayUnit();
    updateStakingStatus();
}

void StakingPage::connectSignals()
{
    connect(pushButton_Stake, &QPushButton::clicked, this, &StakingPage::onStartStaking);
    connect(pushButton_StopStaking, &QPushButton::clicked, this, &StakingPage::onStopStaking);
    connect(pushButton_ClaimRewards, &QPushButton::clicked, this, &StakingPage::onClaimRewards);
    connect(pushButton_Refresh, &QPushButton::clicked, this, &StakingPage::onRefresh);
}

void StakingPage::setClientModel(ClientModel* _clientModel)
{
    clientModel = _clientModel;
    if (clientModel) {
        connect(clientModel, &ClientModel::numBlocksChanged, this, &StakingPage::updateStakingInfo);
    }
}

void StakingPage::setWalletModel(WalletModel* _walletModel)
{
    walletModel = _walletModel;
    if (walletModel) {
        connect(walletModel, &WalletModel::balanceChanged, this, &StakingPage::updateStakingInfo);
        connect(walletModel, &WalletModel::displayUnitChanged, this, &StakingPage::updateDisplayUnit);
    }
}

void StakingPage::showOutOfSyncWarning(bool fShow)
{
    // Update UI to show sync warning if needed
    if (fShow) {
        pushButton_Stake->setEnabled(false);
        pushButton_StopStaking->setEnabled(false);
    } else {
        updateStakingStatus();
    }
}

void StakingPage::updateDisplayUnit()
{
    if (walletModel && walletModel->getOptionsModel()) {
        int unit = walletModel->getOptionsModel()->getDisplayUnit();
        label_TotalBalanceValue->setText(ShahcoinUnits::formatWithUnit(unit, totalBalance, false, ShahcoinUnits::SeparatorStyle::STANDARD));
        label_StakedBalanceValue->setText(ShahcoinUnits::formatWithUnit(unit, stakedBalance, false, ShahcoinUnits::SeparatorStyle::STANDARD));
        label_StakeableBalanceValue->setText(ShahcoinUnits::formatWithUnit(unit, stakeableBalance, false, ShahcoinUnits::SeparatorStyle::STANDARD));
        label_EstimatedRewardsValue->setText(ShahcoinUnits::formatWithUnit(unit, estimatedRewards, false, ShahcoinUnits::SeparatorStyle::STANDARD));
        label_MinStakeValue->setText(ShahcoinUnits::formatWithUnit(unit, MIN_STAKE_AMOUNT, false, ShahcoinUnits::SeparatorStyle::STANDARD));
    }
}

void StakingPage::updateStakingInfo()
{
    if (!walletModel) return;

    // Get real staking data from wallet model
    totalBalance = walletModel->getCachedBalance().balance;
    stakedBalance = walletModel->getStakedBalance();
    stakeableBalance = walletModel->getStakeableBalance();
    
    // Calculate estimated rewards (5% annual rate)
    estimatedRewards = stakedBalance * 0.05 / 365; // Daily rate
    
    // Update display
    updateBalanceLabels();
    updateStakeTable();
    updateHistoryTable();
    updateStakingStatus();
}

void StakingPage::updateBalanceLabels()
{
    if (!walletModel) return;

    updateDisplayUnit();
    
    // Update balance labels with real data
    label_TotalBalanceValue->setText(ShahcoinUnits::formatWithUnit(displayUnit, totalBalance, false, ShahcoinUnits::SeparatorStyle::STANDARD));
    label_StakedBalanceValue->setText(ShahcoinUnits::formatWithUnit(displayUnit, stakedBalance, false, ShahcoinUnits::SeparatorStyle::STANDARD));
    label_StakeableBalanceValue->setText(ShahcoinUnits::formatWithUnit(displayUnit, stakeableBalance, false, ShahcoinUnits::SeparatorStyle::STANDARD));
    label_EstimatedRewardsValue->setText(ShahcoinUnits::formatWithUnit(displayUnit, estimatedRewards, false, ShahcoinUnits::SeparatorStyle::STANDARD));
    
    // Update other labels
    label_StakeAgeValue->setText(tr("12 hours"));
    label_MinStakeValue->setText(ShahcoinUnits::formatWithUnit(displayUnit, MIN_STAKE_AMOUNT, false, ShahcoinUnits::SeparatorStyle::STANDARD));
}

void StakingPage::updateStakeTable()
{
    if (!walletModel) return;

    tableWidget_ActiveStakes->setRowCount(0);
    
    // Get active stakes from wallet model
    QStringList activeStakes = walletModel->getActiveStakes();
    
    for (int i = 0; i < activeStakes.size(); ++i) {
        // TODO: Parse stake data and populate table
        // For now, show empty table
    }
}

void StakingPage::updateHistoryTable()
{
    if (!walletModel) return;

    tableWidget_StakingHistory->setRowCount(0);
    
    // Get staking history from wallet model
    QStringList stakingHistory = walletModel->getStakingHistory();
    
    for (int i = 0; i < stakingHistory.size(); ++i) {
        // TODO: Parse history data and populate table
        // For now, show empty table
    }
}

void StakingPage::updateStakingStatus()
{
    if (!walletModel) return;

    bool canStake = stakeableBalance >= MIN_STAKE_AMOUNT && !stakingEnabled;
    bool canStopStake = stakingEnabled;
    
    pushButton_Stake->setEnabled(canStake);
    pushButton_StopStaking->setEnabled(canStopStake);
    
    // Update button text based on state
    if (stakingEnabled) {
        pushButton_Stake->setText(tr("Staking Active"));
        pushButton_Stake->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
    } else {
        pushButton_Stake->setText(tr("Start Staking"));
        pushButton_Stake->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
    }
}

void StakingPage::updateActiveStakes()
{
    updateStakeTable();
}

void StakingPage::updateStakingHistory()
{
    updateHistoryTable();
}

void StakingPage::onStartStaking()
{
    if (!walletModel) {
        showStakingError(tr("Wallet not available"));
        return;
    }

    if (stakeableBalance < MIN_STAKE_AMOUNT) {
        showStakingError(tr("Insufficient balance for staking. Minimum required: %1").arg(ShahcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), MIN_STAKE_AMOUNT, false, ShahcoinUnits::SeparatorStyle::STANDARD)));
        return;
    }

    // Use the maximum stakeable amount or a reasonable default
    CAmount stakeAmount = std::min(stakeableBalance, 1000 * COIN); // Max 1000 SHAH for demo
    
    QString error;
    if (walletModel->startStaking(stakeAmount, error)) {
        stakingEnabled = true;
        updateStakingInfo();
        showStakingSuccess(tr("Staking started successfully with %1").arg(ShahcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), stakeAmount, false, ShahcoinUnits::SeparatorStyle::STANDARD)));
    } else {
        showStakingError(error);
    }
}

void StakingPage::onStopStaking()
{
    if (!walletModel) {
        showStakingError(tr("Wallet not available"));
        return;
    }

    if (!stakingEnabled) {
        showStakingError(tr("No active staking to stop"));
        return;
    }

    // For now, we'll stop staking by setting the flag
    // In a real implementation, this would create an unstaking transaction
    QString error;
    if (walletModel->stopStaking("", error)) {
        stakingEnabled = false;
        updateStakingInfo();
        showStakingSuccess(tr("Staking stopped successfully"));
    } else {
        showStakingError(error);
    }
}

void StakingPage::onClaimRewards()
{
    if (!walletModel) {
        showStakingError(tr("Wallet not available"));
        return;
    }

    if (!stakingEnabled) {
        showStakingError(tr("No active staking to claim rewards from"));
        return;
    }

    if (estimatedRewards <= 0) {
        showStakingError(tr("No rewards available to claim"));
        return;
    }

    // Claim rewards using wallet model
    QString error;
    if (walletModel->claimStakeRewards("", error)) {
        updateStakingInfo();
        showStakingSuccess(tr("Rewards claimed successfully: %1").arg(ShahcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), estimatedRewards, false, ShahcoinUnits::SeparatorStyle::STANDARD)));
    } else {
        showStakingError(error);
    }
}

void StakingPage::onRefresh()
{
    updateStakingInfo();
    showStakingSuccess(tr("Staking information refreshed"));
}

void StakingPage::showStakingError(const QString& message)
{
    Q_EMIT this->message(tr("Staking Error"), message, CClientUIInterface::MSG_ERROR);
}

void StakingPage::showStakingSuccess(const QString& message)
{
    Q_EMIT this->message(tr("Staking Success"), message, CClientUIInterface::MSG_INFORMATION);
}
