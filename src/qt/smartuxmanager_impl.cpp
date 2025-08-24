#include "smartuxmanager.h"
#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QProgressBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>

// SmartUXManager Implementation
SmartUXManager::SmartUXManager(QObject* parent)
    : QObject(parent)
    , m_settings("Shahcoin", "SmartUX")
    , m_networkManager(new QNetworkAccessManager(this))
    , m_riskAssessmentTimer(new QTimer(this))
    , m_alertTimer(new QTimer(this))
    , m_voiceCommandEnabled(false)
    , m_autoRiskAssessment(true)
    , m_smartAlertsEnabled(true)
    , m_networkMonitoringEnabled(true)
    , m_contractAnalysisEnabled(true)
    , m_transactionAnalysisEnabled(true)
{
    initialize();
}

SmartUXManager::~SmartUXManager()
{
    stopMonitoring();
}

void SmartUXManager::initialize()
{
    loadSettings();
    setupTimers();
    setupNetworkMonitoring();
    setupContractAnalysis();
    setupTransactionAnalysis();
    
    qDebug() << "[SmartUXManager] Initialized with settings:";
    qDebug() << "  Voice Commands:" << m_voiceCommandEnabled;
    qDebug() << "  Auto Risk Assessment:" << m_autoRiskAssessment;
    qDebug() << "  Smart Alerts:" << m_smartAlertsEnabled;
    qDebug() << "  Network Monitoring:" << m_networkMonitoringEnabled;
    qDebug() << "  Contract Analysis:" << m_contractAnalysisEnabled;
    qDebug() << "  Transaction Analysis:" << m_transactionAnalysisEnabled;
}

void SmartUXManager::setupTimers()
{
    // Risk assessment timer - runs every 30 seconds
    connect(m_riskAssessmentTimer, &QTimer::timeout, this, &SmartUXManager::performPeriodicRiskAssessment);
    m_riskAssessmentTimer->setInterval(30000);
    
    // Alert timer - runs every 5 seconds
    connect(m_alertTimer, &QTimer::timeout, this, &SmartUXManager::checkForAlerts);
    m_alertTimer->setInterval(5000);
}

void SmartUXManager::setupNetworkMonitoring()
{
    if (!m_networkMonitoringEnabled) return;
    
    // Monitor network connectivity
    connect(m_networkManager, &QNetworkAccessManager::networkAccessibleChanged,
            this, &SmartUXManager::onNetworkAccessibilityChanged);
    
    // Monitor API endpoints
    m_apiEndpoints = {
        "https://api.shah.vip/price",
        "https://api.shah.vip/marketplace",
        "https://api.shah.vip/staking"
    };
    
    qDebug() << "[SmartUXManager] Network monitoring setup complete";
}

void SmartUXManager::setupContractAnalysis()
{
    if (!m_contractAnalysisEnabled) return;
    
    // Known safe contracts
    m_safeContracts = {
        "0x1234567890abcdef1234567890abcdef12345678", // Shahcoin main contract
        "0xabcdef1234567890abcdef1234567890abcdef12", // ShahSwap contract
        "0x9876543210fedcba9876543210fedcba98765432"  // Staking contract
    };
    
    // Known scam patterns
    m_scamPatterns = {
        "transfer(address,uint256)",
        "approve(address,uint256)",
        "transferFrom(address,address,uint256)"
    };
    
    qDebug() << "[SmartUXManager] Contract analysis setup complete";
}

void SmartUXManager::setupTransactionAnalysis()
{
    if (!m_transactionAnalysisEnabled) return;
    
    // High-risk transaction patterns
    m_highRiskPatterns = {
        "large_transfer",      // Transfers > 50% of balance
        "unknown_contract",    // Contract not in safe list
        "high_gas",           // Gas price > 100 gwei
        "suspicious_timing",   // Multiple transactions in short time
        "unusual_amount"       // Non-standard amounts
    };
    
    qDebug() << "[SmartUXManager] Transaction analysis setup complete";
}

void SmartUXManager::startMonitoring()
{
    if (m_riskAssessmentTimer->isActive()) return;
    
    m_riskAssessmentTimer->start();
    m_alertTimer->start();
    
    qDebug() << "[SmartUXManager] Monitoring started";
    emit monitoringStarted();
}

