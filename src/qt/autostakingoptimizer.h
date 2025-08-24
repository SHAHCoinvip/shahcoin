#ifndef SHAHCOIN_QT_AUTOSTAKINGOPTIMIZER_H
#define SHAHCOIN_QT_AUTOSTAKINGOPTIMIZER_H

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
#include <QSlider>
#include <QDateTimeEdit>
#include <QCalendarWidget>

class WalletModel;
class PlatformStyle;

/**
 * @brief Auto-Staking Optimizer
 *
 * Provides intelligent staking optimization:
 * - Analyzes current balance and staking performance
 * - Suggests optimal staking amounts and schedules
 * - Calculates potential rewards and compound effects
 * - Recommends restaking strategies
 * - Shows historical performance and projections
 * - Integrates with existing staking functionality
 */
class AutoStakingOptimizer : public QWidget
{
    Q_OBJECT

public:
    enum OptimizationStrategy {
        Conservative = 0,
        Balanced,
        Aggressive,
        Custom
    };

    enum StakingFrequency {
        Daily = 0,
        Weekly,
        Monthly,
        Quarterly,
        Yearly
    };

    struct StakingRecommendation {
        double recommendedAmount;
        double currentStaked;
        double availableBalance;
        double potentialReward;
        double compoundEffect;
        int optimalFrequency;
        QString strategy;
        QString reasoning;
        bool autoRestake;
        QDateTime nextRestake;
    };

    struct PerformanceMetrics {
        double totalStaked;
        double totalRewards;
        double averageAPY;
        double compoundGrowth;
        int daysStaking;
        double efficiencyScore;
        QDateTime lastRestake;
        QDateTime nextScheduledRestake;
    };

    struct OptimizationSettings {
        OptimizationStrategy strategy;
        StakingFrequency frequency;
        double minStakeAmount;
        double maxStakePercentage;
        bool autoRestake;
        bool compoundRewards;
        bool notifyOnRestake;
        int restakeThreshold;
    };

    explicit AutoStakingOptimizer(WalletModel* walletModel, QWidget* parent = nullptr);
    ~AutoStakingOptimizer();

    // Main optimization methods
    void analyzeCurrentStaking();
    void generateRecommendations();
    void applyOptimization(const StakingRecommendation& recommendation);
    void scheduleAutoRestake();

    // Performance tracking
    void updatePerformanceMetrics();
    void calculateProjections();
    void trackHistoricalPerformance();

    // Settings management
    void saveSettings();
    void loadSettings();
    void resetToDefaults();

signals:
    void optimizationCompleted(const StakingRecommendation& recommendation);
    void performanceUpdated(const PerformanceMetrics& metrics);
    void autoRestakeScheduled(const QDateTime& nextRestake);
    void settingsChanged(const OptimizationSettings& settings);

public Q_SLOTS:
    void onAnalyzeClicked();
    void onOptimizeClicked();
    void onApplyRecommendationClicked();
    void onAutoRestakeToggled(bool enabled);
    void onStrategyChanged(int index);
    void onFrequencyChanged(int index);
    void onMinAmountChanged();
    void onMaxPercentageChanged();
    void onCompoundRewardsToggled(bool enabled);
    void onNotifyRestakeToggled(bool enabled);
    void onRestakeThresholdChanged();
    void onShowProjectionsClicked();
    void onExportDataClicked();
    void onResetSettingsClicked();

private Q_SLOTS:
    void onUpdateTimer();
    void onBalanceChanged();
    void onStakingStatusChanged();
    void onProjectionRangeChanged();
    void onChartTypeChanged();
    void onContextMenuRequested(const QPoint& pos);

private:
    void setupUI();
    void connectSignals();
    void applyTheme();

    // Analysis and optimization logic
    StakingRecommendation analyzeStakingPerformance();
    StakingRecommendation generateOptimalRecommendation();
    double calculateOptimalStakeAmount(double balance, OptimizationStrategy strategy);
    double calculatePotentialReward(double amount, int days);
    double calculateCompoundEffect(double principal, double rate, int periods);
    int determineOptimalFrequency(double amount, OptimizationStrategy strategy);
    QString generateStrategyReasoning(const StakingRecommendation& recommendation);

    // Performance calculations
    PerformanceMetrics calculateCurrentMetrics();
    void updatePerformanceChart();
    void updateProjectionChart();
    void addPerformanceDataPoint(const QDateTime& timestamp, double value);
    void addProjectionDataPoint(const QDateTime& timestamp, double value);

    // Auto-restake logic
    void scheduleNextRestake(const StakingRecommendation& recommendation);
    void checkRestakeThreshold();
    bool shouldAutoRestake();
    void executeAutoRestake();

    // UI helpers
    void updateRecommendationDisplay();
    void updatePerformanceDisplay();
    void updateProjectionDisplay();
    void showOptimizationDialog(const StakingRecommendation& recommendation);
    void animateOptimizationUpdate();
    void showNotification(const QString& message, const QString& type = "info");

    // Data management
    void savePerformanceHistory();
    void loadPerformanceHistory();
    void exportOptimizationData();

    // Members
    WalletModel* m_walletModel;
    PlatformStyle* m_platformStyle;

    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Analysis Section
    QGroupBox* m_analysisGroup;
    QHBoxLayout* m_analysisLayout;
    QLabel* m_currentBalanceLabel;
    QLabel* m_currentStakedLabel;
    QLabel* m_currentRewardsLabel;
    QPushButton* m_analyzeButton;

    // Optimization Section
    QGroupBox* m_optimizationGroup;
    QVBoxLayout* m_optimizationLayout;
    
