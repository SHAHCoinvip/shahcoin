#include "coldstakingsetup.h"
#include "walletmodel.h"
#include "guiutil.h"
#include "platformstyle.h"

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
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QTimer>
#include <QMessageBox>
#include <QProgressDialog>
#include <QInputDialog>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDateTimeEdit>
#include <QCalendarWidget>
#include <QTextBrowser>
#include <QScrollArea>

// Constants
const QString ColdStakingSetup::SETTINGS_GROUP = "ColdStakingSetup";
const QString ColdStakingSetup::SETTINGS_DELEGATIONS = "Delegations";
const QString ColdStakingSetup::SETTINGS_HOT_NODES = "HotNodes";
const QString ColdStakingSetup::SETTINGS_LAST_UPDATE = "LastUpdate";

ColdStakingSetup::ColdStakingSetup(WalletModel* walletModel, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(walletModel)
    , m_platformStyle(nullptr)
    , m_mainLayout(nullptr)
    , m_currentStep(Welcome)
    , m_setupInProgress(false)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_updateTimer(new QTimer(this))
    , m_animationTimer(new QTimer(this))
    , m_settings(new QSettings("Shahcoin", "ColdStakingSetup"))
    , m_stepAnimation(new QPropertyAnimation(this))
    , m_stepOpacityEffect(new QGraphicsOpacityEffect(this))
{
    setWindowTitle(tr("❄️ Cold Wallet Staking Setup"));
    setupUI();
    connectSignals();
    applyTheme();
    loadSettings();
    
    // Initialize with welcome step
    showStep(Welcome);
}

ColdStakingSetup::~ColdStakingSetup()
{
    saveSettings();
}

void ColdStakingSetup::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Setup Progress Section
    m_progressGroup = new QGroupBox(tr("📋 Setup Progress"), this);
    m_progressLayout = new QHBoxLayout(m_progressGroup);

    m_stepProgressBar = new QProgressBar(this);
    m_stepProgressBar->setRange(0, 6);
    m_stepProgressBar->setValue(0);
    m_stepProgressBar->setFormat(tr("Step %v of %m"));

    m_stepLabel = new QLabel(tr("Welcome to Cold Wallet Staking Setup"), this);
    m_stepLabel->setStyleSheet("font-weight: bold; color: #2E8B57;");

    m_previousButton = new QPushButton(tr("← Previous"), this);
    m_previousButton->setEnabled(false);
    m_nextButton = new QPushButton(tr("Next →"), this);

    m_progressLayout->addWidget(m_stepProgressBar);
    m_progressLayout->addWidget(m_stepLabel);
    m_progressLayout->addWidget(m_previousButton);
    m_progressLayout->addWidget(m_nextButton);

    // Step Content Area
    m_stepContentArea = new QScrollArea(this);
    m_stepContentWidget = new QWidget();
    m_stepContentLayout = new QVBoxLayout(m_stepContentWidget);

    // Welcome Step
    m_welcomeStep = new QWidget();
    QVBoxLayout* welcomeLayout = new QVBoxLayout(m_welcomeStep);
    
    m_welcomeTitle = new QLabel(tr("❄️ Cold Wallet Staking Setup"), this);
    m_welcomeTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2E8B57; text-align: center;");
    
    m_welcomeText = new QTextBrowser(this);
    m_welcomeText->setHtml(tr(R"(
        <h3>Welcome to Cold Wallet Staking!</h3>
        <p>This setup will guide you through the process of delegating your stake from a cold wallet to a hot staking node.</p>
        
        <h4>What is Cold Wallet Staking?</h4>
        <ul>
            <li><strong>Cold Wallet:</strong> Your offline wallet that holds your SHAH coins securely</li>
            <li><strong>Hot Node:</strong> An online staking node that can participate in consensus</li>
            <li><strong>Delegation:</strong> The process of allowing a hot node to stake your coins</li>
        </ul>
        
        <h4>Benefits:</h4>
        <ul>
            <li>✅ Keep your private keys offline for maximum security</li>
            <li>✅ Earn staking rewards without running a node</li>
            <li>✅ No need to keep your computer online 24/7</li>
            <li>✅ Professional node operators handle the technical details</li>
        </ul>
        
        <p><strong>Ready to get started?</strong> Click the button below to begin the setup process.</p>
    )"));
    
    m_startSetupButton = new QPushButton(tr("🚀 Start Setup"), this);
    m_startSetupButton->setStyleSheet("font-size: 16px; padding: 12px;");
    
    welcomeLayout->addWidget(m_welcomeTitle);
    welcomeLayout->addWidget(m_welcomeText);
    welcomeLayout->addWidget(m_startSetupButton);
    welcomeLayout->addStretch();

    // Cold Wallet Selection Step
    m_coldWalletStep = new QWidget();
    QVBoxLayout* coldWalletLayout = new QVBoxLayout(m_coldWalletStep);
    
    m_coldWalletLabel = new QLabel(tr("🔐 Cold Wallet Address"), this);
    m_coldWalletLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2E8B57;");
    
    m_coldWalletInput = new QLineEdit(this);
    m_coldWalletInput->setPlaceholderText(tr("Enter your cold wallet address"));
    
    m_importColdWalletButton = new QPushButton(tr("📁 Import from File"), this);
    m_coldWalletBalanceLabel = new QLabel(tr("Balance: Checking..."), this);
    
    m_coldWalletInstructions = new QTextBrowser(this);
    m_coldWalletInstructions->setHtml(tr(R"(
        <h4>Cold Wallet Requirements:</h4>
        <ul>
            <li>✅ Must be a valid SHAH address</li>
            <li>✅ Must have sufficient balance for delegation</li>
            <li>✅ Should be stored offline for security</li>
            <li>✅ Private keys should be backed up securely</li>
        </ul>
    )"));
    
    coldWalletLayout->addWidget(m_coldWalletLabel);
    coldWalletLayout->addWidget(m_coldWalletInput);
    coldWalletLayout->addWidget(m_importColdWalletButton);
    coldWalletLayout->addWidget(m_coldWalletBalanceLabel);
    coldWalletLayout->addWidget(m_coldWalletInstructions);

    // Hot Node Configuration Step
    m_hotNodeStep = new QWidget();
    QVBoxLayout* hotNodeLayout = new QVBoxLayout(m_hotNodeStep);
    
    QLabel* hotNodeTitle = new QLabel(tr("🔥 Hot Node Selection"), this);
    hotNodeTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2E8B57;");
    
    m_hotNodeCombo = new QComboBox(this);
    m_hotNodeCombo->addItem(tr("Select a hot node..."));
    m_hotNodeCombo->addItem(tr("Official Shahcoin Node (2.5% fee)"));
    m_hotNodeCombo->addItem(tr("Community Node Alpha (2.0% fee)"));
    m_hotNodeCombo->addItem(tr("Enterprise Node Beta (3.0% fee)"));
    
    m_refreshNodesButton = new QPushButton(tr("🔄 Refresh Nodes"), this);
    m_nodeInfoLabel = new QLabel(tr("Node Information: Select a node to view details"), this);
    
    m_nodeDetailsText = new QTextBrowser(this);
    m_nodeDetailsText->setMaximumHeight(150);
    
    m_testConnectionButton = new QPushButton(tr("🔗 Test Connection"), this);
    
    hotNodeLayout->addWidget(hotNodeTitle);
    hotNodeLayout->addWidget(m_hotNodeCombo);
    hotNodeLayout->addWidget(m_refreshNodesButton);
    hotNodeLayout->addWidget(m_nodeInfoLabel);
    hotNodeLayout->addWidget(m_nodeDetailsText);
    hotNodeLayout->addWidget(m_testConnectionButton);

    // Delegation Amount Step
    m_amountStep = new QWidget();
    QVBoxLayout* amountLayout = new QVBoxLayout(m_amountStep);
    
    QLabel* amountTitle = new QLabel(tr("💰 Delegation Amount"), this);
    amountTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2E8B57;");
    
    m_delegationAmountInput = new QDoubleSpinBox(this);
    m_delegationAmountInput->setRange(0.1, 1000000.0);
    m_delegationAmountInput->setDecimals(2);
    m_delegationAmountInput->setValue(100.0);
    m_delegationAmountInput->setSuffix(" SHAH");
    
    m_availableBalanceLabel = new QLabel(tr("Available Balance: 1000.00 SHAH"), this);
    m_feeLabel = new QLabel(tr("Node Fee: 2.5%"), this);
    m_estimatedRewardsLabel = new QLabel(tr("Estimated Annual Rewards: 5.0%"), this);
    
    m_amountSlider = new QSlider(Qt::Horizontal, this);
    m_amountSlider->setRange(1, 100);
    m_amountSlider->setValue(10);
    
    amountLayout->addWidget(amountTitle);
    amountLayout->addWidget(m_delegationAmountInput);
    amountLayout->addWidget(m_availableBalanceLabel);
    amountLayout->addWidget(m_feeLabel);
    amountLayout->addWidget(m_estimatedRewardsLabel);
    amountLayout->addWidget(m_amountSlider);

    // QR Code Generation Step
    m_qrStep = new QWidget();
    QVBoxLayout* qrLayout = new QVBoxLayout(m_qrStep);
    
    QLabel* qrTitle = new QLabel(tr("📱 QR Code Generation"), this);
    qrTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2E8B57;");
    
    m_qrCodeLabel = new QLabel(tr("QR Code will be generated here"), this);
    m_qrCodeLabel->setAlignment(Qt::AlignCenter);
    m_qrCodeLabel->setMinimumSize(200, 200);
    m_qrCodeLabel->setStyleSheet("border: 2px dashed #2E8B57; border-radius: 10px;");
    
    m_generateQRButton = new QPushButton(tr("🔲 Generate QR Code"), this);
    m_copyPayloadButton = new QPushButton(tr("📋 Copy Payload"), this);
    m_saveQRButton = new QPushButton(tr("💾 Save QR Code"), this);
    
    m_qrInstructions = new QTextBrowser(this);
    m_qrInstructions->setHtml(tr(R"(
        <h4>Next Steps:</h4>
        <ol>
            <li>Click "Generate QR Code" to create the delegation transaction</li>
            <li>Scan the QR code with your cold wallet</li>
            <li>Sign the transaction on your cold wallet</li>
            <li>Import the signed transaction back to this wallet</li>
        </ol>
    )"));
    
    qrLayout->addWidget(qrTitle);
    qrLayout->addWidget(m_qrCodeLabel);
    qrLayout->addWidget(m_generateQRButton);
    qrLayout->addWidget(m_copyPayloadButton);
    qrLayout->addWidget(m_saveQRButton);
    qrLayout->addWidget(m_qrInstructions);

    // Confirmation Step
    m_confirmationStep = new QWidget();
    QVBoxLayout* confirmationLayout = new QVBoxLayout(m_confirmationStep);
    
    m_delegationSummaryLabel = new QLabel(tr("📋 Delegation Summary"), this);
    m_delegationSummaryLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2E8B57;");
    
    m_delegationDetailsText = new QTextBrowser(this);
    m_delegationDetailsText->setMaximumHeight(200);
    
    m_confirmDelegationButton = new QPushButton(tr("✅ Confirm Delegation"), this);
    m_editDelegationButton = new QPushButton(tr("✏️ Edit Delegation"), this);
    
    confirmationLayout->addWidget(m_delegationSummaryLabel);
    confirmationLayout->addWidget(m_delegationDetailsText);
    confirmationLayout->addWidget(m_confirmDelegationButton);
    confirmationLayout->addWidget(m_editDelegationButton);

    // Setup Complete Step
    m_completeStep = new QWidget();
    QVBoxLayout* completeLayout = new QVBoxLayout(m_completeStep);
    
    m_completionTitle = new QLabel(tr("🎉 Setup Complete!"), this);
    m_completionTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2E8B57; text-align: center;");
    
    m_completionText = new QTextBrowser(this);
    m_completionText->setHtml(tr(R"(
        <h3>Congratulations! Your cold wallet staking setup is complete.</h3>
        
        <h4>What happens next:</h4>
        <ul>
            <li>✅ Your delegation will be processed on the network</li>
            <li>✅ You'll start earning staking rewards</li>
            <li>✅ You can monitor your delegation status</li>
            <li>✅ Rewards will be automatically credited</li>
        </ul>
    )"));
    
    m_viewDelegationsButton = new QPushButton(tr("📊 View Delegations"), this);
    m_newDelegationButton = new QPushButton(tr("➕ New Delegation"), this);
    
    completeLayout->addWidget(m_completionTitle);
    completeLayout->addWidget(m_completionText);
    completeLayout->addWidget(m_viewDelegationsButton);
    completeLayout->addWidget(m_newDelegationButton);

    // Delegation Management
    m_delegationsGroup = new QGroupBox(tr("📊 Active Delegations"), this);
    m_delegationsLayout = new QVBoxLayout(m_delegationsGroup);
    
    m_delegationsTable = new QTableWidget(this);
    m_delegationsTable->setColumnCount(6);
    m_delegationsTable->setHorizontalHeaderLabels({
        tr("Cold Wallet"), tr("Hot Node"), tr("Amount"), tr("Status"), tr("Rewards"), tr("Actions")
    });
    m_delegationsTable->horizontalHeader()->setStretchLastSection(true);
    
    m_revokeDelegationButton = new QPushButton(tr("🚫 Revoke Delegation"), this);
    m_claimRewardsButton = new QPushButton(tr("💰 Claim Rewards"), this);
    m_exportDelegationsButton = new QPushButton(tr("📤 Export Data"), this);
    
    QHBoxLayout* delegationButtonsLayout = new QHBoxLayout();
    delegationButtonsLayout->addWidget(m_revokeDelegationButton);
    delegationButtonsLayout->addWidget(m_claimRewardsButton);
    delegationButtonsLayout->addWidget(m_exportDelegationsButton);
    delegationButtonsLayout->addStretch();
    
    m_delegationsLayout->addWidget(m_delegationsTable);
    m_delegationsLayout->addLayout(delegationButtonsLayout);

    // Instructions Panel
    m_instructionsGroup = new QGroupBox(tr("📖 Instructions"), this);
    QVBoxLayout* instructionsLayout = new QVBoxLayout(m_instructionsGroup);
    
    m_instructionsText = new QTextBrowser(this);
    m_instructionsText->setMaximumHeight(150);
    m_showInstructionsButton = new QPushButton(tr("📖 Show Instructions"), this);
    
    instructionsLayout->addWidget(m_instructionsText);
    instructionsLayout->addWidget(m_showInstructionsButton);

    // Add all sections to main layout
    m_mainLayout->addWidget(m_progressGroup);
    m_mainLayout->addWidget(m_stepContentArea);
    m_mainLayout->addWidget(m_delegationsGroup);
    m_mainLayout->addWidget(m_instructionsGroup);

    // Set up scroll area
    m_stepContentArea->setWidget(m_stepContentWidget);
    m_stepContentArea->setWidgetResizable(true);
    m_stepContentArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_stepContentArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Apply opacity effect for animations
    m_stepContentWidget->setGraphicsEffect(m_stepOpacityEffect);
}

void ColdStakingSetup::connectSignals()
{
    connect(m_previousButton, &QPushButton::clicked, this, &ColdStakingSetup::onPreviousStepClicked);
    connect(m_nextButton, &QPushButton::clicked, this, &ColdStakingSetup::onNextStepClicked);
    connect(m_startSetupButton, &QPushButton::clicked, this, &ColdStakingSetup::onNextStepClicked);
    
    connect(m_generateQRButton, &QPushButton::clicked, this, &ColdStakingSetup::onGenerateQRClicked);
    connect(m_copyPayloadButton, &QPushButton::clicked, this, &ColdStakingSetup::onExportDelegationClicked);
    connect(m_saveQRButton, &QPushButton::clicked, this, &ColdStakingSetup::onExportDelegationClicked);
    
    connect(m_confirmDelegationButton, &QPushButton::clicked, this, &ColdStakingSetup::onConfirmDelegationClicked);
    connect(m_editDelegationButton, &QPushButton::clicked, this, &ColdStakingSetup::onPreviousStepClicked);
    
    connect(m_viewDelegationsButton, &QPushButton::clicked, this, [this]() {
        m_delegationsGroup->setVisible(true);
    });
    connect(m_newDelegationButton, &QPushButton::clicked, this, &ColdStakingSetup::startSetup);
    
    connect(m_revokeDelegationButton, &QPushButton::clicked, this, &ColdStakingSetup::onRevokeDelegationClicked);
    connect(m_claimRewardsButton, &QPushButton::clicked, this, &ColdStakingSetup::onClaimRewardsClicked);
    connect(m_exportDelegationsButton, &QPushButton::clicked, this, &ColdStakingSetup::onExportDelegationClicked);
    
    connect(m_showInstructionsButton, &QPushButton::clicked, this, &ColdStakingSetup::onShowInstructionsClicked);
    
    connect(m_updateTimer, &QTimer::timeout, this, &ColdStakingSetup::onUpdateTimer);
    connect(m_delegationsTable, &QTableWidget::itemClicked, this, &ColdStakingSetup::onDelegationItemClicked);
    connect(m_delegationsTable, &QTableWidget::customContextMenuRequested, this, &ColdStakingSetup::onContextMenuRequested);
}

void ColdStakingSetup::applyTheme()
{
    // Apply Shahcoin styling
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #2E8B57;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #2E8B57;
        }
        QPushButton {
            background-color: #2E8B57;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #3CB371;
        }
        QPushButton:pressed {
            background-color: #228B22;
        }
        QPushButton:disabled {
            background-color: #808080;
        }
        QComboBox {
            border: 1px solid #2E8B57;
            border-radius: 3px;
            padding: 5px;
        }
        QLineEdit {
            border: 1px solid #2E8B57;
            border-radius: 3px;
            padding: 5px;
        }
        QDoubleSpinBox {
            border: 1px solid #2E8B57;
            border-radius: 3px;
            padding: 5px;
        }
        QTableWidget {
            gridline-color: #2E8B57;
            selection-background-color: #3CB371;
        }
        QHeaderView::section {
            background-color: #2E8B57;
            color: white;
            padding: 5px;
            border: 1px solid #228B22;
        }
        QProgressBar {
            border: 1px solid #2E8B57;
            border-radius: 3px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #2E8B57;
            border-radius: 2px;
        }
    )");
}

void ColdStakingSetup::showStep(SetupStep step)
{
    m_currentStep = step;
    
    // Clear previous step content
    QLayoutItem* item;
    while ((item = m_stepContentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // Add current step content
    switch (step) {
        case Welcome:
            m_stepContentLayout->addWidget(m_welcomeStep);
            m_stepLabel->setText(tr("Welcome to Cold Wallet Staking Setup"));
            break;
        case ColdWalletSelection:
            m_stepContentLayout->addWidget(m_coldWalletStep);
            m_stepLabel->setText(tr("Step 1: Select Cold Wallet"));
            break;
        case HotNodeConfiguration:
            m_stepContentLayout->addWidget(m_hotNodeStep);
            m_stepLabel->setText(tr("Step 2: Configure Hot Node"));
            break;
        case DelegationAmount:
            m_stepContentLayout->addWidget(m_amountStep);
            m_stepLabel->setText(tr("Step 3: Set Delegation Amount"));
            break;
        case QRCodeGeneration:
            m_stepContentLayout->addWidget(m_qrStep);
            m_stepLabel->setText(tr("Step 4: Generate QR Code"));
            break;
        case Confirmation:
            m_stepContentLayout->addWidget(m_confirmationStep);
            m_stepLabel->setText(tr("Step 5: Confirm Delegation"));
            break;
        case SetupComplete:
            m_stepContentLayout->addWidget(m_completeStep);
            m_stepLabel->setText(tr("Setup Complete!"));
            break;
    }
    
    // Update progress bar
    m_stepProgressBar->setValue(step);
    
    // Update navigation buttons
    m_previousButton->setEnabled(step > Welcome);
    m_nextButton->setEnabled(canProceedToNextStep());
    
    if (step == SetupComplete) {
        m_nextButton->setText(tr("Finish"));
    } else {
        m_nextButton->setText(tr("Next →"));
    }
    
    animateStepTransition();
}

void ColdStakingSetup::onNextStepClicked()
{
    if (m_currentStep == SetupComplete) {
        // Setup is complete, show delegations
        m_delegationsGroup->setVisible(true);
        return;
    }
    
    if (!canProceedToNextStep()) {
        showNotification(tr("Please complete the current step before proceeding"), "warning");
        return;
    }
    
    SetupStep nextStep = static_cast<SetupStep>(m_currentStep + 1);
    showStep(nextStep);
}

void ColdStakingSetup::onPreviousStepClicked()
{
    if (m_currentStep > Welcome) {
        SetupStep prevStep = static_cast<SetupStep>(m_currentStep - 1);
        showStep(prevStep);
    }
}

bool ColdStakingSetup::canProceedToNextStep()
{
    switch (m_currentStep) {
        case Welcome:
            return true;
        case ColdWalletSelection:
            return !m_coldWalletInput->text().isEmpty();
        case HotNodeConfiguration:
            return m_hotNodeCombo->currentIndex() > 0;
        case DelegationAmount:
            return m_delegationAmountInput->value() > 0;
        case QRCodeGeneration:
            return !m_delegationQRData.isEmpty();
        case Confirmation:
            return true;
        case SetupComplete:
            return true;
    }
    return false;
}

void ColdStakingSetup::onGenerateQRClicked()
{
    // Generate mock delegation data
    m_currentDelegation.coldWalletAddress = m_coldWalletInput->text();
    m_currentDelegation.hotNodeAddress = "SNode1234567890abcdef";
    m_currentDelegation.delegatedAmount = m_delegationAmountInput->value();
    m_currentDelegation.delegationDate = QDateTime::currentDateTime();
    m_currentDelegation.expiryDate = QDateTime::currentDateTime().addYears(1);
    m_currentDelegation.status = Pending;
    m_currentDelegation.rewardsEarned = 0.0;
    m_currentDelegation.transactionId = "tx_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    
    // Create QR code payload
    m_delegationQRData = createDelegationPayload(m_currentDelegation);
    
    // Generate mock QR code (placeholder)
    QPixmap qrPixmap(200, 200);
    qrPixmap.fill(Qt::white);
    QPainter painter(&qrPixmap);
    painter.setPen(QPen(Qt::black, 2));
    
    // Draw a simple QR-like pattern
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 20; ++j) {
            if ((i + j) % 3 == 0) {
                painter.fillRect(i * 10, j * 10, 10, 10, Qt::black);
            }
        }
    }
    
    m_qrCodeLabel->setPixmap(qrPixmap);
    m_qrCodeLabel->setText("");
    
    showNotification(tr("QR Code generated successfully"), "success");
}

QString ColdStakingSetup::createDelegationPayload(const DelegationInfo& delegation)
{
    QJsonObject payload;
    payload["type"] = "cold_staking_delegation";
    payload["cold_wallet"] = delegation.coldWalletAddress;
    payload["hot_node"] = delegation.hotNodeAddress;
    payload["amount"] = delegation.delegatedAmount;
    payload["timestamp"] = delegation.delegationDate.toString(Qt::ISODate);
    payload["expiry"] = delegation.expiryDate.toString(Qt::ISODate);
    payload["transaction_id"] = delegation.transactionId;
    
    QJsonDocument doc(payload);
    return doc.toJson(QJsonDocument::Compact);
}

void ColdStakingSetup::onConfirmDelegationClicked()
{
    // Add delegation to list
    m_delegations.append(m_currentDelegation);
    
    // Update delegation display
    updateDelegationDisplay();
    
    // Complete setup
    showStep(SetupComplete);
    
    // Save delegations
    saveDelegations(m_delegations);
    
    emit setupCompleted(m_currentDelegation);
    showNotification(tr("Delegation created successfully!"), "success");
}

void ColdStakingSetup::updateDelegationDisplay()
{
    m_delegationsTable->setRowCount(m_delegations.size());
    
    for (int i = 0; i < m_delegations.size(); ++i) {
        const DelegationInfo& delegation = m_delegations[i];
        
        m_delegationsTable->setItem(i, 0, new QTableWidgetItem(delegation.coldWalletAddress));
        m_delegationsTable->setItem(i, 1, new QTableWidgetItem(delegation.hotNodeAddress));
        m_delegationsTable->setItem(i, 2, new QTableWidgetItem(QString("%1 SHAH").arg(delegation.delegatedAmount, 0, 'f', 2)));
        
        QString statusText;
        switch (delegation.status) {
            case NotDelegated: statusText = tr("Not Delegated"); break;
            case Pending: statusText = tr("Pending"); break;
            case Active: statusText = tr("Active"); break;
            case Expired: statusText = tr("Expired"); break;
            case Revoked: statusText = tr("Revoked"); break;
        }
        m_delegationsTable->setItem(i, 3, new QTableWidgetItem(statusText));
        
        m_delegationsTable->setItem(i, 4, new QTableWidgetItem(QString("%1 SHAH").arg(delegation.rewardsEarned, 0, 'f', 2)));
        m_delegationsTable->setItem(i, 5, new QTableWidgetItem(tr("Actions")));
    }
}

void ColdStakingSetup::onRevokeDelegationClicked()
{
    showNotification(tr("Delegation revocation not yet implemented"), "info");
}

void ColdStakingSetup::onClaimRewardsClicked()
{
    showNotification(tr("Rewards claiming not yet implemented"), "info");
}

void ColdStakingSetup::onExportDelegationClicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Delegation Data"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/cold_staking_delegations.json",
        tr("JSON Files (*.json)"));
    
    if (filename.isEmpty()) return;
    
    QJsonArray delegationsArray;
    for (const DelegationInfo& delegation : m_delegations) {
        QJsonObject delegationObj;
        delegationObj["cold_wallet"] = delegation.coldWalletAddress;
        delegationObj["hot_node"] = delegation.hotNodeAddress;
        delegationObj["amount"] = delegation.delegatedAmount;
        delegationObj["delegation_date"] = delegation.delegationDate.toString(Qt::ISODate);
        delegationObj["expiry_date"] = delegation.expiryDate.toString(Qt::ISODate);
        delegationObj["status"] = static_cast<int>(delegation.status);
        delegationObj["rewards_earned"] = delegation.rewardsEarned;
        delegationObj["transaction_id"] = delegation.transactionId;
        delegationsArray.append(delegationObj);
    }
    
    QJsonDocument doc(delegationsArray);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        showNotification(tr("Delegation data exported successfully"), "success");
    } else {
        showNotification(tr("Failed to export delegation data"), "error");
    }
}

