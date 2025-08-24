// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "tokennftexplorer.h"
#include "moderationdialogs.h"
#include "walletmodel.h"
#include "platformstyle.h"
#include "guiutil.h"
#include "util.h"

#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTimer>
#include <QHeaderView>
#include <QGroupBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QTabWidget>
#include <QListWidget>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>

// ModerationSettingsDialog Implementation
ModerationSettingsDialog::ModerationSettingsDialog(const TokenNFTExplorer::ModerationSettings& settings, QWidget* parent)
    : QDialog(parent)
    , m_currentSettings(settings)
    , m_originalSettings(settings)
{
    setWindowTitle(tr("Moderation Settings"));
    setModal(true);
    resize(600, 500);
    
    setupUI();
    populateSettings();
}

void ModerationSettingsDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget(this);
    
    // General settings tab
    m_generalTab = new QWidget();
    QVBoxLayout* generalLayout = new QVBoxLayout(m_generalTab);
    
    QGroupBox* autoHideGroup = new QGroupBox(tr("Auto-Hide Settings"), this);
    QFormLayout* autoHideLayout = new QFormLayout(autoHideGroup);
    
    m_autoHideEnabledCheck = new QCheckBox(tr("Enable auto-hide"), this);
    m_autoHideThresholdSpin = new QSpinBox(this);
    m_autoHideThresholdSpin->setRange(1, 100);
    m_autoHideThresholdSpin->setSuffix(tr(" reports"));
    m_autoHideThresholdSpin->setToolTip(tr("Number of reports before automatically hiding an NFT"));
    
    autoHideLayout->addRow(m_autoHideEnabledCheck);
    autoHideLayout->addRow(tr("Auto-hide threshold:"), m_autoHideThresholdSpin);
    
    QGroupBox* autoReviewGroup = new QGroupBox(tr("Auto-Review Settings"), this);
    QFormLayout* autoReviewLayout = new QFormLayout(autoReviewGroup);
    
    m_autoReviewEnabledCheck = new QCheckBox(tr("Enable auto-review"), this);
    m_autoReviewThresholdSpin = new QSpinBox(this);
    m_autoReviewThresholdSpin->setRange(1, 50);
    m_autoReviewThresholdSpin->setSuffix(tr(" reports"));
    m_autoReviewThresholdSpin->setToolTip(tr("Number of reports before requiring manual review"));
    
    autoReviewLayout->addRow(m_autoReviewEnabledCheck);
    autoReviewLayout->addRow(tr("Auto-review threshold:"), m_autoReviewThresholdSpin);
    
    QGroupBox* approvalGroup = new QGroupBox(tr("Approval Settings"), this);
    QFormLayout* approvalLayout = new QFormLayout(approvalGroup);
    
    m_requireModeratorApprovalCheck = new QCheckBox(tr("Require moderator approval"), this);
    m_requireModeratorApprovalCheck->setToolTip(tr("Require approval from trusted moderators for certain actions"));
    
    approvalLayout->addRow(m_requireModeratorApprovalCheck);
    
    generalLayout->addWidget(autoHideGroup);
    generalLayout->addWidget(autoReviewGroup);
    generalLayout->addWidget(approvalGroup);
    generalLayout->addStretch();
    
    // Trusted moderators tab
    m_moderatorsTab = new QWidget();
    QVBoxLayout* moderatorsLayout = new QVBoxLayout(m_moderatorsTab);
    
    QGroupBox* moderatorsGroup = new QGroupBox(tr("Trusted Moderators"), this);
    QVBoxLayout* moderatorsGroupLayout = new QVBoxLayout(moderatorsGroup);
    
    m_trustedModeratorsList = new QListWidget(this);
    m_trustedModeratorsList->setToolTip(tr("List of trusted moderator addresses"));
    
    QHBoxLayout* addModeratorLayout = new QHBoxLayout();
    m_newModeratorEdit = new QLineEdit(this);
    m_newModeratorEdit->setPlaceholderText(tr("Enter moderator address"));
    m_addModeratorButton = new QPushButton(tr("Add"), this);
    m_removeModeratorButton = new QPushButton(tr("Remove"), this);
    
    addModeratorLayout->addWidget(m_newModeratorEdit);
    addModeratorLayout->addWidget(m_addModeratorButton);
    addModeratorLayout->addWidget(m_removeModeratorButton);
    
    moderatorsGroupLayout->addWidget(m_trustedModeratorsList);
    moderatorsGroupLayout->addLayout(addModeratorLayout);
    
    moderatorsLayout->addWidget(moderatorsGroup);
    moderatorsLayout->addStretch();
    
    // Add tabs
    m_tabWidget->addTab(m_generalTab, tr("General"));
    m_tabWidget->addTab(m_moderatorsTab, tr("Trusted Moderators"));
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Action buttons
    m_actionLayout = new QHBoxLayout();
    m_saveButton = new QPushButton(tr("Save"), this);
    m_resetButton = new QPushButton(tr("Reset to Defaults"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    
    m_actionLayout->addStretch();
    m_actionLayout->addWidget(m_resetButton);
    m_actionLayout->addWidget(m_cancelButton);
    m_actionLayout->addWidget(m_saveButton);
    
    m_mainLayout->addLayout(m_actionLayout);
    
    // Connect signals
    connect(m_saveButton, &QPushButton::clicked, this, &ModerationSettingsDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ModerationSettingsDialog::onCancelClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &ModerationSettingsDialog::onResetToDefaultsClicked);
    connect(m_addModeratorButton, &QPushButton::clicked, this, &ModerationSettingsDialog::onAddTrustedModeratorClicked);
    connect(m_removeModeratorButton, &QPushButton::clicked, this, &ModerationSettingsDialog::onRemoveTrustedModeratorClicked);
    
    connect(m_autoHideEnabledCheck, &QCheckBox::toggled, m_autoHideThresholdSpin, &QSpinBox::setEnabled);
    connect(m_autoReviewEnabledCheck, &QCheckBox::toggled, m_autoReviewThresholdSpin, &QSpinBox::setEnabled);
}

void ModerationSettingsDialog::populateSettings()
{
    m_autoHideEnabledCheck->setChecked(m_currentSettings.autoHideEnabled);
    m_autoHideThresholdSpin->setValue(m_currentSettings.autoHideThreshold);
    m_autoHideThresholdSpin->setEnabled(m_currentSettings.autoHideEnabled);
    
    m_autoReviewEnabledCheck->setChecked(m_currentSettings.autoReviewEnabled);
    m_autoReviewThresholdSpin->setValue(m_currentSettings.autoReviewThreshold);
    m_autoReviewThresholdSpin->setEnabled(m_currentSettings.autoReviewEnabled);
    
    m_requireModeratorApprovalCheck->setChecked(m_currentSettings.requireModeratorApproval);
    
    m_trustedModeratorsList->clear();
    for (const QString& moderator : m_currentSettings.trustedModerators) {
        m_trustedModeratorsList->addItem(moderator);
    }
}

void ModerationSettingsDialog::validateSettings()
{
    bool valid = true;
    
    if (m_autoHideEnabledCheck->isChecked() && m_autoHideThresholdSpin->value() < 1) {
        valid = false;
    }
    
    if (m_autoReviewEnabledCheck->isChecked() && m_autoReviewThresholdSpin->value() < 1) {
        valid = false;
    }
    
    m_saveButton->setEnabled(valid);
}

void ModerationSettingsDialog::onSaveClicked()
{
    // Update current settings
    m_currentSettings.autoHideEnabled = m_autoHideEnabledCheck->isChecked();
    m_currentSettings.autoHideThreshold = m_autoHideThresholdSpin->value();
    m_currentSettings.autoReviewEnabled = m_autoReviewEnabledCheck->isChecked();
    m_currentSettings.autoReviewThreshold = m_autoReviewThresholdSpin->value();
    m_currentSettings.requireModeratorApproval = m_requireModeratorApprovalCheck->isChecked();
    
    m_currentSettings.trustedModerators.clear();
    for (int i = 0; i < m_trustedModeratorsList->count(); ++i) {
        m_currentSettings.trustedModerators.append(m_trustedModeratorsList->item(i)->text());
    }
    
    emit settingsUpdated(m_currentSettings);
    accept();
}

void ModerationSettingsDialog::onCancelClicked()
{
    reject();
}

void ModerationSettingsDialog::onResetToDefaultsClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Reset Settings"), 
                                                           tr("Are you sure you want to reset all settings to defaults?"),
                                                           QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_currentSettings.autoHideThreshold = 5;
        m_currentSettings.autoHideEnabled = true;
        m_currentSettings.autoReviewThreshold = 3;
        m_currentSettings.autoReviewEnabled = true;
        m_currentSettings.requireModeratorApproval = false;
        m_currentSettings.trustedModerators.clear();
        
        populateSettings();
    }
}

