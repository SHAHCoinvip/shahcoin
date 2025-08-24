// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#ifndef SMARTUXMANAGER_H
#define SMARTUXMANAGER_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QProcess>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QBuffer>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

class WalletModel;
class QWidget;
class QLabel;
class QPushButton;
class QProgressBar;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QSlider;
class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTabWidget;
class QTableWidget;
class QListWidget;
class QMenu;
class QAction;
class QDialog;
class QMessageBox;
class QToolTip;
class QStatusBar;

// Risk levels for alerts
enum class RiskLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

// Alert types
enum class AlertType {
    INVALID_TOKEN,
    SCAM_CONTRACT,
    RISKY_TRANSACTION,
    HIGH_SLIPPAGE,
    HIGH_GAS,
    SUSPICIOUS_ADDRESS,
    LARGE_AMOUNT,
    FREQUENT_TRANSACTIONS,
    UNKNOWN_CONTRACT,
    LOW_BALANCE,
    NETWORK_ISSUE,
    SYNC_PROBLEM
};

// Voice command types
enum class VoiceCommandType {
    SEND_MONEY,
    CHECK_BALANCE,
    VIEW_TRANSACTIONS,
    CREATE_TOKEN,
    MINT_NFT,
    STAKE_COINS,
    OPEN_MARKETPLACE,
    SHOW_PORTFOLIO,
    SETTINGS,
    HELP,
    UNKNOWN
};

// Alert structure
struct SmartAlert {
    AlertType type;
    RiskLevel level;
    QString title;
    QString message;
    QString details;
    QString recommendation;
    QString action;
    QDateTime timestamp;
    bool acknowledged;
    bool dismissed;
    QString transactionId;
    QString address;
    double amount;
    QString currency;
};

// Voice command structure
struct VoiceCommand {
    VoiceCommandType type;
    QString command;
    QString parameters;
    double confidence;
    QDateTime timestamp;
    bool executed;
    QString result;
};

// Risk assessment structure
struct RiskAssessment {
    QString transactionId;
    RiskLevel overallRisk;
    QList<SmartAlert> alerts;
    double riskScore;
    QString summary;
    QDateTime timestamp;
    bool recommended;
};

// Smart UX Manager - Main class for intelligent wallet features
class SmartUXManager : public QObject
{
    Q_OBJECT

public:
    explicit SmartUXManager(WalletModel* walletModel, QObject* parent = nullptr);
    ~SmartUXManager();

    // Core functionality
    void initialize();
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;

    // Risk assessment
    RiskAssessment assessTransactionRisk(const QString& txHex, const QString& recipient, double amount);
    RiskLevel assessAddressRisk(const QString& address);
    RiskLevel assessContractRisk(const QString& contractAddress);
    double calculateRiskScore(const QList<SmartAlert>& alerts);

    // Alert management
    void addAlert(const SmartAlert& alert);
    void acknowledgeAlert(const QString& alertId);
    void dismissAlert(const QString& alertId);
    void clearAlerts();
    QList<SmartAlert> getActiveAlerts() const;
    QList<SmartAlert> getAlertsByType(AlertType type) const;
    QList<SmartAlert> getAlertsByRiskLevel(RiskLevel level) const;

    // Voice commands
    void startVoiceRecognition();
    void stopVoiceRecognition();
    bool isVoiceRecognitionActive() const;
    void processVoiceCommand(const QString& audioData);
    VoiceCommand parseVoiceCommand(const QString& text);
    bool executeVoiceCommand(const VoiceCommand& command);
    QList<VoiceCommand> getVoiceCommandHistory() const;

    // Settings
    void setVoiceCommandsEnabled(bool enabled);
    void setRiskAlertsEnabled(bool enabled);
    void setAutoRiskAssessment(bool enabled);
    void setAlertSoundEnabled(bool enabled);
    void setVoiceConfirmationEnabled(bool enabled);
    void setRiskThreshold(RiskLevel threshold);
    void setVoiceLanguage(const QString& language);
    void setVoiceSensitivity(double sensitivity);

    bool isVoiceCommandsEnabled() const;
    bool isRiskAlertsEnabled() const;
    bool isAutoRiskAssessment() const;
    bool isAlertSoundEnabled() const;
    bool isVoiceConfirmationEnabled() const;
    RiskLevel getRiskThreshold() const;
    QString getVoiceLanguage() const;
    double getVoiceSensitivity() const;

    // UI integration
    void showRiskDialog(const RiskAssessment& assessment);
    void showAlertDialog(const SmartAlert& alert);
    void showVoiceCommandDialog();
    void updateStatusBar(QStatusBar* statusBar);
    void showToolTip(const QString& message, const QPoint& pos);

    // Network monitoring
    void checkNetworkHealth();
    void monitorMempool();
    void checkSyncStatus();
    void validateTransaction(const QString& txHex);

