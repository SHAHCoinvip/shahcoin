#include "aiassistantintegration.h"
#include "../ai/assistant.h"
#include "aiaccessgate.h"
#include "monetizationmanager.h"
#include "upgradeaipanel.h"
#include "aipayments.h"
#include "emailservice.h"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QSettings>
#include <QDebug>
#include <QStyle>
#include <QScreen>
#include <QDesktopWidget>
#include <QToolTip>
#include <QEvent>
#include <QMouseEvent>
#include <QHelpEvent>
#include <QMessageBox>

AIAssistantIntegration::AIAssistantIntegration(QMainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_assistant(nullptr)
    , m_walletModel(nullptr)
    , m_monetization(nullptr)
    , m_accessGate(nullptr)
    , m_emailService(nullptr)
    , m_assistantButton(nullptr)
    , m_assistantAction(nullptr)
    , m_assistantMenu(nullptr)
    , m_tooltipLabel(nullptr)
    , m_tooltipTimer(new QTimer(this))
    , m_buttonVisible(true)
    , m_buttonPosition("bottom-right")
    , m_tooltipEnabled(true)
    , m_contextHelpEnabled(true)
{
    loadSettings();
    setupAssistantButton();
    setupAssistantMenu();
    setupTooltips();
    setupContextHelp();
}

AIAssistantIntegration::~AIAssistantIntegration()
{
    saveSettings();
}

void AIAssistantIntegration::loadSettings()
{
    QSettings settings;
    m_buttonVisible = settings.value("aiassistant/buttonVisible", true).toBool();
    m_buttonPosition = settings.value("aiassistant/buttonPosition", "bottom-right").toString();
    m_tooltipEnabled = settings.value("aiassistant/tooltipEnabled", true).toBool();
    m_contextHelpEnabled = settings.value("aiassistant/contextHelpEnabled", true).toBool();
}

void AIAssistantIntegration::saveSettings()
{
    QSettings settings;
    settings.setValue("aiassistant/buttonVisible", m_buttonVisible);
    settings.setValue("aiassistant/buttonPosition", m_buttonPosition);
    settings.setValue("aiassistant/tooltipEnabled", m_tooltipEnabled);
    settings.setValue("aiassistant/contextHelpEnabled", m_contextHelpEnabled);
}

void AIAssistantIntegration::setupAssistantButton()
{
    if (!m_mainWindow) {
        return;
    }

    // Create assistant button
    m_assistantButton = new QPushButton(" 916", m_mainWindow);
    m_assistantButton->setToolTip("Need help? Ask Shahcoin Assistant!");
    m_assistantButton->setFixedSize(40, 40);
    m_assistantButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    border: 2px solid #45a049;"
        "    border-radius: 20px;"
        "    color: white;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45a049;"
        "    border-color: #3d8b40;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3d8b40;"
        "    border-color: #2d5a2d;"
        "}"
    );

    // Position the button
    positionAssistantButton();

    // Connect signals
    connect(m_assistantButton, &QPushButton::clicked, this, &AIAssistantIntegration::onAssistantButtonClicked);

    // Set visibility
    m_assistantButton->setVisible(m_buttonVisible);
}

void AIAssistantIntegration::setupAssistantMenu()
{
    if (!m_mainWindow) {
        return;
    }

    // Create assistant action for menu bar
    m_assistantAction = new QAction(" 916 Shahcoin Assistant", this);
    m_assistantAction->setToolTip("Open AI Assistant for help and guidance");
    m_assistantAction->setShortcut(QKeySequence("Ctrl+Shift+A"));
    connect(m_assistantAction, &QAction::triggered, this, &AIAssistantIntegration::onAssistantActionTriggered);

    // Create assistant menu
    m_assistantMenu = new QMenu("Assistant", m_mainWindow);
    
    // Add actions to menu
    QAction* openAction = m_assistantMenu->addAction("Open Assistant");
    connect(openAction, &QAction::triggered, this, &AIAssistantIntegration::showAssistant);
    QAction* upgradeAction = m_assistantMenu->addAction("Upgrade to AI Wallet…");
    connect(upgradeAction, &QAction::triggered, this, &AIAssistantIntegration::onUpgradeActionTriggered);
    QAction* buyShahAction = m_assistantMenu->addAction("Buy SHAH (Credit Card)…");
    connect(buyShahAction, &QAction::triggered, this, &AIAssistantIntegration::onBuyShahActionTriggered);

    // AI Tools submenu (all gated)
    QMenu* toolsMenu = m_assistantMenu->addMenu("AI Tools");
    toolsMenu->addAction("Insights", this, &AIAssistantIntegration::onAiInsightsActionTriggered);
    toolsMenu->addAction("Troubleshooter", this, &AIAssistantIntegration::onAiTroubleshooterActionTriggered);
    toolsMenu->addAction("Fee Optimizer", this, &AIAssistantIntegration::onAiFeeOptimizerActionTriggered);
    toolsMenu->addAction("Language Settings", this, &AIAssistantIntegration::onAiLanguageSettingsActionTriggered);

    // Add to main window menu bar if available
    if (m_mainWindow->menuBar()) {
        m_mainWindow->menuBar()->addMenu(m_assistantMenu);
    }
}

