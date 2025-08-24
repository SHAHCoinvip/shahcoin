// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "tokennftexplorer.h"
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

void TokenNFTExplorer::setupModerationTab()
{
    m_moderationTab = new QWidget();
    m_moderationLayout = new QVBoxLayout(m_moderationTab);

    m_moderationControlsLayout = new QHBoxLayout();

    // Filter controls
    m_moderationFilterCombo = new QComboBox(this);
    m_moderationFilterCombo->addItems({
        tr("All Flagged"),
        tr("Pending Review"),
        tr("Reviewed"),
        tr("Hidden"),
        tr("Removed")
    });
    m_moderationFilterCombo->setToolTip(tr("Filter flagged NFTs by status"));

    m_moderationSortCombo = new QComboBox(this);
    m_moderationSortCombo->addItems({
        tr("Date (Newest)"),
        tr("Date (Oldest)"),
        tr("Report Count (High)"),
        tr("Report Count (Low)"),
        tr("Creator"),
        tr("Status")
    });
    m_moderationSortCombo->setToolTip(tr("Sort flagged NFTs"));

    // Control buttons
    m_moderationRefreshButton = new QPushButton(tr("🔄 Refresh"), this);
    m_moderationRefreshButton->setToolTip(tr("Refresh moderation data"));

    m_moderationSettingsButton = new QPushButton(tr("⚙️ Settings"), this);
    m_moderationSettingsButton->setToolTip(tr("Moderation settings"));

    m_moderationExportButton = new QPushButton(tr("📁 Export Log"), this);
    m_moderationExportButton->setToolTip(tr("Export moderation log"));

    // Add controls to layout
    m_moderationControlsLayout->addWidget(new QLabel(tr("Filter:")));
    m_moderationControlsLayout->addWidget(m_moderationFilterCombo);
    m_moderationControlsLayout->addWidget(new QLabel(tr("Sort:")));
    m_moderationControlsLayout->addWidget(m_moderationSortCombo);
    m_moderationControlsLayout->addStretch();
    m_moderationControlsLayout->addWidget(m_moderationRefreshButton);
    m_moderationControlsLayout->addWidget(m_moderationSettingsButton);
    m_moderationControlsLayout->addWidget(m_moderationExportButton);

    m_moderationLayout->addLayout(m_moderationControlsLayout);

    // Moderation table
    m_moderationTable = new QTableWidget(this);
    m_moderationTable->setColumnCount(8);
    m_moderationTable->setHorizontalHeaderLabels({
        tr("NFT Name"), tr("Creator"), tr("Reports"), tr("Status"), tr("First Reported"), tr("Last Reported"), tr("Reason"), tr("Actions")
    });
    m_moderationTable->horizontalHeader()->setStretchLastSection(true);
    m_moderationTable->setAlternatingRowColors(true);
    m_moderationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_moderationTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_moderationTable->setContextMenuPolicy(Qt::CustomContextMenu);

    // Set column widths
    m_moderationTable->setColumnWidth(0, 150); // NFT Name
    m_moderationTable->setColumnWidth(1, 120); // Creator
    m_moderationTable->setColumnWidth(2, 80);  // Reports
    m_moderationTable->setColumnWidth(3, 100); // Status
    m_moderationTable->setColumnWidth(4, 150); // First Reported
    m_moderationTable->setColumnWidth(5, 150); // Last Reported
    m_moderationTable->setColumnWidth(6, 200); // Reason
    m_moderationTable->setColumnWidth(7, 100); // Actions

    m_moderationLayout->addWidget(m_moderationTable);

    // Add moderation tab to main tab widget
    m_tabWidget->addTab(m_moderationTab, tr("🛡️ Moderation"));

    // Initialize moderation settings
    m_moderationSettings.autoHideThreshold = 5;
    m_moderationSettings.autoHideEnabled = true;
    m_moderationSettings.autoReviewThreshold = 3;
    m_moderationSettings.autoReviewEnabled = true;
    m_moderationSettings.requireModeratorApproval = false;
    m_moderationSettings.trustedModerators = QStringList();

    // Load moderation data
    loadFlaggedNFTs();
    loadModerationActions();
}

