// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#ifndef SHAHSECURITY_H
#define SHAHSECURITY_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <QMap>
#include <QString>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>

// Forward declarations
class QNetworkAccessManager;
class QNetworkReply;

namespace SHAHSecurity {

    // Risk levels
    enum RiskLevel {
        RiskLow = 0,
        RiskMedium = 1,
        RiskHigh = 2,
        RiskVerified = 3,
        RiskUnknown = 4
    };

    // Asset types
    enum AssetType {
        AssetToken = 0,
        AssetNFT = 1,
        AssetContract = 2,
        AssetCollection = 3
    };

    // Security badge types
    enum BadgeType {
        BadgeVerified = 0,
        BadgeTrusted = 1,
        BadgeWarning = 2,
        BadgeDanger = 3,
        BadgeScam = 4
    };

    // Asset security information
    struct AssetSecurityInfo {
        QString address;
        QString name;
        QString symbol;
        AssetType type;
        RiskLevel riskLevel;
        BadgeType badgeType;
        QString badgeIcon;
        QString riskDescription;
        QString detailsUrl;
        QString creatorAddress;
        QString creatorName;
        bool isVerified;
        bool isTrusted;
        int reputationScore;
        QDateTime lastVerified;
        QDateTime lastUpdated;
        QStringList warnings;
        QStringList flags;
        QJsonObject metadata;
    };

    // Developer reputation information
    struct DeveloperReputation {
        QString address;
        QString name;
        int mintCount;
        int refundRequests;
        int flaggedContent;
        int totalAssets;
        double reputationScore;
        bool isTrusted;
        QDateTime lastActivity;
        QStringList verifiedAssets;
        QStringList warnings;
    };

    // Phishing detection result
    struct PhishingDetectionResult {
        bool isPhishing;
        double confidence;
        QStringList detectedPatterns;
        QStringList lookalikeDomains;
        QStringList fakeMintLinks;
        QString recommendation;
    };

    // On-chain behavior analysis
    struct BehaviorAnalysis {
        QString address;
        bool hasRapidMintDump;
        bool hasGasFeeAnomalies;
        bool hasSuspiciousTransfers;
        double riskScore;
        QStringList anomalies;
        QDateTime analysisTime;
    };

    // Report information
    struct SecurityReport {
        QString reportId;
        QString reporterAddress;
        QString assetAddress;
        QString reportType;
        QString description;
        QStringList evidence;
        QDateTime reportTime;
        bool isResolved;
        QString resolution;
    };

    // Main SHAHSecurity Manager
    class SHAHSecurityManager : public QObject
    {
        Q_OBJECT

    public:
        explicit SHAHSecurityManager(QObject *parent = nullptr);
        ~SHAHSecurityManager();

        // Core security functions
        AssetSecurityInfo getAssetSecurityInfo(const QString &address);
        QList<AssetSecurityInfo> getAssetSecurityInfos(const QStringList &addresses);
        DeveloperReputation getDeveloperReputation(const QString &address);
        PhishingDetectionResult detectPhishing(const QString &url, const QString &content);
        BehaviorAnalysis analyzeOnChainBehavior(const QString &address);
        
        // Report system
        QString submitSecurityReport(const SecurityReport &report);
        QList<SecurityReport> getReportsForAsset(const QString &address);
        
        // Legal protection
        bool checkTrademarkViolation(const QString &name, const QString &symbol);
        QStringList getProtectedTerms();
        
        // Cache management
        void clearCache();
        void refreshSecurityData();
        
        // Settings
        void setApiEndpoint(const QString &endpoint);
        void setRefreshInterval(int minutes);
        void setRiskThreshold(RiskLevel threshold);
        
        // Status
        bool isOnline() const;
        QString getLastError() const;
        QList<AssetSecurityInfo> listCachedAssets() const;

        // Scoring API
        int computeSecurityScore(const AssetSecurityInfo &info) const; // 0..100
        QString badgeForScore(int score) const; // "safe" | "caution" | "risky"
        QString tooltipFor(const AssetSecurityInfo &info, int score) const;
        QList<AssetSecurityInfo> listCachedAssets() const;

    Q_SIGNALS:
        void securityDataUpdated();
        void assetRiskChanged(const QString &address, RiskLevel newRisk);
        void phishingDetected(const QString &url, const PhishingDetectionResult &result);
        void suspiciousActivityDetected(const QString &address, const BehaviorAnalysis &analysis);
        void reportSubmitted(const QString &reportId);
        void errorOccurred(const QString &error);

    private:
        void onNetworkReplyFinished();
        void onRefreshTimerTimeout();
        void onCacheExpired();

    private:
        // Network management
        QNetworkAccessManager *m_networkManager;
        QString m_apiEndpoint;
        QString m_lastError;
        bool m_isOnline;
        
        // Caching
        QMap<QString, AssetSecurityInfo> m_assetCache;
        QMap<QString, DeveloperReputation> m_developerCache;
        QMap<QString, BehaviorAnalysis> m_behaviorCache;
        QMutex m_cacheMutex;
        
        // Timers
        QTimer *m_refreshTimer;
        QTimer *m_cacheTimer;
        