    // Contract analysis
    void analyzeContract(const QString& contractAddress);
    void validateTokenContract(const QString& tokenAddress);
    void checkContractVerification(const QString& contractAddress);

    // Address analysis
    void analyzeAddress(const QString& address);
    void checkAddressReputation(const QString& address);
    void validateAddressFormat(const QString& address);

    // Transaction analysis
    void analyzeTransaction(const QString& txHex);
    void detectAnomalies(const QString& txHex);
    void checkTransactionHistory(const QString& address);

signals:
    void alertRaised(const SmartAlert& alert);
    void alertAcknowledged(const QString& alertId);
    void alertDismissed(const QString& alertId);
    void riskAssessmentCompleted(const RiskAssessment& assessment);
    void voiceCommandReceived(const VoiceCommand& command);
    void voiceCommandExecuted(const VoiceCommand& command);
    void voiceCommandFailed(const VoiceCommand& command, const QString& error);
    void networkHealthChanged(bool healthy);
    void syncStatusChanged(int percentage);
    void mempoolStatusChanged(int pendingTransactions);
    void contractAnalysisCompleted(const QString& contractAddress, bool safe);
    void addressAnalysisCompleted(const QString& address, bool safe);
    void transactionAnalysisCompleted(const QString& txHex, bool safe);

private slots:
    void onNetworkReplyFinished();
    void onAudioRecorderStateChanged();
    void onAudioProbeBufferReady();
    void onVoiceProcessingTimeout();
    void onRiskAssessmentTimeout();
    void onNetworkHealthCheck();
    void onMempoolCheck();
    void onSyncCheck();
    void onContractAnalysisTimeout();
    void onAddressAnalysisTimeout();
    void onTransactionAnalysisTimeout();

private:
    // Core members
    WalletModel* m_walletModel;
    QSettings m_settings;
    bool m_isMonitoring;
    bool m_voiceCommandsEnabled;
    bool m_riskAlertsEnabled;
    bool m_autoRiskAssessment;
    bool m_alertSoundEnabled;
    bool m_voiceConfirmationEnabled;
    RiskLevel m_riskThreshold;
    QString m_voiceLanguage;
    double m_voiceSensitivity;

    // Network
    QNetworkAccessManager* m_networkManager;
    QTimer* m_networkHealthTimer;
    QTimer* m_mempoolTimer;
    QTimer* m_syncTimer;
    bool m_networkHealthy;
    int m_pendingTransactions;
    int m_syncPercentage;

    // Voice recognition
    QAudioRecorder* m_audioRecorder;
    QAudioProbe* m_audioProbe;
    QTimer* m_voiceProcessingTimer;
    QThread* m_voiceProcessingThread;
    QMutex m_voiceMutex;
    QQueue<QByteArray> m_audioBuffer;
    bool m_voiceRecognitionActive;
    QList<VoiceCommand> m_voiceCommandHistory;

    // Risk assessment
    QTimer* m_riskAssessmentTimer;
    QMutex m_riskMutex;
    QList<SmartAlert> m_activeAlerts;
    QList<RiskAssessment> m_riskAssessments;
    QMap<QString, RiskLevel> m_addressRiskCache;
    QMap<QString, RiskLevel> m_contractRiskCache;
    QSet<QString> m_knownSafeAddresses;
    QSet<QString> m_knownScamAddresses;
    QSet<QString> m_knownSafeContracts;
    QSet<QString> m_knownScamContracts;

    // Analysis timers
    QTimer* m_contractAnalysisTimer;
    QTimer* m_addressAnalysisTimer;
    QTimer* m_transactionAnalysisTimer;

    // UI components
    QDialog* m_riskDialog;
    QDialog* m_alertDialog;
    QDialog* m_voiceDialog;
    QStatusBar* m_statusBar;

    // Helper methods
    void setupTimers();
    void setupVoiceRecognition();
    void setupNetworkMonitoring();
    void loadSettings();
    void saveSettings();
    void loadKnownAddresses();
    void loadKnownContracts();
    void updateAlertSound();
    void processAudioData(const QByteArray& audioData);
    QString convertSpeechToText(const QByteArray& audioData);
    VoiceCommandType classifyVoiceCommand(const QString& text);
    QString extractParameters(const QString& text, VoiceCommandType type);
    bool validateVoiceCommand(const VoiceCommand& command);
    void logVoiceCommand(const VoiceCommand& command);
    void logAlert(const SmartAlert& alert);
    void logRiskAssessment(const RiskAssessment& assessment);
    QString generateAlertId() const;
    QString generateRiskAssessmentId() const;
    QString generateVoiceCommandId() const;
    void cleanupOldData();
    void exportData();
    void importData();

