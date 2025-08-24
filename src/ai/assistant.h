#ifndef SHAHCOIN_AI_ASSISTANT_H
#define SHAHCOIN_AI_ASSISTANT_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QMutex>
#include <QTimer>

class WalletModel;
class QWidget;

/**
 * Local AI Assistant with FAQ Knowledge Base
 *
 * Provides intelligent help and guidance to users using a pre-trained FAQ dataset.
 * Works offline without requiring internet connection or external AI APIs.
 */
class LocalAIAssistant : public QObject
{
    Q_OBJECT

public:
    enum MessageType {
        UserMessage,
        AssistantMessage,
        SystemMessage,
        SuggestionMessage
    };

    struct FAQEntry {
        QString question;
        QString answer;
        QStringList keywords;
        QString category;
        int priority;
        QStringList relatedQuestions;
    };

    struct AssistantMessage {
        MessageType type;
        QString content;
        QString timestamp;
        QString context;
        bool isProcessed;
        QJsonObject metadata;
    };

    struct Suggestion {
        QString question;
        double similarity;
        QString category;
    };

    explicit LocalAIAssistant(QObject* parent = nullptr);
    ~LocalAIAssistant();

    // Core functionality
    void askQuestion(const QString& question, const QString& context = "");
    QString getAnswer(const QString& question);
    QList<Suggestion> getSuggestions(const QString& input);
    bool hasAnswer(const QString& question) const;

    // FAQ management
    void loadFAQDatabase();
    void saveFAQDatabase();
    void addFAQEntry(const FAQEntry& entry);
    void removeFAQEntry(const QString& question);
    void updateFAQEntry(const QString& question, const FAQEntry& entry);
    QList<FAQEntry> getAllFAQs() const;
    QList<FAQEntry> getFAQsByCategory(const QString& category) const;

    // Fuzzy matching
    double calculateSimilarity(const QString& input, const QString& question) const;
    QString findBestMatch(const QString& input) const;
    QList<QString> findSimilarQuestions(const QString& input, int maxResults = 5) const;

    // Message history
    QList<AssistantMessage> getMessageHistory() const;
    void clearMessageHistory();
    void saveMessageHistory();
    void loadMessageHistory();

    // Settings
    void setEnabled(bool enabled);
    void setAutoSuggestions(bool autoSuggestions);
    void setSimilarityThreshold(double threshold);
    void setMaxSuggestions(int maxSuggestions);
    void setMaxHistorySize(int maxSize);

    bool isEnabled() const;
    bool isAutoSuggestions() const;
    double getSimilarityThreshold() const;
    int getMaxSuggestions() const;
    int getMaxHistorySize() const;

    // UI integration
    void showAssistantDialog(QWidget* parent = nullptr);
    void showQuickHelp(const QString& topic, QWidget* parent = nullptr);
    void showSuggestions(const QString& input, QWidget* parent = nullptr);

Q_SIGNALS:
    void messageProcessed(const QString& question, const QString& answer);
    void suggestionsUpdated(const QList<Suggestion>& suggestions);
    void suggestionSelected(const QString& suggestion);

public Q_SLOTS:
    void processUserInput(const QString& input);
    void autoSuggest(const QString& input);
    void updateKnowledgeBase();

private Q_SLOTS:
    void onAutoSuggestTimeout();

private:
    // Core processing
    void processQuestion(const QString& question, const QString& context);
    QString generateResponse(const QString& question, const FAQEntry& faq);
    QString formatResponse(const QString& answer, const QString& context);

    // Fuzzy matching algorithms
    double levenshteinDistance(const QString& s1, const QString& s2) const;
    double jaroWinklerDistance(const QString& s1, const QString& s2) const;
    double cosineSimilarity(const QString& s1, const QString& s2) const;
    QStringList tokenize(const QString& text) const;
    QString normalizeText(const QString& text) const;

    // Knowledge base management
    void initializeDefaultFAQs();
    void loadCustomFAQs();
    void saveCustomFAQs();
    QString getFAQFilePath() const;
    QString getHistoryFilePath() const;

    // Suggestion system
    void generateSuggestions(const QString& input);
    void updateSuggestionCache();
    void clearSuggestionCache();

    // Utility functions
    QString formatMessage(const QString& content, MessageType type);
    QString getCurrentContext();
    void logMessage(const QString& message, MessageType type);
    void saveSettings();
    void loadSettings();
    QString getTimestamp() const;

    // Data members
    QList<FAQEntry> m_faqDatabase;
    QList<AssistantMessage> m_messageHistory;
    QList<Suggestion> m_suggestions;
    QMap<QString, QStringList> m_suggestionCache;

    // Settings
    bool m_enabled;
    bool m_autoSuggestions;
    double m_similarityThreshold;
    int m_maxSuggestions;
    int m_maxHistorySize;

    // State
    bool m_isInitialized;
    QMutex m_dataMutex;
    QTimer* m_autoSuggestTimer;

    // Performance optimization
    QMap<QString, QStringList> m_tokenizedQuestions;
    QMap<QString, double> m_similarityCache;
};

/**
 * Local AI Assistant Dialog
 *
 * Main UI for interacting with the local AI Assistant
 */
class LocalAIAssistantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LocalAIAssistantDialog(LocalAIAssistant* assistant, QWidget* parent = nullptr);

private Q_SLOTS:
    void onSendMessage();
    void onClearHistory();
    void onSettingsClicked();
    void onQuickHelpClicked();
    void onSuggestionClicked(const QString& suggestion);
    void onInputChanged(const QString& text);

private:
    void setupUI();
    void setupConnections();
    void addMessage(const QString& content, LocalAIAssistant::MessageType type);
    void updateMessageList();
    void updateSuggestions(const QString& input);
    void showQuickHelpMenu();
    void showSuggestionsList(const QList<LocalAIAssistant::Suggestion>& suggestions);

    LocalAIAssistant* m_assistant;
    QVBoxLayout* m_layout;
    QListWidget* m_messageList;
    QLineEdit* m_inputField;
    QPushButton* m_sendButton;
    QPushButton* m_clearButton;
    QPushButton* m_settingsButton;
    QPushButton* m_helpButton;
    QLabel* m_statusLabel;
    QListWidget* m_suggestionsList;
    QTimer* m_inputTimer;
};

/**
 * Quick Help Dialog
 *
 * Shows quick help for specific topics
 */
class QuickHelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuickHelpDialog(const QString& topic, LocalAIAssistant* assistant, QWidget* parent = nullptr);

private Q_SLOTS:
    void onAskMoreClicked();

private:
    void setupUI();
    void loadTopicHelp();

    QString m_topic;
    LocalAIAssistant* m_assistant;
    QVBoxLayout* m_layout;
    QTextBrowser* m_helpText;
    QPushButton* m_closeButton;
    QPushButton* m_askMoreButton;
};

/**
 * Suggestions Dialog
 *
 * Shows suggestions for user input
 */
class SuggestionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SuggestionsDialog(const QList<LocalAIAssistant::Suggestion>& suggestions, QWidget* parent = nullptr);

signals:
    void suggestionSelected(const QString& suggestion);

private Q_SLOTS:
    void onSuggestionClicked(QListWidgetItem* item);

private:
    void setupUI();
    void populateSuggestions();

    QList<LocalAIAssistant::Suggestion> m_suggestions;
    QVBoxLayout* m_layout;
    QListWidget* m_suggestionsList;
    QPushButton* m_closeButton;
};

#endif // SHAHCOIN_AI_ASSISTANT_H
