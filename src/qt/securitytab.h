// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#ifndef SECURITYTAB_H
#define SECURITYTAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QProgressBar>
#include <QTimer>
#include <QToolTip>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

#include "../security/shahsecurity.h"

class SecurityTab : public QWidget
{
    Q_OBJECT

public:
    explicit SecurityTab(QWidget *parent = nullptr);
    ~SecurityTab();

    // Main interface functions
    void refreshSecurityData();
    void updateAssetSecurity(const QString &address);
    void showSecurityReport(const QString &address);
    void showRiskDetails(const QString &address);

    // Filter and search functions
    void filterByRiskLevel(SHAHSecurity::RiskLevel level);
    void filterByAssetType(SHAHSecurity::AssetType type);
    void searchAssets(const QString &searchTerm);
    void showOnlyTrustedAssets();

    // Settings functions
    void setRiskThreshold(SHAHSecurity::RiskLevel threshold);
    void setRefreshInterval(int minutes);
    void setAutoRefresh(bool enabled);

signals:
    void assetSelected(const QString &address);
    void securityReportSubmitted(const QString &reportId);
    void riskLevelChanged(const QString &address, SHAHSecurity::RiskLevel newRisk);

private slots:
    void onSecurityDataUpdated();
    void onAssetRiskChanged(const QString &address, SHAHSecurity::RiskLevel newRisk);
    void onPhishingDetected(const QString &url, const SHAHSecurity::PhishingDetectionResult &result);
    void onSuspiciousActivityDetected(const QString &address, const SHAHSecurity::BehaviorAnalysis &analysis);
    void onReportSubmitted(const QString &reportId);
    void onErrorOccurred(const QString &error);
    
    void onRefreshButtonClicked();
    void onScanWalletButtonClicked();
    void onReportAssetButtonClicked();
    void onViewRegistryButtonClicked();
    void onFilterChanged();
    void onSearchTextChanged();
    void onAssetTableItemClicked(QTableWidgetItem *item);
    void onAssetTableContextMenu(const QPoint &pos);
    void onAutoRefreshToggled(bool enabled);
    void onRefreshTimerTimeout();

private:
    // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // Risk Dashboard Tab
    QWidget *m_dashboardTab;
    QVBoxLayout *m_dashboardLayout;
    QGroupBox *m_summaryGroup;
    QGroupBox *m_alertsGroup;
    QGroupBox *m_statsGroup;
    
    // Asset Security Tab
    QWidget *m_assetsTab;
    QVBoxLayout *m_assetsLayout;
    QHBoxLayout *m_controlsLayout;
    QTableWidget *m_assetTable;
    QLineEdit *m_searchBox;
    QComboBox *m_riskFilter;
    QComboBox *m_typeFilter;
    QCheckBox *m_trustedOnlyCheck;
    QPushButton *m_refreshButton;
    QPushButton *m_scanWalletButton;
    
    // Reports Tab
    QWidget *m_reportsTab;
    QVBoxLayout *m_reportsLayout;
    QTableWidget *m_reportsTable;
    QPushButton *m_newReportButton;
    QPushButton *m_viewReportsButton;
    
    // Settings Tab
    QWidget *m_settingsTab;
    QVBoxLayout *m_settingsLayout;
    QGroupBox *m_generalSettings;
    QGroupBox *m_apiSettings;
    QGroupBox *m_notificationSettings;
    
    // Security Manager
    SHAHSecurity::SHAHSecurityManager *m_securityManager;
    
    // Data
    QList<SHAHSecurity::AssetSecurityInfo> m_assets;
    QList<SHAHSecurity::SecurityReport> m_reports;
    QMap<QString, SHAHSecurity::DeveloperReputation> m_developers;
    
    // Timers
    QTimer *m_refreshTimer;
    QTimer *m_autoRefreshTimer;
    
    // Settings
    bool m_autoRefreshEnabled;
    int m_refreshInterval;
    SHAHSecurity::RiskLevel m_riskThreshold;
    
    // UI Setup Functions
    void setupDashboardTab();
    void setupAssetsTab();
    void setupReportsTab();
    void setupSettingsTab();
    void setupAssetTable();
    void setupReportsTable();
    void setupControls();
    void setupSummaryWidgets();
    void setupAlertWidgets();
    void setupStatsWidgets();
    
    // Data Management Functions
    void loadAssetData();
    void loadReportData();
    void updateAssetTable();
    void updateReportsTable();
    void updateSummaryWidgets();
    void updateAlertWidgets();
    void updateStatsWidgets();
    void refreshAssetSecurity(const QString &address);
    
    // Utility Functions
    QString getRiskLevelString(SHAHSecurity::RiskLevel level) const;
    QString getAssetTypeString(SHAHSecurity::AssetType type) const;
    QString getBadgeIcon(SHAHSecurity::BadgeType type) const;
    QString getBadgeColor(SHAHSecurity::BadgeType type) const;
    QColor getRiskColor(SHAHSecurity::RiskLevel level) const;
    QString formatDateTime(const QDateTime &dateTime) const;
    QString formatAddress(const QString &address) const;
    
    // Dialog Functions
    void showSecurityReportDialog(const QString &address = QString());
    void showRiskDetailsDialog(const QString &address);
    void showAssetContextMenu(const QString &address, const QPoint &pos);
    void showConfirmationDialog(const QString &title, const QString &message);
    
    // Animation Functions
    void animateRiskChange(const QString &address, SHAHSecurity::RiskLevel oldRisk, SHAHSecurity::RiskLevel newRisk);
    void animateNewAlert(const QString &message);
    void animateRefreshComplete();
    
