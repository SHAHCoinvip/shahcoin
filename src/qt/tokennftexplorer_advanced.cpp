#include <qt/tokennftexplorer.h>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

// Advanced Filtering Implementation

void TokenNFTExplorer::onAdvancedFilterClicked()
{
    showAdvancedFilterDialog();
}

void TokenNFTExplorer::onFilterPresetClicked()
{
    // Show filter preset dialog
    QStringList presets;
    m_settings.beginGroup("filter_presets");
    QStringList keys = m_settings.allKeys();
    for (const QString& key : keys) {
        presets << key;
    }
    m_settings.endGroup();
    
    if (presets.isEmpty()) {
        QMessageBox::information(this, tr("Filter Presets"), tr("No saved filter presets found."));
        return;
    }
    
    bool ok;
    QString selectedPreset = QInputDialog::getItem(this, tr("Load Filter Preset"),
        tr("Select a preset to load:"), presets, 0, false, &ok);
    
    if (ok && !selectedPreset.isEmpty()) {
        loadFilterPreset(selectedPreset);
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
    if (query.length() >= 2) {
        updateSearchSuggestions(query);
        showSearchSuggestions();
    } else {
        m_searchSuggestionsList->hide();
    }
    
    applyAdvancedFilters();
}

void TokenNFTExplorer::onSearchSuggestionSelected(QListWidgetItem* item)
{
    if (!item) return;
    
    SearchSuggestion suggestion;
    suggestion.text = item->text();
    suggestion.type = item->data(Qt::UserRole).toString();
    suggestion.relevance = 1.0;
    
    applySearchSuggestion(suggestion);
    m_searchSuggestionsList->hide();
}

void TokenNFTExplorer::onSortByChanged()
{
    m_currentFilters.sortBy = m_sortByCombo->currentText().toLower();
    applyAdvancedFilters();
}

void TokenNFTExplorer::onSortOrderChanged()
{
    m_currentFilters.sortOrder = m_sortOrderCombo->currentText().toLower();
    applyAdvancedFilters();
}

void TokenNFTExplorer::updateSearchSuggestions(const QString& query)
{
    m_searchSuggestions.clear();
    
    if (query.length() < 2) return;
    
    QString lowerQuery = query.toLower();
    
    // Search in NFT names
    for (const NFTData& nft : m_nftList) {
        if (nft.name.toLower().contains(lowerQuery)) {
            SearchSuggestion suggestion;
            suggestion.text = nft.name;
            suggestion.type = "nft";
            suggestion.relevance = 1.0;
            m_searchSuggestions.append(suggestion);
        }
    }
    
    // Search in creators
    for (const QString& creator : m_creatorProfiles.keys()) {
        if (creator.toLower().contains(lowerQuery)) {
            SearchSuggestion suggestion;
            suggestion.text = creator;
            suggestion.type = "creator";
            suggestion.relevance = 0.8;
            m_searchSuggestions.append(suggestion);
        }
    }
    
    // Search in categories
    QStringList categories = {"art", "gaming", "music", "collectibles", "virtual_worlds"};
    for (const QString& category : categories) {
        if (category.toLower().contains(lowerQuery)) {
            SearchSuggestion suggestion;
            suggestion.text = category;
            suggestion.type = "category";
            suggestion.relevance = 0.6;
            m_searchSuggestions.append(suggestion);
        }
    }
    
    // Sort by relevance and limit results
    std::sort(m_searchSuggestions.begin(), m_searchSuggestions.end(),
        [](const SearchSuggestion& a, const SearchSuggestion& b) {
            return a.relevance > b.relevance;
        });
    
    if (m_searchSuggestions.size() > 10) {
        m_searchSuggestions = m_searchSuggestions.mid(0, 10);
    }
}

void TokenNFTExplorer::showSearchSuggestions()
{
    m_searchSuggestionsList->clear();
    
    for (const SearchSuggestion& suggestion : m_searchSuggestions) {
        QListWidgetItem* item = new QListWidgetItem(suggestion.text);
        item->setData(Qt::UserRole, suggestion.type);
        m_searchSuggestionsList->addItem(item);
    }
    
    if (!m_searchSuggestions.isEmpty()) {
        m_searchSuggestionsList->show();
    }
}

void TokenNFTExplorer::applySearchSuggestion(const SearchSuggestion& suggestion)
{
    m_searchBox->setText(suggestion.text);
    applyAdvancedFilters();
}

void TokenNFTExplorer::applyAdvancedFilters()
{
    // Apply current filters to NFT list
    QList<NFTData> filteredNFTs;
    
    for (const NFTData& nft : m_nftList) {
        bool include = true;
        
        // Search filter
        if (!m_currentFilters.searchText.isEmpty()) {
            QString searchText = m_currentFilters.searchText.toLower();
            if (!nft.name.toLower().contains(searchText) &&
                !nft.description.toLower().contains(searchText) &&
                !nft.creator.toLower().contains(searchText) &&
                !nft.category.toLower().contains(searchText)) {
                include = false;
            }
        }
        
        // Category filter
        if (!m_currentFilters.category.isEmpty() && nft.category != m_currentFilters.category) {
            include = false;
        }
        
        // Creator filter
        if (!m_currentFilters.creator.isEmpty() && nft.creator != m_currentFilters.creator) {
            include = false;
        }
        
        // Price range filter
        if (m_currentFilters.minPriceSatoshis > 0 && nft.listedPriceSatoshis < m_currentFilters.minPriceSatoshis) {
            include = false;
        }
        if (m_currentFilters.maxPriceSatoshis > 0 && nft.listedPriceSatoshis > m_currentFilters.maxPriceSatoshis) {
            include = false;
        }
        
        // Verification filter
        if (m_currentFilters.verifiedOnly && !isVerifiedNFT(nft)) {
            include = false;
        }
        
        // My NFTs filter
        if (m_currentFilters.myNFTsOnly && m_walletModel && nft.owner != m_walletModel->getWalletAddress()) {
            include = false;
        }
        
        // Favorited filter
        if (m_currentFilters.favoritedOnly && !m_localFavorites.contains(nft.id)) {
            include = false;
        }
        
        if (include) {
            filteredNFTs.append(nft);
        }
    }
    
    // Sort filtered results
    std::sort(filteredNFTs.begin(), filteredNFTs.end(),
        [this](const NFTData& a, const NFTData& b) {
            bool ascending = m_currentFilters.sortOrder == "asc";
            
            if (m_currentFilters.sortBy == "price") {
                return ascending ? a.listedPriceSatoshis < b.listedPriceSatoshis : a.listedPriceSatoshis > b.listedPriceSatoshis;
            } else if (m_currentFilters.sortBy == "name") {
                return ascending ? a.name < b.name : a.name > b.name;
            } else if (m_currentFilters.sortBy == "date") {
                return ascending ? a.blockCreated < b.blockCreated : a.blockCreated > b.blockCreated;
            } else if (m_currentFilters.sortBy == "creator") {
                return ascending ? a.creator < b.creator : a.creator > b.creator;
            }
            
            return false;
        });
    
    // Update table with filtered data
    populateMarketplaceTableWithData(filteredNFTs);
}

void TokenNFTExplorer::resetFilters()
{
    m_currentFilters = AdvancedFilters();
    updateFilterUI();
    applyAdvancedFilters();
}

void TokenNFTExplorer::saveFilterPreset(const QString& name)
{
    m_settings.beginGroup("filter_presets");
    m_settings.setValue(name, QJsonDocument(QJsonObject::fromVariantMap({
        {"searchText", m_currentFilters.searchText},
        {"category", m_currentFilters.category},
        {"creator", m_currentFilters.creator},
        {"minPriceSatoshis", m_currentFilters.minPriceSatoshis},
        {"maxPriceSatoshis", m_currentFilters.maxPriceSatoshis},
        {"verifiedOnly", m_currentFilters.verifiedOnly},
        {"myNFTsOnly", m_currentFilters.myNFTsOnly},
        {"favoritedOnly", m_currentFilters.favoritedOnly},
        {"sortBy", m_currentFilters.sortBy},
        {"sortOrder", m_currentFilters.sortOrder}
    })).toJson());
    m_settings.endGroup();
}

void TokenNFTExplorer::loadFilterPreset(const QString& name)
{
    m_settings.beginGroup("filter_presets");
    QJsonDocument doc = QJsonDocument::fromJson(m_settings.value(name).toByteArray());
    m_settings.endGroup();
    
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        m_currentFilters.searchText = obj["searchText"].toString();
        m_currentFilters.category = obj["category"].toString();
        m_currentFilters.creator = obj["creator"].toString();
        m_currentFilters.minPriceSatoshis = obj["minPriceSatoshis"].toVariant().toLongLong();
        m_currentFilters.maxPriceSatoshis = obj["maxPriceSatoshis"].toVariant().toLongLong();
        m_currentFilters.verifiedOnly = obj["verifiedOnly"].toBool();
        m_currentFilters.myNFTsOnly = obj["myNFTsOnly"].toBool();
        m_currentFilters.favoritedOnly = obj["favoritedOnly"].toBool();
        m_currentFilters.sortBy = obj["sortBy"].toString();
        m_currentFilters.sortOrder = obj["sortOrder"].toString();
        
        updateFilterUI();
        applyAdvancedFilters();
    }
}

