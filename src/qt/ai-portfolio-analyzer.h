// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_AI_PORTFOLIO_ANALYZER_H
#define SHAHCOIN_QT_AI_PORTFOLIO_ANALYZER_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Forward declarations to avoid hard dependency on Qt Charts/WebEngine at build time
namespace QtCharts { class QChart; class QChartView; class QPieSeries; class QBarSeries; class QLineSeries; }

namespace Ui {
class AIPortfolioAnalyzer;
}

/**
 * AI Portfolio Analyzer Widget
 * 
 * Provides deep AI-powered analysis of user's cryptocurrency portfolio,
 * including rebalancing suggestions, trend detection, and risk assessment.
 */
class AIPortfolioAnalyzer : public QWidget
{
    Q_OBJECT

public:
    explicit AIPortfolioAnalyzer(QWidget *parent = nullptr);
    ~AIPortfolioAnalyzer();

    // Portfolio analysis methods
    void analyzePortfolio();
    void generateRebalancingSuggestions();
    void detectTrends();
    void assessRisk();
    void optimizeAllocation();

    // Data management
    void loadPortfolioData();
    void saveAnalysisResults();
    void exportReport(const QString& format);

    // UI methods
    void updateCharts();
    void updateTables();
    void updateRecommendations();

signals:
    void analysisComplete(const QJsonObject& results);
    void rebalancingSuggested(const QJsonArray& suggestions);
    void riskAlert(const QString& alert);
    void trendDetected(const QString& trend);

public slots:
    void onAnalysisComplete();
    void onRebalancingSuggested();
    void onRiskAssessmentComplete();
    void onTrendDetectionComplete();

private slots:
    void onAnalyzeButtonClicked();
    void onRebalanceButtonClicked();
    void onExportButtonClicked();
    void onRefreshButtonClicked();
    void onSettingsButtonClicked();
    void onTimeframeChanged(int index);
    void onRiskToleranceChanged(int value);
    void onIncludeNFTsToggled(bool checked);
    void onIncludeDeFiToggled(bool checked);
    void onNetworkResponse(QNetworkReply* reply);
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    Ui::AIPortfolioAnalyzer *ui;
    QNetworkAccessManager *m_networkManager;
    QTimer *m_updateTimer;
    
    // Charts and visualizations
    QChart *m_allocationChart;
    QChart *m_performanceChart;
    QChart *m_riskChart;
    QChart *m_trendChart;
    
    // Data structures
    QJsonObject m_portfolioData;
    QJsonObject m_analysisResults;
    QJsonArray m_rebalancingSuggestions;
    QJsonObject m_riskAssessment;
    QJsonArray m_trends;
    
    // Configuration
    int m_timeframe;           // Analysis timeframe in days
    int m_riskTolerance;       // Risk tolerance level (1-10)
    bool m_includeNFTs;        // Include NFTs in analysis
    bool m_includeDeFi;        // Include DeFi positions
    bool m_autoRebalance;      // Auto-rebalancing enabled
    
    // UI components
    QTabWidget *m_tabWidget;
    QChartView *m_allocationChartView;
    QChartView *m_performanceChartView;
    QChartView *m_riskChartView;
    QChartView *m_trendChartView;
    QTableWidget *m_holdingsTable;
    QTableWidget *m_suggestionsTable;
    QTableWidget *m_riskTable;
    QTextEdit *m_recommendationsText;
    QListWidget *m_alertsList;
    QTreeWidget *m_analysisTree;
    
    // Controls
    QComboBox *m_timeframeCombo;
    QSpinBox *m_riskToleranceSpin;
    QCheckBox *m_includeNFTsCheck;
    QCheckBox *m_includeDeFiCheck;
    QCheckBox *m_autoRebalanceCheck;
    QPushButton *m_analyzeButton;
    QPushButton *m_rebalanceButton;
    QPushButton *m_exportButton;
    QPushButton *m_refreshButton;
    QPushButton *m_settingsButton;
    
    // Progress indicators
    QProgressBar *m_analysisProgress;
    QLabel *m_statusLabel;
    QLabel *m_lastUpdateLabel;

    // Setup methods
    void setupUI();
    void setupCharts();
    void setupTables();
    void setupConnections();
    void setupNetworkManager();
    
    // Analysis methods
    void performPortfolioAnalysis();
    void calculateAllocation();
    void calculatePerformance();
    void calculateRiskMetrics();
    void detectMarketTrends();
    void generateAIRecommendations();
    
    // Chart methods
    void createAllocationChart();
    void createPerformanceChart();
    void createRiskChart();
    void createTrendChart();
    void updateChartData();
    
    // Table methods
    void populateHoldingsTable();
    void populateSuggestionsTable();
    void populateRiskTable();
    void updateAnalysisTree();
    
    // Network methods
    void fetchPriceData();
    void fetchMarketData();
    void fetchHistoricalData();
    void sendAnalysisRequest();
    
    // Utility methods
    double calculateTotalValue();
    double calculateAllocationPercentage(const QString& asset);
    double calculateRiskScore(const QString& asset);
    QString formatCurrency(double amount);
    QString formatPercentage(double percentage);
    QColor getRiskColor(double riskScore);
    QColor getPerformanceColor(double performance);
    
    // AI methods
    void runAIAnalysis();
    void processAIResponse(const QJsonObject& response);
    void generateRebalancingPlan();
    void assessPortfolioRisk();
    void predictTrends();
    
    // Configuration methods
    void loadSettings();
    void saveSettings();
    void applySettings();
    
    // Export methods
    void exportToPDF();
    void exportToCSV();
    void exportToJSON();
    void generateReport();
};

/**
 * Portfolio Analysis Results Structure
 */
struct PortfolioAnalysis {
    double totalValue;
    double totalChange24h;
    double totalChange7d;
    double totalChange30d;
    double riskScore;
    double diversificationScore;
    double performanceScore;
    QJsonArray topPerformers;
    QJsonArray underPerformers;
    QJsonArray rebalancingSuggestions;
    QJsonArray riskAlerts;
    QJsonArray trends;
    QString aiRecommendation;
};

/**
 * Rebalancing Suggestion Structure
 */
struct RebalancingSuggestion {
    QString asset;
    QString action; // "buy", "sell", "hold"
    double amount;
    double percentage;
    QString reason;
    double priority; // 1-10
    double expectedImpact;
};

/**
 * Risk Assessment Structure
 */
struct RiskAssessment {
    QString asset;
    double volatility;
    double correlation;
    double concentrationRisk;
    double marketRisk;
    double liquidityRisk;
    double totalRisk;
    QString riskLevel; // "low", "medium", "high", "critical"
    QString recommendation;
};

/**
 * Trend Analysis Structure
 */
struct TrendAnalysis {
    QString asset;
    QString trend; // "bullish", "bearish", "sideways"
    double strength; // 0-1
    QString timeframe;
    QString pattern;
    double confidence;
    QString prediction;
};

#endif // SHAHCOIN_QT_AI_PORTFOLIO_ANALYZER_H
