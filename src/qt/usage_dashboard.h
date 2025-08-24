#ifndef USAGE_DASHBOARD_H
#define USAGE_DASHBOARD_H

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

// Usage Tracking Structures
struct FeatureUsage {
    QString featureName;
    QString featureCategory;
    int usageCount;
    int totalUsageTime; // in seconds
    QDateTime firstUsed;
    QDateTime lastUsed;
    double averageSessionTime; // in seconds
    int successfulOperations;
    int failedOperations;
    double successRate;
    QString userFeedback; // "positive", "negative", "neutral"
    bool isPremiumFeature;
    QString subscriptionTier; // "basic", "pro", "premium"
};

struct UsageSession {
    QString sessionId;
    QString featureName;
    QDateTime startTime;
    QDateTime endTime;
    int duration; // in seconds
    bool wasSuccessful;
    QString errorMessage;
    QMap<QString, QVariant> sessionData;
    QString userAgent;
    QString platform; // "desktop", "mobile", "web"
};

struct UsageAnalytics {
    QDateTime periodStart;
    QDateTime periodEnd;
    int totalSessions;
    int totalUsageTime;
    int uniqueFeaturesUsed;
    double averageSessionDuration;
    double successRate;
    QMap<QString, int> featureUsageCounts;
    QMap<QString, int> featureUsageTime;
    QMap<QString, double> featureSuccessRates;
    QList<QString> mostUsedFeatures;
    QList<QString> leastUsedFeatures;
    QList<QString> mostSuccessfulFeatures;
    QList<QString> leastSuccessfulFeatures;
    double premiumFeatureUsage;
    double basicFeatureUsage;
    QString peakUsageTime;
    QString peakUsageDay;
};

struct UserInsights {
    QString insightType; // "usage_pattern", "feature_discovery", "efficiency", "engagement"
    QString title;
    QString description;
    QString recommendation;
    double confidence; // 0.0 to 1.0
    QString priority; // "low", "medium", "high", "critical"
    QDateTime generatedAt;
    bool isActionable;
    QString actionUrl;
    QMap<QString, QVariant> insightData;
};

struct SubscriptionMetrics {
    QString subscriptionTier;
    QDateTime subscriptionStart;
    QDateTime subscriptionEnd;
    double monthlyUsage;
    double monthlyValue;
    double costPerUse;
    double roi; // Return on Investment
    QList<QString> unusedFeatures;
    QList<QString> overusedFeatures;
    double featureUtilizationRate;
    QString recommendedTier;
    double potentialSavings;
    double potentialValue;
};

// Usage Dashboard Manager Class
class UsageDashboardManager : public QObject
{
    Q_OBJECT

public:
    explicit UsageDashboardManager(QObject* parent = nullptr);
    ~UsageDashboardManager();

    // Core Usage Tracking
    void startSession(const QString& featureName, const QMap<QString, QVariant>& sessionData = {});
    void endSession(const QString& sessionId, bool wasSuccessful = true, const QString& errorMessage = "");
    void trackFeatureUsage(const QString& featureName, bool wasSuccessful = true);
    void recordUserFeedback(const QString& featureName, const QString& feedback);
    
    // Analytics Generation
    UsageAnalytics generateAnalytics(const QDateTime& startDate, const QDateTime& endDate);
    QList<UserInsights> generateInsights();
    SubscriptionMetrics calculateSubscriptionMetrics();
    
    // Data Management
    void loadUsageData();
    void saveUsageData();
    void exportUsageReport(const QString& filePath, const QString& format = "csv");
    void clearUsageData();
    
    // Settings Management
    void loadSettings();
    void saveSettings();
    
    // Getters and Setters
    bool isTrackingEnabled() const { return m_trackingEnabled; }
    bool isAnalyticsEnabled() const { return m_analyticsEnabled; }
    bool isInsightsEnabled() const { return m_insightsEnabled; }
    int getTrackingInterval() const { return m_trackingInterval; }
    bool isPrivacyMode() const { return m_privacyMode; }
    
    void setTrackingEnabled(bool enabled);
    void setAnalyticsEnabled(bool enabled);
    void setInsightsEnabled(bool enabled);
    void setTrackingInterval(int interval);
    void setPrivacyMode(bool enabled);
    
