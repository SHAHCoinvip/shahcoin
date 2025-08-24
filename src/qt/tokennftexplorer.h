// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#ifndef SHAHCOIN_QT_TOKENNFTEXPLORER_H
#define SHAHCOIN_QT_TOKENNFTEXPLORER_H

#include <QWidget>
#include <QTabWidget>
#include <QTableView>
#include <QTableWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QProgressBar>
#include <QDialog>
#include <QTextEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCheckBox>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateEdit>
#include <QStackedWidget>
#include <QScrollArea>
#include <QSpinBox>
#include "moderationdialogs.h"

class WalletModel;
class PlatformStyle;
class PriceOracle;

/**
 * Token & NFT Explorer Panel
 * 
 * Provides a comprehensive interface for browsing and exploring
 * all tokens and NFTs created on the Shahcoin blockchain.
 */
class TokenNFTExplorer : public QWidget
{
    Q_OBJECT

public:
    explicit TokenNFTExplorer(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~TokenNFTExplorer();

    void setWalletModel(WalletModel* walletModel);

public Q_SLOTS:
    void refreshData();

private Q_SLOTS:
    void onTabChanged(int index);
    void onTokenSearchChanged();
    void onNFTSearchChanged();
    void onTokenFilterChanged();
    void onNFTFilterChanged();
    void onRefreshClicked();
    void onExportClicked();
    void onTokenRowClicked(int row, int column);
    void onNFTRowClicked(int row, int column);
    void onTokenContextMenu(const QPoint& pos);
    void onNFTContextMenu(const QPoint& pos);
    void onViewTokenDetails();
    void onViewNFTDetails();
    void onExportTokenData();
    void onExportNFTData();

    // Marketplace slots
    void onMarketToggleChanged(int idx);
    void onMarketFilterChanged();
    void onMarketplaceContextMenu(const QPoint& pos);
    void onOraclePriceUpdated(double);
    void onAdminToggleVerified();
    void showNFTDetailsDialog();
    void showMarketplaceListingDialog();
    
    // Advanced filtering slots
    void onAdvancedFilterClicked();
    void onFilterPresetClicked();
    void onSearchTextChanged();
    void onSortByChanged();
    void onSortOrderChanged();
    void onFilterCheckboxChanged();
    void onDateRangeChanged();
    void onAddTraitClicked();
    void onRemoveTraitClicked();
    void onSearchSuggestionSelected();
    void onSearchDebounceTimeout();
    
    // Bulk operations slots
    void onBulkOperationsClicked();
    void onSelectAllClicked();
    void onDeselectAllClicked();
    void onBatchListClicked();
    void onBatchPriceUpdateClicked();
    void onBatchDelistClicked();
    void onBulkOperationProgress(int value);
    void onBulkOperationCompleted();
    
    // Mobile responsive slots
    void onToggleViewClicked();
    void onMobileNFTClicked(QListWidgetItem* item);
    void onMobileFilterChanged();
    
    // Performance optimization slots
    void onScrollTimerTimeout();
    void onVirtualScrollValueChanged();

private:
    void setupUI();
    void setupTokenTab();
    void setupNFTTab();
    void setupMarketplaceTab();
    void setupControls();
    void connectSignals();
    void applyTheme();
    void loadTokenData();
    void loadNFTData();
    void populateTokenTable();
    void populateNFTTable();
    void populateMarketplaceTable();
    void updateTokenFilters();
    void updateNFTFilters();
    void showTokenDetailsDialog(const QString& tokenId);
    void showNFTDetailsDialog(const QString& nftId);
    void exportTokenData(const QString& tokenId);
    void exportNFTData(const QString& nftId);
    QPixmap loadImageFromHash(const QString& imageHash);
    QPixmap createPlaceholderImage(const QString& text, const QSize& size);
    QString formatTokenSupply(qint64 supply, int decimals);
    QString formatBlockHeight(int blockHeight);
    QString formatAddress(const QString& address);

    // Marketplace helpers
    bool isVerifiedNFT(const struct NFTData& nft) const;
    QString nftTierFromMetadata(const QString& metadataJson) const;
    void showCreatorProfileDialog(const QString& creatorAddress);
    bool isFavorited(const QString& nftId) const;
    void toggleFavorite(const QString& nftId);

    // API helpers
    void submitListingToApi(const QString& nftId, const QString& title, const QString& description,
                            const QString& category, const QString& tags,
                            qint64 priceSatoshis, double priceUSD, bool submitForVerification);
    void recordPurchaseToApi(const QString& nftId, const QString& buyer, qint64 priceSatoshis, double priceUSD, const QString& txid);
    void setAdminVerifyFlagToApi(const QString& nftId, bool verified);
    void fetchMarketplaceNftsFromApi();
    
    // Wallet signing methods
    bool signAndBroadcastTransaction(const QString& txHex, QString& txid);
    bool createNFTPurchaseTransaction(const QString& nftId, const QString& sellerAddress, qint64 priceSatoshis, QString& txHex);
    bool createNFTListingTransaction(const QString& nftId, qint64 priceSatoshis, QString& txHex);
    
    // Creator trust scoring
    struct CreatorProfile {
        QString address;
        int totalNFTs;
        qint64 totalVolume;
        double trustScore;
        bool isTrusted;
        QString badges;
    };
    QMap<QString, CreatorProfile> m_creatorProfiles;
    void updateCreatorTrustScore(const QString& creatorAddress);
    void loadCreatorProfiles();
    
    // Transaction history tracking
    struct TransactionRecord {
        QString txid;
        QString type; // "purchase", "listing", "transfer", "mint"
        QString nftId;
        QString fromAddress;
        QString toAddress;
        qint64 amountSatoshis;
        double amountUSD;
        QDateTime timestamp;
        QString status; // "pending", "confirmed", "failed"
        QString metadata; // JSON string for additional data
    };
    QList<TransactionRecord> m_transactionHistory;
    void addTransactionRecord(const TransactionRecord& record);
    void loadTransactionHistory();
    void exportTransactionHistory(const QString& filename);
    
    // Analytics dashboard
    struct AnalyticsData {
        qint64 totalVolume24h;
        qint64 totalVolume7d;
        qint64 totalVolume30d;
        int totalTransactions24h;
        int totalTransactions7d;
        int totalTransactions30d;
        double averagePrice24h;
        double averagePrice7d;
        double averagePrice30d;
        QMap<QString, int> topCreators;
        QMap<QString, int> topCategories;
        QList<QPair<QString, double>> priceHistory; // date, price
    };
    AnalyticsData m_analyticsData;
    void updateAnalyticsData();
    void showAnalyticsDashboard();
    
    // Escrow/payment protection
    struct EscrowData {
        QString escrowId;
        QString nftId;
        QString buyer;
        QString seller;
        qint64 amountSatoshis;
        QDateTime createdAt;
        QDateTime expiresAt;
        QString status; // "pending", "funded", "released", "refunded", "expired"
        QString disputeReason;
    };
    QMap<QString, EscrowData> m_escrowData;
    void createEscrow(const QString& nftId, const QString& buyer, const QString& seller, qint64 amount);
    void fundEscrow(const QString& escrowId);
    void releaseEscrow(const QString& escrowId);
    void refundEscrow(const QString& escrowId, const QString& reason);
    void showEscrowDialog(const QString& nftId);
    
    // Helper methods
    bool shouldShowInMarketplace(const NFTData& nft) const;
    bool isFavorited(const QString& nftId) const;
    QString formatAddress(const QString& address) const;
    QString nftTierFromMetadata(const QString& metadata) const;
    
    // Cross-platform sync methods
    void setupCrossPlatformSync();
    void syncMarketplaceData();
    void syncListings();
    void syncFavorites();
    void syncCreatorData();
    void updateLocalListing(const QJsonObject& listing);
    void updateCreatorProfile(const QJsonObject& creatorData);
    void saveCreatorProfileToApi(const CreatorProfile& profile);
    void syncFavoriteToApi(const QString& nftId, bool isFavorited);
    void updateLocalCache();
    
    // NFT action methods
    void handleNFTPurchase(const NFTData& nft);
    void handleNFTTransfer(const NFTData& nft);
    void handleNFTFlag(const NFTData& nft);
    void handleListingApproval(const QString& nftId, bool approved);
    void updateNFTOwnership(const QString& nftId, const QString& newOwner);
    bool createNFTTransferTransaction(const QString& nftId, const QString& recipientAddress, QString& txHex);
    bool isValidAddress(const QString& address) const;
    
    // Verification and filtering methods
    void applyVerifiedFilter();
    bool isVerifiedNFT(const NFTData& nft) const;
    bool metadataAdminVerified(const QString& metadata) const;
    bool isNFTFlagged(const QString& nftId) const;
    
    // Dynamic pricing methods
    void updateDynamicPricing();
    
    // Favorites and watchlist methods
    void toggleFavorite(const QString& nftId);
    void loadLocalFavorites();
    void saveLocalFavorites();
    void loadLocalWatchlist();
    void saveLocalWatchlist();
    
    // UI update methods
    void populateMarketplaceTableWithData(const QList<NFTData>& nfts);
    
    // Bulk operations for creators
    struct BulkOperation {
        QString operationType; // "batch_list", "batch_price_update", "batch_delist"
        QStringList nftIds;
        QMap<QString, qint64> newPrices; // nftId -> new price in satoshis
        QString category;
        QString tags;
        bool submitForVerification;
        QDateTime scheduledTime;
    };
    QList<BulkOperation> m_pendingBulkOperations;
    void showBulkOperationsDialog();
    void executeBulkOperation(const BulkOperation& operation);
    void scheduleBulkOperation(const BulkOperation& operation);
    void cancelBulkOperation(int operationIndex);
    void showBulkOperationProgress(const BulkOperation& operation);
    
    // Advanced filtering and search
    struct AdvancedFilters {
        QString searchText;
        QString category;
        QString creator;
        QString tier;
        qint64 minPriceSatoshis;
        qint64 maxPriceSatoshis;
        bool verifiedOnly;
        bool myNFTsOnly;
        bool favoritedOnly;
        QStringList traits;
        QDateTime minMintDate;
        QDateTime maxMintDate;
        bool hasUnlockableContent;
        bool isTradeLocked;
        QString sortBy; // "price", "date", "rarity", "popularity"
        QString sortOrder; // "asc", "desc"
    };
    AdvancedFilters m_currentFilters;
    void applyAdvancedFilters();
    void resetFilters();
    void saveFilterPreset(const QString& name);
    void loadFilterPreset(const QString& name);
    void showAdvancedFilterDialog();
    void updateFilterUI();
    
    // Search and autocomplete
    struct SearchSuggestion {
        QString text;
        QString type; // "nft", "creator", "category", "trait"
        double relevance;
    };
    QList<SearchSuggestion> m_searchSuggestions;
    void updateSearchSuggestions(const QString& query);
    void showSearchSuggestions();
    void applySearchSuggestion(const SearchSuggestion& suggestion);
    
    // Mobile responsive design helpers
    void setupMobileLayout();
    void setupDesktopLayout();
    void toggleMobileView();
    bool m_isMobileView;
    
    // Performance optimization
    void setupVirtualScrolling();
    void loadVisibleItemsOnly();
    void preloadAdjacentItems();
    int m_visibleItemCount;
    int m_totalItemCount;
    QTimer* m_scrollTimer;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Token Tab
    QWidget* m_tokenTab;
    QVBoxLayout* m_tokenLayout;
    QHBoxLayout* m_tokenControlsLayout;
    QLineEdit* m_tokenSearchEdit;
    QComboBox* m_tokenFilterCombo;
    QPushButton* m_tokenRefreshButton;
    QTableWidget* m_tokenTable;
    
    // NFT Tab
    QWidget* m_nftTab;
    QVBoxLayout* m_nftLayout;
    QHBoxLayout* m_nftControlsLayout;
    QLineEdit* m_nftSearchEdit;
    QComboBox* m_nftFilterCombo;
    QCheckBox* m_verifiedOnlyCheck;
    QPushButton* m_nftRefreshButton;
    QTableWidget* m_nftTable;

    // Marketplace Tab
    QWidget* m_marketTab;
    QVBoxLayout* m_marketLayout;
    QHBoxLayout* m_marketControlsLayout;
    QComboBox* m_marketToggle; // All / Verified Only / My NFTs / Favorites
    QComboBox* m_marketCategory;
    QLineEdit* m_marketCreator;
    QLineEdit* m_marketPriceMin;
    QLineEdit* m_marketPriceMax;
    QLineEdit* m_marketTraits;
    QPushButton* m_marketRefreshButton;
    QPushButton* m_marketAdminButton;
    QTableWidget* m_marketTable;
    QMenu* m_marketContextMenu;
    QString m_selectedMarketNFTId;
    
    // Advanced filtering UI components
    QPushButton* m_advancedFilterButton;
    QPushButton* m_filterPresetButton;
    QLineEdit* m_searchBox;
    QComboBox* m_sortByCombo;
    QComboBox* m_sortOrderCombo;
    QCheckBox* m_verifiedOnlyCheck;
    QCheckBox* m_myNFTsOnlyCheck;
    QCheckBox* m_favoritedOnlyCheck;
    QCheckBox* m_hasUnlockableContentCheck;
    QCheckBox* m_isTradeLockedCheck;
    QDateEdit* m_minMintDateEdit;
    QDateEdit* m_maxMintDateEdit;
    QListWidget* m_traitsList;
    QPushButton* m_addTraitButton;
    QPushButton* m_removeTraitButton;
    
    // Bulk operations UI components
    QPushButton* m_bulkOperationsButton;
    QPushButton* m_selectAllButton;
    QPushButton* m_deselectAllButton;
    QPushButton* m_batchListButton;
    QPushButton* m_batchPriceUpdateButton;
    QPushButton* m_batchDelistButton;
    QProgressBar* m_bulkOperationProgress;
    QLabel* m_bulkOperationStatus;
    
    // Search suggestions UI
    QListWidget* m_searchSuggestionsList;
    QTimer* m_searchDebounceTimer;
    
    // Mobile responsive UI
    QStackedWidget* m_mobileStackedWidget;
    QWidget* m_desktopView;
    QWidget* m_mobileView;
    QPushButton* m_toggleViewButton;
    QListWidget* m_mobileNFTList;
    QVBoxLayout* m_mobileFilterLayout;
    
    // Performance optimization UI
    QScrollArea* m_virtualScrollArea;
    QWidget* m_virtualScrollContent;
    QTimer* m_scrollTimer;
    
    // Global Controls
    QHBoxLayout* m_globalControlsLayout;
    QPushButton* m_exportButton;
    QLabel* m_statusLabel;
    QProgressBar* m_loadingProgress;
    
    // Data
    WalletModel* m_walletModel;
    const PlatformStyle* m_platformStyle;
    QTimer* m_refreshTimer;
    QSettings m_settings { "Shahcoin", "Wallet" };
    PriceOracle* m_priceOracle { nullptr };
    QNetworkAccessManager m_net;
    QString m_apiBaseUrl;
    QTimer* m_marketPollTimer { nullptr };
    
    // Cross-platform sync data
    QSet<QString> m_localFavorites;
    QSet<QString> m_localWatchlist;
    QSet<QString> m_flaggedNFTs;
    
    // Moderation Panel structures and data
    struct FlaggedNFT {
        QString nftId;
        QString nftName;
        QString creatorAddress;
        QString reporterAddress;
        QString reason;
        QString description;
        int reportCount;
        QDateTime firstReported;
        QDateTime lastReported;
        QString status; // "pending", "reviewed", "approved", "hidden", "removed"
        QString moderatorNotes;
        QStringList reportHistory; // List of report reasons
    };
    QList<FlaggedNFT> m_flaggedNFTsList;
    
    struct ModerationAction {
        QString actionId;
        QString nftId;
        QString moderatorAddress;
        QString actionType; // "approve", "hide", "remove", "blacklist_creator", "mark_safe"
        QString reason;
        QDateTime timestamp;
        QString notes;
        bool autoAction; // Whether this was an automatic action
    };
    QList<ModerationAction> m_moderationActions;
    
    struct CreatorBlacklist {
        QString creatorAddress;
        QString reason;
        QDateTime blacklistedAt;
        QString blacklistedBy;
        QString notes;
        bool isPermanent;
        QDateTime expiresAt; // If not permanent
    };
    QMap<QString, CreatorBlacklist> m_blacklistedCreators;
    
    // Moderation settings
    struct ModerationSettings {
        int autoHideThreshold; // Number of reports before auto-hiding
        bool autoHideEnabled;
        int autoReviewThreshold; // Number of reports before requiring review
        bool autoReviewEnabled;
        QStringList trustedModerators; // List of trusted moderator addresses
        bool requireModeratorApproval; // Whether to require moderator approval for actions
    };
    ModerationSettings m_moderationSettings;
    
    // Moderation Panel UI components
    QWidget* m_moderationTab;
    QVBoxLayout* m_moderationLayout;
    QHBoxLayout* m_moderationControlsLayout;
    QComboBox* m_moderationFilterCombo; // "All", "Pending", "Reviewed", "Hidden", "Removed"
    QComboBox* m_moderationSortCombo; // "Date", "Report Count", "Creator", "Status"
    QPushButton* m_moderationRefreshButton;
    QPushButton* m_moderationSettingsButton;
    QPushButton* m_moderationExportButton;
    QTableWidget* m_moderationTable;
    QMenu* m_moderationContextMenu;
    QString m_selectedFlaggedNFTId;
    
    // Moderation Panel methods
    void setupModerationTab();
    void loadFlaggedNFTs();
    void populateModerationTable();
    void showModerationSettingsDialog();
    void showFlaggedNFTDetailsDialog(const QString& nftId);
    void performModerationAction(const QString& nftId, const QString& action, const QString& reason, const QString& notes);
    void blacklistCreator(const QString& creatorAddress, const QString& reason, bool permanent = false, int daysToExpire = 0);
    void unblacklistCreator(const QString& creatorAddress);
    void addModerationAction(const ModerationAction& action);
    void loadModerationActions();
    void exportModerationLog(const QString& filename);
    void updateModerationSettings(const ModerationSettings& settings);
    void checkAutoModeration(const QString& nftId);
    bool isTrustedModerator(const QString& address) const;
    void refreshModerationData();
    
    // Moderation Panel slots
    void onModerationFilterChanged();
    void onModerationSortChanged();
    void onModerationContextMenu(const QPoint& pos);
    void onModerationActionClicked();
    void onModerationSettingsClicked();
    void onModerationExportClicked();
    void onApproveNFT();
    void onHideNFT();
    void onRemoveNFT();
    void onBlacklistCreator();
    void onMarkAsSafe();
    void onViewFlaggedNFTDetails();
    void onViewModerationHistory();
    
    // Token Data
    struct TokenData {
        QString id;
        QString name;
        QString symbol;
        qint64 supply;
        int decimals;
        QString creator;
        int blockCreated;
        QString description;
        QString logoHash;
    };
    QList<TokenData> m_tokenList;
    
    // NFT Data
    struct NFTData {
        QString id;
        QString name;
        QString description;
        QString creator;
        int blockCreated;
        QString owner;
        QString imageHash;
        QJsonObject attributes;
        QString metadata;
        // Extended marketplace fields
        QString txid;
        QString category;
        QJsonObject traits;
        qint64 listedPriceSatoshis {0};
        double listedPriceUSD {0.0};
        bool isListed {false};
    };
    QList<NFTData> m_nftList;

    bool metadataAdminVerified(const QString& metadata) const;
    
    // Context Menu
    QMenu* m_tokenContextMenu;
    QMenu* m_nftContextMenu;
    QString m_selectedTokenId;
    QString m_selectedNFTId;

    // Trademark protection methods
    void validateTrademarkCompliance(const QString& name);
    void showTrademarkWarning();
    bool isTrademarkViolation(const QString& name);
};

/**
 * Token Details Dialog
 * 
 * Shows detailed information about a specific token
 */
class TokenDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TokenDetailsDialog(const TokenNFTExplorer::TokenData& tokenData, QWidget* parent = nullptr);

private:
    void setupUI();
    void populateData();
    
