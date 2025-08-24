#ifndef SHAHCOIN_QT_DASHBOARDWIDGET_H
#define SHAHCOIN_QT_DASHBOARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QProgressBar>
#include <QTimer>
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QLineSeries>

QT_CHARTS_USE_NAMESPACE

class WalletModel;

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget* parent = nullptr);
    void updatePortfolioData();
    void setWalletModel(WalletModel* model);

public Q_SLOTS:
    void onRefreshClicked();
    void onExportReportClicked();
    void onAutoRefreshToggled(bool enabled);

private:
    void setupUI();
    void setupCharts();
    void updateStatistics();
    void updateCharts();
    void loadPortfolioData();
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Summary Cards
    QGroupBox* m_summaryGroup;
    QGridLayout* m_summaryLayout;
    QLabel* m_totalBalanceLabel;
    QLabel* m_nftCountLabel;
    QLabel* m_tokenCountLabel;
    QLabel* m_stakingAmountLabel;
    QLabel* m_miningBlocksLabel;
    QLabel* m_fiatValueLabel;
    
    // Charts
    QGroupBox* m_chartsGroup;
    QHBoxLayout* m_chartsLayout;
    QChartView* m_pieChartView;
    QChartView* m_earningsChartView;
    
    // Controls
    QHBoxLayout* m_controlsLayout;
    QPushButton* m_refreshButton;
    QPushButton* m_exportButton;
    QPushButton* m_autoRefreshButton;
    
    // Auto-refresh timer
    QTimer* m_autoRefreshTimer;
    
                    // Data
                struct PortfolioData {
                    double totalBalance;
                    int nftCount;
                    int tokenCount;
                    double stakingAmount;
                    double stakingRewards;
                    int miningBlocks;
                    double fiatValue;
                    QList<QPair<QString, double>> assetBreakdown;
                    QList<QPair<QString, double>> earningsHistory;
                };
                
                PortfolioData m_portfolioData;
                WalletModel* m_walletModel;
};

#endif // SHAHCOIN_QT_DASHBOARDWIDGET_H