    QList<FeatureUsage> getFeatureUsage() const { return m_featureUsage; }
    QList<UsageSession> getUsageSessions() const { return m_usageSessions; }
    QList<UserInsights> getUserInsights() const { return m_userInsights; }
    SubscriptionMetrics getSubscriptionMetrics() const { return m_subscriptionMetrics; }

signals:
    void sessionStarted(const QString& sessionId, const QString& featureName);
    void sessionEnded(const QString& sessionId, bool wasSuccessful);
    void featureUsed(const QString& featureName, bool wasSuccessful);
    void analyticsGenerated(const UsageAnalytics& analytics);
    void insightsGenerated(const QList<UserInsights>& insights);
    void subscriptionMetricsUpdated(const SubscriptionMetrics& metrics);
    void trackingEnabledChanged(bool enabled);
    void privacyModeChanged(bool enabled);

private slots:
    void performPeriodicAnalytics();
    void cleanupOldData();

private:
    // Core Components
    QSettings m_settings;
    QTimer* m_trackingTimer;
    QTimer* m_analyticsTimer;
    QTimer* m_cleanupTimer;
    
    // Settings
    bool m_trackingEnabled;
    bool m_analyticsEnabled;
    bool m_insightsEnabled;
    int m_trackingInterval;
    bool m_privacyMode;
    int m_dataRetentionDays;
    
    // Data Storage
    QList<FeatureUsage> m_featureUsage;
    QList<UsageSession> m_usageSessions;
    QList<UserInsights> m_userInsights;
    SubscriptionMetrics m_subscriptionMetrics;
    QMap<QString, QDateTime> m_activeSessions;
    
    // Helper Methods
    void setupTimers();
    void setupDataStructures();
    void updateFeatureUsage(const QString& featureName, bool wasSuccessful, int duration = 0);
    void generateUsageInsights();
    void calculateFeatureMetrics();
    void identifyUsagePatterns();
    void recommendFeatures();
    void optimizeSubscription();
    QString generateSessionId();
    QDateTime getCurrentTimestamp();
    void anonymizeData();
    void compressData();
    bool shouldTrackFeature(const QString& featureName);
    double calculateSuccessRate(int successful, int total);
    QString determinePeakUsageTime();
    QString determinePeakUsageDay();
};

// Usage Dashboard Widget
class UsageDashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UsageDashboardWidget(UsageDashboardManager* manager, QWidget* parent = nullptr);
    ~UsageDashboardWidget();

    void refreshData();
    void showInsightsDialog();

private slots:
    void onRefreshClicked();
    void onExportClicked();
    void onSettingsClicked();
    void onClearDataClicked();
    void onInsightsClicked();
    void onAnalyticsGenerated(const UsageAnalytics& analytics);
    void onInsightsGenerated(const QList<UserInsights>& insights);
    void onSubscriptionMetricsUpdated(const SubscriptionMetrics& metrics);

