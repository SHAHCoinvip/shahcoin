// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#include "securitytab.h"
#include <QDesktopServices>
#include <QUrl>

#if __has_include(<QtWebEngineWidgets/QWebEngineView>)
#include <QtWebEngineWidgets/QWebEngineView>
#define HAS_WEBENGINE 1
#endif

using namespace SHAHSecurity;

SecurityTab::SecurityTab(QWidget *parent)
    : QWidget(parent)
    , m_securityManager(new SHAHSecurityManager(this))
    , m_autoRefreshEnabled(true)
    , m_refreshInterval(REFRESH_INTERVAL_DEFAULT)
    , m_riskThreshold(RiskMedium)
{
    m_mainLayout = new QVBoxLayout(this);
    m_tabWidget = new QTabWidget(this);
    m_mainLayout->addWidget(m_tabWidget);

    setupDashboardTab();
    setupAssetsTab();
    setupReportsTab();
    setupSettingsTab();

#ifdef HAS_WEBENGINE
    // Explorer tab embedding public SHAHSecurity Explorer
    {
        QWidget* webTab = new QWidget(this);
        QVBoxLayout* webLayout = new QVBoxLayout(webTab);
        QWebEngineView* view = new QWebEngineView(webTab);
        view->setUrl(QUrl("https://shahexplorer.shah.vip"));
        webLayout->addWidget(view);
        m_tabWidget->addTab(webTab, tr("Explorer"));
    }
#endif

    setupConnections();

    // Initial load
    refreshSecurityData();
}

SecurityTab::~SecurityTab() {}

void SecurityTab::setupDashboardTab()
{
    m_dashboardTab = new QWidget(this);
    m_dashboardLayout = new QVBoxLayout(m_dashboardTab);

    m_summaryGroup = new QGroupBox(tr("Risk Summary"), m_dashboardTab);
    m_dashboardLayout->addWidget(m_summaryGroup);

    m_alertsGroup = new QGroupBox(tr("Active Alerts"), m_dashboardTab);
    m_dashboardLayout->addWidget(m_alertsGroup);

    m_statsGroup = new QGroupBox(tr("Statistics"), m_dashboardTab);
    m_dashboardLayout->addWidget(m_statsGroup);

    m_tabWidget->addTab(m_dashboardTab, tr("Security"));
}

void SecurityTab::setupAssetsTab()
{
    m_assetsTab = new QWidget(this);
    m_assetsLayout = new QVBoxLayout(m_assetsTab);

    // Controls
    m_controlsLayout = new QHBoxLayout();
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText(tr("Search by name or address…"));
    m_riskFilter = new QComboBox(this);
    m_riskFilter->addItems({tr("All"), tr("Verified/Low"), tr("Medium"), tr("High")});
    m_typeFilter = new QComboBox(this);
    m_typeFilter->addItems({tr("All Types"), tr("Token"), tr("NFT"), tr("Contract"), tr("Collection")});
    m_trustedOnlyCheck = new QCheckBox(tr("Trusted only"), this);
    m_refreshButton = new QPushButton(tr("Refresh"), this);
    m_scanWalletButton = new QPushButton(tr("Scan Wallet Again"), this);

    m_controlsLayout->addWidget(m_searchBox);
    m_controlsLayout->addWidget(m_riskFilter);
    m_controlsLayout->addWidget(m_typeFilter);
    m_controlsLayout->addWidget(m_trustedOnlyCheck);
    m_controlsLayout->addStretch();
    m_controlsLayout->addWidget(m_scanWalletButton);
    m_controlsLayout->addWidget(m_refreshButton);

    m_assetsLayout->addLayout(m_controlsLayout);

    // Table
    m_assetTable = new QTableWidget(this);
    m_assetTable->setColumnCount(8);
    m_assetTable->setHorizontalHeaderLabels({tr("Name"), tr("Type"), tr("Risk"), tr("Score"), tr("Badge"), tr("Last Verified"), tr("Creator Status"), tr("Actions")});
    m_assetTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_assetTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_assetTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_assetTable->setContextMenuPolicy(Qt::CustomContextMenu);

    m_assetsLayout->addWidget(m_assetTable);

    m_tabWidget->addTab(m_assetsTab, tr("Assets"));
}