void TokenNFTExplorer::loadFlaggedNFTs()
{
    // Clear existing data
    m_flaggedNFTsList.clear();

    // Load from API (stub implementation)
    QNetworkRequest request(QUrl(m_apiBaseUrl + "/moderation/flagged"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_net.get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray flaggedArray = doc.array();

            for (const QJsonValue& value : flaggedArray) {
                QJsonObject flaggedObj = value.toObject();
                FlaggedNFT flagged;
                flagged.nftId = flaggedObj["nftId"].toString();
                flagged.nftName = flaggedObj["nftName"].toString();
                flagged.creatorAddress = flaggedObj["creatorAddress"].toString();
                flagged.reporterAddress = flaggedObj["reporterAddress"].toString();
                flagged.reason = flaggedObj["reason"].toString();
                flagged.description = flaggedObj["description"].toString();
                flagged.reportCount = flaggedObj["reportCount"].toInt();
                flagged.firstReported = QDateTime::fromString(flaggedObj["firstReported"].toString(), Qt::ISODate);
                flagged.lastReported = QDateTime::fromString(flaggedObj["lastReported"].toString(), Qt::ISODate);
                flagged.status = flaggedObj["status"].toString();
                flagged.moderatorNotes = flaggedObj["moderatorNotes"].toString();

                QJsonArray historyArray = flaggedObj["reportHistory"].toArray();
                for (const QJsonValue& historyValue : historyArray) {
                    flagged.reportHistory.append(historyValue.toString());
                }

                m_flaggedNFTsList.append(flagged);
            }
        } else {
            // Load sample data for testing
            loadSampleFlaggedNFTs();
        }
        reply->deleteLater();
        populateModerationTable();
    });
}

void TokenNFTExplorer::loadSampleFlaggedNFTs()
{
    // Sample flagged NFT data for testing
    FlaggedNFT flagged1;
    flagged1.nftId = "nft_001";
    flagged1.nftName = "Inappropriate Art #1";
    flagged1.creatorAddress = "Sabc123...";
    flagged1.reporterAddress = "Sdef456...";
    flagged1.reason = "Inappropriate content";
    flagged1.description = "Contains explicit content";
    flagged1.reportCount = 3;
    flagged1.firstReported = QDateTime::currentDateTime().addDays(-2);
    flagged1.lastReported = QDateTime::currentDateTime().addHours(-6);
    flagged1.status = "pending";
    flagged1.moderatorNotes = "";
    flagged1.reportHistory = {"Inappropriate content", "Explicit imagery", "Violates guidelines"};

    FlaggedNFT flagged2;
    flagged2.nftId = "nft_002";
    flagged2.nftName = "Spam Collection #5";
    flagged2.creatorAddress = "Sghi789...";
    flagged2.reporterAddress = "Sjkl012...";
    flagged2.reason = "Spam";
    flagged2.description = "Mass-produced low-quality NFTs";
    flagged2.reportCount = 7;
    flagged2.firstReported = QDateTime::currentDateTime().addDays(-5);
    flagged2.lastReported = QDateTime::currentDateTime().addHours(-2);
    flagged2.status = "reviewed";
    flagged2.moderatorNotes = "Confirmed spam, creator blacklisted";
    flagged2.reportHistory = {"Spam", "Low quality", "Mass production", "No artistic value"};

    m_flaggedNFTsList.append(flagged1);
    m_flaggedNFTsList.append(flagged2);
}

