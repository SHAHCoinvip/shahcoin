#include "assistant.h"
#include <qt/walletmodel.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>

#include <QApplication>
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
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QCollator>

// Default FAQ database
static const QList<LocalAIAssistant::FAQEntry> DEFAULT_FAQS = {
    {
        "What is Shahcoin?",
        "Shahcoin is a decentralized hybrid blockchain supporting SHAH cryptocurrency, staking, NFTs, and tokens. It uses a multi-algorithm proof-of-work and proof-of-stake consensus mechanism.",
        {"shahcoin", "blockchain", "cryptocurrency", "shah"},
        "General",
        10,
        {"How does Shahcoin work?", "What makes Shahcoin unique?"}
    },
    {
        "How do I create a wallet?",
        "Go to \"File → New Wallet\" or use \"Create Wallet\" on the welcome screen. You can also use the wallet manager to create multiple wallets.",
        {"wallet", "create", "new", "setup"},
        "Wallet",
        9,
        {"How do I backup my wallet?", "How do I encrypt my wallet?"}
    },
    {
        "How do I stake SHAH?",
        "Just hold ≥333 SHAH in your wallet. Staking rewards are earned automatically. You can also use cold staking for enhanced security.",
        {"stake", "staking", "rewards", "earn"},
        "Staking",
        9,
        {"What is cold staking?", "How much can I earn from staking?"}
    },
    {
        "How do I create a token?",
        "Use the \"Create Asset\" tab and select \"Token Minting\". A small SHAH fee applies based on current market rate.",
        {"token", "create", "mint", "asset"},
        "Tokens",
        8,
        {"How do I create an NFT?", "What are token fees?"}
    },
    {
        "What's the minimum transaction fee?",
        "0.001 SHAH per kilobyte. This protects the network from spam and ensures fast transaction processing.",
        {"fee", "transaction", "cost", "minimum"},
        "Fees",
        7,
        {"How are fees calculated?", "Can I adjust transaction fees?"}
    },
    {
        "How do I send SHAH?",
        "Go to the \"Send\" tab, enter the recipient address, amount, and click \"Send\". Always double-check the address before sending.",
        {"send", "transfer", "transaction"},
        "Transactions",
        8,
        {"How do I receive SHAH?", "How long do transactions take?"}
    },
    {
        "How do I backup my wallet?",
        "Go to \"Settings → Backup Wallet\" to save a backup file. Store it securely offline. You can also use the wallet manager for multiple wallet backups.",
        {"backup", "secure", "recovery", "export"},
        "Security",
        9,
        {"How do I restore a wallet?", "How do I encrypt my wallet?"}
    },
    {
        "What is cold staking?",
        "Cold staking allows you to stake SHAH while keeping your private keys offline for enhanced security. You delegate staking to a hot wallet.",
        {"cold", "staking", "delegate", "security"},
        "Staking",
        7,
        {"How do I set up cold staking?", "What are the benefits of cold staking?"}
    },
    {
        "How do I create an NFT?",
        "Use the \"Create Asset\" tab and select \"NFT Minting\". You can upload images and add metadata to your NFT.",
        {"nft", "mint", "create", "digital", "art"},
        "NFTs",
        8,
        {"How do I trade NFTs?", "What are NFT fees?"}
    },
    {
        "How do I use ShahSwap?",
        "Go to the \"ShahSwap\" tab, select two tokens, enter the amount, and click \"Swap Now\". Review the exchange rate and fees before confirming.",
        {"swap", "dex", "exchange", "trade"},
        "DEX",
        7,
        {"What are swap fees?", "How do I add liquidity?"}
    }
};

LocalAIAssistant::LocalAIAssistant(QObject* parent)
    : QObject(parent)
    , m_enabled(true)
    , m_autoSuggestions(true)
    , m_similarityThreshold(0.6)
    , m_maxSuggestions(5)
    , m_maxHistorySize(100)
    , m_isInitialized(false)
    , m_autoSuggestTimer(new QTimer(this))
{
    setupConnections();
    loadSettings();
    initializeDefaultFAQs();
    loadCustomFAQs();
    loadMessageHistory();
    m_isInitialized = true;
}

LocalAIAssistant::~LocalAIAssistant()
{
    saveSettings();
    saveFAQDatabase();
    saveMessageHistory();
}

void LocalAIAssistant::setupConnections()
{
    connect(m_autoSuggestTimer, &QTimer::timeout, this, &LocalAIAssistant::onAutoSuggestTimeout);
    m_autoSuggestTimer->setSingleShot(true);
    m_autoSuggestTimer->setInterval(500); // 500ms delay for auto-suggestions
}

