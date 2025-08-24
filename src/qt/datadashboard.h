#ifndef DATADASHBOARD_H
#define DATADASHBOARD_H

#include <QWidget>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QPieSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QDateTimeAxis>
#include <QTableWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QT_CHARTS_USE_NAMESPACE

class WalletModel;

// Miner stats structure
struct MinerStats {
    QString address;
    double hashRate;
    int blocksFound;
    double totalRewards;
    double pendingRewards;
    int uptime;
    QString status;
    QDateTime lastSeen;
    double efficiency;
    QString version;
    QString location;
    QMap<QString, double> hourlyHashRates;
    QMap<QString, double> dailyRewards;
    QMap<QString, int> dailyBlocks;
};

// Node stats structure
struct NodeStats {
    QString address;
    QString type; // "masternode", "fullnode", "lightnode"
    QString status;
    int connections;
    double uptime;
    QString version;
    QString location;
    double bandwidth;
    double latency;
    QDateTime lastSeen;
    bool isSynced;
    int syncPercentage;
    QMap<QString, double> hourlyConnections;
    QMap<QString, double> hourlyLatency;
};

// Network stats structure
struct NetworkStats {
    int totalNodes;
    int masternodes;
    int fullNodes;
    int lightNodes;
    int totalMiners;
    double totalHashRate;
    int currentBlockHeight;
    int targetBlockHeight;
    double syncPercentage;
    int pendingTransactions;
    double averageBlockTime;
    double difficulty;
    double totalSupply;
    double circulatingSupply;
    QDateTime lastBlockTime;
    QMap<QString, double> hourlyTransactions;
    QMap<QString, double> hourlyBlocks;
    QMap<QString, double> hourlyDifficulty;
};

// Staking performance structure
struct StakingPerformance {
    QString address;
    double stakedAmount;
    double totalRewards;
    double apy;
    double roi;
    int stakingDuration;
    QDateTime startDate;
    QDateTime lastReward;
    double averageReward;
    int totalRewardsCount;
    QMap<QString, double> dailyRewards;
    QMap<QString, double> dailyApy;
    QMap<QString, double> cumulativeRoi;
};

// Data Dashboard - Main class for analytics and monitoring
class DataDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit DataDashboard(WalletModel* walletModel, QWidget* parent = nullptr);
    ~DataDashboard();

    // Core functionality
    void initialize();
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;

    // Data refresh
    void refreshMinerStats();
    void refreshNodeStats();
    void refreshNetworkStats();
    void refreshStakingPerformance();
    void refreshAllData();

    // Chart management
    void updateHashRateChart();
    void updateRewardsChart();
    void updateNetworkChart();
    void updateStakingChart();
    void updateBlockExplorerChart();
    void updateMempoolChart();
    void updateDifficultyChart();
    void updateConnectionsChart();
    void updateLatencyChart();

    // Export functionality
    void exportMinerData(const QString& filename);
    void exportNodeData(const QString& filename);
    void exportNetworkData(const QString& filename);
    void exportStakingData(const QString& filename);
    void exportAllData(const QString& filename);

    // Settings
    void setAutoRefresh(bool enabled);
    void setRefreshInterval(int seconds);
    void setChartTheme(const QString& theme);
    void setDataRetention(int days);
    void setShowAnimations(bool enabled);
    void setShowTooltips(bool enabled);

    bool isAutoRefresh() const;
    int getRefreshInterval() const;
    QString getChartTheme() const;
    int getDataRetention() const;
    bool isShowAnimations() const;
    bool isShowTooltips() const;

    // Data access
    QList<MinerStats> getMinerStats() const;
    QList<NodeStats> getNodeStats() const;
    NetworkStats getNetworkStats() const;
    QList<StakingPerformance> getStakingPerformance() const;

signals:
    void minerStatsUpdated(const QList<MinerStats>& stats);
    void nodeStatsUpdated(const QList<NodeStats>& stats);
    void networkStatsUpdated(const NetworkStats& stats);
    void stakingPerformanceUpdated(const QList<StakingPerformance>& performance);
    void dataExportCompleted(const QString& filename);
    void dataExportFailed(const QString& error);
    void monitoringStarted();
    void monitoringStopped();
    void refreshCompleted();
    void refreshFailed(const QString& error);

