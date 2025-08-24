#include "wizardai.h"
#include "walletmodel.h"
#include "platformstyle.h"
#include "guiutil.h"
#include "shahcoinunits.h"
#include "interfaces/wallet.h"
#include "util/translation.h"

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QScrollBar>
#include <QStyle>
#include <QScreen>
#include <QPainter>
#include <QFontMetrics>
#include <QStandardPaths>
#include <QJsonArray>
#include <QInputDialog>
#include <QToolTip>
#include <QMenu>
#include <QAction>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QBuffer>
#include <QPainter>
#include <QPen>
#include <QCryptographicHash>

WizardAI::WizardAI(WalletModel* walletModel, QWidget* parent)
    : QDialog(parent)
    , m_walletModel(walletModel)
    , m_platformStyle(nullptr)
    , m_analysisTimer(new QTimer(this))
    , m_currentStep(Welcome)
    , m_wizardCompleted(false)
    , m_autoAnalysis(true)
    , m_analysisInProgress(false)
    , m_showDetails(false)
    , m_stepAnimation(nullptr)
    , m_opacityEffect(nullptr)
    , m_shadowEffect(nullptr)
    , m_settings(new QSettings("Shahcoin", "WizardAI"))
    , m_wizardRunCount(0)
{
    setWindowTitle(tr("🧠 Shah Wallet Wizard"));
    setModal(true);
    resize(800, 600);
    
    setupUI();
    connectSignals();
    applyTheme();
    
    // Load wizard progress
    loadWizardProgress();
    
    // Setup analysis timer
    connect(m_analysisTimer, &QTimer::timeout, this, &WizardAI::onAnalysisComplete);
    
    // Setup animations
    m_stepAnimation = new QPropertyAnimation(this, "geometry");
    m_stepAnimation->setDuration(ANIMATION_DURATION_MS);
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    
    // Check if wizard should auto-start
    if (!m_wizardCompleted && m_autoAnalysis) {
        QTimer::singleShot(500, this, &WizardAI::startWizard);
    }
}

WizardAI::~WizardAI()
{
    saveWizardProgress();
}

void WizardAI::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Create stacked widget for steps
    m_stepStack = new QStackedWidget(this);
    m_mainLayout->addWidget(m_stepStack);
    
    setupWelcomeStep();
    setupAnalysisStep();
    setupSecurityStep();
    setupTestTransactionStep();
    setupStakingStep();
    setupBackupStep();
    setupCompletionStep();
    setupNavigation();
    setupHelpDialog();
}

void WizardAI::setupWelcomeStep()
{
    m_welcomeStep = new QWidget();
    m_welcomeLayout = new QVBoxLayout(m_welcomeStep);
    m_welcomeLayout->setContentsMargins(0, 0, 0, 0);
    m_welcomeLayout->setSpacing(20);
    
    // Welcome title and icon
    m_welcomeTitle = new QLabel(tr("🧠 Welcome to Shah Wallet Wizard"));
    m_welcomeTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2E7D32;");
    m_welcomeTitle->setAlignment(Qt::AlignCenter);
    
    m_welcomeSubtitle = new QLabel(tr("Your AI-powered guide to secure and efficient wallet management"));
    m_welcomeSubtitle->setStyleSheet("font-size: 14px; color: #666;");
    m_welcomeSubtitle->setAlignment(Qt::AlignCenter);
    
    m_welcomeIcon = new QLabel("🧠");
    m_welcomeIcon->setStyleSheet("font-size: 64px;");
    m_welcomeIcon->setAlignment(Qt::AlignCenter);
    
    // Welcome description
    m_welcomeDescription = new QTextEdit();
    m_welcomeDescription->setHtml(tr(
        "<h3>What the Wizard will help you with:</h3>"
        "<ul>"
        "<li><b>🔍 Wallet Analysis:</b> Analyze your current wallet state and identify areas for improvement</li>"
        "<li><b>🔒 Security Setup:</b> Get personalized security recommendations</li>"
        "<li><b>🧪 Test Transactions:</b> Learn how to send your first transaction safely</li>"
        "<li><b>💰 Staking Setup:</b> Configure staking to earn rewards</li>"
        "<li><b>💾 Backup Reminder:</b> Ensure your wallet is properly backed up</li>"
        "</ul>"
        "<p><i>The wizard will analyze your wallet and provide personalized recommendations based on your current setup.</i></p>"
    ));
    m_welcomeDescription->setReadOnly(true);
    m_welcomeDescription->setMaximumHeight(200);
    
    // Auto-analysis checkbox
    m_autoAnalysisCheck = new QCheckBox(tr("Automatically analyze my wallet and provide recommendations"));
    m_autoAnalysisCheck->setChecked(m_autoAnalysis);
    
    // Next button
    m_welcomeNextButton = new QPushButton(tr("Start Wizard"));
    m_welcomeNextButton->setMinimumHeight(40);
    m_welcomeNextButton->setStyleSheet("font-size: 16px; font-weight: bold;");
    
    m_welcomeLayout->addWidget(m_welcomeIcon);
    m_welcomeLayout->addWidget(m_welcomeTitle);
    m_welcomeLayout->addWidget(m_welcomeSubtitle);
    m_welcomeLayout->addWidget(m_welcomeDescription);
    m_welcomeLayout->addWidget(m_autoAnalysisCheck);
    m_welcomeLayout->addStretch();
    m_welcomeLayout->addWidget(m_welcomeNextButton);
    
    m_stepStack->addWidget(m_welcomeStep);
}