void LocalAIAssistant::loadSettings()
{
    QSettings settings;
    m_enabled = settings.value("aiassistant/enabled", true).toBool();
    m_autoSuggestions = settings.value("aiassistant/autoSuggestions", true).toBool();
    m_similarityThreshold = settings.value("aiassistant/similarityThreshold", 0.6).toDouble();
    m_maxSuggestions = settings.value("aiassistant/maxSuggestions", 5).toInt();
    m_maxHistorySize = settings.value("aiassistant/maxHistorySize", 100).toInt();
}

void LocalAIAssistant::saveSettings()
{
    QSettings settings;
    settings.setValue("aiassistant/enabled", m_enabled);
    settings.setValue("aiassistant/autoSuggestions", m_autoSuggestions);
    settings.setValue("aiassistant/similarityThreshold", m_similarityThreshold);
    settings.setValue("aiassistant/maxSuggestions", m_maxSuggestions);
    settings.setValue("aiassistant/maxHistorySize", m_maxHistorySize);
}

void LocalAIAssistant::initializeDefaultFAQs()
{
    QMutexLocker locker(&m_dataMutex);
    
    // Add default FAQs
    for (const auto& faq : DEFAULT_FAQS) {
        m_faqDatabase.append(faq);
    }
    
    // Pre-tokenize questions for performance
    for (const auto& faq : m_faqDatabase) {
        m_tokenizedQuestions[faq.question] = tokenize(faq.question);
    }
}

void LocalAIAssistant::loadCustomFAQs()
{
    QString filePath = getFAQFilePath();
    QFile file(filePath);
    
    if (!file.exists()) {
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open FAQ file:" << filePath;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid FAQ file format:" << filePath;
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray faqsArray = root["faqs"].toArray();
    
    QMutexLocker locker(&m_dataMutex);
    
    for (const auto& faqValue : faqsArray) {
        QJsonObject faqObj = faqValue.toObject();
        FAQEntry faq;
        faq.question = faqObj["question"].toString();
        faq.answer = faqObj["answer"].toString();
        faq.category = faqObj["category"].toString();
        faq.priority = faqObj["priority"].toInt();
        
        QJsonArray keywordsArray = faqObj["keywords"].toArray();
        for (const auto& keyword : keywordsArray) {
            faq.keywords.append(keyword.toString());
        }
        
        QJsonArray relatedArray = faqObj["relatedQuestions"].toArray();
        for (const auto& related : relatedArray) {
            faq.relatedQuestions.append(related.toString());
        }
        
        m_faqDatabase.append(faq);
        m_tokenizedQuestions[faq.question] = tokenize(faq.question);
    }
}

void LocalAIAssistant::saveCustomFAQs()
{
    QString filePath = getFAQFilePath();
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create FAQ file:" << filePath;
        return;
    }
    
    QJsonObject root;
    QJsonArray faqsArray;
    
    QMutexLocker locker(&m_dataMutex);
    
    for (const auto& faq : m_faqDatabase) {
        QJsonObject faqObj;
        faqObj["question"] = faq.question;
        faqObj["answer"] = faq.answer;
        faqObj["category"] = faq.category;
        faqObj["priority"] = faq.priority;
        
        QJsonArray keywordsArray;
        for (const auto& keyword : faq.keywords) {
            keywordsArray.append(keyword);
        }
        faqObj["keywords"] = keywordsArray;
        
        QJsonArray relatedArray;
        for (const auto& related : faq.relatedQuestions) {
            relatedArray.append(related);
        }
        faqObj["relatedQuestions"] = relatedArray;
        
        faqsArray.append(faqObj);
    }
    
    root["faqs"] = faqsArray;
    root["version"] = "1.0";
    root["lastUpdated"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(root);
    file.write(doc.toJson());
}

QString LocalAIAssistant::getFAQFilePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + 
           "/shahcoin/ai/faqs.json";
}

QString LocalAIAssistant::getHistoryFilePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + 
           "/shahcoin/ai/history.json";
}

void LocalAIAssistant::askQuestion(const QString& question, const QString& context)
{
    if (!m_enabled) {
        return;
    }
    
    processQuestion(question, context);
}

QString LocalAIAssistant::getAnswer(const QString& question)
{
    if (!m_enabled) {
        return "AI Assistant is currently disabled.";
    }
    
    QString normalizedQuestion = normalizeText(question);
    QString bestMatch = findBestMatch(normalizedQuestion);
    
    if (bestMatch.isEmpty()) {
        return "Sorry, I don't understand. Try asking about Shahcoin, wallets, staking, tokens, NFTs, or transactions.";
    }
    
    QMutexLocker locker(&m_dataMutex);
    
    for (const auto& faq : m_faqDatabase) {
        if (normalizeText(faq.question) == bestMatch) {
            return generateResponse(question, faq);
        }
    }
    
    return "Sorry, I couldn't find a specific answer. Please try rephrasing your question.";
}

