#ifndef AI_PORTFOLIO_ANALYZER_H
#define AI_PORTFOLIO_ANALYZER_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QProgressBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QPieSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QDateTimeAxis>
#include <QSettings>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QString>
#include <QColor>
#include <QPoint>
#include <QSize>

// Portfolio Analysis Structures
struct PortfolioAsset {
    QString symbol;
    QString name;
    double quantity;
    double currentPrice;
    double totalValue;
    double percentageOfPortfolio;
    double change24h;
    double change7d;
    double change30d;
    QString category;
    double riskScore;
    QDateTime lastUpdated;
};

struct PortfolioMetrics {
    double totalValue;
    double totalChange24h;
    double totalChange7d;
    double totalChange30d;
    double volatility;
    double sharpeRatio;
    double maxDrawdown;
    double beta;
    double alpha;
    double correlation;
    double diversificationScore;
    double riskAdjustedReturn;
    QDateTime analysisDate;
};

struct AIPrediction {
    QString assetSymbol;
    double predictedPrice;
    double confidence;
    QString predictionType; // "short_term", "medium_term", "long_term"
    QString reasoning;
    QDateTime predictionDate;
    QDateTime targetDate;
};

struct PortfolioRecommendation {
    QString type; // "buy", "sell", "hold", "rebalance"
    QString assetSymbol;
    double suggestedQuantity;
    QString reasoning;
    double priority; // 0.0 to 1.0
    QString riskLevel; // "low", "medium", "high"
    QDateTime recommendationDate;
};

// AI Portfolio Analyzer Class
class AIPortfolioAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit AIPortfolioAnalyzer(QObject* parent = nullptr);
    ~AIPortfolioAnalyzer();

    // Core Analysis Methods
    PortfolioMetrics analyzePortfolio(const QList<PortfolioAsset>& assets);
    QList<AIPrediction> generatePredictions(const QList<PortfolioAsset>& assets);
    QList<PortfolioRecommendation> generateRecommendations(const QList<PortfolioAsset>& assets, const PortfolioMetrics& metrics);
    
    // Risk Analysis
    double calculateRiskScore(const PortfolioAsset& asset);
    double calculatePortfolioVolatility(const QList<PortfolioAsset>& assets);
    double calculateSharpeRatio(const QList<PortfolioAsset>& assets);
    double calculateMaxDrawdown(const QList<PortfolioAsset>& assets);
    
    // Performance Analysis
    double calculateTotalReturn(const QList<PortfolioAsset>& assets);
    double calculateDiversificationScore(const QList<PortfolioAsset>& assets);
    QMap<QString, double> calculateSectorAllocation(const QList<PortfolioAsset>& assets);
    
    // AI Prediction Methods
    double predictAssetPrice(const QString& symbol, int daysAhead);
    double predictMarketTrend(const QList<PortfolioAsset>& assets);
    QString predictOptimalRebalancing(const QList<PortfolioAsset>& assets);
    
    // Settings Management
    void loadSettings();
    void saveSettings();
    
    // Getters and Setters
    bool isAnalysisEnabled() const { return m_analysisEnabled; }
    bool isPredictionEnabled() const { return m_predictionEnabled; }
    bool isRecommendationEnabled() const { return m_recommendationEnabled; }
    int getAnalysisInterval() const { return m_analysisInterval; }
    double getRiskTolerance() const { return m_riskTolerance; }
    QString getAnalysisModel() const { return m_analysisModel; }
    
    void setAnalysisEnabled(bool enabled);
    void setPredictionEnabled(bool enabled);
    void setRecommendationEnabled(bool enabled);
    void setAnalysisInterval(int interval);
    void setRiskTolerance(double tolerance);
    void setAnalysisModel(const QString& model);

signals:
    void analysisCompleted(const PortfolioMetrics& metrics);
    void predictionsGenerated(const QList<AIPrediction>& predictions);
    void recommendationsGenerated(const QList<PortfolioRecommendation>& recommendations);
    void analysisError(const QString& error);
    void settingsChanged();

private slots:
    void performPeriodicAnalysis();
    void onNetworkReplyFinished();

private:
    // Core Components
    QSettings m_settings;
    QTimer* m_analysisTimer;
    QNetworkAccessManager* m_networkManager;
    
    // Settings
    bool m_analysisEnabled;
    bool m_predictionEnabled;
    bool m_recommendationEnabled;
    int m_analysisInterval;
    double m_riskTolerance;
    QString m_analysisModel;
    
    // Data Storage
    QList<PortfolioAsset> m_currentAssets;
    PortfolioMetrics m_lastMetrics;
    QList<AIPrediction> m_lastPredictions;
    QList<PortfolioRecommendation> m_lastRecommendations;
    
    // AI Model Data
    QMap<QString, QList<double>> m_historicalPrices;
    QMap<QString, double> m_assetVolatility;
    QMap<QString, double> m_assetCorrelation;
    
    // Helper Methods
    void setupTimers();
    void setupNetworkMonitoring();
    void fetchHistoricalData(const QString& symbol);
    void updateAssetData();
    double calculateCorrelation(const QList<double>& series1, const QList<double>& series2);
    double calculateStandardDeviation(const QList<double>& values);
    double calculateMean(const QList<double>& values);
    QString categorizeAsset(const QString& symbol);
    double estimateRiskScore(const PortfolioAsset& asset);
};