void SecurityTab::setupReportsTab()
{
    m_reportsTab = new QWidget(this);
    m_reportsLayout = new QVBoxLayout(m_reportsTab);

    m_reportsTable = new QTableWidget(this);
    m_reportsTable->setColumnCount(7);
    m_reportsTable->setHorizontalHeaderLabels({tr("Report ID"), tr("Asset"), tr("Type"), tr("Description"), tr("Time"), tr("Status"), tr("Actions")});
    m_reportsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_newReportButton = new QPushButton(tr("Report Suspicious Asset"), this);
    m_viewReportsButton = new QPushButton(tr("View Asset in Registry"), this);

    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->addWidget(m_newReportButton);
    buttonRow->addWidget(m_viewReportsButton);
    buttonRow->addStretch();

    m_reportsLayout->addWidget(m_reportsTable);
    m_reportsLayout->addLayout(buttonRow);

    m_tabWidget->addTab(m_reportsTab, tr("Reports"));
}

void SecurityTab::setupSettingsTab()
{
    m_settingsTab = new QWidget(this);
    m_settingsLayout = new QVBoxLayout(m_settingsTab);

    m_generalSettings = new QGroupBox(tr("General"), m_settingsTab);
    QVBoxLayout *g = new QVBoxLayout(m_generalSettings);
    QCheckBox *autoBlock = new QCheckBox(tr("Auto-hide high-risk assets (recommended)"), m_generalSettings);
    autoBlock->setChecked(true);
    g->addWidget(autoBlock);

    m_apiSettings = new QGroupBox(tr("API"), m_settingsTab);
    QVBoxLayout *a = new QVBoxLayout(m_apiSettings);
    QLabel *apiLabel = new QLabel(tr("SHAHSecurity API Endpoint:"), m_apiSettings);
    QLineEdit *apiEdit = new QLineEdit("https://api.shah.vip/security/v1", m_apiSettings);
    a->addWidget(apiLabel);
    a->addWidget(apiEdit);

    m_settingsLayout->addWidget(m_generalSettings);
    m_settingsLayout->addWidget(m_apiSettings);
    m_settingsLayout->addStretch();

    m_tabWidget->addTab(m_settingsTab, tr("Settings"));
}

void SecurityTab::setupConnections()
{
    connect(m_refreshButton, &QPushButton::clicked, this, &SecurityTab::onRefreshButtonClicked);
    connect(m_scanWalletButton, &QPushButton::clicked, this, &SecurityTab::onScanWalletButtonClicked);
    connect(m_riskFilter, qOverload<int>(&QComboBox::currentIndexChanged), this, &SecurityTab::onFilterChanged);
    connect(m_typeFilter, qOverload<int>(&QComboBox::currentIndexChanged), this, &SecurityTab::onFilterChanged);
    connect(m_trustedOnlyCheck, &QCheckBox::toggled, this, &SecurityTab::onFilterChanged);
    connect(m_searchBox, &QLineEdit::textChanged, this, &SecurityTab::onSearchTextChanged);
    connect(m_assetTable, &QTableWidget::itemClicked, this, &SecurityTab::onAssetTableItemClicked);
    connect(m_assetTable, &QTableWidget::customContextMenuRequested, this, &SecurityTab::onAssetTableContextMenu);
}

void SecurityTab::refreshSecurityData()
{
    // In MVP, just refresh table from current cache
    updateAssetTable();
}

void SecurityTab::updateAssetSecurity(const QString &address)
{
    auto info = m_securityManager->getAssetSecurityInfo(address);
    Q_UNUSED(info);
    updateAssetTable();
}

void SecurityTab::onRefreshButtonClicked()
{
    if (m_securityManager) m_securityManager->refreshSecurityData();
    updateAssetTable();
    animateRefreshComplete();
}

void SecurityTab::onScanWalletButtonClicked()
{
    // TODO: Iterate wallet assets and call updateAssetSecurity
    onRefreshButtonClicked();
}

void SecurityTab::onFilterChanged()
{
    updateAssetTable();
}

void SecurityTab::onSearchTextChanged()
{
    updateAssetTable();
}

void SecurityTab::onAssetTableItemClicked(QTableWidgetItem *item)
{
    if (!item) return;
    int row = item->row();
    QString address = m_assetTable->item(row, COL_NAME)->data(Qt::UserRole).toString();
    Q_UNUSED(address);
}