private slots:
    void onRefreshTimer();
    void onNetworkReplyFinished();
    void onMinerStatsUpdated(const QList<MinerStats>& stats);
    void onNodeStatsUpdated(const QList<NodeStats>& stats);
    void onNetworkStatsUpdated(const NetworkStats& stats);
    void onStakingPerformanceUpdated(const QList<StakingPerformance>& performance);
    void onExportClicked();
    void onSettingsClicked();
    void onRefreshClicked();
    void onChartThemeChanged();
    void onRefreshIntervalChanged();
    void onAutoRefreshToggled();

private:
    // Core members
    WalletModel* m_walletModel;
    QSettings m_settings;
    bool m_isMonitoring;
    bool m_autoRefresh;
    int m_refreshInterval;
    QString m_chartTheme;
    int m_dataRetention;
    bool m_showAnimations;
    bool m_showTooltips;

    // Network
    QNetworkAccessManager* m_networkManager;
    QTimer* m_refreshTimer;

    // Data
    QList<MinerStats> m_minerStats;
    QList<NodeStats> m_nodeStats;
    NetworkStats m_networkStats;
    QList<StakingPerformance> m_stakingPerformance;
    QMutex m_dataMutex;

    // UI components
    QTabWidget* m_tabWidget;
    
    // Miner Stats Tab
    QWidget* m_minerTab;
    QTableWidget* m_minerTable;
    QChartView* m_hashRateChart;
    QChartView* m_rewardsChart;
    QChartView* m_efficiencyChart;
    QLabel* m_totalMinersLabel;
    QLabel* m_totalHashRateLabel;
    QLabel* m_totalRewardsLabel;
    QLabel* m_averageEfficiencyLabel;
    QProgressBar* m_minerSyncProgress;

    // Node Stats Tab
    QWidget* m_nodeTab;
    QTableWidget* m_nodeTable;
    QChartView* m_connectionsChart;
    QChartView* m_latencyChart;
    QChartView* m_uptimeChart;
    QLabel* m_totalNodesLabel;
    QLabel* m_masternodesLabel;
    QLabel* m_fullNodesLabel;
    QLabel* m_lightNodesLabel;
    QProgressBar* m_nodeSyncProgress;

    // Network Stats Tab
    QWidget* m_networkTab;
    QChartView* m_transactionsChart;
    QChartView* m_blocksChart;
    QChartView* m_difficultyChart;
    QChartView* m_mempoolChart;
    QLabel* m_blockHeightLabel;
    QLabel* m_syncPercentageLabel;
    QLabel* m_pendingTxLabel;
    QLabel* m_averageBlockTimeLabel;
    QLabel* m_difficultyLabel;
    QLabel* m_totalSupplyLabel;
    QLabel* m_circulatingSupplyLabel;
    QProgressBar* m_networkSyncProgress;

    // Staking Performance Tab
    QWidget* m_stakingTab;
    QTableWidget* m_stakingTable;
    QChartView* m_apyChart;
    QChartView* m_roiChart;
    QChartView* m_rewardsChart2;
    QLabel* m_totalStakedLabel;
    QLabel* m_totalRewardsLabel2;
    QLabel* m_averageApyLabel;
    QLabel* m_averageRoiLabel;
    QProgressBar* m_stakingProgress;

    // Block Explorer Tab
    QWidget* m_explorerTab;
    QChartView* m_blockExplorerChart;
    QTableWidget* m_recentBlocksTable;
    QTableWidget* m_recentTransactionsTable;
    QLabel* m_latestBlockLabel;
    QLabel* m_latestTransactionLabel;
    QLabel* m_networkHashRateLabel;
    QLabel* m_networkDifficultyLabel;

    // Controls
    QPushButton* m_refreshButton;
    QPushButton* m_exportButton;
    QPushButton* m_settingsButton;
    QComboBox* m_chartThemeCombo;
    QSpinBox* m_refreshIntervalSpin;
    QCheckBox* m_autoRefreshCheck;
    QCheckBox* m_showAnimationsCheck;
    QCheckBox* m_showTooltipsCheck;

    // Helper methods
    void setupUI();
    void setupMinerTab();
    void setupNodeTab();
    void setupNetworkTab();
    void setupStakingTab();
    void setupExplorerTab();
    void setupControls();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    void updateMinerTable();
    void updateNodeTable();
    void updateStakingTable();
    void updateRecentBlocksTable();
    void updateRecentTransactionsTable();
    void updateAllLabels();
    void updateAllProgressBars();
    void updateAllCharts();
    void createHashRateChart();
    void createRewardsChart();
    void createEfficiencyChart();
    void createConnectionsChart();
    void createLatencyChart();
    void createUptimeChart();
    void createTransactionsChart();
    void createBlocksChart();
    void createDifficultyChart();
    void createMempoolChart();
    void createApyChart();
    void createRoiChart();
    void createBlockExplorerChart();
    void applyChartTheme(QChart* chart);
    void setupChartView(QChartView* chartView);
    void setupTableWidget(QTableWidget* table);
    void setupProgressBar(QProgressBar* progressBar);
    void setupLabel(QLabel* label);
    void fetchMinerData();
    void fetchNodeData();
    void fetchNetworkData();
    void fetchStakingData();
    void fetchBlockExplorerData();
    void handleNetworkError(const QString& error);
    void handleApiResponse(const QJsonDocument& response);
    void cleanupOldData();
    void exportDataToFile(const QString& filename, const QJsonDocument& data);
    QString formatHashRate(double hashRate) const;
    QString formatAmount(double amount) const;
    QString formatPercentage(double percentage) const;
    QString formatDuration(int seconds) const;
    QString formatDateTime(const QDateTime& dateTime) const;
    QColor getStatusColor(const QString& status) const;
    QColor getPerformanceColor(double value, double threshold) const;
    void showExportDialog();
    void showSettingsDialog();
    void showRefreshDialog();
    void showErrorDialog(const QString& error);
    void showSuccessDialog(const QString& message);
    void logDataUpdate(const QString& type, const QString& message);
    void logChartUpdate(const QString& chart, const QString& message);
    void logExport(const QString& filename, const QString& type);
    void logError(const QString& error);
    void logInfo(const QString& message);
    void logDebug(const QString& message);
};