void ModerationSettingsDialog::onAddTrustedModeratorClicked()
{
    QString address = m_newModeratorEdit->text().trimmed();
    if (address.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter a moderator address"));
        return;
    }
    
    // Basic address validation
    if (!address.startsWith("S") || address.length() < 10) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter a valid Shahcoin address"));
        return;
    }
    
    // Check if already exists
    for (int i = 0; i < m_trustedModeratorsList->count(); ++i) {
        if (m_trustedModeratorsList->item(i)->text() == address) {
            QMessageBox::warning(this, tr("Error"), tr("This address is already in the list"));
            return;
        }
    }
    
    m_trustedModeratorsList->addItem(address);
    m_newModeratorEdit->clear();
}

void ModerationSettingsDialog::onRemoveTrustedModeratorClicked()
{
    QListWidgetItem* currentItem = m_trustedModeratorsList->currentItem();
    if (currentItem) {
        delete m_trustedModeratorsList->takeItem(m_trustedModeratorsList->row(currentItem));
    }
}

// FlaggedNFTDetailsDialog Implementation
FlaggedNFTDetailsDialog::FlaggedNFTDetailsDialog(const TokenNFTExplorer::FlaggedNFT& flaggedNFT, 
                                                 const TokenNFTExplorer::NFTData& nftData,
                                                 QWidget* parent)
    : QDialog(parent)
    , m_flaggedNFT(flaggedNFT)
    , m_nftData(nftData)
{
    setWindowTitle(tr("Flagged NFT Details - %1").arg(flaggedNFT.nftName));
    setModal(true);
    resize(800, 600);
    
    setupUI();
    populateData();
}

void FlaggedNFTDetailsDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget(this);
    
    // Overview tab
    m_overviewTab = new QWidget();
    QVBoxLayout* overviewLayout = new QVBoxLayout(m_overviewTab);
    
    // NFT image and basic info
    QHBoxLayout* imageLayout = new QHBoxLayout();
    m_nftImageLabel = new QLabel(this);
    m_nftImageLabel->setFixedSize(200, 200);
    m_nftImageLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f0f0f0;");
    m_nftImageLabel->setAlignment(Qt::AlignCenter);
    
    QVBoxLayout* infoLayout = new QVBoxLayout();
    m_nftNameLabel = new QLabel(this);
    m_nftNameLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    
    m_creatorLabel = new QLabel(this);
    m_reportCountLabel = new QLabel(this);
    m_statusLabel = new QLabel(this);
    
    infoLayout->addWidget(m_nftNameLabel);
    infoLayout->addWidget(m_creatorLabel);
    infoLayout->addWidget(m_reportCountLabel);
    infoLayout->addWidget(m_statusLabel);
    infoLayout->addStretch();
    
    imageLayout->addWidget(m_nftImageLabel);
    imageLayout->addLayout(infoLayout);
    imageLayout->addStretch();
    
    overviewLayout->addLayout(imageLayout);
    
    // Reason and notes
    QGroupBox* reasonGroup = new QGroupBox(tr("Report Reason"), this);
    QVBoxLayout* reasonLayout = new QVBoxLayout(reasonGroup);
    m_reasonText = new QTextEdit(this);
    m_reasonText->setReadOnly(true);
    m_reasonText->setMaximumHeight(100);
    reasonLayout->addWidget(m_reasonText);
    overviewLayout->addWidget(reasonGroup);
    
    QGroupBox* notesGroup = new QGroupBox(tr("Moderator Notes"), this);
    QVBoxLayout* notesLayout = new QVBoxLayout(notesGroup);
    m_moderatorNotesText = new QTextEdit(this);
    m_moderatorNotesText->setPlaceholderText(tr("Add moderator notes here..."));
    notesLayout->addWidget(m_moderatorNotesText);
    overviewLayout->addWidget(notesGroup);
    
    // Reports tab
    m_reportsTab = new QWidget();
    QVBoxLayout* reportsLayout = new QVBoxLayout(m_reportsTab);
    
    m_reportsTable = new QTableWidget(this);
    m_reportsTable->setColumnCount(3);
    m_reportsTable->setHorizontalHeaderLabels({tr("Date"), tr("Reporter"), tr("Reason")});
    m_reportsTable->horizontalHeader()->setStretchLastSection(true);
    m_reportsTable->setAlternatingRowColors(true);
    m_reportsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    reportsLayout->addWidget(m_reportsTable);
    
    // Actions tab
    m_actionsTab = new QWidget();
    QVBoxLayout* actionsLayout = new QVBoxLayout(m_actionsTab);
    
    QGroupBox* actionGroup = new QGroupBox(tr("Moderation Action"), this);
    QFormLayout* actionFormLayout = new QFormLayout(actionGroup);
    
    m_actionCombo = new QComboBox(this);
    m_actionCombo->addItems({
        tr("Approve"),
        tr("Hide"),
        tr("Remove"),
        tr("Blacklist Creator"),
        tr("Mark as Safe")
    });
    
    m_actionReasonText = new QTextEdit(this);
    m_actionReasonText->setPlaceholderText(tr("Enter reason for action..."));
    m_actionReasonText->setMaximumHeight(80);
    
    m_actionNotesText = new QTextEdit(this);
    m_actionNotesText->setPlaceholderText(tr("Additional notes..."));
    m_actionNotesText->setMaximumHeight(80);
    
    m_executeActionButton = new QPushButton(tr("Execute Action"), this);
    m_executeActionButton->setStyleSheet("background-color: #dc3545; color: white; font-weight: bold;");
    
    actionFormLayout->addRow(tr("Action:"), m_actionCombo);
    actionFormLayout->addRow(tr("Reason:"), m_actionReasonText);
    actionFormLayout->addRow(tr("Notes:"), m_actionNotesText);
    actionFormLayout->addRow(m_executeActionButton);
    
    actionsLayout->addWidget(actionGroup);
    actionsLayout->addStretch();
    
    // Add tabs
    m_tabWidget->addTab(m_overviewTab, tr("Overview"));
    m_tabWidget->addTab(m_reportsTab, tr("Reports"));
    m_tabWidget->addTab(m_actionsTab, tr("Actions"));
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Action buttons
    m_actionLayout = new QHBoxLayout();
    m_approveButton = new QPushButton(tr("✅ Approve"), this);
    m_hideButton = new QPushButton(tr("🚫 Hide"), this);
    m_removeButton = new QPushButton(tr("🗑️ Remove"), this);
    m_blacklistButton = new QPushButton(tr("🚫 Blacklist Creator"), this);
    m_safeButton = new QPushButton(tr("✅ Mark Safe"), this);
    m_closeButton = new QPushButton(tr("Close"), this);
    
    m_actionLayout->addWidget(m_approveButton);
    m_actionLayout->addWidget(m_hideButton);
    m_actionLayout->addWidget(m_removeButton);
    m_actionLayout->addWidget(m_blacklistButton);
    m_actionLayout->addWidget(m_safeButton);
    m_actionLayout->addStretch();
    m_actionLayout->addWidget(m_closeButton);
    
    m_mainLayout->addLayout(m_actionLayout);
    
    // Connect signals
    connect(m_approveButton, &QPushButton::clicked, this, &FlaggedNFTDetailsDialog::onApproveClicked);
    connect(m_hideButton, &QPushButton::clicked, this, &FlaggedNFTDetailsDialog::onHideClicked);
    connect(m_removeButton, &QPushButton::clicked, this, &FlaggedNFTDetailsDialog::onRemoveClicked);
    connect(m_blacklistButton, &QPushButton::clicked, this, &FlaggedNFTDetailsDialog::onBlacklistCreatorClicked);
    connect(m_safeButton, &QPushButton::clicked, this, &FlaggedNFTDetailsDialog::onMarkAsSafeClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &FlaggedNFTDetailsDialog::accept);
    connect(m_executeActionButton, &QPushButton::clicked, [this]() {
        QString action = m_actionCombo->currentText();
        QString reason = m_actionReasonText->toPlainText();
        QString notes = m_actionNotesText->toPlainText();
        
        if (reason.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Please enter a reason for the action"));
            return;
        }
        
        QString actionType;
        if (action == tr("Approve")) actionType = "approve";
        else if (action == tr("Hide")) actionType = "hide";
        else if (action == tr("Remove")) actionType = "remove";
        else if (action == tr("Blacklist Creator")) actionType = "blacklist_creator";
        else if (action == tr("Mark as Safe")) actionType = "mark_safe";
        
        emit moderationActionRequested(m_flaggedNFT.nftId, actionType, reason, notes);
        accept();
    });
}