    // Strategy Settings
    QHBoxLayout* m_strategyLayout;
    QLabel* m_strategyLabel;
    QComboBox* m_strategyCombo;
    QLabel* m_frequencyLabel;
    QComboBox* m_frequencyCombo;
    QPushButton* m_optimizeButton;

    // Amount Settings
    QHBoxLayout* m_amountLayout;
    QLabel* m_minAmountLabel;
    QDoubleSpinBox* m_minAmountInput;
    QLabel* m_maxPercentageLabel;
    QSlider* m_maxPercentageSlider;
    QLabel* m_maxPercentageValue;

    // Auto-restake Settings
    QHBoxLayout* m_autoRestakeLayout;
    QCheckBox* m_autoRestakeCheck;
    QCheckBox* m_compoundRewardsCheck;
    QCheckBox* m_notifyRestakeCheck;
    QLabel* m_restakeThresholdLabel;
    QSpinBox* m_restakeThresholdSpin;

    // Recommendation Display
    QGroupBox* m_recommendationGroup;
    QVBoxLayout* m_recommendationLayout;
    QLabel* m_recommendedAmountLabel;
    QLabel* m_potentialRewardLabel;
    QLabel* m_compoundEffectLabel;
    QLabel* m_strategyReasoningLabel;
    QPushButton* m_applyRecommendationButton;

    // Performance Metrics
    QGroupBox* m_performanceGroup;
    QHBoxLayout* m_performanceLayout;
    QLabel* m_totalStakedLabel;
    QLabel* m_totalRewardsLabel;
    QLabel* m_averageAPYLabel;
    QLabel* m_efficiencyScoreLabel;
    QLabel* m_lastRestakeLabel;
    QLabel* m_nextRestakeLabel;

    // Charts
    QGroupBox* m_chartsGroup;
    QTabWidget* m_chartTabs;
    QChartView* m_performanceChartView;
    QChartView* m_projectionChartView;
    QChart* m_performanceChart;
    QChart* m_projectionChart;
    QLineSeries* m_performanceSeries;
    QLineSeries* m_projectionSeries;

    // Projection Controls
    QHBoxLayout* m_projectionControlsLayout;
    QLabel* m_projectionRangeLabel;
    QComboBox* m_projectionRangeCombo;
    QPushButton* m_showProjectionsButton;
    QPushButton* m_exportDataButton;

    // Settings
    QGroupBox* m_settingsGroup;
    QHBoxLayout* m_settingsLayout;
    QPushButton* m_resetSettingsButton;
    QPushButton* m_saveSettingsButton;

    // State
    StakingRecommendation m_currentRecommendation;
    PerformanceMetrics m_currentMetrics;
    OptimizationSettings m_settings;
    bool m_optimizationInProgress;
    bool m_autoRestakeEnabled;
    QDateTime m_lastAnalysis;

    // Network and timing
    QNetworkAccessManager* m_networkManager;
    QTimer* m_updateTimer;
    QTimer* m_restakeCheckTimer;
    QTimer* m_animationTimer;

    // Data storage
    QSettings* m_settingsStorage;
    QList<QPair<QDateTime, double>> m_performanceHistory;
    QList<QPair<QDateTime, double>> m_projectionData;

    // Animation
    QPropertyAnimation* m_optimizationAnimation;
    QGraphicsOpacityEffect* m_optimizationOpacityEffect;

    // Constants
    static const int UPDATE_INTERVAL_MS = 60000; // 1 minute
    static const int RESTAKE_CHECK_INTERVAL_MS = 300000; // 5 minutes
    static const int ANIMATION_DURATION_MS = 500;
    static const int MAX_HISTORY_POINTS = 1000;
    static const QString SETTINGS_GROUP;
    static const QString SETTINGS_STRATEGY;
    static const QString SETTINGS_FREQUENCY;
    static const QString SETTINGS_MIN_AMOUNT;
    static const QString SETTINGS_MAX_PERCENTAGE;
    static const QString SETTINGS_AUTO_RESTAKE;
    static const QString SETTINGS_COMPOUND_REWARDS;
    static const QString SETTINGS_NOTIFY_RESTAKE;
    static const QString SETTINGS_RESTAKE_THRESHOLD;
};

/**
 * @brief Performance History Table Widget
 */
class PerformanceHistoryTable : public QTableWidget
{
    Q_OBJECT

public:
    explicit PerformanceHistoryTable(QWidget* parent = nullptr);
    void addPerformanceRecord(const QDateTime& timestamp, double staked, double rewards, double apy);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private Q_SLOTS:
    void onCopyValue();
    void onCopyTimestamp();
    void onExportSelected();
};

/**
 * @brief Optimization Strategy Dialog
 */
class OptimizationStrategyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptimizationStrategyDialog(const AutoStakingOptimizer::StakingRecommendation& recommendation,
                                       QWidget* parent = nullptr);

signals:
    void strategyAccepted(const AutoStakingOptimizer::StakingRecommendation& recommendation);
    void strategyRejected();

private Q_SLOTS:
    void onAcceptStrategy();
    void onRejectStrategy();

private:
    void setupUI();
    void applyTheme();

    AutoStakingOptimizer::StakingRecommendation m_recommendation;
};

/**
 * @brief Projection Calculator Widget
 */
class ProjectionCalculator : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectionCalculator(QWidget* parent = nullptr);
    void calculateProjection(double initialAmount, double apy, int years);

signals:
    void projectionCalculated(const QList<QPair<QDateTime, double>>& data);

private:
    void setupUI();
    void updateProjectionChart();

    QChartView* m_projectionChartView;
    QChart* m_projectionChart;
    QLineSeries* m_projectionSeries;
    QList<QPair<QDateTime, double>> m_projectionData;
};

#endif // SHAHCOIN_QT_AUTOSTAKINGOPTIMIZER_H