void SmartUXManager::stopMonitoring()
{
    m_riskAssessmentTimer->stop();
    m_alertTimer->stop();
    
    qDebug() << "[SmartUXManager] Monitoring stopped";
    emit monitoringStopped();
}

RiskAssessment SmartUXManager::assessTransactionRisk(const QString& transactionData)
{
    RiskAssessment assessment;
    assessment.timestamp = QDateTime::currentDateTime();
    assessment.riskLevel = RiskLevel::LOW;
    assessment.confidence = 0.8;
    assessment.recommendations.clear();
    
    // Parse transaction data
    QJsonDocument doc = QJsonDocument::fromJson(transactionData.toUtf8());
    if (!doc.isObject()) {
        assessment.riskLevel = RiskLevel::HIGH;
        assessment.recommendations.append("Invalid transaction data format");
        return assessment;
    }
    
    QJsonObject tx = doc.object();
    
    // Check for large transfers
    double amount = tx["amount"].toDouble();
    double balance = tx["balance"].toDouble();
    if (amount > balance * 0.5) {
        assessment.riskLevel = RiskLevel::HIGH;
        assessment.recommendations.append("Large transfer detected (>50% of balance)");
    }
    
    // Check contract address
    QString contractAddress = tx["contractAddress"].toString();
    if (!contractAddress.isEmpty() && !m_safeContracts.contains(contractAddress)) {
        assessment.riskLevel = qMax(assessment.riskLevel, RiskLevel::MEDIUM);
        assessment.recommendations.append("Unknown contract address");
    }
    
    // Check gas price
    double gasPrice = tx["gasPrice"].toDouble();
    if (gasPrice > 100) {
        assessment.riskLevel = qMax(assessment.riskLevel, RiskLevel::MEDIUM);
        assessment.recommendations.append("High gas price detected");
    }
    
    // Check for suspicious timing
    QDateTime txTime = QDateTime::fromString(tx["timestamp"].toString(), Qt::ISODate);
    if (txTime.isValid()) {
        QDateTime now = QDateTime::currentDateTime();
        if (txTime.secsTo(now) < 60) {
            assessment.recommendations.append("Recent transaction - verify intent");
        }
    }
    
    assessment.confidence = calculateConfidence(assessment);
    return assessment;
}

double SmartUXManager::calculateConfidence(const RiskAssessment& assessment)
{
    double confidence = 0.8;
    
    // Reduce confidence based on risk level
    switch (assessment.riskLevel) {
        case RiskLevel::LOW:
            confidence = 0.9;
            break;
        case RiskLevel::MEDIUM:
            confidence = 0.7;
            break;
        case RiskLevel::HIGH:
            confidence = 0.5;
            break;
        case RiskLevel::CRITICAL:
            confidence = 0.3;
            break;
    }
    
    // Adjust based on number of recommendations
    int recCount = assessment.recommendations.size();
    if (recCount > 3) confidence *= 0.8;
    if (recCount > 5) confidence *= 0.7;
    
    return qBound(0.1, confidence, 1.0);
}

void SmartUXManager::addSmartAlert(const SmartAlert& alert)
{
    m_alerts.append(alert);
    
    // Limit alerts to last 100
    if (m_alerts.size() > 100) {
        m_alerts.removeFirst();
    }
    
    emit alertAdded(alert);
    
    // Show immediate alert if critical
    if (alert.level == RiskLevel::CRITICAL) {
        showImmediateAlert(alert);
    }
}

void SmartUXManager::showImmediateAlert(const SmartAlert& alert)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("Smart Alert - " + getRiskLevelString(alert.level));
    msgBox.setText(alert.title);
    msgBox.setInformativeText(alert.message);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Ignore);
    msgBox.setDefaultButton(QMessageBox::Ok);
    
    int result = msgBox.exec();
    if (result == QMessageBox::Ignore) {
        // Mark as dismissed
        markAlertAsDismissed(alert.id);
    }
}

QString SmartUXManager::getRiskLevelString(RiskLevel level)
{
    switch (level) {
        case RiskLevel::LOW: return "Low Risk";
        case RiskLevel::MEDIUM: return "Medium Risk";
        case RiskLevel::HIGH: return "High Risk";
        case RiskLevel::CRITICAL: return "Critical Risk";
        default: return "Unknown";
    }
}