void WizardAI::setupAnalysisStep()
{
    m_analysisStep = new QWidget();
    m_analysisLayout = new QVBoxLayout(m_analysisStep);
    m_analysisLayout->setContentsMargins(0, 0, 0, 0);
    m_analysisLayout->setSpacing(20);
    
    // Analysis title
    m_analysisTitle = new QLabel(tr("🔍 Analyzing Your Wallet"));
    m_analysisTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #1976D2;");
    m_analysisTitle->setAlignment(Qt::AlignCenter);
    
    // Progress bar
    m_analysisProgress = new QProgressBar();
    m_analysisProgress->setRange(0, 100);
    m_analysisProgress->setValue(0);
    m_analysisProgress->setTextVisible(true);
    m_analysisProgress->setFormat(tr("Analyzing... %p%"));
    
    // Status label
    m_analysisStatus = new QLabel(tr("Initializing analysis..."));
    m_analysisStatus->setAlignment(Qt::AlignCenter);
    m_analysisStatus->setStyleSheet("font-size: 14px; color: #666;");
    
    // Results text area
    m_analysisResults = new QTextEdit();
    m_analysisResults->setPlaceholderText(tr("Analysis results will appear here..."));
    m_analysisResults->setReadOnly(true);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_analysisNextButton = new QPushButton(tr("Continue"));
    m_analysisNextButton->setEnabled(false);
    m_analysisRetryButton = new QPushButton(tr("Retry Analysis"));
    m_analysisRetryButton->setEnabled(false);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_analysisRetryButton);
    buttonLayout->addWidget(m_analysisNextButton);
    
    m_analysisLayout->addWidget(m_analysisTitle);
    m_analysisLayout->addWidget(m_analysisProgress);
    m_analysisLayout->addWidget(m_analysisStatus);
    m_analysisLayout->addWidget(m_analysisResults, 1);
    m_analysisLayout->addLayout(buttonLayout);
    
    m_stepStack->addWidget(m_analysisStep);
}

void WizardAI::setupSecurityStep()
{
    m_securityStep = new QWidget();
    m_securityLayout = new QVBoxLayout(m_securityStep);
    m_securityLayout->setContentsMargins(0, 0, 0, 0);
    m_securityLayout->setSpacing(20);
    
    // Security title
    m_securityTitle = new QLabel(tr("🔒 Security Recommendations"));
    m_securityTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #D32F2F;");
    m_securityTitle->setAlignment(Qt::AlignCenter);
    
    // Security recommendations group
    m_securityRecommendations = new QGroupBox(tr("Personalized Security Tips"));
    m_securityRecLayout = new QVBoxLayout(m_securityRecommendations);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_securityNextButton = new QPushButton(tr("Continue"));
    m_securitySkipButton = new QPushButton(tr("Skip Security"));
    
    buttonLayout->addWidget(m_securitySkipButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_securityNextButton);
    
    m_securityLayout->addWidget(m_securityTitle);
    m_securityLayout->addWidget(m_securityRecommendations, 1);
    m_securityLayout->addLayout(buttonLayout);
    
    m_stepStack->addWidget(m_securityStep);
}

