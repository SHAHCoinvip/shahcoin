#include "smartuxmanager.h"
#include "walletmodel.h"
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QDateTime>

SmartUXManager::SmartUXManager(WalletModel* walletModel, QObject* parent)
    : QObject(parent)
    , m_walletModel(walletModel)
    , m_settings("Shahcoin", "SmartUX")
    , m_isMonitoring(false)
    , m_voiceCommandsEnabled(false)
    , m_riskAlertsEnabled(true)
    , m_autoRiskAssessment(true)
    , m_alertSoundEnabled(true)
    , m_voiceConfirmationEnabled(false)
    , m_riskThreshold(RiskLevel::MEDIUM)
    , m_voiceLanguage("en-US")
    , m_voiceSensitivity(0.7)
    , m_networkManager(nullptr)
    , m_networkHealthy(true)
    , m_pendingTransactions(0)
    , m_syncPercentage(100)
    , m_voiceRecognitionActive(false)
    , m_riskDialog(nullptr)
    , m_alertDialog(nullptr)
    , m_voiceDialog(nullptr)
    , m_statusBar(nullptr)
{
    initialize();
}

SmartUXManager::~SmartUXManager()
{
    stopMonitoring();
    saveSettings();
}

void SmartUXManager::initialize()
{
    loadSettings();
    setupTimers();
    loadKnownAddresses();
    loadKnownContracts();
}

void SmartUXManager::setupTimers()
{
    m_networkHealthTimer = new QTimer(this);
    m_networkHealthTimer->setInterval(30000);
    connect(m_networkHealthTimer, &QTimer::timeout, this, &SmartUXManager::onNetworkHealthCheck);

    m_mempoolTimer = new QTimer(this);
    m_mempoolTimer->setInterval(60000);
    connect(m_mempoolTimer, &QTimer::timeout, this, &SmartUXManager::onMempoolCheck);

    m_syncTimer = new QTimer(this);
    m_syncTimer->setInterval(10000);
    connect(m_syncTimer, &QTimer::timeout, this, &SmartUXManager::onSyncCheck);
}

void SmartUXManager::startMonitoring()
{
    if (m_isMonitoring) return;
    m_isMonitoring = true;
    
    if (m_riskAlertsEnabled) {
        m_networkHealthTimer->start();
        m_mempoolTimer->start();
        m_syncTimer->start();
    }
    qDebug() << "Smart UX Manager: Monitoring started";
}

void SmartUXManager::stopMonitoring()
{
    if (!m_isMonitoring) return;
    m_isMonitoring = false;
    
    m_networkHealthTimer->stop();
    m_mempoolTimer->stop();
    m_syncTimer->stop();
    qDebug() << "Smart UX Manager: Monitoring stopped";
}

bool SmartUXManager::isMonitoring() const
{
    return m_isMonitoring;
}

RiskAssessment SmartUXManager::assessTransactionRisk(const QString& txHex, const QString& recipient, double amount)
{
    RiskAssessment assessment;
    assessment.transactionId = generateRiskAssessmentId();
    assessment.timestamp = QDateTime::currentDateTime();
    assessment.alerts.clear();

    if (isHighValueTransaction(amount)) {
        SmartAlert alert;
        alert.type = AlertType::LARGE_AMOUNT;
        alert.level = RiskLevel::HIGH;
        alert.title = "High Value Transaction";
        alert.message = QString("Transaction amount of %1 SHAH is unusually high").arg(amount);
        alert.details = "This transaction represents a significant portion of your balance.";
        alert.recommendation = "Double-check the recipient address and amount before proceeding.";
        alert.action = "Review";
        alert.timestamp = QDateTime::currentDateTime();
        alert.acknowledged = false;
        alert.dismissed = false;
        alert.transactionId = assessment.transactionId;
        alert.address = recipient;
        alert.amount = amount;
        alert.currency = "SHAH";
        
        assessment.alerts.append(alert);
    }

    assessment.riskScore = calculateRiskScore(assessment.alerts);
    
    if (assessment.riskScore >= 0.8) {
        assessment.overallRisk = RiskLevel::CRITICAL;
    } else if (assessment.riskScore >= 0.6) {
        assessment.overallRisk = RiskLevel::HIGH;
    } else if (assessment.riskScore >= 0.4) {
        assessment.overallRisk = RiskLevel::MEDIUM;
    } else {
        assessment.overallRisk = RiskLevel::LOW;
    }

    if (assessment.alerts.isEmpty()) {
        assessment.summary = "No risks detected. Transaction appears safe.";
        assessment.recommended = true;
    } else {
        assessment.summary = QString("Found %1 potential risk(s). Review before proceeding.").arg(assessment.alerts.size());
        assessment.recommended = assessment.overallRisk < RiskLevel::HIGH;
    }

    m_riskAssessments.append(assessment);
    logRiskAssessment(assessment);
    emit riskAssessmentCompleted(assessment);

    return assessment;
}

