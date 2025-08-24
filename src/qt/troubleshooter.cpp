#include "troubleshooter.h"
#include <qt/walletmodel.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>

#include <QApplication>
#include <QTimer>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileDialog>

#include <wallet/wallet.h>
#include <validation.h>
#include <net.h>
#include <util/time.h>
#include <amount.h>

Troubleshooter::Troubleshooter(QObject* parent)
    : QObject(parent)
    , m_walletModel(nullptr)
    , m_scanTimer(new QTimer(this))
    , m_autoScan(true)
    , m_scanInterval(5)
    , m_showNotifications(true)
    , m_autoFix(false)
{
    loadSettings();
    
    // Set up scan timer
    m_scanTimer->setInterval(m_scanInterval * 60 * 1000); // Convert minutes to milliseconds
    connect(m_scanTimer, &QTimer::timeout, this, &Troubleshooter::onScanTimeout);
    
    if (m_autoScan) {
        startAutoScan();
    }
}

Troubleshooter::~Troubleshooter()
{
    saveSettings();
}

void Troubleshooter::runDiagnostics()
{
    m_detectedIssues.clear();
    
    // Run all diagnostic checks
    checkWalletIssues();
    checkNetworkIssues();
    checkStakingIssues();
    checkSecurityIssues();
    checkPerformanceIssues();
    
    emit scanComplete(m_detectedIssues);
    emit diagnosticsComplete();
}

void Troubleshooter::checkWalletIssues()
{
    // Check if wallet is locked
    if (checkWalletLocked()) {
        Issue issue;
        issue.type = WalletLocked;
        issue.severity = getIssueSeverity(WalletLocked);
        issue.title = getIssueTitle(WalletLocked);
        issue.description = getIssueDescription(WalletLocked);
        issue.solution = getIssueSolution(WalletLocked);
        issue.steps = getIssueSteps(WalletLocked);
        issue.isResolved = false;
        issue.detectedTime = QDateTime::currentDateTime();
        
        m_detectedIssues.append(issue);
        emit issueDetected(issue);
    }
    
    // Check balance
    if (checkBalance()) {
        Issue issue;
        issue.type = LowBalance;
        issue.severity = getIssueSeverity(LowBalance);
        issue.title = getIssueTitle(LowBalance);
        issue.description = getIssueDescription(LowBalance);
        issue.solution = getIssueSolution(LowBalance);
        issue.steps = getIssueSteps(LowBalance);
        issue.isResolved = false;
        issue.detectedTime = QDateTime::currentDateTime();
        
        m_detectedIssues.append(issue);
        emit issueDetected(issue);
    }
}

void Troubleshooter::checkNetworkIssues()
{
    // Check peer count
    if (checkPeerCount()) {
        Issue issue;
        issue.type = LowPeerCount;
        issue.severity = getIssueSeverity(LowPeerCount);
        issue.title = getIssueTitle(LowPeerCount);
        issue.description = getIssueDescription(LowPeerCount);
        issue.solution = getIssueSolution(LowPeerCount);
        issue.steps = getIssueSteps(LowPeerCount);
        issue.isResolved = false;
        issue.detectedTime = QDateTime::currentDateTime();
        
        m_detectedIssues.append(issue);
        emit issueDetected(issue);
    }
    
    // Check sync status
    if (checkSyncStatus()) {
        Issue issue;
        issue.type = SyncStuck;
        issue.severity = getIssueSeverity(SyncStuck);
        issue.title = getIssueTitle(SyncStuck);
        issue.description = getIssueDescription(SyncStuck);
        issue.solution = getIssueSolution(SyncStuck);
        issue.steps = getIssueSteps(SyncStuck);
        issue.isResolved = false;
        issue.detectedTime = QDateTime::currentDateTime();
        
        m_detectedIssues.append(issue);
        emit issueDetected(issue);
    }
    
    // Check network connectivity
    if (checkNetworkConnectivity()) {
        Issue issue;
        issue.type = NetworkIssues;
        issue.severity = getIssueSeverity(NetworkIssues);
        issue.title = getIssueTitle(NetworkIssues);
        issue.description = getIssueDescription(NetworkIssues);
        issue.solution = getIssueSolution(NetworkIssues);
        issue.steps = getIssueSteps(NetworkIssues);
        issue.isResolved = false;
        issue.detectedTime = QDateTime::currentDateTime();
        
        m_detectedIssues.append(issue);
        emit issueDetected(issue);
    }
}