    TokenNFTExplorer::TokenData m_tokenData;
    QVBoxLayout* m_layout;
    QLabel* m_logoLabel;
    QTextEdit* m_metadataText;
    QPushButton* m_exportButton;
    QPushButton* m_closeButton;
};

/**
 * NFT Details Dialog
 * 
 * Shows detailed information about a specific NFT
 */
class NFTDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NFTDetailsDialog(const TokenNFTExplorer::NFTData& nftData, QWidget* parent = nullptr);
    void setWalletModel(WalletModel* walletModel);

Q_SIGNALS:
    void purchaseCompleted(const QString& nftId, const QString& newOwner);

private Q_SLOTS:
    void onBuyClicked();
    void onTransferClicked();

private:
    void setupUI();
    void populateData();
    bool handleStripePayment(const TokenNFTExplorer::NFTData& nft, const QString& buyerAddress, double usdPrice);
    
    TokenNFTExplorer::NFTData m_nftData;
    WalletModel* m_walletModel;
    QVBoxLayout* m_layout;
    QLabel* m_imageLabel;
    QTextEdit* m_metadataText;
    QPushButton* m_buyButton;
    QPushButton* m_transferButton;
    QPushButton* m_closeButton;
};

/**
 * Bulk Operations Dialog
 * 
 * Handles bulk operations for creators (batch listing, pricing updates, delisting)
 */
class BulkOperationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BulkOperationsDialog(const QList<TokenNFTExplorer::NFTData>& ownedNFTs, QWidget* parent = nullptr);

Q_SIGNALS:
    void bulkOperationRequested(const TokenNFTExplorer::BulkOperation& operation);

private Q_SLOTS:
    void onOperationTypeChanged();
    void onSelectAllClicked();
    void onDeselectAllClicked();
    void onExecuteClicked();
    void onCancelClicked();
    void onPriceUpdateChanged();
    void onCategoryChanged();
    void onTagsChanged();
    void onVerificationChanged();

private:
    void setupUI();
    void updateSelectedNFTs();
    void updatePriceFields();
    void validateOperation();
    
    QList<TokenNFTExplorer::NFTData> m_ownedNFTs;
    QList<TokenNFTExplorer::NFTData> m_selectedNFTs;
    TokenNFTExplorer::BulkOperation m_currentOperation;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QComboBox* m_operationTypeCombo;
    QTableWidget* m_nftSelectionTable;
    QPushButton* m_selectAllButton;
    QPushButton* m_deselectAllButton;
    
    // Batch listing fields
    QGroupBox* m_listingGroup;
    QLineEdit* m_categoryEdit;
    QLineEdit* m_tagsEdit;
    QCheckBox* m_verificationCheck;
    
    // Price update fields
    QGroupBox* m_priceGroup;
    QLineEdit* m_pricePercentEdit;
    QLineEdit* m_priceFixedEdit;
    QComboBox* m_priceTypeCombo;
    QTableWidget* m_priceTable;
    
    // Common fields
    QPushButton* m_executeButton;
    QPushButton* m_cancelButton;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
};

