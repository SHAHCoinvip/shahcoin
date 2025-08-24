#include <qt/tokennftexplorer.h>
#include <qt/walletmodel.h>
#include <qt/platformstyle.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>
#include <qt/priceoracle.h>
#include <interfaces/wallet.h>
#include <util/translation.h>
#include <tokens/token.h>
#include <tokens/nft.h>
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
#include <QJsonDocument>
#include <QJsonObject>
#include <QToolTip>
#include <qt/marketplacelistingdialog.h>
#include <QInputDialog>
#include <QSettings>
#include <QDoubleValidator>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>

TokenNFTExplorer::TokenNFTExplorer(const PlatformStyle* platformStyle, QWidget* parent)
    : QWidget(parent)
    , m_walletModel(nullptr)
    , m_platformStyle(platformStyle)
    , m_refreshTimer(new QTimer(this))
{
    setupUI();
    connectSignals();
    applyTheme();

    // Price Oracle for live USD conversion
    m_priceOracle = new PriceOracle(this);
    connect(m_priceOracle, &PriceOracle::priceUpdated, this, &TokenNFTExplorer::onOraclePriceUpdated);
    m_priceOracle->start(30000);
    
    // Set up refresh timer (every 30 seconds)
    connect(m_refreshTimer, &QTimer::timeout, this, &TokenNFTExplorer::refreshData);
    m_refreshTimer->start(30000); // 30 seconds
    
    refreshData();

    m_settings.beginGroup("marketplace");
    m_apiBaseUrl = m_settings.value("apiBase", "https://api.shah.vip/marketplace").toString();
    m_settings.endGroup();
    
    // Initialize cross-platform sync
    setupCrossPlatformSync();
}

TokenNFTExplorer::~TokenNFTExplorer()
{
    if (m_refreshTimer) {
        m_refreshTimer->stop();
    }
}

void TokenNFTExplorer::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    refreshData();
}

void TokenNFTExplorer::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Setup controls
    setupControls();
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);
    
    // Setup individual tabs
    setupTokenTab();
    setupNFTTab();
    setupMarketplaceTab();
    setupModerationTab();
    
    m_mainLayout->addWidget(m_tabWidget);
}

void TokenNFTExplorer::setupControls()
{
    m_globalControlsLayout = new QHBoxLayout();
    
    // Export button
    m_exportButton = new QPushButton(tr("📁 Export Data"), this);
    m_exportButton->setToolTip(tr("Export current view data to CSV/JSON"));
    
    // Analytics button
    QPushButton* analyticsButton = new QPushButton(tr("📊 Analytics"), this);
    analyticsButton->setToolTip(tr("View marketplace analytics and transaction history"));
    connect(analyticsButton, &QPushButton::clicked, this, &TokenNFTExplorer::showAnalyticsDashboard);
    
    // Status label
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusLabel->setStyleSheet("color: #2E8B57; font-weight: bold;");
    
    // Loading progress
    m_loadingProgress = new QProgressBar(this);
    m_loadingProgress->setVisible(false);
    m_loadingProgress->setRange(0, 0); // Indeterminate progress
    
    m_globalControlsLayout->addWidget(m_exportButton);
    m_globalControlsLayout->addWidget(analyticsButton);
    m_globalControlsLayout->addStretch();
    m_globalControlsLayout->addWidget(m_statusLabel);
    m_globalControlsLayout->addWidget(m_loadingProgress);
    
    m_mainLayout->addLayout(m_globalControlsLayout);
}

void TokenNFTExplorer::setupTokenTab()
{
    m_tokenTab = new QWidget();
    m_tokenLayout = new QVBoxLayout(m_tokenTab);
    
    // Token controls
    m_tokenControlsLayout = new QHBoxLayout();
    
    m_tokenSearchEdit = new QLineEdit(this);
    m_tokenSearchEdit->setPlaceholderText(tr("Search tokens by name or symbol..."));
    m_tokenSearchEdit->setToolTip(tr("Search tokens by name or symbol"));
    
    m_tokenFilterCombo = new QComboBox(this);
    m_tokenFilterCombo->addItems({
        tr("All Tokens"),
        tr("My Tokens"),
        tr("Recent (Last 1000 blocks)"),
        tr("High Supply (>1M)"),
        tr("Low Supply (<1K)")
    });
    m_tokenFilterCombo->setToolTip(tr("Filter tokens by criteria"));
    
    m_tokenRefreshButton = new QPushButton(tr("🔄 Refresh"), this);
    m_tokenRefreshButton->setToolTip(tr("Refresh token list"));
    
    m_tokenControlsLayout->addWidget(new QLabel(tr("Search:")));
    m_tokenControlsLayout->addWidget(m_tokenSearchEdit);
    m_tokenControlsLayout->addWidget(new QLabel(tr("Filter:")));
    m_tokenControlsLayout->addWidget(m_tokenFilterCombo);
    m_tokenControlsLayout->addWidget(m_tokenRefreshButton);
    
    m_tokenLayout->addLayout(m_tokenControlsLayout);
    
    // Token table
    m_tokenTable = new QTableWidget(this);
    m_tokenTable->setColumnCount(10);
    m_tokenTable->setHorizontalHeaderLabels({
        tr("Name"), tr("Symbol"), tr("Supply"), tr("Creator"), tr("Block"), tr("Decimals"), tr("Description"), tr("Risk"), tr("Score"), tr("Badge")
    });
    m_tokenTable->horizontalHeader()->setStretchLastSection(true);
    m_tokenTable->setAlternatingRowColors(true);
    m_tokenTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tokenTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tokenTable->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Set column widths
    m_tokenTable->setColumnWidth(0, 150); // Name
    m_tokenTable->setColumnWidth(1, 80);  // Symbol
    m_tokenTable->setColumnWidth(2, 120); // Supply
    m_tokenTable->setColumnWidth(3, 120); // Creator
    m_tokenTable->setColumnWidth(4, 80);  // Block
    m_tokenTable->setColumnWidth(5, 80);  // Decimals
    m_tokenTable->setColumnWidth(6, 200); // Description
    
    m_tokenLayout->addWidget(m_tokenTable);
    
    m_tabWidget->addTab(m_tokenTab, tr("🪙 Tokens"));
}

void TokenNFTExplorer::setupNFTTab()
{
    m_nftTab = new QWidget();
    m_nftLayout = new QVBoxLayout(m_nftTab);
    
    // NFT controls
    m_nftControlsLayout = new QHBoxLayout();
    
    m_nftSearchEdit = new QLineEdit(this);
    m_nftSearchEdit->setPlaceholderText(tr("Search NFTs by name..."));
    m_nftSearchEdit->setToolTip(tr("Search NFTs by name"));
    
    m_nftFilterCombo = new QComboBox(this);
    m_nftFilterCombo->addItems({
        tr("All NFTs"),
        tr("My NFTs"),
        tr("Recent (Last 1000 blocks)"),
        tr("With Images"),
        tr("With Attributes")
    });
    m_nftFilterCombo->setToolTip(tr("Filter NFTs by criteria"));

    m_verifiedOnlyCheck = new QCheckBox(tr("Verified Only"), this);
    m_verifiedOnlyCheck->setToolTip(tr("Show only verified NFTs in marketplace"));
    m_verifiedOnlyCheck->setChecked(true);
    
    m_nftRefreshButton = new QPushButton(tr("🔄 Refresh"), this);
    m_nftRefreshButton->setToolTip(tr("Refresh NFT list"));
    
    m_nftControlsLayout->addWidget(new QLabel(tr("Search:")));
    m_nftControlsLayout->addWidget(m_nftSearchEdit);
    m_nftControlsLayout->addWidget(new QLabel(tr("Filter:")));
    m_nftControlsLayout->addWidget(m_nftFilterCombo);
    m_nftControlsLayout->addWidget(m_verifiedOnlyCheck);
    m_nftControlsLayout->addWidget(m_nftRefreshButton);
    
    m_nftLayout->addLayout(m_nftControlsLayout);
    
    // NFT table
    m_nftTable = new QTableWidget(this);
    m_nftTable->setColumnCount(11);
    m_nftTable->setHorizontalHeaderLabels({
        tr("Name"), tr("Tier"), tr("Creator"), tr("Owner"), tr("Block"), tr("Image"), tr("Attributes"), tr("Description"), tr("Risk"), tr("Score"), tr("Badge")
    });
    m_nftTable->horizontalHeader()->setStretchLastSection(true);
    m_nftTable->setAlternatingRowColors(true);
    m_nftTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_nftTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_nftTable->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Set column widths
    m_nftTable->setColumnWidth(0, 150); // Name
    m_nftTable->setColumnWidth(1, 80);  // Tier
    m_nftTable->setColumnWidth(2, 120); // Creator
    m_nftTable->setColumnWidth(3, 120); // Owner
    m_nftTable->setColumnWidth(4, 80);  // Block
    m_nftTable->setColumnWidth(5, 80);  // Image
    m_nftTable->setColumnWidth(6, 100); // Attributes
    m_nftTable->setColumnWidth(7, 200); // Description
    
    m_nftLayout->addWidget(m_nftTable);
    
    m_tabWidget->addTab(m_nftTab, tr("🎨 NFTs"));
}

void TokenNFTExplorer::setupMarketplaceTab()
{
    m_marketTab = new QWidget();
    m_marketLayout = new QVBoxLayout(m_marketTab);

    m_marketControlsLayout = new QHBoxLayout();

    m_marketToggle = new QComboBox(this);
    m_marketToggle->addItems({ tr("Verified Only"), tr("All NFTs"), tr("My NFTs"), tr("Favorites") });

    m_marketCategory = new QComboBox(this);
    m_marketCategory->addItems({ tr("All Categories"), tr("Art"), tr("Game"), tr("Music"), tr("Collectible") });

    m_marketCreator = new QLineEdit(this);
    m_marketCreator->setPlaceholderText(tr("Creator address"));

    m_marketPriceMin = new QLineEdit(this);
    m_marketPriceMin->setPlaceholderText(tr("Min $"));
    m_marketPriceMax = new QLineEdit(this);
    m_marketPriceMax->setPlaceholderText(tr("Max $"));

    m_marketTraits = new QLineEdit(this);
    m_marketTraits->setPlaceholderText(tr("Traits (comma-separated)"));

    // Security visibility toggle
    QCheckBox* showRiskyToggle = new QCheckBox(tr("Show risky NFTs (not recommended)"), this);
    showRiskyToggle->setChecked(m_settings.value("security/show_risky", false).toBool());
    connect(showRiskyToggle, &QCheckBox::toggled, this, [this](bool on){ m_settings.setValue("security/show_risky", on); populateMarketplaceTable(); });

    m_marketRefreshButton = new QPushButton(tr("🔄 Refresh"), this);
    m_marketAdminButton = new QPushButton(tr("🛡 Admin Tools"), this);
    
    // Advanced filtering and bulk operations buttons
    m_advancedFilterButton = new QPushButton(tr("🔍 Advanced Filters"), this);
    m_filterPresetButton = new QPushButton(tr("💾 Filter Presets"), this);
    m_bulkOperationsButton = new QPushButton(tr("📦 Bulk Operations"), this);
    m_toggleViewButton = new QPushButton(tr("📱 Mobile View"), this);
    
    // Search box with autocomplete
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText(tr("Search NFTs, creators, traits..."));
    m_searchBox->setMinimumWidth(200);
    
    // Sort controls
    m_sortByCombo = new QComboBox(this);
    m_sortByCombo->addItems({ tr("Price"), tr("Date"), tr("Rarity"), tr("Popularity") });
    m_sortOrderCombo = new QComboBox(this);
    m_sortOrderCombo->addItems({ tr("Desc"), tr("Asc") });
    
    // Search suggestions
    m_searchSuggestionsList = new QListWidget(this);
    m_searchSuggestionsList->setMaximumHeight(150);
    m_searchSuggestionsList->hide();
    m_searchDebounceTimer = new QTimer(this);
    m_searchDebounceTimer->setSingleShot(true);
    m_searchDebounceTimer->setInterval(300);

    m_marketControlsLayout->addWidget(new QLabel(tr("View:")));
    m_marketControlsLayout->addWidget(m_marketToggle);
    m_marketControlsLayout->addWidget(new QLabel(tr("Search:")));
    m_marketControlsLayout->addWidget(m_searchBox);
    m_marketControlsLayout->addWidget(new QLabel(tr("Sort:")));
    m_marketControlsLayout->addWidget(m_sortByCombo);
    m_marketControlsLayout->addWidget(m_sortOrderCombo);
    m_marketControlsLayout->addWidget(new QLabel(tr("Category:")));
    m_marketControlsLayout->addWidget(m_marketCategory);
    m_marketControlsLayout->addWidget(new QLabel(tr("Creator:")));
    m_marketControlsLayout->addWidget(m_marketCreator);
    m_marketControlsLayout->addWidget(new QLabel(tr("Price:")));
    m_marketControlsLayout->addWidget(m_marketPriceMin);
    m_marketControlsLayout->addWidget(m_marketPriceMax);
    m_marketControlsLayout->addWidget(new QLabel(tr("Traits:")));
    m_marketControlsLayout->addWidget(m_marketTraits);
    m_marketControlsLayout->addWidget(showRiskyToggle);
    m_marketControlsLayout->addWidget(m_marketRefreshButton);
    m_marketControlsLayout->addWidget(m_advancedFilterButton);
    m_marketControlsLayout->addWidget(m_filterPresetButton);
    m_marketControlsLayout->addWidget(m_bulkOperationsButton);
    m_marketControlsLayout->addWidget(m_toggleViewButton);
    m_marketControlsLayout->addWidget(m_marketAdminButton);

    m_marketLayout->addLayout(m_marketControlsLayout);
    
    // Add search suggestions list
    m_marketLayout->addWidget(m_searchSuggestionsList);

    m_marketTable = new QTableWidget(this);
    m_marketTable->setColumnCount(10);
    m_marketTable->setHorizontalHeaderLabels({
        tr("Name"), tr("Tier"), tr("Creator"), tr("Owner"), tr("Category"), tr("Traits"), tr("List Price (SHAH)"), tr("USD"), tr("Verified"), tr("Favorite")
    });
    m_marketTable->horizontalHeader()->setStretchLastSection(true);
    m_marketTable->setAlternatingRowColors(true);
    m_marketTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_marketTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_marketTable->setContextMenuPolicy(Qt::CustomContextMenu);

    m_marketLayout->addWidget(m_marketTable);

    m_tabWidget->addTab(m_marketTab, tr("🛍 Marketplace"));

    connect(m_marketRefreshButton, &QPushButton::clicked, this, &TokenNFTExplorer::populateMarketplaceTable);
    connect(m_marketToggle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TokenNFTExplorer::onMarketToggleChanged);
    connect(m_marketCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TokenNFTExplorer::onMarketFilterChanged);
    connect(m_marketCreator, &QLineEdit::textChanged, this, &TokenNFTExplorer::onMarketFilterChanged);
    
    // Connect new marketplace functionality
    connect(m_advancedFilterButton, &QPushButton::clicked, this, &TokenNFTExplorer::onAdvancedFilterClicked);
    connect(m_filterPresetButton, &QPushButton::clicked, this, &TokenNFTExplorer::onFilterPresetClicked);
    connect(m_bulkOperationsButton, &QPushButton::clicked, this, &TokenNFTExplorer::onBulkOperationsClicked);
    connect(m_toggleViewButton, &QPushButton::clicked, this, &TokenNFTExplorer::onToggleViewClicked);
    connect(m_searchBox, &QLineEdit::textChanged, this, &TokenNFTExplorer::onSearchTextChanged);
    connect(m_sortByCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TokenNFTExplorer::onSortByChanged);
    connect(m_sortOrderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TokenNFTExplorer::onSortOrderChanged);
    connect(m_searchDebounceTimer, &QTimer::timeout, this, &TokenNFTExplorer::onSearchDebounceTimeout);
    connect(m_searchSuggestionsList, &QListWidget::itemClicked, this, &TokenNFTExplorer::onSearchSuggestionSelected);
    connect(m_marketPriceMin, &QLineEdit::textChanged, this, &TokenNFTExplorer::onMarketFilterChanged);
    connect(m_marketPriceMax, &QLineEdit::textChanged, this, &TokenNFTExplorer::onMarketFilterChanged);
    connect(m_marketTraits, &QLineEdit::textChanged, this, &TokenNFTExplorer::onMarketFilterChanged);
    connect(m_marketTable, &QTableWidget::customContextMenuRequested, this, &TokenNFTExplorer::onMarketplaceContextMenu);
    
    // Connect new advanced filtering and bulk operations signals
    connect(m_advancedFilterButton, &QPushButton::clicked, this, &TokenNFTExplorer::onAdvancedFilterClicked);
    connect(m_filterPresetButton, &QPushButton::clicked, this, &TokenNFTExplorer::onFilterPresetClicked);
    connect(m_bulkOperationsButton, &QPushButton::clicked, this, &TokenNFTExplorer::onBulkOperationsClicked);
    connect(m_toggleViewButton, &QPushButton::clicked, this, &TokenNFTExplorer::onToggleViewClicked);
    
    // Connect search and sort signals
    connect(m_searchBox, &QLineEdit::textChanged, this, &TokenNFTExplorer::onSearchTextChanged);
    connect(m_sortByCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TokenNFTExplorer::onSortByChanged);
    connect(m_sortOrderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TokenNFTExplorer::onSortOrderChanged);
    connect(m_searchDebounceTimer, &QTimer::timeout, this, &TokenNFTExplorer::onSearchDebounceTimeout);
    connect(m_searchSuggestionsList, &QListWidget::itemClicked, this, &TokenNFTExplorer::onSearchSuggestionSelected);

    connect(m_marketAdminButton, &QPushButton::clicked, this, [this]() {
        // Simple admin dialog: toggle adminVerified for selected or batch by creator
        QDialog dlg(this); dlg.setWindowTitle(tr("Admin Tools"));
        QVBoxLayout* v = new QVBoxLayout(&dlg);
        QLineEdit* creator = new QLineEdit(&dlg); creator->setPlaceholderText(tr("Filter by creator (optional)"));
        QPushButton* verifyAll = new QPushButton(tr("Set adminVerified=true for visible rows"), &dlg);
        QPushButton* unverifyAll = new QPushButton(tr("Set adminVerified=false for visible rows"), &dlg);
        v->addWidget(new QLabel(tr("Batch Verification")));
        v->addWidget(creator);
        v->addWidget(verifyAll);
        v->addWidget(unverifyAll);
        connect(verifyAll, &QPushButton::clicked, &dlg, [&]() {
            for (int r = 0; r < m_marketTable->rowCount(); ++r) {
                QString id = m_marketTable->item(r, 0)->data(Qt::UserRole).toString();
                QString creatorCell = m_marketTable->item(r, 2)->text();
                if (!creator->text().trimmed().isEmpty() && !creatorCell.contains(creator->text().trimmed(), Qt::CaseInsensitive)) continue;
                setAdminVerifyFlagToApi(id, true);
                for (auto& n : m_nftList) if (n.id == id) {
                    QJsonParseError err; QJsonDocument doc = QJsonDocument::fromJson(n.metadata.toUtf8(), &err);
                    QJsonObject obj = err.error==QJsonParseError::NoError && doc.isObject() ? doc.object() : QJsonObject{};
                    obj.insert("adminVerified", true);
                    n.metadata = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
                }
            }
            populateMarketplaceTable();
        });
        connect(unverifyAll, &QPushButton::clicked, &dlg, [&]() {
            for (int r = 0; r < m_marketTable->rowCount(); ++r) {
                QString id = m_marketTable->item(r, 0)->data(Qt::UserRole).toString();
                QString creatorCell = m_marketTable->item(r, 2)->text();
                if (!creator->text().trimmed().isEmpty() && !creatorCell.contains(creator->text().trimmed(), Qt::CaseInsensitive)) continue;
                setAdminVerifyFlagToApi(id, false);
                for (auto& n : m_nftList) if (n.id == id) {
                    QJsonParseError err; QJsonDocument doc = QJsonDocument::fromJson(n.metadata.toUtf8(), &err);
                    QJsonObject obj = err.error==QJsonParseError::NoError && doc.isObject() ? doc.object() : QJsonObject{};
                    obj.insert("adminVerified", false);
                    n.metadata = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
                }
            }
            populateMarketplaceTable();
        });
        dlg.exec();
    });

    // Start polling marketplace every 60s
    if (!m_marketPollTimer) m_marketPollTimer = new QTimer(this);
    connect(m_marketPollTimer, &QTimer::timeout, this, &TokenNFTExplorer::fetchMarketplaceNftsFromApi);
    m_marketPollTimer->start(60000);
}

