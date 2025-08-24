#ifndef SHAHCOIN_QT_AIASSISTANT_H
#define SHAHCOIN_QT_AIASSISTANT_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QTimer>
#include <QMutex>

class WalletModel;
class QWidget;

/**
 * AI Assistant for Shahcoin Wallet
 * 
 * Provides intelligent help and guidance to users, including:
 * - Transaction explanations
 * - Fee recommendations
 * - Security warnings
 * - Feature explanations
 * - Scam detection
 */
class AIAssistant : public QObject
{
    Q_OBJECT

public:
    enum AssistantMode {
        Local,      // Local AI processing
        Cloud,      // Cloud AI API
        Hybrid      // Hybrid local/cloud
    };

    enum MessageType {
        UserMessage,
        AssistantMessage,
        SystemMessage,
        WarningMessage,
        ErrorMessage
    };

    enum FeatureType {
        Transaction,
        Staking,
        Mining,
        TokenCreation,
        NFTCreation,
        DEX,
        Security,
        General
    };

    struct AssistantMessage {
        MessageType type;
        QString content;
        QString timestamp;
        QString context;
        bool isProcessed;
        QJsonObject metadata;
    };

    struct SecurityWarning {
        QString title;
        QString description;
        QString severity; // "low", "medium", "high", "critical"
        QString recommendation;
        QString transactionId;
        QString address;
    };

    struct FeatureExplanation {
        QString feature;
        QString description;
        QStringList steps;
        QStringList tips;
        QStringList warnings;
    };

    struct TransactionAnalysis {
        QString transactionId;
        QString type;
        QString status;
        QString risk;
        QString recommendation;
        double fee;
        int confirmations;
        QStringList warnings;
    };

    explicit AIAssistant(QObject* parent = nullptr);
    ~AIAssistant();

    // Configuration
    void setMode(AssistantMode mode);
    void setApiKey(const QString& apiKey);
    void setApiEndpoint(const QString& endpoint);
    void setLocalModelPath(const QString& path);
    void setMaxTokens(int maxTokens);
    void setTemperature(double temperature);

    AssistantMode getMode() const;
    QString getApiKey() const;
    QString getApiEndpoint() const;
    QString getLocalModelPath() const;
    int getMaxTokens() const;
    double getTemperature() const;

    // Core functionality
    void askQuestion(const QString& question, const QString& context = "");
    void analyzeTransaction(const QString& transactionId);
    void explainFeature(FeatureType feature);
    void checkSecurity(const QString& address, const QString& transactionId = "");
    void getFeeRecommendation(double amount, const QString& priority = "normal");
    void detectScam(const QString& message, const QString& address = "");

    // Message history
    QList<AssistantMessage> getMessageHistory() const;
    void clearMessageHistory();
    void saveMessageHistory();
    void loadMessageHistory();

    // Settings
    void setEnabled(bool enabled);
    void setAutoAnalyze(bool autoAnalyze);
    void setSecurityChecks(bool securityChecks);
    void setScamDetection(bool scamDetection);
    void setFeeRecommendations(bool feeRecommendations);

    bool isEnabled() const;
    bool isAutoAnalyze() const;
    bool isSecurityChecks() const;
    bool isScamDetection() const;
    bool isFeeRecommendations() const;

    // UI integration
    void showAssistantDialog(QWidget* parent = nullptr);
    void showQuickHelp(FeatureType feature, QWidget* parent = nullptr);
    void showSecurityWarning(const SecurityWarning& warning, QWidget* parent = nullptr);

public Q_SLOTS:
    void processUserInput(const QString& input);
    void autoAnalyzeTransaction(const QString& transactionId);
    void checkForSecurityIssues();
    void updateKnowledgeBase();

private Q_SLOTS:
    void onNetworkReply(QNetworkReply* reply);
    void onLocalProcessingComplete();
    void onSecurityCheckComplete();

private:
    // Core processing
    void processQuestion(const QString& question, const QString& context);
    void processLocalAI(const QString& input);
    void processCloudAI(const QString& input);
    void processHybridAI(const QString& input);

    // Analysis functions
    void analyzeTransactionData(const QString& transactionId);
    void analyzeSecurityRisks(const QString& address, const QString& transactionId);
    void analyzeFeeStructure(double amount, const QString& priority);
    void analyzeScamPatterns(const QString& message, const QString& address);

