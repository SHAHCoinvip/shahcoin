#ifndef SHAHCOIN_QT_PREMIUMFEATURES_H
#define SHAHCOIN_QT_PREMIUMFEATURES_H

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QTableWidget>
#include <QTabWidget>
#include <QGroupBox>
#include <QSlider>
#include <QSpinBox>
#include <QListWidget>
#include <QScrollArea>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QPieSeries>
#include <QValueAxis>

class WalletModel;
class PlatformStyle;
class PremiumSubscriptionManager;

/**
 * 1. AI Wallet Assistant - Premium Feature
 */
class AIWalletAssistant : public QWidget
{
    Q_OBJECT

public:
    explicit AIWalletAssistant(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent = nullptr);

private Q_SLOTS:
    void onSendMessageClicked();
    void onQuickActionClicked();
    void onLanguageChanged();
    void onClearChatClicked();

private:
    void setupUI();
    void addMessage(const QString& sender, const QString& message, bool isBot = false);
    void processAIResponse(const QString& userMessage);
    QString generateAIResponse(const QString& input);
    void addQuickActions();

    PremiumSubscriptionManager* m_premiumManager;
    WalletModel* m_walletModel;
    
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_chatScrollArea;
    QWidget* m_chatWidget;
    QVBoxLayout* m_chatLayout;
    QLineEdit* m_messageInput;
    QPushButton* m_sendButton;
    QComboBox* m_languageCombo;
    QPushButton* m_clearChatButton;
    QHBoxLayout* m_quickActionsLayout;
    
    QNetworkAccessManager* m_networkManager;
    int m_messageCounter;
};

/**
 * 2. AI Insights Panel - Premium Feature
 */
class AIInsightsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AIInsightsPanel(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent = nullptr);

public Q_SLOTS:
    void refreshInsights();

private Q_SLOTS:
    void onAnalyzePortfolioClicked();
    void onCheckSecurityClicked();
    void onOptimizeFeesClicked();

private:
    void setupUI();
    void loadSecurityInsights();
    void loadPerformanceInsights();
    void loadRecommendations();
    void generateAIInsight(const QString& category, const QString& data);

    PremiumSubscriptionManager* m_premiumManager;
    WalletModel* m_walletModel;
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Security Tab
    QWidget* m_securityTab;
    QLabel* m_securityScore;
    QListWidget* m_securityWarnings;
    QListWidget* m_securityRecommendations;
    
    // Performance Tab
    QWidget* m_performanceTab;
    QChartView* m_performanceChart;
    QLabel* m_portfolioValue;
    QLabel* m_change24h;
    QLabel* m_changeWeek;
    
    // Recommendations Tab
    QWidget* m_recommendationsTab;
    QListWidget* m_aiRecommendations;
    QPushButton* m_analyzeButton;
    QPushButton* m_securityButton;
    QPushButton* m_optimizeButton;
    
    QTimer* m_refreshTimer;
};

/**
 * 3. AI Fee Optimizer - Premium Feature
 */
class AIFeeOptimizer : public QWidget
{
    Q_OBJECT

public:
    explicit AIFeeOptimizer(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent = nullptr);

public Q_SLOTS:
    void analyzeFees();
    void optimizeTransaction(qint64 amount, const QString& recipient);

private Q_SLOTS:
    void onOptimizeClicked();
    void onApplyOptimizationClicked();
    void onFeePresetChanged();

private:
    void setupUI();
    void calculateOptimalFee(qint64 amount);
    void showFeeComparison();
    void updateFeeRecommendation();

    PremiumSubscriptionManager* m_premiumManager;
    WalletModel* m_walletModel;
    
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_currentFeesGroup;
    QGroupBox* m_optimizationGroup;
    QGroupBox* m_recommendationGroup;
    
    QLabel* m_currentFeeLabel;
    QLabel* m_networkStatusLabel;
    QLabel* m_estimatedTimeLabel;
    
    QComboBox* m_feePresetCombo;
    QSlider* m_customFeeSlider;
    QLabel* m_optimizedFeeLabel;
    QLabel* m_savingsLabel;
    QLabel* m_confirmationTimeLabel;
    