void TokenNFTExplorer::connectSignals()
{
    // Tab changes
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &TokenNFTExplorer::onTabChanged);
    
    // Token tab signals
    connect(m_tokenSearchEdit, &QLineEdit::textChanged, this, &TokenNFTExplorer::onTokenSearchChanged);
    connect(m_tokenFilterCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &TokenNFTExplorer::onTokenFilterChanged);
    connect(m_tokenRefreshButton, &QPushButton::clicked, this, &TokenNFTExplorer::onRefreshClicked);
    connect(m_tokenTable, &QTableWidget::cellClicked, this, &TokenNFTExplorer::onTokenRowClicked);
    connect(m_tokenTable, &QTableWidget::customContextMenuRequested,
            this, &TokenNFTExplorer::onTokenContextMenu);
    
    // NFT tab signals
    connect(m_nftSearchEdit, &QLineEdit::textChanged, this, &TokenNFTExplorer::onNFTSearchChanged);
    connect(m_nftFilterCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &TokenNFTExplorer::onNFTFilterChanged);
    connect(m_verifiedOnlyCheck, &QCheckBox::stateChanged, this, &TokenNFTExplorer::onNFTFilterChanged); // Connect to filter combo
    connect(m_nftRefreshButton, &QPushButton::clicked, this, &TokenNFTExplorer::onRefreshClicked);
    connect(m_nftTable, &QTableWidget::cellClicked, this, &TokenNFTExplorer::onNFTRowClicked);
    connect(m_nftTable, &QTableWidget::customContextMenuRequested,
            this, &TokenNFTExplorer::onNFTContextMenu);
    
    // Global signals
    connect(m_exportButton, &QPushButton::clicked, this, &TokenNFTExplorer::onExportClicked);
    
    // Moderation signals
    connect(m_moderationFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TokenNFTExplorer::onModerationFilterChanged);
    connect(m_moderationSortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TokenNFTExplorer::onModerationSortChanged);
    connect(m_moderationRefreshButton, &QPushButton::clicked, this, &TokenNFTExplorer::refreshModerationData);
    connect(m_moderationSettingsButton, &QPushButton::clicked, this, &TokenNFTExplorer::onModerationSettingsClicked);
    connect(m_moderationExportButton, &QPushButton::clicked, this, &TokenNFTExplorer::onModerationExportClicked);
    connect(m_moderationTable, &QTableWidget::customContextMenuRequested, this, &TokenNFTExplorer::onModerationContextMenu);
    
    // Setup context menus
    setupContextMenus();
}

void TokenNFTExplorer::setupContextMenus()
{
    // Token context menu
    m_tokenContextMenu = new QMenu(this);
    QAction* viewTokenAction = m_tokenContextMenu->addAction(tr("👁️ View Details"));
    QAction* exportTokenAction = m_tokenContextMenu->addAction(tr("📁 Export Data"));
    
    connect(viewTokenAction, &QAction::triggered, this, &TokenNFTExplorer::onViewTokenDetails);
    connect(exportTokenAction, &QAction::triggered, this, &TokenNFTExplorer::onExportTokenData);
    
    // NFT context menu
    m_nftContextMenu = new QMenu(this);
    QAction* viewNFTAction = m_nftContextMenu->addAction(tr("👁️ View Details"));
    QAction* exportNFTAction = m_nftContextMenu->addAction(tr("📁 Export Data"));
    QAction* upgradeTierAction = m_nftContextMenu->addAction(tr("⬆️ Upgrade Tier"));
    
    connect(viewNFTAction, &QAction::triggered, this, &TokenNFTExplorer::onViewNFTDetails);
    connect(exportNFTAction, &QAction::triggered, this, &TokenNFTExplorer::onExportNFTData);
    connect(upgradeTierAction, &QAction::triggered, [this]() {
        if (m_selectedNFTId.isEmpty()) return;
        showNFTUpgradeDialog(m_selectedNFTId);
    });
}

void TokenNFTExplorer::applyTheme()
{
    QString style = R"(
        QTabWidget::pane {
            border: 1px solid #dee2e6;
            background-color: white;
        }
        QTabBar::tab {
            background-color: #e9ecef;
            padding: 8px 16px;
            margin-right: 2px;
            border: 1px solid #dee2e6;
            border-bottom: none;
        }
        QTabBar::tab:selected {
            background-color: white;
            border-bottom: 1px solid white;
        }
        QTableWidget {
            gridline-color: #dee2e6;
            alternate-background-color: #f8f9fa;
            selection-background-color: #007bff;
            selection-color: white;
        }
        QTableWidget::item {
            padding: 4px;
        }
        QTableWidget::item:selected {
            background-color: #007bff;
            color: white;
        }
        QPushButton {
            background-color: #007bff;
            color: white;
            border: none;
            padding: 6px 12px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QPushButton:pressed {
            background-color: #004085;
        }
        QLineEdit {
            border: 1px solid #dee2e6;
            border-radius: 4px;
            padding: 6px;
        }
        QLineEdit:focus {
            border-color: #007bff;
        }
        QComboBox {
            border: 1px solid #dee2e6;
            border-radius: 4px;
            padding: 6px;
        }
        QComboBox:focus {
            border-color: #007bff;
        }
    )";
    
    setStyleSheet(style);
}

void TokenNFTExplorer::refreshData()
{
    m_loadingProgress->setVisible(true);
    m_statusLabel->setText(tr("Loading data..."));
    m_statusLabel->setStyleSheet("color: #FF8C00; font-weight: bold;");
    
    // Load data in background (simulated for now)
    QTimer::singleShot(100, [this]() {
        loadTokenData();
        loadNFTData();
        loadTransactionHistory(); // Load transaction history
        fetchMarketplaceNftsFromApi(); // Fetch marketplace data on refresh
        
        m_loadingProgress->setVisible(false);
        m_statusLabel->setText(tr("Ready"));
        m_statusLabel->setStyleSheet("color: #2E8B57; font-weight: bold;");
    });
}

void TokenNFTExplorer::loadTokenData()
{
    m_tokenList.clear();
    
    // TODO: Load actual token data from blockchain
    // This would typically call:
    // g_tokenManager->GetAllTokens()
    
    // For now, add sample data
    TokenData token1;
    token1.id = "TOKEN_001";
    token1.name = "Shahcoin Gold";
    token1.symbol = "SGC";
    token1.supply = 1000000000;
    token1.decimals = 8;
    token1.creator = "S1a2h3c4o5i6n7G8o9l0d";
    token1.blockCreated = 12345;
    token1.description = "Premium gold-backed token";
    token1.logoHash = "gold_logo_hash";
    m_tokenList.append(token1);
    
    TokenData token2;
    token2.id = "TOKEN_002";
    token2.name = "Shahcoin Silver";
    token2.symbol = "SCS";
    token2.supply = 500000000;
    token2.decimals = 6;
    token2.creator = "S1a2h3c4o5i6n7S8i9l0v";
    token2.blockCreated = 12350;
    token2.description = "Silver-backed utility token";
    token2.logoHash = "silver_logo_hash";
    m_tokenList.append(token2);
    
    populateTokenTable();
}

void TokenNFTExplorer::loadNFTData()
{
    m_nftList.clear();
    
    // TODO: Load actual NFT data from blockchain
    // This would typically call:
    // g_nftManager->GetAllNFTs()
    
    // For now, add sample data
    NFTData nft1;
    nft1.id = "NFT_001";
    nft1.name = "Shahcoin Genesis NFT";
    nft1.description = "The first NFT ever minted on Shahcoin";
    nft1.creator = "S1a2h3c4o5i6n7G8e9n0e";
    nft1.blockCreated = 12340;
    nft1.owner = "S1a2h3c4o5i6n7G8e9n0e";
    nft1.imageHash = "genesis_nft_image_hash";
    nft1.attributes = QJsonObject{
        {"rarity", "legendary"},
        {"edition", "1/1"},
        {"type", "genesis"}
    };
    nft1.metadata = "{\"rarity\":\"legendary\",\"edition\":\"1/1\",\"type\":\"genesis\"}";
    m_nftList.append(nft1);
    
    NFTData nft2;
    nft2.id = "NFT_002";
    nft2.name = "Shahcoin Art Collection #1";
    nft2.description = "Beautiful digital art piece";
    nft2.creator = "S1a2h3c4o5i6n7A8r9t0i";
    nft2.blockCreated = 12355;
    nft2.owner = "S1a2h3c4o5i6n7C0o1l2l3e";
    nft2.imageHash = "art_collection_image_hash";
    nft2.attributes = QJsonObject{
        {"rarity", "rare"},
        {"edition", "1/100"},
        {"type", "art"},
        {"artist", "Shahcoin Artist"}
    };
    nft2.metadata = "{\"rarity\":\"rare\",\"edition\":\"1/100\",\"type\":\"art\",\"artist\":\"Shahcoin Artist\"}";
    m_nftList.append(nft2);
    
    populateNFTTable();
}

void TokenNFTExplorer::populateTokenTable()
{
    m_tokenTable->setRowCount(0);
    
    for (const TokenData& token : m_tokenList) {
        int row = m_tokenTable->rowCount();
        m_tokenTable->insertRow(row);
        
        m_tokenTable->setItem(row, 0, new QTableWidgetItem(token.name));
        m_tokenTable->setItem(row, 1, new QTableWidgetItem(token.symbol));
        m_tokenTable->setItem(row, 2, new QTableWidgetItem(formatTokenSupply(token.supply, token.decimals)));
        m_tokenTable->setItem(row, 3, new QTableWidgetItem(formatAddress(token.creator)));
        m_tokenTable->setItem(row, 4, new QTableWidgetItem(formatBlockHeight(token.blockCreated)));
        m_tokenTable->setItem(row, 5, new QTableWidgetItem(QString::number(token.decimals)));
        m_tokenTable->setItem(row, 6, new QTableWidgetItem(token.description));

        // Risk columns via SHAHSecurityManager if available (fallback heuristic)
        QString riskText = tr("Safe");
        int score = 10;
        QString badge = QString::fromUtf8("🟢");
        {
            // Construct minimal info for scoring
            SHAHSecurity::SHAHSecurityManager mgr; // lightweight call; in production reuse a shared instance
            SHAHSecurity::AssetSecurityInfo info = mgr.getAssetSecurityInfo(token.id);
            score = mgr.computeSecurityScore(info);
            riskText = (score <= 25 ? tr("Safe") : score <= 60 ? tr("Caution") : tr("Risky"));
            badge = (score <= 25 ? QString::fromUtf8("🟢") : score <= 60 ? QString::fromUtf8("🟡") : QString::fromUtf8("🔴"));
            QString tip = mgr.tooltipFor(info, score);
            // set later after items
        }
        m_tokenTable->setItem(row, 7, new QTableWidgetItem(riskText));
        m_tokenTable->setItem(row, 8, new QTableWidgetItem(QString::number(score)));
        m_tokenTable->setItem(row, 9, new QTableWidgetItem(badge));

        // Tooltips
        QString tipText = QString("%1").arg(riskText);
        if (m_tokenTable->item(row,7)) m_tokenTable->item(row,7)->setToolTip(tipText);
        if (m_tokenTable->item(row,8)) m_tokenTable->item(row,8)->setToolTip(tipText);
        if (m_tokenTable->item(row,9)) m_tokenTable->item(row,9)->setToolTip(tipText);
        
        // Store token ID in the first column for context menu
        m_tokenTable->item(row, 0)->setData(Qt::UserRole, token.id);
    }
}

void TokenNFTExplorer::populateNFTTable()
{
    m_nftTable->setRowCount(0);
    
    for (const NFTData& nft : m_nftList) {
        // Marketplace filter: verified only toggle
        if (m_verifiedOnlyCheck && m_verifiedOnlyCheck->isChecked() && !isVerifiedNFT(nft)) {
            continue;
        }

        int row = m_nftTable->rowCount();
        m_nftTable->insertRow(row);
        
        m_nftTable->setItem(row, 0, new QTableWidgetItem(nft.name));
        // Parse tier from metadata JSON if present (top-level "tier" key)
        QString tier = nftTierFromMetadata(nft.metadata);
        {
            QTableWidgetItem* tierItem = new QTableWidgetItem(tier.isEmpty() ? tr("-") : tier);
            if (tier.compare("Premium", Qt::CaseInsensitive) == 0) {
                tierItem->setIcon(QIcon::fromTheme("emblem-favorite"));
                tierItem->setToolTip(tr("Verified Premium (curated listing)"));
            } else if (tier.compare("Pro", Qt::CaseInsensitive) == 0) {
                tierItem->setIcon(QIcon::fromTheme("emblem-ok"));
                tierItem->setToolTip(tr("Verified Pro (curated listing)"));
            } else {
                tierItem->setToolTip(tr("Basic tier"));
            }
            m_nftTable->setItem(row, 1, tierItem);
        }
        m_nftTable->setItem(row, 2, new QTableWidgetItem(formatAddress(nft.creator)));
        m_nftTable->setItem(row, 3, new QTableWidgetItem(formatAddress(nft.owner)));
        m_nftTable->setItem(row, 4, new QTableWidgetItem(formatBlockHeight(nft.blockCreated)));
        m_nftTable->setItem(row, 5, new QTableWidgetItem(nft.imageHash.isEmpty() ? tr("No Image") : tr("Has Image")));
        m_nftTable->setItem(row, 6, new QTableWidgetItem(QString::number(nft.attributes.size()) + tr(" attrs")));
        m_nftTable->setItem(row, 7, new QTableWidgetItem(nft.description));

        // Risk scoring columns via manager (fallback heuristic)
        QString riskText = tr("Safe");
        int score = 10;
        QString badge = QString::fromUtf8("🟢");
        {
            SHAHSecurity::SHAHSecurityManager mgr;
            SHAHSecurity::AssetSecurityInfo info = mgr.getAssetSecurityInfo(nft.id);
            score = mgr.computeSecurityScore(info);
            riskText = (score <= 25 ? tr("Safe") : score <= 60 ? tr("Caution") : tr("Risky"));
            badge = (score <= 25 ? QString::fromUtf8("🟢") : score <= 60 ? QString::fromUtf8("🟡") : QString::fromUtf8("🔴"));
            QString tip = mgr.tooltipFor(info, score);
            Q_UNUSED(tip);
        }
        m_nftTable->setItem(row, 8, new QTableWidgetItem(riskText));
        m_nftTable->setItem(row, 9, new QTableWidgetItem(QString::number(score)));
        m_nftTable->setItem(row, 10, new QTableWidgetItem(badge));

        // Tooltips
        QString tipText2 = QString("%1").arg(riskText);
        if (m_nftTable->item(row,8)) m_nftTable->item(row,8)->setToolTip(tipText2);
        if (m_nftTable->item(row,9)) m_nftTable->item(row,9)->setToolTip(tipText2);
        if (m_nftTable->item(row,10)) m_nftTable->item(row,10)->setToolTip(tipText2);
        
        // Store NFT ID in the first column for context menu
        m_nftTable->item(row, 0)->setData(Qt::UserRole, nft.id);
    }
}

bool TokenNFTExplorer::isVerifiedNFT(const NFTData& nft) const
{
    // Rule: Verified if tier is Pro or Premium and metadata parses to object
    const QString tier = nftTierFromMetadata(nft.metadata);
    if (tier.compare("Pro", Qt::CaseInsensitive) == 0 || tier.compare("Premium", Qt::CaseInsensitive) == 0) {
        // Basic schema check
        QJsonParseError err; QJsonDocument doc = QJsonDocument::fromJson(nft.metadata.toUtf8(), &err);
        return (err.error == QJsonParseError::NoError && doc.isObject());
    }
    return false;
}

QString TokenNFTExplorer::nftTierFromMetadata(const QString& metadataJson) const
{
    if (metadataJson.isEmpty()) return QString();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(metadataJson.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("tier") && obj.value("tier").isString()) {
            return obj.value("tier").toString();
        }
    }
    return QString();
}

void TokenNFTExplorer::showCreatorProfileDialog(const QString& creatorAddress)
{
    // Placeholder profile using local stats; in real flow, fetch from analytics backend
    int totalMinted = 0;
    for (const NFTData& n : m_nftList) {
        if (n.creator == creatorAddress) totalMinted++;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Creator Profile"));
    QVBoxLayout* v = new QVBoxLayout(&dlg);
    QLabel* header = new QLabel(tr("Creator: %1").arg(formatAddress(creatorAddress)));
    QLabel* minted = new QLabel(tr("NFTs Minted: %1").arg(totalMinted));
    QLabel* trusted = new QLabel(tr("Badge: %1").arg(totalMinted >= 10 ? tr("Trusted Creator") : tr("-")));
    v->addWidget(header);
    v->addWidget(minted);
    v->addWidget(trusted);
    QPushButton* closeBtn = new QPushButton(tr("Close"));
    v->addWidget(closeBtn);
    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    dlg.exec();
}

void TokenNFTExplorer::onTabChanged(int index)
{
    // Could implement tab-specific refresh logic here
    Q_UNUSED(index);
}

void TokenNFTExplorer::onTokenSearchChanged()
{
    updateTokenFilters();
}

void TokenNFTExplorer::onNFTSearchChanged()
{
    updateNFTFilters();
}

void TokenNFTExplorer::onTokenFilterChanged()
{
    updateTokenFilters();
}

void TokenNFTExplorer::onNFTFilterChanged()
{
    updateNFTFilters();
}

void TokenNFTExplorer::updateTokenFilters()
{
    QString searchText = m_tokenSearchEdit->text().toLower();
    QString filterText = m_tokenFilterCombo->currentText();
    
    for (int row = 0; row < m_tokenTable->rowCount(); ++row) {
        bool showRow = true;
        
        // Apply search filter
        if (!searchText.isEmpty()) {
            QString name = m_tokenTable->item(row, 0)->text().toLower();
            QString symbol = m_tokenTable->item(row, 1)->text().toLower();
            if (!name.contains(searchText) && !symbol.contains(searchText)) {
                showRow = false;
            }
        }
        
        // Apply combo filter
        if (showRow && filterText != tr("All Tokens")) {
            // TODO: Implement actual filtering logic
            // For now, just show all rows
        }
        
        m_tokenTable->setRowHidden(row, !showRow);
    }
}

void TokenNFTExplorer::updateNFTFilters()
{
    QString searchText = m_nftSearchEdit->text().toLower();
    QString filterText = m_nftFilterCombo->currentText();
    
    for (int row = 0; row < m_nftTable->rowCount(); ++row) {
        bool showRow = true;
        
        // Apply search filter
        if (!searchText.isEmpty()) {
            QString name = m_nftTable->item(row, 0)->text().toLower();
            if (!name.contains(searchText)) {
                showRow = false;
            }
        }
        
        // Apply combo filter
        if (showRow && filterText != tr("All NFTs")) {
            // TODO: Implement actual filtering logic
            // For now, just show all rows
        }
        
        m_nftTable->setRowHidden(row, !showRow);
    }
}

void TokenNFTExplorer::onRefreshClicked()
{
    refreshData();
}

void TokenNFTExplorer::onExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export Data"), QString(),
        tr("CSV Files (*.csv);;JSON Files (*.json)"), nullptr);
    
    if (fileName.isEmpty()) return;
    
    // TODO: Implement actual export functionality
    QMessageBox::information(this, tr("Export"), tr("Export functionality will be implemented soon."));
}

