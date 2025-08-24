#ifndef SHAHCOIN_QT_TROUBLESHOOTER_H
#define SHAHCOIN_QT_TROUBLESHOOTER_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QProgressBar>
#include <QTimer>
#include <QDialog>
#include <QTextBrowser>
#include <QCheckBox>

class WalletModel;
class CWallet;

/**
 * AI Troubleshooter for Shahcoin Qt Wallet
 *
 * Detects common wallet issues and provides intelligent solutions
 * and recommendations to resolve problems.
 */
class Troubleshooter : public QObject
{
    Q_OBJECT

public:
    enum IssueType {
        NoIssues,
        WalletLocked,
        LowPeerCount,
        SyncStuck,
        StakingInactive,
        LowBalance,
        NetworkIssues,
        FeeIssues,
        SecurityIssues,
        PerformanceIssues
    };

    enum IssueSeverity {
        Info,
        Warning,
        Error,
        Critical
    };

    struct Issue {
        IssueType type;
        IssueSeverity severity;
        QString title;
        QString description;
        QString solution;
        QStringList steps;
        bool isResolved;
        QDateTime detectedTime;
    };

    explicit Troubleshooter(QObject* parent = nullptr);
    ~Troubleshooter();

    // Troubleshooting
    void runDiagnostics();
    void checkWalletIssues();
    void checkNetworkIssues();
    void checkStakingIssues();
    void checkSecurityIssues();
    void checkPerformanceIssues();

    // Issue management
    QList<Issue> getDetectedIssues() const;
    QList<Issue> getResolvedIssues() const;
    void markIssueResolved(IssueType type);
    void clearResolvedIssues();

    // Settings
    void setAutoScan(bool enabled);
    void setScanInterval(int minutes);
    void setShowNotifications(bool enabled);
    void setAutoFix(bool enabled);

    bool isAutoScan() const;
    int getScanInterval() const;
    bool isShowNotifications() const;
    bool isAutoFix() const;

public Q_SLOTS:
    void startAutoScan();
    void stopAutoScan();
    void onScanTimeout();

Q_SIGNALS:
    void issueDetected(const Issue& issue);
    void issueResolved(const Issue& issue);
    void scanComplete(const QList<Issue>& issues);
    void diagnosticsComplete();

private Q_SLOTS:
    void onWalletDataChanged();

private:
    // Diagnostic functions
    bool checkWalletLocked();
    bool checkPeerCount();
    bool checkSyncStatus();
    bool checkStakingStatus();
    bool checkBalance();
    bool checkNetworkConnectivity();
    bool checkFeeEstimation();
    bool checkSecuritySettings();
    bool checkPerformanceMetrics();

    // Issue resolution
    void resolveWalletLocked();
    void resolveLowPeerCount();
    void resolveSyncStuck();
    void resolveStakingInactive();
    void resolveLowBalance();
    void resolveNetworkIssues();
    void resolveFeeIssues();
    void resolveSecurityIssues();
    void resolvePerformanceIssues();

    // Utility functions
    QString getIssueTitle(IssueType type) const;
    QString getIssueDescription(IssueType type) const;
    QString getIssueSolution(IssueType type) const;
    QStringList getIssueSteps(IssueType type) const;
    IssueSeverity getIssueSeverity(IssueType type) const;
    bool canAutoResolve(IssueType type) const;

    // Settings management
    void loadSettings();
    void saveSettings();

    // Data members
    WalletModel* m_walletModel;
    QTimer* m_scanTimer;
    QList<Issue> m_detectedIssues;
    QList<Issue> m_resolvedIssues;

    // Settings
    bool m_autoScan;
    int m_scanInterval;
    bool m_showNotifications;
    bool m_autoFix;
};

/**
 * Troubleshooter Dialog
 *
 * Main UI for the troubleshooter functionality
 */
class TroubleshooterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TroubleshooterDialog(Troubleshooter* troubleshooter, QWidget* parent = nullptr);

public Q_SLOTS:
    void runTroubleshooter();
    void showIssueDetails(QListWidgetItem* item);
    void resolveSelectedIssue();
    void exportReport();

private Q_SLOTS:
    void onScanComplete(const QList<Troubleshooter::Issue>& issues);
    void onIssueDetected(const Troubleshooter::Issue& issue);
    void onIssueResolved(const Troubleshooter::Issue& issue);

private:
    void setupUI();
    void setupConnections();
    void updateIssueList();
    void showIssueDialog(const Troubleshooter::Issue& issue);
    void updateStatusLabel();
    QString getSeverityColor(Troubleshooter::IssueSeverity severity) const;
    QString getSeverityIcon(Troubleshooter::IssueSeverity severity) const;

    Troubleshooter* m_troubleshooter;
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_statusGroup;
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    QPushButton* m_runButton;
    QPushButton* m_exportButton;
    QListWidget* m_issueList;
    QPushButton* m_resolveButton;
    QPushButton* m_detailsButton;
    QCheckBox* m_autoScanCheckBox;
    QLabel* m_summaryLabel;
};

/**
 * Issue Details Dialog
 *
 * Shows detailed information about a specific issue
 */
class IssueDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IssueDetailsDialog(const Troubleshooter::Issue& issue, QWidget* parent = nullptr);

private Q_SLOTS:
    void onResolveClicked();
    void onIgnoreClicked();

private:
    void setupUI();
    void populateIssueInfo();

    Troubleshooter::Issue m_issue;
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QLabel* m_severityLabel;
    QTextBrowser* m_descriptionBrowser;
    QTextBrowser* m_solutionBrowser;
    QListWidget* m_stepsList;
    QPushButton* m_resolveButton;
    QPushButton* m_ignoreButton;
    QPushButton* m_closeButton;
};

#endif // SHAHCOIN_QT_TROUBLESHOOTER_H