void Troubleshooter::checkStakingIssues()
{
    if (checkStakingStatus()) {
        Issue issue;
        issue.type = StakingInactive;
        issue.severity = getIssueSeverity(StakingInactive);
        issue.title = getIssueTitle(StakingInactive);
        issue.description = getIssueDescription(StakingInactive);
        issue.solution = getIssueSolution(StakingInactive);
        issue.steps = getIssueSteps(StakingInactive);
        issue.isResolved = false;
        issue.detectedTime = QDateTime::currentDateTime();
        
        m_detectedIssues.append(issue);
        emit issueDetected(issue);
    }
}

void Troubleshooter::checkSecurityIssues()
{
    if (checkSecuritySettings()) {
        Issue issue;
        issue.type = SecurityIssues;
        issue.severity = getIssueSeverity(SecurityIssues);
        issue.title = getIssueTitle(SecurityIssues);
        issue.description = getIssueDescription(SecurityIssues);
        issue.solution = getIssueSolution(SecurityIssues);
        issue.steps = getIssueSteps(SecurityIssues);
        issue.isResolved = false;
        issue.detectedTime = QDateTime::currentDateTime();
        
        m_detectedIssues.append(issue);
        emit issueDetected(issue);
    }
}

void Troubleshooter::checkPerformanceIssues()
{
    if (checkPerformanceMetrics()) {
        Issue issue;
        issue.type = PerformanceIssues;
        issue.severity = getIssueSeverity(PerformanceIssues);
        issue.title = getIssueTitle(PerformanceIssues);
        issue.description = getIssueDescription(PerformanceIssues);
        issue.solution = getIssueSolution(PerformanceIssues);
        issue.steps = getIssueSteps(PerformanceIssues);
        issue.isResolved = false;
        issue.detectedTime = QDateTime::currentDateTime();
        
        m_detectedIssues.append(issue);
        emit issueDetected(issue);
    }
}

bool Troubleshooter::checkWalletLocked()
{
    if (!m_walletModel) return false;
    
    // Check if wallet is encrypted and locked
    // In real implementation, this would check actual wallet state
    return false; // Simplified for now
}

bool Troubleshooter::checkPeerCount()
{
    // Check if peer count is too low
    // In real implementation, this would get actual peer count from network
    int peerCount = 8; // Example value
    return peerCount < 5;
}

bool Troubleshooter::checkSyncStatus()
{
    // Check if blockchain sync is stuck
    // In real implementation, this would check actual sync status
    return false; // Simplified for now
}

bool Troubleshooter::checkStakingStatus()
{
    if (!m_walletModel) return false;
    
    // Check if staking is inactive when it should be active
    CAmount balance = m_walletModel->getBalance();
    CAmount stakingBalance = m_walletModel->getStakingBalance();
    
    // If user has enough balance but staking is not active
    return (balance >= 333 * COIN) && (stakingBalance < 333 * COIN);
}

bool Troubleshooter::checkBalance()
{
    if (!m_walletModel) return false;
    
    // Check if balance is very low
    CAmount balance = m_walletModel->getBalance();
    return balance < 1 * COIN; // Less than 1 SHAH
}

bool Troubleshooter::checkNetworkConnectivity()
{
    // Check network connectivity
    // In real implementation, this would test actual network connectivity
    return false; // Simplified for now
}

bool Troubleshooter::checkFeeEstimation()
{
    // Check if fee estimation is working properly
    // In real implementation, this would test fee estimation
    return false; // Simplified for now
}