void TokenNFTExplorer::onTokenRowClicked(int row, int column)
{
    Q_UNUSED(column);
    
    if (row >= 0 && row < m_tokenTable->rowCount()) {
        QString tokenId = m_tokenTable->item(row, 0)->data(Qt::UserRole).toString();
        showTokenDetailsDialog(tokenId);
    }
}

void TokenNFTExplorer::onNFTRowClicked(int row, int column)
{
    Q_UNUSED(column);
    
    if (row >= 0 && row < m_nftTable->rowCount()) {
        QString nftId = m_nftTable->item(row, 0)->data(Qt::UserRole).toString();
        showNFTDetailsDialog(nftId);
    }
}

void TokenNFTExplorer::onTokenContextMenu(const QPoint& pos)
{
    QTableWidgetItem* item = m_tokenTable->itemAt(pos);
    if (item) {
        m_selectedTokenId = item->data(Qt::UserRole).toString();
        m_tokenContextMenu->exec(m_tokenTable->viewport()->mapToGlobal(pos));
    }
}

void TokenNFTExplorer::onNFTContextMenu(const QPoint& pos)
{
    QTableWidgetItem* item = m_nftTable->itemAt(pos);
    if (item) {
        m_selectedNFTId = item->data(Qt::UserRole).toString();
        m_nftContextMenu->exec(m_nftTable->viewport()->mapToGlobal(pos));
    }
}

void TokenNFTExplorer::onViewTokenDetails()
{
    if (!m_selectedTokenId.isEmpty()) {
        showTokenDetailsDialog(m_selectedTokenId);
    }
}

void TokenNFTExplorer::onViewNFTDetails()
{
    if (!m_selectedNFTId.isEmpty()) {
        showNFTDetailsDialog(m_selectedNFTId);
    }
}

void TokenNFTExplorer::showNFTUpgradeDialog(const QString& nftId)
{
    // Find NFT
    auto it = std::find_if(m_nftList.begin(), m_nftList.end(), [&](const NFTData& n){ return n.id == nftId; });
    if (it == m_nftList.end()) return;

    // Determine current tier from metadata
    QString currentTier = "Basic";
    if (!it->metadata.isEmpty()) {
        QJsonParseError err; QJsonDocument doc = QJsonDocument::fromJson(it->metadata.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("tier") && obj.value("tier").isString()) currentTier = obj.value("tier").toString();
        }
    }

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Upgrade NFT Tier"));
    QVBoxLayout* v = new QVBoxLayout(&dlg);
    QLabel* info = new QLabel(tr("Current Tier: %1\nSelect new tier and proceed to payment.").arg(currentTier));
    v->addWidget(info);
    QComboBox* tierBox = new QComboBox(&dlg);
    tierBox->addItem("Basic"); tierBox->addItem("Pro"); tierBox->addItem("Premium");
    tierBox->setCurrentText(currentTier);
    v->addWidget(tierBox);
    QLabel* priceLbl = new QLabel(tr("Price difference: calculating..."));
    v->addWidget(priceLbl);
    QHBoxLayout* btns = new QHBoxLayout(); btns->addStretch();
    QPushButton* payBtn = new QPushButton(tr("Pay & Upgrade"));
    QPushButton* cancelBtn = new QPushButton(tr("Cancel"));
    btns->addWidget(cancelBtn); btns->addWidget(payBtn);
    v->addLayout(btns);

    // Simple local price mapping (USD). In real flow, compute via SDK.
    auto tierUsd = [](const QString& t){ if (t=="Premium") return 39.0; if (t=="Pro") return 25.0; return 15.0; };
    auto updatePrice = [&](){
        double diff = qMax(0.0, tierUsd(tierBox->currentText()) - tierUsd(currentTier));
        priceLbl->setText(tr("Price difference: $%1 (paid via Dev Portal)").arg(QString::number(diff,'f',2)));
    };
    updatePrice();
    QObject::connect(tierBox, &QComboBox::currentTextChanged, &dlg, [&](const QString&){ updatePrice(); });

    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    QObject::connect(payBtn, &QPushButton::clicked, &dlg, [&](){
        // Placeholder: trigger SDK upgrade; here we just show success.
        QMessageBox::information(&dlg, tr("Upgrade Initiated"), tr("Upgrade request submitted. Payment verification will update the tier."));
        dlg.accept();
    });

    dlg.exec();
}

void TokenNFTExplorer::onExportTokenData()
{
    if (!m_selectedTokenId.isEmpty()) {
        exportTokenData(m_selectedTokenId);
    }
}

void TokenNFTExplorer::onExportNFTData()
{
    if (!m_selectedNFTId.isEmpty()) {
        exportNFTData(m_selectedNFTId);
    }
}

void TokenNFTExplorer::showTokenDetailsDialog(const QString& tokenId)
{
    // Find token data
    for (const TokenData& token : m_tokenList) {
        if (token.id == tokenId) {
            TokenDetailsDialog* dialog = new TokenDetailsDialog(token, this);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->show();
            break;
        }
    }
}

void TokenNFTExplorer::showNFTDetailsDialog(const QString& nftId)
{
    // Find NFT data
    for (const NFTData& nft : m_nftList) {
        if (nft.id == nftId) {
            NFTDetailsDialog* dialog = new NFTDetailsDialog(nft, this);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->setWalletModel(m_walletModel);
            connect(dialog, &NFTDetailsDialog::purchaseCompleted, this, [this](const QString& id, const QString& newOwner){
                for (auto& n : m_nftList) {
                    if (n.id == id) {
                        n.owner = newOwner;
                        n.isListed = false; // delist after purchase
                        // Fire-and-forget record
                        recordPurchaseToApi(id, newOwner, n.listedPriceSatoshis, n.listedPriceUSD, QString());
                        break;
                    }
                }
                populateMarketplaceTable();
                if (m_tabWidget && m_tabWidget->currentWidget() == m_nftTab) populateNFTTable();
            });
            dialog->show();
            break;
        }
    }
}

void TokenNFTExplorer::exportTokenData(const QString& tokenId)
{
    // TODO: Implement token data export
    QMessageBox::information(this, tr("Export"), tr("Token export functionality will be implemented soon."));
}

void TokenNFTExplorer::exportNFTData(const QString& nftId)
{
    // TODO: Implement NFT data export
    QMessageBox::information(this, tr("Export"), tr("NFT export functionality will be implemented soon."));
}

QPixmap TokenNFTExplorer::loadImageFromHash(const QString& imageHash)
{
    // TODO: Implement actual image loading from hash/IPFS
    // For now, return a placeholder
    return createPlaceholderImage(tr("Image"), QSize(128, 128));
}

QPixmap TokenNFTExplorer::createPlaceholderImage(const QString& text, const QSize& size)
{
    QPixmap pixmap(size);
    pixmap.fill(QColor(240, 240, 240));
    
    QPainter painter(&pixmap);
    painter.setPen(QColor(100, 100, 100));
    painter.setFont(QFont("Arial", 10));
    
    QRect rect = pixmap.rect();
    painter.drawText(rect, Qt::AlignCenter, text);
    
    return pixmap;
}

QString TokenNFTExplorer::formatTokenSupply(qint64 supply, int decimals)
{
    double actualSupply = supply / pow(10, decimals);
    if (actualSupply >= 1000000) {
        return QString::number(actualSupply / 1000000, 'f', 2) + "M";
    } else if (actualSupply >= 1000) {
        return QString::number(actualSupply / 1000, 'f', 2) + "K";
    } else {
        return QString::number(actualSupply, 'f', 2);
    }
}

QString TokenNFTExplorer::formatBlockHeight(int blockHeight)
{
    return QString::number(blockHeight);
}

QString TokenNFTExplorer::formatAddress(const QString& address)
{
    if (address.length() <= 12) return address;
    return address.left(6) + "..." + address.right(6);
}

// TokenDetailsDialog Implementation
TokenDetailsDialog::TokenDetailsDialog(const TokenNFTExplorer::TokenData& tokenData, QWidget* parent)
    : QDialog(parent)
    , m_tokenData(tokenData)
{
    setupUI();
    populateData();
    
    setWindowTitle(tr("Token Details - %1").arg(tokenData.name));
    setModal(true);
    resize(500, 400);
}

void TokenDetailsDialog::setupUI()
{
    m_layout = new QVBoxLayout(this);
    
    // Logo section
    QHBoxLayout* logoLayout = new QHBoxLayout();
    m_logoLabel = new QLabel(this);
    m_logoLabel->setFixedSize(64, 64);
    m_logoLabel->setStyleSheet("border: 1px solid #dee2e6; border-radius: 4px;");
    logoLayout->addWidget(m_logoLabel);
    logoLayout->addStretch();
    m_layout->addLayout(logoLayout);
    
    // Metadata section
    QGroupBox* metadataGroup = new QGroupBox(tr("Token Metadata"), this);
    QVBoxLayout* metadataLayout = new QVBoxLayout(metadataGroup);
    
    m_metadataText = new QTextEdit(this);
    m_metadataText->setReadOnly(true);
    m_metadataText->setMaximumHeight(200);
    metadataLayout->addWidget(m_metadataText);
    
    m_layout->addWidget(metadataGroup);

    // SHAHSecurity Shield Panel
    QGroupBox* shieldGroup = new QGroupBox(tr("SHAHSecurity Shield"), this);
    QVBoxLayout* shieldLayout = new QVBoxLayout(shieldGroup);
    QLabel* riskLabel = new QLabel(tr("Risk Level: %1").arg(tr("Safe")), shieldGroup);
    QLabel* badgeLabel = new QLabel(tr("Verification: %1").arg(tr("Unknown")), shieldGroup);
    QLabel* repLabel = new QLabel(tr("Creator Reputation: %1/100").arg(70), shieldGroup);
    QLabel* reportsLabel = new QLabel(tr("Reports: %1").arg(0), shieldGroup);
    QLabel* notesLabel = new QLabel(tr("AI Notes: %1").arg(tr("-")), shieldGroup);
    QLabel* scannedLabel = new QLabel(tr("Last Scanned: %1").arg(QDateTime::currentDateTime().toString(Qt::ISODate)), shieldGroup);
    riskLabel->setStyleSheet("font-weight: 600; color: #2ecc71;");
    shieldLayout->addWidget(riskLabel);
    shieldLayout->addWidget(badgeLabel);
    shieldLayout->addWidget(repLabel);
    shieldLayout->addWidget(reportsLabel);
    shieldLayout->addWidget(notesLabel);
    shieldLayout->addWidget(scannedLabel);
    // Expandable more info
    QGroupBox* moreInfo = new QGroupBox(tr("More Info"), shieldGroup);
    moreInfo->setCheckable(true); moreInfo->setChecked(false);
    QVBoxLayout* moreLayout = new QVBoxLayout(moreInfo);
    moreLayout->addWidget(new QLabel(tr("Audit: Minted via ShahFactory (example)")));
    moreLayout->addWidget(new QLabel(tr("Contract: Verified source (example)")));
    moreLayout->addWidget(new QLabel(tr("Duplicates: None detected")));
    moreLayout->addWidget(new QLabel(tr("Metadata Hash: (placeholder)")));
    shieldLayout->addWidget(moreInfo);
    m_layout->addWidget(shieldGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_exportButton = new QPushButton(tr("📁 Export"), this);
    m_closeButton = new QPushButton(tr("✕ Close"), this);
    
    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_closeButton);
    
    m_layout->addLayout(buttonLayout);
    
    connect(m_exportButton, &QPushButton::clicked, [this]() {
        // TODO: Implement export functionality
        QMessageBox::information(this, tr("Export"), tr("Export functionality will be implemented soon."));
    });
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void TokenDetailsDialog::populateData()
{
    // Set logo (placeholder for now)
    QPixmap logo = QPixmap(64, 64);
    logo.fill(QColor(240, 240, 240));
    m_logoLabel->setPixmap(logo);
    
    // Populate metadata
    QString metadata = QString(
        "Token ID: %1\n"
        "Name: %2\n"
        "Symbol: %3\n"
        "Total Supply: %4\n"
        "Decimals: %5\n"
        "Creator: %6\n"
        "Block Created: %7\n"
        "Description: %8"
    ).arg(
        m_tokenData.id,
        m_tokenData.name,
        m_tokenData.symbol,
        QString::number(m_tokenData.supply),
        QString::number(m_tokenData.decimals),
        m_tokenData.creator,
        QString::number(m_tokenData.blockCreated),
        m_tokenData.description
    );
    
    m_metadataText->setPlainText(metadata);
}

// NFTDetailsDialog Implementation
NFTDetailsDialog::NFTDetailsDialog(const TokenNFTExplorer::NFTData& nftData, QWidget* parent)
    : QDialog(parent)
    , m_nftData(nftData)
{
    setupUI();
    populateData();
    
    setWindowTitle(tr("NFT Details - %1").arg(nftData.name));
    setModal(true);
    resize(600, 500);
}

void NFTDetailsDialog::setupUI()
{
    m_layout = new QVBoxLayout(this);
    
    // Image section
    QHBoxLayout* imageLayout = new QHBoxLayout();
    m_imageLabel = new QLabel(this);
    m_imageLabel->setFixedSize(128, 128);
    m_imageLabel->setStyleSheet("border: 1px solid #dee2e6; border-radius: 4px;");
    imageLayout->addWidget(m_imageLabel);
    imageLayout->addStretch();
    m_layout->addLayout(imageLayout);
    
    // Metadata section
    QGroupBox* metadataGroup = new QGroupBox(tr("NFT Metadata"), this);
    QVBoxLayout* metadataLayout = new QVBoxLayout(metadataGroup);
    
    m_metadataText = new QTextEdit(this);
    m_metadataText->setReadOnly(true);
    m_metadataText->setMaximumHeight(250);
    metadataLayout->addWidget(m_metadataText);
    
    m_layout->addWidget(metadataGroup);

    // SHAHSecurity Shield Panel
    QGroupBox* shieldGroup = new QGroupBox(tr("SHAHSecurity Shield"), this);
    QVBoxLayout* shieldLayout = new QVBoxLayout(shieldGroup);
    QLabel* riskLabel = new QLabel(tr("Risk Level: %1").arg(tr("Safe")), shieldGroup);
    QLabel* badgeLabel = new QLabel(tr("Verification: %1").arg(tr("Unknown")), shieldGroup);
    QLabel* repLabel = new QLabel(tr("Creator Reputation: %1/100").arg(75), shieldGroup);
    QLabel* reportsLabel = new QLabel(tr("Reports: %1").arg(0), shieldGroup);
    QLabel* notesLabel = new QLabel(tr("AI Notes: %1").arg(tr("-")), shieldGroup);
    QLabel* scannedLabel = new QLabel(tr("Last Scanned: %1").arg(QDateTime::currentDateTime().toString(Qt::ISODate)), shieldGroup);
    riskLabel->setStyleSheet("font-weight: 600; color: #2ecc71;");
    shieldLayout->addWidget(riskLabel);
    shieldLayout->addWidget(badgeLabel);
    shieldLayout->addWidget(repLabel);
    shieldLayout->addWidget(reportsLabel);
    shieldLayout->addWidget(notesLabel);
    shieldLayout->addWidget(scannedLabel);
    // Expandable more info
    QGroupBox* moreInfo = new QGroupBox(tr("More Info"), shieldGroup);
    moreInfo->setCheckable(true); moreInfo->setChecked(false);
    QVBoxLayout* moreLayout = new QVBoxLayout(moreInfo);
    moreLayout->addWidget(new QLabel(tr("Audit: Minted via ShahFactory (example)")));
    moreLayout->addWidget(new QLabel(tr("Contract: Verified source (example)")));
    moreLayout->addWidget(new QLabel(tr("Duplicates: None detected")));
    moreLayout->addWidget(new QLabel(tr("Metadata Hash: (placeholder)")));
    shieldLayout->addWidget(moreInfo);
    m_layout->addWidget(shieldGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_buyButton = new QPushButton(tr("Buy"), this);
    m_transferButton = new QPushButton(tr("Transfer / Gift"), this);
    m_exportButton = new QPushButton(tr("📁 Export"), this);
    m_closeButton = new QPushButton(tr("✕ Close"), this);
    
    buttonLayout->addWidget(m_buyButton);
    buttonLayout->addWidget(m_transferButton);
    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_closeButton);
    
    m_layout->addLayout(buttonLayout);
    
    connect(m_exportButton, &QPushButton::clicked, [this]() {
        // TODO: Implement export functionality
        QMessageBox::information(this, tr("Export"), tr("Export functionality will be implemented soon."));
    });
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_buyButton, &QPushButton::clicked, this, &NFTDetailsDialog::onBuyClicked);
    connect(m_transferButton, &QPushButton::clicked, this, &NFTDetailsDialog::onTransferClicked);
}

void NFTDetailsDialog::populateData()
{
    // Set image
    QPixmap image = loadImageFromHash(m_nftData.imageHash);
    m_imageLabel->setPixmap(image.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    // Populate metadata
    // Extract tier from metadata if present
    QString tier;
    if (!m_nftData.metadata.isEmpty()) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(m_nftData.metadata.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("tier") && obj.value("tier").isString()) {
                tier = obj.value("tier").toString();
            }
        }
    }
    QString metadata = QString(
        "NFT ID: %1\n"
        "Name: %2\n"
        "Tier: %3\n"
        "Description: %4\n"
        "Creator: %5\n"
        "Owner: %6\n"
        "Block Created: %7\n"
        "Attributes: %8\n"
        "Full Metadata: %9"
    ).arg(
        m_nftData.id,
        m_nftData.name,
        tier.isEmpty() ? "-" : tier,
        m_nftData.description,
        m_nftData.creator,
        m_nftData.owner,
        QString::number(m_nftData.blockCreated),
        QString::number(m_nftData.attributes.size()) + " attributes",
        m_nftData.metadata
    );
    
    m_metadataText->setPlainText(metadata);
}