/**
 * Advanced Filter Dialog
 * 
 * Provides advanced filtering and search capabilities for the marketplace
 */
class AdvancedFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdvancedFilterDialog(const TokenNFTExplorer::AdvancedFilters& currentFilters, QWidget* parent = nullptr);

Q_SIGNALS:
    void filtersApplied(const TokenNFTExplorer::AdvancedFilters& filters);
    void filterPresetSaved(const QString& name, const TokenNFTExplorer::AdvancedFilters& filters);

private Q_SLOTS:
    void onApplyClicked();
    void onResetClicked();
    void onSavePresetClicked();
    void onLoadPresetClicked();
    void onSearchTextChanged();
    void onTraitAdded();
    void onTraitRemoved();
    void onDateRangeChanged();
    void onSortChanged();

private:
    void setupUI();
    void populatePresets();
    void updateFilterDisplay();
    void validateFilters();
    
    TokenNFTExplorer::AdvancedFilters m_currentFilters;
    TokenNFTExplorer::AdvancedFilters m_originalFilters;
    QStringList m_availableTraits;
    QStringList m_availableCategories;
    QStringList m_availableCreators;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Search tab
    QWidget* m_searchTab;
    QLineEdit* m_searchTextEdit;
    QComboBox* m_searchTypeCombo;
    QListWidget* m_searchSuggestionsList;
    
    // Filters tab
    QWidget* m_filtersTab;
    QComboBox* m_categoryCombo;
    QComboBox* m_creatorCombo;
    QComboBox* m_tierCombo;
    QLineEdit* m_minPriceEdit;
    QLineEdit* m_maxPriceEdit;
    QCheckBox* m_verifiedOnlyCheck;
    QCheckBox* m_myNFTsOnlyCheck;
    QCheckBox* m_favoritedOnlyCheck;
    QCheckBox* m_hasUnlockableContentCheck;
    QCheckBox* m_isTradeLockedCheck;
    QDateEdit* m_minMintDateEdit;
    QDateEdit* m_maxMintDateEdit;
    
    // Traits tab
    QWidget* m_traitsTab;
    QListWidget* m_availableTraitsList;
    QListWidget* m_selectedTraitsList;
    QPushButton* m_addTraitButton;
    QPushButton* m_removeTraitButton;
    QLineEdit* m_newTraitEdit;
    QPushButton* m_addNewTraitButton;
    
    // Sort tab
    QWidget* m_sortTab;
    QComboBox* m_sortByCombo;
    QComboBox* m_sortOrderCombo;
    QCheckBox* m_randomizeCheck;
    
    // Presets tab
    QWidget* m_presetsTab;
    QListWidget* m_presetsList;
    QLineEdit* m_presetNameEdit;
    QPushButton* m_savePresetButton;
    QPushButton* m_loadPresetButton;
    QPushButton* m_deletePresetButton;
    
    // Action buttons
    QHBoxLayout* m_actionLayout;
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;
    QPushButton* m_cancelButton;
};



#endif // SHAHCOIN_QT_TOKENNFTEXPLORER_H