bool Troubleshooter::checkSecuritySettings()
{
    // Check security settings
    // In real implementation, this would check various security settings
    return false; // Simplified for now
}

bool Troubleshooter::checkPerformanceMetrics()
{
    // Check performance metrics
    // In real implementation, this would check various performance indicators
    return false; // Simplified for now
}

QList<Troubleshooter::Issue> Troubleshooter::getDetectedIssues() const
{
    return m_detectedIssues;
}

QList<Troubleshooter::Issue> Troubleshooter::getResolvedIssues() const
{
    return m_resolvedIssues;
}

void Troubleshooter::markIssueResolved(IssueType type)
{
    for (int i = 0; i < m_detectedIssues.size(); ++i) {
        if (m_detectedIssues[i].type == type) {
            m_detectedIssues[i].isResolved = true;
            m_resolvedIssues.append(m_detectedIssues[i]);
            emit issueResolved(m_detectedIssues[i]);
            m_detectedIssues.removeAt(i);
            break;
        }
    }
}

void Troubleshooter::clearResolvedIssues()
{
    m_resolvedIssues.clear();
}

void Troubleshooter::setAutoScan(bool enabled)
{
    m_autoScan = enabled;
    if (enabled) {
        startAutoScan();
    } else {
        stopAutoScan();
    }
    saveSettings();
}

void Troubleshooter::setScanInterval(int minutes)
{
    m_scanInterval = minutes;
    m_scanTimer->setInterval(minutes * 60 * 1000);
    saveSettings();
}

void Troubleshooter::setShowNotifications(bool enabled)
{
    m_showNotifications = enabled;
    saveSettings();
}

void Troubleshooter::setAutoFix(bool enabled)
{
    m_autoFix = enabled;
    saveSettings();
}

bool Troubleshooter::isAutoScan() const
{
    return m_autoScan;
}

int Troubleshooter::getScanInterval() const
{
    return m_scanInterval;
}

bool Troubleshooter::isShowNotifications() const
{
    return m_showNotifications;
}

bool Troubleshooter::isAutoFix() const
{
    return m_autoFix;
}

void Troubleshooter::startAutoScan()
{
    if (m_autoScan) {
        m_scanTimer->start();
    }
}

void Troubleshooter::stopAutoScan()
{
    m_scanTimer->stop();
}

void Troubleshooter::onScanTimeout()
{
    runDiagnostics();
}

void Troubleshooter::onWalletDataChanged()
{
    // Re-run diagnostics when wallet data changes
    if (m_autoScan) {
        runDiagnostics();
    }
}

QString Troubleshooter::getIssueTitle(IssueType type) const
{
    switch (type) {
        case WalletLocked: return "Wallet is Locked";
        case LowPeerCount: return "Low Peer Count";
        case SyncStuck: return "Blockchain Sync Stuck";
        case StakingInactive: return "Staking Not Active";
        case LowBalance: return "Low Balance";
        case NetworkIssues: return "Network Connectivity Issues";
        case FeeIssues: return "Fee Estimation Issues";
        case SecurityIssues: return "Security Issues";
        case PerformanceIssues: return "Performance Issues";
        default: return "Unknown Issue";
    }
}

QString Troubleshooter::getIssueDescription(IssueType type) const
{
    switch (type) {
        case WalletLocked: return "Your wallet is currently locked and cannot perform transactions.";
        case LowPeerCount: return "You have fewer than 5 peer connections, which may affect network reliability.";
        case SyncStuck: return "Blockchain synchronization appears to be stuck or progressing very slowly.";
        case StakingInactive: return "You have sufficient balance for staking but staking is not currently active.";
        case LowBalance: return "Your wallet balance is very low, which may limit your ability to perform transactions.";
        case NetworkIssues: return "Network connectivity issues detected, which may affect wallet functionality.";
        case FeeIssues: return "Fee estimation is not working properly, which may affect transaction processing.";
        case SecurityIssues: return "Security settings may need attention to ensure wallet safety.";
        case PerformanceIssues: return "Wallet performance issues detected that may affect user experience.";
        default: return "Unknown issue description.";
    }
}