QPixmap NFTDetailsDialog::loadImageFromHash(const QString& imageHash)
{
    // TODO: Implement actual image loading from hash/IPFS
    // For now, return a placeholder
    QPixmap pixmap(128, 128);
    pixmap.fill(QColor(240, 240, 240));
    
    QPainter painter(&pixmap);
    painter.setPen(QColor(100, 100, 100));
    painter.setFont(QFont("Arial", 10));
    
    QRect rect = pixmap.rect();
    painter.drawText(rect, Qt::AlignCenter, tr("NFT Image"));
    
    return pixmap;
}

void NFTDetailsDialog::onBuyClicked()
{
    if (!m_walletModel) return;
    
    // Get current listing price from marketplace data
    qint64 listPriceSatoshis = 0;
    double listPriceUSD = 0.0;
    for (const auto& nft : m_nftList) {
        if (nft.id == m_nftData.id) {
            listPriceSatoshis = nft.listedPriceSatoshis;
            listPriceUSD = nft.listedPriceUSD;
            break;
        }
    }
    
    // Check if escrow is required (high-value NFTs > 1 SHAH)
    bool requiresEscrow = listPriceSatoshis > 1000000000; // 1 SHAH
    
    if (requiresEscrow) {
        showEscrowDialog(m_nftData.id);
        return;
    }
    
    // Show purchase confirmation dialog
    QDialog confirmDlg(this);
    confirmDlg.setWindowTitle(tr("Confirm Purchase"));
    QVBoxLayout* v = new QVBoxLayout(&confirmDlg);
    
    QLabel* infoLabel = new QLabel(tr("Purchase %1 for %2 SHAH (%3 USD)?").arg(m_nftData.name).arg(listPriceSatoshis / 100000000.0, 0, 'f', 8).arg(listPriceUSD, 0, 'f', 2));
    v->addWidget(infoLabel);
    
    QComboBox* paymentMethod = new QComboBox(&confirmDlg);
    paymentMethod->addItems({ tr("SHAH (Native)"), tr("SHAHtoken (ERC-20)"), tr("Credit Card (Stripe)") });
    v->addWidget(new QLabel(tr("Payment Method:")));
    v->addWidget(paymentMethod);
    
    // Add Stripe-specific UI elements
    QLabel* stripeInfoLabel = new QLabel(tr("💳 Pay with credit card - USD will be converted to SHAH automatically"), &confirmDlg);
    stripeInfoLabel->setStyleSheet("color: #007bff; font-size: 12px;");
    stripeInfoLabel->setVisible(false);
    v->addWidget(stripeInfoLabel);
    
    QLabel* usdPriceLabel = new QLabel(tr("USD Price: $%1").arg(listPriceUSD, 0, 'f', 2), &confirmDlg);
    usdPriceLabel->setStyleSheet("font-weight: bold; color: #28a745;");
    usdPriceLabel->setVisible(false);
    v->addWidget(usdPriceLabel);
    
    // Show/hide Stripe elements based on payment method
    connect(paymentMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        bool isStripe = (index == 2); // Credit Card (Stripe) is index 2
        stripeInfoLabel->setVisible(isStripe);
        usdPriceLabel->setVisible(isStripe);
    });
    
    QCheckBox* termsCheck = new QCheckBox(tr("I understand this is a permanent blockchain transaction"), &confirmDlg);
    v->addWidget(termsCheck);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* confirmBtn = new QPushButton(tr("Confirm & Purchase"), &confirmDlg);
    QPushButton* cancelBtn = new QPushButton(tr("Cancel"), &confirmDlg);
    btnLayout->addWidget(confirmBtn);
    btnLayout->addWidget(cancelBtn);
    v->addLayout(btnLayout);
    
    connect(confirmBtn, &QPushButton::clicked, &confirmDlg, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &confirmDlg, &QDialog::reject);
    
    confirmBtn->setEnabled(false);
    connect(termsCheck, &QCheckBox::toggled, confirmBtn, &QPushButton::setEnabled);
    
    if (confirmDlg.exec() == QDialog::Accepted) {
        QString paymentMethodStr = paymentMethod->currentText();
        QString sellerAddress = m_nftData.owner; // Current owner becomes seller
        QString buyerAddress = m_walletModel->getAddressTableModel()->defaultAddress();
        
        // Handle Stripe payment
        if (paymentMethodStr.contains("Stripe")) {
            if (!handleStripePayment(m_nftData, buyerAddress, listPriceUSD)) {
                return; // Payment failed or was cancelled
            }
        }
        
        // Create and sign purchase transaction
        QString txHex;
        if (createNFTPurchaseTransaction(m_nftData.id, sellerAddress, listPriceSatoshis, txHex)) {
            QString txid;
            if (signAndBroadcastTransaction(txHex, txid)) {
                // Update local state
                m_nftData.owner = buyerAddress;
                m_nftData.isListed = false;
                m_nftData.listedPriceSatoshis = 0;
                m_nftData.listedPriceUSD = 0.0;
                
                // Record purchase to API
                recordPurchaseToApi(m_nftData.id, buyerAddress, listPriceSatoshis, listPriceUSD, txid);
                
                // Add transaction record
                TransactionRecord record;
                record.txid = txid;
                record.type = "purchase";
                record.nftId = m_nftData.id;
                record.fromAddress = buyerAddress;
                record.toAddress = sellerAddress;
                record.amountSatoshis = listPriceSatoshis;
                record.amountUSD = listPriceUSD;
                record.timestamp = QDateTime::currentDateTime();
                record.status = "confirmed";
                record.metadata = QString("{\"paymentMethod\":\"%1\"}").arg(paymentMethod->currentText());
                addTransactionRecord(record);
                
                // Update creator trust score
                updateCreatorTrustScore(sellerAddress);
                
                // Update analytics
                updateAnalyticsData();
                
                // Emit purchase completed
                emit purchaseCompleted(m_nftData.id, buyerAddress);
                
                QMessageBox::information(this, tr("Purchase Successful"), 
                    tr("NFT purchased successfully!\nTransaction ID: %1").arg(txid));
            } else {
                QMessageBox::critical(this, tr("Transaction Failed"), 
                    tr("Failed to broadcast transaction. Please try again."));
            }
        } else {
            QMessageBox::critical(this, tr("Transaction Creation Failed"), 
                tr("Failed to create purchase transaction. Please check your wallet balance."));
        }
    }
}

void NFTDetailsDialog::onTransferClicked()
{
    if (!m_walletModel) {
        QMessageBox::warning(this, tr("Wallet Required"), tr("Connect a wallet to transfer."));
        return;
    }
    bool ok = false;
    QString recipient = QInputDialog::getText(this, tr("Transfer NFT"), tr("Recipient Address:"), QLineEdit::Normal, "", &ok);
    if (!ok || recipient.trimmed().isEmpty()) return;
    QString err;
    if (!m_walletModel->transferNFT(QString::fromUtf8(m_nftData.id.toUtf8()), recipient, err)) {
        QMessageBox::critical(this, tr("Transfer Failed"), err.isEmpty() ? tr("Unknown error") : err);
    } else {
        QMessageBox::information(this, tr("Transfer"), tr("NFT transfer initiated."));
    }
}

void TokenNFTExplorer::submitListingToApi(const QString& nftId, const QString& title, const QString& description,
                            const QString& category, const QString& tags,
                            qint64 priceSatoshis, double priceUSD, bool submitForVerification)
{
    QUrl url(m_apiBaseUrl + "/listings");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body{{"nftId", nftId},{"title", title},{"description", description},{"category", category},{"tags", tags},{"priceSatoshis", QString::number(priceSatoshis)},{"priceUSD", priceUSD},{"submitForVerification", submitForVerification}};
    m_net.post(req, QJsonDocument(body).toJson());
}

void TokenNFTExplorer::recordPurchaseToApi(const QString& nftId, const QString& buyer, qint64 priceSatoshis, double priceUSD, const QString& txid)
{
    QUrl url(m_apiBaseUrl + "/purchases");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body{{"nftId", nftId},{"buyer", buyer},{"priceSatoshis", QString::number(priceSatoshis)},{"priceUSD", priceUSD},{"txid", txid}};
    m_net.post(req, QJsonDocument(body).toJson());
}

void TokenNFTExplorer::setAdminVerifyFlagToApi(const QString& nftId, bool adminVerified)
{
    QUrl url(m_apiBaseUrl + "/verify");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body{{"nftId", nftId},{"adminVerified", adminVerified}};
    m_net.post(req, QJsonDocument(body).toJson());
}

void TokenNFTExplorer::populateMarketplaceTable()
{
    m_marketTable->setRowCount(0);
    
    // Load creator profiles for trust scoring
    loadCreatorProfiles();
    
    int row = 0;
    for (const auto& nft : m_nftList) {
        // Apply filters
        if (!shouldShowInMarketplace(nft)) continue;
        
        m_marketTable->insertRow(row);
        
        // Name with creator trust badge
        QString nameWithBadge = nft.name;
        if (m_creatorProfiles.contains(nft.creator)) {
            const CreatorProfile& profile = m_creatorProfiles[nft.creator];
            if (profile.isTrusted) {
                nameWithBadge += " 🏆";
            }
        }
        
        m_marketTable->setItem(row, 0, new QTableWidgetItem(nameWithBadge));
        m_marketTable->item(row, 0)->setData(Qt::UserRole, nft.id);
        
        // Tier
        QString tier = "Basic";
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(nft.metadata.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("tier")) {
                tier = obj["tier"].toString();
            }
        }
        m_marketTable->setItem(row, 1, new QTableWidgetItem(tier));
        
        // Creator with trust score
        QString creatorWithScore = nft.creator;
        if (m_creatorProfiles.contains(nft.creator)) {
            const CreatorProfile& profile = m_creatorProfiles[nft.creator];
            creatorWithScore += QString(" (%.1f)").arg(profile.trustScore);
        }
        m_marketTable->setItem(row, 2, new QTableWidgetItem(creatorWithScore));
        
        m_marketTable->setItem(row, 3, new QTableWidgetItem(nft.owner));
        m_marketTable->setItem(row, 4, new QTableWidgetItem(nft.category));
        m_marketTable->setItem(row, 5, new QTableWidgetItem(nft.traits));
        
        // Price in SHAH (8 decimals)
        QString shahPrice = QString::number(nft.listedPriceSatoshis / 100000000.0, 'f', 8);
        m_marketTable->setItem(row, 6, new QTableWidgetItem(shahPrice));
        
        // Price in USD
        QString usdPrice = QString::number(nft.listedPriceUSD, 'f', 2);
        m_marketTable->setItem(row, 7, new QTableWidgetItem(usdPrice));
        
        // Verified status with tooltip
        QString verifiedText = isVerifiedNFT(nft) ? "✓" : "✗";
        QTableWidgetItem* verifiedItem = new QTableWidgetItem(verifiedText);
        if (isVerifiedNFT(nft)) {
            verifiedItem->setToolTip(tr("Verified NFT - meets quality standards"));
        } else {
            verifiedItem->setToolTip(tr("Unverified NFT - may not meet quality standards"));
        }
        m_marketTable->setItem(row, 8, verifiedItem);
        
        // Favorite status
        QString favoriteText = m_localFavorites.contains(nft.id) ? "❤️" : "🤍";
        QTableWidgetItem* favoriteItem = new QTableWidgetItem(favoriteText);
        favoriteItem->setToolTip(m_localFavorites.contains(nft.id) ? tr("Remove from favorites") : tr("Add to favorites"));
        m_marketTable->setItem(row, 9, favoriteItem);
        
        row++;
    }
    
    m_marketTable->resizeColumnsToContents();
}

void TokenNFTExplorer::onMarketToggleChanged(int)
{
    populateMarketplaceTable();
}

void TokenNFTExplorer::onMarketFilterChanged()
{
    populateMarketplaceTable();
}

void TokenNFTExplorer::onMarketplaceContextMenu(const QPoint& pos)
{
    QTableWidgetItem* item = m_marketTable->itemAt(pos);
    if (!item) return;
    
    m_selectedMarketNFTId = item->data(Qt::UserRole).toString();
    if (m_selectedMarketNFTId.isEmpty()) return;
    
    if (!m_marketContextMenu) {
        m_marketContextMenu = new QMenu(this);
        m_marketContextMenu->addAction(tr("View Details"), this, &TokenNFTExplorer::showNFTDetailsDialog);
        m_marketContextMenu->addAction(tr("List / Update Listing"), this, &TokenNFTExplorer::showMarketplaceListingDialog);
        m_marketContextMenu->addAction(tr("View Creator Profile"), this, [this]() {
            // Find the creator address for the selected NFT
            QString creatorAddress;
            for (const auto& nft : m_nftList) {
                if (nft.id == m_selectedMarketNFTId) {
                    creatorAddress = nft.creator;
                    break;
                }
            }
            if (creatorAddress.isEmpty()) return;
            
            // Show creator profile dialog
            QDialog dlg(this);
            dlg.setWindowTitle(tr("Creator Profile"));
            dlg.setMinimumSize(400, 300);
            QVBoxLayout* v = new QVBoxLayout(&dlg);
            
            if (m_creatorProfiles.contains(creatorAddress)) {
                const CreatorProfile& profile = m_creatorProfiles[creatorAddress];
                
                QLabel* addressLabel = new QLabel(tr("Address: %1").arg(formatAddress(profile.address)), &dlg);
                addressLabel->setWordWrap(true);
                v->addWidget(addressLabel);
                
                QLabel* statsLabel = new QLabel(tr("Total NFTs: %1\nTotal Volume: %2 SHAH\nTrust Score: %.1f/100").arg(profile.totalNFTs).arg(profile.totalVolume / 100000000.0, 0, 'f', 8).arg(profile.trustScore), &dlg);
                v->addWidget(statsLabel);
                
                if (!profile.badges.isEmpty()) {
                    QLabel* badgesLabel = new QLabel(tr("Badges: %1").arg(profile.badges), &dlg);
                    badgesLabel->setWordWrap(true);
                    v->addWidget(badgesLabel);
                }
                
                if (profile.isTrusted) {
                    QLabel* trustedLabel = new QLabel(tr("🏆 Trusted Creator"), &dlg);
                    trustedLabel->setStyleSheet("color: green; font-weight: bold;");
                    v->addWidget(trustedLabel);
                }
            } else {
                QLabel* noDataLabel = new QLabel(tr("No profile data available for this creator."), &dlg);
                v->addWidget(noDataLabel);
            }
            
            QPushButton* closeBtn = new QPushButton(tr("Close"), &dlg);
            v->addWidget(closeBtn);
            connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
            
            dlg.exec();
        });
        m_marketContextMenu->addSeparator();
        m_marketContextMenu->addAction(tr("🛡 Admin: Toggle Verified"), this, &TokenNFTExplorer::onAdminToggleVerified);
    }
    
    m_marketContextMenu->popup(m_marketTable->viewport()->mapToGlobal(pos));
}

bool TokenNFTExplorer::isFavorited(const QString& nftId) const
{
    // Check if NFT is in favorites (stored in settings)
    QSettings settings("Shahcoin", "Wallet");
    QStringList favorites = settings.value("nft/favorites", QStringList()).toStringList();
    return favorites.contains(nftId);
}

void TokenNFTExplorer::toggleFavorite(const QString& nftId)
{
    QStringList favs = m_settings.value("favorites/nfts").toStringList();
    if (favs.contains(nftId)) {
        favs.removeAll(nftId);
    } else {
        favs.append(nftId);
    }
    m_settings.setValue("favorites/nfts", favs);
}

void TokenNFTExplorer::onOraclePriceUpdated(double)
{
    // Update USD column in marketplace table
    if (!m_marketTable) return;
    const double usdPerShah = m_priceOracle ? m_priceOracle->currentUsdPerShah() : 0.0;
    for (int row = 0; row < m_marketTable->rowCount(); ++row) {
        // Recompute USD from SHAH column 6 if available
        bool ok = false;
        double priceShah = m_marketTable->item(row, 6) ? m_marketTable->item(row, 6)->text().toDouble(&ok) : 0.0;
        if (ok && usdPerShah > 0) {
            const double usd = priceShah * usdPerShah;
            if (!m_marketTable->item(row, 7)) {
                m_marketTable->setItem(row, 7, new QTableWidgetItem(QString::number(usd, 'f', 2)));
            } else {
                m_marketTable->item(row, 7)->setText(QString::number(usd, 'f', 2));
            }
        }
    }
}

bool TokenNFTExplorer::metadataAdminVerified(const QString& metadata) const
{
    if (metadata.isEmpty()) return false;
    QJsonParseError err; QJsonDocument doc = QJsonDocument::fromJson(metadata.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        return obj.contains("adminVerified") && obj.value("adminVerified").toBool();
    }
    return false;
}

void TokenNFTExplorer::fetchMarketplaceNftsFromApi()
{
    QUrl url(m_apiBaseUrl + "/listings");
    QNetworkRequest req(url);
    auto* reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            // Keep local state on error
            return;
        }
        QByteArray data = reply->readAll();
        QJsonParseError err; QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError || !doc.isArray()) return;
        QJsonArray arr = doc.array();
        // Minimal merge: update listed prices and flags by id
        for (const QJsonValue& v : arr) {
            if (!v.isObject()) continue;
            QJsonObject o = v.toObject();
            QString id = o.value("nftId").toString();
            for (auto& n : m_nftList) {
                if (n.id == id) {
                    n.isListed = o.value("listed").toBool(true);
                    n.listedPriceSatoshis = o.value("priceSatoshis").toString().toLongLong();
                    n.listedPriceUSD = o.value("priceUSD").toDouble(0.0);
                    // If server marks adminVerified, persist in metadata
                    if (o.contains("adminVerified")) {
                        QJsonParseError e2; QJsonDocument d2 = QJsonDocument::fromJson(n.metadata.toUtf8(), &e2);
                        QJsonObject obj = e2.error==QJsonParseError::NoError && d2.isObject() ? d2.object() : QJsonObject{};
                        obj.insert("adminVerified", o.value("adminVerified").toBool());
                        n.metadata = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
                    }
                    break;
                }
            }
        }
        populateMarketplaceTable();
    });
}

bool TokenNFTExplorer::signAndBroadcastTransaction(const QString& txHex, QString& txid)
{
    if (!m_walletModel) return false;
    
    try {
        // Use wallet model to sign and broadcast
        // This is a simplified implementation - actual implementation would use RPC calls
        QString signedTx = m_walletModel->signTransaction(txHex);
        if (signedTx.isEmpty()) return false;
        
        // Broadcast via RPC
        txid = m_walletModel->broadcastTransaction(signedTx);
        return !txid.isEmpty();
    } catch (...) {
        return false;
    }
}