// AI Portfolio Analyzer Widget
class AIPortfolioAnalyzerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AIPortfolioAnalyzerWidget(AIPortfolioAnalyzer* analyzer, QWidget* parent = nullptr);
    ~AIPortfolioAnalyzerWidget();

    void updatePortfolioData(const QList<PortfolioAsset>& assets);
    void showAnalysisResults(const PortfolioMetrics& metrics);
    void showPredictions(const QList<AIPrediction>& predictions);
    void showRecommendations(const QList<PortfolioRecommendation>& recommendations);

private slots:
    void onAnalyzeClicked();
    void onRefreshClicked();
    void onExportClicked();
    void onSettingsClicked();
    void onAnalysisCompleted(const PortfolioMetrics& metrics);
    void onPredictionsGenerated(const QList<AIPrediction>& predictions);
    void onRecommendationsGenerated(const QList<PortfolioRecommendation>& recommendations);
    void onAnalysisError(const QString& error);

private:
    // Core Components
    AIPortfolioAnalyzer* m_analyzer;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Overview Tab
    QWidget* m_overviewTab;
    QLabel* m_totalValueLabel;
    QLabel* m_totalChangeLabel;
    QLabel* m_volatilityLabel;
    QLabel* m_sharpeRatioLabel;
    QLabel* m_diversificationLabel;
    QProgressBar* m_riskBar;
    QChartView* m_performanceChart;
    
    // Assets Tab
    QWidget* m_assetsTab;
    QTableWidget* m_assetsTable;
    
    // Predictions Tab
    QWidget* m_predictionsTab;
    QTableWidget* m_predictionsTable;
    QChartView* m_predictionsChart;
    
    // Recommendations Tab
    QWidget* m_recommendationsTab;
    QTableWidget* m_recommendationsTable;
    QTextEdit* m_recommendationsText;
    
    // Analysis Tab
    QWidget* m_analysisTab;
    QChartView* m_correlationChart;
    QChartView* m_sectorChart;
    QChartView* m_riskChart;
    
    // Control Panel
    QWidget* m_controlPanel;
    QPushButton* m_analyzeButton;
    QPushButton* m_refreshButton;
    QPushButton* m_exportButton;
    QPushButton* m_settingsButton;
    QProgressBar* m_analysisProgress;
    
    // Helper Methods
    void setupUI();
    void setupOverviewTab();
    void setupAssetsTab();
    void setupPredictionsTab();
    void setupRecommendationsTab();
    void setupAnalysisTab();
    void setupControlPanel();
    
    void updateOverviewDisplay(const PortfolioMetrics& metrics);
    void updateAssetsTable(const QList<PortfolioAsset>& assets);
    void updatePredictionsTable(const QList<AIPrediction>& predictions);
    void updateRecommendationsTable(const QList<PortfolioRecommendation>& recommendations);
    
    QChart* createPerformanceChart(const QList<PortfolioAsset>& assets);
    QChart* createPredictionsChart(const QList<AIPrediction>& predictions);
    QChart* createCorrelationChart(const QList<PortfolioAsset>& assets);
    QChart* createSectorChart(const QList<PortfolioAsset>& assets);
    QChart* createRiskChart(const QList<PortfolioAsset>& assets);
    
    QString formatCurrency(double value);
    QString formatPercentage(double value);
    QString formatNumber(double value);
    QColor getChangeColor(double change);
    QColor getRiskColor(double risk);
};

// AI Portfolio Analyzer Settings Dialog
class AIPortfolioAnalyzerSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AIPortfolioAnalyzerSettingsDialog(AIPortfolioAnalyzer* analyzer, QWidget* parent = nullptr);

private slots:
    void onAnalysisEnabledToggled(bool enabled);
    void onPredictionEnabledToggled(bool enabled);
    void onRecommendationEnabledToggled(bool enabled);
    void onIntervalChanged(int seconds);
    void onRiskToleranceChanged(double tolerance);
    void onModelChanged(const QString& model);
    void onOkClicked();
    void onCancelClicked();

private:
    AIPortfolioAnalyzer* m_analyzer;
    
    // UI Components
    QVBoxLayout* m_layout;
    QCheckBox* m_analysisEnabledCheck;
    QCheckBox* m_predictionEnabledCheck;
    QCheckBox* m_recommendationEnabledCheck;
    QSpinBox* m_intervalSpin;
    QSlider* m_riskToleranceSlider;
    QLabel* m_riskToleranceLabel;
    QComboBox* m_modelCombo;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    void setupUI();
    void loadCurrentSettings();
};

#endif // AI_PORTFOLIO_ANALYZER_H
