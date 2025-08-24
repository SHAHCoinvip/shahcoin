// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#include "shahsecurity.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

namespace SHAHSecurity {

// SHAHSecurityManager Implementation
SHAHSecurityManager::SHAHSecurityManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_refreshTimer(nullptr)
    , m_cacheTimer(nullptr)
    , m_isOnline(false)
    , m_refreshInterval(720) // 12 hours default
    , m_riskThreshold(RiskMedium)
{
    initializeNetwork();
    initializeTimers();
    loadCachedData();
}

SHAHSecurityManager::~SHAHSecurityManager()
{
    saveCachedData();
    delete m_networkManager;
    delete m_refreshTimer;
    delete m_cacheTimer;
}

void SHAHSecurityManager::initializeNetwork()
{
    m_networkManager = new QNetworkAccessManager(this);
    m_apiEndpoint = "https://api.shah.vip/security/v1";
    
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &SHAHSecurityManager::onNetworkReplyFinished);
}

void SHAHSecurityManager::initializeTimers()
{
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(m_refreshInterval * 60 * 1000); // Convert to milliseconds
    connect(m_refreshTimer, &QTimer::timeout, this, &SHAHSecurityManager::onRefreshTimerTimeout);
    m_refreshTimer->start();

    m_cacheTimer = new QTimer(this);
    m_cacheTimer->setInterval(3600000); // 1 hour
    connect(m_cacheTimer, &QTimer::timeout, this, &SHAHSecurityManager::onCacheExpired);
    m_cacheTimer->start();
}

AssetSecurityInfo SHAHSecurityManager::getAssetSecurityInfo(const QString &address)
{
    QMutexLocker locker(&m_cacheMutex);
    
    if (m_assetCache.contains(address)) {
        return m_assetCache[address];
    }
    
    // Create default info
    AssetSecurityInfo info;
    info.address = address;
    info.riskLevel = RiskUnknown;
    info.badgeType = BadgeWarning;
    info.badgeIcon = "⚠️";
    info.isVerified = false;
    info.isTrusted = false;
    info.reputationScore = 0;
    info.lastUpdated = QDateTime::currentDateTime();
    
    // Try to fetch from API
    updateAssetSecurity(address);
    
    return info;
}

DeveloperReputation SHAHSecurityManager::getDeveloperReputation(const QString &address)
{
    QMutexLocker locker(&m_cacheMutex);
    
    if (m_developerCache.contains(address)) {
        return m_developerCache[address];
    }
    
    // Create default reputation
    DeveloperReputation reputation;
    reputation.address = address;
    reputation.mintCount = 0;
    reputation.refundRequests = 0;
    reputation.flaggedContent = 0;
    reputation.totalAssets = 0;
    reputation.reputationScore = 0.0;
    reputation.isTrusted = false;
    reputation.lastActivity = QDateTime::currentDateTime();
    
    // Try to fetch from API
    updateDeveloperReputation(address);
    
    return reputation;
}

PhishingDetectionResult SHAHSecurityManager::detectPhishing(const QString &url, const QString &content)
{
    PhishingDetector detector(this);
    return detector.detectPhishing(url, content);
}

BehaviorAnalysis SHAHSecurityManager::analyzeOnChainBehavior(const QString &address)
{
    QMutexLocker locker(&m_cacheMutex);
    
    if (m_behaviorCache.contains(address)) {
        BehaviorAnalysis analysis = m_behaviorCache[address];
        // Check if analysis is recent (within 1 hour)
        if (analysis.analysisTime.secsTo(QDateTime::currentDateTime()) < 3600) {
            return analysis;
        }
    }
    
    // Perform new analysis
    BehaviorAnalyzer analyzer(this);
    BehaviorAnalysis analysis = analyzer.analyzeBehavior(address);
    analysis.analysisTime = QDateTime::currentDateTime();
    
    m_behaviorCache[address] = analysis;
    
    return analysis;
}