    // Helper Functions
    void createTableItem(const QString &text, const QColor &color = Qt::black);
    void createBadgeItem(const QString &icon, const QString &tooltip, const QColor &color);
    void createRiskProgressBar(double riskScore);
    void createActionButton(const QString &text, const QString &icon, const QString &tooltip);
    void setupContextMenu(QTableWidget *table);
    void setupTooltips();
    void setupConnections();
    
    // Validation Functions
    bool isValidAddress(const QString &address) const;
    bool isHighRiskAsset(const SHAHSecurity::AssetSecurityInfo &asset) const;
    bool isTrustedAsset(const SHAHSecurity::AssetSecurityInfo &asset) const;
    bool matchesSearchFilter(const SHAHSecurity::AssetSecurityInfo &asset, const QString &searchTerm) const;
    bool matchesRiskFilter(const SHAHSecurity::AssetSecurityInfo &asset, SHAHSecurity::RiskLevel filterLevel) const;
    bool matchesTypeFilter(const SHAHSecurity::AssetSecurityInfo &asset, SHAHSecurity::AssetType filterType) const;
    
    // Statistics Functions
    int getTotalAssets() const;
    int getVerifiedAssets() const;
    int getTrustedAssets() const;
    int getHighRiskAssets() const;
    int getMediumRiskAssets() const;
    int getLowRiskAssets() const;
    double getAverageRiskScore() const;
    int getActiveAlerts() const;
    int getPendingReports() const;
    
    // Alert Management
    void addAlert(const QString &message, const QString &type = "info");
    void clearAlerts();
    void dismissAlert(int index);
    void showAlertBanner(const QString &message, const QString &type = "warning");
    
    // Cache Management
    void saveSettings();
    void loadSettings();
    void clearCache();
    
    // Network Functions
    void checkApiConnection();
    void handleApiError(const QString &error);
    void retryFailedRequests();
    
    // Security Functions
    void validateSecurityData();
    void checkForSuspiciousActivity();
    void monitorAssetChanges();
    void generateSecurityReport();
    
    // UI State Management
    void setLoadingState(bool loading);
    void setErrorState(const QString &error);
    void setOfflineState(bool offline);
    void updateConnectionStatus();
    void updateLastRefreshTime();
    
    // Event Handlers
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    
    // Context Menu Actions
    void copyAddress();
    void viewOnExplorer();
    void reportAsset();
    void viewDetails();
    void addToWatchlist();
    void removeFromWatchlist();
    void refreshAsset();
    void showHistory();
    void exportData();
    
    // Settings Actions
    void saveGeneralSettings();
    void saveApiSettings();
    void saveNotificationSettings();
    void resetToDefaults();
    void importSettings();
    void exportSettings();
    
    // Report Actions
    void createNewReport();
    void editReport(const QString &reportId);
    void deleteReport(const QString &reportId);
    void submitReport(const SHAHSecurity::SecurityReport &report);
    void viewReportDetails(const QString &reportId);
    void exportReports();
    
    // Filter Actions
    void applyFilters();
    void clearFilters();
    void saveFilterPreset(const QString &name);
    void loadFilterPreset(const QString &name);
    void deleteFilterPreset(const QString &name);
    
    // Search Actions
    void performSearch();
    void clearSearch();
    void saveSearchHistory();
    void loadSearchHistory();
    void clearSearchHistory();
    
    // Refresh Actions
    void performRefresh();
    void cancelRefresh();
    void scheduleRefresh(int delaySeconds);
    void updateRefreshProgress(int progress);
    
    // Animation Actions
    void startLoadingAnimation();
    void stopLoadingAnimation();
    void startPulseAnimation(QWidget *widget);
    void stopPulseAnimation(QWidget *widget);
    void startFadeAnimation(QWidget *widget, bool fadeIn);
    
    // Utility Actions
    void copyToClipboard(const QString &text);
    void openUrl(const QString &url);
    void showHelp();
    void showAbout();
    void checkForUpdates();
    void backupData();
    void restoreData();
    
    // Debug Functions
    void logSecurityEvent(const QString &event, const QString &details);
    void dumpSecurityData();
    void validateDataIntegrity();
    void performanceTest();
    
    // Constants
    static const int REFRESH_INTERVAL_DEFAULT = 720; // 12 hours
    static const int AUTO_REFRESH_INTERVAL = 3600; // 1 hour
    static const int CACHE_EXPIRY_TIME = 3600; // 1 hour
    static const int MAX_SEARCH_HISTORY = 50;
    static const int MAX_FILTER_PRESETS = 10;
    static const int MAX_ALERTS = 100;
    static const int ANIMATION_DURATION = 300; // milliseconds
    
    // Column indices for tables
    enum AssetTableColumns {
        COL_NAME = 0,
        COL_TYPE = 1,
        COL_RISK_SCORE = 2, // textual risk (Safe/Caution/Risky)
        COL_SCORE_NUMERIC = 3, // numeric 0..100
        COL_BADGE = 4,
        COL_LAST_VERIFIED = 5,
        COL_CREATOR_STATUS = 6,
        COL_ACTIONS = 7
    };
    
    enum ReportsTableColumns {
        COL_REPORT_ID = 0,
        COL_ASSET_ADDRESS = 1,
        COL_REPORT_TYPE = 2,
        COL_DESCRIPTION = 3,
        COL_REPORT_TIME = 4,
        COL_STATUS = 5,
        COL_ACTIONS = 6
    };
};

#endif // SECURITYTAB_H