void TokenNFTExplorer::populateModerationTable()
{
    m_moderationTable->setRowCount(0);

    QString filterStatus = m_moderationFilterCombo->currentText();
    QString sortBy = m_moderationSortCombo->currentText();

    // Filter flagged NFTs
    QList<FlaggedNFT> filteredList;
    for (const FlaggedNFT& flagged : m_flaggedNFTsList) {
        if (filterStatus == tr("All Flagged") || 
            (filterStatus == tr("Pending Review") && flagged.status == "pending") ||
            (filterStatus == tr("Reviewed") && flagged.status == "reviewed") ||
            (filterStatus == tr("Hidden") && flagged.status == "hidden") ||
            (filterStatus == tr("Removed") && flagged.status == "removed")) {
            filteredList.append(flagged);
        }
    }

    // Sort filtered list
    std::sort(filteredList.begin(), filteredList.end(), [sortBy](const FlaggedNFT& a, const FlaggedNFT& b) {
        if (sortBy == tr("Date (Newest)")) {
            return a.lastReported > b.lastReported;
        } else if (sortBy == tr("Date (Oldest)")) {
            return a.lastReported < b.lastReported;
        } else if (sortBy == tr("Report Count (High)")) {
            return a.reportCount > b.reportCount;
        } else if (sortBy == tr("Report Count (Low)")) {
            return a.reportCount < b.reportCount;
        } else if (sortBy == tr("Creator")) {
            return a.creatorAddress < b.creatorAddress;
        } else if (sortBy == tr("Status")) {
            return a.status < b.status;
        }
        return false;
    });

    // Populate table
    m_moderationTable->setRowCount(filteredList.size());
    for (int i = 0; i < filteredList.size(); ++i) {
        const FlaggedNFT& flagged = filteredList[i];

        // NFT Name
        QTableWidgetItem* nameItem = new QTableWidgetItem(flagged.nftName);
        nameItem->setData(Qt::UserRole, flagged.nftId);
        m_moderationTable->setItem(i, 0, nameItem);

        // Creator
        QTableWidgetItem* creatorItem = new QTableWidgetItem(formatAddress(flagged.creatorAddress));
        m_moderationTable->setItem(i, 1, creatorItem);

        // Report Count
        QTableWidgetItem* countItem = new QTableWidgetItem(QString::number(flagged.reportCount));
        countItem->setTextAlignment(Qt::AlignCenter);
        m_moderationTable->setItem(i, 2, countItem);

        // Status
        QTableWidgetItem* statusItem = new QTableWidgetItem(flagged.status);
        if (flagged.status == "pending") {
            statusItem->setBackground(QColor(255, 255, 0, 100)); // Yellow
        } else if (flagged.status == "hidden") {
            statusItem->setBackground(QColor(255, 165, 0, 100)); // Orange
        } else if (flagged.status == "removed") {
            statusItem->setBackground(QColor(255, 0, 0, 100)); // Red
        } else if (flagged.status == "reviewed") {
            statusItem->setBackground(QColor(0, 255, 0, 100)); // Green
        }
        m_moderationTable->setItem(i, 3, statusItem);

        // First Reported
        QTableWidgetItem* firstItem = new QTableWidgetItem(flagged.firstReported.toString("MMM dd, yyyy"));
        m_moderationTable->setItem(i, 4, firstItem);

        // Last Reported
        QTableWidgetItem* lastItem = new QTableWidgetItem(flagged.lastReported.toString("MMM dd, yyyy"));
        m_moderationTable->setItem(i, 5, lastItem);

        // Reason
        QTableWidgetItem* reasonItem = new QTableWidgetItem(flagged.reason);
        reasonItem->setToolTip(flagged.description);
        m_moderationTable->setItem(i, 6, reasonItem);

        // Actions
        QWidget* actionsWidget = new QWidget();
        QHBoxLayout* actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->setContentsMargins(2, 2, 2, 2);

        QPushButton* viewButton = new QPushButton(tr("👁️"));
        viewButton->setToolTip(tr("View Details"));
        viewButton->setMaximumWidth(30);
        connect(viewButton, &QPushButton::clicked, [this, flagged]() {
            showFlaggedNFTDetailsDialog(flagged.nftId);
        });

        QPushButton* approveButton = new QPushButton(tr("✅"));
        approveButton->setToolTip(tr("Approve"));
        approveButton->setMaximumWidth(30);
        connect(approveButton, &QPushButton::clicked, [this, flagged]() {
            performModerationAction(flagged.nftId, "approve", "Approved by moderator", "");
        });

        QPushButton* hideButton = new QPushButton(tr("🚫"));
        hideButton->setToolTip(tr("Hide"));
        hideButton->setMaximumWidth(30);
        connect(hideButton, &QPushButton::clicked, [this, flagged]() {
            performModerationAction(flagged.nftId, "hide", "Hidden by moderator", "");
        });

        actionsLayout->addWidget(viewButton);
        actionsLayout->addWidget(approveButton);
        actionsLayout->addWidget(hideButton);
        actionsLayout->addStretch();

        m_moderationTable->setCellWidget(i, 7, actionsWidget);
    }
}

