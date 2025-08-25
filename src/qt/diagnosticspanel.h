// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_DIAGNOSTICSPANEL_H
#define SHAHCOIN_QT_DIAGNOSTICSPANEL_H

#include <QDialog>
#include <QTimer>
#include <QTabWidget>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QTableWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>

class WalletModel;
class PlatformStyle;

class DiagnosticsPanel : public QDialog
{
    Q_OBJECT

public:
    explicit DiagnosticsPanel(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~DiagnosticsPanel();
    void setWalletModel(WalletModel* walletModel);

public Q_SLOTS:
    void refreshData();

private Q_SLOTS:
    void onRefreshClicked();
    void onOpenLogFileClicked();
    void onLogFilterChanged();
    void onTabChanged(int index);

private:
    void setupUI();
    void setupMiningTab();
    void setupStakingTab();
    void setupChainSyncTab();
    void setupNetworkTab();
    void setupWalletTab();
    void setupLogsTab();
    void connectSignals();
    void applyTheme();
    void updateMiningMetrics();
    void updateStakingMetrics();
    void updateChainSyncMetrics();
    void updateNetworkMetrics();
    void updateWalletMetrics();
    void updateLogs();
    void startUpdateTimer();
    void stopUpdateTimer();
    QString formatHashrate(double hashrate);
    QString formatTime(int seconds);
    QString getLogLevelColor(const QString& level);

    // UI Components
    QTabWidget* m_tabWidget;
    QPushButton* m_refreshButton;
    QPushButton* m_closeButton;
    QTimer* m_updateTimer;

    // Mining Tab
    QLabel* m_miningAlgorithmLabel;
    QLabel* m_hashrateLabel;
    QLabel* m_threadsLabel;
    QLabel* m_acceptedSharesLabel;
    QLabel* m_rejectedSharesLabel;
    QLabel* m_miningErrorsLabel;
    QProgressBar* m_miningProgressBar;

    // Staking Tab
    QLabel* m_stakingStatusLabel;
    QLabel* m_stakedAmountLabel;
    QLabel* m_expectedRewardTimeLabel;
    QLabel* m_walletUnlockStatusLabel;
    QProgressBar* m_stakingProgressBar;

    // Chain Sync Tab
    QLabel* m_blockHeightLabel;
    QLabel* m_headersHeightLabel;
    QLabel* m_syncProgressLabel;
    QLabel* m_reorgCountLabel;
    QLabel* m_lastBlockTimeLabel;
    QProgressBar* m_syncProgressBar;

    // Network Tab
    QLabel* m_totalPeersLabel;
    QTableWidget* m_peersTable;
    QLabel* m_pingTimeLabel;
    QLabel* m_connectionTypeLabel;

    // Wallet Tab
    QLabel* m_confirmedBalanceLabel;
    QLabel* m_unconfirmedBalanceLabel;
    QLabel* m_stakedBalanceLabel;
    QLabel* m_transactionCountLabel;
    QLabel* m_diskSpaceLabel;
    QLabel* m_dbErrorsLabel;
    QProgressBar* m_diskSpaceProgressBar;

    // Logs Tab
    QTextEdit* m_logsTextEdit;
    QComboBox* m_logFilterCombo;
    QPushButton* m_openLogFileButton;
    QPushButton* m_clearLogsButton;

    // Data
    WalletModel* m_walletModel;
    const PlatformStyle* m_platformStyle;
    bool m_isUpdating;
    int m_currentTab;
    QString m_logFilter;
    QStringList m_recentLogs;
};

#endif // SHAHCOIN_QT_DIAGNOSTICSPANEL_H