void FlaggedNFTDetailsDialog::populateData()
{
    // Basic info
    m_nftNameLabel->setText(m_flaggedNFT.nftName);
    m_creatorLabel->setText(tr("Creator: %1").arg(m_flaggedNFT.creatorAddress));
    m_reportCountLabel->setText(tr("Reports: %1").arg(m_flaggedNFT.reportCount));
    m_statusLabel->setText(tr("Status: %1").arg(m_flaggedNFT.status));
    
    // Load NFT image if available
    if (!m_nftData.imageHash.isEmpty()) {
        // Load image from hash (placeholder implementation)
        m_nftImageLabel->setText(tr("Image\n%1").arg(m_nftData.imageHash.left(20) + "..."));
    } else {
        m_nftImageLabel->setText(tr("No Image"));
    }
    
    // Reason and notes
    m_reasonText->setText(m_flaggedNFT.description);
    m_moderatorNotesText->setText(m_flaggedNFT.moderatorNotes);
    
    // Populate reports table
    m_reportsTable->setRowCount(m_flaggedNFT.reportHistory.size());
    for (int i = 0; i < m_flaggedNFT.reportHistory.size(); ++i) {
        QTableWidgetItem* dateItem = new QTableWidgetItem(m_flaggedNFT.lastReported.toString("MMM dd, yyyy"));
        QTableWidgetItem* reporterItem = new QTableWidgetItem(m_flaggedNFT.reporterAddress);
        QTableWidgetItem* reasonItem = new QTableWidgetItem(m_flaggedNFT.reportHistory[i]);
        
        m_reportsTable->setItem(i, 0, dateItem);
        m_reportsTable->setItem(i, 1, reporterItem);
        m_reportsTable->setItem(i, 2, reasonItem);
    }
    
    updateActionButtons();
}