void SecurityTab::onAssetTableContextMenu(const QPoint &pos)
{
    QTableWidgetItem *item = m_assetTable->itemAt(pos);
    if (!item) return;
    int row = item->row();
    QString address = m_assetTable->item(row, COL_NAME)->data(Qt::UserRole).toString();

    QMenu menu(this);
    QAction *view = menu.addAction(tr("View Details"));
    QAction *report = menu.addAction(tr("Report Asset"));
    QAction *copy = menu.addAction(tr("Copy Address"));
    QAction *act = menu.exec(m_assetTable->viewport()->mapToGlobal(pos));

    if (act == view) showRiskDetails(address);
    else if (act == report) showSecurityReport(address);
    else if (act == copy) copyToClipboard(address);
}

void SecurityTab::updateAssetTable()
{
    // For MVP, show cache snapshot
    m_assetTable->setRowCount(0);

    // Fake empty list in MVP (real integration wires in wallet assets)
    // Append a demo row for UI validation
    int row = m_assetTable->rowCount();
    m_assetTable->insertRow(row);

    QTableWidgetItem *name = new QTableWidgetItem("SHAHCOIN");
    name->setData(Qt::UserRole, "0x0000000000000000000000000000000000000001");
    QTableWidgetItem *type = new QTableWidgetItem("Token");
    // Risk columns
    int score = m_securityManager->computeSecurityScore(m_securityManager->getAssetSecurityInfo("0x0000000000000000000000000000000000000001"));
    QString riskStr = score <= 25 ? tr("Safe") : score <= 60 ? tr("Caution") : tr("Risky");
    QTableWidgetItem *risk = new QTableWidgetItem(riskStr);
    QTableWidgetItem *scoreItem = new QTableWidgetItem(QString::number(score));
    QTableWidgetItem *badge = new QTableWidgetItem(score <= 25 ? "🟢" : score <= 60 ? "🟡" : "🔴");
    QTableWidgetItem *last = new QTableWidgetItem(QDateTime::currentDateTime().toString(Qt::ISODate));
    QTableWidgetItem *creator = new QTableWidgetItem("Trusted");
    QTableWidgetItem *actions = new QTableWidgetItem("⋯");

    m_assetTable->setItem(row, COL_NAME, name);
    m_assetTable->setItem(row, COL_TYPE, type);
    // Tooltips using manager
    auto info = m_securityManager->getAssetSecurityInfo("0x0000000000000000000000000000000000000001");
    QString tip = m_securityManager->tooltipFor(info, score);
    risk->setToolTip(tip);
    scoreItem->setToolTip(tip);
    badge->setToolTip(tip);

    m_assetTable->setItem(row, COL_RISK_SCORE, risk);
    m_assetTable->setItem(row, COL_BADGE, badge);
    m_assetTable->setItem(row, COL_RISK_SCORE + 1, scoreItem);
    m_assetTable->setItem(row, COL_LAST_VERIFIED, last);
    m_assetTable->setItem(row, COL_CREATOR_STATUS, creator);
    m_assetTable->setItem(row, COL_ACTIONS, actions);
}

// Stubs (kept minimal for MVP)
void SecurityTab::showSecurityReport(const QString &address)
{
    Q_UNUSED(address);
    QMessageBox::information(this, tr("Report"), tr("Report dialog will be implemented."));
}

void SecurityTab::showRiskDetails(const QString &address)
{
    auto info = m_securityManager->getAssetSecurityInfo(address);
    QString text = tr("Address: %1\nRisk: %2\nVerified: %3\nDetails: %4")
        .arg(info.address)
        .arg(getRiskLevelString(info.riskLevel))
        .arg(info.isVerified ? tr("Yes") : tr("No"))
        .arg(info.detailsUrl);
    QMessageBox::information(this, tr("Risk Details"), text);
}

void SecurityTab::filterByRiskLevel(SHAHSecurity::RiskLevel level) { Q_UNUSED(level); }
void SecurityTab::filterByAssetType(SHAHSecurity::AssetType type) { Q_UNUSED(type); }
void SecurityTab::searchAssets(const QString &searchTerm) { Q_UNUSED(searchTerm); }
void SecurityTab::showOnlyTrustedAssets() {}
void SecurityTab::setRiskThreshold(SHAHSecurity::RiskLevel threshold) { m_riskThreshold = threshold; }
void SecurityTab::setRefreshInterval(int minutes) { m_refreshInterval = minutes; }
void SecurityTab::setAutoRefresh(bool enabled) { m_autoRefreshEnabled = enabled; }

