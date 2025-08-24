#include <qt/advancedfilterdialog.h>
#include <qt/tokennftexplorer.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

AdvancedFilterDialog::AdvancedFilterDialog(const TokenNFTExplorer::AdvancedFilters& currentFilters, QWidget* parent)
    : QDialog(parent)
    , m_currentFilters(currentFilters)
    , m_originalFilters(currentFilters)
{
    setupUI();
    populatePresets();
    updateFilterDisplay();
    validateFilters();
}

void AdvancedFilterDialog::setupUI()
{
    setWindowTitle(tr("Advanced Filters"));
    setMinimumSize(700, 500);
    
    m_mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    
    // Search tab
    m_searchTab = new QWidget();
    QVBoxLayout* searchLayout = new QVBoxLayout(m_searchTab);
    
    m_searchTextEdit = new QLineEdit(this);
    m_searchTextEdit->setPlaceholderText(tr("Search NFTs, creators, traits..."));
    
    m_searchTypeCombo = new QComboBox(this);
    m_searchTypeCombo->addItems({
        tr("All"),
        tr("NFT Names"),
        tr("Creators"),
        tr("Categories"),
        tr("Traits")
    });
    
    m_searchSuggestionsList = new QListWidget(this);
    m_searchSuggestionsList->setMaximumHeight(150);
    
    searchLayout->addWidget(new QLabel(tr("Search:")));
    searchLayout->addWidget(m_searchTextEdit);
    searchLayout->addWidget(new QLabel(tr("Search Type:")));
    searchLayout->addWidget(m_searchTypeCombo);
    searchLayout->addWidget(new QLabel(tr("Suggestions:")));
    searchLayout->addWidget(m_searchSuggestionsList);
    searchLayout->addStretch();
    
    m_tabWidget->addTab(m_searchTab, tr("🔍 Search"));
    
    // Filters tab
    m_filtersTab = new QWidget();
    QGridLayout* filtersLayout = new QGridLayout(m_filtersTab);
    
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItems({ tr("All Categories"), tr("Art"), tr("Game"), tr("Music"), tr("Collectible"), tr("Photography"), tr("Video") });
    m_categoryCombo->setEditable(true);
    
    m_creatorCombo = new QComboBox(this);
    m_creatorCombo->setEditable(true);
    m_creatorCombo->setPlaceholderText(tr("Enter creator address"));
    
    m_tierCombo = new QComboBox(this);
    m_tierCombo->addItems({ tr("All Tiers"), tr("Basic"), tr("Pro"), tr("Premium") });
    
    m_minPriceEdit = new QLineEdit(this);
    m_minPriceEdit->setPlaceholderText(tr("Min price (SHAH)"));
    m_minPriceEdit->setValidator(new QDoubleValidator(0, 1000000, 8, this));
    
    m_maxPriceEdit = new QLineEdit(this);
    m_maxPriceEdit->setPlaceholderText(tr("Max price (SHAH)"));
    m_maxPriceEdit->setValidator(new QDoubleValidator(0, 1000000, 8, this));
    
    m_verifiedOnlyCheck = new QCheckBox(tr("Verified NFTs only"), this);
    m_myNFTsOnlyCheck = new QCheckBox(tr("My NFTs only"), this);
    m_favoritedOnlyCheck = new QCheckBox(tr("Favorited NFTs only"), this);
    m_hasUnlockableContentCheck = new QCheckBox(tr("Has unlockable content"), this);
    m_isTradeLockedCheck = new QCheckBox(tr("Trade locked"), this);
    
    m_minMintDateEdit = new QDateEdit(this);
    m_minMintDateEdit->setCalendarPopup(true);
    m_minMintDateEdit->setDate(QDate::currentDate().addDays(-30));
    
    m_maxMintDateEdit = new QDateEdit(this);
    m_maxMintDateEdit->setCalendarPopup(true);
    m_maxMintDateEdit->setDate(QDate::currentDate());
    
    // Add widgets to grid
    filtersLayout->addWidget(new QLabel(tr("Category:")), 0, 0);
    filtersLayout->addWidget(m_categoryCombo, 0, 1);
    filtersLayout->addWidget(new QLabel(tr("Creator:")), 1, 0);
    filtersLayout->addWidget(m_creatorCombo, 1, 1);
    filtersLayout->addWidget(new QLabel(tr("Tier:")), 2, 0);
    filtersLayout->addWidget(m_tierCombo, 2, 1);
    filtersLayout->addWidget(new QLabel(tr("Min Price:")), 3, 0);
    filtersLayout->addWidget(m_minPriceEdit, 3, 1);
    filtersLayout->addWidget(new QLabel(tr("Max Price:")), 4, 0);
    filtersLayout->addWidget(m_maxPriceEdit, 4, 1);
    filtersLayout->addWidget(m_verifiedOnlyCheck, 5, 0, 1, 2);
    filtersLayout->addWidget(m_myNFTsOnlyCheck, 6, 0, 1, 2);
    filtersLayout->addWidget(m_favoritedOnlyCheck, 7, 0, 1, 2);
    filtersLayout->addWidget(m_hasUnlockableContentCheck, 8, 0, 1, 2);
    filtersLayout->addWidget(m_isTradeLockedCheck, 9, 0, 1, 2);
    filtersLayout->addWidget(new QLabel(tr("Min Mint Date:")), 10, 0);
    filtersLayout->addWidget(m_minMintDateEdit, 10, 1);
    filtersLayout->addWidget(new QLabel(tr("Max Mint Date:")), 11, 0);
    filtersLayout->addWidget(m_maxMintDateEdit, 11, 1);
    
    m_tabWidget->addTab(m_filtersTab, tr("🎯 Filters"));
    
    // Traits tab
    m_traitsTab = new QWidget();
    QHBoxLayout* traitsLayout = new QHBoxLayout(m_traitsTab);
    
    QVBoxLayout* availableTraitsLayout = new QVBoxLayout();
    availableTraitsLayout->addWidget(new QLabel(tr("Available Traits:")));
    m_availableTraitsList = new QListWidget(this);
    availableTraitsLayout->addWidget(m_availableTraitsList);
    
    QVBoxLayout* traitButtonsLayout = new QVBoxLayout();
    m_addTraitButton = new QPushButton(tr(">>"), this);
    m_removeTraitButton = new QPushButton(tr("<<"), this);
    traitButtonsLayout->addStretch();
    traitButtonsLayout->addWidget(m_addTraitButton);
    traitButtonsLayout->addWidget(m_removeTraitButton);
    traitButtonsLayout->addStretch();
    
    QVBoxLayout* selectedTraitsLayout = new QVBoxLayout();
    selectedTraitsLayout->addWidget(new QLabel(tr("Selected Traits:")));
    m_selectedTraitsList = new QListWidget(this);
    selectedTraitsLayout->addWidget(m_selectedTraitsList);
    
    QHBoxLayout* newTraitLayout = new QHBoxLayout();
    m_newTraitEdit = new QLineEdit(this);
    m_newTraitEdit->setPlaceholderText(tr("Add new trait"));
    m_addNewTraitButton = new QPushButton(tr("Add"), this);
    newTraitLayout->addWidget(m_newTraitEdit);
    newTraitLayout->addWidget(m_addNewTraitButton);
    selectedTraitsLayout->addLayout(newTraitLayout);
    
    traitsLayout->addLayout(availableTraitsLayout);
    traitsLayout->addLayout(traitButtonsLayout);
    traitsLayout->addLayout(selectedTraitsLayout);
    
    m_tabWidget->addTab(m_traitsTab, tr("🏷 Traits"));
    
    // Sort tab
    m_sortTab = new QWidget();
    QGridLayout* sortLayout = new QGridLayout(m_sortTab);
    
    m_sortByCombo = new QComboBox(this);
    m_sortByCombo->addItems({
        tr("Price"),
        tr("Date"),
        tr("Rarity"),
        tr("Popularity"),
        tr("Name"),
        tr("Creator")
    });
    
    m_sortOrderCombo = new QComboBox(this);
    m_sortOrderCombo->addItems({ tr("Descending"), tr("Ascending") });
    
    m_randomizeCheck = new QCheckBox(tr("Randomize results"), this);
    
    sortLayout->addWidget(new QLabel(tr("Sort By:")), 0, 0);
    sortLayout->addWidget(m_sortByCombo, 0, 1);
    sortLayout->addWidget(new QLabel(tr("Order:")), 1, 0);
    sortLayout->addWidget(m_sortOrderCombo, 1, 1);
    sortLayout->addWidget(m_randomizeCheck, 2, 0, 1, 2);
    sortLayout->addStretch();
    
    m_tabWidget->addTab(m_sortTab, tr("📊 Sort"));
    
    // Presets tab
    m_presetsTab = new QWidget();
    QVBoxLayout* presetsLayout = new QVBoxLayout(m_presetsTab);
    
    m_presetsList = new QListWidget(this);
    presetsLayout->addWidget(new QLabel(tr("Saved Presets:")));
    presetsLayout->addWidget(m_presetsList);
    
    QHBoxLayout* presetNameLayout = new QHBoxLayout();
    m_presetNameEdit = new QLineEdit(this);
    m_presetNameEdit->setPlaceholderText(tr("Preset name"));
    presetNameLayout->addWidget(m_presetNameEdit);
    presetNameLayout->addStretch();
    
    QHBoxLayout* presetButtonsLayout = new QHBoxLayout();
    m_savePresetButton = new QPushButton(tr("Save Current"), this);
    m_loadPresetButton = new QPushButton(tr("Load Selected"), this);
    m_deletePresetButton = new QPushButton(tr("Delete Selected"), this);
    presetButtonsLayout->addWidget(m_savePresetButton);
    presetButtonsLayout->addWidget(m_loadPresetButton);
    presetButtonsLayout->addWidget(m_deletePresetButton);
    
    presetsLayout->addLayout(presetNameLayout);
    presetsLayout->addLayout(presetButtonsLayout);
    
    m_tabWidget->addTab(m_presetsTab, tr("💾 Presets"));
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Action buttons
    m_actionLayout = new QHBoxLayout();
    m_applyButton = new QPushButton(tr("Apply Filters"), this);
    m_resetButton = new QPushButton(tr("Reset"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    
    m_actionLayout->addWidget(m_applyButton);
    m_actionLayout->addWidget(m_resetButton);
    m_actionLayout->addStretch();
    m_actionLayout->addWidget(m_cancelButton);
    
    m_mainLayout->addLayout(m_actionLayout);
    
    // Connect signals
    connect(m_applyButton, &QPushButton::clicked, this, &AdvancedFilterDialog::onApplyClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &AdvancedFilterDialog::onResetClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    connect(m_savePresetButton, &QPushButton::clicked, this, &AdvancedFilterDialog::onSavePresetClicked);
    connect(m_loadPresetButton, &QPushButton::clicked, this, &AdvancedFilterDialog::onLoadPresetClicked);
    connect(m_deletePresetButton, &QPushButton::clicked, this, &AdvancedFilterDialog::onDeletePresetClicked);
    
    connect(m_addTraitButton, &QPushButton::clicked, this, &AdvancedFilterDialog::onTraitAdded);
    connect(m_removeTraitButton, &QPushButton::clicked, this, &AdvancedFilterDialog::onTraitRemoved);
    connect(m_addNewTraitButton, &QPushButton::clicked, this, &AdvancedFilterDialog::onTraitAdded);
    
    connect(m_searchTextEdit, &QLineEdit::textChanged, this, &AdvancedFilterDialog::onSearchTextChanged);
    connect(m_minMintDateEdit, &QDateEdit::dateChanged, this, &AdvancedFilterDialog::onDateRangeChanged);
    connect(m_maxMintDateEdit, &QDateEdit::dateChanged, this, &AdvancedFilterDialog::onDateRangeChanged);
    connect(m_sortByCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AdvancedFilterDialog::onSortChanged);
    connect(m_sortOrderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AdvancedFilterDialog::onSortChanged);
}

void AdvancedFilterDialog::populatePresets()
{
    QSettings settings("Shahcoin", "Wallet");
    settings.beginGroup("filterPresets");
    
    QStringList presets = settings.childKeys();
    m_presetsList->clear();
    
    for (const QString& preset : presets) {
        m_presetsList->addItem(preset);
    }
    
    settings.endGroup();
}

void AdvancedFilterDialog::updateFilterDisplay()
{
    // Update UI to reflect current filters
    m_searchTextEdit->setText(m_currentFilters.searchText);
    
    if (!m_currentFilters.category.isEmpty()) {
        int index = m_categoryCombo->findText(m_currentFilters.category);
        if (index >= 0) {
            m_categoryCombo->setCurrentIndex(index);
        } else {
            m_categoryCombo->setEditText(m_currentFilters.category);
        }
    }
    
    if (!m_currentFilters.creator.isEmpty()) {
        m_creatorCombo->setEditText(m_currentFilters.creator);
    }
    
    if (!m_currentFilters.tier.isEmpty()) {
        int index = m_tierCombo->findText(m_currentFilters.tier);
        if (index >= 0) {
            m_tierCombo->setCurrentIndex(index);
        }
    }
    
    if (m_currentFilters.minPriceSatoshis > 0) {
        m_minPriceEdit->setText(QString::number(m_currentFilters.minPriceSatoshis / 100000000.0, 'f', 8));
    }
    
    if (m_currentFilters.maxPriceSatoshis > 0) {
        m_maxPriceEdit->setText(QString::number(m_currentFilters.maxPriceSatoshis / 100000000.0, 'f', 8));
    }
    
    m_verifiedOnlyCheck->setChecked(m_currentFilters.verifiedOnly);
    m_myNFTsOnlyCheck->setChecked(m_currentFilters.myNFTsOnly);
    m_favoritedOnlyCheck->setChecked(m_currentFilters.favoritedOnly);
    m_hasUnlockableContentCheck->setChecked(m_currentFilters.hasUnlockableContent);
    m_isTradeLockedCheck->setChecked(m_currentFilters.isTradeLocked);
    
    if (m_currentFilters.minMintDate.isValid()) {
        m_minMintDateEdit->setDate(m_currentFilters.minMintDate.date());
    }
    
    if (m_currentFilters.maxMintDate.isValid()) {
        m_maxMintDateEdit->setDate(m_currentFilters.maxMintDate.date());
    }
    
    // Update traits lists
    m_availableTraitsList->clear();
    m_selectedTraitsList->clear();
    
    // Populate available traits (this would come from actual NFT data)
    QStringList allTraits = { "Rare", "Common", "Epic", "Legendary", "Limited", "Exclusive", "1/1", "Series" };
    for (const QString& trait : allTraits) {
        if (!m_currentFilters.traits.contains(trait)) {
            m_availableTraitsList->addItem(trait);
        }
    }
    
    // Populate selected traits
    for (const QString& trait : m_currentFilters.traits) {
        m_selectedTraitsList->addItem(trait);
    }
    
    // Update sort settings
    if (!m_currentFilters.sortBy.isEmpty()) {
        int index = m_sortByCombo->findText(m_currentFilters.sortBy, Qt::MatchFixedString);
        if (index >= 0) {
            m_sortByCombo->setCurrentIndex(index);
        }
    }
    
    if (!m_currentFilters.sortOrder.isEmpty()) {
        int index = m_sortOrderCombo->findText(m_currentFilters.sortOrder, Qt::MatchFixedString);
        if (index >= 0) {
            m_sortOrderCombo->setCurrentIndex(index);
        }
    }
}

void AdvancedFilterDialog::validateFilters()
{
    // Validate price range
    bool priceValid = true;
    if (!m_minPriceEdit->text().isEmpty() && !m_maxPriceEdit->text().isEmpty()) {
        double minPrice = m_minPriceEdit->text().toDouble();
        double maxPrice = m_maxPriceEdit->text().toDouble();
        if (minPrice > maxPrice) {
            priceValid = false;
        }
    }
    
    // Validate date range
    bool dateValid = true;
    if (m_minMintDateEdit->date() > m_maxMintDateEdit->date()) {
        dateValid = false;
    }
    
    // Update apply button state
    m_applyButton->setEnabled(priceValid && dateValid);
}

void AdvancedFilterDialog::onApplyClicked()
{
    // Update current filters from UI
    m_currentFilters.searchText = m_searchTextEdit->text();
    m_currentFilters.category = m_categoryCombo->currentText();
    m_currentFilters.creator = m_creatorCombo->currentText();
    m_currentFilters.tier = m_tierCombo->currentText();
    
    if (!m_minPriceEdit->text().isEmpty()) {
        m_currentFilters.minPriceSatoshis = m_minPriceEdit->text().toDouble() * 100000000;
    } else {
        m_currentFilters.minPriceSatoshis = 0;
    }
    
    if (!m_maxPriceEdit->text().isEmpty()) {
        m_currentFilters.maxPriceSatoshis = m_maxPriceEdit->text().toDouble() * 100000000;
    } else {
        m_currentFilters.maxPriceSatoshis = 0;
    }
    
    m_currentFilters.verifiedOnly = m_verifiedOnlyCheck->isChecked();
    m_currentFilters.myNFTsOnly = m_myNFTsOnlyCheck->isChecked();
    m_currentFilters.favoritedOnly = m_favoritedOnlyCheck->isChecked();
    m_currentFilters.hasUnlockableContent = m_hasUnlockableContentCheck->isChecked();
    m_currentFilters.isTradeLocked = m_isTradeLockedCheck->isChecked();
    
    m_currentFilters.minMintDate = QDateTime(m_minMintDateEdit->date());
    m_currentFilters.maxMintDate = QDateTime(m_maxMintDateEdit->date());
    
    // Update traits
    m_currentFilters.traits.clear();
    for (int i = 0; i < m_selectedTraitsList->count(); ++i) {
        m_currentFilters.traits.append(m_selectedTraitsList->item(i)->text());
    }
    
    // Update sort settings
    m_currentFilters.sortBy = m_sortByCombo->currentText().toLower();
    m_currentFilters.sortOrder = m_sortOrderCombo->currentText().toLower();
    
    // Emit signal
    emit filtersApplied(m_currentFilters);
    
    // Close dialog
    accept();
}

void AdvancedFilterDialog::onResetClicked()
{
    m_currentFilters = m_originalFilters;
    updateFilterDisplay();
    validateFilters();
}

void AdvancedFilterDialog::onSavePresetClicked()
{
    QString presetName = m_presetNameEdit->text().trimmed();
    if (presetName.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Name"), tr("Please enter a name for the preset."));
        return;
    }
    
    // Save current filters as preset
    emit filterPresetSaved(presetName, m_currentFilters);
    
    // Update presets list
    populatePresets();
    
    // Clear name field
    m_presetNameEdit->clear();
    
    QMessageBox::information(this, tr("Preset Saved"), tr("Filter preset '%1' has been saved.").arg(presetName));
}

void AdvancedFilterDialog::onLoadPresetClicked()
{
    QListWidgetItem* selectedItem = m_presetsList->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a preset to load."));
        return;
    }
    
    QString presetName = selectedItem->text();
    
    // Load preset (this would be handled by the parent)
    // For now, just show a message
    QMessageBox::information(this, tr("Load Preset"), tr("Loading preset '%1'...").arg(presetName));
}

void AdvancedFilterDialog::onDeletePresetClicked()
{
    QListWidgetItem* selectedItem = m_presetsList->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a preset to delete."));
        return;
    }
    
    QString presetName = selectedItem->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Delete Preset"),
        tr("Are you sure you want to delete the preset '%1'?").arg(presetName),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Delete preset from settings
        QSettings settings("Shahcoin", "Wallet");
        settings.beginGroup("filterPresets");
        settings.remove(presetName);
        settings.endGroup();
        
        // Update presets list
        populatePresets();
        
        QMessageBox::information(this, tr("Preset Deleted"), tr("Preset '%1' has been deleted.").arg(presetName));
    }
}