QList<LocalAIAssistant::Suggestion> LocalAIAssistant::getSuggestions(const QString& input)
{
    QList<Suggestion> suggestions;
    
    if (input.length() < 2) {
        return suggestions;
    }
    
    QString normalizedInput = normalizeText(input);
    QMutexLocker locker(&m_dataMutex);
    
    for (const auto& faq : m_faqDatabase) {
        double similarity = calculateSimilarity(normalizedInput, normalizeText(faq.question));
        
        if (similarity >= m_similarityThreshold) {
            Suggestion suggestion;
            suggestion.question = faq.question;
            suggestion.similarity = similarity;
            suggestion.category = faq.category;
            suggestions.append(suggestion);
        }
    }
    
    // Sort by similarity and priority
    std::sort(suggestions.begin(), suggestions.end(), 
              [this](const Suggestion& a, const Suggestion& b) {
                  if (qAbs(a.similarity - b.similarity) < 0.01) {
                      // If similarity is close, sort by priority
                      auto faqA = std::find_if(m_faqDatabase.begin(), m_faqDatabase.end(),
                                              [&a](const FAQEntry& faq) { return faq.question == a.question; });
                      auto faqB = std::find_if(m_faqDatabase.begin(), m_faqDatabase.end(),
                                              [&b](const FAQEntry& faq) { return faq.question == b.question; });
                      
                      if (faqA != m_faqDatabase.end() && faqB != m_faqDatabase.end()) {
                          return faqA->priority > faqB->priority;
                      }
                  }
                  return a.similarity > b.similarity;
              });
    
    // Limit to max suggestions
    if (suggestions.size() > m_maxSuggestions) {
        suggestions = suggestions.mid(0, m_maxSuggestions);
    }
    
    return suggestions;
}

double LocalAIAssistant::calculateSimilarity(const QString& input, const QString& question) const
{
    // Check cache first
    QString cacheKey = input + "|" + question;
    if (m_similarityCache.contains(cacheKey)) {
        return m_similarityCache[cacheKey];
    }
    
    // Calculate multiple similarity metrics
    double levenshtein = 1.0 - (levenshteinDistance(input, question) / qMax(input.length(), question.length()));
    double jaroWinkler = jaroWinklerDistance(input, question);
    double cosine = cosineSimilarity(input, question);
    
    // Weighted combination
    double similarity = (levenshtein * 0.3) + (jaroWinkler * 0.4) + (cosine * 0.3);
    
    // Cache the result
    m_similarityCache[cacheKey] = similarity;
    
    return similarity;
}

double LocalAIAssistant::levenshteinDistance(const QString& s1, const QString& s2) const
{
    int len1 = s1.length();
    int len2 = s2.length();
    
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;
    
    QVector<QVector<int>> matrix(len1 + 1, QVector<int>(len2 + 1));
    
    for (int i = 0; i <= len1; i++) {
        matrix[i][0] = i;
    }
    
    for (int j = 0; j <= len2; j++) {
        matrix[0][j] = j;
    }
    
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            matrix[i][j] = qMin(qMin(matrix[i-1][j] + 1, matrix[i][j-1] + 1), matrix[i-1][j-1] + cost);
        }
    }
    
    return matrix[len1][len2];
}

double LocalAIAssistant::jaroWinklerDistance(const QString& s1, const QString& s2) const
{
    if (s1 == s2) return 1.0;
    if (s1.isEmpty() || s2.isEmpty()) return 0.0;
    
    int len1 = s1.length();
    int len2 = s2.length();
    
    int matchDistance = qMax(len1, len2) / 2 - 1;
    if (matchDistance < 0) matchDistance = 0;
    
    QVector<bool> s1Matches(len1, false);
    QVector<bool> s2Matches(len2, false);
    
    int matches = 0;
    int transpositions = 0;
    
    // Find matches
    for (int i = 0; i < len1; i++) {
        int start = qMax(0, i - matchDistance);
        int end = qMin(len2 - 1, i + matchDistance);
        
        for (int j = start; j <= end; j++) {
            if (s2Matches[j]) continue;
            if (s1[i] != s2[j]) continue;
            
            s1Matches[i] = true;
            s2Matches[j] = true;
            matches++;
            break;
        }
    }
    
    if (matches == 0) return 0.0;
    
    // Find transpositions
    int k = 0;
    for (int i = 0; i < len1; i++) {
        if (!s1Matches[i]) continue;
        
        while (!s2Matches[k]) k++;
        if (s1[i] != s2[k]) transpositions++;
        k++;
    }
    
    double jaro = (matches / (double)len1 + matches / (double)len2 + (matches - transpositions / 2.0) / matches) / 3.0;
    
    // Winkler modification
    int prefixLength = 0;
    for (int i = 0; i < qMin(4, qMin(len1, len2)); i++) {
        if (s1[i] == s2[i]) {
            prefixLength++;
        } else {
            break;
        }
    }
    
    return jaro + (0.1 * prefixLength * (1.0 - jaro));
}