void WizardAI::setupTestTransactionStep()
{
    m_testStep = new QWidget();
    m_testLayout = new QVBoxLayout(m_testStep);
    m_testLayout->setContentsMargins(0, 0, 0, 0);
    m_testLayout->setSpacing(20);
    
    // Test transaction title
    m_testTitle = new QLabel(tr("🧪 Test Transaction"));
    m_testTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #FF8F00;");
    m_testTitle->setAlignment(Qt::AlignCenter);
    
    // Description
    m_testDescription = new QTextEdit();
    m_testDescription->setHtml(tr(
        "<h3>Learn to Send Your First Transaction</h3>"
        "<p>Before sending real transactions, it's recommended to practice with a small test transaction. "
        "This helps you understand the process and ensures everything is set up correctly.</p>"
        "<p><b>Current Balance:</b> %1 SHAH</p>"
        "<p><b>Minimum for Test:</b> 1 SHAH</p>"
    ).arg(ShahcoinUnits::format(ShahcoinUnit::SHAH, getWalletBalance())));
    m_testDescription->setReadOnly(true);
    m_testDescription->setMaximumHeight(150);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_testSendButton = new QPushButton(tr("Send Test Transaction"));
    m_testSkipButton = new QPushButton(tr("Skip Test"));
    m_testNextButton = new QPushButton(tr("Continue"));
    
    buttonLayout->addWidget(m_testSkipButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_testSendButton);
    buttonLayout->addWidget(m_testNextButton);
    
    m_testLayout->addWidget(m_testTitle);
    m_testLayout->addWidget(m_testDescription);
    m_testLayout->addLayout(buttonLayout);
    
    m_stepStack->addWidget(m_testStep);
}

void WizardAI::setupStakingStep()
{
    m_stakingStep = new QWidget();
    m_stakingLayout = new QVBoxLayout(m_stakingStep);
    m_stakingLayout->setContentsMargins(0, 0, 0, 0);
    m_stakingLayout->setSpacing(20);
    
    // Staking title
    m_stakingTitle = new QLabel(tr("💰 Staking Setup"));
    m_stakingTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #388E3C;");
    m_stakingTitle->setAlignment(Qt::AlignCenter);
    
    // Staking recommendations group
    m_stakingRecommendations = new QGroupBox(tr("Staking Recommendations"));
    m_stakingRecLayout = new QVBoxLayout(m_stakingRecommendations);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_stakingSetupButton = new QPushButton(tr("Setup Staking"));
    m_stakingSkipButton = new QPushButton(tr("Skip Staking"));
    m_stakingNextButton = new QPushButton(tr("Continue"));
    
    buttonLayout->addWidget(m_stakingSkipButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_stakingSetupButton);
    buttonLayout->addWidget(m_stakingNextButton);
    
    m_stakingLayout->addWidget(m_stakingTitle);
    m_stakingLayout->addWidget(m_stakingRecommendations, 1);
    m_stakingLayout->addLayout(buttonLayout);
    
    m_stepStack->addWidget(m_stakingStep);
}

void WizardAI::setupBackupStep()
{
    m_backupStep = new QWidget();
    m_backupLayout = new QVBoxLayout(m_backupStep);
    m_backupLayout->setContentsMargins(0, 0, 0, 0);
    m_backupLayout->setSpacing(20);
    
    // Backup title
    m_backupTitle = new QLabel(tr("💾 Backup Your Wallet"));
    m_backupTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #7B1FA2;");
    m_backupTitle->setAlignment(Qt::AlignCenter);
    
    // Description
    m_backupDescription = new QTextEdit();
    m_backupDescription->setHtml(tr(
        "<h3>Secure Your Digital Assets</h3>"
        "<p>Creating a backup is crucial for protecting your wallet. A backup allows you to restore your wallet "
        "if your device is lost, stolen, or damaged.</p>"
        "<p><b>What's included in the backup:</b></p>"
        "<ul>"
        "<li>All wallet addresses</li>"
        "<li>Transaction history</li>"
        "<li>Staking configuration</li>"
        "<li>Wallet settings</li>"
        "</ul>"
        "<p><i>Your backup will be encrypted with a password you choose.</i></p>"
    ));
    m_backupDescription->setReadOnly(true);
    m_backupDescription->setMaximumHeight(200);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_backupNowButton = new QPushButton(tr("Create Backup Now"));
    m_backupLaterButton = new QPushButton(tr("Remind Me Later"));
    m_backupNextButton = new QPushButton(tr("Continue"));
    
    buttonLayout->addWidget(m_backupLaterButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_backupNowButton);
    buttonLayout->addWidget(m_backupNextButton);
    
    m_backupLayout->addWidget(m_backupTitle);
    m_backupLayout->addWidget(m_backupDescription);
    m_backupLayout->addLayout(buttonLayout);
    
    m_stepStack->addWidget(m_backupStep);
}