void SmartUXManager::markAlertAsDismissed(const QString& alertId)
{
    for (auto& alert : m_alerts) {
        if (alert.id == alertId) {
            alert.dismissed = true;
            alert.dismissedAt = QDateTime::currentDateTime();
            break;
        }
    }
}

void SmartUXManager::performPeriodicRiskAssessment()
{
    if (!m_autoRiskAssessment) return;
    
    // Assess current wallet state
    RiskAssessment assessment;
    assessment.timestamp = QDateTime::currentDateTime();
    assessment.riskLevel = RiskLevel::LOW;
    
    // Check for unusual activity patterns
    if (hasUnusualActivity()) {
        assessment.riskLevel = RiskLevel::MEDIUM;
        assessment.recommendations.append("Unusual activity pattern detected");
    }
    
    // Check network connectivity
    if (!isNetworkHealthy()) {
        assessment.riskLevel = RiskLevel::HIGH;
        assessment.recommendations.append("Network connectivity issues detected");
    }
    
    // Check for pending high-risk transactions
    if (hasPendingHighRiskTransactions()) {
        assessment.riskLevel = RiskLevel::HIGH;
        assessment.recommendations.append("Pending high-risk transactions detected");
    }
    
    assessment.confidence = calculateConfidence(assessment);
    
    if (assessment.riskLevel > RiskLevel::LOW) {
        SmartAlert alert;
        alert.id = QUuid::createUuid().toString();
        alert.type = AlertType::PERIODIC_ASSESSMENT;
        alert.level = assessment.riskLevel;
        alert.title = "Periodic Risk Assessment";
        alert.message = "Risk assessment completed: " + getRiskLevelString(assessment.riskLevel);
        alert.timestamp = QDateTime::currentDateTime();
        alert.dismissed = false;
        
        addSmartAlert(alert);
    }
    
    emit riskAssessmentCompleted(assessment);
}

bool SmartUXManager::hasUnusualActivity()
{
    // Check for rapid transactions
    QDateTime now = QDateTime::currentDateTime();
    int recentTransactions = 0;
    
    // This would normally check actual transaction history
    // For now, return false as placeholder
    return false;
}

bool SmartUXManager::isNetworkHealthy()
{
    // Check network connectivity
    QNetworkRequest request(QUrl("https://api.shah.vip/health"));
    QNetworkReply* reply = m_networkManager->get(request);
    
    // For now, assume healthy
    return true;
}

bool SmartUXManager::hasPendingHighRiskTransactions()
{
    // Check for pending transactions with high risk
    // For now, return false as placeholder
    return false;
}

void SmartUXManager::checkForAlerts()
{
    if (!m_smartAlertsEnabled) return;
    
    // Check for new alerts from external sources
    // This would normally poll an API or check local conditions
    
    // Check for dismissed alerts that should be re-raised
    for (auto& alert : m_alerts) {
        if (alert.dismissed && shouldReRaiseAlert(alert)) {
            alert.dismissed = false;
            alert.dismissedAt = QDateTime();
            emit alertReRaised(alert);
        }
    }
}

bool SmartUXManager::shouldReRaiseAlert(const SmartAlert& alert)
{
    // Re-raise critical alerts after 1 hour
    if (alert.level == RiskLevel::CRITICAL) {
        QDateTime now = QDateTime::currentDateTime();
        return alert.dismissedAt.secsTo(now) > 3600;
    }
    
    return false;
}

void SmartUXManager::onNetworkAccessibilityChanged(QNetworkAccessManager::NetworkAccessibility accessibility)
{
    if (accessibility == QNetworkAccessManager::NotAccessible) {
        SmartAlert alert;
        alert.id = QUuid::createUuid().toString();
        alert.type = AlertType::NETWORK_ISSUE;
        alert.level = RiskLevel::HIGH;
        alert.title = "Network Connectivity Issue";
        alert.message = "Network connection lost. Some features may be unavailable.";
        alert.timestamp = QDateTime::currentDateTime();
        alert.dismissed = false;
        
        addSmartAlert(alert);
    }
}

// Voice Command Implementation
void SmartUXManager::startVoiceCommand()
{
    if (!m_voiceCommandEnabled) {
        qDebug() << "[SmartUXManager] Voice commands are disabled";
        return;
    }
    
    // This would integrate with system speech recognition
    // For now, simulate voice command processing
    qDebug() << "[SmartUXManager] Voice command started";
    emit voiceCommandStarted();
}