    // Knowledge base
    void loadKnowledgeBase();
    void saveKnowledgeBase();
    QString getFeatureExplanation(FeatureType feature);
    QString getSecurityGuidelines();
    QString getFeeGuidelines();
    QString getScamPatterns();

    // Local AI processing
    void initializeLocalAI();
    void processWithLocalModel(const QString& input);
    QString generateLocalResponse(const QString& input);

    // Cloud AI processing
    void sendToCloudAPI(const QString& input);
    void processCloudResponse(const QJsonObject& response);
    QString formatCloudRequest(const QString& input);

    // Security functions
    void checkAddressReputation(const QString& address);
    void checkTransactionPatterns(const QString& transactionId);
    void validateTransaction(const QString& transactionId);
    void detectSuspiciousActivity(const QString& address);

    // Utility functions
    QString formatMessage(const QString& content, MessageType type);
    QString getCurrentContext();
    void logMessage(const QString& message, MessageType type);
    void saveSettings();
    void loadSettings();

    // Data members
    AssistantMode m_mode;
    QString m_apiKey;
    QString m_apiEndpoint;
    QString m_localModelPath;
    int m_maxTokens;
    double m_temperature;
    
    // State
    bool m_enabled;
    bool m_autoAnalyze;
    bool m_securityChecks;
    bool m_scamDetection;
    bool m_feeRecommendations;
    
    // Network
    QNetworkAccessManager* m_networkManager;
    QMap<QNetworkReply*, QString> m_pendingRequests;
    
    // Data storage
    QList<AssistantMessage> m_messageHistory;
    QMap<FeatureType, FeatureExplanation> m_featureExplanations;
    QList<SecurityWarning> m_securityWarnings;
    QMap<QString, TransactionAnalysis> m_transactionAnalyses;
    
    // Knowledge base
    QJsonObject m_knowledgeBase;
    QList<QString> m_scamPatterns;
    QMap<QString, QString> m_addressReputations;
    
    // Threading
    QMutex m_dataMutex;
    QTimer* m_autoAnalyzeTimer;
    
    // Local AI state
    bool m_localAIInitialized;
    QJsonObject m_localModelConfig;
};

/**
 * AI Assistant Dialog
 * 
 * Main UI for interacting with the AI Assistant
 */
class AIAssistantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AIAssistantDialog(AIAssistant* assistant, QWidget* parent = nullptr);

private Q_SLOTS:
    void onSendMessage();
    void onClearHistory();
    void onSettingsClicked();
    void onQuickHelpClicked();
    void onSecurityCheckClicked();

private:
    void setupUI();
    void setupConnections();
    void addMessage(const QString& content, AIAssistant::MessageType type);
    void updateMessageList();
    void showQuickHelpMenu();

    AIAssistant* m_assistant;
    QVBoxLayout* m_layout;
    QListWidget* m_messageList;
    QLineEdit* m_inputField;
    QPushButton* m_sendButton;
    QPushButton* m_clearButton;
    QPushButton* m_settingsButton;
    QPushButton* m_helpButton;
    QPushButton* m_securityButton;
    QLabel* m_statusLabel;
};

/**
 * Quick Help Dialog
 * 
 * Shows quick help for specific features
 */
class QuickHelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuickHelpDialog(AIAssistant::FeatureType feature, QWidget* parent = nullptr);

private:
    void setupUI();
    void loadFeatureHelp();

    AIAssistant::FeatureType m_feature;
    QVBoxLayout* m_layout;
    QTextBrowser* m_helpText;
    QPushButton* m_closeButton;
    QPushButton* m_askMoreButton;
};

/**
 * Security Warning Dialog
 * 
 * Shows security warnings and recommendations
 */
class SecurityWarningDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SecurityWarningDialog(const AIAssistant::SecurityWarning& warning, QWidget* parent = nullptr);

private Q_SLOTS:
    void onIgnoreClicked();
    void onBlockClicked();
    void onReportClicked();

private:
    void setupUI();
    void setSeverityStyle(const QString& severity);

    AIAssistant::SecurityWarning m_warning;
    QVBoxLayout* m_layout;
    QLabel* m_titleLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_recommendationLabel;
    QPushButton* m_ignoreButton;
    QPushButton* m_blockButton;
    QPushButton* m_reportButton;
    QPushButton* m_closeButton;
};

#endif // SHAHCOIN_QT_AIASSISTANT_H
