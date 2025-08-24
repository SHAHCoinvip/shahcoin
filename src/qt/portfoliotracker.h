#ifndef SHAHCOIN_QT_PORTFOLIOTRACKER_H
#define SHAHCOIN_QT_PORTFOLIOTRACKER_H

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

class WalletModel;
class PlatformStyle;

/**
 * @brief Portfolio Performance Tracker
 *
 * Provides comprehensive portfolio tracking and analysis:
 * - SHAH balance value over time (daily/weekly/monthly)
 * - Staking rewards tracking
 * - Token/NFT value integration
 * - Performance charts and analytics
 * - Date range selection and filtering
 * - Export capabilities for analysis
 */
class PortfolioTracker : public QWidget
{
    Q_OBJECT

public:
    enum TimeRange {
        Day = 0,
        Week,
        Month,
        Quarter,
        Year,
        Custom
    };

    enum ChartType {
        BalanceChart = 0,
        PerformanceChart,
        RewardsChart,
        TokenChart
    };

    struct PortfolioData {
        QDateTime timestamp;
        double shahBalance;
        double shahValue;
        double stakingRewards;
        double tokenValue;
        double nftValue;
        double totalValue;
        double changePercent;
    };

    struct PerformanceMetrics {
        double totalReturn;
        double averageReturn;
        double bestDay;
        double worstDay;
        double volatility;
        double sharpeRatio;
        int daysTracked;
    };

    explicit PortfolioTracker(WalletModel* walletModel, QWidget* parent = nullptr);
    ~PortfolioTracker();

    // Main tracking methods
    void refreshPortfolioData();
    void updatePortfolioValue();
    void calculatePerformanceMetrics();
    void exportPortfolioData();

    // Chart management
    void updateBalanceChart();
    void updatePerformanceChart();
    void updateRewardsChart();
    void updateTokenChart();

    // Settings
    void saveSettings();
    void loadSettings();

signals:
    void portfolioUpdated(const QList<PortfolioData>& data);
    void performanceCalculated(const PerformanceMetrics& metrics);
    void dataExported(const QString& filename);

public Q_SLOTS:
    void onRefreshClicked();
    void onTimeRangeChanged(int index);
    void onChartTypeChanged(int index);
    void onExportDataClicked();
    void onShowAdvancedClicked();
    void onAutoRefreshToggled(bool enabled);
    void onIncludeStakingToggled(bool enabled);
    void onIncludeTokensToggled(bool enabled);
    void onIncludeNFTsToggled(bool enabled);

private Q_SLOTS:
    void onUpdateTimer();
    void onChartRangeChanged();
    void onPortfolioItemClicked(QTableWidgetItem* item);
    void onContextMenuRequested(const QPoint& pos);

private:
    void setupUI();
    void connectSignals();
    void applyTheme();

    // Data management
    QList<PortfolioData> fetchPortfolioData(TimeRange range);
    PortfolioData calculatePortfolioSnapshot();
    PerformanceMetrics calculateMetrics(const QList<PortfolioData>& data);
    double fetchShahPrice(const QDateTime& timestamp);
    double fetchTokenValue(const QString& tokenId);
    double fetchNFTValue(const QString& nftId);

    // Chart helpers
    void setupCharts();
    void addDataPoint(const PortfolioData& data);
    void clearOldDataPoints();
    void updateChartAxes();

    // UI helpers
    void updatePortfolioDisplay();
    void updateMetricsDisplay();
    void showExportDialog();
    void animateValueUpdate();
    void showNotification(const QString& message, const QString& type = "info");

    // Data persistence
    void savePortfolioHistory();
    void loadPortfolioHistory();
    void cleanupOldHistory();

    // Members
    WalletModel* m_walletModel;
    PlatformStyle* m_platformStyle;

    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Portfolio Overview
    QGroupBox* m_overviewGroup;
    QHBoxLayout* m_overviewLayout;
    QLabel* m_totalValueLabel;
    QLabel* m_changeLabel;
    QLabel* m_changePercentLabel;
    QPushButton* m_refreshButton;