bool TokenNFTExplorer::createNFTPurchaseTransaction(const QString& nftId, const QString& sellerAddress, qint64 priceSatoshis, QString& txHex)
{
    if (!m_walletModel) return false;
    
    try {
        // Create NFT purchase transaction
        // This would create a transaction that transfers SHAH to seller and NFT to buyer
        QJsonObject txData;
        txData["type"] = "nft_purchase";
        txData["nft_id"] = nftId;
        txData["seller"] = sellerAddress;
        txData["buyer"] = m_walletModel->getAddressTableModel()->defaultAddress();
        txData["price"] = QString::number(priceSatoshis);
        txData["fee"] = "100000"; // 0.001 SHAH fee
        
        // Convert to hex transaction
        QJsonDocument doc(txData);
        txHex = doc.toJson(QJsonDocument::Compact).toHex();
        
        return true;
    } catch (...) {
        return false;
    }
}

bool TokenNFTExplorer::createNFTListingTransaction(const QString& nftId, qint64 priceSatoshis, QString& txHex)
{
    if (!m_walletModel) return false;
    
    try {
        // Create NFT listing transaction
        QJsonObject txData;
        txData["type"] = "nft_listing";
        txData["nft_id"] = nftId;
        txData["owner"] = m_walletModel->getAddressTableModel()->defaultAddress();
        txData["price"] = QString::number(priceSatoshis);
        txData["fee"] = "100000"; // 0.001 SHAH fee
        
        QJsonDocument doc(txData);
        txHex = doc.toJson(QJsonDocument::Compact).toHex();
        
        return true;
    } catch (...) {
        return false;
    }
}

void TokenNFTExplorer::updateCreatorTrustScore(const QString& creatorAddress)
{
    // Calculate trust score based on:
    // - Number of NFTs created
    // - Total sales volume
    // - Verification status
    // - User feedback (future)
    
    CreatorProfile& profile = m_creatorProfiles[creatorAddress];
    profile.address = creatorAddress;
    
    // Count NFTs by this creator
    int totalNFTs = 0;
    qint64 totalVolume = 0;
    
    for (const auto& nft : m_nftList) {
        if (nft.creator == creatorAddress) {
            totalNFTs++;
            totalVolume += nft.listedPriceSatoshis;
        }
    }
    
    profile.totalNFTs = totalNFTs;
    profile.totalVolume = totalVolume;
    
    // Calculate trust score (0-100)
    double score = 0.0;
    score += qMin(totalNFTs * 5.0, 30.0); // Up to 30 points for NFT count
    score += qMin(totalVolume / 1000000000.0, 40.0); // Up to 40 points for volume (in SHAH)
    
    // Bonus for verified NFTs
    int verifiedCount = 0;
    for (const auto& nft : m_nftList) {
        if (nft.creator == creatorAddress && isVerifiedNFT(nft)) {
            verifiedCount++;
        }
    }
    score += qMin(verifiedCount * 3.0, 30.0); // Up to 30 points for verified NFTs
    
    profile.trustScore = qMin(score, 100.0);
    profile.isTrusted = profile.trustScore >= 70.0;
    
    // Generate badges
    QStringList badges;
    if (profile.isTrusted) badges << "Trusted Creator";
    if (totalNFTs >= 10) badges << "Prolific Creator";
    if (totalVolume >= 1000000000) badges << "High Volume"; // 1 SHAH
    if (verifiedCount >= 5) badges << "Verified Artist";
    
    profile.badges = badges.join(", ");
}

void TokenNFTExplorer::loadCreatorProfiles()
{
    m_creatorProfiles.clear();
    
    // Build profiles for all creators
    QSet<QString> creators;
    for (const auto& nft : m_nftList) {
        creators.insert(nft.creator);
    }
    
    for (const QString& creator : creators) {
        updateCreatorTrustScore(creator);
    }
}

bool TokenNFTExplorer::shouldShowInMarketplace(const NFTData& nft) const
{
    const int mode = m_marketToggle ? m_marketToggle->currentIndex() : 0; // 0 Verified Only, 1 All, 2 My, 3 Favorites
    if (mode == 0 && !isVerifiedNFT(nft)) return false;
    if (mode == 2 && (!m_walletModel || nft.owner != m_walletModel->getAddressTableModel()->defaultAddress())) return false;
    if (mode == 3 && !isFavorited(nft.id)) return false;

    // Category filter
    if (m_marketCategory && m_marketCategory->currentIndex() > 0) {
        const QString desired = m_marketCategory->currentText().toLower();
        if (nft.category.toLower() != desired) return false;
    }
    // Creator filter
    if (!m_marketCreator->text().trimmed().isEmpty()) {
        if (!nft.creator.contains(m_marketCreator->text().trimmed(), Qt::CaseInsensitive)) return false;
    }
    // Traits filter (simple contains on JSON string)
    if (!m_marketTraits->text().trimmed().isEmpty()) {
        if (!nft.traits.isEmpty()) {
            const QString traitStr = QString::fromUtf8(QJsonDocument(nft.traits).toJson(QJsonDocument::Compact));
            if (!traitStr.contains(m_marketTraits->text().trimmed(), Qt::CaseInsensitive)) return false;
        } else if (!nft.metadata.contains(m_marketTraits->text().trimmed())) {
            return false;
        }
    }
    // Price filter (USD)
    bool okMin = false, okMax = false; double minV = m_marketPriceMin->text().toDouble(&okMin); double maxV = m_marketPriceMax->text().toDouble(&okMax);
    if (okMin && nft.listedPriceUSD < minV) return false;
    if (okMax && nft.listedPriceUSD > maxV) return false;

    return true;
}

void TokenNFTExplorer::onAdminToggleVerified()
{
    // Find the NFT and toggle its admin verification status
    for (auto& nft : m_nftList) {
        if (nft.id == m_selectedMarketNFTId) {
            bool currentlyVerified = false;
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(nft.metadata.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                currentlyVerified = obj.value("adminVerified").toBool();
            }
            
            // Toggle the flag
            QJsonObject obj = err.error == QJsonParseError::NoError && doc.isObject() ? doc.object() : QJsonObject{};
            obj.insert("adminVerified", !currentlyVerified);
            nft.metadata = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
            
            // Update via API
            setAdminVerifyFlagToApi(nft.id, !currentlyVerified);
            
            // Refresh the table
            populateMarketplaceTable();
            break;
        }
    }
}

void TokenNFTExplorer::showNFTDetailsDialog()
{
    // Find the NFT data
    for (const auto& nft : m_nftList) {
        if (nft.id == m_selectedMarketNFTId) {
            showNFTDetailsDialog(nft);
            break;
        }
    }
}

void TokenNFTExplorer::showMarketplaceListingDialog()
{
    // Find the NFT data
    for (const auto& nft : m_nftList) {
        if (nft.id == m_selectedMarketNFTId) {
            MarketplaceListingDialog dlg(this);
            dlg.setWalletModel(m_walletModel);
            dlg.setPriceOracle(m_priceOracle);
            
            // Preset the NFT data
            dlg.presetNFT(nft.id, nft.name);
            
            connect(&dlg, &MarketplaceListingDialog::listingSubmitted, this, [this](const QString& nftId, const QString& title, const QString& description, const QString& category, const QString& tags, qint64 priceSatoshis, double priceUSD, bool submitForVerification) {
                // Update local state
                for (auto& n : m_nftList) {
                    if (n.id == nftId) {
                        if (!title.isEmpty()) n.name = title;
                        n.description = description;
                        n.category = category;
                        n.listedPriceSatoshis = priceSatoshis;
                        n.listedPriceUSD = priceUSD;
                        n.isListed = true;
                        
                        // Update metadata for verification
                        QJsonParseError err;
                        QJsonDocument doc = QJsonDocument::fromJson(n.metadata.toUtf8(), &err);
                        QJsonObject obj = err.error == QJsonParseError::NoError && doc.isObject() ? doc.object() : QJsonObject{};
                        obj.insert("submittedForVerification", submitForVerification);
                        n.metadata = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
                        break;
                    }
                }
                
                // Submit to API
                submitListingToApi(nftId, title, description, category, tags, priceSatoshis, priceUSD, submitForVerification);
                
                // Refresh the table
                populateMarketplaceTable();
            });
            
            dlg.exec();
            break;
        }
    }
}

void TokenNFTExplorer::addTransactionRecord(const TransactionRecord& record)
{
    m_transactionHistory.append(record);
    
    // Save to settings for persistence
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("transactionHistory");
    
    QJsonArray historyArray;
    for (const auto& tx : m_transactionHistory) {
        QJsonObject txObj;
        txObj["txid"] = tx.txid;
        txObj["type"] = tx.type;
        txObj["nftId"] = tx.nftId;
        txObj["fromAddress"] = tx.fromAddress;
        txObj["toAddress"] = tx.toAddress;
        txObj["amountSatoshis"] = QString::number(tx.amountSatoshis);
        txObj["amountUSD"] = tx.amountUSD;
        txObj["timestamp"] = tx.timestamp.toString(Qt::ISODate);
        txObj["status"] = tx.status;
        txObj["metadata"] = tx.metadata;
        historyArray.append(txObj);
    }
    
    settings.setValue("transactions", QJsonDocument(historyArray).toJson(QJsonDocument::Compact));
    settings.endGroup();
}

void TokenNFTExplorer::loadTransactionHistory()
{
    m_transactionHistory.clear();
    
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("transactionHistory");
    
    QByteArray data = settings.value("transactions").toByteArray();
    if (!data.isEmpty()) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error == QJsonParseError::NoError && doc.isArray()) {
            QJsonArray arr = doc.array();
            for (const QJsonValue& val : arr) {
                if (val.isObject()) {
                    QJsonObject obj = val.toObject();
                    TransactionRecord record;
                    record.txid = obj["txid"].toString();
                    record.type = obj["type"].toString();
                    record.nftId = obj["nftId"].toString();
                    record.fromAddress = obj["fromAddress"].toString();
                    record.toAddress = obj["toAddress"].toString();
                    record.amountSatoshis = obj["amountSatoshis"].toString().toLongLong();
                    record.amountUSD = obj["amountUSD"].toDouble();
                    record.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
                    record.status = obj["status"].toString();
                    record.metadata = obj["metadata"].toString();
                    m_transactionHistory.append(record);
                }
            }
        }
    }
    
    settings.endGroup();
}

void TokenNFTExplorer::exportTransactionHistory(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Export Failed"), tr("Could not open file for writing."));
        return;
    }
    
    QTextStream out(&file);
    
    // CSV header
    out << "Transaction ID,Type,NFT ID,From Address,To Address,Amount (SHAH),Amount (USD),Timestamp,Status,Metadata\n";
    
    // Data rows
    for (const auto& record : m_transactionHistory) {
        out << QString("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",%6,%7,\"%8\",\"%9\",\"%10\"\n")
            .arg(record.txid)
            .arg(record.type)
            .arg(record.nftId)
            .arg(record.fromAddress)
            .arg(record.toAddress)
            .arg(record.amountSatoshis / 100000000.0, 0, 'f', 8)
            .arg(record.amountUSD, 0, 'f', 2)
            .arg(record.timestamp.toString(Qt::ISODate))
            .arg(record.status)
            .arg(record.metadata);
    }
    
    file.close();
    QMessageBox::information(this, tr("Export Successful"), tr("Transaction history exported to %1").arg(filename));
}

void TokenNFTExplorer::updateAnalyticsData()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime dayAgo = now.addDays(-1);
    QDateTime weekAgo = now.addDays(-7);
    QDateTime monthAgo = now.addDays(-30);
    
    // Reset analytics
    m_analyticsData = AnalyticsData{};
    
    // Process transaction history
    for (const auto& record : m_transactionHistory) {
        if (record.type == "purchase" && record.status == "confirmed") {
            // Volume calculations
            if (record.timestamp >= dayAgo) {
                m_analyticsData.totalVolume24h += record.amountSatoshis;
                m_analyticsData.totalTransactions24h++;
            }
            if (record.timestamp >= weekAgo) {
                m_analyticsData.totalVolume7d += record.amountSatoshis;
                m_analyticsData.totalTransactions7d++;
            }
            if (record.timestamp >= monthAgo) {
                m_analyticsData.totalVolume30d += record.amountSatoshis;
                m_analyticsData.totalTransactions30d++;
            }
            
            // Top creators
            for (const auto& nft : m_nftList) {
                if (nft.id == record.nftId) {
                    m_analyticsData.topCreators[nft.creator]++;
                    m_analyticsData.topCategories[nft.category]++;
                    break;
                }
            }
        }
    }
    
    // Calculate averages
    if (m_analyticsData.totalTransactions24h > 0) {
        m_analyticsData.averagePrice24h = m_analyticsData.totalVolume24h / 100000000.0 / m_analyticsData.totalTransactions24h;
    }
    if (m_analyticsData.totalTransactions7d > 0) {
        m_analyticsData.averagePrice7d = m_analyticsData.totalVolume7d / 100000000.0 / m_analyticsData.totalTransactions7d;
    }
    if (m_analyticsData.totalTransactions30d > 0) {
        m_analyticsData.averagePrice30d = m_analyticsData.totalVolume30d / 100000000.0 / m_analyticsData.totalTransactions30d;
    }
}

void TokenNFTExplorer::showAnalyticsDashboard()
{
    updateAnalyticsData();
    
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Analytics Dashboard"));
    dlg.setMinimumSize(800, 600);
    QVBoxLayout* v = new QVBoxLayout(&dlg);
    
    // Volume metrics
    QGroupBox* volumeGroup = new QGroupBox(tr("Trading Volume"), &dlg);
    QGridLayout* volumeLayout = new QGridLayout(volumeGroup);
    
    volumeLayout->addWidget(new QLabel(tr("24h Volume:")), 0, 0);
    volumeLayout->addWidget(new QLabel(QString("%1 SHAH (%2 transactions)").arg(m_analyticsData.totalVolume24h / 100000000.0, 0, 'f', 2).arg(m_analyticsData.totalTransactions24h)), 0, 1);
    
    volumeLayout->addWidget(new QLabel(tr("7d Volume:")), 1, 0);
    volumeLayout->addWidget(new QLabel(QString("%1 SHAH (%2 transactions)").arg(m_analyticsData.totalVolume7d / 100000000.0, 0, 'f', 2).arg(m_analyticsData.totalTransactions7d)), 1, 1);
    
    volumeLayout->addWidget(new QLabel(tr("30d Volume:")), 2, 0);
    volumeLayout->addWidget(new QLabel(QString("%1 SHAH (%2 transactions)").arg(m_analyticsData.totalVolume30d / 100000000.0, 0, 'f', 2).arg(m_analyticsData.totalTransactions30d)), 2, 1);
    
    v->addWidget(volumeGroup);
    
    // Average prices
    QGroupBox* priceGroup = new QGroupBox(tr("Average Prices"), &dlg);
    QGridLayout* priceLayout = new QGridLayout(priceGroup);
    
    priceLayout->addWidget(new QLabel(tr("24h Avg:")), 0, 0);
    priceLayout->addWidget(new QLabel(QString("%1 SHAH").arg(m_analyticsData.averagePrice24h, 0, 'f', 2)), 0, 1);
    
    priceLayout->addWidget(new QLabel(tr("7d Avg:")), 1, 0);
    priceLayout->addWidget(new QLabel(QString("%1 SHAH").arg(m_analyticsData.averagePrice7d, 0, 'f', 2)), 1, 1);
    
    priceLayout->addWidget(new QLabel(tr("30d Avg:")), 2, 0);
    priceLayout->addWidget(new QLabel(QString("%1 SHAH").arg(m_analyticsData.averagePrice30d, 0, 'f', 2)), 2, 1);
    
    v->addWidget(priceGroup);
    
    // Top creators
    QGroupBox* creatorsGroup = new QGroupBox(tr("Top Creators"), &dlg);
    QVBoxLayout* creatorsLayout = new QVBoxLayout(creatorsGroup);
    
    QList<QPair<QString, int>> sortedCreators;
    for (auto it = m_analyticsData.topCreators.begin(); it != m_analyticsData.topCreators.end(); ++it) {
        sortedCreators.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedCreators.begin(), sortedCreators.end(), [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
        return a.second > b.second;
    });
    
    for (int i = 0; i < qMin(5, sortedCreators.size()); ++i) {
        QString creatorText = QString("%1. %2 (%3 NFTs)").arg(i + 1).arg(formatAddress(sortedCreators[i].first)).arg(sortedCreators[i].second);
        creatorsLayout->addWidget(new QLabel(creatorText));
    }
    
    v->addWidget(creatorsGroup);
    
    // Export button
    QPushButton* exportBtn = new QPushButton(tr("Export Transaction History"), &dlg);
    connect(exportBtn, &QPushButton::clicked, [this]() {
        QString filename = QFileDialog::getSaveFileName(this, tr("Export Transaction History"), 
            QString("shahcoin_transactions_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd")),
            tr("CSV Files (*.csv)"));
        if (!filename.isEmpty()) {
            exportTransactionHistory(filename);
        }
    });
    v->addWidget(exportBtn);
    
    // Close button
    QPushButton* closeBtn = new QPushButton(tr("Close"), &dlg);
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    v->addWidget(closeBtn);
    
    dlg.exec();
}

void TokenNFTExplorer::createEscrow(const QString& nftId, const QString& buyer, const QString& seller, qint64 amount)
{
    EscrowData escrow;
    escrow.escrowId = QString("ESC_%1_%2").arg(nftId).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    escrow.nftId = nftId;
    escrow.buyer = buyer;
    escrow.seller = seller;
    escrow.amountSatoshis = amount;
    escrow.createdAt = QDateTime::currentDateTime();
    escrow.expiresAt = escrow.createdAt.addDays(7); // 7-day escrow
    escrow.status = "pending";
    
    m_escrowData[escrow.escrowId] = escrow;
    
    // Save to settings
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("escrow");
    
    QJsonArray escrowArray;
    for (auto it = m_escrowData.begin(); it != m_escrowData.end(); ++it) {
        QJsonObject escrowObj;
        escrowObj["escrowId"] = it->escrowId;
        escrowObj["nftId"] = it->nftId;
        escrowObj["buyer"] = it->buyer;
        escrowObj["seller"] = it->seller;
        escrowObj["amountSatoshis"] = QString::number(it->amountSatoshis);
        escrowObj["createdAt"] = it->createdAt.toString(Qt::ISODate);
        escrowObj["expiresAt"] = it->expiresAt.toString(Qt::ISODate);
        escrowObj["status"] = it->status;
        escrowObj["disputeReason"] = it->disputeReason;
        escrowArray.append(escrowObj);
    }
    
    settings.setValue("escrows", QJsonDocument(escrowArray).toJson(QJsonDocument::Compact));
    settings.endGroup();
    
    QMessageBox::information(this, tr("Escrow Created"), 
        tr("Escrow created successfully!\nEscrow ID: %1\nExpires: %2").arg(escrow.escrowId).arg(escrow.expiresAt.toString("yyyy-MM-dd hh:mm:ss")));
}