void WizardAI::setupCompletionStep()
{
    m_completionStep = new QWidget();
    m_completionLayout = new QVBoxLayout(m_completionStep);
    m_completionLayout->setContentsMargins(0, 0, 0, 0);
    m_completionLayout->setSpacing(20);
    
    // Completion title and icon
    m_completionTitle = new QLabel(tr("🎉 Wizard Complete!"));
    m_completionTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2E7D32;");
    m_completionTitle->setAlignment(Qt::AlignCenter);
    
    m_completionIcon = new QLabel("🎉");
    m_completionIcon->setStyleSheet("font-size: 64px;");
    m_completionIcon->setAlignment(Qt::AlignCenter);
    
    // Summary
    m_completionSummary = new QTextEdit();
    m_completionSummary->setHtml(tr(
        "<h3>Congratulations!</h3>"
        "<p>You've successfully completed the Shah Wallet Wizard. Your wallet is now optimized for security and efficiency.</p>"
        "<h4>What we accomplished:</h4>"
        "<ul>"
        "<li>✅ Analyzed your wallet configuration</li>"
        "<li>✅ Provided security recommendations</li>"
        "<li>✅ Guided you through test transactions</li>"
        "<li>✅ Set up staking for rewards</li>"
        "<li>✅ Ensured proper backup procedures</li>"
        "</ul>"
        "<p><b>Next Steps:</b></p>"
        "<ul>"
        "<li>Explore the wallet features</li>"
        "<li>Start staking to earn rewards</li>"
        "<li>Keep your backup secure</li>"
        "<li>Run the wizard again anytime from Help menu</li>"
        "</ul>"
    ));
    m_completionSummary->setReadOnly(true);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_completionFinishButton = new QPushButton(tr("Finish"));
    m_completionRestartButton = new QPushButton(tr("Run Wizard Again"));
    
    buttonLayout->addWidget(m_completionRestartButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_completionFinishButton);
    
    m_completionLayout->addWidget(m_completionIcon);
    m_completionLayout->addWidget(m_completionTitle);
    m_completionLayout->addWidget(m_completionSummary, 1);
    m_completionLayout->addLayout(buttonLayout);
    
    m_stepStack->addWidget(m_completionStep);
}

void WizardAI::setupNavigation()
{
    m_navigationLayout = new QHBoxLayout();
    
    m_previousButton = new QPushButton(tr("← Previous"));
    m_nextButton = new QPushButton(tr("Next →"));
    m_skipButton = new QPushButton(tr("Skip"));
    m_helpButton = new QPushButton(tr("?"));
    m_helpButton->setMaximumWidth(40);
    
    m_stepIndicator = new QLabel();
    m_stepIndicator->setAlignment(Qt::AlignCenter);
    m_stepIndicator->setStyleSheet("font-size: 12px; color: #666;");
    
    m_navigationLayout->addWidget(m_previousButton);
    m_navigationLayout->addStretch();
    m_navigationLayout->addWidget(m_stepIndicator);
    m_navigationLayout->addStretch();
    m_navigationLayout->addWidget(m_helpButton);
    m_navigationLayout->addWidget(m_skipButton);
    m_navigationLayout->addWidget(m_nextButton);
    
    m_mainLayout->addLayout(m_navigationLayout);
}

void WizardAI::setupHelpDialog()
{
    m_helpDialog = new QDialog(this);
    m_helpDialog->setWindowTitle(tr("Wizard Help"));
    m_helpDialog->setModal(true);
    m_helpDialog->setFixedSize(500, 400);
    
    m_helpLayout = new QVBoxLayout(m_helpDialog);
    
    m_helpTitle = new QLabel(tr("🧠 Shah Wallet Wizard Help"));
    m_helpTitle->setStyleSheet("font-size: 16px; font-weight: bold;");
    
    m_helpContent = new QTextEdit();
    m_helpContent->setHtml(tr(
        "<h3>About the Wizard</h3>"
        "<p>The Shah Wallet Wizard is an AI-powered assistant that helps you optimize your wallet for security and efficiency.</p>"
        "<h3>Steps Overview</h3>"
        "<ul>"
        "<li><b>Welcome:</b> Introduction and setup</li>"
        "<li><b>Analysis:</b> AI analysis of your wallet state</li>"
        "<li><b>Security:</b> Personalized security recommendations</li>"
        "<li><b>Test Transaction:</b> Learn to send transactions safely</li>"
        "<li><b>Staking:</b> Setup staking to earn rewards</li>"
        "<li><b>Backup:</b> Ensure your wallet is backed up</li>"
        "<li><b>Completion:</b> Summary and next steps</li>"
        "</ul>"
        "<h3>Tips</h3>"
        "<ul>"
        "<li>You can skip any step if you're not ready</li>"
        "<li>The wizard can be run again anytime</li>"
        "<li>All recommendations are personalized to your wallet</li>"
        "<li>No sensitive data is transmitted during analysis</li>"
        "</ul>"
    ));
    m_helpContent->setReadOnly(true);
    
    m_helpCloseButton = new QPushButton(tr("Close"));
    
    m_helpLayout->addWidget(m_helpTitle);
    m_helpLayout->addWidget(m_helpContent, 1);
    m_helpLayout->addWidget(m_helpCloseButton);
}

