#ifndef SHAHCOIN_QT_AUTOSWAPENGINE_H
#define SHAHCOIN_QT_AUTOSWAPENGINE_H

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
#include <QDateTimeEdit>
#include <QCalendarWidget>
#include <QTextBrowser>
#include <QScrollArea>

class WalletModel;
class PlatformStyle;

/**
 * @brief Auto Swap / DCA Engine
 *
 * Provides automated dollar-cost averaging and recurring swaps:
 * - Set up recurring SHAH/token swaps (DCA)
 * - Schedule-based trading automation
 * - Price-based triggers and limits
 * - Performance tracking and analytics
 * - Integration with native ShahSwap layer
 * - Risk management and stop-loss features
 */
class AutoSwapEngine : public QWidget
{
    Q_OBJECT

public:
    enum SwapFrequency {
        Daily = 0,
        Weekly,
        Monthly,
        Custom
    };

    enum TriggerType {
        TimeBased = 0,
        PriceBased,
        VolumeBased,
        Hybrid
    };

    enum SwapStatus {
        Active = 0,
        Paused,
        Completed,
        Cancelled,
        Error
    };

    struct SwapStrategy {
        QString id;
        QString name;
        QString description;
        QString fromToken;
        QString toToken;
        double amount;
        SwapFrequency frequency;
        TriggerType triggerType;
        double priceTrigger;
        double stopLoss;
        double takeProfit;
        QDateTime startDate;
        QDateTime endDate;
        SwapStatus status;
        int totalSwaps;
        double totalAmount;
        double averagePrice;
        double totalFees;
        QDateTime lastSwap;
        QDateTime nextSwap;
    };

    struct SwapHistory {
        QDateTime timestamp;
        QString fromToken;
        QString toToken;
        double amount;
        double price;
        double fees;
        QString transactionId;
        bool success;
    };

    explicit AutoSwapEngine(WalletModel* walletModel, QWidget* parent = nullptr);
    ~AutoSwapEngine();

    // Main engine methods
    void createSwapStrategy(const SwapStrategy& strategy);
    void pauseSwapStrategy(const QString& strategyId);
    void resumeSwapStrategy(const QString& strategyId);
    void cancelSwapStrategy(const QString& strategyId);
    void executeSwap(const SwapStrategy& strategy);

    // Strategy management
    void updateStrategy(const SwapStrategy& strategy);
    void duplicateStrategy(const QString& strategyId);
    void exportStrategy(const QString& strategyId);

    // Settings
    void saveSettings();
    void loadSettings();

signals:
    void strategyCreated(const SwapStrategy& strategy);
    void strategyUpdated(const SwapStrategy& strategy);
    void strategyExecuted(const SwapHistory& history);
    void swapCompleted(const SwapHistory& history);

public Q_SLOTS:
    void onCreateStrategyClicked();
    void onPauseStrategyClicked();
    void onResumeStrategyClicked();
    void onCancelStrategyClicked();
    void onEditStrategyClicked();
    void onDuplicateStrategyClicked();
    void onExportStrategyClicked();
    void onShowAnalyticsClicked();
    void onAutoExecuteToggled(bool enabled);

private Q_SLOTS:
    void onUpdateTimer();
    void onStrategyItemClicked(QTableWidgetItem* item);
    void onContextMenuRequested(const QPoint& pos);

private:
    void setupUI();
    void connectSignals();
    void applyTheme();

    // Strategy management
    void addStrategyToTable(const SwapStrategy& strategy);
    void updateStrategyInTable(const SwapStrategy& strategy);
    void removeStrategyFromTable(const QString& strategyId);
    void refreshStrategyList();

    // Execution logic
    void checkAndExecuteStrategies();
    bool shouldExecuteStrategy(const SwapStrategy& strategy);
    double getCurrentPrice(const QString& token);
    bool executeSwapTransaction(const SwapStrategy& strategy);

    // Analytics and tracking
    void updatePerformanceMetrics();
    void updateSwapHistory();
    void calculateStrategyPerformance(const SwapStrategy& strategy);

    // UI helpers
    void showCreateStrategyDialog();
    void showEditStrategyDialog(const SwapStrategy& strategy);
    void showAnalyticsDialog();
    void animateStrategyUpdate();
    void showNotification(const QString& message, const QString& type = "info");

    // Data persistence
    void saveStrategies();
    void loadStrategies();
    void saveSwapHistory();
    void loadSwapHistory();

    // Members
    WalletModel* m_walletModel;
    PlatformStyle* m_platformStyle;

    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Strategy Overview
    QGroupBox* m_overviewGroup;
    QHBoxLayout* m_overviewLayout;
    QLabel* m_activeStrategiesLabel;
    QLabel* m_totalSwapsLabel;
    QLabel* m_totalVolumeLabel;
    QLabel* m_totalFeesLabel;
    QPushButton* m_createStrategyButton;

    // Strategy List
    QGroupBox* m_strategiesGroup;
    QVBoxLayout* m_strategiesLayout;
    QTableWidget* m_strategiesTable;
    QPushButton* m_pauseStrategyButton;
    QPushButton* m_resumeStrategyButton;
    QPushButton* m_cancelStrategyButton;
    QPushButton* m_editStrategyButton;
    QPushButton* m_duplicateStrategyButton;
    QPushButton* m_exportStrategyButton;

