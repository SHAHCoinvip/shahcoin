#include "onboardingtour.h"
#include "shahcoingui.h"
#include "guiutil.h"

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QMenu>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QTimer>
#include <QSettings>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>

OnboardingTour::OnboardingTour(QMainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_tooltipWidget(nullptr)
    , m_currentStepIndex(0)
    , m_currentHighlightedWidget(nullptr)
    , m_highlightEffect(nullptr)
    , m_autoAdvanceTimer(nullptr)
    , m_tourInProgress(false)
    , m_dontShowAgain(false)
{
    setupTourSteps();
    createTooltipWidget();
    loadTourProgress();
}

OnboardingTour::~OnboardingTour()
{
    disconnectFromActions();
    if (m_tooltipWidget) {
        m_tooltipWidget->deleteLater();
    }
}

void OnboardingTour::setupTourSteps()
{
    m_tourSteps.clear();
    
    // Welcome step
    m_tourSteps.append({
        tr("Welcome to Shahcoin Wallet!"),
        tr("Let's take a quick tour of your new wallet. We'll show you how to create wallets, stake SHAH, create NFTs, and use ShahSwap."),
        "",
        "",
        "center",
        false
    });

    // Create wallet step
    m_tourSteps.append({
        tr("Create Your Wallet"),
        tr("Start by creating a new wallet or loading an existing one. Click 'File' → 'Create Wallet' to get started."),
        "m_create_wallet_action",
        "File",
        "bottom",
        true
    });

    // Load wallet step
    m_tourSteps.append({
        tr("Load Existing Wallet"),
        tr("If you already have a wallet, you can load it by clicking 'File' → 'Open Wallet'."),
        "m_open_wallet_action",
        "File",
        "bottom",
        true
    });

    // Stake SHAH step
    m_tourSteps.append({
        tr("Stake Your SHAH"),
        tr("Earn rewards by staking your SHAH coins. Navigate to the 'Staking' tab to start earning passive income."),
        "",
        "Staking",
        "bottom",
        false
    });

    // Create NFT step
    m_tourSteps.append({
        tr("Create NFTs"),
        tr("Create unique digital assets! Go to 'NFTs' → 'Create NFT' to mint your own non-fungible tokens."),
        "",
        "NFTs",
        "bottom",
        false
    });

    // Create Token step
    m_tourSteps.append({
        tr("Create Tokens"),
        tr("Launch your own SHI-20 tokens on Shahcoin. Visit 'Tokens' → 'Create SHI-20 Token' to create custom tokens."),
        "",
        "Tokens",
        "bottom",
        false
    });

    // ShahSwap step
    m_tourSteps.append({
        tr("Trade on ShahSwap"),
        tr("Swap tokens and NFTs on our decentralized exchange. Access ShahSwap from the main menu."),
        "",
        "ShahSwap",
        "bottom",
        false
    });

    // Complete step
    m_tourSteps.append({
        tr("Tour Complete!"),
        tr("You're all set! Explore the wallet features and start your Shahcoin journey. You can always access help from the 'Help' menu."),
        "",
        "",
        "center",
        false
    });
}

void OnboardingTour::createTooltipWidget()
{
    m_tooltipWidget = new TourTooltipWidget(m_mainWindow);
    m_tooltipWidget->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    m_tooltipWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_tooltipWidget->setAttribute(Qt::WA_ShowWithoutActivating);

    QVBoxLayout* layout = new QVBoxLayout(m_tooltipWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // Title
    m_tooltipTitle = new QLabel(m_tooltipWidget);
    m_tooltipTitle->setStyleSheet("QLabel { color: #2E86AB; font-size: 16px; font-weight: bold; }");
    m_tooltipTitle->setWordWrap(true);
    layout->addWidget(m_tooltipTitle);

    // Description
    m_tooltipDescription = new QLabel(m_tooltipWidget);
    m_tooltipDescription->setStyleSheet("QLabel { color: #333333; font-size: 14px; }");
    m_tooltipDescription->setWordWrap(true);
    layout->addWidget(m_tooltipDescription);

    // Buttons layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    m_previousButton = new QPushButton(tr("Previous"), m_tooltipWidget);
    m_previousButton->setStyleSheet(
        "QPushButton { background-color: #6C757D; color: white; border: none; padding: 8px 16px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #5A6268; }"
        "QPushButton:disabled { background-color: #CCCCCC; }"
    );
    m_previousButton->setEnabled(false);

    m_nextButton = new QPushButton(tr("Next"), m_tooltipWidget);
    m_nextButton->setStyleSheet(
        "QPushButton { background-color: #2E86AB; color: white; border: none; padding: 8px 16px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #1E6B8B; }"
    );

    m_skipButton = new QPushButton(tr("Skip Tour"), m_tooltipWidget);
    m_skipButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #6C757D; border: 1px solid #6C757D; padding: 8px 16px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #6C757D; color: white; }"
    );

    buttonLayout->addWidget(m_previousButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_skipButton);
    buttonLayout->addWidget(m_nextButton);

    layout->addLayout(buttonLayout);

    // Don't show again checkbox
    m_dontShowAgainCheckbox = new QCheckBox(tr("Don't show this tour again"), m_tooltipWidget);
    m_dontShowAgainCheckbox->setStyleSheet("QCheckBox { color: #6C757D; font-size: 12px; }");
    layout->addWidget(m_dontShowAgainCheckbox);

    // Connect signals
    connect(m_previousButton, &QPushButton::clicked, this, &OnboardingTour::onPreviousStep);
    connect(m_nextButton, &QPushButton::clicked, this, &OnboardingTour::onNextStep);
    connect(m_skipButton, &QPushButton::clicked, this, &OnboardingTour::onSkipTour);
    connect(m_dontShowAgainCheckbox, &QCheckBox::toggled, this, &OnboardingTour::onDontShowAgainToggled);

    // Setup effects
    m_tooltipOpacity = new QGraphicsOpacityEffect(m_tooltipWidget);
    m_tooltipWidget->setGraphicsEffect(m_tooltipOpacity);
    m_tooltipOpacity->setOpacity(0.0);

    m_tooltipShadow = new QGraphicsDropShadowEffect(m_tooltipWidget);
    m_tooltipShadow->setBlurRadius(20);
    m_tooltipShadow->setColor(QColor(0, 0, 0, 80));
    m_tooltipShadow->setOffset(0, 4);

    // Animation
    m_tooltipAnimation = new QPropertyAnimation(m_tooltipOpacity, "opacity", this);
    m_tooltipAnimation->setDuration(300);
    m_tooltipAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // Auto-advance timer
    m_autoAdvanceTimer = new QTimer(this);
    m_autoAdvanceTimer->setSingleShot(true);
    connect(m_autoAdvanceTimer, &QTimer::timeout, this, &OnboardingTour::onNextStep);
}

void OnboardingTour::startTour()
{
    if (m_tourInProgress) return;
    
    m_tourInProgress = true;
    m_currentStepIndex = 0;
    connectToActions();
    showStep(static_cast<TourStep>(m_currentStepIndex));
}

void OnboardingTour::showStep(TourStep step)
{
    if (step < 0 || step >= m_tourSteps.size()) return;

    const TourStepData& stepData = m_tourSteps[step];
    
    // Remove previous highlight
    removeHighlight();
    
    // Update tooltip content
    m_tooltipTitle->setText(stepData.title);
    m_tooltipDescription->setText(stepData.description);
    
    // Update button states
    m_previousButton->setEnabled(step > Welcome);
    if (step == Complete) {
        m_nextButton->setText(tr("Finish"));
        m_skipButton->setVisible(false);
    } else {
        m_nextButton->setText(tr("Next"));
        m_skipButton->setVisible(true);
    }
    
    // Position tooltip
    QPoint position;
    if (stepData.requiresAction && !stepData.actionName.isEmpty()) {
        // Find the target widget/action
        QWidget* targetWidget = nullptr;
        if (ShahcoinGUI* gui = qobject_cast<ShahcoinGUI*>(m_mainWindow)) {
            // Try to find the action
            QAction* action = gui->findChild<QAction*>(stepData.actionName);
            if (action && action->parentWidget()) {
                targetWidget = action->parentWidget();
            }
        }
        
        if (targetWidget) {
            position = calculateTooltipPosition(targetWidget, stepData.position);
            highlightWidget(targetWidget);
        } else {
            position = calculateTooltipPosition(nullptr, "center");
        }
    } else {
        position = calculateTooltipPosition(nullptr, stepData.position);
    }
    
    // Show tooltip with animation
    m_tooltipWidget->move(position);
    m_tooltipWidget->show();
    
    m_tooltipAnimation->setStartValue(0.0);
    m_tooltipAnimation->setEndValue(1.0);
    m_tooltipAnimation->start();
    
    // Auto-advance for non-interactive steps
    if (!stepData.requiresAction && step != Complete) {
        m_autoAdvanceTimer->start(5000); // 5 seconds
    } else {
        m_autoAdvanceTimer->stop();
    }
}

void OnboardingTour::onNextStep()
{
    m_autoAdvanceTimer->stop();
    
    if (m_currentStepIndex < m_tourSteps.size() - 1) {
        m_currentStepIndex++;
        showStep(static_cast<TourStep>(m_currentStepIndex));
    } else {
        onCompleteTour();
    }
}

void OnboardingTour::onPreviousStep()
{
    m_autoAdvanceTimer->stop();
    
    if (m_currentStepIndex > 0) {
        m_currentStepIndex--;
        showStep(static_cast<TourStep>(m_currentStepIndex));
    }
}

void OnboardingTour::onSkipTour()
{
    m_autoAdvanceTimer->stop();
    hideTooltip();
    removeHighlight();
    disconnectFromActions();
    m_tourInProgress = false;
    
    if (m_dontShowAgain) {
        setTourCompleted();
    }
    
    emit tourSkipped();
}

void OnboardingTour::onCompleteTour()
{
    m_autoAdvanceTimer->stop();
    hideTooltip();
    removeHighlight();
    disconnectFromActions();
    m_tourInProgress = false;
    
    if (m_dontShowAgain) {
        setTourCompleted();
    }
    
    emit tourCompleted();
}

void OnboardingTour::onDontShowAgainToggled(bool checked)
{
    m_dontShowAgain = checked;
}

void OnboardingTour::onActionTriggered()
{
    // Auto-advance when user performs the suggested action
    QTimer::singleShot(1000, this, &OnboardingTour::onNextStep);
}

void OnboardingTour::onMenuAboutToShow()
{
    // Auto-advance when user opens the suggested menu
    QTimer::singleShot(500, this, &OnboardingTour::onNextStep);
}

void OnboardingTour::showTooltip(const QString& title, const QString& description, 
                                const QPoint& position, QWidget* targetWidget)
{
    m_tooltipTitle->setText(title);
    m_tooltipDescription->setText(description);
    m_tooltipWidget->move(position);
    m_tooltipWidget->show();
    
    if (targetWidget) {
        highlightWidget(targetWidget);
    }
}

void OnboardingTour::hideTooltip()
{
    if (m_tooltipWidget) {
        m_tooltipWidget->hide();
    }
}

void OnboardingTour::highlightWidget(QWidget* widget)
{
    if (!widget) return;
    
    removeHighlight();
    
    m_currentHighlightedWidget = widget;
    m_highlightEffect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(m_highlightEffect);
    
    // Create highlight animation
    QPropertyAnimation* highlightAnim = new QPropertyAnimation(m_highlightEffect, "opacity", this);
    highlightAnim->setDuration(1000);
    highlightAnim->setStartValue(1.0);
    highlightAnim->setEndValue(0.7);
    highlightAnim->setLoopCount(-1); // Infinite loop
    highlightAnim->setEasingCurve(QEasingCurve::InOutQuad);
    highlightAnim->start();
}

void OnboardingTour::removeHighlight()
{
    if (m_currentHighlightedWidget && m_highlightEffect) {
        m_currentHighlightedWidget->setGraphicsEffect(nullptr);
        m_currentHighlightedWidget = nullptr;
        m_highlightEffect = nullptr;
    }
}

void OnboardingTour::connectToActions()
{
    if (!m_mainWindow) return;
    
    disconnectFromActions();
    
    // Connect to menu actions
    if (QMenuBar* menuBar = m_mainWindow->menuBar()) {
        for (QAction* action : menuBar->actions()) {
            if (QMenu* menu = action->menu()) {
                connect(menu, &QMenu::aboutToShow, this, &OnboardingTour::onMenuAboutToShow);
                m_connectedMenus.append(menu);
            }
        }
    }
    
    // Connect to specific actions
    if (ShahcoinGUI* gui = qobject_cast<ShahcoinGUI*>(m_mainWindow)) {
        // Find and connect to specific actions
        QList<QAction*> actions = gui->findChildren<QAction*>();
        for (QAction* action : actions) {
            if (action->objectName().contains("create_wallet") || 
                action->objectName().contains("open_wallet")) {
                connect(action, &QAction::triggered, this, &OnboardingTour::onActionTriggered);
                m_connectedActions.append(action);
            }
        }
    }
}

void OnboardingTour::disconnectFromActions()
{
    for (QAction* action : m_connectedActions) {
        disconnect(action, nullptr, this, nullptr);
    }
    m_connectedActions.clear();
    
    for (QMenu* menu : m_connectedMenus) {
        disconnect(menu, nullptr, this, nullptr);
    }
    m_connectedMenus.clear();
}

QPoint OnboardingTour::calculateTooltipPosition(QWidget* targetWidget, const QString& position)
{
    QPoint pos;
    
    if (targetWidget) {
        QRect widgetRect = targetWidget->mapToGlobal(targetWidget->rect());
        
        if (position == "top") {
            pos = QPoint(widgetRect.center().x(), widgetRect.top() - 20);
        } else if (position == "bottom") {
            pos = QPoint(widgetRect.center().x(), widgetRect.bottom() + 20);
        } else if (position == "left") {
            pos = QPoint(widgetRect.left() - 20, widgetRect.center().y());
        } else if (position == "right") {
            pos = QPoint(widgetRect.right() + 20, widgetRect.center().y());
        } else {
            pos = widgetRect.center();
        }
    } else {
        // Center on screen
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->geometry();
            pos = screenGeometry.center();
        }
    }
    
    // Adjust for tooltip size
    if (m_tooltipWidget) {
        pos.setX(pos.x() - m_tooltipWidget->width() / 2);
        pos.setY(pos.y() - m_tooltipWidget->height() / 2);
    }
    
    return pos;
}