    QPushButton* m_optimizeButton;
    QPushButton* m_applyButton;
    
    QChartView* m_feeHistoryChart;
    
    qint64 m_currentAmount;
    QString m_currentRecipient;
};

/**
 * 4. AI Troubleshooter - Premium Feature
 */
class AITroubleshooter : public QWidget
{
    Q_OBJECT

public:
    explicit AITroubleshooter(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent = nullptr);

private Q_SLOTS:
    void onRunDiagnosticsClicked();
    void onFixIssueClicked();
    void onViewLogsClicked();
    void onContactSupportClicked();

private:
    void setupUI();
    void runSystemDiagnostics();
    void checkWalletHealth();
    void checkNetworkConnectivity();
    void checkBlockchainSync();
    void generateTroubleshootingReport();

    PremiumSubscriptionManager* m_premiumManager;
    WalletModel* m_walletModel;
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Diagnostics Tab
    QWidget* m_diagnosticsTab;
    QListWidget* m_diagnosticsList;
    QPushButton* m_runDiagnosticsButton;
    QProgressBar* m_diagnosticsProgress;
    QLabel* m_diagnosticsStatus;
    
    // Issues Tab
    QWidget* m_issuesTab;
    QTableWidget* m_issuesTable;
    QPushButton* m_fixSelectedButton;
    QPushButton* m_fixAllButton;
    
    // Logs Tab
    QWidget* m_logsTab;
    QTextEdit* m_logsDisplay;
    QPushButton* m_viewLogsButton;
    QPushButton* m_exportLogsButton;
    
    // Support Tab
    QWidget* m_supportTab;
    QTextEdit* m_issueDescription;
    QPushButton* m_contactSupportButton;
    QLabel* m_supportStatus;
};

/**
 * 5. Multilingual Assistant - Premium Feature
 */
class MultilingualAssistant : public QWidget
{
    Q_OBJECT

public:
    explicit MultilingualAssistant(PremiumSubscriptionManager* premiumManager, QWidget* parent = nullptr);

public Q_SLOTS:
    void translateText(const QString& text, const QString& targetLanguage);
    void setInterfaceLanguage(const QString& language);

private Q_SLOTS:
    void onTranslateClicked();
    void onLanguageChanged();
    void onVoiceInputClicked();
    void onSpeakOutputClicked();

private:
    void setupUI();
    void loadSupportedLanguages();
    void performTranslation(const QString& text, const QString& fromLang, const QString& toLang);

    PremiumSubscriptionManager* m_premiumManager;
    
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_translationGroup;
    QGroupBox* m_interfaceGroup;
    
    QComboBox* m_fromLanguageCombo;
    QComboBox* m_toLanguageCombo;
    QTextEdit* m_inputText;
    QTextEdit* m_outputText;
    QPushButton* m_translateButton;
    QPushButton* m_voiceInputButton;
    QPushButton* m_speakOutputButton;
    
    QComboBox* m_interfaceLanguageCombo;
    QPushButton* m_applyLanguageButton;
    
    QNetworkAccessManager* m_networkManager;
    QMap<QString, QString> m_languageCodes;
};

/**
 * 6. AI Art Booster - Premium Feature (Integration with existing AI Art system)
 */
class AIArtBooster : public QWidget
{
    Q_OBJECT

public:
    explicit AIArtBooster(PremiumSubscriptionManager* premiumManager, QWidget* parent = nullptr);

private Q_SLOTS:
    void onGenerateArtClicked();
    void onEnhanceImageClicked();
    void onApplyStyleClicked();
    void onSaveArtworkClicked();

private:
    void setupUI();
    void generateAIArtwork(const QString& prompt, const QString& style);
    void enhanceExistingImage(const QString& imagePath);
    void applyArtisticStyle(const QString& imagePath, const QString& style);

    PremiumSubscriptionManager* m_premiumManager;
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Generate Tab
    QWidget* m_generateTab;
    QLineEdit* m_promptInput;
    QComboBox* m_styleCombo;
    QComboBox* m_sizeCombo;
    QPushButton* m_generateButton;
    QLabel* m_generatedImageLabel;
    
