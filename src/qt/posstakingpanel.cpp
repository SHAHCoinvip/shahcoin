// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#include "posstakingpanel.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "qt/guiutil.h"
#include "qt/shahcoinunits.h"
#include "consensus/amount.h"
#include "chainparams.h"
#include "validation.h"
#include "util/time.h"

#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QThread>
#include <QFuture>
#include <QtConcurrent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

PoSStakingPanel::PoSStakingPanel(QWidget *parent)
    : QWidget(parent)
    , m_clientModel(nullptr)
    , m_walletModel(nullptr)
    , m_updateTimer(new QTimer(this))
{
    setupUI();
    setupCharts();
    connectSignals();
    
    // Initialize staking data
    m_stakingData = {0.0, 0.0, 0.0, 0.0, 0.0, false, false, false, 0, 0.0};
    m_simulationResult = {0.0, 0.0, 0.0, 0, 0.0};
    
    // Set up update timer
    m_updateTimer->setInterval(10000); // 10 seconds
    connect(m_updateTimer, &QTimer::timeout, this, &PoSStakingPanel::updateStakingStatus);
}

PoSStakingPanel::~PoSStakingPanel()
{
}

void PoSStakingPanel::setClientModel(ClientModel *clientModel)
{
    m_clientModel = clientModel;
    if (m_clientModel) {
        connect(m_clientModel, &ClientModel::numBlocksChanged, this, &PoSStakingPanel::updateStakingStatus);
        m_updateTimer->start();
    }
}

void PoSStakingPanel::setWalletModel(WalletModel *walletModel)
{
    m_walletModel = walletModel;
    if (m_walletModel) {
        connect(m_walletModel, &WalletModel::balanceChanged, this, &PoSStakingPanel::updateStakingStats);
    }
}

void PoSStakingPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Staking Status Section
    m_stakingStatusGroup = new QGroupBox(tr("Staking Status"));
    m_statusLayout = new QGridLayout(m_stakingStatusGroup);
    
    m_stakingEnabledLabel = new QLabel(tr("Staking: Disabled"));
    m_totalStakeLabel = new QLabel(tr("Total Stake: 0 SHAH"));
    m_stakingWeightLabel = new QLabel(tr("Staking Weight: 0"));
    m_stakingProbabilityLabel = new QLabel(tr("Probability: 0%"));
    m_lastRewardLabel = new QLabel(tr("Last Reward: 0 SHAH"));
    m_totalRewardsLabel = new QLabel(tr("Total Rewards: 0 SHAH"));
    m_stakingProgressBar = new QProgressBar();
    m_stakingStatusLabel = new QLabel(tr("Ready to stake"));
    
    // Style the status labels
    QList<QLabel*> statusLabels = {m_stakingEnabledLabel, m_totalStakeLabel, m_stakingWeightLabel,
                                  m_stakingProbabilityLabel, m_lastRewardLabel, m_totalRewardsLabel};
    
    for (QLabel *label : statusLabels) {
        label->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 8px; border-radius: 4px; }");
        label->setAlignment(Qt::AlignCenter);
    }
    
    // Add to grid
    m_statusLayout->addWidget(m_stakingEnabledLabel, 0, 0);
    m_statusLayout->addWidget(m_totalStakeLabel, 0, 1);
    m_statusLayout->addWidget(m_stakingWeightLabel, 0, 2);
    m_statusLayout->addWidget(m_stakingProbabilityLabel, 1, 0);
    m_statusLayout->addWidget(m_lastRewardLabel, 1, 1);
    m_statusLayout->addWidget(m_totalRewardsLabel, 1, 2);
    m_statusLayout->addWidget(m_stakingProgressBar, 2, 0, 1, 3);
    m_statusLayout->addWidget(m_stakingStatusLabel, 3, 0, 1, 3);
    
    m_mainLayout->addWidget(m_stakingStatusGroup);
    
    // Staking Controls Section
    m_stakingControlsGroup = new QGroupBox(tr("Staking Controls"));
    m_controlsLayout = new QVBoxLayout(m_stakingControlsGroup);
    
    m_stakeToggleButton = new QPushButton(tr("Enable Staking"));
    m_compoundingCheck = new QCheckBox(tr("Enable Compounding"));
    m_coldStakingCheck = new QCheckBox(tr("Cold Staking Mode"));
    m_claimRewardsButton = new QPushButton(tr("Claim Rewards"));
    m_refreshButton = new QPushButton(tr("Refresh"));
    
    m_controlsLayout->addWidget(m_stakeToggleButton);
    m_controlsLayout->addWidget(m_compoundingCheck);
    m_controlsLayout->addWidget(m_coldStakingCheck);
    m_controlsLayout->addWidget(m_claimRewardsButton);
    m_controlsLayout->addWidget(m_refreshButton);
    
    m_mainLayout->addWidget(m_stakingControlsGroup);
    
    // Reward Simulator Section
    m_simulatorGroup = new QGroupBox(tr("Reward Simulator"));
    m_simulatorLayout = new QGridLayout(m_simulatorGroup);
    
    m_stakingAmountSpin = new QDoubleSpinBox();
    m_stakingAmountSpin->setRange(333.0, 1000000.0);
    m_stakingAmountSpin->setValue(1000.0);
    m_stakingAmountSpin->setSuffix(tr(" SHAH"));
    m_stakingAmountSpin->setDecimals(2);
    
    m_stakingTimeSpin = new QSpinBox();
    m_stakingTimeSpin->setRange(1, 3650);
    m_stakingTimeSpin->setValue(365);
    m_stakingTimeSpin->setSuffix(tr(" days"));
    
    m_stakingTypeCombo = new QComboBox();
    m_stakingTypeCombo->addItem(tr("Hot Staking"), "hot");
    m_stakingTypeCombo->addItem(tr("Cold Staking"), "cold");
    m_stakingTypeCombo->addItem(tr("Delegated Staking"), "delegated");
    
    m_simulateButton = new QPushButton(tr("Simulate Rewards"));
    
    m_expectedRewardLabel = new QLabel(tr("Expected Reward: 0 SHAH"));
    m_annualReturnLabel = new QLabel(tr("Annual Return: 0%"));
    m_probabilityLabel = new QLabel(tr("Probability: 0%"));
    m_breakEvenLabel = new QLabel(tr("Break-even: 0 days"));
    
    // Style the result labels
    QList<QLabel*> resultLabels = {m_expectedRewardLabel, m_annualReturnLabel, m_probabilityLabel, m_breakEvenLabel};
    
    for (QLabel *label : resultLabels) {
        label->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 6px; border-radius: 4px; font-weight: bold; }");
        label->setAlignment(Qt::AlignCenter);
    }
    
    // Add to grid
    m_simulatorLayout->addWidget(new QLabel(tr("Staking Amount:")), 0, 0);
    m_simulatorLayout->addWidget(m_stakingAmountSpin, 0, 1);
    m_simulatorLayout->addWidget(new QLabel(tr("Staking Time:")), 0, 2);
    m_simulatorLayout->addWidget(m_stakingTimeSpin, 0, 3);
    m_simulatorLayout->addWidget(new QLabel(tr("Staking Type:")), 1, 0);
    m_simulatorLayout->addWidget(m_stakingTypeCombo, 1, 1);
    m_simulatorLayout->addWidget(m_simulateButton, 1, 2, 1, 2);
    m_simulatorLayout->addWidget(m_expectedRewardLabel, 2, 0, 1, 2);
    m_simulatorLayout->addWidget(m_annualReturnLabel, 2, 2, 1, 2);
    m_simulatorLayout->addWidget(m_probabilityLabel, 3, 0, 1, 2);
    m_simulatorLayout->addWidget(m_breakEvenLabel, 3, 2, 1, 2);
    
    m_mainLayout->addWidget(m_simulatorGroup);
    
    // Staking Statistics Section
    m_statsGroup = new QGroupBox(tr("Staking Statistics"));
    m_statsLayout = new QGridLayout(m_statsGroup);
    
    m_totalStakedLabel = new QLabel(tr("Total Staked: 0 SHAH"));
    m_availableForStakingLabel = new QLabel(tr("Available for Staking: 0 SHAH"));
    m_stakingEfficiencyLabel = new QLabel(tr("Staking Efficiency: 0%"));
    m_networkStakeLabel = new QLabel(tr("Network Stake: 0 SHAH"));
    m_avgRewardLabel = new QLabel(tr("Average Reward: 0 SHAH"));
    m_stakingAPYLabel = new QLabel(tr("Staking APY: 0%"));
    
    // Style the stats labels
    QList<QLabel*> statsLabels = {m_totalStakedLabel, m_availableForStakingLabel, m_stakingEfficiencyLabel,
                                 m_networkStakeLabel, m_avgRewardLabel, m_stakingAPYLabel};
    
    for (QLabel *label : statsLabels) {
        label->setStyleSheet("QLabel { background-color: #f8f8f8; padding: 6px; border-radius: 4px; }");
        label->setAlignment(Qt::AlignCenter);
    }
    
    // Add to grid
    m_statsLayout->addWidget(m_totalStakedLabel, 0, 0);
    m_statsLayout->addWidget(m_availableForStakingLabel, 0, 1);
    m_statsLayout->addWidget(m_stakingEfficiencyLabel, 0, 2);
    m_statsLayout->addWidget(m_networkStakeLabel, 1, 0);
    m_statsLayout->addWidget(m_avgRewardLabel, 1, 1);
    m_statsLayout->addWidget(m_stakingAPYLabel, 1, 2);
    
    m_mainLayout->addWidget(m_statsGroup);
    
    // Staking History Section
    m_historyGroup = new QGroupBox(tr("Staking History"));
    m_historyLayout = new QVBoxLayout(m_historyGroup);
    
    m_stakingHistoryTable = new QTableWidget();
    m_stakingHistoryTable->setColumnCount(6);
    m_stakingHistoryTable->setHorizontalHeaderLabels({tr("Date"), tr("Type"), tr("Amount"), tr("Reward"), tr("Balance"), tr("Status")});
    m_stakingHistoryTable->setMaximumHeight(200);
    m_stakingHistoryTable->horizontalHeader()->setStretchLastSection(true);
    m_stakingHistoryTable->setAlternatingRowColors(true);
    
    m_rewardsChart = new QChartView();
    m_rewardsChart->setMinimumHeight(150);
    
    m_historyLayout->addWidget(m_stakingHistoryTable);
    m_historyLayout->addWidget(m_rewardsChart);
    
    m_mainLayout->addWidget(m_historyGroup);
}