void AIAssistantIntegration::setupTooltips()
{
    if (!m_tooltipEnabled) {
        return;
    }

    // Create tooltip label
    m_tooltipLabel = new QLabel(m_mainWindow);
    m_tooltipLabel->setStyleSheet(
        "QLabel {"
        "    background-color: #2c3e50;"
        "    color: white;"
        "    border: 1px solid #34495e;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "    font-size: 12px;"
        "}"
    );
    m_tooltipLabel->setWordWrap(true);
    m_tooltipLabel->setVisible(false);

    // Setup tooltip timer
    m_tooltipTimer->setSingleShot(true);
    m_tooltipTimer->setInterval(3000); // 3 seconds
    connect(m_tooltipTimer, &QTimer::timeout, this, &AIAssistantIntegration::onTooltipTimeout);
}

void AIAssistantIntegration::setupContextHelp()
{
    if (!m_contextHelpEnabled || !m_mainWindow) {
        return;
    }

    // Install event filter for context help
    m_mainWindow->installEventFilter(this);
}

void AIAssistantIntegration::positionAssistantButton()
{
    if (!m_assistantButton || !m_mainWindow) {
        return;
    }

    QRect windowRect = m_mainWindow->geometry();
    int buttonSize = 40;
    int margin = 20;

    if (m_buttonPosition == "bottom-right") {
        m_assistantButton->move(
            windowRect.width() - buttonSize - margin,
            windowRect.height() - buttonSize - margin
        );
    } else if (m_buttonPosition == "bottom-left") {
        m_assistantButton->move(margin, windowRect.height() - buttonSize - margin);
    } else if (m_buttonPosition == "top-right") {
        m_assistantButton->move(windowRect.width() - buttonSize - margin, margin);
    } else if (m_buttonPosition == "top-left") {
        m_assistantButton->move(margin, margin);
    }
}

void AIAssistantIntegration::createAssistantButton()
{
    setupAssistantButton();
}

void AIAssistantIntegration::createAssistantMenu()
{
    setupAssistantMenu();
}

void AIAssistantIntegration::createTooltipIntegration()
{
    setupTooltips();
}

void AIAssistantIntegration::createContextHelp()
{
    setupContextHelp();
}

LocalAIAssistant* AIAssistantIntegration::getAssistant() const
{
    return m_assistant;
}

void AIAssistantIntegration::setAssistant(LocalAIAssistant* assistant)
{
    m_assistant = assistant;
    updateAssistantStatus();
}

void AIAssistantIntegration::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
}

void AIAssistantIntegration::setMonetizationManager(MonetizationManager* monetizationManager)
{
    m_monetization = monetizationManager;
    if (!m_accessGate) {
        m_accessGate = new AIAccessGate(m_monetization, this);
    }
}

void AIAssistantIntegration::setEmailService(EmailService* emailService)
{
    m_emailService = emailService;
}

QPushButton* AIAssistantIntegration::getAssistantButton() const
{
    return m_assistantButton;
}

QAction* AIAssistantIntegration::getAssistantAction() const
{
    return m_assistantAction;
}

QMenu* AIAssistantIntegration::getAssistantMenu() const
{
    return m_assistantMenu;
}

void AIAssistantIntegration::setButtonVisible(bool visible)
{
    m_buttonVisible = visible;
    if (m_assistantButton) {
        m_assistantButton->setVisible(visible);
    }
    saveSettings();
}

void AIAssistantIntegration::setButtonPosition(const QString& position)
{
    m_buttonPosition = position;
    positionAssistantButton();
    saveSettings();
}

void AIAssistantIntegration::setTooltipEnabled(bool enabled)
{
    m_tooltipEnabled = enabled;
    if (m_tooltipLabel) {
        m_tooltipLabel->setVisible(false);
    }
    saveSettings();
}

void AIAssistantIntegration::setContextHelpEnabled(bool enabled)
{
    m_contextHelpEnabled = enabled;
    saveSettings();
}

bool AIAssistantIntegration::isButtonVisible() const
{
    return m_buttonVisible;
}

bool AIAssistantIntegration::isTooltipEnabled() const
{
    return m_tooltipEnabled;
}

bool AIAssistantIntegration::isContextHelpEnabled() const
{
    return m_contextHelpEnabled;
}