void WizardAI::connectSignals()
{
    // Welcome step
    connect(m_welcomeNextButton, &QPushButton::clicked, this, &WizardAI::onNextClicked);
    connect(m_autoAnalysisCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_autoAnalysis = checked;
    });
    
    // Analysis step
    connect(m_analysisNextButton, &QPushButton::clicked, this, &WizardAI::onNextClicked);
    connect(m_analysisRetryButton, &QPushButton::clicked, this, &WizardAI::onRetryAnalysis);
    
    // Security step
    connect(m_securityNextButton, &QPushButton::clicked, this, &WizardAI::onNextClicked);
    connect(m_securitySkipButton, &QPushButton::clicked, this, &WizardAI::onSkipClicked);
    
    // Test transaction step
    connect(m_testNextButton, &QPushButton::clicked, this, &WizardAI::onNextClicked);
    connect(m_testSkipButton, &QPushButton::clicked, this, &WizardAI::onSkipClicked);
    connect(m_testSendButton, &QPushButton::clicked, this, [this]() {
        // TODO: Open send coins dialog
        showSuccess(tr("Send coins dialog will open here. This is a placeholder for the actual implementation."));
    });
    
    // Staking step
    connect(m_stakingNextButton, &QPushButton::clicked, this, &WizardAI::onNextClicked);
    connect(m_stakingSkipButton, &QPushButton::clicked, this, &WizardAI::onSkipClicked);
    connect(m_stakingSetupButton, &QPushButton::clicked, this, [this]() {
        // TODO: Open staking setup
        showSuccess(tr("Staking setup will open here. This is a placeholder for the actual implementation."));
    });
    
    // Backup step
    connect(m_backupNextButton, &QPushButton::clicked, this, &WizardAI::onNextClicked);
    connect(m_backupLaterButton, &QPushButton::clicked, this, &WizardAI::onSkipClicked);
    connect(m_backupNowButton, &QPushButton::clicked, this, [this]() {
        // TODO: Open vault manager
        showSuccess(tr("Vault manager will open here. This is a placeholder for the actual implementation."));
    });
    
    // Completion step
    connect(m_completionFinishButton, &QPushButton::clicked, this, &WizardAI::onCompleteClicked);
    connect(m_completionRestartButton, &QPushButton::clicked, this, [this]() {
        m_currentStep = Welcome;
        showStep(Welcome);
        startWizard();
    });
    
    // Navigation
    connect(m_previousButton, &QPushButton::clicked, this, &WizardAI::onPreviousClicked);
    connect(m_nextButton, &QPushButton::clicked, this, &WizardAI::onNextClicked);
    connect(m_skipButton, &QPushButton::clicked, this, &WizardAI::onSkipClicked);
    connect(m_helpButton, &QPushButton::clicked, this, &WizardAI::onHelpClicked);
    
    // Help dialog
    connect(m_helpCloseButton, &QPushButton::clicked, this, [this]() {
        m_helpDialog->accept();
    });
    
    // Step stack
    connect(m_stepStack, QOverload<int>::of(&QStackedWidget::currentChanged), this, &WizardAI::onStepChanged);
}

void WizardAI::applyTheme()
{
    // Apply consistent styling
    QString style = R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:disabled {
            background-color: #cccccc;
        }
        QProgressBar {
            border: 2px solid grey;
            border-radius: 5px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #4CAF50;
            border-radius: 3px;
        }
        QTextEdit {
            border: 1px solid #cccccc;
            border-radius: 4px;
            padding: 8px;
        }
    )";
    
    setStyleSheet(style);
}

void WizardAI::startWizard()
{
    m_wizardRunCount++;
    showStep(Welcome);
    updateNavigationButtons();
    show();
    raise();
    activateWindow();
}

void WizardAI::showStep(WizardStep step)
{
    m_currentStep = step;
    m_stepStack->setCurrentIndex(static_cast<int>(step));
    updateNavigationButtons();
    updateStepIndicator();
    
    // Auto-start analysis if needed
    if (step == WalletAnalysis && m_autoAnalysis) {
        performWalletAnalysis();
    }
}

void WizardAI::nextStep()
{
    WizardStep nextStep = static_cast<WizardStep>(static_cast<int>(m_currentStep) + 1);
    if (nextStep <= Completion) {
        markStepCompleted(m_currentStep);
        showStep(nextStep);
    }
}

void WizardAI::previousStep()
{
    WizardStep prevStep = static_cast<WizardStep>(static_cast<int>(m_currentStep) - 1);
    if (prevStep >= Welcome) {
        showStep(prevStep);
    }
}