void SmartUXManager::processVoiceCommand(const QString& command)
{
    VoiceCommand vc;
    vc.id = QUuid::createUuid().toString();
    vc.command = command;
    vc.timestamp = QDateTime::currentDateTime();
    vc.processed = false;
    
    // Parse command
    QString lowerCommand = command.toLower();
    
    if (lowerCommand.contains("send") || lowerCommand.contains("transfer")) {
        vc.type = VoiceCommandType::TRANSFER;
        vc.processed = true;
    } else if (lowerCommand.contains("balance") || lowerCommand.contains("check")) {
        vc.type = VoiceCommandType::BALANCE_CHECK;
        vc.processed = true;
    } else if (lowerCommand.contains("stake") || lowerCommand.contains("staking")) {
        vc.type = VoiceCommandType::STAKING;
        vc.processed = true;
    } else {
        vc.type = VoiceCommandType::UNKNOWN;
        vc.processed = false;
    }
    
    m_voiceCommands.append(vc);
    emit voiceCommandProcessed(vc);
}

// Settings Management
void SmartUXManager::loadSettings()
{
    m_voiceCommandEnabled = m_settings.value("voiceCommandEnabled", false).toBool();
    m_autoRiskAssessment = m_settings.value("autoRiskAssessment", true).toBool();
    m_smartAlertsEnabled = m_settings.value("smartAlertsEnabled", true).toBool();
    m_networkMonitoringEnabled = m_settings.value("networkMonitoringEnabled", true).toBool();
    m_contractAnalysisEnabled = m_settings.value("contractAnalysisEnabled", true).toBool();
    m_transactionAnalysisEnabled = m_settings.value("transactionAnalysisEnabled", true).toBool();
}

void SmartUXManager::saveSettings()
{
    m_settings.setValue("voiceCommandEnabled", m_voiceCommandEnabled);
    m_settings.setValue("autoRiskAssessment", m_autoRiskAssessment);
    m_settings.setValue("smartAlertsEnabled", m_smartAlertsEnabled);
    m_settings.setValue("networkMonitoringEnabled", m_networkMonitoringEnabled);
    m_settings.setValue("contractAnalysisEnabled", m_contractAnalysisEnabled);
    m_settings.setValue("transactionAnalysisEnabled", m_transactionAnalysisEnabled);
    
    m_settings.sync();
}

// Getters and Setters
bool SmartUXManager::isVoiceCommandEnabled() const { return m_voiceCommandEnabled; }
bool SmartUXManager::isAutoRiskAssessmentEnabled() const { return m_autoRiskAssessment; }
bool SmartUXManager::areSmartAlertsEnabled() const { return m_smartAlertsEnabled; }
bool SmartUXManager::isNetworkMonitoringEnabled() const { return m_networkMonitoringEnabled; }
bool SmartUXManager::isContractAnalysisEnabled() const { return m_contractAnalysisEnabled; }
bool SmartUXManager::isTransactionAnalysisEnabled() const { return m_transactionAnalysisEnabled; }

void SmartUXManager::setVoiceCommandEnabled(bool enabled)
{
    if (m_voiceCommandEnabled != enabled) {
        m_voiceCommandEnabled = enabled;
        saveSettings();
        emit voiceCommandEnabledChanged(enabled);
    }
}

void SmartUXManager::setAutoRiskAssessmentEnabled(bool enabled)
{
    if (m_autoRiskAssessment != enabled) {
        m_autoRiskAssessment = enabled;
        saveSettings();
        emit autoRiskAssessmentEnabledChanged(enabled);
    }
}

void SmartUXManager::setSmartAlertsEnabled(bool enabled)
{
    if (m_smartAlertsEnabled != enabled) {
        m_smartAlertsEnabled = enabled;
        saveSettings();
        emit smartAlertsEnabledChanged(enabled);
    }
}

void SmartUXManager::setNetworkMonitoringEnabled(bool enabled)
{
    if (m_networkMonitoringEnabled != enabled) {
        m_networkMonitoringEnabled = enabled;
        if (enabled) {
            setupNetworkMonitoring();
        }
        saveSettings();
        emit networkMonitoringEnabledChanged(enabled);
    }
}