RiskLevel SmartUXManager::assessAddressRisk(const QString& address)
{
    if (m_addressRiskCache.contains(address)) {
        return m_addressRiskCache[address];
    }

    RiskLevel risk = RiskLevel::LOW;

    if (m_knownScamAddresses.contains(address)) {
        risk = RiskLevel::CRITICAL;
    } else if (m_knownSafeAddresses.contains(address)) {
        risk = RiskLevel::LOW;
    } else if (isSuspiciousPattern(address)) {
        risk = RiskLevel::HIGH;
    } else if (!validateAddressFormat(address)) {
        risk = RiskLevel::HIGH;
    }

    m_addressRiskCache[address] = risk;
    return risk;
}

RiskLevel SmartUXManager::assessContractRisk(const QString& contractAddress)
{
    if (m_contractRiskCache.contains(contractAddress)) {
        return m_contractRiskCache[contractAddress];
    }

    RiskLevel risk = RiskLevel::LOW;

    if (m_knownScamContracts.contains(contractAddress)) {
        risk = RiskLevel::CRITICAL;
    } else if (m_knownSafeContracts.contains(contractAddress)) {
        risk = RiskLevel::LOW;
    } else {
        analyzeContract(contractAddress);
        risk = RiskLevel::MEDIUM;
    }

    m_contractRiskCache[contractAddress] = risk;
    return risk;
}

double SmartUXManager::calculateRiskScore(const QList<SmartAlert>& alerts)
{
    if (alerts.isEmpty()) return 0.0;

    double totalScore = 0.0;
    double totalWeight = 0.0;

    for (const SmartAlert& alert : alerts) {
        double weight = 1.0;
        switch (alert.level) {
            case RiskLevel::LOW: weight = 0.2; break;
            case RiskLevel::MEDIUM: weight = 0.5; break;
            case RiskLevel::HIGH: weight = 0.8; break;
            case RiskLevel::CRITICAL: weight = 1.0; break;
        }
        
        totalScore += weight;
        totalWeight += 1.0;
    }

    return totalWeight > 0 ? totalScore / totalWeight : 0.0;
}

void SmartUXManager::addAlert(const SmartAlert& alert)
{
    m_activeAlerts.append(alert);
    logAlert(alert);
    emit alertRaised(alert);

    if (m_alertSoundEnabled) {
        playAlertSound();
    }

    if (m_statusBar) {
        showStatusMessage(QString("Alert: %1").arg(alert.title), 5000);
    }
}

void SmartUXManager::acknowledgeAlert(const QString& alertId)
{
    for (SmartAlert& alert : m_activeAlerts) {
        if (alert.transactionId == alertId) {
            alert.acknowledged = true;
            emit alertAcknowledged(alertId);
            break;
        }
    }
}

void SmartUXManager::dismissAlert(const QString& alertId)
{
    for (int i = 0; i < m_activeAlerts.size(); ++i) {
        if (m_activeAlerts[i].transactionId == alertId) {
            m_activeAlerts[i].dismissed = true;
            emit alertDismissed(alertId);
            m_activeAlerts.removeAt(i);
            break;
        }
    }
}

void SmartUXManager::clearAlerts()
{
    m_activeAlerts.clear();
}