void AdvancedFilterDialog::onSearchTextChanged()
{
    // Update search suggestions (this would be implemented with actual data)
    m_searchSuggestionsList->clear();
    
    QString searchText = m_searchTextEdit->text();
    if (searchText.length() >= 2) {
        // Add some dummy suggestions
        QStringList suggestions = { "Art", "Game", "Music", "Rare", "Limited" };
        for (const QString& suggestion : suggestions) {
            if (suggestion.contains(searchText, Qt::CaseInsensitive)) {
                m_searchSuggestionsList->addItem(suggestion);
            }
        }
    }
}

void AdvancedFilterDialog::onTraitAdded()
{
    // Add trait from available to selected
    QListWidgetItem* selectedItem = m_availableTraitsList->currentItem();
    if (selectedItem) {
        QString trait = selectedItem->text();
        m_selectedTraitsList->addItem(trait);
        delete m_availableTraitsList->takeItem(m_availableTraitsList->row(selectedItem));
    } else {
        // Add new trait from text field
        QString newTrait = m_newTraitEdit->text().trimmed();
        if (!newTrait.isEmpty()) {
            m_selectedTraitsList->addItem(newTrait);
            m_newTraitEdit->clear();
        }
    }
}

void AdvancedFilterDialog::onTraitRemoved()
{
    // Remove trait from selected to available
    QListWidgetItem* selectedItem = m_selectedTraitsList->currentItem();
    if (selectedItem) {
        QString trait = selectedItem->text();
        m_availableTraitsList->addItem(trait);
        delete m_selectedTraitsList->takeItem(m_selectedTraitsList->row(selectedItem));
    }
}

void AdvancedFilterDialog::onDateRangeChanged()
{
    validateFilters();
}

void AdvancedFilterDialog::onSortChanged()
{
    validateFilters();
}
