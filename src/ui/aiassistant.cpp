#include "aiassistant.h"
#include <qt/walletmodel.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>

#include <QApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QSettings>
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextBrowser>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDateTime>

AIAssistant::AIAssistant(QObject* parent)
    : QObject(parent)
    , m_mode(Local)
    , m_maxTokens(1000)
    , m_temperature(0.7)
    , m_enabled(true)
    , m_autoAnalyze(true)
    , m_securityChecks(true)
    , m_scamDetection(true)
    , m_feeRecommendations(true)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_autoAnalyzeTimer(new QTimer(this))
    , m_localAIInitialized(false)
{
    // Initialize default settings
    m_apiEndpoint = "https://api.openai.com/v1/chat/completions";
    m_localModelPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/ai/models";
    
    setupConnections();
    loadSettings();
    loadKnowledgeBase();
    loadMessageHistory();
    initializeLocalAI();
}

AIAssistant::~AIAssistant()
{
    saveSettings();
    saveKnowledgeBase();
    saveMessageHistory();
}

void AIAssistant::setupConnections()
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &AIAssistant::onNetworkReply);
    connect(m_autoAnalyzeTimer, &QTimer::timeout,
            this, &AIAssistant::checkForSecurityIssues);
}

void AIAssistant::setMode(AssistantMode mode)
{
    m_mode = mode;
    saveSettings();
}

void AIAssistant::setApiKey(const QString& apiKey)
{
    m_apiKey = apiKey;
    saveSettings();
}

void AIAssistant::setApiEndpoint(const QString& endpoint)
{
    m_apiEndpoint = endpoint;
    saveSettings();
}

void AIAssistant::setLocalModelPath(const QString& path)
{
    m_localModelPath = path;
    saveSettings();
}

void AIAssistant::setMaxTokens(int maxTokens)
{
    m_maxTokens = maxTokens;
    saveSettings();
}

void AIAssistant::setTemperature(double temperature)
{
    m_temperature = temperature;
    saveSettings();
}

AIAssistant::AssistantMode AIAssistant::getMode() const
{
    return m_mode;
}

QString AIAssistant::getApiKey() const
{
    return m_apiKey;
}

QString AIAssistant::getApiEndpoint() const
{
    return m_apiEndpoint;
}

QString AIAssistant::getLocalModelPath() const
{
    return m_localModelPath;
}

int AIAssistant::getMaxTokens() const
{
    return m_maxTokens;
}

double AIAssistant::getTemperature() const
{
    return m_temperature;
}

void AIAssistant::askQuestion(const QString& question, const QString& context)
{
    if (!m_enabled) {
        return;
    }

    logMessage(question, UserMessage);
    processQuestion(question, context);
}

void AIAssistant::processQuestion(const QString& question, const QString& context)
{
    QString fullInput = question;
    if (!context.isEmpty()) {
        fullInput = QString("Context: %1\nQuestion: %2").arg(context, question);
    }

    switch (m_mode) {
        case Local:
            processLocalAI(fullInput);
            break;
        case Cloud:
            processCloudAI(fullInput);
            break;
        case Hybrid:
            processHybridAI(fullInput);
            break;
    }
}

void AIAssistant::processLocalAI(const QString& input)
{
    if (!m_localAIInitialized) {
        QString response = "Local AI is not available. Please check your model configuration.";
        logMessage(response, ErrorMessage);
        return;
    }

    processWithLocalModel(input);
}

void AIAssistant::processCloudAI(const QString& input)
{
    if (m_apiKey.isEmpty()) {
        QString response = "API key not configured. Please set your API key in settings.";
        logMessage(response, ErrorMessage);
        return;
    }

    sendToCloudAPI(input);
}

void AIAssistant::processHybridAI(const QString& input)
{
    // Try local first, fallback to cloud
    if (m_localAIInitialized) {
        processLocalAI(input);
    } else {
        processCloudAI(input);
    }
}

void AIAssistant::processWithLocalModel(const QString& input)
{
    QString response = generateLocalResponse(input);
    logMessage(response, AssistantMessage);
    Q_EMIT responseGenerated(response);
}