void WizardAI::completeWizard()
{
    setWizardCompleted();
    showStep(Completion);
}

QJsonObject WizardAI::analyzeWalletState() const
{
    QJsonObject analysis;
    
    // Analyze balance
    qint64 balance = getWalletBalance();
    analysis["balance"] = QString::number(balance);
    analysis["balance_sufficient"] = balance >= MIN_BALANCE_FOR_STAKING;
    analysis["balance_for_test"] = balance >= MIN_BALANCE_FOR_TEST;
    
    // Analyze transactions
    int txCount = getTransactionCount();
    analysis["transaction_count"] = txCount;
    analysis["has_transactions"] = txCount > 0;
    
    // Analyze security
    analysis["has_staking_setup"] = hasStakingSetup();
    analysis["has_backup"] = hasBackup();
    analysis["has_test_transaction"] = hasTestTransaction();
    
    // Analyze fees
    analysis["fee_setting"] = getFeeSetting();
    
    // Analyze addresses
    QStringList addresses = getAddressList();
    analysis["address_count"] = addresses.size();
    analysis["has_addresses"] = addresses.size() > 0;
    
    return analysis;
}

QJsonObject WizardAI::generateRecommendations() const
{
    QJsonObject recommendations;
    QJsonObject analysis = analyzeWalletState();
    
    // Security recommendations
    QStringList securityRecs = getSecurityRecommendations();
    recommendations["security"] = QJsonArray::fromStringList(securityRecs);
    
    // Staking recommendations
    if (analysis["balance_sufficient"].toBool()) {
        QStringList stakingRecs = getStakingRecommendations();
        recommendations["staking"] = QJsonArray::fromStringList(stakingRecs);
    }
    
    // Backup recommendations
    if (!analysis["has_backup"].toBool()) {
        QStringList backupRecs = getBackupRecommendations();
        recommendations["backup"] = QJsonArray::fromStringList(backupRecs);
    }
    
    // Fee recommendations
    QStringList feeRecs = getFeeRecommendations();
    recommendations["fees"] = QJsonArray::fromStringList(feeRecs);
    
    return recommendations;
}

QString WizardAI::getPersonalizedMessage() const
{
    QJsonObject analysis = analyzeWalletState();
    qint64 balance = analysis["balance"].toString().toLongLong();
    
    if (balance == 0) {
        return tr("Welcome! I see this is a new wallet. Let me help you get started with some SHAH and learn about the features.");
    } else if (balance < MIN_BALANCE_FOR_STAKING) {
        return tr("Great! You have some SHAH. Let me help you optimize your wallet and potentially set up staking when you have more funds.");
    } else {
        return tr("Excellent! You have a good balance. Let me help you maximize your earnings through staking and ensure your wallet is secure.");
    }
}

bool WizardAI::shouldShowStaking() const
{
    QJsonObject analysis = analyzeWalletState();
    return analysis["balance_sufficient"].toBool() && !analysis["has_staking_setup"].toBool();
}

bool WizardAI::shouldShowBackup() const
{
    QJsonObject analysis = analyzeWalletState();
    return !analysis["has_backup"].toBool();
}

bool WizardAI::shouldShowTestTransaction() const
{
    QJsonObject analysis = analyzeWalletState();
    return analysis["balance_for_test"].toBool() && !analysis["has_test_transaction"].toBool();
}

QStringList WizardAI::getSecurityRecommendations() const
{
    QStringList recommendations;
    QJsonObject analysis = analyzeWalletState();
    
    if (!analysis["has_backup"].toBool()) {
        recommendations << tr("Create a secure backup of your wallet");
    }
    
    if (analysis["address_count"].toInt() < 3) {
        recommendations << tr("Generate more addresses for better privacy");
    }
    
    recommendations << tr("Enable wallet encryption if not already enabled");
    recommendations << tr("Consider using cold staking for enhanced security");
    recommendations << tr("Keep your private keys secure and never share them");
    
    return recommendations;
}

QStringList WizardAI::getStakingRecommendations() const
{
    QStringList recommendations;
    
    recommendations << tr("Set up cold staking to earn rewards while keeping funds secure");
    recommendations << tr("Configure automatic staking for consistent earnings");
    recommendations << tr("Monitor your staking performance regularly");
    recommendations << tr("Consider diversifying your staking across multiple addresses");
    
    return recommendations;
}

QStringList WizardAI::getBackupRecommendations() const
{
    QStringList recommendations;
    
    recommendations << tr("Create an encrypted vault backup");
    recommendations << tr("Store backup in multiple secure locations");
    recommendations << tr("Test your backup by restoring it on a test wallet");
    recommendations << tr("Update your backup regularly when you make changes");
    
    return recommendations;
}