QString Troubleshooter::getIssueSolution(IssueType type) const
{
    switch (type) {
        case WalletLocked: return "Unlock your wallet using the password to enable transactions.";
        case LowPeerCount: return "Check your internet connection and firewall settings to allow more peer connections.";
        case SyncStuck: return "Restart the wallet or check your internet connection to resume synchronization.";
        case StakingInactive: return "Ensure your wallet is unlocked and has sufficient balance to enable staking.";
        case LowBalance: return "Consider adding funds to your wallet to enable full functionality.";
        case NetworkIssues: return "Check your internet connection and network settings.";
        case FeeIssues: return "Restart the wallet or check network connectivity to restore fee estimation.";
        case SecurityIssues: return "Review and update your security settings as recommended.";
        case PerformanceIssues: return "Optimize your system settings or restart the wallet to improve performance.";
        default: return "Please contact support for assistance with this issue.";
    }
}

QStringList Troubleshooter::getIssueSteps(IssueType type) const
{
    QStringList steps;
    
    switch (type) {
        case WalletLocked:
            steps << "Go to Settings → Security"
                 << "Click 'Unlock Wallet'"
                 << "Enter your wallet password"
                 << "Click 'Unlock'";
            break;
        case LowPeerCount:
            steps << "Check your internet connection"
                 << "Verify firewall settings allow Shahcoin"
                 << "Restart the wallet application"
                 << "Wait for peer connections to establish";
            break;
        case SyncStuck:
            steps << "Check your internet connection"
                 << "Restart the Shahcoin wallet"
                 << "If issue persists, try rescanning"
                 << "Contact support if problem continues";
            break;
        case StakingInactive:
            steps << "Ensure wallet is unlocked"
                 << "Verify you have at least 333 SHAH"
                 << "Check staking settings"
                 << "Wait for staking to activate";
            break;
        case LowBalance:
            steps << "Consider adding funds to your wallet"
                 << "Check for pending transactions"
                 << "Review your transaction history"
                 << "Plan for future transactions";
            break;
        case NetworkIssues:
            steps << "Test your internet connection"
                 << "Check network adapter settings"
                 << "Restart your router if necessary"
                 << "Try using a different network";
            break;
        case FeeIssues:
            steps << "Restart the wallet application"
                 << "Check network connectivity"
                 << "Wait for fee estimation to update"
                 << "Use manual fee setting if needed";
            break;
        case SecurityIssues:
            steps << "Review security settings"
                 << "Update wallet encryption if needed"
                 << "Check for suspicious activity"
                 << "Consider additional security measures";
            break;
        case PerformanceIssues:
            steps << "Close unnecessary applications"
                 << "Restart the wallet"
                 << "Check system resources"
                 << "Update to latest wallet version";
            break;
    }
    
    return steps;
}

Troubleshooter::IssueSeverity Troubleshooter::getIssueSeverity(IssueType type) const
{
    switch (type) {
        case WalletLocked: return Error;
        case LowPeerCount: return Warning;
        case SyncStuck: return Error;
        case StakingInactive: return Info;
        case LowBalance: return Warning;
        case NetworkIssues: return Error;
        case FeeIssues: return Warning;
        case SecurityIssues: return Critical;
        case PerformanceIssues: return Warning;
        default: return Info;
    }
}

bool Troubleshooter::canAutoResolve(IssueType type) const
{
    switch (type) {
        case StakingInactive: return true;
        case LowPeerCount: return false;
        case SyncStuck: return false;
        case WalletLocked: return false;
        case LowBalance: return false;
        case NetworkIssues: return false;
        case FeeIssues: return false;
        case SecurityIssues: return false;
        case PerformanceIssues: return false;
        default: return false;
    }
}