void SmartUXManager::setContractAnalysisEnabled(bool enabled)
{
    if (m_contractAnalysisEnabled != enabled) {
        m_contractAnalysisEnabled = enabled;
        if (enabled) {
            setupContractAnalysis();
        }
        saveSettings();
        emit contractAnalysisEnabledChanged(enabled);
    }
}

void SmartUXManager::setTransactionAnalysisEnabled(bool enabled)
{
    if (m_transactionAnalysisEnabled != enabled) {
        m_transactionAnalysisEnabled = enabled;
        if (enabled) {
            setupTransactionAnalysis();
        }
        saveSettings();
        emit transactionAnalysisEnabledChanged(enabled);
    }
}

QList<SmartAlert> SmartUXManager::getAlerts() const { return m_alerts; }
QList<VoiceCommand> SmartUXManager::getVoiceCommands() const { return m_voiceCommands; }
QList<RiskAssessment> SmartUXManager::getRiskAssessments() const { return m_riskAssessments; }

// Smart Alert Widget Implementation
SmartAlertWidget::SmartAlertWidget(const SmartAlert& alert, QWidget* parent)
    : QWidget(parent)
    , m_alert(alert)
{
    setupUI();
    setupAnimations();
}

void SmartAlertWidget::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(6);
    
    // Header with icon and title
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    QLabel* iconLabel = new QLabel();
    iconLabel->setFixedSize(24, 24);
    iconLabel->setPixmap(getAlertIcon(m_alert.level).pixmap(24, 24));
    headerLayout->addWidget(iconLabel);
    
    QLabel* titleLabel = new QLabel(m_alert.title);
    titleLabel->setFont(QFont("Segoe UI", 10, QFont::Bold));
    titleLabel->setStyleSheet("color: " + getRiskLevelColor(m_alert.level));
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    QPushButton* dismissBtn = new QPushButton("×");
    dismissBtn->setFixedSize(20, 20);
    dismissBtn->setStyleSheet("QPushButton { border: none; background: transparent; color: #666; }"
                             "QPushButton:hover { color: #333; }");
    connect(dismissBtn, &QPushButton::clicked, this, &SmartAlertWidget::dismiss);
    headerLayout->addWidget(dismissBtn);
    
    layout->addLayout(headerLayout);
    
    // Message
    QLabel* messageLabel = new QLabel(m_alert.message);
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet("color: #333; font-size: 9px;");
    layout->addWidget(messageLabel);
    
    // Timestamp
    QLabel* timeLabel = new QLabel(m_alert.timestamp.toString("MMM dd, hh:mm"));
    timeLabel->setStyleSheet("color: #666; font-size: 8px;");
    layout->addWidget(timeLabel);
    
    // Set widget style
    setStyleSheet(QString(
        "SmartAlertWidget {"
        "  background: white;"
        "  border: 1px solid %1;"
        "  border-radius: 8px;"
        "  margin: 2px;"
        "}"
        "SmartAlertWidget:hover {"
        "  border-color: %2;"
        "}"
    ).arg(getRiskLevelColor(m_alert.level)).arg(getRiskLevelColor(m_alert.level) + "80"));
}

void SmartAlertWidget::setupAnimations()
{
    // Fade in animation
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    
    QPropertyAnimation* fadeIn = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->start();
}

QIcon SmartAlertWidget::getAlertIcon(RiskLevel level)
{
    switch (level) {
        case RiskLevel::LOW: return QIcon(":/icons/info.png");
        case RiskLevel::MEDIUM: return QIcon(":/icons/warning.png");
        case RiskLevel::HIGH: return QIcon(":/icons/error.png");
        case RiskLevel::CRITICAL: return QIcon(":/icons/critical.png");
        default: return QIcon(":/icons/info.png");
    }
}

QString SmartAlertWidget::getRiskLevelColor(RiskLevel level)
{
    switch (level) {
        case RiskLevel::LOW: return "#4CAF50";
        case RiskLevel::MEDIUM: return "#FF9800";
        case RiskLevel::HIGH: return "#F44336";
        case RiskLevel::CRITICAL: return "#9C27B0";
        default: return "#2196F3";
    }
}