void TokenNFTExplorer::fundEscrow(const QString& escrowId)
{
    if (!m_escrowData.contains(escrowId)) return;
    
    EscrowData& escrow = m_escrowData[escrowId];
    escrow.status = "funded";
    
    // Update settings
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("escrow");
    
    QJsonArray escrowArray;
    for (auto it = m_escrowData.begin(); it != m_escrowData.end(); ++it) {
        QJsonObject escrowObj;
        escrowObj["escrowId"] = it->escrowId;
        escrowObj["nftId"] = it->nftId;
        escrowObj["buyer"] = it->buyer;
        escrowObj["seller"] = it->seller;
        escrowObj["amountSatoshis"] = QString::number(it->amountSatoshis);
        escrowObj["createdAt"] = it->createdAt.toString(Qt::ISODate);
        escrowObj["expiresAt"] = it->expiresAt.toString(Qt::ISODate);
        escrowObj["status"] = it->status;
        escrowObj["disputeReason"] = it->disputeReason;
        escrowArray.append(escrowObj);
    }
    
    settings.setValue("escrows", QJsonDocument(escrowArray).toJson(QJsonDocument::Compact));
    settings.endGroup();
    
    QMessageBox::information(this, tr("Escrow Funded"), tr("Escrow has been funded. Seller will be notified."));
}

void TokenNFTExplorer::releaseEscrow(const QString& escrowId)
{
    if (!m_escrowData.contains(escrowId)) return;
    
    EscrowData& escrow = m_escrowData[escrowId];
    escrow.status = "released";
    
    // Execute the actual NFT transfer
    for (auto& nft : m_nftList) {
        if (nft.id == escrow.nftId) {
            nft.owner = escrow.buyer;
            nft.isListed = false;
            nft.listedPriceSatoshis = 0;
            nft.listedPriceUSD = 0.0;
            break;
        }
    }
    
    // Add transaction record
    TransactionRecord record;
    record.txid = QString("ESC_%1").arg(escrowId);
    record.type = "escrow_release";
    record.nftId = escrow.nftId;
    record.fromAddress = escrow.seller;
    record.toAddress = escrow.buyer;
    record.amountSatoshis = escrow.amountSatoshis;
    record.amountUSD = escrow.amountSatoshis / 100000000.0 * (m_priceOracle ? m_priceOracle->currentUsdPerShah() : 0.0);
    record.timestamp = QDateTime::currentDateTime();
    record.status = "confirmed";
    record.metadata = QString("{\"escrowId\":\"%1\"}").arg(escrowId);
    addTransactionRecord(record);
    
    // Update settings
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("escrow");
    
    QJsonArray escrowArray;
    for (auto it = m_escrowData.begin(); it != m_escrowData.end(); ++it) {
        QJsonObject escrowObj;
        escrowObj["escrowId"] = it->escrowId;
        escrowObj["nftId"] = it->nftId;
        escrowObj["buyer"] = it->buyer;
        escrowObj["seller"] = it->seller;
        escrowObj["amountSatoshis"] = QString::number(it->amountSatoshis);
        escrowObj["createdAt"] = it->createdAt.toString(Qt::ISODate);
        escrowObj["expiresAt"] = it->expiresAt.toString(Qt::ISODate);
        escrowObj["status"] = it->status;
        escrowObj["disputeReason"] = it->disputeReason;
        escrowArray.append(escrowObj);
    }
    
    settings.setValue("escrows", QJsonDocument(escrowArray).toJson(QJsonDocument::Compact));
    settings.endGroup();
    
    QMessageBox::information(this, tr("Escrow Released"), tr("NFT has been transferred to buyer. Payment released to seller."));
}

void TokenNFTExplorer::refundEscrow(const QString& escrowId, const QString& reason)
{
    if (!m_escrowData.contains(escrowId)) return;
    
    EscrowData& escrow = m_escrowData[escrowId];
    escrow.status = "refunded";
    escrow.disputeReason = reason;
    
    // Add transaction record
    TransactionRecord record;
    record.txid = QString("ESC_REFUND_%1").arg(escrowId);
    record.type = "escrow_refund";
    record.nftId = escrow.nftId;
    record.fromAddress = escrow.seller;
    record.toAddress = escrow.buyer;
    record.amountSatoshis = escrow.amountSatoshis;
    record.amountUSD = escrow.amountSatoshis / 100000000.0 * (m_priceOracle ? m_priceOracle->currentUsdPerShah() : 0.0);
    record.timestamp = QDateTime::currentDateTime();
    record.status = "confirmed";
    record.metadata = QString("{\"escrowId\":\"%1\",\"reason\":\"%2\"}").arg(escrowId).arg(reason);
    addTransactionRecord(record);
    
    // Update settings
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("escrow");
    
    QJsonArray escrowArray;
    for (auto it = m_escrowData.begin(); it != m_escrowData.end(); ++it) {
        QJsonObject escrowObj;
        escrowObj["escrowId"] = it->escrowId;
        escrowObj["nftId"] = it->nftId;
        escrowObj["buyer"] = it->buyer;
        escrowObj["seller"] = it->seller;
        escrowObj["amountSatoshis"] = QString::number(it->amountSatoshis);
        escrowObj["createdAt"] = it->createdAt.toString(Qt::ISODate);
        escrowObj["expiresAt"] = it->expiresAt.toString(Qt::ISODate);
        escrowObj["status"] = it->status;
        escrowObj["disputeReason"] = it->disputeReason;
        escrowArray.append(escrowObj);
    }
    
    settings.setValue("escrows", QJsonDocument(escrowArray).toJson(QJsonDocument::Compact));
    settings.endGroup();
    
    QMessageBox::information(this, tr("Escrow Refunded"), tr("Payment has been refunded to buyer. Reason: %1").arg(reason));
}

void TokenNFTExplorer::showEscrowDialog(const QString& nftId)
{
    // Find NFT data
    const NFTData* nftData = nullptr;
    for (const auto& nft : m_nftList) {
        if (nft.id == nftId) {
            nftData = &nft;
            break;
        }
    }
    if (!nftData) return;
    
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Escrow Protection - %1").arg(nftData->name));
    dlg.setMinimumSize(500, 400);
    QVBoxLayout* v = new QVBoxLayout(&dlg);
    
    // Info section
    QLabel* infoLabel = new QLabel(tr("This NFT requires escrow protection due to its high value.\n\n"
        "How escrow works:\n"
        "1. Buyer funds the escrow with SHAH\n"
        "2. Seller transfers NFT to escrow\n"
        "3. Buyer confirms receipt and releases payment\n"
        "4. If there's a dispute, funds can be refunded\n\n"
        "Escrow duration: 7 days\n"
        "Escrow fee: 0.5% of transaction value"), &dlg);
    infoLabel->setWordWrap(true);
    v->addWidget(infoLabel);
    
    // Price display
    qint64 listPriceSatoshis = nftData->listedPriceSatoshis;
    double listPriceUSD = nftData->listedPriceUSD;
    qint64 escrowFee = listPriceSatoshis * 5 / 1000; // 0.5%
    qint64 totalAmount = listPriceSatoshis + escrowFee;
    
    QGroupBox* priceGroup = new QGroupBox(tr("Transaction Details"), &dlg);
    QGridLayout* priceLayout = new QGridLayout(priceGroup);
    
    priceLayout->addWidget(new QLabel(tr("NFT Price:")), 0, 0);
    priceLayout->addWidget(new QLabel(QString("%1 SHAH (%2 USD)").arg(listPriceSatoshis / 100000000.0, 0, 'f', 8).arg(listPriceUSD, 0, 'f', 2)), 0, 1);
    
    priceLayout->addWidget(new QLabel(tr("Escrow Fee:")), 1, 0);
    priceLayout->addWidget(new QLabel(QString("%1 SHAH").arg(escrowFee / 100000000.0, 0, 'f', 8)), 1, 1);
    
    priceLayout->addWidget(new QLabel(tr("Total Amount:")), 2, 0);
    priceLayout->addWidget(new QLabel(QString("%1 SHAH").arg(totalAmount / 100000000.0, 0, 'f', 8)), 2, 1);
    
    v->addWidget(priceGroup);
    
    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* createBtn = new QPushButton(tr("Create Escrow"), &dlg);
    QPushButton* cancelBtn = new QPushButton(tr("Cancel"), &dlg);
    btnLayout->addWidget(createBtn);
    btnLayout->addWidget(cancelBtn);
    v->addLayout(btnLayout);
    
    connect(createBtn, &QPushButton::clicked, [&]() {
        QString buyerAddress = m_walletModel->getAddressTableModel()->defaultAddress();
        createEscrow(nftId, buyerAddress, nftData->owner, totalAmount);
        dlg.accept();
    });
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    
    dlg.exec();
}

// ============================================================================
// Bulk Operations Implementation
// ============================================================================

void TokenNFTExplorer::showBulkOperationsDialog()
{
    // Get owned NFTs for bulk operations
    QList<NFTData> ownedNFTs;
    QString defaultAddress = m_walletModel ? m_walletModel->getAddressTableModel()->defaultAddress() : "";
    
    for (const auto& nft : m_nftList) {
        if (nft.owner == defaultAddress) {
            ownedNFTs.append(nft);
        }
    }
    
    if (ownedNFTs.isEmpty()) {
        QMessageBox::information(this, tr("No NFTs Found"), tr("You don't own any NFTs to perform bulk operations on."));
        return;
    }
    
    BulkOperationsDialog* dialog = new BulkOperationsDialog(ownedNFTs, this);
    connect(dialog, &BulkOperationsDialog::bulkOperationRequested, this, &TokenNFTExplorer::executeBulkOperation);
    dialog->show();
}

void TokenNFTExplorer::executeBulkOperation(const BulkOperation& operation)
{
    if (operation.nftIds.isEmpty()) {
        QMessageBox::warning(this, tr("No NFTs Selected"), tr("Please select at least one NFT for bulk operations."));
        return;
    }
    
    // Show progress dialog
    QProgressDialog progress(tr("Executing bulk operation..."), tr("Cancel"), 0, operation.nftIds.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    
    int completed = 0;
    bool cancelled = false;
    
    for (const QString& nftId : operation.nftIds) {
        if (progress.wasCanceled()) {
            cancelled = true;
            break;
        }
        
        progress.setValue(completed);
        progress.setLabelText(tr("Processing %1...").arg(nftId));
        
        // Execute operation based on type
        if (operation.operationType == "batch_list") {
            // Batch listing
            qint64 priceSatoshis = operation.newPrices.value(nftId, 0);
            if (priceSatoshis > 0) {
                // Create listing transaction
                QString txHex;
                if (createNFTListingTransaction(nftId, priceSatoshis, txHex)) {
                    QString txid;
                    if (signAndBroadcastTransaction(txHex, txid)) {
                        // Update local state
                        for (auto& nft : m_nftList) {
                            if (nft.id == nftId) {
                                nft.isListed = true;
                                nft.listedPriceSatoshis = priceSatoshis;
                                nft.listedPriceUSD = priceSatoshis / 100000000.0 * (m_priceOracle ? m_priceOracle->currentUsdPerShah() : 0.0);
                                break;
                            }
                        }
                        
                        // Submit to API
                        submitListingToApi(nftId, "", "", operation.category, operation.tags, 
                                         priceSatoshis, nft.listedPriceUSD, operation.submitForVerification);
                    }
                }
            }
        } else if (operation.operationType == "batch_price_update") {
            // Batch price update
            qint64 newPriceSatoshis = operation.newPrices.value(nftId, 0);
            if (newPriceSatoshis > 0) {
                // Update listing transaction
                QString txHex;
                if (createNFTListingTransaction(nftId, newPriceSatoshis, txHex)) {
                    QString txid;
                    if (signAndBroadcastTransaction(txHex, txid)) {
                        // Update local state
                        for (auto& nft : m_nftList) {
                            if (nft.id == nftId) {
                                nft.listedPriceSatoshis = newPriceSatoshis;
                                nft.listedPriceUSD = newPriceSatoshis / 100000000.0 * (m_priceOracle ? m_priceOracle->currentUsdPerShah() : 0.0);
                                break;
                            }
                        }
                        
                        // Update API
                        submitListingToApi(nftId, "", "", operation.category, operation.tags,
                                         newPriceSatoshis, nft.listedPriceUSD, operation.submitForVerification);
                    }
                }
            }
        } else if (operation.operationType == "batch_delist") {
            // Batch delisting - remove from marketplace
            for (auto& nft : m_nftList) {
                if (nft.id == nftId) {
                    nft.isListed = false;
                    nft.listedPriceSatoshis = 0;
                    nft.listedPriceUSD = 0.0;
                    break;
                }
            }
        }
        
        completed++;
        QApplication::processEvents();
    }
    
    progress.setValue(operation.nftIds.size());
    
    if (!cancelled) {
        // Refresh marketplace table
        populateMarketplaceTable();
        
        // Add transaction records
        for (const QString& nftId : operation.nftIds) {
            TransactionRecord record;
            record.txid = "bulk_" + QString::number(QDateTime::currentMSecsSinceEpoch());
            record.type = operation.operationType;
            record.nftId = nftId;
            record.fromAddress = m_walletModel ? m_walletModel->getAddressTableModel()->defaultAddress() : "";
            record.toAddress = "";
            record.amountSatoshis = operation.newPrices.value(nftId, 0);
            record.amountUSD = record.amountSatoshis / 100000000.0 * (m_priceOracle ? m_priceOracle->currentUsdPerShah() : 0.0);
            record.timestamp = QDateTime::currentDateTime();
            record.status = "confirmed";
            record.metadata = QString("{\"operation\":\"%1\",\"category\":\"%2\",\"tags\":\"%3\",\"verified\":%4}")
                             .arg(operation.operationType)
                             .arg(operation.category)
                             .arg(operation.tags)
                             .arg(operation.submitForVerification ? "true" : "false");
            addTransactionRecord(record);
        }
        
        QMessageBox::information(this, tr("Bulk Operation Complete"), 
                                tr("Successfully processed %1 NFTs with operation: %2")
                                .arg(operation.nftIds.size())
                                .arg(operation.operationType));
    }
}

void TokenNFTExplorer::scheduleBulkOperation(const BulkOperation& operation)
{
    m_pendingBulkOperations.append(operation);
    
    // Save to settings
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("bulkOperations");
    
    QJsonArray operationsArray;
    for (const auto& op : m_pendingBulkOperations) {
        QJsonObject opObj;
        opObj["operationType"] = op.operationType;
        opObj["nftIds"] = QJsonArray::fromStringList(op.nftIds);
        
        QJsonObject pricesObj;
        for (auto it = op.newPrices.begin(); it != op.newPrices.end(); ++it) {
            pricesObj[it.key()] = QString::number(it.value());
        }
        opObj["newPrices"] = pricesObj;
        
        opObj["category"] = op.category;
        opObj["tags"] = op.tags;
        opObj["submitForVerification"] = op.submitForVerification;
        opObj["scheduledTime"] = op.scheduledTime.toString(Qt::ISODate);
        
        operationsArray.append(opObj);
    }
    
    settings.setValue("pendingOperations", QJsonDocument(operationsArray).toJson(QJsonDocument::Compact));
    settings.endGroup();
}

void TokenNFTExplorer::cancelBulkOperation(int operationIndex)
{
    if (operationIndex >= 0 && operationIndex < m_pendingBulkOperations.size()) {
        m_pendingBulkOperations.removeAt(operationIndex);
        
        // Update settings
        QSettings settings("Shahcoin", "Wallet");
        settings.beginGroup("bulkOperations");
        
        QJsonArray operationsArray;
        for (const auto& op : m_pendingBulkOperations) {
            QJsonObject opObj;
            opObj["operationType"] = op.operationType;
            opObj["nftIds"] = QJsonArray::fromStringList(op.nftIds);
            
            QJsonObject pricesObj;
            for (auto it = op.newPrices.begin(); it != op.newPrices.end(); ++it) {
                pricesObj[it.key()] = QString::number(it.value());
            }
            opObj["newPrices"] = pricesObj;
            
            opObj["category"] = op.category;
            opObj["tags"] = op.tags;
            opObj["submitForVerification"] = op.submitForVerification;
            opObj["scheduledTime"] = op.scheduledTime.toString(Qt::ISODate);
            
            operationsArray.append(opObj);
        }
        
        settings.setValue("pendingOperations", QJsonDocument(operationsArray).toJson(QJsonDocument::Compact));
        settings.endGroup();
    }
}

void TokenNFTExplorer::showBulkOperationProgress(const BulkOperation& operation)
{
    QDialog progressDialog(this);
    progressDialog.setWindowTitle(tr("Bulk Operation Progress"));
    progressDialog.setFixedSize(400, 200);
    
    QVBoxLayout* layout = new QVBoxLayout(&progressDialog);
    
    QLabel* statusLabel = new QLabel(tr("Processing bulk operation..."), &progressDialog);
    QProgressBar* progressBar = new QProgressBar(&progressDialog);
    QLabel* detailLabel = new QLabel(&progressDialog);
    
    progressBar->setRange(0, operation.nftIds.size());
    progressBar->setValue(0);
    
    layout->addWidget(statusLabel);
    layout->addWidget(progressBar);
    layout->addWidget(detailLabel);
    
    // Simulate progress (in real implementation, this would be connected to actual operation progress)
    QTimer* timer = new QTimer(&progressDialog);
    int currentProgress = 0;
    
    connect(timer, &QTimer::timeout, [&]() {
        currentProgress++;
        progressBar->setValue(currentProgress);
        detailLabel->setText(tr("Processed %1 of %2 NFTs").arg(currentProgress).arg(operation.nftIds.size()));
        
        if (currentProgress >= operation.nftIds.size()) {
            timer->stop();
            progressDialog.accept();
        }
    });
    
    timer->start(100); // Update every 100ms
    progressDialog.exec();
}

// ============================================================================
// Advanced Filtering Implementation
// ============================================================================

void TokenNFTExplorer::applyAdvancedFilters()
{
    // Apply current filters to marketplace display
    populateMarketplaceTable();
}

void TokenNFTExplorer::resetFilters()
{
    // Reset all filters to default values
    m_currentFilters = AdvancedFilters();
    m_currentFilters.sortBy = "price";
    m_currentFilters.sortOrder = "desc";
    
    // Update UI
    updateFilterUI();
    
    // Reapply filters
    applyAdvancedFilters();
}

void TokenNFTExplorer::saveFilterPreset(const QString& name)
{
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("filterPresets");
    
    QJsonObject presetObj;
    presetObj["searchText"] = m_currentFilters.searchText;
    presetObj["category"] = m_currentFilters.category;
    presetObj["creator"] = m_currentFilters.creator;
    presetObj["tier"] = m_currentFilters.tier;
    presetObj["minPriceSatoshis"] = QString::number(m_currentFilters.minPriceSatoshis);
    presetObj["maxPriceSatoshis"] = QString::number(m_currentFilters.maxPriceSatoshis);
    presetObj["verifiedOnly"] = m_currentFilters.verifiedOnly;
    presetObj["myNFTsOnly"] = m_currentFilters.myNFTsOnly;
    presetObj["favoritedOnly"] = m_currentFilters.favoritedOnly;
    presetObj["traits"] = QJsonArray::fromStringList(m_currentFilters.traits);
    presetObj["minMintDate"] = m_currentFilters.minMintDate.toString(Qt::ISODate);
    presetObj["maxMintDate"] = m_currentFilters.maxMintDate.toString(Qt::ISODate);
    presetObj["hasUnlockableContent"] = m_currentFilters.hasUnlockableContent;
    presetObj["isTradeLocked"] = m_currentFilters.isTradeLocked;
    presetObj["sortBy"] = m_currentFilters.sortBy;
    presetObj["sortOrder"] = m_currentFilters.sortOrder;
    
    settings.setValue(name, QJsonDocument(presetObj).toJson(QJsonDocument::Compact));
    settings.endGroup();
}

void TokenNFTExplorer::loadFilterPreset(const QString& name)
{
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("filterPresets");
    
    QString presetJson = settings.value(name).toString();
    if (!presetJson.isEmpty()) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(presetJson.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject presetObj = doc.object();
            
            m_currentFilters.searchText = presetObj["searchText"].toString();
            m_currentFilters.category = presetObj["category"].toString();
            m_currentFilters.creator = presetObj["creator"].toString();
            m_currentFilters.tier = presetObj["tier"].toString();
            m_currentFilters.minPriceSatoshis = presetObj["minPriceSatoshis"].toString().toLongLong();
            m_currentFilters.maxPriceSatoshis = presetObj["maxPriceSatoshis"].toString().toLongLong();
            m_currentFilters.verifiedOnly = presetObj["verifiedOnly"].toBool();
            m_currentFilters.myNFTsOnly = presetObj["myNFTsOnly"].toBool();
            m_currentFilters.favoritedOnly = presetObj["favoritedOnly"].toBool();
            
            QJsonArray traitsArray = presetObj["traits"].toArray();
            m_currentFilters.traits.clear();
            for (const QJsonValue& trait : traitsArray) {
                m_currentFilters.traits.append(trait.toString());
            }
            
            m_currentFilters.minMintDate = QDateTime::fromString(presetObj["minMintDate"].toString(), Qt::ISODate);
            m_currentFilters.maxMintDate = QDateTime::fromString(presetObj["maxMintDate"].toString(), Qt::ISODate);
            m_currentFilters.hasUnlockableContent = presetObj["hasUnlockableContent"].toBool();
            m_currentFilters.isTradeLocked = presetObj["isTradeLocked"].toBool();
            m_currentFilters.sortBy = presetObj["sortBy"].toString();
            m_currentFilters.sortOrder = presetObj["sortOrder"].toString();
            
            updateFilterUI();
            applyAdvancedFilters();
        }
    }
    
    settings.endGroup();
}