        // Settings
        int m_refreshInterval;
        RiskLevel m_riskThreshold;
        
        // Internal functions
        void initializeNetwork();
        void initializeTimers();
        void loadCachedData();
        void saveCachedData();
        void updateAssetSecurity(const QString &address);
        void updateAssetSecurities(const QStringList &addresses);
        void updateDeveloperReputation(const QString &address);
        void analyzeBehavior(const QString &address);
        QString getRiskLevelString(RiskLevel level) const;
        QString getBadgeIcon(BadgeType type) const;
        QString getBadgeColor(BadgeType type) const;
        bool isValidAddress(const QString &address) const;
        void logSecurityEvent(const QString &event, const QString &details);

        // Local registries
        struct LocalRegistries { QJsonObject official; QJsonObject phishing; QJsonObject bots; bool loaded{false}; };
        mutable LocalRegistries m_local;
        void loadLocalRegistries() const;
        bool isOfficialOrTrusted(const QString &address, AssetSecurityInfo &info) const;
    };

    // Phishing Detector
    class PhishingDetector : public QObject
    {

    public:
        explicit PhishingDetector(QObject *parent = nullptr);
        ~PhishingDetector();

        PhishingDetectionResult detectPhishing(const QString &url, const QString &content);
        void addPhishingPattern(const QString &pattern);
        void addLookalikeDomain(const QString &domain);
        void addFakeMintLink(const QString &link);
        void loadPhishingDatabase();

    private:
        QStringList m_phishingPatterns;
        QStringList m_lookalikeDomains;
        QStringList m_fakeMintLinks;
        QStringList m_legitimateDomains;
        
        bool checkUrlPatterns(const QString &url);
        bool checkContentPatterns(const QString &content);
        bool checkLookalikeDomains(const QString &url);
        bool checkFakeMintLinks(const QString &url);
        double calculateConfidence(const QString &url, const QString &content);
    };

    // On-chain Behavior Analyzer
    class BehaviorAnalyzer : public QObject
    {

    public:
        explicit BehaviorAnalyzer(QObject *parent = nullptr);
        ~BehaviorAnalyzer();

        BehaviorAnalysis analyzeBehavior(const QString &address);
        void setAnalysisParameters(double mintDumpThreshold, double gasAnomalyThreshold);
        void addSuspiciousPattern(const QString &pattern);

    private:
        double m_mintDumpThreshold;
        double m_gasAnomalyThreshold;
        QStringList m_suspiciousPatterns;
        
        bool detectRapidMintDump(const QString &address);
        bool detectGasFeeAnomalies(const QString &address);
        bool detectSuspiciousTransfers(const QString &address);
        double calculateRiskScore(const QString &address);
    };

    // Developer Reputation System
    class DeveloperReputationSystem : public QObject
    {

    public:
        explicit DeveloperReputationSystem(QObject *parent = nullptr);
        ~DeveloperReputationSystem();

        DeveloperReputation getReputation(const QString &address);
        void updateReputation(const QString &address, int mintCount, int refunds, int flags);
        void markAsTrusted(const QString &address);
        void addWarning(const QString &address, const QString &warning);
        double calculateReputationScore(int mintCount, int refunds, int flags);

    private:
        QMap<QString, DeveloperReputation> m_reputationCache;
        QMutex m_reputationMutex;
        
        void loadReputationData();
        void saveReputationData();
        void updateReputationScore(const QString &address);
    };

    // Legal Protection System
    class LegalProtectionSystem : public QObject
    {

    public:
        explicit LegalProtectionSystem(QObject *parent = nullptr);
        ~LegalProtectionSystem();

        bool checkTrademarkViolation(const QString &name, const QString &symbol);
        QStringList getProtectedTerms();
        void addProtectedTerm(const QString &term);
        void removeProtectedTerm(const QString &term);
        QString generateLegalWarning(const QString &violation);

    private:
        QStringList m_protectedTerms;
        QStringList m_protectedLogos;
        QStringList m_authorizedUsers;
        
        bool checkNameViolation(const QString &name);
        bool checkSymbolViolation(const QString &symbol);
        bool checkLogoViolation(const QString &logoUrl);
        bool isAuthorizedUser(const QString &address);
    };

    // Audit Log System
    class AuditLogSystem : public QObject
    {

    public:
        explicit AuditLogSystem(QObject *parent = nullptr);
        ~AuditLogSystem();

        void logSecurityEvent(const QString &event, const QString &details, const QString &address = QString());
        void logBadgeUpdate(const QString &address, BadgeType oldBadge, BadgeType newBadge);
        void logRiskUpdate(const QString &address, RiskLevel oldRisk, RiskLevel newRisk);
        void logReportSubmission(const QString &reportId, const QString &reporter);
        QList<QJsonObject> getAuditLog(const QString &address = QString(), int limit = 100);

    private:
        QString m_logFilePath;
        QMutex m_logMutex;
        
        void writeLogEntry(const QJsonObject &entry);
        QJsonObject createLogEntry(const QString &event, const QString &details, const QString &address);
        QString getTimestamp();
    };

} // namespace SHAHSecurity

#endif // SHAHSECURITY_H