double LocalAIAssistant::cosineSimilarity(const QString& s1, const QString& s2) const
{
    QStringList tokens1 = tokenize(s1);
    QStringList tokens2 = tokenize(s2);
    
    if (tokens1.isEmpty() && tokens2.isEmpty()) return 1.0;
    if (tokens1.isEmpty() || tokens2.isEmpty()) return 0.0;
    
    QMap<QString, int> freq1, freq2;
    
    for (const auto& token : tokens1) {
        freq1[token]++;
    }
    
    for (const auto& token : tokens2) {
        freq2[token]++;
    }
    
    double dotProduct = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;
    
    QSet<QString> allTokens;
    for (auto it = freq1.begin(); it != freq1.end(); ++it) {
        allTokens.insert(it.key());
    }
    for (auto it = freq2.begin(); it != freq2.end(); ++it) {
        allTokens.insert(it.key());
    }
    
    for (const auto& token : allTokens) {
        int f1 = freq1.value(token, 0);
        int f2 = freq2.value(token, 0);
        
        dotProduct += f1 * f2;
        norm1 += f1 * f1;
        norm2 += f2 * f2;
    }
    
    if (norm1 == 0.0 || norm2 == 0.0) return 0.0;
    
    return dotProduct / (sqrt(norm1) * sqrt(norm2));
}

QStringList LocalAIAssistant::tokenize(const QString& text) const
{
    QStringList tokens;
    QString normalized = normalizeText(text);
    
    // Simple word tokenization
    QRegularExpression wordRegex("\\b\\w+\\b");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(normalized);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString token = match.captured().toLower();
        if (token.length() > 1) { // Skip single characters
            tokens.append(token);
        }
    }
    
    return tokens;
}

QString LocalAIAssistant::normalizeText(const QString& text) const
{
    QString normalized = text.toLower();
    
    // Remove punctuation except apostrophes
    normalized = normalized.replace(QRegularExpression("[^\\w\\s']"), " ");
    
    // Normalize whitespace
    normalized = normalized.replace(QRegularExpression("\\s+"), " ").trimmed();
    
    return normalized;
}

QString LocalAIAssistant::findBestMatch(const QString& input) const
{
    QString bestMatch;
    double bestSimilarity = 0.0;
    
    QMutexLocker locker(&m_dataMutex);
    
    for (const auto& faq : m_faqDatabase) {
        double similarity = calculateSimilarity(input, normalizeText(faq.question));
        
        if (similarity > bestSimilarity && similarity >= m_similarityThreshold) {
            bestSimilarity = similarity;
            bestMatch = normalizeText(faq.question);
        }
    }
    
    return bestMatch;
}

QList<QString> LocalAIAssistant::findSimilarQuestions(const QString& input, int maxResults) const
{
    QList<QPair<QString, double>> similarities;
    
    QMutexLocker locker(&m_dataMutex);
    
    for (const auto& faq : m_faqDatabase) {
        double similarity = calculateSimilarity(input, normalizeText(faq.question));
        if (similarity >= m_similarityThreshold) {
            similarities.append(qMakePair(faq.question, similarity));
        }
    }
    
    // Sort by similarity
    std::sort(similarities.begin(), similarities.end(),
              [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
                  return a.second > b.second;
              });
    
    QList<QString> results;
    for (int i = 0; i < qMin(maxResults, similarities.size()); i++) {
        results.append(similarities[i].first);
    }
    
    return results;
}

void LocalAIAssistant::processQuestion(const QString& question, const QString& context)
{
    QString answer = getAnswer(question);
    
    // Log the interaction
    logMessage(question, UserMessage);
    logMessage(answer, AssistantMessage);
    
    // Emit signals for UI updates
    emit messageProcessed(question, answer);
}

QString LocalAIAssistant::generateResponse(const QString& question, const FAQEntry& faq)
{
    QString response = faq.answer;
    
    // Add context if available
    if (!faq.relatedQuestions.isEmpty()) {
        response += "\n\nRelated questions: ";
        for (int i = 0; i < qMin(3, faq.relatedQuestions.size()); i++) {
            if (i > 0) response += ", ";
            response += faq.relatedQuestions[i];
        }
    }
    
    return response;
}

void LocalAIAssistant::logMessage(const QString& message, MessageType type)
{
    QMutexLocker locker(&m_dataMutex);
    
    AssistantMessage msg;
    msg.type = type;
    msg.content = message;
    msg.timestamp = getTimestamp();
    msg.context = getCurrentContext();
    msg.isProcessed = true;
    
    m_messageHistory.append(msg);
    
    // Limit history size
    if (m_messageHistory.size() > m_maxHistorySize) {
        m_messageHistory = m_messageHistory.mid(m_messageHistory.size() - m_maxHistorySize);
    }
}