QList<SmartAlert> SmartUXManager::getActiveAlerts() const
{
    return m_activeAlerts;
}

QList<SmartAlert> SmartUXManager::getAlertsByType(AlertType type) const
{
    QList<SmartAlert> filtered;
    for (const SmartAlert& alert : m_activeAlerts) {
        if (alert.type == type) {
            filtered.append(alert);
        }
    }
    return filtered;
}

QList<SmartAlert> SmartUXManager::getAlertsByRiskLevel(RiskLevel level) const
{
    QList<SmartAlert> filtered;
    for (const SmartAlert& alert : m_activeAlerts) {
        if (alert.level == level) {
            filtered.append(alert);
        }
    }
    return filtered;
}

void SmartUXManager::startVoiceRecognition()
{
    if (!m_voiceCommandsEnabled || m_voiceRecognitionActive) return;
    m_voiceRecognitionActive = true;
    qDebug() << "Smart UX Manager: Voice recognition started";
}

void SmartUXManager::stopVoiceRecognition()
{
    if (!m_voiceRecognitionActive) return;
    m_voiceRecognitionActive = false;
    qDebug() << "Smart UX Manager: Voice recognition stopped";
}

bool SmartUXManager::isVoiceRecognitionActive() const
{
    return m_voiceRecognitionActive;
}

void SmartUXManager::processVoiceCommand(const QString& audioData)
{
    QString text = convertSpeechToText(QByteArray::fromBase64(audioData.toUtf8()));
    
    if (!text.isEmpty()) {
        VoiceCommand command = parseVoiceCommand(text);
        if (command.type != VoiceCommandType::UNKNOWN) {
            m_voiceCommandHistory.append(command);
            emit voiceCommandReceived(command);
            
            if (executeVoiceCommand(command)) {
                emit voiceCommandExecuted(command);
            } else {
                emit voiceCommandFailed(command, "Failed to execute command");
            }
        }
    }
}

VoiceCommand SmartUXManager::parseVoiceCommand(const QString& text)
{
    VoiceCommand command;
    command.command = text;
    command.timestamp = QDateTime::currentDateTime();
    command.confidence = 0.8;
    command.executed = false;

    QString lowerText = text.toLower();

    if (lowerText.contains("send") || lowerText.contains("transfer")) {
        command.type = VoiceCommandType::SEND_MONEY;
    } else if (lowerText.contains("balance")) {
        command.type = VoiceCommandType::CHECK_BALANCE;
    } else if (lowerText.contains("transaction")) {
        command.type = VoiceCommandType::VIEW_TRANSACTIONS;
    } else if (lowerText.contains("create token")) {
        command.type = VoiceCommandType::CREATE_TOKEN;
    } else if (lowerText.contains("mint nft")) {
        command.type = VoiceCommandType::MINT_NFT;
    } else if (lowerText.contains("stake")) {
        command.type = VoiceCommandType::STAKE_COINS;
    } else if (lowerText.contains("marketplace")) {
        command.type = VoiceCommandType::OPEN_MARKETPLACE;
    } else if (lowerText.contains("portfolio")) {
        command.type = VoiceCommandType::SHOW_PORTFOLIO;
    } else if (lowerText.contains("settings")) {
        command.type = VoiceCommandType::SETTINGS;
    } else if (lowerText.contains("help")) {
        command.type = VoiceCommandType::HELP;
    } else {
        command.type = VoiceCommandType::UNKNOWN;
    }

    return command;
}

bool SmartUXManager::executeVoiceCommand(const VoiceCommand& command)
{
    switch (command.type) {
        case VoiceCommandType::CHECK_BALANCE:
        case VoiceCommandType::VIEW_TRANSACTIONS:
        case VoiceCommandType::OPEN_MARKETPLACE:
        case VoiceCommandType::SHOW_PORTFOLIO:
        case VoiceCommandType::SETTINGS:
        case VoiceCommandType::HELP:
            return true;
        default:
            return false;
    }
}

QList<VoiceCommand> SmartUXManager::getVoiceCommandHistory() const
{
    return m_voiceCommandHistory;
}