void TokenNFTExplorer::showModerationSettingsDialog()
{
    ModerationSettingsDialog* dialog = new ModerationSettingsDialog(m_moderationSettings, this);
    connect(dialog, &ModerationSettingsDialog::settingsUpdated, this, &TokenNFTExplorer::updateModerationSettings);
    dialog->exec();
    dialog->deleteLater();
}

void TokenNFTExplorer::showFlaggedNFTDetailsDialog(const QString& nftId)
{
    // Find the flagged NFT
    FlaggedNFT* flagged = nullptr;
    for (FlaggedNFT& f : m_flaggedNFTsList) {
        if (f.nftId == nftId) {
            flagged = &f;
            break;
        }
    }

    if (!flagged) {
        QMessageBox::warning(this, tr("Error"), tr("Flagged NFT not found"));
        return;
    }

    // Find the NFT data
    NFTData* nftData = nullptr;
    for (NFTData& nft : m_nftList) {
        if (nft.id == nftId) {
            nftData = &nft;
            break;
        }
    }

    if (!nftData) {
        QMessageBox::warning(this, tr("Error"), tr("NFT data not found"));
        return;
    }

    FlaggedNFTDetailsDialog* dialog = new FlaggedNFTDetailsDialog(*flagged, *nftData, this);
    connect(dialog, &FlaggedNFTDetailsDialog::moderationActionRequested, this, &TokenNFTExplorer::performModerationAction);
    dialog->exec();
    dialog->deleteLater();
}

void TokenNFTExplorer::performModerationAction(const QString& nftId, const QString& action, const QString& reason, const QString& notes)
{
    // Find the flagged NFT
    for (FlaggedNFT& flagged : m_flaggedNFTsList) {
        if (flagged.nftId == nftId) {
            // Update status based on action
            if (action == "approve") {
                flagged.status = "approved";
            } else if (action == "hide") {
                flagged.status = "hidden";
            } else if (action == "remove") {
                flagged.status = "removed";
            } else if (action == "mark_safe") {
                flagged.status = "safe";
            }

            flagged.moderatorNotes = notes;

            // Add moderation action to history
            ModerationAction modAction;
            modAction.actionId = QString("action_%1").arg(QDateTime::currentMSecsSinceEpoch());
            modAction.nftId = nftId;
            modAction.moderatorAddress = m_walletModel ? m_walletModel->getAddress() : "Unknown";
            modAction.actionType = action;
            modAction.reason = reason;
            modAction.timestamp = QDateTime::currentDateTime();
            modAction.notes = notes;
            modAction.autoAction = false;

            addModerationAction(modAction);

            // Handle blacklist action
            if (action == "blacklist_creator") {
                blacklistCreator(flagged.creatorAddress, reason, true);
            }

            break;
        }
    }

    // Update the table
    populateModerationTable();

    // Send to API
    QJsonObject actionData;
    actionData["nftId"] = nftId;
    actionData["action"] = action;
    actionData["reason"] = reason;
    actionData["notes"] = notes;
    actionData["moderatorAddress"] = m_walletModel ? m_walletModel->getAddress() : "Unknown";
    actionData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QNetworkRequest request(QUrl(m_apiBaseUrl + "/moderation/action"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_net.post(request, QJsonDocument(actionData).toJson());
    connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });
}

void TokenNFTExplorer::blacklistCreator(const QString& creatorAddress, const QString& reason, bool permanent, int daysToExpire)
{
    CreatorBlacklist blacklist;
    blacklist.creatorAddress = creatorAddress;
    blacklist.reason = reason;
    blacklist.blacklistedAt = QDateTime::currentDateTime();
    blacklist.blacklistedBy = m_walletModel ? m_walletModel->getAddress() : "Unknown";
    blacklist.isPermanent = permanent;
    if (!permanent) {
        blacklist.expiresAt = QDateTime::currentDateTime().addDays(daysToExpire);
    }

    m_blacklistedCreators[creatorAddress] = blacklist;

    // Save to settings
    QSettings settings("Shahcoin", "Wallet");
    QJsonObject blacklistData;
    blacklistData["creatorAddress"] = blacklist.creatorAddress;
    blacklistData["reason"] = blacklist.reason;
    blacklistData["blacklistedAt"] = blacklist.blacklistedAt.toString(Qt::ISODate);
    blacklistData["blacklistedBy"] = blacklist.blacklistedBy;
    blacklistData["isPermanent"] = blacklist.isPermanent;
    if (!blacklist.isPermanent) {
        blacklistData["expiresAt"] = blacklist.expiresAt.toString(Qt::ISODate);
    }

    QJsonArray blacklistArray = settings.value("moderation/blacklist", QJsonArray()).toJsonArray();
    blacklistArray.append(blacklistData);
    settings.setValue("moderation/blacklist", blacklistArray);
}