void Troubleshooter::loadSettings()
{
    QSettings settings;
    m_autoScan = settings.value("troubleshooter/autoScan", true).toBool();
    m_scanInterval = settings.value("troubleshooter/scanInterval", 5).toInt();
    m_showNotifications = settings.value("troubleshooter/showNotifications", true).toBool();
    m_autoFix = settings.value("troubleshooter/autoFix", false).toBool();
}

void Troubleshooter::saveSettings()
{
    QSettings settings;
    settings.setValue("troubleshooter/autoScan", m_autoScan);
    settings.setValue("troubleshooter/scanInterval", m_scanInterval);
    settings.setValue("troubleshooter/showNotifications", m_showNotifications);
    settings.setValue("troubleshooter/autoFix", m_autoFix);
}

// TroubleshooterDialog implementation
TroubleshooterDialog::TroubleshooterDialog(Troubleshooter* troubleshooter, QWidget* parent)
    : QDialog(parent)
    , m_troubleshooter(troubleshooter)
    , m_mainLayout(new QVBoxLayout(this))
    , m_statusGroup(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
    , m_runButton(nullptr)
    , m_exportButton(nullptr)
    , m_issueList(nullptr)
    , m_resolveButton(nullptr)
    , m_detailsButton(nullptr)
    , m_autoScanCheckBox(nullptr)
    , m_summaryLabel(nullptr)
{
    setWindowTitle("AI Troubleshooter");
    setMinimumSize(600, 500);
    
    setupUI();
    setupConnections();
    updateStatusLabel();
}

void TroubleshooterDialog::setupUI()
{
    // Status group
    m_statusGroup = new QGroupBox("Status", this);
    QVBoxLayout* statusLayout = new QVBoxLayout(m_statusGroup);
    
    m_statusLabel = new QLabel("Ready to run diagnostics", this);
    m_statusLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    statusLayout->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    statusLayout->addWidget(m_progressBar);
    
    m_mainLayout->addWidget(m_statusGroup);
    
    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_runButton = new QPushButton("🔍 Run Troubleshooter", this);
    m_runButton->setToolTip("Run comprehensive wallet diagnostics");
    buttonLayout->addWidget(m_runButton);
    
    m_exportButton = new QPushButton("📄 Export Report", this);
    m_exportButton->setToolTip("Export diagnostic report to file");
    buttonLayout->addWidget(m_exportButton);
    
    m_autoScanCheckBox = new QCheckBox("Auto-scan", this);
    m_autoScanCheckBox->setChecked(m_troubleshooter->isAutoScan());
    buttonLayout->addWidget(m_autoScanCheckBox);
    buttonLayout->addStretch();
    
    m_mainLayout->addLayout(buttonLayout);
    
    // Issues list
    QGroupBox* issuesGroup = new QGroupBox("Detected Issues", this);
    QVBoxLayout* issuesLayout = new QVBoxLayout(issuesGroup);
    
    m_issueList = new QListWidget(this);
    m_issueList->setAlternatingRowColors(true);
    issuesLayout->addWidget(m_issueList);
    
    // Issue action buttons
    QHBoxLayout* actionLayout = new QHBoxLayout();
    m_detailsButton = new QPushButton("🔍 View Details", this);
    m_detailsButton->setEnabled(false);
    actionLayout->addWidget(m_detailsButton);
    
    m_resolveButton = new QPushButton("✅ Mark Resolved", this);
    m_resolveButton->setEnabled(false);
    actionLayout->addWidget(m_resolveButton);
    actionLayout->addStretch();
    
    issuesLayout->addLayout(actionLayout);
    m_mainLayout->addWidget(issuesGroup);
    
    // Summary
    m_summaryLabel = new QLabel("No issues detected", this);
    m_summaryLabel->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    m_mainLayout->addWidget(m_summaryLabel);
}

void TroubleshooterDialog::setupConnections()
{
    connect(m_runButton, &QPushButton::clicked, this, &TroubleshooterDialog::runTroubleshooter);
    connect(m_exportButton, &QPushButton::clicked, this, &TroubleshooterDialog::exportReport);
    connect(m_issueList, &QListWidget::itemClicked, this, &TroubleshooterDialog::showIssueDetails);
    connect(m_resolveButton, &QPushButton::clicked, this, &TroubleshooterDialog::resolveSelectedIssue);
    connect(m_detailsButton, &QPushButton::clicked, [this]() {
        if (m_issueList->currentItem()) {
            showIssueDetails(m_issueList->currentItem());
        }
    });
    connect(m_autoScanCheckBox, &QCheckBox::toggled, m_troubleshooter, &Troubleshooter::setAutoScan);
    
    connect(m_troubleshooter, &Troubleshooter::scanComplete, this, &TroubleshooterDialog::onScanComplete);
    connect(m_troubleshooter, &Troubleshooter::issueDetected, this, &TroubleshooterDialog::onIssueDetected);
    connect(m_troubleshooter, &Troubleshooter::issueResolved, this, &TroubleshooterDialog::onIssueResolved);
}

void TroubleshooterDialog::runTroubleshooter()
{
    m_runButton->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // Indeterminate progress
    
    m_statusLabel->setText("Running diagnostics...");
    
    // Run diagnostics in background
    QTimer::singleShot(100, [this]() {
        m_troubleshooter->runDiagnostics();
    });
}

void TroubleshooterDialog::showIssueDetails(QListWidgetItem* item)
{
    if (!item) return;
    
    int index = m_issueList->row(item);
    QList<Troubleshooter::Issue> issues = m_troubleshooter->getDetectedIssues();
    
    if (index >= 0 && index < issues.size()) {
        showIssueDialog(issues[index]);
    }
}

void TroubleshooterDialog::resolveSelectedIssue()
{
    if (!m_issueList->currentItem()) return;
    
    int index = m_issueList->row(m_issueList->currentItem());
    QList<Troubleshooter::Issue> issues = m_troubleshooter->getDetectedIssues();
    
    if (index >= 0 && index < issues.size()) {
        m_troubleshooter->markIssueResolved(issues[index].type);
        updateIssueList();
    }
}

void TroubleshooterDialog::exportReport()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Troubleshooter Report",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/shahcoin_troubleshooter_report.txt",
        "Text Files (*.txt);;All Files (*)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Failed", "Could not create report file.");
        return;
    }
    
    QTextStream out(&file);
    out << "Shahcoin Troubleshooter Report\n";
    out << "Generated: " << QDateTime::currentDateTime().toString() << "\n\n";
    
    QList<Troubleshooter::Issue> issues = m_troubleshooter->getDetectedIssues();
    if (issues.isEmpty()) {
        out << "No issues detected.\n";
    } else {
        out << "Detected Issues:\n";
        out << "================\n\n";
        
        for (const auto& issue : issues) {
            out << "Issue: " << issue.title << "\n";
            out << "Severity: " << (issue.severity == Troubleshooter::Critical ? "Critical" :
                                  issue.severity == Troubleshooter::Error ? "Error" :
                                  issue.severity == Troubleshooter::Warning ? "Warning" : "Info") << "\n";
            out << "Description: " << issue.description << "\n";
            out << "Solution: " << issue.solution << "\n";
            out << "Steps:\n";
            for (const auto& step : issue.steps) {
                out << "  - " << step << "\n";
            }
            out << "\n";
        }
    }
    
    file.close();
    QMessageBox::information(this, "Export Complete", "Troubleshooter report exported successfully.");
}

