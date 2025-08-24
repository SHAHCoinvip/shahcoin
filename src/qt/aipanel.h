#ifndef SHAHCOIN_QT_AIPANEL_H
#define SHAHCOIN_QT_AIPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QGroupBox>
#include <QTimer>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>

QT_CHARTS_USE_NAMESPACE

class WalletModel;
class CWallet;

/**
 * AI Insights Panel for Shahcoin Qt Wallet
 *
 * Displays intelligent insights about wallet activity, staking performance,
 * and transaction patterns using AI-powered analytics.
 */
class AIPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AIPanel(QWidget* parent = nullptr);
    ~AIPanel();

    // Panel management
    void setWalletModel(WalletModel* model);
    void refreshInsights();
    void updateStakingInsights();
    void updateTransactionInsights();
    void updatePerformanceMetrics();

    // Settings
    void setAutoRefresh(bool enabled);
    void setRefreshInterval(int seconds);
    void setShowCharts(bool show);
    void setShowPredictions(bool show);

    bool isAutoRefresh() const;
    int getRefreshInterval() const;
    bool isShowCharts() const;
    bool isShowPredictions() const;

Q_SIGNALS:
    void feeSavingsEstimated(double savingsShah);

public Q_SLOTS:
    void onRefreshClicked();
    void onAutoRefreshTimeout();
    void onWalletDataChanged();

private Q_SLOTS:
    void onStakingToggleClicked();
    void onOptimizeFeesClicked();
    void onViewDetailsClicked();

private:
    // UI setup
    void setupUI();
    void setupCharts();
    void setupConnections();
    void createInsightWidgets();
    void createStakingWidget();
    void createTransactionWidget();
    void createPerformanceWidget();

    // Data processing
    void calculateStakingMetrics();
    void calculateTransactionMetrics();
    void calculatePerformanceMetrics();
    void generateInsights();
    void updateCharts();

    // Chart management
    void createStakingChart();
    void createTransactionChart();
    void createPerformanceChart();
    void updateStakingChart();
    void updateTransactionChart();
    void updatePerformanceChart();

    // Utility functions
    QString formatAmount(CAmount amount);
    QString formatPercentage(double percentage);
    QString formatTime(int seconds);
    QString getStakingStatusText();
    QString getPerformanceRating();
    QString getInsightMessage();

    // Data members
    WalletModel* m_walletModel;
    QTimer* m_refreshTimer;

    // UI elements
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_stakingGroup;
    QGroupBox* m_transactionGroup;
    QGroupBox* m_performanceGroup;
    QGroupBox* m_insightsGroup;

    // Staking widgets
    QLabel* m_stakingStatusLabel;
    QLabel* m_stakingBalanceLabel;
    QLabel* m_stakingRewardsLabel;
    QLabel* m_stakingAPRLabel;
    QProgressBar* m_stakingProgressBar;
    QPushButton* m_stakingToggleButton;
    QChartView* m_stakingChartView;

    // Transaction widgets
    QLabel* m_totalSentLabel;
    QLabel* m_totalReceivedLabel;
    QLabel* m_transactionCountLabel;
    QLabel* m_averageFeeLabel;
    QPushButton* m_optimizeFeesButton;
    QChartView* m_transactionChartView;

    // Performance widgets
    QLabel* m_performanceRatingLabel;
    QLabel* m_networkHealthLabel;
    QLabel* m_syncStatusLabel;
    QLabel* m_peerCountLabel;
    QPushButton* m_viewDetailsButton;
    QChartView* m_performanceChartView;

    // Insights widgets
    QLabel* m_primaryInsightLabel;
    QLabel* m_secondaryInsightLabel;
    QLabel* m_recommendationLabel;
    QPushButton* m_refreshButton;

    // Settings
    bool m_autoRefresh;
    int m_refreshInterval;
    bool m_showCharts;
    bool m_showPredictions;

    // Data storage
    struct StakingMetrics {
        CAmount totalStaked;
        CAmount rewardsEarned;
        double apr;
        int daysStaking;
        bool isActive;
        QString status;
    };

    struct TransactionMetrics {
        CAmount totalSent;
        CAmount totalReceived;
        int transactionCount;
        double averageFee;
        int averageConfirmations;
        QStringList topRecipients;
    };

    struct PerformanceMetrics {
        double performanceRating;
        QString networkHealth;
        bool isSynced;
        int peerCount;
        int blockHeight;
        QString syncStatus;
    };

    StakingMetrics m_stakingMetrics;
    TransactionMetrics m_transactionMetrics;
    PerformanceMetrics m_performanceMetrics;

    // Charts
    QChart* m_stakingChart;
    QChart* m_transactionChart;
    QChart* m_performanceChart;
    QLineSeries* m_stakingSeries;
    QBarSeries* m_transactionSeries;
    QLineSeries* m_performanceSeries;
};

#endif // SHAHCOIN_QT_AIPANEL_H