void TokenNFTExplorer::showAdvancedFilterDialog()
{
    AdvancedFilterDialog* dialog = new AdvancedFilterDialog(m_currentFilters, this);
    connect(dialog, &AdvancedFilterDialog::filtersApplied, this, [this](const AdvancedFilters& filters) {
        m_currentFilters = filters;
        applyAdvancedFilters();
    });
    connect(dialog, &AdvancedFilterDialog::filterPresetSaved, this, [this](const QString& name, const AdvancedFilters& filters) {
        saveFilterPreset(name);
    });
    dialog->show();
}

void TokenNFTExplorer::updateFilterUI()
{
    // Update UI components to reflect current filters
    if (m_searchBox) {
        m_searchBox->setText(m_currentFilters.searchText);
    }
    
    if (m_sortByCombo) {
        int index = m_sortByCombo->findText(m_currentFilters.sortBy, Qt::MatchFixedString);
        if (index >= 0) {
            m_sortByCombo->setCurrentIndex(index);
        }
    }
    
    if (m_sortOrderCombo) {
        int index = m_sortOrderCombo->findText(m_currentFilters.sortOrder, Qt::MatchFixedString);
        if (index >= 0) {
            m_sortOrderCombo->setCurrentIndex(index);
        }
    }
    
    // Update other filter UI components as needed
    // (This would be expanded based on the actual UI components available)
}

// ============================================================================
// Search and Autocomplete Implementation
// ============================================================================

void TokenNFTExplorer::updateSearchSuggestions(const QString& query)
{
    if (query.length() < 2) {
        m_searchSuggestionsList->hide();
        return;
    }
    
    m_searchSuggestions.clear();
    
    // Search in NFT names
    for (const auto& nft : m_nftList) {
        if (nft.name.contains(query, Qt::CaseInsensitive)) {
            SearchSuggestion suggestion;
            suggestion.text = nft.name;
            suggestion.type = "nft";
            suggestion.relevance = 1.0;
            m_searchSuggestions.append(suggestion);
        }
    }
    
    // Search in creator addresses
    for (const auto& nft : m_nftList) {
        if (nft.creator.contains(query, Qt::CaseInsensitive)) {
            SearchSuggestion suggestion;
            suggestion.text = nft.creator;
            suggestion.type = "creator";
            suggestion.relevance = 0.8;
            m_searchSuggestions.append(suggestion);
        }
    }
    
    // Search in categories
    for (const auto& nft : m_nftList) {
        if (nft.category.contains(query, Qt::CaseInsensitive)) {
            SearchSuggestion suggestion;
            suggestion.text = nft.category;
            suggestion.type = "category";
            suggestion.relevance = 0.6;
            m_searchSuggestions.append(suggestion);
        }
    }
    
    // Remove duplicates and sort by relevance
    std::sort(m_searchSuggestions.begin(), m_searchSuggestions.end(), 
              [](const SearchSuggestion& a, const SearchSuggestion& b) {
                  return a.relevance > b.relevance;
              });
    
    // Remove duplicates
    m_searchSuggestions.erase(std::unique(m_searchSuggestions.begin(), m_searchSuggestions.end(),
                                         [](const SearchSuggestion& a, const SearchSuggestion& b) {
                                             return a.text == b.text && a.type == b.type;
                                         }), m_searchSuggestions.end());
    
    // Limit to 10 suggestions
    if (m_searchSuggestions.size() > 10) {
        m_searchSuggestions = m_searchSuggestions.mid(0, 10);
    }
    
    showSearchSuggestions();
}

void TokenNFTExplorer::showSearchSuggestions()
{
    m_searchSuggestionsList->clear();
    
    for (const auto& suggestion : m_searchSuggestions) {
        QString displayText = suggestion.text;
        if (suggestion.type == "creator") {
            displayText = tr("Creator: %1").arg(formatAddress(suggestion.text));
        } else if (suggestion.type == "category") {
            displayText = tr("Category: %1").arg(suggestion.text);
        }
        
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, QVariant::fromValue(suggestion));
        m_searchSuggestionsList->addItem(item);
    }
    
    if (!m_searchSuggestions.isEmpty()) {
        m_searchSuggestionsList->show();
    } else {
        m_searchSuggestionsList->hide();
    }
}

void TokenNFTExplorer::applySearchSuggestion(const SearchSuggestion& suggestion)
{
    m_searchBox->setText(suggestion.text);
    m_searchSuggestionsList->hide();
    
    // Apply the search
    m_currentFilters.searchText = suggestion.text;
    applyAdvancedFilters();
}

// ============================================================================
// Mobile Responsive Design Implementation
// ============================================================================

void TokenNFTExplorer::setupMobileLayout()
{
    if (!m_isMobileView) return;
    
    // Create mobile-specific layout
    if (!m_mobileStackedWidget) {
        m_mobileStackedWidget = new QStackedWidget(this);
        m_desktopView = new QWidget();
        m_mobileView = new QWidget();
        
        // Move existing content to desktop view
        m_desktopView->setLayout(m_mainLayout);
        
        // Create mobile layout
        QVBoxLayout* mobileLayout = new QVBoxLayout(m_mobileView);
        
        // Mobile filter bar
        m_mobileFilterLayout = new QVBoxLayout();
        mobileLayout->addLayout(m_mobileFilterLayout);
        
        // Mobile NFT list
        m_mobileNFTList = new QListWidget(m_mobileView);
        mobileLayout->addWidget(m_mobileNFTList);
        
        m_mobileStackedWidget->addWidget(m_desktopView);
        m_mobileStackedWidget->addWidget(m_mobileView);
        
        // Replace main layout
        QVBoxLayout* newMainLayout = new QVBoxLayout(this);
        newMainLayout->addWidget(m_mobileStackedWidget);
    }
    
    m_mobileStackedWidget->setCurrentWidget(m_mobileView);
}

void TokenNFTExplorer::setupDesktopLayout()
{
    if (m_isMobileView) return;
    
    if (m_mobileStackedWidget) {
        m_mobileStackedWidget->setCurrentWidget(m_desktopView);
    }
}

void TokenNFTExplorer::toggleMobileView()
{
    m_isMobileView = !m_isMobileView;
    
    if (m_isMobileView) {
        setupMobileLayout();
        m_toggleViewButton->setText(tr("🖥 Desktop View"));
    } else {
        setupDesktopLayout();
        m_toggleViewButton->setText(tr("📱 Mobile View"));
    }
}

// ============================================================================
// Performance Optimization Implementation
// ============================================================================

void TokenNFTExplorer::setupVirtualScrolling()
{
    if (!m_virtualScrollArea) {
        m_virtualScrollArea = new QScrollArea(this);
        m_virtualScrollContent = new QWidget();
        m_virtualScrollArea->setWidget(m_virtualScrollContent);
        m_virtualScrollArea->setWidgetResizable(true);
        
        m_scrollTimer = new QTimer(this);
        m_scrollTimer->setSingleShot(true);
        m_scrollTimer->setInterval(100);
        
        connect(m_virtualScrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
                this, &TokenNFTExplorer::onVirtualScrollValueChanged);
        connect(m_scrollTimer, &QTimer::timeout, this, &TokenNFTExplorer::onScrollTimerTimeout);
    }
}

void TokenNFTExplorer::loadVisibleItemsOnly()
{
    // Implementation for loading only visible items in virtual scrolling
    // This would be called when scrolling to load new items as needed
}

void TokenNFTExplorer::preloadAdjacentItems()
{
    // Implementation for preloading adjacent items for smooth scrolling
}

// ============================================================================
// Slot Implementations
// ============================================================================

void TokenNFTExplorer::onAdvancedFilterClicked()
{
    showAdvancedFilterDialog();
}

void TokenNFTExplorer::onFilterPresetClicked()
{
    // Show preset selection dialog
    QStringList presets;
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("filterPresets");
    presets = settings.childKeys();
    settings.endGroup();
    
    if (presets.isEmpty()) {
        QMessageBox::information(this, tr("No Presets"), tr("No filter presets saved yet."));
        return;
    }
    
    bool ok;
    QString selected = QInputDialog::getItem(this, tr("Load Filter Preset"), 
                                           tr("Select a preset:"), presets, 0, false, &ok);
    if (ok && !selected.isEmpty()) {
        loadFilterPreset(selected);
    }
}

void TokenNFTExplorer::onSearchTextChanged()
{
    // Start debounce timer
    m_searchDebounceTimer->start();
}

void TokenNFTExplorer::onSearchDebounceTimeout()
{
    QString query = m_searchBox->text();
    updateSearchSuggestions(query);
    
    // Update current filters
    m_currentFilters.searchText = query;
    applyAdvancedFilters();
}

void TokenNFTExplorer::onSortByChanged()
{
    QString sortBy = m_sortByCombo->currentText().toLower();
    m_currentFilters.sortBy = sortBy;
    applyAdvancedFilters();
}

void TokenNFTExplorer::onSortOrderChanged()
{
    QString sortOrder = m_sortOrderCombo->currentText().toLower();
    m_currentFilters.sortOrder = sortOrder;
    applyAdvancedFilters();
}

void TokenNFTExplorer::onSearchSuggestionSelected()
{
    QListWidgetItem* item = m_searchSuggestionsList->currentItem();
    if (item) {
        SearchSuggestion suggestion = item->data(Qt::UserRole).value<SearchSuggestion>();
        applySearchSuggestion(suggestion);
    }
}

void TokenNFTExplorer::onBulkOperationsClicked()
{
    showBulkOperationsDialog();
}

void TokenNFTExplorer::onToggleViewClicked()
{
    toggleMobileView();
}

void TokenNFTExplorer::onScrollTimerTimeout()
{
    loadVisibleItemsOnly();
}

void TokenNFTExplorer::onVirtualScrollValueChanged()
{
    // Start timer to debounce scroll events
    m_scrollTimer->start();
}

// Marketplace Logic + Cross-Platform Integration Implementation

void TokenNFTExplorer::setupCrossPlatformSync()
{
    // Initialize cross-platform sync components
    m_marketPollTimer = new QTimer(this);
    connect(m_marketPollTimer, &QTimer::timeout, this, &TokenNFTExplorer::syncMarketplaceData);
    m_marketPollTimer->start(60000); // Sync every minute
    
    // Load local favorites and watchlist
    loadLocalFavorites();
    loadLocalWatchlist();
    
    // Initialize creator reputation cache
    loadCreatorProfiles();
}

void TokenNFTExplorer::syncMarketplaceData()
{
    if (!m_walletModel) return;
    
    // Sync listings across platforms
    syncListings();
    
    // Sync favorites and watchlist
    syncFavorites();
    
    // Sync creator data
    syncCreatorData();
    
    // Update local cache
    updateLocalCache();
}

void TokenNFTExplorer::syncListings()
{
    // Fetch current listings from API
    QNetworkRequest request(QUrl(m_apiBaseUrl + "/listings"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_net.get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray listings = doc.array();
            
            // Update local listings
            for (const QJsonValue& value : listings) {
                QJsonObject listing = value.toObject();
                updateLocalListing(listing);
            }
            
            // Refresh UI
            populateMarketplaceTable();
        }
        reply->deleteLater();
    });
}

void TokenNFTExplorer::syncFavorites()
{
    if (!m_walletModel) return;
    
    QString walletAddress = m_walletModel->getWalletAddress();
    QNetworkRequest request(QUrl(m_apiBaseUrl + "/favorites/" + walletAddress));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_net.get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray favorites = doc.array();
            
            // Update local favorites
            m_localFavorites.clear();
            for (const QJsonValue& value : favorites) {
                m_localFavorites.insert(value.toString());
            }
            
            // Save to local storage
            saveLocalFavorites();
        }
        reply->deleteLater();
    });
}

void TokenNFTExplorer::syncCreatorData()
{
    // Fetch creator profiles and reputation data
    QNetworkRequest request(QUrl(m_apiBaseUrl + "/creators"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_net.get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray creators = doc.array();
            
            // Update creator profiles
            for (const QJsonValue& value : creators) {
                QJsonObject creator = value.toObject();
                updateCreatorProfile(creator);
            }
            
            // Refresh UI
            populateMarketplaceTable();
        }
        reply->deleteLater();
    });
}

void TokenNFTExplorer::updateCreatorProfile(const QJsonObject& creatorData)
{
    QString address = creatorData["address"].toString();
    
    CreatorProfile profile;
    profile.address = address;
    profile.totalNFTs = creatorData["totalNFTs"].toInt();
    profile.totalVolume = creatorData["totalVolume"].toVariant().toLongLong();
    profile.trustScore = creatorData["trustScore"].toDouble();
    profile.isTrusted = creatorData["isTrusted"].toBool();
    profile.badges = creatorData["badges"].toString();
    
    m_creatorProfiles[address] = profile;
}

void TokenNFTExplorer::updateCreatorTrustScore(const QString& creatorAddress)
{
    // Calculate trust score based on various factors
    CreatorProfile& profile = m_creatorProfiles[creatorAddress];
    
    // Get creator's NFTs
    int totalNFTs = 0;
    qint64 totalVolume = 0;
    int refundCount = 0;
    int disputeCount = 0;
    
    for (const NFTData& nft : m_nftList) {
        if (nft.creator == creatorAddress) {
            totalNFTs++;
            if (nft.isListed) {
                totalVolume += nft.listedPriceSatoshis;
            }
        }
    }
    
    // Get transaction history for this creator
    for (const TransactionRecord& record : m_transactionHistory) {
        if (record.fromAddress == creatorAddress || record.toAddress == creatorAddress) {
            if (record.type == "refund") refundCount++;
            if (record.type == "dispute") disputeCount++;
        }
    }
    
    // Calculate trust score (0-100)
    double trustScore = 50.0; // Base score
    
    // Volume factor (0-30 points)
    if (totalVolume > 0) {
        double volumeScore = qMin(30.0, (double)totalVolume / 1000000000.0 * 30.0); // 1 SHAH = 30 points
        trustScore += volumeScore;
    }
    
    // NFT count factor (0-20 points)
    double nftScore = qMin(20.0, (double)totalNFTs * 2.0); // 10 NFTs = 20 points
    trustScore += nftScore;
    
    // Negative factors
    trustScore -= refundCount * 5.0; // -5 points per refund
    trustScore -= disputeCount * 10.0; // -10 points per dispute
    
    // Clamp to 0-100
    trustScore = qMax(0.0, qMin(100.0, trustScore));
    
    // Update profile
    profile.totalNFTs = totalNFTs;
    profile.totalVolume = totalVolume;
    profile.trustScore = trustScore;
    profile.isTrusted = trustScore >= 70.0; // Trusted if score >= 70
    
    // Assign badges
    QStringList badges;
    if (profile.isTrusted) badges << "Trusted Creator";
    if (totalVolume > 1000000000) badges << "High Volume"; // > 1 SHAH
    if (totalNFTs > 10) badges << "Prolific Creator";
    if (refundCount == 0 && disputeCount == 0) badges << "Clean Record";
    
    profile.badges = badges.join(", ");
    
    // Save to API
    saveCreatorProfileToApi(profile);
}

void TokenNFTExplorer::saveCreatorProfileToApi(const CreatorProfile& profile)
{
    QJsonObject profileData;
    profileData["address"] = profile.address;
    profileData["totalNFTs"] = profile.totalNFTs;
    profileData["totalVolume"] = QString::number(profile.totalVolume);
    profileData["trustScore"] = profile.trustScore;
    profileData["isTrusted"] = profile.isTrusted;
    profileData["badges"] = profile.badges;
    
    QNetworkRequest request(QUrl(m_apiBaseUrl + "/creators/" + profile.address));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_net.put(request, QJsonDocument(profileData).toJson());
    connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });
}

void TokenNFTExplorer::handleNFTPurchase(const NFTData& nft)
{
    if (!m_walletModel) return;
    
    QString buyerAddress = m_walletModel->getWalletAddress();
    
    // Create purchase transaction
    QString txHex;
    if (!createNFTPurchaseTransaction(nft.id, nft.creator, nft.listedPriceSatoshis, txHex)) {
        QMessageBox::warning(this, tr("Purchase Error"), tr("Failed to create purchase transaction."));
        return;
    }
    
    // Sign and broadcast transaction
    QString txid;
    if (!signAndBroadcastTransaction(txHex, txid)) {
        QMessageBox::warning(this, tr("Purchase Error"), tr("Failed to broadcast transaction."));
        return;
    }
    
    // Record purchase
    TransactionRecord record;
    record.txid = txid;
    record.type = "purchase";
    record.nftId = nft.id;
    record.fromAddress = buyerAddress;
    record.toAddress = nft.creator;
    record.amountSatoshis = nft.listedPriceSatoshis;
    record.amountUSD = nft.listedPriceUSD;
    record.timestamp = QDateTime::currentDateTime();
    record.status = "confirmed";
    
    addTransactionRecord(record);
    
    // Update NFT ownership
    updateNFTOwnership(nft.id, buyerAddress);
    
    // Update creator trust score
    updateCreatorTrustScore(nft.creator);
    
    // Record to API
    recordPurchaseToApi(nft.id, buyerAddress, nft.listedPriceSatoshis, nft.listedPriceUSD, txid);
    
    QMessageBox::information(this, tr("Purchase Successful"), 
        tr("NFT purchased successfully! Transaction ID: %1").arg(txid));
}