QStringList WizardAI::getFeeRecommendations() const
{
    QStringList recommendations;
    
    recommendations << tr("Use dynamic fee estimation for optimal transaction costs");
    recommendations << tr("Consider using the fee optimizer for better rates");
    recommendations << tr("Monitor network conditions for fee adjustments");
    
    return recommendations;
}

void WizardAI::onNextClicked()
{
    nextStep();
}

void WizardAI::onPreviousClicked()
{
    previousStep();
}

void WizardAI::onSkipClicked()
{
    markStepCompleted(m_currentStep);
    nextStep();
}

void WizardAI::onCompleteClicked()
{
    completeWizard();
    accept();
}

void WizardAI::onHelpClicked()
{
    m_helpDialog->show();
    m_helpDialog->raise();
    m_helpDialog->activateWindow();
}

void WizardAI::onRetryAnalysis()
{
    performWalletAnalysis();
}

void WizardAI::onShowDetails()
{
    m_showDetails = !m_showDetails;
    // Update UI to show/hide detailed information
}

void WizardAI::onHideDetails()
{
    m_showDetails = false;
    // Update UI to hide detailed information
}

void WizardAI::onStepChanged(int step)
{
    updateNavigationButtons();
    updateStepIndicator();
}

void WizardAI::onAnalysisComplete()
{
    m_analysisInProgress = false;
    m_analysisProgress->setValue(100);
    m_analysisStatus->setText(tr("Analysis complete!"));
    m_analysisNextButton->setEnabled(true);
    m_analysisRetryButton->setEnabled(true);
    
    // Display results
    QJsonObject analysis = analyzeWalletState();
    QJsonObject recommendations = generateRecommendations();
    
    QString results = tr("<h3>Wallet Analysis Results</h3>");
    results += tr("<p><b>Balance:</b> %1 SHAH</p>").arg(ShahcoinUnits::format(ShahcoinUnit::SHAH, analysis["balance"].toString().toLongLong()));
    results += tr("<p><b>Addresses:</b> %1</p>").arg(analysis["address_count"].toInt());
    results += tr("<p><b>Transactions:</b> %1</p>").arg(analysis["transaction_count"].toInt());
    results += tr("<p><b>Staking Setup:</b> %1</p>").arg(analysis["has_staking_setup"].toBool() ? tr("Yes") : tr("No"));
    results += tr("<p><b>Backup:</b> %1</p>").arg(analysis["has_backup"].toBool() ? tr("Yes") : tr("No"));
    
    results += tr("<h3>Recommendations</h3>");
    
    if (recommendations.contains("security")) {
        results += tr("<p><b>Security:</b></p><ul>");
        QJsonArray securityRecs = recommendations["security"].toArray();
        for (const QJsonValue& rec : securityRecs) {
            results += tr("<li>%1</li>").arg(rec.toString());
        }
        results += "</ul>";
    }
    
    if (recommendations.contains("staking")) {
        results += tr("<p><b>Staking:</b></p><ul>");
        QJsonArray stakingRecs = recommendations["staking"].toArray();
        for (const QJsonValue& rec : stakingRecs) {
            results += tr("<li>%1</li>").arg(rec.toString());
        }
        results += "</ul>";
    }
    
    m_analysisResults->setHtml(results);
}

void WizardAI::onRecommendationGenerated()
{
    // Update UI with generated recommendations
}

void WizardAI::onAnimationFinished()
{
    // Handle animation completion
}

void WizardAI::performWalletAnalysis()
{
    m_analysisInProgress = true;
    m_analysisProgress->setValue(0);
    m_analysisStatus->setText(tr("Analyzing wallet state..."));
    m_analysisNextButton->setEnabled(false);
    m_analysisRetryButton->setEnabled(false);
    
    // Simulate analysis progress
    QTimer* progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, [this, progressTimer]() {
        int currentValue = m_analysisProgress->value();
        if (currentValue < 90) {
            m_analysisProgress->setValue(currentValue + 10);
            m_analysisStatus->setText(tr("Analyzing... %1%").arg(currentValue + 10));
        } else {
            progressTimer->stop();
            progressTimer->deleteLater();
            m_analysisTimer->start(500); // Complete after 500ms
        }
    });
    progressTimer->start(200);
}

void WizardAI::analyzeBalance()
{
    // Analyze wallet balance
}

void WizardAI::analyzeSecurity()
{
    // Analyze security settings
}

void WizardAI::analyzeStaking()
{
    // Analyze staking configuration
}

void WizardAI::analyzeBackup()
{
    // Analyze backup status
}