QString SHAHSecurityManager::submitSecurityReport(const SecurityReport &report)
{
    QString reportId = QString("REP_%1_%2").arg(
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"),
        report.assetAddress.left(8)
    );
    
    // Log the report
    AuditLogSystem auditLog(this);
    auditLog.logReportSubmission(reportId, report.reporterAddress);
    
    // TODO: Send to API
    QJsonObject reportData;
    reportData["reportId"] = reportId;
    reportData["reporterAddress"] = report.reporterAddress;
    reportData["assetAddress"] = report.assetAddress;
    reportData["reportType"] = report.reportType;
    reportData["description"] = report.description;
    reportData["reportTime"] = report.reportTime.toString(Qt::ISODate);
    
    QNetworkRequest request(QUrl(m_apiEndpoint + "/reports"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(reportData).toJson());
    reply->setProperty("type", "submit_report");
    reply->setProperty("reportId", reportId);
    
    emit reportSubmitted(reportId);
    return reportId;
}

QList<SecurityReport> SHAHSecurityManager::getReportsForAsset(const QString &address)
{
    QList<SecurityReport> reports;
    
    // TODO: Fetch from API
    QNetworkRequest request(QUrl(m_apiEndpoint + "/reports/" + address));
    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("type", "get_reports");
    reply->setProperty("address", address);
    
    return reports;
}

bool SHAHSecurityManager::checkTrademarkViolation(const QString &name, const QString &symbol)
{
    LegalProtectionSystem legalSystem(this);
    return legalSystem.checkTrademarkViolation(name, symbol);
}

QStringList SHAHSecurityManager::getProtectedTerms()
{
    LegalProtectionSystem legalSystem(this);
    return legalSystem.getProtectedTerms();
}

void SHAHSecurityManager::clearCache()
{
    QMutexLocker locker(&m_cacheMutex);
    m_assetCache.clear();
    m_developerCache.clear();
    m_behaviorCache.clear();
}

void SHAHSecurityManager::refreshSecurityData()
{
    // Refresh all cached data
    QMutexLocker locker(&m_cacheMutex);
    
    for (auto it = m_assetCache.begin(); it != m_assetCache.end(); ++it) {
        updateAssetSecurity(it.key());
    }
    
    for (auto it = m_developerCache.begin(); it != m_developerCache.end(); ++it) {
        updateDeveloperReputation(it.key());
    }
    
    emit securityDataUpdated();
}

void SHAHSecurityManager::setApiEndpoint(const QString &endpoint)
{
    m_apiEndpoint = endpoint;
}

void SHAHSecurityManager::setRefreshInterval(int minutes)
{
    m_refreshInterval = minutes;
    if (m_refreshTimer) {
        m_refreshTimer->setInterval(minutes * 60 * 1000);
    }
}

void SHAHSecurityManager::setRiskThreshold(RiskLevel threshold)
{
    m_riskThreshold = threshold;
}

bool SHAHSecurityManager::isOnline() const
{
    return m_isOnline;
}

QString SHAHSecurityManager::getLastError() const
{
    return m_lastError;
}

QList<AssetSecurityInfo> SHAHSecurityManager::listCachedAssets() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_cacheMutex));
    return m_assetCache.values();
}

int SHAHSecurityManager::computeSecurityScore(const AssetSecurityInfo &info) const
{
    // Map existing risk to a baseline
    int base = 10;
    switch (info.riskLevel) {
        case RiskVerified: base = 5; break;
        case RiskLow: base = 15; break;
        case RiskMedium: base = 45; break;
        case RiskHigh: base = 80; break;
        case RiskUnknown: default: base = 30; break;
    }

    // Adjust by flags and reputation
    int score = base;
    if (info.isTrusted) score -= 10;
    if (info.isVerified) score -= 20;
    if (info.reputationScore < 0) score += 10;
    score += static_cast<int>(qMin(20, info.warnings.size() * 5));

    // Clamp 0..100
    score = qMax(0, qMin(100, score));
    return score;
}

QString SHAHSecurityManager::badgeForScore(int score) const
{
    if (score <= 25) return "safe";
    if (score <= 60) return "caution";
    return "risky";
}

QString SHAHSecurityManager::tooltipFor(const AssetSecurityInfo &info, int score) const
{
    QStringList lines;
    lines << QString("Score: %1/100").arg(score);
    if (!info.riskDescription.isEmpty()) lines << info.riskDescription;
    for (const QString &w : info.warnings) lines << QString("• %1").arg(w);
    if (!info.detailsUrl.isEmpty()) lines << QString("Details: %1").arg(info.detailsUrl);
    return lines.join('\n');
}