void TokenNFTExplorer::updateFilterUI()
{
    // Update UI elements to reflect current filters
    m_searchBox->setText(m_currentFilters.searchText);
    m_sortByCombo->setCurrentText(m_currentFilters.sortBy);
    m_sortOrderCombo->setCurrentText(m_currentFilters.sortOrder);
    // Add more UI updates as needed
}

// Bulk Operations Implementation

void TokenNFTExplorer::onBulkOperationsClicked()
{
    showBulkOperationsDialog();
}

void TokenNFTExplorer::showBulkOperationsDialog()
{
    // Get owned NFTs for bulk operations
    QList<NFTData> ownedNFTs;
    if (m_walletModel) {
        QString walletAddress = m_walletModel->getWalletAddress();
        for (const NFTData& nft : m_nftList) {
            if (nft.owner == walletAddress) {
                ownedNFTs.append(nft);
            }
        }
    }
    
    if (ownedNFTs.isEmpty()) {
        QMessageBox::information(this, tr("Bulk Operations"), tr("You don't have any NFTs to perform bulk operations on."));
        return;
    }
    
    BulkOperationsDialog* dialog = new BulkOperationsDialog(ownedNFTs, this);
    connect(dialog, &BulkOperationsDialog::bulkOperationRequested, this, &TokenNFTExplorer::executeBulkOperation);
    dialog->show();
}