void TokenNFTExplorer::unblacklistCreator(const QString& creatorAddress)
{
    m_blacklistedCreators.remove(creatorAddress);

    // Remove from settings
    QSettings settings("Shahcoin", "Wallet");
    QJsonArray blacklistArray = settings.value("moderation/blacklist", QJsonArray()).toJsonArray();
    QJsonArray newArray;
    for (const QJsonValue& value : blacklistArray) {
        QJsonObject blacklist = value.toObject();
        if (blacklist["creatorAddress"].toString() != creatorAddress) {
            newArray.append(blacklist);
        }
    }
    settings.setValue("moderation/blacklist", newArray);
}

void TokenNFTExplorer::addModerationAction(const ModerationAction& action)
{
    m_moderationActions.append(action);

    // Save to settings
    QSettings settings("Shahcoin", "Wallet");
    QJsonObject actionData;
    actionData["actionId"] = action.actionId;
    actionData["nftId"] = action.nftId;
    actionData["moderatorAddress"] = action.moderatorAddress;
    actionData["actionType"] = action.actionType;
    actionData["reason"] = action.reason;
    actionData["timestamp"] = action.timestamp.toString(Qt::ISODate);
    actionData["notes"] = action.notes;
    actionData["autoAction"] = action.autoAction;

    QJsonArray actionsArray = settings.value("moderation/actions", QJsonArray()).toJsonArray();
    actionsArray.append(actionData);
    settings.setValue("moderation/actions", actionsArray);
}

void TokenNFTExplorer::loadModerationActions()
{
    QSettings settings("Shahcoin", "Wallet");
    QJsonArray actionsArray = settings.value("moderation/actions", QJsonArray()).toJsonArray();

    for (const QJsonValue& value : actionsArray) {
        QJsonObject actionData = value.toObject();
        ModerationAction action;
        action.actionId = actionData["actionId"].toString();
        action.nftId = actionData["nftId"].toString();
        action.moderatorAddress = actionData["moderatorAddress"].toString();
        action.actionType = actionData["actionType"].toString();
        action.reason = actionData["reason"].toString();
        action.timestamp = QDateTime::fromString(actionData["timestamp"].toString(), Qt::ISODate);
        action.notes = actionData["notes"].toString();
        action.autoAction = actionData["autoAction"].toBool();

        m_moderationActions.append(action);
    }
}

