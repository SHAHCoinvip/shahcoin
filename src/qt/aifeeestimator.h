#ifndef SHAHCOIN_QT_AIFEEESTIMATOR_H
#define SHAHCOIN_QT_AIFEEESTIMATOR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QProgressBar>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class WalletModel;
class PlatformStyle;

/**
 * @brief AI Fee Estimator with Live Dashboard
 *
 * Provides real-time fee estimation and AI-powered optimization:
 * - Live network fee monitoring
 * - AI suggestions for optimal transaction timing
 * - Fee history charts and trends
 * - Smart fee recommendations based on urgency
 * - Integration with wallet send functionality
 */
class AIFeeEstimator : public QWidget
{
    Q_OBJECT

public:
    enum FeePriority {
        Low = 0,
        Normal,
        High,
        Urgent
    };

    enum NetworkCondition {
        LowCongestion = 0,
        NormalCongestion,
        HighCongestion,
        ExtremeCongestion
    };

    struct FeeEstimate {
        FeePriority priority;
        double feePerKB;
        double totalFee;
        int confirmationTime; // minutes
        QString description;
        bool recommended;
    };

    struct NetworkStats {
        NetworkCondition condition;
        double avgFeePerKB;
        int avgConfirmationTime;
        int pendingTransactions;
        double networkLoad; // percentage
        QDateTime lastUpdate;
    };

    explicit AIFeeEstimator(WalletModel* walletModel, QWidget* parent = nullptr);
    ~AIFeeEstimator();

    // Main estimation methods
    void refreshFeeEstimates();
    void estimateFeesForAmount(double amount);
    void getAIRecommendation(double amount, FeePriority urgency = Normal);
    void applyRecommendedFee();

    // Network monitoring
    void startNetworkMonitoring();
    void stopNetworkMonitoring();
    void updateNetworkStats();

    // Chart and visualization
    void updateFeeChart();
    void updateNetworkLoadChart();
    void exportFeeHistory();

    // Settings
    void saveSettings();
    void loadSettings();

signals:
    void feeEstimateUpdated(const QList<FeeEstimate>& estimates);
    void aiRecommendationReady(const FeeEstimate& recommendation);
    void networkStatsUpdated(const NetworkStats& stats);
    void feeApplied(double fee, FeePriority priority);
    void optimizationCompleted(double savings);

public Q_SLOTS:
    void onRefreshClicked();
    void onAmountChanged();
    void onPriorityChanged(int index);
    void onApplyFeeClicked();
    void onOptimizeClicked();
    void onExportHistoryClicked();
    void onShowAdvancedClicked();
    void onAutoRefreshToggled(bool enabled);
    void onNetworkReplyFinished();

private Q_SLOTS:
    void onUpdateTimer();
    void onChartRangeChanged();
    void onFeeHistoryItemClicked(QTableWidgetItem* item);
    void onContextMenuRequested(const QPoint& pos);

private:
    void setupUI();
    void connectSignals();
    void applyTheme();

    // Fee estimation logic
    QList<FeeEstimate> calculateFeeEstimates(double amount);
    FeeEstimate generateAIRecommendation(double amount, FeePriority urgency);
    double predictOptimalFee(double amount, int targetConfirmationTime);
    double calculateFeeSavings(const FeeEstimate& current, const FeeEstimate& optimized);

    // Network monitoring
    void fetchNetworkData();
    void parseNetworkResponse(const QByteArray& data);
    NetworkCondition determineNetworkCondition(double avgFee, int pendingTx);
    void updateNetworkIndicators();

    // Chart management
    void setupCharts();
    void addFeeDataPoint(double fee, const QDateTime& timestamp);
    void addNetworkDataPoint(double load, const QDateTime& timestamp);
    void clearOldDataPoints();

    // AI and optimization
    void runAIAnalysis();
    QString generateOptimizationMessage(const FeeEstimate& recommendation);
    bool shouldRecommendOptimization(const FeeEstimate& current, const FeeEstimate& optimized);

    // UI helpers
    void updateFeeDisplay();
    void updateNetworkDisplay();
    void showOptimizationDialog(const FeeEstimate& recommendation);
    void animateFeeUpdate();
    void showNotification(const QString& message, const QString& type = "info");

    // Data management
    void saveFeeHistory();
    void loadFeeHistory();
    void cleanupOldHistory();

    // Members
    WalletModel* m_walletModel;
    PlatformStyle* m_platformStyle;

    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Fee Estimation Section
    QGroupBox* m_feeGroup;
    QHBoxLayout* m_feeInputLayout;
    QLabel* m_amountLabel;
    QDoubleSpinBox* m_amountInput;
    QLabel* m_priorityLabel;
    QComboBox* m_priorityCombo;
    QPushButton* m_refreshButton;
    QPushButton* m_optimizeButton;