private:
    // Core Components
    UsageDashboardManager* m_manager;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Overview Tab
    QWidget* m_overviewTab;
    QLabel* m_totalUsageLabel;
    QLabel* m_totalSessionsLabel;
    QLabel* m_successRateLabel;
    QLabel* m_averageSessionLabel;
    QLabel* m_premiumUsageLabel;
    QProgressBar* m_usageProgressBar;
    QChartView* m_usageChart;
    QChartView* m_successRateChart;
    
    // Features Tab
    QWidget* m_featuresTab;
    QTableWidget* m_featuresTable;
    QPushButton* m_sortByNameButton;
    QPushButton* m_sortByUsageButton;
    QPushButton* m_sortBySuccessButton;
    QChartView* m_featureUsageChart;
    QChartView* m_featureSuccessChart;
    
    // Sessions Tab
    QWidget* m_sessionsTab;
    QTableWidget* m_sessionsTable;
    QComboBox* m_sessionFilterCombo;
    QLineEdit* m_sessionSearchEdit;
    QChartView* m_sessionDurationChart;
    QChartView* m_sessionTimelineChart;
    
    // Insights Tab
    QWidget* m_insightsTab;
    QListWidget* m_insightsList;
    QTextEdit* m_insightDetails;
    QPushButton* m_applyInsightButton;
    QPushButton* m_dismissInsightButton;
    QChartView* m_insightsChart;
    
    // Subscription Tab
    QWidget* m_subscriptionTab;
    QLabel* m_currentTierLabel;
    QLabel* m_utilizationLabel;
    QLabel* m_costPerUseLabel;
    QLabel* m_roiLabel;
    QLabel* m_recommendedTierLabel;
    QLabel* m_potentialSavingsLabel;
    QProgressBar* m_utilizationBar;
    QChartView* m_subscriptionChart;
    QChartView* m_roiChart;
    
    // Control Panel
    QWidget* m_controlPanel;
    QPushButton* m_refreshButton;
    QPushButton* m_exportButton;
    QPushButton* m_settingsButton;
    QPushButton* m_clearDataButton;
    QPushButton* m_insightsButton;
    QLabel* m_statusLabel;
    QProgressBar* m_trackingProgress;
    
    // Helper Methods
    void setupUI();
    void setupOverviewTab();
    void setupFeaturesTab();
    void setupSessionsTab();
    void setupInsightsTab();
    void setupSubscriptionTab();
    void setupControlPanel();
    
    void updateOverviewDisplay(const UsageAnalytics& analytics);
    void updateFeaturesTable(const QList<FeatureUsage>& features);
    void updateSessionsTable(const QList<UsageSession>& sessions);
    void updateInsightsList(const QList<UserInsights>& insights);
    void updateSubscriptionDisplay(const SubscriptionMetrics& metrics);
    void updateStatusDisplay();
    
    QChart* createUsageChart(const UsageAnalytics& analytics);
    QChart* createSuccessRateChart(const UsageAnalytics& analytics);
    QChart* createFeatureUsageChart(const QList<FeatureUsage>& features);
    QChart* createFeatureSuccessChart(const QList<FeatureUsage>& features);
    QChart* createSessionDurationChart(const QList<UsageSession>& sessions);
    QChart* createSessionTimelineChart(const QList<UsageSession>& sessions);
    QChart* createInsightsChart(const QList<UserInsights>& insights);
    QChart* createSubscriptionChart(const SubscriptionMetrics& metrics);
    QChart* createROIChart(const SubscriptionMetrics& metrics);
    
    QString formatDuration(int seconds);
    QString formatPercentage(double percentage);
    QString formatNumber(int number);
    QString formatCurrency(double amount);
    QColor getSuccessRateColor(double rate);
    QColor getUsageIntensityColor(int usage);
    QColor getInsightPriorityColor(const QString& priority);
};

// Usage Settings Dialog
class UsageSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UsageSettingsDialog(UsageDashboardManager* manager, QWidget* parent = nullptr);

private slots:
    void onTrackingToggled(bool enabled);
    void onAnalyticsToggled(bool enabled);
    void onInsightsToggled(bool enabled);
    void onPrivacyToggled(bool enabled);
    void onIntervalChanged(int seconds);
    void onRetentionChanged(int days);
    void onOkClicked();
    void onCancelClicked();

private:
    UsageDashboardManager* m_manager;
    
    // UI Components
    QVBoxLayout* m_layout;
    QCheckBox* m_trackingCheck;
    QCheckBox* m_analyticsCheck;
    QCheckBox* m_insightsCheck;
    QCheckBox* m_privacyCheck;
    QSpinBox* m_intervalSpin;
    QSpinBox* m_retentionSpin;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    void setupUI();
    void loadCurrentSettings();
};

// Insights Dialog
class InsightsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsightsDialog(const QList<UserInsights>& insights, QWidget* parent = nullptr);

private slots:
    void onInsightSelected(int index);
    void onApplyInsightClicked();
    void onDismissInsightClicked();
    void onExportInsightsClicked();

private:
    QList<UserInsights> m_insights;
    
    // UI Components
    QVBoxLayout* m_layout;
    QListWidget* m_insightsList;
    QTextEdit* m_insightDetails;
    QPushButton* m_applyButton;
    QPushButton* m_dismissButton;
    QPushButton* m_exportButton;
    QPushButton* m_closeButton;
    
    void setupUI();
    void populateInsightsList();
    void showInsightDetails(const UserInsights& insight);
};

// Export Dialog
class UsageExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UsageExportDialog(UsageDashboardManager* manager, QWidget* parent = nullptr);

private slots:
    void onExportClicked();
    void onCancelClicked();
    void onBrowseClicked();

private:
    UsageDashboardManager* m_manager;
    
    // UI Components
    QVBoxLayout* m_layout;
    QComboBox* m_formatCombo;
    QComboBox* m_dateRangeCombo;
    QCheckBox* m_includeFeaturesCheck;
    QCheckBox* m_includeSessionsCheck;
    QCheckBox* m_includeInsightsCheck;
    QCheckBox* m_includeAnalyticsCheck;
    QLineEdit* m_filePathEdit;
    QPushButton* m_browseButton;
    QPushButton* m_exportButton;
    QPushButton* m_cancelButton;
    
    void setupUI();
    void exportData();
};

#endif // USAGE_DASHBOARD_H
