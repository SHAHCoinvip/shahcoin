// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#ifndef POWPOSDASHBOARD_H
#define POWPOSDASHBOARD_H

#include <QWidget>
#include <QTimer>
#include <QChart>
#include <QChartView>
#include <QBarSeries>
#include <QPieSeries>
#include <QLineSeries>
#include <QTableWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

QT_CHARTS_USE_NAMESPACE

class ClientModel;
class WalletModel;

/**
 * PoW/PoS Dashboard Tab
 * 
 * Displays comprehensive hybrid consensus statistics including:
 * - Block distribution (PoW vs PoS)
 * - Algorithm usage charts
 * - Difficulty trends
 * - Staking statistics
 * - Network health metrics
 */
class PoWPoSDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit PoWPoSDashboard(QWidget *parent = nullptr);
    ~PoWPoSDashboard();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);

private slots:
    void updateDashboard();
    void updateBlockDistribution();
    void updateAlgorithmUsage();
    void updateDifficultyChart();
    void updateStakingStats();
    void updateNetworkHealth();
    void onRefreshClicked();
    void onTimeRangeChanged(int index);
    void onAutoRefreshToggled(bool checked);

private:
    void setupUI();
    void setupCharts();
    void setupTables();
    void setupControls();
    void connectSignals();
    
    // Data fetching methods
    void fetchBlockData();
    void fetchStakingData();
    void fetchNetworkData();
    
    // Chart update methods
    void updatePieChart(QPieSeries *series, const QMap<QString, double> &data);
    void updateBarChart(QBarSeries *series, const QMap<QString, double> &data);
    void updateLineChart(QLineSeries *series, const QVector<QPair<qint64, double>> &data);

    // UI Components
    QVBoxLayout *m_mainLayout;
    
    // Control Panel
    QHBoxLayout *m_controlLayout;
    QPushButton *m_refreshButton;
    QComboBox *m_timeRangeCombo;
    QCheckBox *m_autoRefreshCheck;
    QSpinBox *m_refreshIntervalSpin;
    
    // Statistics Cards
    QGridLayout *m_statsLayout;
    QLabel *m_totalBlocksLabel;
    QLabel *m_powBlocksLabel;
    QLabel *m_posBlocksLabel;
    QLabel *m_currentAlgoLabel;
    QLabel *m_nextAlgoLabel;
    QLabel *m_avgBlockTimeLabel;
    QLabel *m_networkHashrateLabel;
    QLabel *m_totalStakeLabel;
    QLabel *m_stakingRewardsLabel;
    
    // Charts
    QChartView *m_blockDistributionChart;
    QChartView *m_algorithmUsageChart;
    QChartView *m_difficultyChart;
    QChartView *m_stakingChart;
    
    // Tables
    QTableWidget *m_recentBlocksTable;
    QTableWidget *m_stakingStatsTable;
    QTableWidget *m_algorithmStatsTable;
    
    // Progress indicators
    QProgressBar *m_syncProgressBar;
    QLabel *m_syncStatusLabel;
    
    // Data storage
    QMap<QString, double> m_blockDistributionData;
    QMap<QString, double> m_algorithmUsageData;
    QVector<QPair<qint64, double>> m_difficultyData;
    QVector<QPair<qint64, double>> m_stakingData;
    
    // Models
    ClientModel *m_clientModel;
    WalletModel *m_walletModel;
    
    // Timer for auto-refresh
    QTimer *m_refreshTimer;
    
    // Current time range (in blocks)
    int m_timeRange;
    
    // Chart objects
    QChart *m_blockDistributionChartObj;
    QChart *m_algorithmUsageChartObj;
    QChart *m_difficultyChartObj;
    QChart *m_stakingChartObj;
    
    QPieSeries *m_blockDistributionSeries;
    QBarSeries *m_algorithmUsageSeries;
    QLineSeries *m_difficultySeries;
    QLineSeries *m_stakingSeries;
};

#endif // POWPOSDASHBOARD_H