void TokenNFTExplorer::executeBulkOperation(const BulkOperation& operation)
{
    QProgressDialog progress(tr("Executing bulk operation..."), tr("Cancel"), 0, operation.nftIds.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    
    int completed = 0;
    for (const QString& nftId : operation.nftIds) {
        if (progress.wasCanceled()) break;
        
        progress.setValue(completed);
        progress.setLabelText(tr("Processing NFT: %1").arg(nftId));
        
        if (operation.operationType == "batch_list") {
            // Create listing transaction
            QString txHex;
            qint64 price = operation.newPrices.value(nftId, 0);
            if (createNFTListingTransaction(nftId, price, txHex)) {
                QString txid;
                if (signAndBroadcastTransaction(txHex, txid)) {
                    // Update local state
                    for (NFTData& nft : m_nftList) {
                        if (nft.id == nftId) {
                            nft.isListed = true;
                            nft.listedPriceSatoshis = price;
                            nft.category = operation.category;
                            break;
                        }
                    }
                    
                    // Record to API
                    submitListingToApi(nftId, "", "", operation.category, operation.tags, price, 0.0, operation.submitForVerification);
                }
            }
        } else if (operation.operationType == "batch_price_update") {
            // Update price
            qint64 newPrice = operation.newPrices.value(nftId, 0);
            if (newPrice > 0) {
                for (NFTData& nft : m_nftList) {
                    if (nft.id == nftId) {
                        nft.listedPriceSatoshis = newPrice;
                        break;
                    }
                }
            }
        } else if (operation.operationType == "batch_delist") {
            // Remove from marketplace
            for (NFTData& nft : m_nftList) {
                if (nft.id == nftId) {
                    nft.isListed = false;
                    nft.listedPriceSatoshis = 0;
                    nft.listedPriceUSD = 0.0;
                    break;
                }
            }
        }
        
        completed++;
    }
    
    progress.setValue(operation.nftIds.size());
    
    // Refresh marketplace
    populateMarketplaceTable();
    
    QMessageBox::information(this, tr("Bulk Operation Complete"), 
        tr("Successfully processed %1 NFTs").arg(completed));
}

// Mobile Responsive Implementation

void TokenNFTExplorer::onToggleViewClicked()
{
    toggleMobileView();
}

void TokenNFTExplorer::toggleMobileView()
{
    m_isMobileView = !m_isMobileView;
    
    if (m_isMobileView) {
        setupMobileLayout();
        m_toggleViewButton->setText(tr("🖥️ Desktop View"));
    } else {
        setupDesktopLayout();
        m_toggleViewButton->setText(tr("📱 Mobile View"));
    }
}

void TokenNFTExplorer::setupMobileLayout()
{
    // Hide desktop controls
    m_marketControlsLayout->setVisible(false);
    
    // Show mobile layout
    if (!m_mobileView) {
        m_mobileView = new QWidget();
        m_mobileNFTList = new QListWidget();
        m_mobileFilterLayout = new QVBoxLayout();
        
        // Add mobile-specific controls
        QPushButton* mobileSearchButton = new QPushButton(tr("🔍 Search"));
        QPushButton* mobileFilterButton = new QPushButton(tr("⚙️ Filters"));
        
        m_mobileFilterLayout->addWidget(mobileSearchButton);
        m_mobileFilterLayout->addWidget(mobileFilterButton);
        m_mobileFilterLayout->addWidget(m_mobileNFTList);
        
        m_mobileView->setLayout(m_mobileFilterLayout);
    }
    
    // Switch to mobile view
    if (m_mobileStackedWidget) {
        m_mobileStackedWidget->setCurrentWidget(m_mobileView);
    }
}

void TokenNFTExplorer::setupDesktopLayout()
{
    // Show desktop controls
    m_marketControlsLayout->setVisible(true);
    
    // Switch to desktop view
    if (m_mobileStackedWidget) {
        m_mobileStackedWidget->setCurrentWidget(m_desktopView);
    }
}

void TokenNFTExplorer::onMobileNFTClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    QString nftId = item->data(Qt::UserRole).toString();
    if (!nftId.isEmpty()) {
        // Find NFT and show details
        for (const NFTData& nft : m_nftList) {
            if (nft.id == nftId) {
                showNFTDetailsDialog(nft.id);
                break;
            }
        }
    }
}