void ColdStakingSetup::onShowInstructionsClicked()
{
    m_instructionsText->setHtml(tr(R"(
        <h3>Cold Wallet Staking Instructions</h3>
        
        <h4>Step 1: Prepare Your Cold Wallet</h4>
        <ul>
            <li>Ensure your cold wallet is properly backed up</li>
            <li>Verify you have sufficient SHAH balance</li>
            <li>Make sure your cold wallet supports QR code scanning</li>
        </ul>
        
        <h4>Step 2: Select a Hot Node</h4>
        <ul>
            <li>Choose a reliable hot node with good uptime</li>
            <li>Check the node's fee structure</li>
            <li>Verify the node's reputation and performance</li>
        </ul>
        
        <h4>Step 3: Generate and Sign Transaction</h4>
        <ul>
            <li>Generate the delegation QR code</li>
            <li>Scan with your cold wallet</li>
            <li>Review transaction details carefully</li>
            <li>Sign the transaction on your cold wallet</li>
        </ul>
        
        <h4>Security Best Practices</h4>
        <ul>
            <li>🔒 Never share your private keys</li>
            <li>🔒 Keep your cold wallet offline</li>
            <li>🔒 Use a dedicated device for cold storage</li>
            <li>🔒 Regularly backup your wallet</li>
            <li>🔒 Monitor your delegations regularly</li>
        </ul>
    )"));
}

void ColdStakingSetup::onUpdateTimer()
{
    // TODO: Update delegation statuses and rewards
}

void ColdStakingSetup::onDelegationItemClicked(QTableWidgetItem* item)
{
    Q_UNUSED(item);
    // TODO: Show delegation details
}

void ColdStakingSetup::onContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);
    // TODO: Implement context menu
}

