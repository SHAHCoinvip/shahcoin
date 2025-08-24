#ifndef SHAHCOIN_QT_AIASSISTANTINTEGRATION_H
#define SHAHCOIN_QT_AIASSISTANTINTEGRATION_H

#include <QObject>
#include <QAction>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QMenu>
#include <QTimer>

class LocalAIAssistant;
class WalletModel;
class MonetizationManager;
class AIAccessGate;
class EmailService;
class QMainWindow;
class QWidget;

/**
 * AI Assistant Integration for Shahcoin Qt Wallet
 *
 * Integrates the local AI Assistant into the main wallet interface,
 * providing easy access through buttons, menus, and tooltips.
 */
class AIAssistantIntegration : public QObject
{
    Q_OBJECT

public:
    explicit AIAssistantIntegration(QMainWindow* mainWindow, QObject* parent = nullptr);
    ~AIAssistantIntegration();

    // UI Integration
    void createAssistantButton();
    void createAssistantMenu();
    void createTooltipIntegration();
    void createContextHelp();

    // Assistant access
    LocalAIAssistant* getAssistant() const;
    void setAssistant(LocalAIAssistant* assistant);
    void setWalletModel(WalletModel* walletModel);
    void setMonetizationManager(MonetizationManager* monetizationManager);
    void setEmailService(EmailService* emailService);

    // UI elements
    QPushButton* getAssistantButton() const;
    QAction* getAssistantAction() const;
    QMenu* getAssistantMenu() const;

    // Settings
    void setButtonVisible(bool visible);
    void setButtonPosition(const QString& position);
    void setTooltipEnabled(bool enabled);
    void setContextHelpEnabled(bool enabled);

    bool isButtonVisible() const;
    bool isTooltipEnabled() const;
    bool isContextHelpEnabled() const;

public Q_SLOTS:
    void showAssistant();
    void showQuickHelp(const QString& topic);
    void showContextHelp(const QString& context);
    void updateAssistantStatus();

private Q_SLOTS:
    void onAssistantButtonClicked();
    void onAssistantActionTriggered();
    void onQuickHelpActionTriggered();
    void onSettingsActionTriggered();
    void onAboutActionTriggered();
    void onTooltipTimeout();
    void onUpgradeActionTriggered();
    void onBuyShahActionTriggered();
    void onAiInsightsActionTriggered();
    void onAiTroubleshooterActionTriggered();
    void onAiFeeOptimizerActionTriggered();
    void onAiLanguageSettingsActionTriggered();

private:
    // UI setup
    void setupAssistantButton();
    void setupAssistantMenu();
    void setupTooltips();
    void setupContextHelp();
    void positionAssistantButton();

    // Tooltip management
    void showTooltip(const QString& message, const QPoint& position);
    void hideTooltip();
    void updateTooltipContent();

    // Context help
    void setupContextHelpForWidget(QWidget* widget, const QString& helpText);
    void showContextHelpForWidget(QWidget* widget);

    // Settings
    void saveSettings();
    void loadSettings();

    // Data members
    QMainWindow* m_mainWindow;
    LocalAIAssistant* m_assistant;
    WalletModel* m_walletModel;
    MonetizationManager* m_monetization;
    AIAccessGate* m_accessGate;
    EmailService* m_emailService;
    
    // UI elements
    QPushButton* m_assistantButton;
    QAction* m_assistantAction;
    QMenu* m_assistantMenu;
    QLabel* m_tooltipLabel;
    QTimer* m_tooltipTimer;
    
    // Settings
    bool m_buttonVisible;
    QString m_buttonPosition;
    bool m_tooltipEnabled;
    bool m_contextHelpEnabled;
    
    // Context help mapping
    QMap<QWidget*, QString> m_contextHelpMap;
};

#endif // SHAHCOIN_QT_AIASSISTANTINTEGRATION_H