void SHAHSecurityManager::onNetworkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        QString type = reply->property("type").toString();
        
        if (type == "asset_security") {
            QString address = reply->property("address").toString();
            QJsonObject obj = doc.object();
            
            AssetSecurityInfo info;
            info.address = address;
            info.name = obj["name"].toString();
            info.symbol = obj["symbol"].toString();
            info.riskLevel = static_cast<RiskLevel>(obj["risk_level"].toInt());
            info.badgeType = static_cast<BadgeType>(obj["badge_type"].toInt());
            info.badgeIcon = obj["badge_icon"].toString();
            info.riskDescription = obj["risk_description"].toString();
            info.detailsUrl = obj["details_url"].toString();
            info.isVerified = obj["is_verified"].toBool();
            info.isTrusted = obj["is_trusted"].toBool();
            info.reputationScore = obj["reputation_score"].toInt();
            info.lastVerified = QDateTime::fromString(obj["last_verified"].toString(), Qt::ISODate);
            info.lastUpdated = QDateTime::currentDateTime();
            
            QMutexLocker locker(&m_cacheMutex);
            m_assetCache[address] = info;
            
            emit assetRiskChanged(address, info.riskLevel);
        }
        else if (type == "developer_reputation") {
            QString address = reply->property("address").toString();
            QJsonObject obj = doc.object();
            
            DeveloperReputation reputation;
            reputation.address = address;
            reputation.name = obj["name"].toString();
            reputation.mintCount = obj["mint_count"].toInt();
            reputation.refundRequests = obj["refund_requests"].toInt();
            reputation.flaggedContent = obj["flagged_content"].toInt();
            reputation.totalAssets = obj["total_assets"].toInt();
            reputation.reputationScore = obj["reputation_score"].toDouble();
            reputation.isTrusted = obj["is_trusted"].toBool();
            reputation.lastActivity = QDateTime::fromString(obj["last_activity"].toString(), Qt::ISODate);
            
            QMutexLocker locker(&m_cacheMutex);
            m_developerCache[address] = reputation;
        }
        
        m_isOnline = true;
        m_lastError.clear();
    } else {
        m_isOnline = false;
        m_lastError = reply->errorString();
        emit errorOccurred(m_lastError);
    }
    
    reply->deleteLater();
}

void SHAHSecurityManager::onRefreshTimerTimeout()
{
    refreshSecurityData();
}

void SHAHSecurityManager::onCacheExpired()
{
    // Clean up old cache entries
    QMutexLocker locker(&m_cacheMutex);
    
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-3600); // 1 hour ago
    
    for (auto it = m_assetCache.begin(); it != m_assetCache.end();) {
        if (it.value().lastUpdated < cutoff) {
            it = m_assetCache.erase(it);
        } else {
            ++it;
        }
    }
    
    for (auto it = m_developerCache.begin(); it != m_developerCache.end();) {
        if (it.value().lastActivity < cutoff) {
            it = m_developerCache.erase(it);
        } else {
            ++it;
        }
    }
}

void SHAHSecurityManager::updateAssetSecurity(const QString &address)
{
    if (!isValidAddress(address)) return;
    
    QNetworkRequest request(QUrl(m_apiEndpoint + "/assets/" + address));
    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("type", "asset_security");
    reply->setProperty("address", address);
}

void SHAHSecurityManager::updateDeveloperReputation(const QString &address)
{
    if (!isValidAddress(address)) return;
    
    QNetworkRequest request(QUrl(m_apiEndpoint + "/developers/" + address));
    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("type", "developer_reputation");
    reply->setProperty("address", address);
}

QString SHAHSecurityManager::getRiskLevelString(RiskLevel level) const
{
    switch (level) {
        case RiskLow: return "Low Risk";
        case RiskMedium: return "Medium Risk";
        case RiskHigh: return "High Risk";
        case RiskVerified: return "Verified";
        case RiskUnknown: return "Unknown";
        default: return "Unknown";
    }
}

QString SHAHSecurityManager::getBadgeIcon(BadgeType type) const
{
    switch (type) {
        case BadgeVerified: return "✅";
        case BadgeTrusted: return "🛡️";
        case BadgeWarning: return "⚠️";
        case BadgeDanger: return "🚨";
        case BadgeScam: return "🔥";
        default: return "❓";
    }
}

QString SHAHSecurityManager::getBadgeColor(BadgeType type) const
{
    switch (type) {
        case BadgeVerified: return "#28a745";
        case BadgeTrusted: return "#17a2b8";
        case BadgeWarning: return "#ffc107";
        case BadgeDanger: return "#fd7e14";
        case BadgeScam: return "#dc3545";
        default: return "#6c757d";
    }
}