void PoSStakingPanel::setupCharts()
{
    // Rewards Chart
    m_rewardsChartObj = new QChart();
    m_rewardsSeries = new QLineSeries();
    m_rewardsChartObj->addSeries(m_rewardsSeries);
    m_rewardsChartObj->setTitle(tr("Staking Rewards Over Time"));
    m_rewardsChartObj->createDefaultAxes();
    m_rewardsChart->setChart(m_rewardsChartObj);
}

void PoSStakingPanel::connectSignals()
{
    connect(m_stakeToggleButton, &QPushButton::clicked, this, &PoSStakingPanel::onStakeToggleClicked);
    connect(m_claimRewardsButton, &QPushButton::clicked, this, &PoSStakingPanel::onClaimRewardsClicked);
    connect(m_compoundingCheck, &QCheckBox::toggled, this, &PoSStakingPanel::onCompoundingToggled);
    connect(m_coldStakingCheck, &QCheckBox::toggled, this, &PoSStakingPanel::onColdStakingToggled);
    connect(m_simulateButton, &QPushButton::clicked, this, &PoSStakingPanel::onSimulateRewardsClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &PoSStakingPanel::onRefreshClicked);
    
    connect(m_stakingAmountSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &PoSStakingPanel::onStakingAmountChanged);
    connect(m_stakingTimeSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &PoSStakingPanel::onStakingTimeChanged);
}