    // Enhance Tab
    QWidget* m_enhanceTab;
    QPushButton* m_loadImageButton;
    QLabel* m_originalImageLabel;
    QLabel* m_enhancedImageLabel;
    QPushButton* m_enhanceButton;
    
    // Style Transfer Tab
    QWidget* m_styleTab;
    QPushButton* m_loadContentButton;
    QComboBox* m_artisticStyleCombo;
    QLabel* m_contentImageLabel;
    QLabel* m_styledImageLabel;
    QPushButton* m_applyStyleButton;
    
    QPushButton* m_saveButton;
    QNetworkAccessManager* m_networkManager;
};

/**
 * 7. NFT Collection Drops - Premium Feature
 */
class NFTCollectionDrops : public QWidget
{
    Q_OBJECT

public:
    explicit NFTCollectionDrops(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent = nullptr);

private Q_SLOTS:
    void onCreateDropClicked();
    void onLaunchDropClicked();
    void onManageDropClicked();
    void onViewAnalyticsClicked();

private:
    void setupUI();
    void loadActiveDrops();
    void loadDropHistory();

    PremiumSubscriptionManager* m_premiumManager;
    WalletModel* m_walletModel;
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Create Drop Tab
    QWidget* m_createTab;
    QLineEdit* m_dropNameInput;
    QSpinBox* m_totalSupplyInput;
    QLineEdit* m_baseURIInput;
    QDateTimeEdit* m_launchDateInput;
    QPushButton* m_createDropButton;
    
    // Active Drops Tab
    QWidget* m_activeTab;
    QTableWidget* m_activeDropsTable;
    QPushButton* m_launchButton;
    QPushButton* m_manageButton;
    
    // Analytics Tab
    QWidget* m_analyticsTab;
    QChartView* m_salesChart;
    QLabel* m_totalSalesLabel;
    QLabel* m_totalVolumeLabel;
    QPushButton* m_viewAnalyticsButton;
};

/**
 * 8. Auction Module - Premium Feature
 */
class AuctionModule : public QWidget
{
    Q_OBJECT

public:
    explicit AuctionModule(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent = nullptr);

private Q_SLOTS:
    void onCreateAuctionClicked();
    void onPlaceBidClicked();
    void onManageAuctionsClicked();
    void refreshAuctions();

private:
    void setupUI();
    void loadLiveAuctions();
    void loadMyAuctions();
    void loadAuctionHistory();

    PremiumSubscriptionManager* m_premiumManager;
    WalletModel* m_walletModel;
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Live Auctions Tab
    QWidget* m_liveTab;
    QTableWidget* m_liveAuctionsTable;
    QPushButton* m_placeBidButton;
    QPushButton* m_refreshButton;
    
    // My Auctions Tab
    QWidget* m_myAuctionsTab;
    QTableWidget* m_myAuctionsTable;
    QPushButton* m_createAuctionButton;
    QPushButton* m_manageButton;
    
    // History Tab
    QWidget* m_historyTab;
    QTableWidget* m_historyTable;
    
    QTimer* m_refreshTimer;
};

/**
 * 9. AI Portfolio Analyzer - Premium Feature
 */
class AIPortfolioAnalyzer : public QWidget
{
    Q_OBJECT

public:
    explicit AIPortfolioAnalyzer(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent = nullptr);

public Q_SLOTS:
    void analyzePortfolio();
    void generateReport();

private Q_SLOTS:
    void onAnalyzeClicked();
    void onOptimizeClicked();
    void onExportReportClicked();
    void onRebalanceClicked();

private:
    void setupUI();
    void loadPortfolioData();
    void performAIAnalysis();
    void generateOptimizationSuggestions();
    void calculateRiskScore();

    PremiumSubscriptionManager* m_premiumManager;
    WalletModel* m_walletModel;
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Overview Tab
    QWidget* m_overviewTab;
    QChartView* m_portfolioChart;
    QLabel* m_totalValueLabel;
    QLabel* m_riskScoreLabel;
    QLabel* m_diversificationLabel;
    