void FlaggedNFTDetailsDialog::updateActionButtons()
{
    // Enable/disable buttons based on current status
    bool canApprove = (m_flaggedNFT.status == "pending" || m_flaggedNFT.status == "hidden");
    bool canHide = (m_flaggedNFT.status == "pending" || m_flaggedNFT.status == "approved");
    bool canRemove = (m_flaggedNFT.status != "removed");
    bool canBlacklist = (m_flaggedNFT.status != "removed");
    bool canMarkSafe = (m_flaggedNFT.status == "pending" || m_flaggedNFT.status == "hidden");
    
    m_approveButton->setEnabled(canApprove);
    m_hideButton->setEnabled(canHide);
    m_removeButton->setEnabled(canRemove);
    m_blacklistButton->setEnabled(canBlacklist);
    m_safeButton->setEnabled(canMarkSafe);
}

void FlaggedNFTDetailsDialog::onApproveClicked()
{
    emit moderationActionRequested(m_flaggedNFT.nftId, "approve", "Approved by moderator", m_moderatorNotesText->toPlainText());
    accept();
}

void FlaggedNFTDetailsDialog::onHideClicked()
{
    emit moderationActionRequested(m_flaggedNFT.nftId, "hide", "Hidden by moderator", m_moderatorNotesText->toPlainText());
    accept();
}