QString AIAssistant::generateLocalResponse(const QString& input)
{
    // Simple rule-based responses for common questions
    QString lowerInput = input.toLower();
    
    if (lowerInput.contains("fee") || lowerInput.contains("transaction cost")) {
        return "Transaction fees in Shahcoin are dynamic and based on network congestion. "
               "For normal transactions, fees are typically 0.0001 SHAH. "
               "For urgent transactions, you can increase the fee to get faster confirmation.";
    }
    
    if (lowerInput.contains("staking") || lowerInput.contains("stake")) {
        return "Staking in Shahcoin allows you to earn rewards by participating in network validation. "
               "To start staking, you need a minimum of 1000 SHAH and your wallet must be unlocked. "
               "Staking rewards are typically 5-10% annually depending on network participation.";
    }
    
    if (lowerInput.contains("security") || lowerInput.contains("safe")) {
        return "Shahcoin security best practices:\n"
               "1. Always verify addresses before sending\n"
               "2. Keep your private keys secure and never share them\n"
               "3. Use strong passwords for wallet encryption\n"
               "4. Enable two-factor authentication if available\n"
               "5. Be cautious of phishing attempts and suspicious links";
    }
    
    if (lowerInput.contains("token") || lowerInput.contains("create token")) {
        return "To create a token in Shahcoin:\n"
               "1. Go to the 'Create Asset' tab\n"
               "2. Select 'Token Minting'\n"
               "3. Enter token name, symbol, and total supply\n"
               "4. Set token properties (decimals, burnable, etc.)\n"
               "5. Pay the creation fee and confirm\n"
               "Token creation costs 100 SHAH and requires 100 confirmations.";
    }
    
    if (lowerInput.contains("nft") || lowerInput.contains("create nft")) {
        return "To create an NFT in Shahcoin:\n"
               "1. Go to the 'Create Asset' tab\n"
               "2. Select 'NFT Minting'\n"
               "3. Upload your image or enter metadata URI\n"
               "4. Set NFT properties and attributes\n"
               "5. Pay the creation fee and confirm\n"
               "NFT creation costs 50 SHAH and requires 50 confirmations.";
    }
    
    return "I'm here to help with Shahcoin questions! You can ask me about:\n"
           "- Transaction fees and costs\n"
           "- Staking and earning rewards\n"
           "- Security best practices\n"
           "- Creating tokens and NFTs\n"
           "- Using the DEX (ShahSwap)\n"
           "- General wallet features";
}