    // Controls
    QGroupBox* m_controlsGroup;
    QHBoxLayout* m_controlsLayout;
    QLabel* m_timeRangeLabel;
    QComboBox* m_timeRangeCombo;
    QLabel* m_chartTypeLabel;
    QComboBox* m_chartTypeCombo;
    QCheckBox* m_includeStakingCheck;
    QCheckBox* m_includeTokensCheck;
    QCheckBox* m_includeNFTsCheck;
    QCheckBox* m_autoRefreshCheck;

    // Performance Metrics
    QGroupBox* m_metricsGroup;
    QHBoxLayout* m_metricsLayout;
    QLabel* m_totalReturnLabel;
    QLabel* m_averageReturnLabel;
    QLabel* m_bestDayLabel;
    QLabel* m_worstDayLabel;
    QLabel* m_volatilityLabel;
    QLabel* m_sharpeRatioLabel;

    // Charts
    QGroupBox* m_chartsGroup;
    QVBoxLayout* m_chartsLayout;
    QChartView* m_chartView;
    QChart* m_chart;
    QLineSeries* m_balanceSeries;
    QLineSeries* m_rewardsSeries;
    QLineSeries* m_tokenSeries;
    QValueAxis* m_valueAxis;
    QDateTimeAxis* m_timeAxis;

    // Portfolio History Table
    QGroupBox* m_historyGroup;
    QTableWidget* m_portfolioTable;
    QPushButton* m_exportButton;

    // State
    QList<PortfolioData> m_portfolioHistory;
    PerformanceMetrics m_currentMetrics;
    TimeRange m_currentTimeRange;
    ChartType m_currentChartType;
    bool m_autoRefreshEnabled;
    bool m_includeStaking;
    bool m_includeTokens;
    bool m_includeNFTs;

    // Network and timing
    QNetworkAccessManager* m_networkManager;
    QTimer* m_updateTimer;
    QTimer* m_animationTimer;
    QDateTime m_lastUpdate;

    // Data storage
    QSettings* m_settings;
    QList<QPair<QDateTime, double>> m_priceHistory;

    // Animation
    QPropertyAnimation* m_valueAnimation;
    QGraphicsOpacityEffect* m_valueOpacityEffect;

    // Constants
    static const int UPDATE_INTERVAL_MS = 300000; // 5 minutes
    static const int ANIMATION_DURATION_MS = 500;
    static const int MAX_HISTORY_POINTS = 1000;
    static const QString SETTINGS_GROUP;
    static const QString SETTINGS_TIME_RANGE;
    static const QString SETTINGS_CHART_TYPE;
    static const QString SETTINGS_AUTO_REFRESH;
    static const QString SETTINGS_INCLUDE_STAKING;
    static const QString SETTINGS_INCLUDE_TOKENS;
    static const QString SETTINGS_INCLUDE_NFTS;
};

/**
 * @brief Portfolio History Table Widget
 */
class PortfolioHistoryTable : public QTableWidget
{
    Q_OBJECT

public:
    explicit PortfolioHistoryTable(QWidget* parent = nullptr);
    void addPortfolioRecord(const QDateTime& timestamp, double totalValue, double changePercent, bool positive);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private Q_SLOTS:
    void onCopyValue();
    void onCopyTimestamp();
    void onExportSelected();
};

/**
 * @brief Performance Metrics Display Widget
 */
class PerformanceMetricsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PerformanceMetricsWidget(QWidget* parent = nullptr);
    void updateMetrics(const PortfolioTracker::PerformanceMetrics& metrics);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    PortfolioTracker::PerformanceMetrics m_metrics;
    QColor m_positiveColor;
    QColor m_negativeColor;
};

#endif // SHAHCOIN_QT_PORTFOLIOTRACKER_H