bool OnboardingTour::shouldShowTour() const
{
    QSettings settings;
    return !settings.value("onboarding/tourCompleted", false).toBool();
}

void OnboardingTour::setTourCompleted()
{
    QSettings settings;
    settings.setValue("onboarding/tourCompleted", true);
    settings.setValue("onboarding/dontShowAgain", m_dontShowAgain);
}

void OnboardingTour::saveTourProgress()
{
    QSettings settings;
    settings.setValue("onboarding/currentStep", m_currentStepIndex);
}

void OnboardingTour::loadTourProgress()
{
    QSettings settings;
    m_currentStepIndex = settings.value("onboarding/currentStep", 0).toInt();
    m_dontShowAgain = settings.value("onboarding/dontShowAgain", false).toBool();
}

// TourTooltipWidget implementation
TourTooltipWidget::TourTooltipWidget(QWidget* parent)
    : QWidget(parent)
    , m_cornerRadius(8)
    , m_borderWidth(1)
    , m_padding(20)
    , m_arrowSize(10)
    , m_arrowOffset(0)
{
    m_backgroundColor = QColor(255, 255, 255);
    m_borderColor = QColor(46, 134, 171);
    m_textColor = QColor(51, 51, 51);
    m_titleColor = QColor(46, 134, 171);
    
    setFixedSize(400, 200);
}

