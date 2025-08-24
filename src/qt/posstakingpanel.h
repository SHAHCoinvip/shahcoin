// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#ifndef POSSTAKINGPANEL_H
#define POSSTAKINGPANEL_H

#include <QWidget>
#include <QTimer>
#include <QTableWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QSlider>
#include <QChart>
#include <QChartView>
#include <QLineSeries>

QT_CHARTS_USE_NAMESPACE

class ClientModel;
class WalletModel;

/**
 * PoS Staking Panel
 * 
 * Provides comprehensive Proof of Stake functionality including:
 * - Stake status monitoring
 * - Cold/Hot staking options
 * - Reward claiming
 * - Compounding toggle
 * - Staking weight calculation
 * - Probability estimation
 * - Expected rewards calculation
 * - Staking simulator
 */
class PoSStakingPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PoSStakingPanel(QWidget *parent = nullptr);
    ~PoSStakingPanel();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);

private slots:
    void updateStakingStatus();
    void updateStakingStats();
    void updateRewardSimulation();
    void onStakeToggleClicked();
    void onClaimRewardsClicked();
    void onCompoundingToggled(bool checked);
    void onColdStakingToggled(bool checked);
    void onSimulateRewardsClicked();
    void onStakingAmountChanged(double amount);
    void onStakingTimeChanged(int days);
    void onRefreshClicked();

private:
    void setupUI();
    void setupStakingStatus();
    void setupStakingControls();
    void setupRewardSimulator();
    void setupStakingHistory();
    void setupCharts();
    void connectSignals();
    
    // Data calculation methods
    void calculateStakingWeight();
    void calculateStakingProbability();
    void calculateExpectedRewards();
    void updateStakingHistory();
    
    // Validation methods
    bool validateStakingAmount(double amount);
    bool validateStakingTime(int days);
    bool checkStakingEligibility();

    // UI Components
    QVBoxLayout *m_mainLayout;
    
    // Staking Status Section
    QGroupBox *m_stakingStatusGroup;
    QGridLayout *m_statusLayout;
    QLabel *m_stakingEnabledLabel;
    QLabel *m_totalStakeLabel;
    QLabel *m_stakingWeightLabel;
    QLabel *m_stakingProbabilityLabel;
    QLabel *m_lastRewardLabel;
    QLabel *m_totalRewardsLabel;
    QProgressBar *m_stakingProgressBar;
    QLabel *m_stakingStatusLabel;
    
    // Staking Controls Section
    QGroupBox *m_stakingControlsGroup;
    QVBoxLayout *m_controlsLayout;
    QPushButton *m_stakeToggleButton;
    QCheckBox *m_compoundingCheck;
    QCheckBox *m_coldStakingCheck;
    QPushButton *m_claimRewardsButton;
    QPushButton *m_refreshButton;
    
    // Reward Simulator Section
    QGroupBox *m_simulatorGroup;
    QGridLayout *m_simulatorLayout;
    QDoubleSpinBox *m_stakingAmountSpin;
    QSpinBox *m_stakingTimeSpin;
    QComboBox *m_stakingTypeCombo;
    QPushButton *m_simulateButton;
    QLabel *m_expectedRewardLabel;
    QLabel *m_annualReturnLabel;
    QLabel *m_probabilityLabel;
    QLabel *m_breakEvenLabel;
    
    // Staking History Section
    QGroupBox *m_historyGroup;
    QVBoxLayout *m_historyLayout;
    QTableWidget *m_stakingHistoryTable;
    QChartView *m_rewardsChart;
    
    // Staking Statistics Section
    QGroupBox *m_statsGroup;
    QGridLayout *m_statsLayout;
    QLabel *m_totalStakedLabel;
    QLabel *m_availableForStakingLabel;
    QLabel *m_stakingEfficiencyLabel;
    QLabel *m_networkStakeLabel;
    QLabel *m_avgRewardLabel;
    QLabel *m_stakingAPYLabel;
    
    // Data storage
    struct StakingData {
        double totalStake;
        double stakingWeight;
        double stakingProbability;
        double lastReward;
        double totalRewards;
        bool isStaking;
        bool isCompounding;
        bool isColdStaking;
        int eligibleCoins;
        double avgCoinAge;
    };
    
    StakingData m_stakingData;
    
    // Models
    ClientModel *m_clientModel;
    WalletModel *m_walletModel;
    
    // Timer for updates
    QTimer *m_updateTimer;
    
    // Chart objects
    QChart *m_rewardsChartObj;
    QLineSeries *m_rewardsSeries;
    
    // Simulation data
    struct SimulationResult {
        double expectedReward;
        double annualReturn;
        double probability;
        int breakEvenDays;
        double totalRewards;
    };
    
    SimulationResult m_simulationResult;
};

#endif // POSSTAKINGPANEL_H