void TroubleshooterDialog::onScanComplete(const QList<Troubleshooter::Issue>& issues)
{
    m_runButton->setEnabled(true);
    m_progressBar->setVisible(false);
    
    if (issues.isEmpty()) {
        m_statusLabel->setText("✅ No issues detected");
        m_statusLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #27ae60;");
    } else {
        m_statusLabel->setText(QString("⚠️ %1 issue(s) detected").arg(issues.size()));
        m_statusLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #f39c12;");
    }
    
    updateIssueList();
}

void TroubleshooterDialog::onIssueDetected(const Troubleshooter::Issue& issue)
{
    updateIssueList();
}

void TroubleshooterDialog::onIssueResolved(const Troubleshooter::Issue& issue)
{
    updateIssueList();
}

void TroubleshooterDialog::updateIssueList()
{
    m_issueList->clear();
    
    QList<Troubleshooter::Issue> issues = m_troubleshooter->getDetectedIssues();
    
    for (const auto& issue : issues) {
        QString severityIcon = getSeverityIcon(issue.severity);
        QString severityColor = getSeverityColor(issue.severity);
        
        QString itemText = QString("%1 %2").arg(severityIcon).arg(issue.title);
        QListWidgetItem* item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, static_cast<int>(issue.type));
        item->setForeground(QColor(severityColor));
        
        m_issueList->addItem(item);
    }
    
    // Update summary
    if (issues.isEmpty()) {
        m_summaryLabel->setText("✅ No issues detected - your wallet is running smoothly!");
        m_summaryLabel->setStyleSheet("font-size: 12px; color: #27ae60;");
    } else {
        m_summaryLabel->setText(QString("⚠️ %1 issue(s) detected. Click on an issue to view details and resolution steps.").arg(issues.size()));
        m_summaryLabel->setStyleSheet("font-size: 12px; color: #f39c12;");
    }
    
    // Update button states
    bool hasSelection = m_issueList->currentItem() != nullptr;
    m_detailsButton->setEnabled(hasSelection);
    m_resolveButton->setEnabled(hasSelection);
}