void SmartAlertWidget::dismiss()
{
    // Fade out animation
    QGraphicsOpacityEffect* opacityEffect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (opacityEffect) {
        QPropertyAnimation* fadeOut = new QPropertyAnimation(opacityEffect, "opacity", this);
        fadeOut->setDuration(200);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        fadeOut->setEasingCurve(QEasingCurve::InCubic);
        
        connect(fadeOut, &QPropertyAnimation::finished, this, [this]() {
            emit dismissed(m_alert.id);
            deleteLater();
        });
        
        fadeOut->start();
    } else {
        emit dismissed(m_alert.id);
        deleteLater();
    }
}

// Risk Assessment Dialog Implementation
RiskAssessmentDialog::RiskAssessmentDialog(const RiskAssessment& assessment, QWidget* parent)
    : QDialog(parent)
    , m_assessment(assessment)
{
    setWindowTitle("Risk Assessment");
    setModal(true);
    resize(500, 400);
    
    setupUI();
}

void RiskAssessmentDialog::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Header
    QLabel* headerLabel = new QLabel("Risk Assessment Results");
    headerLabel->setFont(QFont("Segoe UI", 14, QFont::Bold));
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);
    
    // Risk Level Display
    QHBoxLayout* riskLayout = new QHBoxLayout();
    QLabel* riskLabel = new QLabel("Risk Level:");
    riskLabel->setFont(QFont("Segoe UI", 10, QFont::Bold));
    riskLayout->addWidget(riskLabel);
    
    QLabel* levelLabel = new QLabel(getRiskLevelString(m_assessment.riskLevel));
    levelLabel->setStyleSheet("color: " + getRiskLevelColor(m_assessment.riskLevel) + "; font-weight: bold;");
    riskLayout->addWidget(levelLabel);
    riskLayout->addStretch();
    layout->addLayout(riskLayout);
    
    // Confidence Level
    QHBoxLayout* confLayout = new QHBoxLayout();
    QLabel* confLabel = new QLabel("Confidence:");
    confLabel->setFont(QFont("Segoe UI", 10, QFont::Bold));
    confLayout->addWidget(confLabel);
    
    QProgressBar* confBar = new QProgressBar();
    confBar->setRange(0, 100);
    confBar->setValue(static_cast<int>(m_assessment.confidence * 100));
    confBar->setFormat("%.1f%%");
    confLayout->addWidget(confBar);
    layout->addLayout(confLayout);
    
    // Recommendations
    QLabel* recLabel = new QLabel("Recommendations:");
    recLabel->setFont(QFont("Segoe UI", 10, QFont::Bold));
    layout->addWidget(recLabel);
    
    QTextEdit* recText = new QTextEdit();
    recText->setReadOnly(true);
    recText->setMaximumHeight(150);
    
    QString recommendations;
    for (const QString& rec : m_assessment.recommendations) {
        recommendations += "• " + rec + "\n";
    }
    if (recommendations.isEmpty()) {
        recommendations = "No specific recommendations at this time.";
    }
    recText->setPlainText(recommendations);
    layout->addWidget(recText);
    
    // Timestamp
    QLabel* timeLabel = new QLabel("Assessment Time: " + m_assessment.timestamp.toString("MMM dd, yyyy hh:mm:ss"));
    timeLabel->setStyleSheet("color: #666; font-size: 9px;");
    layout->addWidget(timeLabel);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* okBtn = new QPushButton("OK");
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(okBtn);
    
    layout->addLayout(buttonLayout);
}

QString RiskAssessmentDialog::getRiskLevelString(RiskLevel level)
{
    switch (level) {
        case RiskLevel::LOW: return "Low Risk";
        case RiskLevel::MEDIUM: return "Medium Risk";
        case RiskLevel::HIGH: return "High Risk";
        case RiskLevel::CRITICAL: return "Critical Risk";
        default: return "Unknown";
    }
}

QString RiskAssessmentDialog::getRiskLevelColor(RiskLevel level)
{
    switch (level) {
        case RiskLevel::LOW: return "#4CAF50";
        case RiskLevel::MEDIUM: return "#FF9800";
        case RiskLevel::HIGH: return "#F44336";
        case RiskLevel::CRITICAL: return "#9C27B0";
        default: return "#2196F3";
    }
}

// Voice Command Dialog Implementation
VoiceCommandDialog::VoiceCommandDialog(QWidget* parent)
    : QDialog(parent)
    , m_voiceCommandEnabled(false)
{
    setWindowTitle("Voice Commands");
    setModal(true);
    resize(400, 300);
    
    setupUI();
}