void ColdStakingSetup::showNotification(const QString& message, const QString& type)
{
    Q_UNUSED(type);
    // TODO: Implement proper notification system
    qDebug() << "Cold Staking Setup:" << message;
}

void ColdStakingSetup::animateStepTransition()
{
    m_stepAnimation->setTargetObject(m_stepOpacityEffect);
    m_stepAnimation->setPropertyName("opacity");
    m_stepAnimation->setDuration(ANIMATION_DURATION_MS);
    m_stepAnimation->setStartValue(0.3);
    m_stepAnimation->setEndValue(1.0);
    m_stepAnimation->start();
}

void ColdStakingSetup::saveSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    saveDelegations(m_delegations);
    m_settings->setValue(SETTINGS_LAST_UPDATE, QDateTime::currentDateTime().toString(Qt::ISODate));
    m_settings->endGroup();
}

void ColdStakingSetup::loadSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    m_delegations = loadDelegations();
    m_settings->endGroup();
    
    updateDelegationDisplay();
}

QList<ColdStakingSetup::DelegationInfo> ColdStakingSetup::loadDelegations()
{
    QList<DelegationInfo> delegations;
    QJsonArray delegationsArray = m_settings->value(SETTINGS_DELEGATIONS).toJsonArray();
    
    for (const QJsonValue& value : delegationsArray) {
        QJsonObject obj = value.toObject();
        DelegationInfo delegation;
        delegation.coldWalletAddress = obj["cold_wallet"].toString();
        delegation.hotNodeAddress = obj["hot_node"].toString();
        delegation.delegatedAmount = obj["amount"].toDouble();
        delegation.delegationDate = QDateTime::fromString(obj["delegation_date"].toString(), Qt::ISODate);
        delegation.expiryDate = QDateTime::fromString(obj["expiry_date"].toString(), Qt::ISODate);
        delegation.status = static_cast<DelegationStatus>(obj["status"].toInt());
        delegation.rewardsEarned = obj["rewards_earned"].toDouble();
        delegation.transactionId = obj["transaction_id"].toString();
        delegations.append(delegation);
    }
    
    return delegations;
}