void WizardAI::analyzeFees()
{
    // Analyze fee settings
}

void WizardAI::analyzeTransactions()
{
    // Analyze transaction history
}

void WizardAI::showRecommendation(const QString& title, const QString& description, const QString& action)
{
    // Show a recommendation in the UI
}

void WizardAI::showTip(const QString& tip, const QString& icon)
{
    // Show a tip in the UI
}

void WizardAI::showWarning(const QString& warning, const QString& icon)
{
    // Show a warning in the UI
}

void WizardAI::showSuccess(const QString& success, const QString& icon)
{
    QMessageBox::information(this, tr("Success"), success);
}

void WizardAI::showProgress(const QString& message)
{
    // Show progress message
}

void WizardAI::hideProgress()
{
    // Hide progress message
}

qint64 WizardAI::getWalletBalance() const
{
    if (m_walletModel) {
        return m_walletModel->getBalance();
    }
    return 0;
}

int WizardAI::getTransactionCount() const
{
    // Get transaction count from wallet model
    return 0; // Placeholder
}

bool WizardAI::hasStakingSetup() const
{
    // Check if staking is configured
    return false; // Placeholder
}

bool WizardAI::hasBackup() const
{
    // Check if backup exists
    return false; // Placeholder
}

bool WizardAI::hasTestTransaction() const
{
    // Check if test transaction was sent
    return false; // Placeholder
}

QString WizardAI::getFeeSetting() const
{
    // Get current fee setting
    return tr("Dynamic"); // Placeholder
}

QStringList WizardAI::getAddressList() const
{
    if (m_walletModel && m_walletModel->getAddressTableModel()) {
        return m_walletModel->getAddressTableModel()->getAddressList();
    }
    return QStringList();
}

void WizardAI::saveWizardProgress()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_settings->setValue(SETTINGS_COMPLETED, m_wizardCompleted);
    m_settings->setValue(SETTINGS_RUN_COUNT, m_wizardRunCount);
    m_settings->setValue(SETTINGS_LAST_ANALYSIS, m_lastAnalysisTime);
    
    // Save completed steps
    QJsonArray completedStepsArray;
    for (WizardStep step : m_completedSteps) {
        completedStepsArray.append(static_cast<int>(step));
    }
    m_settings->setValue("CompletedSteps", QString::fromUtf8(QJsonDocument(completedStepsArray).toJson()));
    
    m_settings->endGroup();
}

void WizardAI::loadWizardProgress()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_wizardCompleted = m_settings->value(SETTINGS_COMPLETED, false).toBool();
    m_wizardRunCount = m_settings->value(SETTINGS_RUN_COUNT, 0).toInt();
    m_lastAnalysisTime = m_settings->value(SETTINGS_LAST_ANALYSIS).toString();
    
    // Load completed steps
    QString completedStepsJson = m_settings->value("CompletedSteps").toString();
    if (!completedStepsJson.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(completedStepsJson.toUtf8());
        QJsonArray array = doc.array();
        m_completedSteps.clear();
        for (const QJsonValue& value : array) {
            m_completedSteps.append(static_cast<WizardStep>(value.toInt()));
        }
    }
    
    m_settings->endGroup();
}

void WizardAI::markStepCompleted(WizardStep step)
{
    if (!m_completedSteps.contains(step)) {
        m_completedSteps.append(step);
    }
}

bool WizardAI::isStepCompleted(WizardStep step) const
{
    return m_completedSteps.contains(step);
}

void WizardAI::setWizardCompleted()
{
    m_wizardCompleted = true;
    saveWizardProgress();
}

bool WizardAI::isWizardCompleted() const
{
    return m_wizardCompleted;
}

void WizardAI::showCurrentStep()
{
    m_stepStack->setCurrentIndex(static_cast<int>(m_currentStep));
}

void WizardAI::hideAllSteps()
{
    // Hide all steps (not needed with QStackedWidget)
}

void WizardAI::updateNavigationButtons()
{
    bool canGoPrevious = m_currentStep > Welcome;
    bool canGoNext = m_currentStep < Completion;
    bool canSkip = m_currentStep != Welcome && m_currentStep != Completion;
    
    m_previousButton->setEnabled(canGoPrevious);
    m_nextButton->setEnabled(canGoNext);
    m_skipButton->setEnabled(canSkip);
    m_helpButton->setEnabled(true);
}

void WizardAI::animateStepTransition(WizardStep fromStep, WizardStep toStep)
{
    // Animate step transition (placeholder)
}

void WizardAI::updateStepIndicator()
{
    QString stepText = tr("Step %1 of %2").arg(static_cast<int>(m_currentStep) + 1).arg(7);
    m_stepIndicator->setText(stepText);
}