void TokenNFTExplorer::exportModerationLog(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file for writing"));
        return;
    }

    QTextStream out(&file);
    out << "Shahcoin NFT Marketplace - Moderation Log\n";
    out << "Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n\n";

    out << "Flagged NFTs:\n";
    out << "=============\n";
    for (const FlaggedNFT& flagged : m_flaggedNFTsList) {
        out << "NFT ID: " << flagged.nftId << "\n";
        out << "Name: " << flagged.nftName << "\n";
        out << "Creator: " << flagged.creatorAddress << "\n";
        out << "Reports: " << flagged.reportCount << "\n";
        out << "Status: " << flagged.status << "\n";
        out << "First Reported: " << flagged.firstReported.toString(Qt::ISODate) << "\n";
        out << "Last Reported: " << flagged.lastReported.toString(Qt::ISODate) << "\n";
        out << "Reason: " << flagged.reason << "\n";
        out << "Description: " << flagged.description << "\n";
        out << "Moderator Notes: " << flagged.moderatorNotes << "\n";
        out << "Report History: " << flagged.reportHistory.join(", ") << "\n\n";
    }

    out << "Moderation Actions:\n";
    out << "==================\n";
    for (const ModerationAction& action : m_moderationActions) {
        out << "Action ID: " << action.actionId << "\n";
        out << "NFT ID: " << action.nftId << "\n";
        out << "Moderator: " << action.moderatorAddress << "\n";
        out << "Action: " << action.actionType << "\n";
        out << "Reason: " << action.reason << "\n";
        out << "Timestamp: " << action.timestamp.toString(Qt::ISODate) << "\n";
        out << "Notes: " << action.notes << "\n";
        out << "Auto Action: " << (action.autoAction ? "Yes" : "No") << "\n\n";
    }

    out << "Blacklisted Creators:\n";
    out << "====================\n";
    for (auto it = m_blacklistedCreators.begin(); it != m_blacklistedCreators.end(); ++it) {
        const CreatorBlacklist& blacklist = it.value();
        out << "Creator: " << blacklist.creatorAddress << "\n";
        out << "Reason: " << blacklist.reason << "\n";
        out << "Blacklisted At: " << blacklist.blacklistedAt.toString(Qt::ISODate) << "\n";
        out << "Blacklisted By: " << blacklist.blacklistedBy << "\n";
        out << "Permanent: " << (blacklist.isPermanent ? "Yes" : "No") << "\n";
        if (!blacklist.isPermanent) {
            out << "Expires At: " << blacklist.expiresAt.toString(Qt::ISODate) << "\n";
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Success"), tr("Moderation log exported successfully"));
}

void TokenNFTExplorer::updateModerationSettings(const ModerationSettings& settings)
{
    m_moderationSettings = settings;

    // Save to settings
    QSettings settingsStorage("Shahcoin", "Wallet");
    QJsonObject settingsData;
    settingsData["autoHideThreshold"] = settings.autoHideThreshold;
    settingsData["autoHideEnabled"] = settings.autoHideEnabled;
    settingsData["autoReviewThreshold"] = settings.autoReviewThreshold;
    settingsData["autoReviewEnabled"] = settings.autoReviewEnabled;
    settingsData["requireModeratorApproval"] = settings.requireModeratorApproval;
    settingsData["trustedModerators"] = QJsonArray::fromStringList(settings.trustedModerators);

    settingsStorage.setValue("moderation/settings", settingsData);
}

void TokenNFTExplorer::checkAutoModeration(const QString& nftId)
{
    // Find the flagged NFT
    for (FlaggedNFT& flagged : m_flaggedNFTsList) {
        if (flagged.nftId == nftId) {
            // Check auto-hide threshold
            if (m_moderationSettings.autoHideEnabled && 
                flagged.reportCount >= m_moderationSettings.autoHideThreshold) {
                performModerationAction(nftId, "hide", "Auto-hidden due to report threshold", "Automatic action");
            }
            // Check auto-review threshold
            else if (m_moderationSettings.autoReviewEnabled && 
                     flagged.reportCount >= m_moderationSettings.autoReviewThreshold) {
                // Mark for review
                flagged.status = "pending_review";
            }
            break;
        }
    }
}

bool TokenNFTExplorer::isTrustedModerator(const QString& address) const
{
    return m_moderationSettings.trustedModerators.contains(address);
}

void TokenNFTExplorer::refreshModerationData()
{
    loadFlaggedNFTs();
    loadModerationActions();
    populateModerationTable();
}

// Moderation Panel Slots
void TokenNFTExplorer::onModerationFilterChanged()
{
    populateModerationTable();
}

void TokenNFTExplorer::onModerationSortChanged()
{
    populateModerationTable();
}

void TokenNFTExplorer::onModerationContextMenu(const QPoint& pos)
{
    QTableWidgetItem* item = m_moderationTable->itemAt(pos);
    if (!item) return;

    int row = item->row();
    QString nftId = m_moderationTable->item(row, 0)->data(Qt::UserRole).toString();
    m_selectedFlaggedNFTId = nftId;

    QMenu contextMenu(this);
    QAction* viewDetailsAction = contextMenu.addAction(tr("👁️ View Details"));
    QAction* approveAction = contextMenu.addAction(tr("✅ Approve"));
    QAction* hideAction = contextMenu.addAction(tr("🚫 Hide"));
    QAction* removeAction = contextMenu.addAction(tr("🗑️ Remove"));
    QAction* blacklistCreatorAction = contextMenu.addAction(tr("🚫 Blacklist Creator"));
    QAction* markSafeAction = contextMenu.addAction(tr("✅ Mark as Safe"));
    QAction* viewHistoryAction = contextMenu.addAction(tr("📋 View History"));

    connect(viewDetailsAction, &QAction::triggered, this, &TokenNFTExplorer::onViewFlaggedNFTDetails);
    connect(approveAction, &QAction::triggered, this, &TokenNFTExplorer::onApproveNFT);
    connect(hideAction, &QAction::triggered, this, &TokenNFTExplorer::onHideNFT);
    connect(removeAction, &QAction::triggered, this, &TokenNFTExplorer::onRemoveNFT);
    connect(blacklistCreatorAction, &QAction::triggered, this, &TokenNFTExplorer::onBlacklistCreator);
    connect(markSafeAction, &QAction::triggered, this, &TokenNFTExplorer::onMarkAsSafe);
    connect(viewHistoryAction, &QAction::triggered, this, &TokenNFTExplorer::onViewModerationHistory);

    contextMenu.exec(m_moderationTable->viewport()->mapToGlobal(pos));
}

void TokenNFTExplorer::onModerationSettingsClicked()
{
    showModerationSettingsDialog();
}

void TokenNFTExplorer::onModerationExportClicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Moderation Log"), 
                                                   QString("moderation_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd")),
                                                   tr("Text Files (*.txt)"));
    if (!filename.isEmpty()) {
        exportModerationLog(filename);
    }
}