// Settings Dialog for Data Dashboard
class DataDashboardSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataDashboardSettingsDialog(DataDashboard* dashboard, QWidget* parent = nullptr);

signals:
    void settingsUpdated();

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onResetClicked();

private:
    DataDashboard* m_dashboard;
    QCheckBox* m_autoRefreshCheck;
    QSpinBox* m_refreshIntervalSpin;
    QComboBox* m_chartThemeCombo;
    QSpinBox* m_dataRetentionSpin;
    QCheckBox* m_showAnimationsCheck;
    QCheckBox* m_showTooltipsCheck;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    QPushButton* m_resetButton;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;

    void setupUI();
    void loadCurrentSettings();
    void saveSettings();
    void resetToDefaults();
};

// Export Dialog for Data Dashboard
class DataDashboardExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataDashboardExportDialog(DataDashboard* dashboard, QWidget* parent = nullptr);

signals:
    void exportRequested(const QString& filename, const QStringList& types);

private slots:
    void onExportClicked();
    void onCancelClicked();
    void onBrowseClicked();

private:
    DataDashboard* m_dashboard;
    QCheckBox* m_exportMinerCheck;
    QCheckBox* m_exportNodeCheck;
    QCheckBox* m_exportNetworkCheck;
    QCheckBox* m_exportStakingCheck;
    QCheckBox* m_exportAllCheck;
    QLineEdit* m_filenameEdit;
    QComboBox* m_formatCombo;
    QPushButton* m_browseButton;
    QPushButton* m_exportButton;
    QPushButton* m_cancelButton;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;

    void setupUI();
    void updateFilename();
    QString getDefaultFilename() const;
    QString getFileExtension() const;
};

#endif // DATADASHBOARD_H