// Settings methods
void SmartUXManager::setVoiceCommandsEnabled(bool enabled) { m_voiceCommandsEnabled = enabled; m_settings.setValue("voiceCommandsEnabled", enabled); }
void SmartUXManager::setRiskAlertsEnabled(bool enabled) { m_riskAlertsEnabled = enabled; m_settings.setValue("riskAlertsEnabled", enabled); }
void SmartUXManager::setAutoRiskAssessment(bool enabled) { m_autoRiskAssessment = enabled; m_settings.setValue("autoRiskAssessment", enabled); }
void SmartUXManager::setAlertSoundEnabled(bool enabled) { m_alertSoundEnabled = enabled; m_settings.setValue("alertSoundEnabled", enabled); }
void SmartUXManager::setVoiceConfirmationEnabled(bool enabled) { m_voiceConfirmationEnabled = enabled; m_settings.setValue("voiceConfirmationEnabled", enabled); }
void SmartUXManager::setRiskThreshold(RiskLevel threshold) { m_riskThreshold = threshold; m_settings.setValue("riskThreshold", static_cast<int>(threshold)); }
void SmartUXManager::setVoiceLanguage(const QString& language) { m_voiceLanguage = language; m_settings.setValue("voiceLanguage", language); }
void SmartUXManager::setVoiceSensitivity(double sensitivity) { m_voiceSensitivity = qBound(0.0, sensitivity, 1.0); m_settings.setValue("voiceSensitivity", sensitivity); }

bool SmartUXManager::isVoiceCommandsEnabled() const { return m_voiceCommandsEnabled; }
bool SmartUXManager::isRiskAlertsEnabled() const { return m_riskAlertsEnabled; }
bool SmartUXManager::isAutoRiskAssessment() const { return m_autoRiskAssessment; }
bool SmartUXManager::isAlertSoundEnabled() const { return m_alertSoundEnabled; }
bool SmartUXManager::isVoiceConfirmationEnabled() const { return m_voiceConfirmationEnabled; }
RiskLevel SmartUXManager::getRiskThreshold() const { return m_riskThreshold; }
QString SmartUXManager::getVoiceLanguage() const { return m_voiceLanguage; }
double SmartUXManager::getVoiceSensitivity() const { return m_voiceSensitivity; }

void SmartUXManager::loadSettings()
{
    m_voiceCommandsEnabled = m_settings.value("voiceCommandsEnabled", false).toBool();
    m_riskAlertsEnabled = m_settings.value("riskAlertsEnabled", true).toBool();
    m_autoRiskAssessment = m_settings.value("autoRiskAssessment", true).toBool();
    m_alertSoundEnabled = m_settings.value("alertSoundEnabled", true).toBool();
    m_voiceConfirmationEnabled = m_settings.value("voiceConfirmationEnabled", false).toBool();
    m_riskThreshold = static_cast<RiskLevel>(m_settings.value("riskThreshold", static_cast<int>(RiskLevel::MEDIUM)).toInt());
    m_voiceLanguage = m_settings.value("voiceLanguage", "en-US").toString();
    m_voiceSensitivity = m_settings.value("voiceSensitivity", 0.7).toDouble();
}

void SmartUXManager::saveSettings()
{
    m_settings.setValue("voiceCommandsEnabled", m_voiceCommandsEnabled);
    m_settings.setValue("riskAlertsEnabled", m_riskAlertsEnabled);
    m_settings.setValue("autoRiskAssessment", m_autoRiskAssessment);
    m_settings.setValue("alertSoundEnabled", m_alertSoundEnabled);
    m_settings.setValue("voiceConfirmationEnabled", m_voiceConfirmationEnabled);
    m_settings.setValue("riskThreshold", static_cast<int>(m_riskThreshold));
    m_settings.setValue("voiceLanguage", m_voiceLanguage);
    m_settings.setValue("voiceSensitivity", m_voiceSensitivity);
}

void SmartUXManager::loadKnownAddresses()
{
    QStringList safeAddresses = m_settings.value("knownSafeAddresses").toStringList();
    for (const QString& address : safeAddresses) {
        m_knownSafeAddresses.insert(address);
    }

    QStringList scamAddresses = m_settings.value("knownScamAddresses").toStringList();
    for (const QString& address : scamAddresses) {
        m_knownScamAddresses.insert(address);
    }
}