void FlaggedNFTDetailsDialog::onRemoveClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Removal"), 
                                                           tr("Are you sure you want to permanently remove this NFT?"),
                                                           QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        emit moderationActionRequested(m_flaggedNFT.nftId, "remove", "Permanently removed by moderator", m_moderatorNotesText->toPlainText());
        accept();
    }
}

void FlaggedNFTDetailsDialog::onBlacklistCreatorClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Blacklist"), 
                                                           tr("Are you sure you want to blacklist creator %1?").arg(m_flaggedNFT.creatorAddress),
                                                           QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        emit moderationActionRequested(m_flaggedNFT.nftId, "blacklist_creator", "Creator blacklisted by moderator", m_moderatorNotesText->toPlainText());
        accept();
    }
}

void FlaggedNFTDetailsDialog::onMarkAsSafeClicked()
{
    emit moderationActionRequested(m_flaggedNFT.nftId, "mark_safe", "Marked as safe by moderator", m_moderatorNotesText->toPlainText());
    accept();
}

// ModerationHistoryDialog Implementation
ModerationHistoryDialog::ModerationHistoryDialog(const QList<TokenNFTExplorer::ModerationAction>& actions, QWidget* parent)
    : QDialog(parent)
    , m_allActions(actions)
    , m_filteredActions(actions)
{
    setWindowTitle(tr("Moderation History"));
    setModal(true);
    resize(800, 500);
    
    setupUI();
    populateActions();
}

void ModerationHistoryDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Controls
    m_controlsLayout = new QHBoxLayout();
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItems({
        tr("All Actions"),
        tr("Approve"),
        tr("Hide"),
        tr("Remove"),
        tr("Blacklist Creator"),
        tr("Mark Safe")
    });
    
    m_exportButton = new QPushButton(tr("📁 Export"), this);
    
    m_controlsLayout->addWidget(new QLabel(tr("Filter:")));
    m_controlsLayout->addWidget(m_filterCombo);
    m_controlsLayout->addStretch();
    m_controlsLayout->addWidget(m_exportButton);
    
    m_mainLayout->addLayout(m_controlsLayout);
    
    // Actions table
    m_actionsTable = new QTableWidget(this);
    m_actionsTable->setColumnCount(6);
    m_actionsTable->setHorizontalHeaderLabels({
        tr("Date"), tr("Action"), tr("Moderator"), tr("Reason"), tr("Notes"), tr("Auto")
    });
    m_actionsTable->horizontalHeader()->setStretchLastSection(true);
    m_actionsTable->setAlternatingRowColors(true);
    m_actionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    m_mainLayout->addWidget(m_actionsTable);
    
    // Close button
    m_closeButton = new QPushButton(tr("Close"), this);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);
    m_mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModerationHistoryDialog::onFilterChanged);
    connect(m_exportButton, &QPushButton::clicked, this, &ModerationHistoryDialog::onExportClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &ModerationHistoryDialog::accept);
}