bool SHAHSecurityManager::isValidAddress(const QString &address) const
{
    // Basic Ethereum address validation
    QRegularExpression re("^0x[a-fA-F0-9]{40}$");
    return re.match(address).hasMatch();
}

void SHAHSecurityManager::loadCachedData()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/shahcoin/security";
    QDir().mkpath(cacheDir);
    
    // Load asset cache
    QFile assetFile(cacheDir + "/asset_cache.json");
    if (assetFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(assetFile.readAll());
        QJsonObject obj = doc.object();
        
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            QJsonObject assetObj = it.value().toObject();
            AssetSecurityInfo info;
            info.address = it.key();
            info.name = assetObj["name"].toString();
            info.symbol = assetObj["symbol"].toString();
            info.riskLevel = static_cast<RiskLevel>(assetObj["risk_level"].toInt());
            info.badgeType = static_cast<BadgeType>(assetObj["badge_type"].toInt());
            info.badgeIcon = assetObj["badge_icon"].toString();
            info.isVerified = assetObj["is_verified"].toBool();
            info.isTrusted = assetObj["is_trusted"].toBool();
            info.lastUpdated = QDateTime::fromString(assetObj["last_updated"].toString(), Qt::ISODate);
            
            m_assetCache[it.key()] = info;
        }
        assetFile.close();
    }
}

void SHAHSecurityManager::saveCachedData()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/shahcoin/security";
    QDir().mkpath(cacheDir);
    
    // Save asset cache
    QJsonObject cacheObj;
    for (auto it = m_assetCache.begin(); it != m_assetCache.end(); ++it) {
        QJsonObject assetObj;
        assetObj["name"] = it.value().name;
        assetObj["symbol"] = it.value().symbol;
        assetObj["risk_level"] = static_cast<int>(it.value().riskLevel);
        assetObj["badge_type"] = static_cast<int>(it.value().badgeType);
        assetObj["badge_icon"] = it.value().badgeIcon;
        assetObj["is_verified"] = it.value().isVerified;
        assetObj["is_trusted"] = it.value().isTrusted;
        assetObj["last_updated"] = it.value().lastUpdated.toString(Qt::ISODate);
        
        cacheObj[it.key()] = assetObj;
    }
    
    QFile assetFile(cacheDir + "/asset_cache.json");
    if (assetFile.open(QIODevice::WriteOnly)) {
        assetFile.write(QJsonDocument(cacheObj).toJson());
        assetFile.close();
    }
}

// PhishingDetector Implementation
PhishingDetector::PhishingDetector(QObject *parent)
    : QObject(parent)
{
    loadPhishingDatabase();
}

PhishingDetector::~PhishingDetector()
{
}

PhishingDetectionResult PhishingDetector::detectPhishing(const QString &url, const QString &content)
{
    PhishingDetectionResult result;
    result.isPhishing = false;
    result.confidence = 0.0;
    
    // Check various patterns
    bool urlPatterns = checkUrlPatterns(url);
    bool contentPatterns = checkContentPatterns(content);
    bool lookalikeDomains = checkLookalikeDomains(url);
    bool fakeMintLinks = checkFakeMintLinks(url);
    
    // Calculate confidence
    result.confidence = calculateConfidence(url, content);
    
    // Determine if phishing
    if (urlPatterns || contentPatterns || lookalikeDomains || fakeMintLinks) {
        result.isPhishing = true;
    }
    
    // Build recommendation
    if (result.isPhishing) {
        result.recommendation = "⚠️ This appears to be a phishing attempt. Do not interact with this link.";
    } else {
        result.recommendation = "✅ This link appears to be safe.";
    }
    
    return result;
}

void PhishingDetector::loadPhishingDatabase()
{
    // Load known phishing patterns
    m_phishingPatterns = {
        "shahcoin.*airdrop",
        "claim.*shahcoin",
        "verify.*wallet",
        "connect.*wallet.*now",
        "urgent.*action.*required",
        "limited.*time.*offer",
        "free.*shahcoin.*claim"
    };
    
    // Load lookalike domains
    m_lookalikeDomains = {
        "shahco1n.com",
        "shahc0in.com",
        "shahcoin.net",
        "shah.vip",
        "shahcoin.xyz",
        "shahcoin.io"
    };
    
    // Load legitimate domains
    m_legitimateDomains = {
        "shah.vip",
        "shahcoin.com",
        "shah.vip"
    };
    
    // Load fake mint links
    m_fakeMintLinks = {
        "mint.*shahcoin",
        "free.*mint",
        "claim.*nft",
        "airdrop.*claim"
    };
}