void VoiceCommandDialog::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Enable/Disable Toggle
    QHBoxLayout* toggleLayout = new QHBoxLayout();
    QLabel* toggleLabel = new QLabel("Voice Commands:");
    toggleLabel->setFont(QFont("Segoe UI", 10, QFont::Bold));
    toggleLayout->addWidget(toggleLabel);
    
    QCheckBox* enableCheck = new QCheckBox("Enable");
    enableCheck->setChecked(m_voiceCommandEnabled);
    connect(enableCheck, &QCheckBox::toggled, this, &VoiceCommandDialog::setVoiceCommandEnabled);
    toggleLayout->addWidget(enableCheck);
    toggleLayout->addStretch();
    layout->addLayout(toggleLayout);
    
    // Voice Command Button
    m_voiceBtn = new QPushButton("🎤 Start Voice Command");
    m_voiceBtn->setEnabled(m_voiceCommandEnabled);
    m_voiceBtn->setMinimumHeight(50);
    m_voiceBtn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #4CAF50, stop:1 #45a049);"
        "  border: none;"
        "  border-radius: 25px;"
        "  color: white;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #45a049, stop:1 #3d8b40);"
        "}"
        "QPushButton:pressed {"
        "  background: #3d8b40;"
        "}"
        "QPushButton:disabled {"
        "  background: #cccccc;"
        "  color: #666666;"
        "}"
    );
    connect(m_voiceBtn, &QPushButton::clicked, this, &VoiceCommandDialog::startVoiceCommand);
    layout->addWidget(m_voiceBtn);
    
    // Status Label
    m_statusLabel = new QLabel("Voice commands are disabled");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #666; font-size: 12px;");
    layout->addWidget(m_statusLabel);
    
    // Command History
    QLabel* historyLabel = new QLabel("Recent Commands:");
    historyLabel->setFont(QFont("Segoe UI", 10, QFont::Bold));
    layout->addWidget(historyLabel);
    
    m_historyTable = new QTableWidget();
    m_historyTable->setColumnCount(3);
    m_historyTable->setHorizontalHeaderLabels({"Command", "Type", "Time"});
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->setMaximumHeight(150);
    layout->addWidget(m_historyTable);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* clearBtn = new QPushButton("Clear History");
    connect(clearBtn, &QPushButton::clicked, this, &VoiceCommandDialog::clearHistory);
    buttonLayout->addWidget(clearBtn);
    
    QPushButton* closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeBtn);
    
    layout->addLayout(buttonLayout);
    
    updateStatus();
}

void VoiceCommandDialog::setVoiceCommandEnabled(bool enabled)
{
    m_voiceCommandEnabled = enabled;
    m_voiceBtn->setEnabled(enabled);
    updateStatus();
}

void VoiceCommandDialog::updateStatus()
{
    if (m_voiceCommandEnabled) {
        m_statusLabel->setText("Voice commands are enabled. Click the button to start.");
        m_statusLabel->setStyleSheet("color: #4CAF50; font-size: 12px;");
    } else {
        m_statusLabel->setText("Voice commands are disabled. Enable to use voice features.");
        m_statusLabel->setStyleSheet("color: #666; font-size: 12px;");
    }
}

void VoiceCommandDialog::startVoiceCommand()
{
    if (!m_voiceCommandEnabled) return;
    
    // Simulate voice command processing
    m_voiceBtn->setText("🎤 Listening...");
    m_voiceBtn->setEnabled(false);
    
    // Simulate processing delay
    QTimer::singleShot(2000, this, [this]() {
        m_voiceBtn->setText("🎤 Start Voice Command");
        m_voiceBtn->setEnabled(true);
        
        // Add sample command to history
        addCommandToHistory("Check balance", "Balance Check", QDateTime::currentDateTime());
    });
}

void VoiceCommandDialog::addCommandToHistory(const QString& command, const QString& type, const QDateTime& time)
{
    int row = m_historyTable->rowCount();
    m_historyTable->insertRow(row);
    
    m_historyTable->setItem(row, 0, new QTableWidgetItem(command));
    m_historyTable->setItem(row, 1, new QTableWidgetItem(type));
    m_historyTable->setItem(row, 2, new QTableWidgetItem(time.toString("hh:mm:ss")));
}

void VoiceCommandDialog::clearHistory()
{
    m_historyTable->setRowCount(0);
}