void TokenNFTExplorer::onApproveNFT()
{
    if (m_selectedFlaggedNFTId.isEmpty()) return;
    performModerationAction(m_selectedFlaggedNFTId, "approve", "Approved by moderator", "");
}

void TokenNFTExplorer::onHideNFT()
{
    if (m_selectedFlaggedNFTId.isEmpty()) return;
    performModerationAction(m_selectedFlaggedNFTId, "hide", "Hidden by moderator", "");
}

void TokenNFTExplorer::onRemoveNFT()
{
    if (m_selectedFlaggedNFTId.isEmpty()) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Removal"), 
                                                             tr("Are you sure you want to permanently remove this NFT?"),
                                                             QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        performModerationAction(m_selectedFlaggedNFTId, "remove", "Permanently removed by moderator", "");
    }
}

void TokenNFTExplorer::onBlacklistCreator()
{
    if (m_selectedFlaggedNFTId.isEmpty()) return;
    
    // Find the flagged NFT to get creator address
    for (const FlaggedNFT& flagged : m_flaggedNFTsList) {
        if (flagged.nftId == m_selectedFlaggedNFTId) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Blacklist"), 
                                                                     tr("Are you sure you want to blacklist creator %1?").arg(formatAddress(flagged.creatorAddress)),
                                                                     QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                performModerationAction(m_selectedFlaggedNFTId, "blacklist_creator", "Creator blacklisted by moderator", "");
            }
            break;
        }
    }
}

void TokenNFTExplorer::onMarkAsSafe()
{
    if (m_selectedFlaggedNFTId.isEmpty()) return;
    performModerationAction(m_selectedFlaggedNFTId, "mark_safe", "Marked as safe by moderator", "");
}

void TokenNFTExplorer::onViewFlaggedNFTDetails()
{
    if (m_selectedFlaggedNFTId.isEmpty()) return;
    showFlaggedNFTDetailsDialog(m_selectedFlaggedNFTId);
}

void TokenNFTExplorer::onViewModerationHistory()
{
    if (m_selectedFlaggedNFTId.isEmpty()) return;
    
    // Filter actions for this NFT
    QList<ModerationAction> nftActions;
    for (const ModerationAction& action : m_moderationActions) {
        if (action.nftId == m_selectedFlaggedNFTId) {
            nftActions.append(action);
        }
    }
    
    ModerationHistoryDialog* dialog = new ModerationHistoryDialog(nftActions, this);
    dialog->exec();
    dialog->deleteLater();
}
