#ifndef SHAHCOIN_QT_ADVANCEDFILTERDIALOG_H
#define SHAHCOIN_QT_ADVANCEDFILTERDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QDateTime>

// Forward declarations
class TokenNFTExplorer;

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
    void onDeletePresetClicked();
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

#endif // SHAHCOIN_QT_ADVANCEDFILTERDIALOG_H