QString LocalAIAssistant::getCurrentContext() const
{
    // Could be enhanced to include current wallet state, active tab, etc.
    return "Shahcoin Wallet";
}

QString LocalAIAssistant::getTimestamp() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void LocalAIAssistant::loadMessageHistory()
{
    QString filePath = getHistoryFilePath();
    QFile file(filePath);
    
    if (!file.exists()) {
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open history file:" << filePath;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid history file format:" << filePath;
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray historyArray = root["history"].toArray();
    
    QMutexLocker locker(&m_dataMutex);
    m_messageHistory.clear();
    
    for (const auto& msgValue : historyArray) {
        QJsonObject msgObj = msgValue.toObject();
        AssistantMessage msg;
        msg.type = static_cast<MessageType>(msgObj["type"].toInt());
        msg.content = msgObj["content"].toString();
        msg.timestamp = msgObj["timestamp"].toString();
        msg.context = msgObj["context"].toString();
        msg.isProcessed = msgObj["isProcessed"].toBool();
        
        m_messageHistory.append(msg);
    }
}

void LocalAIAssistant::saveMessageHistory()
{
    QString filePath = getHistoryFilePath();
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create history file:" << filePath;
        return;
    }
    
    QJsonObject root;
    QJsonArray historyArray;
    
    QMutexLocker locker(&m_dataMutex);
    
    for (const auto& msg : m_messageHistory) {
        QJsonObject msgObj;
        msgObj["type"] = static_cast<int>(msg.type);
        msgObj["content"] = msg.content;
        msgObj["timestamp"] = msg.timestamp;
        msgObj["context"] = msg.context;
        msgObj["isProcessed"] = msg.isProcessed;
        
        historyArray.append(msgObj);
    }
    
    root["history"] = historyArray;
    root["version"] = "1.0";
    root["lastUpdated"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(root);
    file.write(doc.toJson());
}

// UI Dialog implementations
LocalAIAssistantDialog::LocalAIAssistantDialog(LocalAIAssistant* assistant, QWidget* parent)
    : QDialog(parent)
    , m_assistant(assistant)
    , m_layout(new QVBoxLayout(this))
    , m_messageList(new QListWidget(this))
    , m_inputField(new QLineEdit(this))
    , m_sendButton(new QPushButton("Send", this))
    , m_clearButton(new QPushButton("Clear", this))
    , m_settingsButton(new QPushButton("Settings", this))
    , m_helpButton(new QPushButton("Quick Help", this))
    , m_statusLabel(new QLabel("Shahcoin Assistant", this))
    , m_suggestionsList(new QListWidget(this))
    , m_inputTimer(new QTimer(this))
{
    setWindowTitle("Shahcoin AI Assistant");
    setMinimumSize(500, 400);
    
    setupUI();
    setupConnections();
    updateMessageList();
}

void LocalAIAssistantDialog::setupUI()
{
    // Status label
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
    m_layout->addWidget(m_statusLabel);
    
    // Message list
    m_messageList->setAlternatingRowColors(true);
    m_messageList->setWordWrap(true);
    m_layout->addWidget(m_messageList);
    
    // Suggestions list
    m_suggestionsList->setMaximumHeight(100);
    m_suggestionsList->setVisible(false);
    m_layout->addWidget(m_suggestionsList);
    
    // Input area
    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_inputField->setPlaceholderText("Ask me anything about Shahcoin...");
    inputLayout->addWidget(m_inputField);
    inputLayout->addWidget(m_sendButton);
    m_layout->addLayout(inputLayout);
    
    // Button area
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_helpButton);
    buttonLayout->addWidget(m_settingsButton);
    buttonLayout->addStretch();
    m_layout->addLayout(buttonLayout);
    
    // Set up input timer for auto-suggestions
    m_inputTimer->setSingleShot(true);
    m_inputTimer->setInterval(300);
}

void LocalAIAssistantDialog::setupConnections()
{
    connect(m_sendButton, &QPushButton::clicked, this, &LocalAIAssistantDialog::onSendMessage);
    connect(m_clearButton, &QPushButton::clicked, this, &LocalAIAssistantDialog::onClearHistory);
    connect(m_settingsButton, &QPushButton::clicked, this, &LocalAIAssistantDialog::onSettingsClicked);
    connect(m_helpButton, &QPushButton::clicked, this, &LocalAIAssistantDialog::onQuickHelpClicked);
    
    connect(m_inputField, &QLineEdit::textChanged, this, &LocalAIAssistantDialog::onInputChanged);
    connect(m_inputField, &QLineEdit::returnPressed, this, &LocalAIAssistantDialog::onSendMessage);
    
    connect(m_inputTimer, &QTimer::timeout, [this]() {
        updateSuggestions(m_inputField->text());
    });
    
    connect(m_suggestionsList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        onSuggestionClicked(item->text());
    });
}