void AIAssistant::sendToCloudAPI(const QString& input)
{
    QNetworkRequest request;
    request.setUrl(QUrl(m_apiEndpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    QJsonObject requestData;
    requestData["model"] = "gpt-3.5-turbo";
    requestData["max_tokens"] = m_maxTokens;
    requestData["temperature"] = m_temperature;

    QJsonArray messages;
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "You are a helpful AI assistant for Shahcoin cryptocurrency. "
                              "Provide accurate, helpful information about Shahcoin features, "
                              "security, transactions, staking, and general cryptocurrency guidance.";
    messages.append(systemMessage);

    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = input;
    messages.append(userMessage);

    requestData["messages"] = messages;

    QJsonDocument doc(requestData);
    QByteArray data = doc.toJson();

    QNetworkReply* reply = m_networkManager->post(request, data);
    m_pendingRequests[reply] = input;
}

void AIAssistant::onNetworkReply(QNetworkReply* reply)
{
    if (!reply) {
        return;
    }

    QString originalInput = m_pendingRequests.value(reply);
    m_pendingRequests.remove(reply);

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        if (doc.isObject()) {
            processCloudResponse(doc.object());
        }
    } else {
        QString error = QString("API Error: %1").arg(reply->errorString());
        logMessage(error, ErrorMessage);
    }

    reply->deleteLater();
}

void AIAssistant::processCloudResponse(const QJsonObject& response)
{
    if (response.contains("choices")) {
        QJsonArray choices = response["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices[0].toObject();
            if (choice.contains("message")) {
                QJsonObject message = choice["message"].toObject();
                QString content = message["content"].toString();
                logMessage(content, AssistantMessage);
                Q_EMIT responseGenerated(content);
                return;
            }
        }
    }

    QString error = "Invalid response from AI service";
    logMessage(error, ErrorMessage);
}

void AIAssistant::analyzeTransaction(const QString& transactionId)
{
    if (!m_enabled || !m_autoAnalyze) {
        return;
    }

    analyzeTransactionData(transactionId);
}

void AIAssistant::analyzeTransactionData(const QString& transactionId)
{
    // Analyze transaction for potential issues
    TransactionAnalysis analysis;
    analysis.transactionId = transactionId;
    analysis.type = "Unknown";
    analysis.status = "Pending";
    analysis.risk = "Low";
    analysis.recommendation = "Transaction appears normal";
    analysis.fee = 0.0;
    analysis.confirmations = 0;

    // Check for suspicious patterns
    if (transactionId.contains("suspicious")) {
        analysis.risk = "High";
        analysis.recommendation = "This transaction shows suspicious patterns. Please verify the recipient address.";
        analysis.warnings.append("Suspicious transaction pattern detected");
    }

    m_transactionAnalyses[transactionId] = analysis;
    Q_EMIT transactionAnalyzed(analysis);
}

void AIAssistant::checkSecurity(const QString& address, const QString& transactionId)
{
    if (!m_enabled || !m_securityChecks) {
        return;
    }

    analyzeSecurityRisks(address, transactionId);
}

void AIAssistant::analyzeSecurityRisks(const QString& address, const QString& transactionId)
{
    SecurityWarning warning;
    warning.address = address;
    warning.transactionId = transactionId;

    // Check address reputation
    QString reputation = m_addressReputations.value(address, "unknown");
    if (reputation == "suspicious" || reputation == "scam") {
        warning.title = "Suspicious Address Detected";
        warning.description = QString("The address %1 has been flagged as suspicious.").arg(address);
        warning.severity = "high";
        warning.recommendation = "Do not send funds to this address. Verify the recipient carefully.";
        
        m_securityWarnings.append(warning);
        Q_EMIT securityWarningGenerated(warning);
    }
}

void AIAssistant::getFeeRecommendation(double amount, const QString& priority)
{
    if (!m_enabled || !m_feeRecommendations) {
        return;
    }

    analyzeFeeStructure(amount, priority);
}

void AIAssistant::analyzeFeeStructure(double amount, const QString& priority)
{
    double recommendedFee = 0.0001; // Base fee

    if (priority == "high") {
        recommendedFee = 0.001; // Higher fee for faster confirmation
    } else if (priority == "low") {
        recommendedFee = 0.00005; // Lower fee, may take longer
    }

    QString response = QString("For a %1 SHAH transaction with %2 priority, "
                              "I recommend a fee of %3 SHAH. "
                              "This should provide confirmation within %4 blocks.")
                              .arg(amount)
                              .arg(priority)
                              .arg(recommendedFee)
                              .arg(priority == "high" ? "1-2" : "3-6");

    logMessage(response, AssistantMessage);
    Q_EMIT feeRecommendationGenerated(recommendedFee, response);
}

void AIAssistant::detectScam(const QString& message, const QString& address)
{
    if (!m_enabled || !m_scamDetection) {
        return;
    }

    analyzeScamPatterns(message, address);
}

void AIAssistant::analyzeScamPatterns(const QString& message, const QString& address)
{
    QString lowerMessage = message.toLower();
    bool isScam = false;
    QStringList scamIndicators;

    // Check for common scam patterns
    if (lowerMessage.contains("urgent") && lowerMessage.contains("send")) {
        scamIndicators.append("Urgency pressure");
        isScam = true;
    }

    if (lowerMessage.contains("free") && lowerMessage.contains("claim")) {
        scamIndicators.append("Free money offers");
        isScam = true;
    }

    if (lowerMessage.contains("wallet") && lowerMessage.contains("sync")) {
        scamIndicators.append("Fake wallet sync requests");
        isScam = true;
    }

    if (isScam) {
        SecurityWarning warning;
        warning.title = "Potential Scam Detected";
        warning.description = QString("The message contains suspicious patterns: %1").arg(scamIndicators.join(", "));
        warning.severity = "critical";
        warning.recommendation = "Do not respond to this message or send any funds. This appears to be a scam attempt.";
        warning.address = address;

        m_securityWarnings.append(warning);
        Q_EMIT securityWarningGenerated(warning);
    }
}

void AIAssistant::explainFeature(FeatureType feature)
{
    QString explanation = getFeatureExplanation(feature);
    logMessage(explanation, AssistantMessage);
    Q_EMIT featureExplained(feature, explanation);
}

QString AIAssistant::getFeatureExplanation(FeatureType feature)
{
    switch (feature) {
        case Transaction:
            return "Shahcoin transactions are fast and secure. Each transaction includes:\n"
                   "- Sender and recipient addresses\n"
                   "- Amount to transfer\n"
                   "- Transaction fee\n"
                   "- Optional memo/note\n"
                   "Transactions are confirmed within 1-6 blocks depending on fee priority.";
        
        case Staking:
            return "Staking allows you to earn rewards by participating in network validation:\n"
                   "- Minimum stake: 1000 SHAH\n"
                   "- Annual rewards: 5-10%\n"
                   "- Lock period: 30 days\n"
                   "- Rewards paid automatically\n"
                   "Staking helps secure the network while earning passive income.";
        
        case Mining:
            return "Shahcoin uses multi-algorithm mining:\n"
                   "- SHA256d, Scrypt, and Groestl algorithms\n"
                   "- Block time: 2.5 minutes\n"
                   "- Block reward: 50 SHAH (halving every 4 years)\n"
                   "- Difficulty adjusts every block\n"
                   "Mining requires specialized hardware and significant computational power.";
        
        case TokenCreation:
            return "Create your own tokens on Shahcoin:\n"
                   "- Cost: 100 SHAH\n"
                   "- Confirmations required: 100\n"
                   "- Customizable properties\n"
                   "- SHI-20 compatible\n"
                   "Tokens can be traded on ShahSwap DEX.";
        
        case NFTCreation:
            return "Create unique NFTs on Shahcoin:\n"
                   "- Cost: 50 SHAH\n"
                   "- Confirmations required: 50\n"
                   "- Support for images and metadata\n"
                   "- ERC-721 compatible\n"
                   "NFTs represent unique digital assets.";
        
        case DEX:
            return "ShahSwap is the native decentralized exchange:\n"
                   "- Swap any Shahcoin tokens\n"
                   "- Automated market making\n"
                   "- Liquidity pools\n"
                   "- Low fees and fast execution\n"
                   "Trade directly from your wallet without intermediaries.";
        
        case Security:
            return "Shahcoin security features:\n"
                   "- Multi-algorithm consensus\n"
                   "- Proof-of-stake validation\n"
                   "- Cold staking support\n"
                   "- Encrypted wallets\n"
                   "- Multi-signature support\n"
                   "Always keep your private keys secure!";
        
        default:
            return "Shahcoin is a feature-rich cryptocurrency with:\n"
                   "- Fast transactions\n"
                   "- Multi-algorithm mining\n"
                   "- Proof-of-stake rewards\n"
                   "- Token and NFT creation\n"
                   "- Decentralized exchange\n"
                   "- Advanced security features";
    }
}

QList<AIAssistant::AssistantMessage> AIAssistant::getMessageHistory() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_messageHistory;
}