    // Swap History
    QGroupBox* m_historyGroup;
    QVBoxLayout* m_historyLayout;
    QTableWidget* m_swapHistoryTable;
    QPushButton* m_exportHistoryButton;
    QPushButton* m_clearHistoryButton;

    // Performance Analytics
    QGroupBox* m_analyticsGroup;
    QVBoxLayout* m_analyticsLayout;
    QChartView* m_performanceChartView;
    QChart* m_performanceChart;
    QLineSeries* m_swapSeries;
    QLineSeries* m_priceSeries;
    QValueAxis* m_valueAxis;
    QDateTimeAxis* m_timeAxis;
    QPushButton* m_showAnalyticsButton;

    // Settings Panel
    QGroupBox* m_settingsGroup;
    QVBoxLayout* m_settingsLayout;
    QCheckBox* m_autoExecuteCheck;
    QCheckBox* m_notifyOnSwapCheck;
    QCheckBox* m_stopOnErrorCheck;
    QSpinBox* m_maxConcurrentSwapsSpin;
    QDoubleSpinBox* m_maxSlippageSpin;
    QPushButton* m_saveSettingsButton;

    // State
    QList<SwapStrategy> m_strategies;
    QList<SwapHistory> m_swapHistory;
    bool m_autoExecuteEnabled;
    bool m_notifyOnSwap;
    bool m_stopOnError;
    int m_maxConcurrentSwaps;
    double m_maxSlippage;

    // Network and timing
    QNetworkAccessManager* m_networkManager;
    QTimer* m_updateTimer;
    QTimer* m_executionTimer;
    QTimer* m_animationTimer;
    QDateTime m_lastUpdate;

    // Data storage
    QSettings* m_settings;
    QList<QPair<QDateTime, double>> m_priceHistory;

    // Animation
    QPropertyAnimation* m_strategyAnimation;
    QGraphicsOpacityEffect* m_strategyOpacityEffect;

    // Constants
    static const int UPDATE_INTERVAL_MS = 30000; // 30 seconds
    static const int EXECUTION_INTERVAL_MS = 60000; // 1 minute
    static const int ANIMATION_DURATION_MS = 300;
    static const QString SETTINGS_GROUP;
    static const QString SETTINGS_STRATEGIES;
    static const QString SETTINGS_SWAP_HISTORY;
    static const QString SETTINGS_AUTO_EXECUTE;
    static const QString SETTINGS_NOTIFY_ON_SWAP;
    static const QString SETTINGS_STOP_ON_ERROR;
    static const QString SETTINGS_MAX_CONCURRENT_SWAPS;
    static const QString SETTINGS_MAX_SLIPPAGE;
};

/**
 * @brief Strategy Creation Dialog
 */
class StrategyCreationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StrategyCreationDialog(QWidget* parent = nullptr);
    AutoSwapEngine::SwapStrategy getStrategy() const;

signals:
    void strategyCreated(const AutoSwapEngine::SwapStrategy& strategy);

private Q_SLOTS:
    void onCreateClicked();
    void onCancelClicked();

private:
    void setupUI();
    void applyTheme();
    void validateInputs();

    AutoSwapEngine::SwapStrategy m_strategy;
    QVBoxLayout* m_layout;
    
    // Strategy Details
    QLineEdit* m_nameInput;
    QTextEdit* m_descriptionInput;
    QComboBox* m_fromTokenCombo;
    QComboBox* m_toTokenCombo;
    QDoubleSpinBox* m_amountInput;
    
    // Frequency and Triggers
    QComboBox* m_frequencyCombo;
    QComboBox* m_triggerTypeCombo;
    QDoubleSpinBox* m_priceTriggerInput;
    QDoubleSpinBox* m_stopLossInput;
    QDoubleSpinBox* m_takeProfitInput;
    
    // Schedule
    QDateTimeEdit* m_startDateEdit;
    QDateTimeEdit* m_endDateEdit;
    
    // Buttons
    QPushButton* m_createButton;
    QPushButton* m_cancelButton;
};

/**
 * @brief Swap History Table Widget
 */
class SwapHistoryTable : public QTableWidget
{
    Q_OBJECT

public:
    explicit SwapHistoryTable(QWidget* parent = nullptr);
    void addSwapRecord(const AutoSwapEngine::SwapHistory& history);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private Q_SLOTS:
    void onCopyTransactionId();
    void onCopyAmount();
    void onExportSwap();
};

/**
 * @brief Performance Analytics Dialog
 */
class PerformanceAnalyticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PerformanceAnalyticsDialog(const QList<AutoSwapEngine::SwapStrategy>& strategies,
                                       const QList<AutoSwapEngine::SwapHistory>& history,
                                       QWidget* parent = nullptr);

private:
    void setupUI();
    void updateCharts();
    void calculateMetrics();

    QList<AutoSwapEngine::SwapStrategy> m_strategies;
    QList<AutoSwapEngine::SwapHistory> m_history;
    QVBoxLayout* m_layout;
    QChartView* m_chartView;
    QChart* m_chart;
    QTextBrowser* m_metricsText;
};

#endif // SHAHCOIN_QT_AUTOSWAPENGINE_H