    // Risk assessment helpers
    bool isHighValueTransaction(double amount) const;
    bool isFrequentTransaction(const QString& address) const;
    bool isSuspiciousPattern(const QString& address) const;
    bool isKnownScamAddress(const QString& address) const;
    bool isKnownSafeAddress(const QString& address) const;
    bool isKnownScamContract(const QString& contract) const;
    bool isKnownSafeContract(const QString& contract) const;
    double calculateSlippageRisk(double slippage) const;
    double calculateGasRisk(double gasPrice) const;
    double calculateAmountRisk(double amount) const;
    double calculateAddressRisk(const QString& address) const;
    double calculateContractRisk(const QString& contract) const;

    // Network helpers
    void fetchNetworkData();
    void fetchMempoolData();
    void fetchSyncData();
    void fetchContractData(const QString& contractAddress);
    void fetchAddressData(const QString& address);
    void fetchTransactionData(const QString& txHex);
    void handleNetworkError(const QString& error);
    void handleApiResponse(const QJsonDocument& response);

    // UI helpers
    void createRiskDialog();
    void createAlertDialog();
    void createVoiceDialog();
    void updateRiskDialog(const RiskAssessment& assessment);
    void updateAlertDialog(const SmartAlert& alert);
    void updateVoiceDialog();
    void showNotification(const QString& title, const QString& message);
    void playAlertSound();
    void showStatusMessage(const QString& message, int timeout = 5000);
};

// Smart Alert Widget - Displays individual alerts
class SmartAlertWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SmartAlertWidget(const SmartAlert& alert, QWidget* parent = nullptr);

signals:
    void acknowledged(const QString& alertId);
    void dismissed(const QString& alertId);
    void actionRequested(const QString& alertId, const QString& action);

private slots:
    void onAcknowledgeClicked();
    void onDismissClicked();
    void onActionClicked();

private:
    SmartAlert m_alert;
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_messageLabel;
    QLabel* m_detailsLabel;
    QLabel* m_recommendationLabel;
    QPushButton* m_acknowledgeButton;
    QPushButton* m_dismissButton;
    QPushButton* m_actionButton;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;

    void setupUI();
    void updateStyle();
    QString getRiskLevelColor(RiskLevel level) const;
    QString getAlertTypeIcon(AlertType type) const;
};

// Risk Assessment Dialog - Shows detailed risk analysis
class RiskAssessmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RiskAssessmentDialog(const RiskAssessment& assessment, QWidget* parent = nullptr);

signals:
    void proceedRequested(const QString& transactionId);
    void cancelRequested(const QString& transactionId);
    void modifyRequested(const QString& transactionId);

private slots:
    void onProceedClicked();
    void onCancelClicked();
    void onModifyClicked();
    void onDetailsClicked();

private:
    RiskAssessment m_assessment;
    QLabel* m_riskLevelLabel;
    QLabel* m_riskScoreLabel;
    QLabel* m_summaryLabel;
    QTextEdit* m_detailsText;
    QTableWidget* m_alertsTable;
    QPushButton* m_proceedButton;
    QPushButton* m_cancelButton;
    QPushButton* m_modifyButton;
    QPushButton* m_detailsButton;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;

    void setupUI();
    void populateAlertsTable();
    void updateRiskDisplay();
    QString getRiskLevelText(RiskLevel level) const;
    QString getRiskLevelColor(RiskLevel level) const;
    QString getRiskLevelIcon(RiskLevel level) const;
};

// Voice Command Dialog - Interface for voice commands
class VoiceCommandDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VoiceCommandDialog(QWidget* parent = nullptr);

signals:
    void commandReceived(const VoiceCommand& command);

private slots:
    void onStartListeningClicked();
    void onStopListeningClicked();
    void onClearHistoryClicked();
    void onSettingsClicked();
    void onVoiceCommandReceived(const VoiceCommand& command);
    void onVoiceCommandExecuted(const VoiceCommand& command);
    void onVoiceCommandFailed(const VoiceCommand& command, const QString& error);

private:
    QLabel* m_statusLabel;
    QLabel* m_confidenceLabel;
    QTextEdit* m_commandText;
    QListWidget* m_historyList;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_clearButton;
    QPushButton* m_settingsButton;
    QProgressBar* m_confidenceBar;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
    QHBoxLayout* m_controlLayout;

    bool m_isListening;
    QList<VoiceCommand> m_commandHistory;

    void setupUI();
    void updateListeningState();
    void updateConfidenceDisplay(double confidence);
    void addCommandToHistory(const VoiceCommand& command);
    void clearHistory();
    void showSettings();
    QString getCommandTypeText(VoiceCommandType type) const;
    QString getCommandTypeIcon(VoiceCommandType type) const;
};

#endif // SMARTUXMANAGER_H
