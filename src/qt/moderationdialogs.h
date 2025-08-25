// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_MODERATIONDIALOGS_H
#define SHAHCOIN_QT_MODERATIONDIALOGS_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QTableWidget>
#include <QComboBox>

// Forward declarations
class TokenNFTExplorer;

/**
 * Moderation Settings Dialog
 * 
 * Allows administrators to configure moderation settings and thresholds
 */
class ModerationSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModerationSettingsDialog(const TokenNFTExplorer::ModerationSettings& settings, QWidget* parent = nullptr);

Q_SIGNALS:
    void settingsUpdated(const TokenNFTExplorer::ModerationSettings& settings);

private Q_SLOTS:
    void onSaveClicked();
    void onCancelClicked();
    void onResetToDefaultsClicked();
    void onAddTrustedModeratorClicked();
    void onRemoveTrustedModeratorClicked();

private:
    void setupUI();
    void populateSettings();
    void validateSettings();
    
    TokenNFTExplorer::ModerationSettings m_currentSettings;
    TokenNFTExplorer::ModerationSettings m_originalSettings;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // General settings tab
    QWidget* m_generalTab;
    QSpinBox* m_autoHideThresholdSpin;
    QCheckBox* m_autoHideEnabledCheck;
    QSpinBox* m_autoReviewThresholdSpin;
    QCheckBox* m_autoReviewEnabledCheck;
    QCheckBox* m_requireModeratorApprovalCheck;
    
    // Trusted moderators tab
    QWidget* m_moderatorsTab;
    QListWidget* m_trustedModeratorsList;
    QLineEdit* m_newModeratorEdit;
    QPushButton* m_addModeratorButton;
    QPushButton* m_removeModeratorButton;
    
    // Action buttons
    QHBoxLayout* m_actionLayout;
    QPushButton* m_saveButton;
    QPushButton* m_resetButton;
    QPushButton* m_cancelButton;
};

/**
 * Flagged NFT Details Dialog
 * 
 * Shows detailed information about a flagged NFT and allows moderation actions
 */
class FlaggedNFTDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FlaggedNFTDetailsDialog(const TokenNFTExplorer::FlaggedNFT& flaggedNFT, 
                                    const TokenNFTExplorer::NFTData& nftData,
                                    QWidget* parent = nullptr);

Q_SIGNALS:
    void moderationActionRequested(const QString& nftId, const QString& action, const QString& reason, const QString& notes);

private Q_SLOTS:
    void onApproveClicked();
    void onHideClicked();
    void onRemoveClicked();
    void onBlacklistCreatorClicked();
    void onMarkAsSafeClicked();
    void onViewNFTDetailsClicked();
    void onViewCreatorProfileClicked();

private:
    void setupUI();
    void populateData();
    void updateActionButtons();
    
    TokenNFTExplorer::FlaggedNFT m_flaggedNFT;
    TokenNFTExplorer::NFTData m_nftData;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Overview tab
    QWidget* m_overviewTab;
    QLabel* m_nftImageLabel;
    QLabel* m_nftNameLabel;
    QLabel* m_creatorLabel;
    QLabel* m_reportCountLabel;
    QLabel* m_statusLabel;
    QTextEdit* m_reasonText;
    QTextEdit* m_moderatorNotesText;
    
    // Reports tab
    QWidget* m_reportsTab;
    QTableWidget* m_reportsTable;
    
    // Actions tab
    QWidget* m_actionsTab;
    QComboBox* m_actionCombo;
    QTextEdit* m_actionReasonText;
    QTextEdit* m_actionNotesText;
    QPushButton* m_executeActionButton;
    
    // Action buttons
    QHBoxLayout* m_actionLayout;
    QPushButton* m_approveButton;
    QPushButton* m_hideButton;
    QPushButton* m_removeButton;
    QPushButton* m_blacklistButton;
    QPushButton* m_safeButton;
    QPushButton* m_closeButton;
};

/**
 * Moderation History Dialog
 * 
 * Shows the complete moderation history for an NFT or creator
 */
class ModerationHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModerationHistoryDialog(const QList<TokenNFTExplorer::ModerationAction>& actions, QWidget* parent = nullptr);

private Q_SLOTS:
    void onExportClicked();
    void onFilterChanged();

private:
    void setupUI();
    void populateActions();
    void applyFilter();
    
    QList<TokenNFTExplorer::ModerationAction> m_allActions;
    QList<TokenNFTExplorer::ModerationAction> m_filteredActions;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_controlsLayout;
    QComboBox* m_filterCombo;
    QPushButton* m_exportButton;
    QTableWidget* m_actionsTable;
    QPushButton* m_closeButton;
};

#endif // SHAHCOIN_QT_MODERATIONDIALOGS_H