void ColdStakingSetup::saveDelegations(const QList<DelegationInfo>& delegations)
{
    QJsonArray delegationsArray;
    for (const DelegationInfo& delegation : delegations) {
        QJsonObject obj;
        obj["cold_wallet"] = delegation.coldWalletAddress;
        obj["hot_node"] = delegation.hotNodeAddress;
        obj["amount"] = delegation.delegatedAmount;
        obj["delegation_date"] = delegation.delegationDate.toString(Qt::ISODate);
        obj["expiry_date"] = delegation.expiryDate.toString(Qt::ISODate);
        obj["status"] = static_cast<int>(delegation.status);
        obj["rewards_earned"] = delegation.rewardsEarned;
        obj["transaction_id"] = delegation.transactionId;
        delegationsArray.append(obj);
    }
    
    m_settings->setValue(SETTINGS_DELEGATIONS, delegationsArray);
}

void ColdStakingSetup::startSetup()
{
    showStep(Welcome);
    m_setupInProgress = true;
}

// DelegationHistoryTable implementation
DelegationHistoryTable::DelegationHistoryTable(QWidget* parent)
    : QTableWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void DelegationHistoryTable::addDelegationRecord(const ColdStakingSetup::DelegationInfo& delegation)
{
    int row = rowCount();
    insertRow(row);
    
    setItem(row, 0, new QTableWidgetItem(delegation.coldWalletAddress));
    setItem(row, 1, new QTableWidgetItem(delegation.hotNodeAddress));
    setItem(row, 2, new QTableWidgetItem(QString("%1 SHAH").arg(delegation.delegatedAmount, 0, 'f', 2)));
    
    QString statusText;
    switch (delegation.status) {
        case ColdStakingSetup::NotDelegated: statusText = tr("Not Delegated"); break;
        case ColdStakingSetup::Pending: statusText = tr("Pending"); break;
        case ColdStakingSetup::Active: statusText = tr("Active"); break;
        case ColdStakingSetup::Expired: statusText = tr("Expired"); break;
        case ColdStakingSetup::Revoked: statusText = tr("Revoked"); break;
    }
    setItem(row, 3, new QTableWidgetItem(statusText));
    
    setItem(row, 4, new QTableWidgetItem(QString("%1 SHAH").arg(delegation.rewardsEarned, 0, 'f', 2)));
    setItem(row, 5, new QTableWidgetItem(tr("Actions")));
}

void DelegationHistoryTable::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    QAction* copyAddress = menu.addAction(tr("Copy Address"));
    QAction* copyTransactionId = menu.addAction(tr("Copy Transaction ID"));
    menu.addSeparator();
    QAction* exportDelegation = menu.addAction(tr("Export Delegation"));
    
    QAction* selectedAction = menu.exec(event->globalPos());
    
    if (selectedAction == copyAddress) {
        onCopyAddress();
    } else if (selectedAction == copyTransactionId) {
        onCopyTransactionId();
    } else if (selectedAction == exportDelegation) {
        onExportDelegation();
    }
}

void DelegationHistoryTable::onCopyAddress()
{
    // TODO: Implement copy address functionality
}

void DelegationHistoryTable::onCopyTransactionId()
{
    // TODO: Implement copy transaction ID functionality
}

void DelegationHistoryTable::onExportDelegation()
{
    // TODO: Implement export delegation functionality
}