    // Fee Display
    QGroupBox* m_feeDisplayGroup;
    QVBoxLayout* m_feeDisplayLayout;
    QLabel* m_currentFeeLabel;
    QLabel* m_recommendedFeeLabel;
    QLabel* m_savingsLabel;
    QPushButton* m_applyFeeButton;

    // Network Status
    QGroupBox* m_networkGroup;
    QHBoxLayout* m_networkLayout;
    QLabel* m_networkStatusIcon;
    QLabel* m_networkStatusLabel;
    QLabel* m_pendingTxLabel;
    QLabel* m_avgConfirmationLabel;
    QProgressBar* m_networkLoadBar;

    // Charts
    QGroupBox* m_chartsGroup;
    QTabWidget* m_chartTabs;
    QChartView* m_feeChartView;
    QChartView* m_networkChartView;
    QChart* m_feeChart;
    QChart* m_networkChart;
    QLineSeries* m_feeSeries;
    QLineSeries* m_networkSeries;

    // Fee History Table
    QGroupBox* m_historyGroup;
    QTableWidget* m_feeHistoryTable;
    QPushButton* m_exportHistoryButton;

    // Advanced Options
    QGroupBox* m_advancedGroup;
    QCheckBox* m_autoRefreshCheck;
    QSpinBox* m_refreshIntervalSpin;
    QCheckBox* m_showNotificationsCheck;
    QCheckBox* m_optimizeAutomaticallyCheck;

    // State
    QList<FeeEstimate> m_currentEstimates;
    FeeEstimate m_aiRecommendation;
    NetworkStats m_networkStats;
    double m_lastAmount;
    FeePriority m_lastPriority;
    bool m_autoRefreshEnabled;
    bool m_optimizationInProgress;

    // Network and timing
    QNetworkAccessManager* m_networkManager;
    QTimer* m_updateTimer;
    QTimer* m_animationTimer;
    QDateTime m_lastUpdate;

    // Data storage
    QSettings* m_settings;
    QList<QPair<QDateTime, double>> m_feeHistory;
    QList<QPair<QDateTime, double>> m_networkHistory;

    // Animation
    QPropertyAnimation* m_feeAnimation;
    QGraphicsOpacityEffect* m_feeOpacityEffect;

    // Constants
    static const int UPDATE_INTERVAL_MS = 30000; // 30 seconds
    static const int ANIMATION_DURATION_MS = 500;
    static const int MAX_HISTORY_POINTS = 1000;
    static const QString SETTINGS_GROUP;
    static const QString SETTINGS_AUTO_REFRESH;
    static const QString SETTINGS_REFRESH_INTERVAL;
    static const QString SETTINGS_SHOW_NOTIFICATIONS;
    static const QString SETTINGS_OPTIMIZE_AUTO;
};

/**
 * @brief Fee History Table Widget
 */
class FeeHistoryTable : public QTableWidget
{
    Q_OBJECT

public:
    explicit FeeHistoryTable(QWidget* parent = nullptr);
    void addFeeRecord(const QDateTime& timestamp, double fee, AIFeeEstimator::FeePriority priority, bool applied);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private Q_SLOTS:
    void onCopyFee();
    void onCopyTimestamp();
    void onExportSelected();
};

/**
 * @brief Network Status Indicator
 */
class NetworkStatusIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkStatusIndicator(QWidget* parent = nullptr);
    void updateStatus(AIFeeEstimator::NetworkCondition condition, const QString& description);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    AIFeeEstimator::NetworkCondition m_condition;
    QString m_description;
    QColor m_statusColor;
};

/**
 * @brief Fee Optimization Dialog
 */
class FeeOptimizationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FeeOptimizationDialog(const AIFeeEstimator::FeeEstimate& current,
                                  const AIFeeEstimator::FeeEstimate& optimized,
                                  QWidget* parent = nullptr);

signals:
    void optimizationAccepted(const AIFeeEstimator::FeeEstimate& fee);
    void optimizationRejected();

private Q_SLOTS:
    void onAcceptOptimization();
    void onRejectOptimization();

private:
    void setupUI();
    void applyTheme();

    AIFeeEstimator::FeeEstimate m_currentFee;
    AIFeeEstimator::FeeEstimate m_optimizedFee;
    double m_savings;
};

#endif // SHAHCOIN_QT_AIFEEESTIMATOR_H