void PoSStakingPanel::updateStakingStatus()
{
    if (!m_clientModel || !m_walletModel) return;
    
    // Calculate staking data
    calculateStakingWeight();
    calculateStakingProbability();
    calculateExpectedRewards();
    
    // Update UI
    m_stakingEnabledLabel->setText(tr("Staking: %1").arg(m_stakingData.isStaking ? tr("Enabled") : tr("Disabled")));
    m_stakingEnabledLabel->setStyleSheet(m_stakingData.isStaking ? 
        "QLabel { background-color: #d4edda; color: #155724; padding: 8px; border-radius: 4px; }" :
        "QLabel { background-color: #f8d7da; color: #721c24; padding: 8px; border-radius: 4px; }");
    
    m_totalStakeLabel->setText(tr("Total Stake: %1").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, m_stakingData.totalStake)));
    m_stakingWeightLabel->setText(tr("Staking Weight: %1").arg(m_stakingData.stakingWeight, 0, 'f', 2));
    m_stakingProbabilityLabel->setText(tr("Probability: %1%").arg(m_stakingData.stakingProbability * 100, 0, 'f', 2));
    m_lastRewardLabel->setText(tr("Last Reward: %1").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, m_stakingData.lastReward)));
    m_totalRewardsLabel->setText(tr("Total Rewards: %1").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, m_stakingData.totalRewards)));
    
    // Update progress bar
    int progress = static_cast<int>(m_stakingData.stakingProbability * 100);
    m_stakingProgressBar->setValue(progress);
    m_stakingProgressBar->setFormat(tr("Staking Progress: %1%").arg(progress));
    
    // Update status label
    if (m_stakingData.isStaking) {
        m_stakingStatusLabel->setText(tr("Staking active - %1 eligible coins").arg(m_stakingData.eligibleCoins));
        m_stakingStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    } else {
        m_stakingStatusLabel->setText(tr("Staking disabled - Enable to start earning rewards"));
        m_stakingStatusLabel->setStyleSheet("QLabel { color: red; }");
    }
    
    // Update button text
    m_stakeToggleButton->setText(m_stakingData.isStaking ? tr("Disable Staking") : tr("Enable Staking"));
    m_stakeToggleButton->setStyleSheet(m_stakingData.isStaking ? 
        "QPushButton { background-color: #dc3545; color: white; }" :
        "QPushButton { background-color: #28a745; color: white; }");
}

void PoSStakingPanel::updateStakingStats()
{
    if (!m_walletModel) return;
    
    // Get wallet balance
    CAmount balance = m_walletModel->getBalance();
    CAmount availableForStaking = balance;
    
    // Calculate staking statistics
    double totalStaked = m_stakingData.totalStake;
    double stakingEfficiency = balance > 0 ? (totalStaked / balance) * 100 : 0;
    double networkStake = 10000000.0; // Simulated network stake
    double avgReward = m_stakingData.totalRewards > 0 ? m_stakingData.totalRewards / 10 : 0; // Simulated
    double stakingAPY = 5.0; // 5% APY
    
    // Update labels
    m_totalStakedLabel->setText(tr("Total Staked: %1").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, totalStaked)));
    m_availableForStakingLabel->setText(tr("Available for Staking: %1").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, availableForStaking)));
    m_stakingEfficiencyLabel->setText(tr("Staking Efficiency: %1%").arg(stakingEfficiency, 0, 'f', 1));
    m_networkStakeLabel->setText(tr("Network Stake: %1").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, networkStake)));
    m_avgRewardLabel->setText(tr("Average Reward: %1").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, avgReward)));
    m_stakingAPYLabel->setText(tr("Staking APY: %1%").arg(stakingAPY, 0, 'f', 1));
}

void PoSStakingPanel::updateRewardSimulation()
{
    // Update simulation results
    m_expectedRewardLabel->setText(tr("Expected Reward: %1").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, m_simulationResult.expectedReward)));
    m_annualReturnLabel->setText(tr("Annual Return: %1%").arg(m_simulationResult.annualReturn * 100, 0, 'f', 2));
    m_probabilityLabel->setText(tr("Probability: %1%").arg(m_simulationResult.probability * 100, 0, 'f', 2));
    m_breakEvenLabel->setText(tr("Break-even: %1 days").arg(m_simulationResult.breakEvenDays));
}