QString SecurityTab::getRiskLevelString(SHAHSecurity::RiskLevel level) const
{
    switch (level) {
        case RiskVerified: return tr("Verified");
        case RiskLow: return tr("Low");
        case RiskMedium: return tr("Medium");
        case RiskHigh: return tr("High");
        default: return tr("Unknown");
    }
}

QString SecurityTab::getAssetTypeString(SHAHSecurity::AssetType type) const
{
    switch (type) {
        case AssetToken: return tr("Token");
        case AssetNFT: return tr("NFT");
        case AssetContract: return tr("Contract");
        case AssetCollection: return tr("Collection");
        default: return tr("Unknown");
    }
}

QString SecurityTab::getBadgeIcon(SHAHSecurity::BadgeType type) const
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

QString SecurityTab::getBadgeColor(SHAHSecurity::BadgeType type) const
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

QColor SecurityTab::getRiskColor(SHAHSecurity::RiskLevel level) const
{
    switch (level) {
        case RiskVerified: return QColor("#28a745");
        case RiskLow: return QColor("#2ecc71");
        case RiskMedium: return QColor("#f1c40f");
        case RiskHigh: return QColor("#e74c3c");
        default: return QColor("#95a5a6");
    }
}

QString SecurityTab::formatDateTime(const QDateTime &dt) const { return dt.toString(Qt::ISODate); }
QString SecurityTab::formatAddress(const QString &address) const { return address.left(6) + "…" + address.right(4); }

void SecurityTab::animateRiskChange(const QString &, SHAHSecurity::RiskLevel, SHAHSecurity::RiskLevel) {}
void SecurityTab::animateNewAlert(const QString &) {}
void SecurityTab::animateRefreshComplete() {}

void SecurityTab::copyToClipboard(const QString &text) { QApplication::clipboard()->setText(text); }
void SecurityTab::openUrl(const QString &url) { QDesktopServices::openUrl(QUrl(url)); }
void SecurityTab::showHelp() { QMessageBox::information(this, tr("Help"), tr("Security help coming soon.")); }
void SecurityTab::showAbout() { QMessageBox::information(this, tr("About"), tr("SHAHSecurity — Shahcoin security layer.")); }
void SecurityTab::checkForUpdates() {}
void SecurityTab::backupData() {}
void SecurityTab::restoreData() {}

// Unused interface stubs
void SecurityTab::loadAssetData() {}
void SecurityTab::loadReportData() {}
void SecurityTab::updateSummaryWidgets() {}
void SecurityTab::updateAlertWidgets() {}
void SecurityTab::updateStatsWidgets() {}
void SecurityTab::refreshAssetSecurity(const QString &) {}

// Boilerplate stubs
void SecurityTab::resizeEvent(QResizeEvent *e) { QWidget::resizeEvent(e); }
void SecurityTab::showEvent(QShowEvent *e) { QWidget::showEvent(e); }
void SecurityTab::hideEvent(QHideEvent *e) { QWidget::hideEvent(e); }
void SecurityTab::closeEvent(QCloseEvent *e) { QWidget::closeEvent(e); }

void SecurityTab::saveSettings() {}
void SecurityTab::loadSettings() {}
void SecurityTab::clearCache() {}
void SecurityTab::checkApiConnection() {}
void SecurityTab::handleApiError(const QString &) {}
void SecurityTab::retryFailedRequests() {}
void SecurityTab::validateSecurityData() {}
void SecurityTab::checkForSuspiciousActivity() {}
void SecurityTab::monitorAssetChanges() {}
void SecurityTab::generateSecurityReport() {}

void SecurityTab::createTableItem(const QString &, const QColor &) {}
void SecurityTab::createBadgeItem(const QString &, const QString &, const QColor &) {}
void SecurityTab::createRiskProgressBar(double) {}
void SecurityTab::createActionButton(const QString &, const QString &, const QString &) {}
void SecurityTab::setupContextMenu(QTableWidget *) {}
void SecurityTab::setupTooltips() {}