void AIAssistantIntegration::showAssistant()
{
    if (m_assistant) {
        if (m_accessGate && !m_accessGate->ensureAccess(m_mainWindow, m_walletModel)) {
            return; // user cancelled upgrade
        }
        m_assistant->showAssistantDialog(m_mainWindow);
    } else {
        qWarning() << "AI Assistant not available";
    }
}

void AIAssistantIntegration::showQuickHelp(const QString& topic)
{
    if (m_assistant) {
        m_assistant->showQuickHelp(topic, m_mainWindow);
    }
}

void AIAssistantIntegration::showContextHelp(const QString& context)
{
    if (m_assistant && m_tooltipEnabled) {
        QString helpText = "Need help with " + context + "? Click the 916 button!";
        showTooltip(helpText, QCursor::pos());
    }
}

void AIAssistantIntegration::updateAssistantStatus()
{
    if (m_assistantButton) {
        bool enabled = m_assistant && m_assistant->isEnabled();
        m_assistantButton->setEnabled(enabled);
        
        if (enabled) {
            m_assistantButton->setToolTip("Need help? Ask Shahcoin Assistant!");
        } else {
            m_assistantButton->setToolTip("AI Assistant is disabled");
        }
    }
}

void AIAssistantIntegration::onAssistantButtonClicked()
{
    showAssistant();
}

void AIAssistantIntegration::onAssistantActionTriggered()
{
    showAssistant();
}

void AIAssistantIntegration::onQuickHelpActionTriggered()
{
    // Show quick help menu
    if (m_assistantMenu) {
        QAction* quickHelpAction = m_assistantMenu->actions().at(2); // Quick Help action (adjust if menu changes)
        if (quickHelpAction) {
            QMenu* quickHelpMenu = new QMenu(m_mainWindow);
            
            QStringList topics = {"General", "Wallet", "Staking", "Tokens", "NFTs", "Transactions", "Security", "Fees", "DEX"};
            
            for (const auto& topic : topics) {
                QAction* action = quickHelpMenu->addAction(topic);
                connect(action, &QAction::triggered, [this, topic]() {
                    showQuickHelp(topic);
                });
            }
            
            quickHelpMenu->exec(QCursor::pos());
        }
    }
}

void AIAssistantIntegration::onSettingsActionTriggered()
{
    if (m_assistant) {
        // Show assistant settings dialog
        QMessageBox::information(m_mainWindow, "AI Assistant Settings",
            QString("AI Assistant Settings:\n\n"
                   "• Enabled: %1\n"
                   "• Auto-suggestions: %2\n"
                   "• Similarity threshold: %3\n"
                   "• Max suggestions: %4\n"
                   "• Button visible: %5\n"
                   "• Tooltip enabled: %6")
            .arg(m_assistant->isEnabled() ? "Yes" : "No")
            .arg(m_assistant->isAutoSuggestions() ? "Yes" : "No")
            .arg(m_assistant->getSimilarityThreshold())
            .arg(m_assistant->getMaxSuggestions())
            .arg(m_buttonVisible ? "Yes" : "No")
            .arg(m_tooltipEnabled ? "Yes" : "No"));
    }
}

void AIAssistantIntegration::onAboutActionTriggered()
{
    QMessageBox::about(m_mainWindow, "About Shahcoin AI Assistant",
        "Shahcoin AI Assistant\n\n"
        "A local, intelligent assistant that helps you with:\n"
        "• Wallet setup and management\n"
        "• Staking and mining questions\n"
        "• Token and NFT creation\n"
        "• Transaction guidance\n"
        "• Security best practices\n\n"
        "Works offline with a pre-trained FAQ knowledge base.\n"
        "No internet connection required!");
}

void AIAssistantIntegration::onUpgradeActionTriggered()
{
    if (!m_accessGate) return;
    if (m_monetization && m_monetization->hasPremiumAccess()) {
        QMessageBox::information(m_mainWindow, "AI Wallet", "You already have Premium AI Wallet access.");
        return;
    }
    // Show upgrade dialog directly to capture payment events
    UpgradeAIPanel dlg(m_walletModel, m_monetization, m_mainWindow);
    QObject::connect(&dlg, &UpgradeAIPanel::paymentRecorded, m_mainWindow, [this](const QString& method, double amountUsd, const QString& txn, const QString& plan, const QString& invoiceUrl){
        PaymentHistoryModel::PaymentRecord rec;
        rec.date = QDateTime::currentDateTime();
        rec.amount = amountUsd;
        rec.currency = QStringLiteral("USD");
        rec.method = method;
        rec.transactionId = txn;
        rec.invoiceUrl = invoiceUrl;
        rec.planType = plan;
        PaymentHistoryModel model(m_mainWindow);
        model.addRecord(rec);
        
        // Send email confirmation if enabled
        if (m_emailService && m_emailService->isEmailEnabled()) {
            EmailService::PaymentEmailData emailData;
            emailData.email = "user@example.com"; // TODO: Get from user settings
            emailData.plan = plan;
            emailData.amount = QString::number(amountUsd, 'f', 2) + " USD";
            emailData.method = method;
            emailData.date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
            emailData.transactionId = txn;
            m_emailService->sendPaymentConfirmation(emailData);
        }
    });
    dlg.exec();
}