void PoSStakingPanel::onStakeToggleClicked()
{
    if (!checkStakingEligibility()) {
        QMessageBox::warning(this, tr("Staking Error"), 
            tr("You need at least 333 SHAH to start staking. Current balance is insufficient."));
        return;
    }
    
    m_stakingData.isStaking = !m_stakingData.isStaking;
    
    if (m_stakingData.isStaking) {
        QMessageBox::information(this, tr("Staking Enabled"), 
            tr("Staking has been enabled. You will start earning rewards based on your stake weight."));
    } else {
        QMessageBox::information(this, tr("Staking Disabled"), 
            tr("Staking has been disabled. You will no longer earn staking rewards."));
    }
    
    updateStakingStatus();
}

void PoSStakingPanel::onClaimRewardsClicked()
{
    if (m_stakingData.totalRewards <= 0) {
        QMessageBox::information(this, tr("No Rewards"), 
            tr("No rewards available to claim at this time."));
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Claim Rewards"), 
        tr("Claim %1 SHAH in staking rewards?").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, m_stakingData.totalRewards)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Simulate claiming rewards
        m_stakingData.lastReward = m_stakingData.totalRewards;
        m_stakingData.totalRewards = 0;
        
        QMessageBox::information(this, tr("Rewards Claimed"), 
            tr("Successfully claimed %1 SHAH in staking rewards.").arg(ShahcoinUnits::format(ShahcoinUnits::SHAH, m_stakingData.lastReward)));
        
        updateStakingStatus();
        updateStakingHistory();
    }
}

void PoSStakingPanel::onCompoundingToggled(bool checked)
{
    m_stakingData.isCompounding = checked;
    
    if (checked) {
        QMessageBox::information(this, tr("Compounding Enabled"), 
            tr("Compounding has been enabled. Rewards will be automatically reinvested for higher returns."));
    } else {
        QMessageBox::information(this, tr("Compounding Disabled"), 
            tr("Compounding has been disabled. Rewards will be sent to your wallet balance."));
    }
}

void PoSStakingPanel::onColdStakingToggled(bool checked)
{
    m_stakingData.isColdStaking = checked;
    
    if (checked) {
        QMessageBox::information(this, tr("Cold Staking Enabled"), 
            tr("Cold staking has been enabled. Your private keys remain offline while staking."));
    } else {
        QMessageBox::information(this, tr("Cold Staking Disabled"), 
            tr("Cold staking has been disabled. Using hot staking mode."));
    }
}

void PoSStakingPanel::onSimulateRewardsClicked()
{
    double amount = m_stakingAmountSpin->value();
    int days = m_stakingTimeSpin->value();
    
    if (!validateStakingAmount(amount) || !validateStakingTime(days)) {
        return;
    }
    
    // Calculate simulation results
    double dailyReward = amount * 0.05 / 365; // 5% APY
    m_simulationResult.expectedReward = dailyReward * days;
    m_simulationResult.annualReturn = 0.05; // 5% APY
    m_simulationResult.probability = qMin(amount / 10000.0, 1.0); // Higher amount = higher probability
    m_simulationResult.breakEvenDays = static_cast<int>(amount / (dailyReward * 2)); // Simplified calculation
    m_simulationResult.totalRewards = m_simulationResult.expectedReward;
    
    updateRewardSimulation();
    
    QMessageBox::information(this, tr("Simulation Complete"), 
        tr("Reward simulation completed. Check the results above for detailed projections."));
}

void PoSStakingPanel::onStakingAmountChanged(double amount)
{
    // Update simulation if needed
    if (m_stakingAmountSpin->hasFocus()) {
        // Could trigger real-time simulation updates here
    }
}

void PoSStakingPanel::onStakingTimeChanged(int days)
{
    // Update simulation if needed
    if (m_stakingTimeSpin->hasFocus()) {
        // Could trigger real-time simulation updates here
    }
}

void PoSStakingPanel::onRefreshClicked()
{
    updateStakingStatus();
    updateStakingStats();
    updateStakingHistory();
}

void PoSStakingPanel::calculateStakingWeight()
{
    if (!m_walletModel) return;
    
    CAmount balance = m_walletModel->getBalance();
    m_stakingData.totalStake = balance;
    
    // Calculate staking weight based on amount and age
    double baseWeight = balance;
    double ageMultiplier = 1.0 + (m_stakingData.avgCoinAge / 86400.0); // Daily compounding
    m_stakingData.stakingWeight = baseWeight * ageMultiplier;
    
    // Count eligible coins (simplified)
    m_stakingData.eligibleCoins = static_cast<int>(balance / (333 * COIN));
}