void ModerationHistoryDialog::populateActions()
{
    applyFilter();
    
    m_actionsTable->setRowCount(m_filteredActions.size());
    for (int i = 0; i < m_filteredActions.size(); ++i) {
        const TokenNFTExplorer::ModerationAction& action = m_filteredActions[i];
        
        QTableWidgetItem* dateItem = new QTableWidgetItem(action.timestamp.toString("MMM dd, yyyy HH:mm"));
        QTableWidgetItem* actionItem = new QTableWidgetItem(action.actionType);
        QTableWidgetItem* moderatorItem = new QTableWidgetItem(action.moderatorAddress);
        QTableWidgetItem* reasonItem = new QTableWidgetItem(action.reason);
        QTableWidgetItem* notesItem = new QTableWidgetItem(action.notes);
        QTableWidgetItem* autoItem = new QTableWidgetItem(action.autoAction ? tr("Yes") : tr("No"));
        
        // Color code actions
        if (action.actionType == "approve") {
            actionItem->setBackground(QColor(0, 255, 0, 100)); // Green
        } else if (action.actionType == "hide") {
            actionItem->setBackground(QColor(255, 165, 0, 100)); // Orange
        } else if (action.actionType == "remove") {
            actionItem->setBackground(QColor(255, 0, 0, 100)); // Red
        } else if (action.actionType == "blacklist_creator") {
            actionItem->setBackground(QColor(128, 0, 128, 100)); // Purple
        }
        
        m_actionsTable->setItem(i, 0, dateItem);
        m_actionsTable->setItem(i, 1, actionItem);
        m_actionsTable->setItem(i, 2, moderatorItem);
        m_actionsTable->setItem(i, 3, reasonItem);
        m_actionsTable->setItem(i, 4, notesItem);
        m_actionsTable->setItem(i, 5, autoItem);
    }
}

void ModerationHistoryDialog::applyFilter()
{
    QString filterText = m_filterCombo->currentText();
    
    m_filteredActions.clear();
    for (const TokenNFTExplorer::ModerationAction& action : m_allActions) {
        if (filterText == tr("All Actions") ||
            (filterText == tr("Approve") && action.actionType == "approve") ||
            (filterText == tr("Hide") && action.actionType == "hide") ||
            (filterText == tr("Remove") && action.actionType == "remove") ||
            (filterText == tr("Blacklist Creator") && action.actionType == "blacklist_creator") ||
            (filterText == tr("Mark Safe") && action.actionType == "mark_safe")) {
            m_filteredActions.append(action);
        }
    }
}

void ModerationHistoryDialog::onFilterChanged()
{
    populateActions();
}

void ModerationHistoryDialog::onExportClicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Moderation History"), 
                                                   QString("moderation_history_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd")),
                                                   tr("CSV Files (*.csv)"));
    if (filename.isEmpty()) return;
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file for writing"));
        return;
    }
    
    QTextStream out(&file);
    out << "Date,Action,Moderator,Reason,Notes,Auto\n";
    
    for (const TokenNFTExplorer::ModerationAction& action : m_filteredActions) {
        out << QString("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",\"%6\"\n")
               .arg(action.timestamp.toString("yyyy-MM-dd HH:mm:ss"))
               .arg(action.actionType)
               .arg(action.moderatorAddress)
               .arg(action.reason)
               .arg(action.notes)
               .arg(action.autoAction ? "Yes" : "No");
    }
    
    file.close();
    QMessageBox::information(this, tr("Success"), tr("Moderation history exported successfully"));
}