void SmartUXManager::loadKnownContracts()
{
    QStringList safeContracts = m_settings.value("knownSafeContracts").toStringList();
    for (const QString& contract : safeContracts) {
        m_knownSafeContracts.insert(contract);
    }

    QStringList scamContracts = m_settings.value("knownScamContracts").toStringList();
    for (const QString& contract : scamContracts) {
        m_knownScamContracts.insert(contract);
    }
}

bool SmartUXManager::isHighValueTransaction(double amount) const
{
    return amount > 1000.0;
}

bool SmartUXManager::isFrequentTransaction(const QString& address) const
{
    return false; // Stub implementation
}

bool SmartUXManager::isSuspiciousPattern(const QString& address) const
{
    if (address.length() != 34) return true;
    return false; // Stub implementation
}

bool SmartUXManager::isKnownScamAddress(const QString& address) const
{
    return m_knownScamAddresses.contains(address);
}

bool SmartUXManager::isKnownSafeAddress(const QString& address) const
{
    return m_knownSafeAddresses.contains(address);
}

bool SmartUXManager::isKnownScamContract(const QString& contract) const
{
    return m_knownScamContracts.contains(contract);
}

bool SmartUXManager::isKnownSafeContract(const QString& contract) const
{
    return m_knownSafeContracts.contains(contract);
}

bool SmartUXManager::validateAddressFormat(const QString& address) const
{
    if (address.length() != 34) return false;
    if (!address.startsWith("S")) return false;
    return true; // Basic validation
}

QString SmartUXManager::convertSpeechToText(const QByteArray& audioData)
{
    return "check balance"; // Stub implementation
}

QString SmartUXManager::generateAlertId() const
{
    return QString("alert_%1").arg(QDateTime::currentMSecsSinceEpoch());
}

QString SmartUXManager::generateRiskAssessmentId() const
{
    return QString("risk_%1").arg(QDateTime::currentMSecsSinceEpoch());
}

QString SmartUXManager::generateVoiceCommandId() const
{
    return QString("voice_%1").arg(QDateTime::currentMSecsSinceEpoch());
}

void SmartUXManager::logAlert(const SmartAlert& alert)
{
    qDebug() << "Smart Alert:" << alert.title << "-" << alert.message;
}

void SmartUXManager::logRiskAssessment(const RiskAssessment& assessment)
{
    qDebug() << "Risk Assessment:" << assessment.summary << "Score:" << assessment.riskScore;
}

void SmartUXManager::playAlertSound()
{
    qDebug() << "Playing alert sound";
}

void SmartUXManager::showStatusMessage(const QString& message, int timeout)
{
    if (m_statusBar) {
        m_statusBar->showMessage(message, timeout);
    }
}

void SmartUXManager::checkNetworkHealth()
{
    m_networkHealthy = true;
    emit networkHealthChanged(m_networkHealthy);
}

void SmartUXManager::monitorMempool()
{
    m_pendingTransactions = 0;
    emit mempoolStatusChanged(m_pendingTransactions);
}

void SmartUXManager::checkSyncStatus()
{
    m_syncPercentage = 100;
    emit syncStatusChanged(m_syncPercentage);
}

void SmartUXManager::analyzeContract(const QString& contractAddress)
{
    qDebug() << "Analyzing contract:" << contractAddress;
}

void SmartUXManager::analyzeAddress(const QString& address)
{
    qDebug() << "Analyzing address:" << address;
}

void SmartUXManager::analyzeTransaction(const QString& txHex)
{
    qDebug() << "Analyzing transaction:" << txHex.left(20) + "...";
}

void SmartUXManager::onNetworkReplyFinished()
{
    // Handle network responses
}

void SmartUXManager::onNetworkHealthCheck()
{
    checkNetworkHealth();
}

void SmartUXManager::onMempoolCheck()
{
    monitorMempool();
}

void SmartUXManager::onSyncCheck()
{
    checkSyncStatus();
}