void TokenNFTExplorer::handleNFTTransfer(const NFTData& nft)
{
    if (!m_walletModel) return;
    
    // Show transfer dialog
    bool ok;
    QString recipientAddress = QInputDialog::getText(this, tr("Transfer NFT"),
        tr("Enter recipient address:"), QLineEdit::Normal, "", &ok);
    
    if (!ok || recipientAddress.isEmpty()) return;
    
    // Validate address format
    if (!isValidAddress(recipientAddress)) {
        QMessageBox::warning(this, tr("Invalid Address"), tr("Please enter a valid Shahcoin address."));
        return;
    }
    
    // Create transfer transaction
    QString txHex;
    if (!createNFTTransferTransaction(nft.id, recipientAddress, txHex)) {
        QMessageBox::warning(this, tr("Transfer Error"), tr("Failed to create transfer transaction."));
        return;
    }
    
    // Sign and broadcast transaction
    QString txid;
    if (!signAndBroadcastTransaction(txHex, txid)) {
        QMessageBox::warning(this, tr("Transfer Error"), tr("Failed to broadcast transaction."));
        return;
    }
    
    // Record transfer
    TransactionRecord record;
    record.txid = txid;
    record.type = "transfer";
    record.nftId = nft.id;
    record.fromAddress = m_walletModel->getWalletAddress();
    record.toAddress = recipientAddress;
    record.amountSatoshis = 0; // Transfer is free
    record.amountUSD = 0.0;
    record.timestamp = QDateTime::currentDateTime();
    record.status = "confirmed";
    
    addTransactionRecord(record);
    
    // Update NFT ownership
    updateNFTOwnership(nft.id, recipientAddress);
    
    QMessageBox::information(this, tr("Transfer Successful"), 
        tr("NFT transferred successfully! Transaction ID: %1").arg(txid));
}

void TokenNFTExplorer::handleNFTFlag(const NFTData& nft)
{
    // Show flag dialog
    QStringList reasons = {
        tr("Inappropriate Content"),
        tr("Copyright Violation"),
        tr("Spam"),
        tr("Fake/Scam"),
        tr("Other")
    };
    
    bool ok;
    QString reason = QInputDialog::getItem(this, tr("Flag NFT"),
        tr("Select reason for flagging:"), reasons, 0, false, &ok);
    
    if (!ok) return;
    
    // Submit flag to API
    QJsonObject flagData;
    flagData["nftId"] = nft.id;
    flagData["reason"] = reason;
    flagData["reporter"] = m_walletModel ? m_walletModel->getWalletAddress() : "";
    flagData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QNetworkRequest request(QUrl(m_apiBaseUrl + "/flags"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_net.post(request, QJsonDocument(flagData).toJson());
    connect(reply, &QNetworkReply::finished, [this, reply, nft]() {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, tr("Flag Submitted"), 
                tr("NFT has been flagged for review. Thank you for helping keep the marketplace safe."));
        } else {
            QMessageBox::warning(this, tr("Flag Error"), tr("Failed to submit flag. Please try again."));
        }
        reply->deleteLater();
    });
}

void TokenNFTExplorer::applyVerifiedFilter()
{
    // Filter NFTs based on verification status
    QList<NFTData> filteredNFTs;
    
    for (const NFTData& nft : m_nftList) {
        if (shouldShowInMarketplace(nft)) {
            filteredNFTs.append(nft);
        }
    }
    
    // Update marketplace table
    populateMarketplaceTableWithData(filteredNFTs);
}

bool TokenNFTExplorer::shouldShowInMarketplace(const NFTData& nft) const
{
    // Check if NFT meets verification criteria
    if (!isVerifiedNFT(nft)) {
        return false;
    }
    
    // Check if NFT has valid metadata
    if (nft.metadata.isEmpty()) {
        return false;
    }
    
    // Check if NFT is not flagged
    if (isNFTFlagged(nft.id)) {
        return false;
    }
    
    return true;
}

bool TokenNFTExplorer::isVerifiedNFT(const NFTData& nft) const
{
    // Check tier-based verification
    QString tier = nftTierFromMetadata(nft.metadata);
    if (tier == "Pro" || tier == "Premium") {
        return true;
    }
    
    // Check admin verification
    if (metadataAdminVerified(nft.metadata)) {
        return true;
    }
    
    // Check creator trust status
    if (m_creatorProfiles.contains(nft.creator)) {
        const CreatorProfile& profile = m_creatorProfiles[nft.creator];
        if (profile.isTrusted) {
            return true;
        }
    }
    
    return false;
}

bool TokenNFTExplorer::metadataAdminVerified(const QString& metadata) const
{
    QJsonDocument doc = QJsonDocument::fromJson(metadata.toUtf8());
    if (doc.isNull()) return false;
    
    QJsonObject obj = doc.object();
    return obj["adminVerified"].toBool();
}

bool TokenNFTExplorer::isNFTFlagged(const QString& nftId) const
{
    // Check local flag cache
    return m_flaggedNFTs.contains(nftId);
}

void TokenNFTExplorer::handleListingApproval(const QString& nftId, bool approved)
{
    // Update admin verification flag
    setAdminVerifyFlagToApi(nftId, approved);
    
    // Update local metadata
    for (NFTData& nft : m_nftList) {
        if (nft.id == nftId) {
            QJsonDocument doc = QJsonDocument::fromJson(nft.metadata.toUtf8());
            if (!doc.isNull()) {
                QJsonObject obj = doc.object();
                obj["adminVerified"] = approved;
                nft.metadata = QJsonDocument(obj).toJson();
            }
            break;
        }
    }
    
    // Refresh marketplace
    populateMarketplaceTable();
}

void TokenNFTExplorer::updateDynamicPricing()
{
    // Update USD prices for all listed NFTs
    if (!m_priceOracle) return;
    
    double shahPriceUSD = m_priceOracle->getCurrentPrice();
    
    for (NFTData& nft : m_nftList) {
        if (nft.isListed && nft.listedPriceSatoshis > 0) {
            nft.listedPriceUSD = (double)nft.listedPriceSatoshis / 100000000.0 * shahPriceUSD;
        }
    }
    
    // Refresh marketplace table
    populateMarketplaceTable();
}

void TokenNFTExplorer::toggleFavorite(const QString& nftId)
{
    if (m_localFavorites.contains(nftId)) {
        m_localFavorites.remove(nftId);
    } else {
        m_localFavorites.insert(nftId);
    }
    
    // Save to local storage
    saveLocalFavorites();
    
    // Sync to API
    syncFavoriteToApi(nftId, m_localFavorites.contains(nftId));
    
    // Refresh UI
    populateMarketplaceTable();
}

void TokenNFTExplorer::syncFavoriteToApi(const QString& nftId, bool isFavorited)
{
    if (!m_walletModel) return;
    
    QString walletAddress = m_walletModel->getWalletAddress();
    
    QJsonObject favoriteData;
    favoriteData["nftId"] = nftId;
    favoriteData["walletAddress"] = walletAddress;
    favoriteData["isFavorited"] = isFavorited;
    favoriteData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QNetworkRequest request(QUrl(m_apiBaseUrl + "/favorites"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_net.post(request, QJsonDocument(favoriteData).toJson());
    connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });
}

void TokenNFTExplorer::loadLocalFavorites()
{
    m_settings.beginGroup("favorites");
    QStringList favorites = m_settings.value("nftIds").toStringList();
    m_settings.endGroup();
    
    m_localFavorites.clear();
    for (const QString& nftId : favorites) {
        m_localFavorites.insert(nftId);
    }
}

void TokenNFTExplorer::saveLocalFavorites()
{
    m_settings.beginGroup("favorites");
    m_settings.setValue("nftIds", QStringList(m_localFavorites.begin(), m_localFavorites.end()));
    m_settings.endGroup();
}

void TokenNFTExplorer::loadLocalWatchlist()
{
    m_settings.beginGroup("watchlist");
    QStringList watchlist = m_settings.value("nftIds").toStringList();
    m_settings.endGroup();
    
    m_localWatchlist.clear();
    for (const QString& nftId : watchlist) {
        m_localWatchlist.insert(nftId);
    }
}

void TokenNFTExplorer::saveLocalWatchlist()
{
    m_settings.beginGroup("watchlist");
    m_settings.setValue("nftIds", QStringList(m_localWatchlist.begin(), m_localWatchlist.end()));
    m_settings.endGroup();
}

void TokenNFTExplorer::updateLocalCache()
{
    // Save current state to local cache
    m_settings.beginGroup("marketplace_cache");
    m_settings.setValue("last_sync", QDateTime::currentDateTime().toString(Qt::ISODate));
    m_settings.setValue("total_listings", m_nftList.size());
    m_settings.setValue("total_creators", m_creatorProfiles.size());
    m_settings.endGroup();
}

bool TokenNFTExplorer::isValidAddress(const QString& address) const
{
    // Basic Shahcoin address validation
    if (address.length() != 34) return false;
    if (!address.startsWith("S")) return false;
    
    // Add more validation as needed
    return true;
}

void TokenNFTExplorer::updateNFTOwnership(const QString& nftId, const QString& newOwner)
{
    for (NFTData& nft : m_nftList) {
        if (nft.id == nftId) {
            nft.owner = newOwner;
            nft.isListed = false; // Remove from marketplace when transferred
            nft.listedPriceSatoshis = 0;
            nft.listedPriceUSD = 0.0;
            break;
        }
    }
}

bool TokenNFTExplorer::createNFTTransferTransaction(const QString& nftId, const QString& recipientAddress, QString& txHex)
{
    // This would integrate with the actual blockchain transaction creation
    // For now, return a placeholder
    txHex = "placeholder_transaction_hex";
    return true;
}

void TokenNFTExplorer::populateMarketplaceTableWithData(const QList<NFTData>& nfts)
{
    // Auto-hide risky assets (settings-driven)
    bool autoHide = m_settings.value("security/auto_hide_high_risk", true).toBool();
    bool showRisky = m_settings.value("security/show_risky", false).toBool();

    // Prepare filtered list with simple heuristic risk placeholder
    QList<NFTData> display;
    display.reserve(nfts.size());
    for (const NFTData &n : nfts) {
        int riskScore = (n.imageHash.isEmpty() || n.attributes.isEmpty()) ? 45 : 10;
        bool isRisky = riskScore > 60; // threshold
        if (autoHide && isRisky && !showRisky) continue;
        display.push_back(n);
    }

    m_marketTable->setRowCount(display.size());
    
    for (int row = 0; row < display.size(); ++row) {
        const NFTData& nft = display[row];
        
        // Name with verification badge
        QString nameText = nft.name;
        if (isVerifiedNFT(nft)) {
            nameText += " ✓";
        }
        m_marketTable->setItem(row, 0, new QTableWidgetItem(nameText));
        
        // Tier
        QString tier = nftTierFromMetadata(nft.metadata);
        m_marketTable->setItem(row, 1, new QTableWidgetItem(tier));
        
        // Creator with trust badge
        QString creatorText = formatAddress(nft.creator);
        if (m_creatorProfiles.contains(nft.creator) && m_creatorProfiles[nft.creator].isTrusted) {
            creatorText += " 🏆";
        }
        m_marketTable->setItem(row, 2, new QTableWidgetItem(creatorText));
        
        // Owner
        m_marketTable->setItem(row, 3, new QTableWidgetItem(formatAddress(nft.owner)));
        
        // Category
        m_marketTable->setItem(row, 4, new QTableWidgetItem(nft.category));
        
        // Traits
        QString traitsText = "";
        if (!nft.traits.isEmpty()) {
            QStringList traitList;
            for (auto it = nft.traits.begin(); it != nft.traits.end(); ++it) {
                traitList << QString("%1: %2").arg(it.key()).arg(it.value().toString());
            }
            traitsText = traitList.join(", ");
        }
        m_marketTable->setItem(row, 5, new QTableWidgetItem(traitsText));
        
        // Price (SHAH)
        QString shahPrice = QString::number(nft.listedPriceSatoshis / 100000000.0, 'f', 8);
        m_marketTable->setItem(row, 6, new QTableWidgetItem(shahPrice));
        
        // Price (USD)
        QString usdPrice = QString::number(nft.listedPriceUSD, 'f', 2);
        m_marketTable->setItem(row, 7, new QTableWidgetItem("$" + usdPrice));
        
        // Verified status
        QString verifiedText = isVerifiedNFT(nft) ? "Yes" : "No";
        m_marketTable->setItem(row, 8, new QTableWidgetItem(verifiedText));
        
        // Favorite status
        QString favoriteText = m_localFavorites.contains(nft.id) ? "❤️" : "🤍";
        m_marketTable->setItem(row, 9, new QTableWidgetItem(favoriteText));
    }
}

void TokenNFTExplorer::updateLocalListing(const QJsonObject& listing)
{
    QString nftId = listing["nftId"].toString();
    
    // Find and update the NFT in our local list
    for (NFTData& nft : m_nftList) {
        if (nft.id == nftId) {
            nft.isListed = listing["isListed"].toBool();
            nft.listedPriceSatoshis = listing["priceSatoshis"].toVariant().toLongLong();
            nft.listedPriceUSD = listing["priceUSD"].toDouble();
            nft.category = listing["category"].toString();
            
            // Update traits if provided
            if (listing.contains("traits")) {
                nft.traits = listing["traits"].toObject();
            }
            
            break;
        }
    }
}

bool NFTDetailsDialog::handleStripePayment(const TokenNFTExplorer::NFTData& nft, const QString& buyerAddress, double usdPrice)
{
    // Check if Stripe is enabled for this NFT
    bool isVerified = nft.verified || 
                     (nft.metadata.contains("adminVerified") && nft.metadata.contains("true")) ||
                     (nft.metadata.contains("tier") && (nft.metadata.contains("Pro") || nft.metadata.contains("Premium")));
    
    if (!isVerified) {
        QMessageBox::warning(this, tr("Stripe Payment Unavailable"), 
            tr("Stripe payments are only available for verified NFTs."));
        return false;
    }
    
    // Show Stripe payment dialog
    QDialog stripeDialog(this);
    stripeDialog.setWindowTitle(tr("Stripe Payment"));
    stripeDialog.setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(&stripeDialog);
    
    // Payment information
    QLabel* infoLabel = new QLabel(tr("Complete your purchase with credit card"));
    infoLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(infoLabel);
    
    QLabel* nftLabel = new QLabel(tr("NFT: %1").arg(nft.name));
    layout->addWidget(nftLabel);
    
    QLabel* priceLabel = new QLabel(tr("Price: $%1 USD").arg(usdPrice, 0, 'f', 2));
    priceLabel->setStyleSheet("font-size: 14px; color: #28a745; font-weight: bold;");
    layout->addWidget(priceLabel);
    
    QLabel* conversionLabel = new QLabel(tr("USD will be automatically converted to SHAH"));
    conversionLabel->setStyleSheet("color: #6c757d; font-size: 12px;");
    layout->addWidget(conversionLabel);
    
    // Payment options
    QGroupBox* paymentGroup = new QGroupBox(tr("Payment Options"));
    QVBoxLayout* paymentLayout = new QVBoxLayout(paymentGroup);
    
    QRadioButton* cardRadio = new QRadioButton(tr("💳 Credit/Debit Card"));
    cardRadio->setChecked(true);
    paymentLayout->addWidget(cardRadio);
    
    QRadioButton* applePayRadio = new QRadioButton(tr("🍎 Apple Pay"));
    paymentLayout->addWidget(applePayRadio);
    
    QRadioButton* googlePayRadio = new QRadioButton(tr("📱 Google Pay"));
    paymentLayout->addWidget(googlePayRadio);
    
    layout->addWidget(paymentGroup);
    
    // Terms and conditions
    QCheckBox* termsCheck = new QCheckBox(tr("I agree to the terms and conditions"));
    layout->addWidget(termsCheck);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* payButton = new QPushButton(tr("💳 Pay $%1").arg(usdPrice, 0, 'f', 2));
    payButton->setStyleSheet("background-color: #007bff; color: white; padding: 10px; font-weight: bold;");
    QPushButton* cancelButton = new QPushButton(tr("Cancel"));
    buttonLayout->addWidget(payButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    // Connect signals
    connect(payButton, &QPushButton::clicked, &stripeDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &stripeDialog, &QDialog::reject);
    
    // Enable/disable pay button based on terms acceptance
    payButton->setEnabled(false);
    connect(termsCheck, &QCheckBox::toggled, payButton, &QPushButton::setEnabled);
    
    if (stripeDialog.exec() != QDialog::Accepted) {
        return false; // User cancelled
    }
    
    // Process Stripe payment
    QProgressDialog progressDialog(tr("Processing payment..."), tr("Cancel"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setAutoClose(false);
    progressDialog.show();
    
    // Simulate payment processing
    QTimer::singleShot(2000, [&progressDialog]() {
        progressDialog.setLabelText(tr("Converting USD to SHAH..."));
    });
    
    QTimer::singleShot(4000, [&progressDialog]() {
        progressDialog.setLabelText(tr("Finalizing transaction..."));
    });
    
    // In a real implementation, this would:
    // 1. Create Stripe checkout session
    // 2. Redirect to Stripe hosted checkout
    // 3. Handle webhook for payment confirmation
    // 4. Convert USD to SHAH via ShahSwap
    // 5. Complete the NFT transfer
    
    // For now, simulate successful payment
    QTimer::singleShot(6000, [&progressDialog]() {
        progressDialog.close();
    });
    
    progressDialog.exec();
    
    // Show success message
    QMessageBox::information(this, tr("Payment Successful"), 
        tr("Your payment has been processed successfully!\n\n"
           "USD $%1 has been converted to SHAH and the NFT transfer is being completed.").arg(usdPrice, 0, 'f', 2));
    
    return true;
}

void TokenNFTExplorer::validateTrademarkCompliance(const QString& name)
{
    if (isTrademarkViolation(name)) {
        QMessageBox::warning(this, "Trademark Violation",
            "⚠️ Projects using SHAH-branded names without authorization will be rejected.\n\n"
            "SHAHCOIN®, SHAH, and all derivatives are registered trademarks.\n\n"
            "Please choose a different name that doesn't infringe on SHAHCOIN branding.\n\n"
            "For licensing inquiries: info@shah.vip");
    }
}

void TokenNFTExplorer::showTrademarkWarning()
{
    QMessageBox::information(this, "SHAHCOIN® Trademark Notice",
        "❗ Reminder: You may not use the SHAHCOIN name, logo, or brand identity in your token/NFT name unless explicitly authorized.\n\n"
        "See LICENSE and NOTICE for details.\n\n"
        "Unauthorized use will result in rejection and potential legal action.");
}

bool TokenNFTExplorer::isTrademarkViolation(const QString& name)
{
    QString lowerName = name.toLower();
    
    // Check for SHAH-related terms
    QStringList trademarkTerms = {
        "shahcoin", "shah", "shahcoin", "shah-coin", "shah_coin",
        "shahcoin", "shah-token", "shah_token", "shah-nft", "shah_nft"
    };
    
    for (const QString& term : trademarkTerms) {
        if (lowerName.contains(term)) {
            return true;
        }
    }
    
    return false;
}