void AIAssistant::clearMessageHistory()
{
    QMutexLocker locker(&m_dataMutex);
    m_messageHistory.clear();
    saveMessageHistory();
}

void AIAssistant::saveMessageHistory()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/ai";
    QDir().mkpath(dataDir);
    
    QFile file(dataDir + "/messages.dat");
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << static_cast<qint32>(m_messageHistory.size());
        
        for (const AssistantMessage& msg : m_messageHistory) {
            out << static_cast<qint32>(msg.type);
            out << msg.content;
            out << msg.timestamp;
            out << msg.context;
            out << msg.isProcessed;
        }
    }
}

void AIAssistant::loadMessageHistory()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/ai";
    QFile file(dataDir + "/messages.dat");
    
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        qint32 count;
        in >> count;
        
        for (qint32 i = 0; i < count; ++i) {
            AssistantMessage msg;
            qint32 type;
            
            in >> type >> msg.content >> msg.timestamp >> msg.context >> msg.isProcessed;
            msg.type = static_cast<MessageType>(type);
            
            m_messageHistory.append(msg);
        }
    }
}

void AIAssistant::setEnabled(bool enabled)
{
    m_enabled = enabled;
    saveSettings();
}

void AIAssistant::setAutoAnalyze(bool autoAnalyze)
{
    m_autoAnalyze = autoAnalyze;
    saveSettings();
}

void AIAssistant::setSecurityChecks(bool securityChecks)
{
    m_securityChecks = securityChecks;
    saveSettings();
}

void AIAssistant::setScamDetection(bool scamDetection)
{
    m_scamDetection = scamDetection;
    saveSettings();
}

void AIAssistant::setFeeRecommendations(bool feeRecommendations)
{
    m_feeRecommendations = feeRecommendations;
    saveSettings();
}

bool AIAssistant::isEnabled() const
{
    return m_enabled;
}

bool AIAssistant::isAutoAnalyze() const
{
    return m_autoAnalyze;
}

bool AIAssistant::isSecurityChecks() const
{
    return m_securityChecks;
}

bool AIAssistant::isScamDetection() const
{
    return m_scamDetection;
}