void LocalAIAssistantDialog::onSendMessage()
{
    QString text = m_inputField->text().trimmed();
    if (text.isEmpty()) {
        return;
    }
    
    addMessage(text, LocalAIAssistant::UserMessage);
    m_inputField->clear();
    m_suggestionsList->setVisible(false);
    
    // Process the question
    if (m_assistant) {
        m_assistant->askQuestion(text);
        
        // Get the answer
        QString answer = m_assistant->getAnswer(text);
        addMessage(answer, LocalAIAssistant::AssistantMessage);
    }
}

void LocalAIAssistantDialog::onClearHistory()
{
    if (m_assistant) {
        m_assistant->clearMessageHistory();
    }
    m_messageList->clear();
}

void LocalAIAssistantDialog::onSettingsClicked()
{
    QMessageBox::information(this, "Settings", 
        "AI Assistant Settings:\n\n"
        "• Enabled: " + QString(m_assistant->isEnabled() ? "Yes" : "No") + "\n"
        "• Auto-suggestions: " + QString(m_assistant->isAutoSuggestions() ? "Yes" : "No") + "\n"
        "• Similarity threshold: " + QString::number(m_assistant->getSimilarityThreshold()) + "\n"
        "• Max suggestions: " + QString::number(m_assistant->getMaxSuggestions()));
}

void LocalAIAssistantDialog::onQuickHelpClicked()
{
    showQuickHelpMenu();
}

void LocalAIAssistantDialog::onSuggestionClicked(const QString& suggestion)
{
    m_inputField->setText(suggestion);
    m_suggestionsList->setVisible(false);
    onSendMessage();
}

void LocalAIAssistantDialog::onInputChanged(const QString& text)
{
    if (text.length() >= 2 && m_assistant && m_assistant->isAutoSuggestions()) {
        m_inputTimer->start();
    } else {
        m_suggestionsList->setVisible(false);
    }
}

void LocalAIAssistantDialog::addMessage(const QString& content, LocalAIAssistant::MessageType type)
{
    QListWidgetItem* item = new QListWidgetItem();
    
    QString displayText;
    switch (type) {
        case LocalAIAssistant::UserMessage:
            displayText = "You: " + content;
            item->setBackground(QColor(240, 248, 255)); // Light blue
            break;
        case LocalAIAssistant::AssistantMessage:
            displayText = "Assistant: " + content;
            item->setBackground(QColor(255, 248, 240)); // Light orange
            break;
        case LocalAIAssistant::SystemMessage:
            displayText = "System: " + content;
            item->setBackground(QColor(240, 240, 240)); // Light gray
            break;
        case LocalAIAssistant::SuggestionMessage:
            displayText = "💡 " + content;
            item->setBackground(QColor(255, 255, 224)); // Light yellow
            break;
    }
    
    item->setText(displayText);
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_messageList->addItem(item);
    m_messageList->scrollToBottom();
}

void LocalAIAssistantDialog::updateMessageList()
{
    if (!m_assistant) {
        return;
    }
    
    m_messageList->clear();
    QList<LocalAIAssistant::AssistantMessage> history = m_assistant->getMessageHistory();
    
    for (const auto& msg : history) {
        addMessage(msg.content, msg.type);
    }
}

void LocalAIAssistantDialog::updateSuggestions(const QString& input)
{
    if (!m_assistant) {
        return;
    }
    
    QList<LocalAIAssistant::Suggestion> suggestions = m_assistant->getSuggestions(input);
    
    m_suggestionsList->clear();
    
    if (suggestions.isEmpty()) {
        m_suggestionsList->setVisible(false);
        return;
    }
    
    for (const auto& suggestion : suggestions) {
        QListWidgetItem* item = new QListWidgetItem(suggestion.question);
        item->setToolTip(QString("Similarity: %1%").arg(suggestion.similarity * 100, 0, 'f', 1));
        m_suggestionsList->addItem(item);
    }
    
    m_suggestionsList->setVisible(true);
}

void LocalAIAssistantDialog::showQuickHelpMenu()
{
    QMenu menu(this);
    
    QStringList topics = {"General", "Wallet", "Staking", "Tokens", "NFTs", "Transactions", "Security", "Fees", "DEX"};
    
    for (const auto& topic : topics) {
        QAction* action = menu.addAction(topic);
        connect(action, &QAction::triggered, [this, topic]() {
            if (m_assistant) {
                QList<LocalAIAssistant::FAQEntry> faqs = m_assistant->getFAQsByCategory(topic);
                if (!faqs.isEmpty()) {
                    QString question = faqs.first().question;
                    m_inputField->setText(question);
                    onSendMessage();
                }
            }
        });
    }
    
    menu.exec(m_helpButton->mapToGlobal(m_helpButton->rect().bottomLeft()));
}