    // Analysis Tab
    QWidget* m_analysisTab;
    QTextEdit* m_analysisReport;
    QPushButton* m_analyzeButton;
    QPushButton* m_exportButton;
    
    // Optimization Tab
    QWidget* m_optimizationTab;
    QListWidget* m_suggestionsList;
    QPushButton* m_optimizeButton;
    QPushButton* m_rebalanceButton;
    
    QNetworkAccessManager* m_networkManager;
};

/**
 * 10. NFT Floor Price Alerts - Premium Feature
 */
class NFTFloorPriceAlerts : public QWidget
{
    Q_OBJECT

public:
    explicit NFTFloorPriceAlerts(PremiumSubscriptionManager* premiumManager, QWidget* parent = nullptr);

private Q_SLOTS:
    void onAddAlertClicked();
    void onRemoveAlertClicked();
    void onEditAlertClicked();
    void checkPriceAlerts();

private:
    void setupUI();
    void loadAlerts();
    void saveAlerts();
    void sendNotification(const QString& collection, double currentPrice, double alertPrice);

    struct PriceAlert {
        QString collectionName;
        QString contractAddress;
        double alertPrice;
        QString condition; // "above", "below"
        bool isActive;
        QDateTime createdAt;
        QString notificationMethod; // "popup", "email", "both"
    };
    
    PremiumSubscriptionManager* m_premiumManager;
    
    QVBoxLayout* m_mainLayout;
    QTableWidget* m_alertsTable;
    QPushButton* m_addAlertButton;
    QPushButton* m_removeAlertButton;
    QPushButton* m_editAlertButton;
    
    QList<PriceAlert> m_alerts;
    QTimer* m_checkTimer;
    QNetworkAccessManager* m_networkManager;
};

/**
 * 11. Usage Dashboard - Premium Feature
 */
class UsageDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit UsageDashboard(PremiumSubscriptionManager* premiumManager, WalletModel* walletModel, QWidget* parent = nullptr);

public Q_SLOTS:
    void refreshDashboard();

private Q_SLOTS:
    void onExportDataClicked();
    void onFilterChanged();

private:
    void setupUI();
    void loadUsageStatistics();
    void loadTransactionHistory();
    void generateAnalytics();

    PremiumSubscriptionManager* m_premiumManager;
    WalletModel* m_walletModel;
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Statistics Tab
    QWidget* m_statsTab;
    QLabel* m_totalTransactionsLabel;
    QLabel* m_totalVolumeLabel;
    QLabel* m_averageFeesLabel;
    QChartView* m_usageChart;
    
    // History Tab
    QWidget* m_historyTab;
    QTableWidget* m_historyTable;
    QComboBox* m_filterCombo;
    QPushButton* m_exportButton;
    
    // Analytics Tab
    QWidget* m_analyticsTab;
    QChartView* m_analyticsChart;
    QListWidget* m_insightsList;
};

/**
 * 12. Email Confirmation Toggle - Premium Feature
 */
class EmailConfirmationSettings : public QWidget
{
    Q_OBJECT

public:
    explicit EmailConfirmationSettings(PremiumSubscriptionManager* premiumManager, QWidget* parent = nullptr);

private Q_SLOTS:
    void onSettingsChanged();
    void onTestEmailClicked();
    void onSaveSettingsClicked();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void sendTestEmail();

    PremiumSubscriptionManager* m_premiumManager;
    
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_emailGroup;
    QGroupBox* m_notificationGroup;
    
    QLineEdit* m_emailInput;
    QCheckBox* m_enableEmailCheck;
    QCheckBox* m_transactionEmailCheck;
    QCheckBox* m_securityEmailCheck;
    QCheckBox* m_marketingEmailCheck;
    
    QPushButton* m_testEmailButton;
    QPushButton* m_saveButton;
    QLabel* m_statusLabel;
};

#endif // SHAHCOIN_QT_PREMIUMFEATURES_H