void AIAssistantIntegration::onBuyShahActionTriggered()
{
    if (!m_accessGate) return;
    (void)m_accessGate->ensureAccess(m_mainWindow, m_walletModel);
}

void AIAssistantIntegration::onAiInsightsActionTriggered()
{
    if (!m_accessGate || !m_accessGate->ensureAccess(m_mainWindow, m_walletModel)) return;
    QMessageBox::information(m_mainWindow, "AI Insights", "Insights panel coming soon (gated by AI Wallet access).\nThis stub confirms gating works.");
}

void AIAssistantIntegration::onAiTroubleshooterActionTriggered()
{
    if (!m_accessGate || !m_accessGate->ensureAccess(m_mainWindow, m_walletModel)) return;
    QMessageBox::information(m_mainWindow, "AI Troubleshooter", "Troubleshooter panel coming soon (gated by AI Wallet access).\nThis stub confirms gating works.");
}

void AIAssistantIntegration::onAiFeeOptimizerActionTriggered()
{
    if (!m_accessGate || !m_accessGate->ensureAccess(m_mainWindow, m_walletModel)) return;
    QMessageBox::information(m_mainWindow, "AI Fee Optimizer", "Fee Optimizer coming soon (gated by AI Wallet access).\nThis stub confirms gating works.");
}

void AIAssistantIntegration::onAiLanguageSettingsActionTriggered()
{
    if (!m_accessGate || !m_accessGate->ensureAccess(m_mainWindow, m_walletModel)) return;
    QMessageBox::information(m_mainWindow, "Multilingual Assistant", "Language settings coming soon (gated by AI Wallet access).\nThis stub confirms gating works.");
}

void AIAssistantIntegration::onTooltipTimeout()
{
    hideTooltip();
}

void AIAssistantIntegration::showTooltip(const QString& message, const QPoint& position)
{
    if (!m_tooltipLabel || !m_tooltipEnabled) {
        return;
    }

    m_tooltipLabel->setText(message);
    m_tooltipLabel->adjustSize();
    
    // Position tooltip near cursor but within window bounds
    QPoint tooltipPos = position;
    QRect windowRect = m_mainWindow->geometry();
    QSize tooltipSize = m_tooltipLabel->size();
    
    if (tooltipPos.x() + tooltipSize.width() > windowRect.width()) {
        tooltipPos.setX(windowRect.width() - tooltipSize.width() - 10);
    }
    if (tooltipPos.y() + tooltipSize.height() > windowRect.height()) {
        tooltipPos.setY(position.y() - tooltipSize.height() - 10);
    }
    
    m_tooltipLabel->move(tooltipPos);
    m_tooltipLabel->setVisible(true);
    m_tooltipLabel->raise();
    
    // Auto-hide after timeout
    m_tooltipTimer->start();
}

void AIAssistantIntegration::hideTooltip()
{
    if (m_tooltipLabel) {
        m_tooltipLabel->setVisible(false);
    }
    m_tooltipTimer->stop();
}

void AIAssistantIntegration::updateTooltipContent()
{
    // Update tooltip content based on current context
    if (m_assistant && m_tooltipEnabled) {
        QString tooltipText = "Need help? Ask Shahcoin Assistant!";
        if (m_assistantButton) {
            m_assistantButton->setToolTip(tooltipText);
        }
    }
}

void AIAssistantIntegration::setupContextHelpForWidget(QWidget* widget, const QString& helpText)
{
    if (!m_contextHelpEnabled || !widget) {
        return;
    }
    
    m_contextHelpMap[widget] = helpText;
    
    // Install event filter for the widget
    widget->installEventFilter(this);
}

void AIAssistantIntegration::showContextHelpForWidget(QWidget* widget)
{
    if (!m_contextHelpEnabled || !widget) {
        return;
    }
    
    if (m_contextHelpMap.contains(widget)) {
        QString helpText = m_contextHelpMap[widget];
        showContextHelp(helpText);
    }
}

bool AIAssistantIntegration::eventFilter(QObject* obj, QEvent* event)
{
    if (!m_contextHelpEnabled) {
        return QObject::eventFilter(obj, event);
    }
    
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        QWidget* widget = qobject_cast<QWidget*>(obj);
        
        if (widget && m_contextHelpMap.contains(widget)) {
            QString helpText = m_contextHelpMap[widget];
            showTooltip(helpText, helpEvent->globalPos());
            return true;
        }
    }
    
    return QObject::eventFilter(obj, event);
}
