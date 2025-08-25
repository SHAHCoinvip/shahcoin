// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_STAKINGPAGE_H
#define SHAHCOIN_QT_STAKINGPAGE_H

#include <QWidget>
#include <QTimer>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

class ClientModel;
class WalletModel;
class PlatformStyle;

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QGroupBox;
class QTableWidget;
class QTableWidgetItem;
class QLabel;
class QPushButton;
class QProgressBar;
QT_END_NAMESPACE

/**
 * @brief The StakingPage class provides a comprehensive staking interface for SHAHCOIN Core
 * 
 * This page allows users to:
 * - View their staking balance and estimated rewards
 * - Start and stop staking operations
 * - Monitor active stakes and staking history
 * - View staking statistics and performance
 */
class StakingPage : public QWidget
{
    Q_OBJECT

public:
    explicit StakingPage(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~StakingPage();

    void setClientModel(ClientModel* clientModel);
    void setWalletModel(WalletModel* walletModel);

    void showOutOfSyncWarning(bool fShow);

public Q_SLOTS:
    void updateStakingInfo();
    void updateActiveStakes();
    void updateStakingHistory();
    void onStartStaking();
    void onStopStaking();
    void onClaimRewards();
    void onRefresh();

Q_SIGNALS:
    void message(const QString& title, const QString& message, unsigned int style);

private Q_SLOTS:
    void updateDisplayUnit();
    void updateStakingStatus();

private:
    void setupUI();
    void connectSignals();
    void updateBalanceLabels();
    void updateStakeTable();
    void updateHistoryTable();
    void setStakingEnabled(bool enabled);
    void showStakingError(const QString& message);
    void showStakingSuccess(const QString& message);

    // UI Components
    QLabel* label_TotalBalanceValue;
    QLabel* label_StakedBalanceValue;
    QLabel* label_StakeableBalanceValue;
    QLabel* label_EstimatedRewardsValue;
    QLabel* label_StakeAgeValue;
    QLabel* label_MinStakeValue;

    QPushButton* pushButton_Stake;
    QPushButton* pushButton_StopStaking;
    QPushButton* pushButton_ClaimRewards;
    QPushButton* pushButton_Refresh;

    QTableWidget* tableWidget_ActiveStakes;
    QTableWidget* tableWidget_StakingHistory;

    // Models
    ClientModel* clientModel{nullptr};
    WalletModel* walletModel{nullptr};
    const PlatformStyle* platformStyle;

    // Timer for periodic updates
    QTimer* updateTimer;

    // Staking state
    bool stakingEnabled{false};
    CAmount totalBalance{0};
    CAmount stakedBalance{0};
    CAmount stakeableBalance{0};
    CAmount estimatedRewards{0};

    // Constants
    static constexpr CAmount MIN_STAKE_AMOUNT = 333 * COIN;
    static constexpr int STAKE_AGE_HOURS = 12;
    static constexpr int UPDATE_INTERVAL_MS = 30000; // 30 seconds
};

#endif // SHAHCOIN_QT_STAKINGPAGE_H