// Include the rest of the implementation...

// Missing method implementations
void LocalAIAssistant::processUserInput(const QString& input)
{
    askQuestion(input);
}

void LocalAIAssistant::autoSuggest(const QString& input)
{
    if (!m_autoSuggestions || input.length() < 2) {
        return;
    }
    
    generateSuggestions(input);
}

void LocalAIAssistant::updateKnowledgeBase()
{
    // This could be enhanced to update from external sources
    // For now, just reload custom FAQs
    loadCustomFAQs();
}

void LocalAIAssistant::onAutoSuggestTimeout()
{
    // Auto-suggestion timeout handler
    // Could be used for delayed suggestions
}

void LocalAIAssistant::generateSuggestions(const QString& input)
{
    m_suggestions = getSuggestions(input);
    emit suggestionsUpdated(m_suggestions);
}

void LocalAIAssistant::updateSuggestionCache()
{
    // Update suggestion cache for performance
    m_suggestionCache.clear();
    
    QMutexLocker locker(&m_dataMutex);
    for (const auto& faq : m_faqDatabase) {
        QStringList tokens = tokenize(faq.question);
        for (const auto& token : tokens) {
            if (!m_suggestionCache.contains(token)) {
                m_suggestionCache[token] = QStringList();
            }
            m_suggestionCache[token].append(faq.question);
        }
    }
}

void LocalAIAssistant::clearSuggestionCache()
{
    m_suggestionCache.clear();
    m_similarityCache.clear();
}

void LocalAIAssistant::addFAQEntry(const FAQEntry& entry)
{
    QMutexLocker locker(&m_dataMutex);
    m_faqDatabase.append(entry);
    m_tokenizedQuestions[entry.question] = tokenize(entry.question);
    saveCustomFAQs();
}

void LocalAIAssistant::removeFAQEntry(const QString& question)
{
    QMutexLocker locker(&m_dataMutex);
    
    for (int i = 0; i < m_faqDatabase.size(); i++) {
        if (m_faqDatabase[i].question == question) {
            m_faqDatabase.removeAt(i);
            m_tokenizedQuestions.remove(question);
            break;
        }
    }
    
    saveCustomFAQs();
}

void LocalAIAssistant::updateFAQEntry(const QString& question, const FAQEntry& entry)
{
    QMutexLocker locker(&m_dataMutex);
    
    for (int i = 0; i < m_faqDatabase.size(); i++) {
        if (m_faqDatabase[i].question == question) {
            m_faqDatabase[i] = entry;
            m_tokenizedQuestions[entry.question] = tokenize(entry.question);
            break;
        }
    }
    
    saveCustomFAQs();
}

QList<LocalAIAssistant::FAQEntry> LocalAIAssistant::getAllFAQs() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_faqDatabase;
}

QList<LocalAIAssistant::FAQEntry> LocalAIAssistant::getFAQsByCategory(const QString& category) const
{
    QMutexLocker locker(&m_dataMutex);
    QList<FAQEntry> result;
    
    for (const auto& faq : m_faqDatabase) {
        if (faq.category == category) {
            result.append(faq);
        }
    }
    
    return result;
}

QList<LocalAIAssistant::AssistantMessage> LocalAIAssistant::getMessageHistory() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_messageHistory;
}

void LocalAIAssistant::clearMessageHistory()
{
    QMutexLocker locker(&m_dataMutex);
    m_messageHistory.clear();
    saveMessageHistory();
}

void LocalAIAssistant::setEnabled(bool enabled)
{
    m_enabled = enabled;
    saveSettings();
}

void LocalAIAssistant::setAutoSuggestions(bool autoSuggestions)
{
    m_autoSuggestions = autoSuggestions;
    saveSettings();
}

void LocalAIAssistant::setSimilarityThreshold(double threshold)
{
    m_similarityThreshold = qBound(0.0, threshold, 1.0);
    saveSettings();
}

void LocalAIAssistant::setMaxSuggestions(int maxSuggestions)
{
    m_maxSuggestions = qBound(1, maxSuggestions, 20);
    saveSettings();
}

void LocalAIAssistant::setMaxHistorySize(int maxSize)
{
    m_maxHistorySize = qBound(10, maxSize, 1000);
    saveSettings();
}

bool LocalAIAssistant::isEnabled() const
{
    return m_enabled;
}

bool LocalAIAssistant::isAutoSuggestions() const
{
    return m_autoSuggestions;
}