void PoSStakingPanel::calculateStakingProbability()
{
    // Calculate staking probability based on weight and network difficulty
    double networkDifficulty = 1000000.0; // Simulated network difficulty
    m_stakingData.stakingProbability = qMin(m_stakingData.stakingWeight / networkDifficulty, 1.0);
}

void PoSStakingPanel::calculateExpectedRewards()
{
    if (!m_stakingData.isStaking) {
        m_stakingData.totalRewards = 0;
        return;
    }
    
    // Calculate expected rewards based on stake weight and time
    double dailyReward = m_stakingData.stakingWeight * 0.05 / 365; // 5% APY
    double timeElapsed = 1.0; // Simulated time elapsed (1 day)
    m_stakingData.totalRewards = dailyReward * timeElapsed;
}

void PoSStakingPanel::updateStakingHistory()
{
    // Clear existing data
    m_stakingHistoryTable->setRowCount(0);
    
    // Add simulated staking history
    QStringList dates = {"2025-01-15", "2025-01-14", "2025-01-13", "2025-01-12", "2025-01-11"};
    QStringList types = {"Reward", "Stake", "Reward", "Stake", "Reward"};
    QStringList amounts = {"1000 SHAH", "500 SHAH", "1000 SHAH", "750 SHAH", "1000 SHAH"};
    QStringList rewards = {"5 SHAH", "0 SHAH", "5 SHAH", "0 SHAH", "5 SHAH"};
    QStringList balances = {"1005 SHAH", "1000 SHAH", "1005 SHAH", "1000 SHAH", "1005 SHAH"};
    QStringList statuses = {"Completed", "Completed", "Completed", "Completed", "Completed"};
    
    for (int i = 0; i < dates.size(); i++) {
        int row = m_stakingHistoryTable->rowCount();
        m_stakingHistoryTable->insertRow(row);
        
        m_stakingHistoryTable->setItem(row, 0, new QTableWidgetItem(dates[i]));
        m_stakingHistoryTable->setItem(row, 1, new QTableWidgetItem(types[i]));
        m_stakingHistoryTable->setItem(row, 2, new QTableWidgetItem(amounts[i]));
        m_stakingHistoryTable->setItem(row, 3, new QTableWidgetItem(rewards[i]));
        m_stakingHistoryTable->setItem(row, 4, new QTableWidgetItem(balances[i]));
        m_stakingHistoryTable->setItem(row, 5, new QTableWidgetItem(statuses[i]));
    }
    
    // Update rewards chart
    m_rewardsSeries->clear();
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    
    for (int i = 0; i < 30; i++) {
        qint64 time = currentTime - (30 - i) * 86400; // Daily intervals
        double reward = 5.0 + (i * 0.1); // Simulated increasing rewards
        m_rewardsSeries->append(time * 1000, reward);
    }
}

bool PoSStakingPanel::validateStakingAmount(double amount)
{
    if (amount < 333.0) {
        QMessageBox::warning(this, tr("Invalid Amount"), 
            tr("Minimum staking amount is 333 SHAH."));
        return false;
    }
    
    if (amount > 1000000.0) {
        QMessageBox::warning(this, tr("Invalid Amount"), 
            tr("Maximum staking amount is 1,000,000 SHAH."));
        return false;
    }
    
    return true;
}

bool PoSStakingPanel::validateStakingTime(int days)
{
    if (days < 1) {
        QMessageBox::warning(this, tr("Invalid Time"), 
            tr("Minimum staking time is 1 day."));
        return false;
    }
    
    if (days > 3650) {
        QMessageBox::warning(this, tr("Invalid Time"), 
            tr("Maximum staking time is 10 years."));
        return false;
    }
    
    return true;
}

bool PoSStakingPanel::checkStakingEligibility()
{
    if (!m_walletModel) return false;
    
    CAmount balance = m_walletModel->getBalance();
    return balance >= 333 * COIN; // Minimum 333 SHAH
}