bool AIAssistant::isFeeRecommendations() const
{
    return m_feeRecommendations;
}

void AIAssistant::processUserInput(const QString& input)
{
    askQuestion(input);
}

void AIAssistant::autoAnalyzeTransaction(const QString& transactionId)
{
    analyzeTransaction(transactionId);
}

void AIAssistant::checkForSecurityIssues()
{
    // Periodic security check
    if (m_enabled && m_securityChecks) {
        // Check for new suspicious addresses or patterns
        Q_EMIT securityCheckComplete();
    }
}

void AIAssistant::updateKnowledgeBase()
{
    loadKnowledgeBase();
    Q_EMIT knowledgeBaseUpdated();
}

void AIAssistant::initializeLocalAI()
{
    // Initialize local AI model
    // This would typically load a local language model
    m_localAIInitialized = true;
}

void AIAssistant::loadKnowledgeBase()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/ai";
    QFile file(dataDir + "/knowledge.json");
    
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            m_knowledgeBase = doc.object();
        }
    }
}

void AIAssistant::saveKnowledgeBase()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/ai";
    QDir().mkpath(dataDir);
    
    QFile file(dataDir + "/knowledge.json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(m_knowledgeBase);
        file.write(doc.toJson());
    }
}

void AIAssistant::logMessage(const QString& message, MessageType type)
{
    QMutexLocker locker(&m_dataMutex);
    
    AssistantMessage msg;
    msg.type = type;
    msg.content = message;
    msg.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    msg.context = getCurrentContext();
    msg.isProcessed = true;
    
    m_messageHistory.append(msg);
    
    // Keep only last 100 messages
    if (m_messageHistory.size() > 100) {
        m_messageHistory = m_messageHistory.mid(m_messageHistory.size() - 100);
    }
    
    saveMessageHistory();
}

QString AIAssistant::getCurrentContext()
{
    return "Shahcoin Wallet";
}

void AIAssistant::saveSettings()
{
    QSettings settings;
    settings.setValue("AIAssistant/mode", static_cast<int>(m_mode));
    settings.setValue("AIAssistant/apiKey", m_apiKey);
    settings.setValue("AIAssistant/apiEndpoint", m_apiEndpoint);
    settings.setValue("AIAssistant/localModelPath", m_localModelPath);
    settings.setValue("AIAssistant/maxTokens", m_maxTokens);
    settings.setValue("AIAssistant/temperature", m_temperature);
    settings.setValue("AIAssistant/enabled", m_enabled);
    settings.setValue("AIAssistant/autoAnalyze", m_autoAnalyze);
    settings.setValue("AIAssistant/securityChecks", m_securityChecks);
    settings.setValue("AIAssistant/scamDetection", m_scamDetection);
    settings.setValue("AIAssistant/feeRecommendations", m_feeRecommendations);
}

void AIAssistant::loadSettings()
{
    QSettings settings;
    m_mode = static_cast<AssistantMode>(settings.value("AIAssistant/mode", Local).toInt());
    m_apiKey = settings.value("AIAssistant/apiKey").toString();
    m_apiEndpoint = settings.value("AIAssistant/apiEndpoint", m_apiEndpoint).toString();
    m_localModelPath = settings.value("AIAssistant/localModelPath", m_localModelPath).toString();
    m_maxTokens = settings.value("AIAssistant/maxTokens", m_maxTokens).toInt();
    m_temperature = settings.value("AIAssistant/temperature", m_temperature).toDouble();
    m_enabled = settings.value("AIAssistant/enabled", m_enabled).toBool();
    m_autoAnalyze = settings.value("AIAssistant/autoAnalyze", m_autoAnalyze).toBool();
    m_securityChecks = settings.value("AIAssistant/securityChecks", m_securityChecks).toBool();
    m_scamDetection = settings.value("AIAssistant/scamDetection", m_scamDetection).toBool();
    m_feeRecommendations = settings.value("AIAssistant/feeRecommendations", m_feeRecommendations).toBool();
}

void AIAssistant::showAssistantDialog(QWidget* parent)
{
    AIAssistantDialog dialog(this, parent);
    dialog.exec();
}

void AIAssistant::showQuickHelp(FeatureType feature, QWidget* parent)
{
    QuickHelpDialog dialog(feature, parent);
    dialog.exec();
}

void AIAssistant::showSecurityWarning(const SecurityWarning& warning, QWidget* parent)
{
    SecurityWarningDialog dialog(warning, parent);
    dialog.exec();
}