double LocalAIAssistant::getSimilarityThreshold() const
{
    return m_similarityThreshold;
}

int LocalAIAssistant::getMaxSuggestions() const
{
    return m_maxSuggestions;
}

int LocalAIAssistant::getMaxHistorySize() const
{
    return m_maxHistorySize;
}

bool LocalAIAssistant::hasAnswer(const QString& question) const
{
    QString normalizedQuestion = normalizeText(question);
    QString bestMatch = findBestMatch(normalizedQuestion);
    return !bestMatch.isEmpty();
}

void LocalAIAssistant::showAssistantDialog(QWidget* parent)
{
    LocalAIAssistantDialog* dialog = new LocalAIAssistantDialog(this, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void LocalAIAssistant::showQuickHelp(const QString& topic, QWidget* parent)
{
    QuickHelpDialog* dialog = new QuickHelpDialog(topic, this, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void LocalAIAssistant::showSuggestions(const QString& input, QWidget* parent)
{
    QList<Suggestion> suggestions = getSuggestions(input);
    if (!suggestions.isEmpty()) {
        SuggestionsDialog* dialog = new SuggestionsDialog(suggestions, parent);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &SuggestionsDialog::suggestionSelected, [this](const QString& suggestion) {
            emit suggestionSelected(suggestion);
        });
        dialog->show();
        dialog->raise();
        dialog->activateWindow();
    }
}

// QuickHelpDialog implementation
QuickHelpDialog::QuickHelpDialog(const QString& topic, LocalAIAssistant* assistant, QWidget* parent)
    : QDialog(parent)
    , m_topic(topic)
    , m_assistant(assistant)
    , m_layout(new QVBoxLayout(this))
    , m_helpText(new QTextBrowser(this))
    , m_closeButton(new QPushButton("Close", this))
    , m_askMoreButton(new QPushButton("Ask More", this))
{
    setWindowTitle("Quick Help - " + topic);
    setMinimumSize(400, 300);
    
    setupUI();
    loadTopicHelp();
}

void QuickHelpDialog::setupUI()
{
    // Help text
    m_helpText->setOpenExternalLinks(true);
    m_layout->addWidget(m_helpText);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_askMoreButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);
    m_layout->addLayout(buttonLayout);
    
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_askMoreButton, &QPushButton::clicked, this, &QuickHelpDialog::onAskMoreClicked);
}

void QuickHelpDialog::loadTopicHelp()
{
    if (!m_assistant) {
        m_helpText->setText("Assistant not available.");
        return;
    }
    
    QList<LocalAIAssistant::FAQEntry> faqs = m_assistant->getFAQsByCategory(m_topic);
    
    QString helpContent = QString("<h2>%1 Help</h2>").arg(m_topic);
    
    if (faqs.isEmpty()) {
        helpContent += "<p>No help content available for this topic.</p>";
    } else {
        helpContent += "<ul>";
        for (const auto& faq : faqs) {
            helpContent += QString("<li><strong>%1</strong><br>%2</li>")
                          .arg(faq.question)
                          .arg(faq.answer);
        }
        helpContent += "</ul>";
    }
    
    m_helpText->setHtml(helpContent);
}

void QuickHelpDialog::onAskMoreClicked()
{
    if (m_assistant) {
        m_assistant->showAssistantDialog(this);
    }
}

// SuggestionsDialog implementation
SuggestionsDialog::SuggestionsDialog(const QList<LocalAIAssistant::Suggestion>& suggestions, QWidget* parent)
    : QDialog(parent)
    , m_suggestions(suggestions)
    , m_layout(new QVBoxLayout(this))
    , m_suggestionsList(new QListWidget(this))
    , m_closeButton(new QPushButton("Close", this))
{
    setWindowTitle("Suggestions");
    setMinimumSize(300, 200);
    
    setupUI();
    populateSuggestions();
}

void SuggestionsDialog::setupUI()
{
    QLabel* titleLabel = new QLabel("Did you mean:", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    m_layout->addWidget(titleLabel);
    
    m_suggestionsList->setAlternatingRowColors(true);
    m_layout->addWidget(m_suggestionsList);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);
    m_layout->addLayout(buttonLayout);
    
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_suggestionsList, &QListWidget::itemClicked, this, &SuggestionsDialog::onSuggestionClicked);
}

void SuggestionsDialog::populateSuggestions()
{
    for (const auto& suggestion : m_suggestions) {
        QListWidgetItem* item = new QListWidgetItem(suggestion.question);
        item->setToolTip(QString("Similarity: %1%").arg(suggestion.similarity * 100, 0, 'f', 1));
        m_suggestionsList->addItem(item);
    }
}

void SuggestionsDialog::onSuggestionClicked(QListWidgetItem* item)
{
    emit suggestionSelected(item->text());
    accept();
}