bool PhishingDetector::checkUrlPatterns(const QString &url)
{
    for (const QString &pattern : m_phishingPatterns) {
        QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
        if (re.match(url).hasMatch()) {
            return true;
        }
    }
    return false;
}

bool PhishingDetector::checkContentPatterns(const QString &content)
{
    QStringList suspiciousPatterns = {
        "urgent.*action",
        "limited.*time",
        "free.*claim",
        "verify.*now",
        "connect.*immediately"
    };
    
    for (const QString &pattern : suspiciousPatterns) {
        QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
        if (re.match(content).hasMatch()) {
            return true;
        }
    }
    return false;
}

bool PhishingDetector::checkLookalikeDomains(const QString &url)
{
    QUrl qurl(url);
    QString domain = qurl.host();
    
    for (const QString &lookalike : m_lookalikeDomains) {
        if (domain.contains(lookalike, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool PhishingDetector::checkFakeMintLinks(const QString &url)
{
    for (const QString &pattern : m_fakeMintLinks) {
        QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
        if (re.match(url).hasMatch()) {
            return true;
        }
    }
    return false;
}

double PhishingDetector::calculateConfidence(const QString &url, const QString &content)
{
    double confidence = 0.0;
    
    if (checkUrlPatterns(url)) confidence += 0.3;
    if (checkContentPatterns(content)) confidence += 0.3;
    if (checkLookalikeDomains(url)) confidence += 0.2;
    if (checkFakeMintLinks(url)) confidence += 0.2;
    
    return qMin(confidence, 1.0);
}

// BehaviorAnalyzer Implementation
BehaviorAnalyzer::BehaviorAnalyzer(QObject *parent)
    : QObject(parent)
    , m_mintDumpThreshold(0.8)
    , m_gasAnomalyThreshold(0.7)
{
    m_suspiciousPatterns = {
        "rapid_mint_dump",
        "gas_fee_manipulation",
        "wash_trading",
        "pump_and_dump"
    };
}

BehaviorAnalyzer::~BehaviorAnalyzer()
{
}

BehaviorAnalysis BehaviorAnalyzer::analyzeBehavior(const QString &address)
{
    BehaviorAnalysis analysis;
    analysis.address = address;
    analysis.hasRapidMintDump = detectRapidMintDump(address);
    analysis.hasGasFeeAnomalies = detectGasFeeAnomalies(address);
    analysis.hasSuspiciousTransfers = detectSuspiciousTransfers(address);
    analysis.riskScore = calculateRiskScore(address);
    analysis.analysisTime = QDateTime::currentDateTime();
    
    // Build anomalies list
    if (analysis.hasRapidMintDump) {
        analysis.anomalies.append("Rapid mint and dump detected");
    }
    if (analysis.hasGasFeeAnomalies) {
        analysis.anomalies.append("Unusual gas fee patterns");
    }
    if (analysis.hasSuspiciousTransfers) {
        analysis.anomalies.append("Suspicious transfer patterns");
    }
    
    return analysis;
}

bool BehaviorAnalyzer::detectRapidMintDump(const QString &address)
{
    // TODO: Implement blockchain analysis
    // This would analyze transaction history for rapid minting followed by dumping
    return false;
}

bool BehaviorAnalyzer::detectGasFeeAnomalies(const QString &address)
{
    // TODO: Implement gas fee analysis
    // This would analyze gas fee patterns for anomalies
    return false;
}

bool BehaviorAnalyzer::detectSuspiciousTransfers(const QString &address)
{
    // TODO: Implement transfer pattern analysis
    // This would analyze transfer patterns for suspicious activity
    return false;
}

double BehaviorAnalyzer::calculateRiskScore(const QString &address)
{
    double score = 0.0;
    
    if (detectRapidMintDump(address)) score += 0.4;
    if (detectGasFeeAnomalies(address)) score += 0.3;
    if (detectSuspiciousTransfers(address)) score += 0.3;
    
    return qMin(score, 1.0);
}

} // namespace SHAHSecurity