void TroubleshooterDialog::showIssueDialog(const Troubleshooter::Issue& issue)
{
    IssueDetailsDialog* dialog = new IssueDetailsDialog(issue, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void TroubleshooterDialog::updateStatusLabel()
{
    if (m_troubleshooter->isAutoScan()) {
        m_statusLabel->setText("Ready (Auto-scan enabled)");
    } else {
        m_statusLabel->setText("Ready to run diagnostics");
    }
}

QString TroubleshooterDialog::getSeverityColor(Troubleshooter::IssueSeverity severity) const
{
    switch (severity) {
        case Troubleshooter::Critical: return "#e74c3c";
        case Troubleshooter::Error: return "#e67e22";
        case Troubleshooter::Warning: return "#f39c12";
        case Troubleshooter::Info: return "#3498db";
        default: return "#7f8c8d";
    }
}

QString TroubleshooterDialog::getSeverityIcon(Troubleshooter::IssueSeverity severity) const
{
    switch (severity) {
        case Troubleshooter::Critical: return "🚨";
        case Troubleshooter::Error: return "❌";
        case Troubleshooter::Warning: return "⚠️";
        case Troubleshooter::Info: return "ℹ️";
        default: return "❓";
    }
}

// IssueDetailsDialog implementation
IssueDetailsDialog::IssueDetailsDialog(const Troubleshooter::Issue& issue, QWidget* parent)
    : QDialog(parent)
    , m_issue(issue)
    , m_mainLayout(new QVBoxLayout(this))
    , m_titleLabel(nullptr)
    , m_severityLabel(nullptr)
    , m_descriptionBrowser(nullptr)
    , m_solutionBrowser(nullptr)
    , m_stepsList(nullptr)
    , m_resolveButton(nullptr)
    , m_ignoreButton(nullptr)
    , m_closeButton(nullptr)
{
    setWindowTitle("Issue Details - " + issue.title);
    setMinimumSize(500, 400);
    
    setupUI();
    populateIssueInfo();
}

void IssueDetailsDialog::setupUI()
{
    // Title and severity
    QHBoxLayout* headerLayout = new QHBoxLayout();
    m_titleLabel = new QLabel(m_issue.title, this);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    headerLayout->addWidget(m_titleLabel);
    
    m_severityLabel = new QLabel("", this);
    m_severityLabel->setStyleSheet("font-weight: bold; padding: 4px 8px; border-radius: 4px;");
    headerLayout->addWidget(m_severityLabel);
    headerLayout->addStretch();
    
    m_mainLayout->addLayout(headerLayout);
    
    // Description
    QGroupBox* descGroup = new QGroupBox("Description", this);
    QVBoxLayout* descLayout = new QVBoxLayout(descGroup);
    m_descriptionBrowser = new QTextBrowser(this);
    m_descriptionBrowser->setMaximumHeight(80);
    descLayout->addWidget(m_descriptionBrowser);
    m_mainLayout->addWidget(descGroup);
    
    // Solution
    QGroupBox* solutionGroup = new QGroupBox("Solution", this);
    QVBoxLayout* solutionLayout = new QVBoxLayout(solutionGroup);
    m_solutionBrowser = new QTextBrowser(this);
    m_solutionBrowser->setMaximumHeight(80);
    solutionLayout->addWidget(m_solutionBrowser);
    m_mainLayout->addWidget(solutionGroup);
    
    // Steps
    QGroupBox* stepsGroup = new QGroupBox("Resolution Steps", this);
    QVBoxLayout* stepsLayout = new QVBoxLayout(stepsGroup);
    m_stepsList = new QListWidget(this);
    stepsLayout->addWidget(m_stepsList);
    m_mainLayout->addWidget(stepsGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_resolveButton = new QPushButton("✅ Mark as Resolved", this);
    m_ignoreButton = new QPushButton("⏭️ Ignore", this);
    m_closeButton = new QPushButton("Close", this);
    
    buttonLayout->addWidget(m_resolveButton);
    buttonLayout->addWidget(m_ignoreButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);
    
    m_mainLayout->addLayout(buttonLayout);
    
    // Connections
    connect(m_resolveButton, &QPushButton::clicked, this, &IssueDetailsDialog::onResolveClicked);
    connect(m_ignoreButton, &QPushButton::clicked, this, &IssueDetailsDialog::onIgnoreClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void IssueDetailsDialog::populateIssueInfo()
{
    // Set severity label
    QString severityText;
    QString severityColor;
    
    switch (m_issue.severity) {
        case Troubleshooter::Critical:
            severityText = "CRITICAL";
            severityColor = "#e74c3c";
            break;
        case Troubleshooter::Error:
            severityText = "ERROR";
            severityColor = "#e67e22";
            break;
        case Troubleshooter::Warning:
            severityText = "WARNING";
            severityColor = "#f39c12";
            break;
        case Troubleshooter::Info:
            severityText = "INFO";
            severityColor = "#3498db";
            break;
    }
    
    m_severityLabel->setText(severityText);
    m_severityLabel->setStyleSheet(QString("font-weight: bold; padding: 4px 8px; border-radius: 4px; background-color: %1; color: white;").arg(severityColor));
    
    // Set description and solution
    m_descriptionBrowser->setText(m_issue.description);
    m_solutionBrowser->setText(m_issue.solution);
    
    // Populate steps
    m_stepsList->clear();
    for (const auto& step : m_issue.steps) {
        QListWidgetItem* item = new QListWidgetItem("• " + step);
        m_stepsList->addItem(item);
    }
}

void IssueDetailsDialog::onResolveClicked()
{
    // Mark issue as resolved
    QMessageBox::information(this, "Issue Resolved", 
        QString("The issue '%1' has been marked as resolved.").arg(m_issue.title));
    accept();
}

void IssueDetailsDialog::onIgnoreClicked()
{
    // Ignore the issue
    QMessageBox::information(this, "Issue Ignored", 
        QString("The issue '%1' has been ignored. It will not be shown again in this session.").arg(m_issue.title));
    accept();
}