void TokenNFTExplorer::onMobileFilterChanged()
{
    // Apply filters to mobile view
    applyAdvancedFilters();
}

// Performance Optimization Implementation

void TokenNFTExplorer::setupVirtualScrolling()
{
    // Initialize virtual scrolling for large datasets
    m_visibleItemCount = 50;
    m_totalItemCount = m_nftList.size();
    
    if (m_scrollTimer) {
        m_scrollTimer->stop();
    }
    
    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setInterval(100);
    connect(m_scrollTimer, &QTimer::timeout, this, &TokenNFTExplorer::onScrollTimerTimeout);
}

void TokenNFTExplorer::loadVisibleItemsOnly()
{
    // Load only visible items for performance
    if (m_marketTable) {
        int visibleRows = m_marketTable->viewport()->height() / m_marketTable->rowHeight(0);
        int startRow = m_marketTable->verticalScrollBar()->value() / m_marketTable->rowHeight(0);
        
        // Load items from startRow to startRow + visibleRows
        // This is a simplified implementation
    }
}

void TokenNFTExplorer::preloadAdjacentItems()
{
    // Preload items just outside the visible area
    // This is a simplified implementation
}

void TokenNFTExplorer::onScrollTimerTimeout()
{
    // Handle scroll events for virtual scrolling
    loadVisibleItemsOnly();
    preloadAdjacentItems();
}

void TokenNFTExplorer::onVirtualScrollValueChanged()
{
    // Handle scroll bar value changes
    if (m_scrollTimer) {
        m_scrollTimer->start();
    }
}