void TourTooltipWidget::setTitle(const QString& title)
{
    m_title = title;
    update();
}

void TourTooltipWidget::setDescription(const QString& description)
{
    m_description = description;
    update();
}

void TourTooltipWidget::setPosition(const QString& position)
{
    m_position = position;
    update();
}

void TourTooltipWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Create rounded rectangle path
    QPainterPath path;
    QRect rect = this->rect().adjusted(m_borderWidth, m_borderWidth, -m_borderWidth, -m_borderWidth);
    path.addRoundedRect(rect, m_cornerRadius, m_cornerRadius);
    
    // Draw background
    painter.fillPath(path, m_backgroundColor);
    
    // Draw border
    painter.setPen(QPen(m_borderColor, m_borderWidth));
    painter.drawPath(path);
    
    // Draw arrow if needed
    if (m_position != "center") {
        QPainterPath arrowPath;
        QPoint arrowBase;
        QPoint arrowTip;
        
        if (m_position == "top") {
            arrowBase = QPoint(width() / 2 + m_arrowOffset, height() - m_borderWidth);
            arrowTip = QPoint(arrowBase.x(), arrowBase.y() + m_arrowSize);
        } else if (m_position == "bottom") {
            arrowBase = QPoint(width() / 2 + m_arrowOffset, m_borderWidth);
            arrowTip = QPoint(arrowBase.x(), arrowBase.y() - m_arrowSize);
        } else if (m_position == "left") {
            arrowBase = QPoint(width() - m_borderWidth, height() / 2 + m_arrowOffset);
            arrowTip = QPoint(arrowBase.x() + m_arrowSize, arrowBase.y());
        } else if (m_position == "right") {
            arrowBase = QPoint(m_borderWidth, height() / 2 + m_arrowOffset);
            arrowTip = QPoint(arrowBase.x() - m_arrowSize, arrowBase.y());
        }
        
        if (arrowBase != arrowTip) {
            arrowPath.moveTo(arrowBase);
            arrowPath.lineTo(arrowTip);
            arrowPath.lineTo(QPoint(arrowTip.x() - 5, arrowTip.y() - 5));
            arrowPath.lineTo(QPoint(arrowTip.x() + 5, arrowTip.y() - 5));
            arrowPath.closeSubpath();
            
            painter.fillPath(arrowPath, m_backgroundColor);
            painter.setPen(QPen(m_borderColor, m_borderWidth));
            painter.drawPath(arrowPath);
        }
    }
}

void TourTooltipWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    raise();
}

void TourTooltipWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
}
