#include <qt/enhancednotifications.h>
#include <qt/walletmodel.h>
#include <qt/notificator.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QSettings>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QSoundEffect>
#include <QMediaPlayer>
#include <QUrl>

EnhancedNotifications::EnhancedNotifications(QObject* parent)
    : QObject(parent)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_showHistoryAction(nullptr)
    , m_clearNotificationsAction(nullptr)
    , m_settingsAction(nullptr)
    , m_walletModel(nullptr)
    , m_notificator(nullptr)
    , m_checkTimer(new QTimer(this))
    , m_settings(new QSettings(this))
    , m_notificationsEnabled(true)
    , m_soundEnabled(true)
    , m_popupEnabled(true)
    , m_trayIconEnabled(true)
    , m_lastBalance(0.0)
    , m_stakingActive(false)
    , m_miningActive(false)
    , m_lastCheckTime(QDateTime::currentDateTime())
{
    // Initialize enabled notification types
    m_enabledTypes.insert(IncomingSHAH);
    m_enabledTypes.insert(StakingReward);
    m_enabledTypes.insert(NFTMinted);
    m_enabledTypes.insert(TokenCreated);
    m_enabledTypes.insert(MiningReward);
    m_enabledTypes.insert(TransactionSent);
    m_enabledTypes.insert(TransactionFailed);

    setupUI();
    loadSettings();

    // Set up periodic checking
    connect(m_checkTimer, &QTimer::timeout, this, &EnhancedNotifications::checkForNewTransactions);
    m_checkTimer->start(30000); // Check every 30 seconds
}

EnhancedNotifications::~EnhancedNotifications()
{
    saveSettings();
}

void EnhancedNotifications::setWalletModel(WalletModel* model)
{
    m_walletModel = model;
    if (m_walletModel) {
        // Connect to wallet signals
        connect(m_walletModel, &WalletModel::balanceChanged, this, &EnhancedNotifications::onBalanceChanged);
        connect(m_walletModel, &WalletModel::transactionChanged, this, &EnhancedNotifications::onWalletTransactionChanged);
        
        // Initialize last balance
        m_lastBalance = ShahcoinUnits::fromAmount(m_walletModel->getBalance());
    }
}

void EnhancedNotifications::setNotificator(Notificator* notificator)
{
    m_notificator = notificator;
}

void EnhancedNotifications::setSystemTrayIcon(QSystemTrayIcon* trayIcon)
{
    m_trayIcon = trayIcon;
    setupTrayIcon();
}

void EnhancedNotifications::setupUI()
{
    // Create tray menu
    m_trayMenu = new QMenu();
    m_showHistoryAction = new QAction(tr("🔔 Show Notifications"), this);
    m_clearNotificationsAction = new QAction(tr("🗑️ Clear All"), this);
    m_settingsAction = new QAction(tr("⚙️ Settings"), this);

    connect(m_showHistoryAction, &QAction::triggered, this, &EnhancedNotifications::onShowNotificationHistory);
    connect(m_clearNotificationsAction, &QAction::triggered, this, &EnhancedNotifications::onClearNotifications);
    connect(m_settingsAction, &QAction::triggered, this, &EnhancedNotifications::showNotificationHistory);

    m_trayMenu->addAction(m_showHistoryAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_clearNotificationsAction);
    m_trayMenu->addAction(m_settingsAction);
}

void EnhancedNotifications::setupTrayIcon()
{
    if (m_trayIcon) {
        m_trayIcon->setContextMenu(m_trayMenu);
        updateTrayIcon();
    }
}

void EnhancedNotifications::loadSettings()
{
    m_notificationsEnabled = m_settings->value("EnhancedNotifications/enabled", true).toBool();
    m_soundEnabled = m_settings->value("EnhancedNotifications/sound", true).toBool();
    m_popupEnabled = m_settings->value("EnhancedNotifications/popup", true).toBool();
    m_trayIconEnabled = m_settings->value("EnhancedNotifications/trayIcon", true).toBool();

    // Load enabled notification types
    m_enabledTypes.clear();
    QStringList enabledTypes = m_settings->value("EnhancedNotifications/enabledTypes").toStringList();
    for (const QString& typeStr : enabledTypes) {
        bool ok;
        int type = typeStr.toInt(&ok);
        if (ok && type >= 0 && type < TransactionFailed + 1) {
            m_enabledTypes.insert(static_cast<NotificationType>(type));
        }
    }

    // If no types are saved, use defaults
    if (m_enabledTypes.isEmpty()) {
        m_enabledTypes.insert(IncomingSHAH);
        m_enabledTypes.insert(StakingReward);
        m_enabledTypes.insert(NFTMinted);
        m_enabledTypes.insert(TokenCreated);
        m_enabledTypes.insert(MiningReward);
        m_enabledTypes.insert(TransactionSent);
        m_enabledTypes.insert(TransactionFailed);
    }
}

void EnhancedNotifications::saveSettings()
{
    m_settings->setValue("EnhancedNotifications/enabled", m_notificationsEnabled);
    m_settings->setValue("EnhancedNotifications/sound", m_soundEnabled);
    m_settings->setValue("EnhancedNotifications/popup", m_popupEnabled);
    m_settings->setValue("EnhancedNotifications/trayIcon", m_trayIconEnabled);

    // Save enabled notification types
    QStringList enabledTypes;
    for (NotificationType type : m_enabledTypes) {
        enabledTypes.append(QString::number(static_cast<int>(type)));
    }
    m_settings->setValue("EnhancedNotifications/enabledTypes", enabledTypes);
}

void EnhancedNotifications::setNotificationsEnabled(bool enabled)
{
    m_notificationsEnabled = enabled;
    saveSettings();
}

void EnhancedNotifications::setNotificationTypeEnabled(NotificationType type, bool enabled)
{
    if (enabled) {
        m_enabledTypes.insert(type);
    } else {
        m_enabledTypes.remove(type);
    }
    saveSettings();
}

void EnhancedNotifications::setSoundEnabled(bool enabled)
{
    m_soundEnabled = enabled;
    saveSettings();
}

void EnhancedNotifications::setPopupEnabled(bool enabled)
{
    m_popupEnabled = enabled;
    saveSettings();
}

void EnhancedNotifications::setTrayIconEnabled(bool enabled)
{
    m_trayIconEnabled = enabled;
    saveSettings();
    updateTrayIcon();
}

bool EnhancedNotifications::isNotificationsEnabled() const
{
    return m_notificationsEnabled;
}

bool EnhancedNotifications::isNotificationTypeEnabled(NotificationType type) const
{
    return m_enabledTypes.contains(type);
}

bool EnhancedNotifications::isSoundEnabled() const
{
    return m_soundEnabled;
}

bool EnhancedNotifications::isPopupEnabled() const
{
    return m_popupEnabled;
}

bool EnhancedNotifications::isTrayIconEnabled() const
{
    return m_trayIconEnabled;
}

QList<EnhancedNotifications::NotificationItem> EnhancedNotifications::getNotificationHistory() const
{
    return m_notificationHistory;
}

void EnhancedNotifications::clearNotificationHistory()
{
    m_notificationHistory.clear();
    updateTrayIcon();
}

void EnhancedNotifications::markAsRead(const QString& transactionId)
{
    for (NotificationItem& item : m_notificationHistory) {
        if (item.transactionId == transactionId) {
            item.isRead = true;
            break;
        }
    }
    updateTrayIcon();
}

int EnhancedNotifications::getUnreadCount() const
{
    int count = 0;
    for (const NotificationItem& item : m_notificationHistory) {
        if (!item.isRead) {
            count++;
        }
    }
    return count;
}

void EnhancedNotifications::checkForNewTransactions()
{
    if (!m_walletModel || !m_notificationsEnabled) {
        return;
    }

    // Check for new transactions
    // This would typically involve checking the wallet's transaction list
    // For now, we'll simulate this with a simple balance check
    double currentBalance = ShahcoinUnits::fromAmount(m_walletModel->getBalance());
    
    if (currentBalance > m_lastBalance) {
        double difference = currentBalance - m_lastBalance;
        processTransactionNotification("", "", difference);
    }
    
    m_lastBalance = currentBalance;
    m_lastCheckTime = QDateTime::currentDateTime();
}

void EnhancedNotifications::checkForStakingRewards()
{
    if (!m_walletModel || !m_notificationsEnabled) {
        return;
    }

    // Check for staking rewards
    // This would involve checking if the wallet has received staking rewards
    // For now, we'll simulate this
    if (m_stakingActive) {
        // Simulate staking reward detection
        double stakingReward = 0.1; // Placeholder
        if (stakingReward > 0) {
            processStakingReward(stakingReward);
        }
    }
}

void EnhancedNotifications::checkForMiningRewards()
{
    if (!m_walletModel || !m_notificationsEnabled) {
        return;
    }

    // Check for mining rewards
    // This would involve checking if the wallet has received mining rewards
    // For now, we'll simulate this
    if (m_miningActive) {
        // Simulate mining reward detection
        double miningReward = 0.5; // Placeholder
        if (miningReward > 0) {
            processMiningReward(miningReward);
        }
    }
}

void EnhancedNotifications::showNotificationHistory()
{
    NotificationHistoryDialog dialog(m_notificationHistory, qobject_cast<QWidget*>(parent()));
    dialog.exec();
}

void EnhancedNotifications::clearAllNotifications()
{
    clearNotificationHistory();
}

void EnhancedNotifications::onWalletTransactionChanged()
{
    if (!m_notificationsEnabled) {
        return;
    }

    // Process new transactions
    // This would involve checking the wallet's transaction list for new entries
    checkForNewTransactions();
}

void EnhancedNotifications::onBalanceChanged()
{
    if (!m_notificationsEnabled) {
        return;
    }

    // Check for balance changes that might indicate new transactions
    checkForNewTransactions();
}

void EnhancedNotifications::onStakingStatusChanged()
{
    m_stakingActive = !m_stakingActive;
    checkForStakingRewards();
}

void EnhancedNotifications::onMiningStatusChanged()
{
    m_miningActive = !m_miningActive;
    checkForMiningRewards();
}

void EnhancedNotifications::onNotificationClicked()
{
    showNotificationHistory();
}

void EnhancedNotifications::onShowNotificationHistory()
{
    showNotificationHistory();
}

void EnhancedNotifications::onClearNotifications()
{
    clearAllNotifications();
}

void EnhancedNotifications::addNotification(NotificationType type, const QString& title, const QString& message, 
                                           const QString& details, const QString& transactionId)
{
    if (!m_notificationsEnabled || !m_enabledTypes.contains(type)) {
        return;
    }

    NotificationItem item;
    item.type = type;
    item.title = title;
    item.message = message;
    item.details = details;
    item.timestamp = QDateTime::currentDateTime();
    item.isRead = false;
    item.transactionId = transactionId;

    m_notificationHistory.prepend(item);

    // Keep only last 100 notifications
    if (m_notificationHistory.size() > 100) {
        m_notificationHistory = m_notificationHistory.mid(0, 100);
    }

    showNotification(item);
    updateTrayIcon();
    Q_EMIT alertRaised(type);
}

void EnhancedNotifications::showNotification(const NotificationItem& item)
{
    if (!m_popupEnabled || !m_notificator) {
        return;
    }

    Notificator::Class notifClass = Notificator::Information;
    switch (item.type) {
        case TransactionFailed:
            notifClass = Notificator::Critical;
            break;
        case StakingReward:
        case MiningReward:
        case IncomingSHAH:
            notifClass = Notificator::Information;
            break;
        default:
            notifClass = Notificator::Information;
            break;
    }

    m_notificator->notify(notifClass, item.title, item.message);

    if (m_soundEnabled) {
        playNotificationSound();
    }
}

void EnhancedNotifications::playNotificationSound()
{
    // Play notification sound
    // This could use QSoundEffect or QMediaPlayer depending on the platform
    // For now, we'll just log that sound should be played
    qDebug() << "Playing notification sound";
}

void EnhancedNotifications::updateTrayIcon()
{
    if (!m_trayIcon || !m_trayIconEnabled) {
        return;
    }

    int unreadCount = getUnreadCount();
    if (unreadCount > 0) {
        m_trayIcon->setToolTip(tr("Shahcoin - %1 unread notifications").arg(unreadCount));
        // You could also set a different icon here to indicate unread notifications
    } else {
        m_trayIcon->setToolTip(tr("Shahcoin"));
    }
}

void EnhancedNotifications::processTransactionNotification(const QString& transactionId, const QString& address, double amount)
{
    QString title = tr("💰 Incoming SHAH");
    QString message = tr("You received %1 SHAH").arg(ShahcoinUnits::formatWithUnit(ShahcoinUnits::SHAH, amount));
    QString details = tr("From: %1").arg(address.isEmpty() ? tr("Unknown") : address);
    
    addNotification(IncomingSHAH, title, message, details, transactionId);
}

void EnhancedNotifications::processStakingReward(double amount)
{
    QString title = tr("🎯 Staking Reward");
    QString message = tr("You earned %1 SHAH from staking").arg(ShahcoinUnits::formatWithUnit(ShahcoinUnits::SHAH, amount));
    QString details = tr("Staking reward received");
    
    addNotification(StakingReward, title, message, details);
}

void EnhancedNotifications::processMiningReward(double amount)
{
    QString title = tr("⛏️ Mining Reward");
    QString message = tr("You earned %1 SHAH from mining").arg(ShahcoinUnits::formatWithUnit(ShahcoinUnits::SHAH, amount));
    QString details = tr("Block reward received");
    
    addNotification(MiningReward, title, message, details);
}

void EnhancedNotifications::processNFTNotification(const QString& nftId, const QString& action)
{
    QString title = tr("🖼️ NFT %1").arg(action);
    QString message = tr("NFT %1 was %2").arg(nftId, action.toLower());
    QString details = tr("NFT ID: %1").arg(nftId);
    
    NotificationType type = (action == "Minted") ? NFTMinted : NFTTransferred;
    addNotification(type, title, message, details, nftId);
}

void EnhancedNotifications::processTokenNotification(const QString& tokenId, const QString& action)
{
    QString title = tr("🪙 Token %1").arg(action);
    QString message = tr("Token %1 was %2").arg(tokenId, action.toLower());
    QString details = tr("Token ID: %1").arg(tokenId);
    
    addNotification(TokenCreated, title, message, details, tokenId);
}

// NotificationHistoryDialog Implementation

NotificationHistoryDialog::NotificationHistoryDialog(const QList<EnhancedNotifications::NotificationItem>& notifications, QWidget* parent)
    : QDialog(parent)
    , m_notifications(notifications)
    , m_layout(new QVBoxLayout(this))
    , m_notificationList(new QListWidget(this))
    , m_clearAllButton(new QPushButton(tr("Clear All"), this))
    , m_markAllReadButton(new QPushButton(tr("Mark All Read"), this))
    , m_exportButton(new QPushButton(tr("Export"), this))
    , m_closeButton(new QPushButton(tr("Close"), this))
{
    setWindowTitle(tr("Notification History"));
    setModal(true);
    resize(600, 400);
    
    setupUI();
    populateNotifications();
    
    connect(m_clearAllButton, &QPushButton::clicked, this, &NotificationHistoryDialog::onClearAllClicked);
    connect(m_markAllReadButton, &QPushButton::clicked, this, &NotificationHistoryDialog::onMarkAllReadClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &NotificationHistoryDialog::onExportClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void NotificationHistoryDialog::setupUI()
{
    // Header
    QLabel* headerLabel = new QLabel(tr("🔔 Notification History"), this);
    QFont headerFont = headerLabel->font();
    headerFont.setPointSize(14);
    headerFont.setBold(true);
    headerLabel->setFont(headerFont);
    
    m_layout->addWidget(headerLabel);
    
    // Notification list
    m_notificationList->setAlternatingRowColors(true);
    m_layout->addWidget(m_notificationList);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_clearAllButton);
    buttonLayout->addWidget(m_markAllReadButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_closeButton);
    
    m_layout->addLayout(buttonLayout);
}

void NotificationHistoryDialog::populateNotifications()
{
    m_notificationList->clear();
    
    for (const EnhancedNotifications::NotificationItem& item : m_notifications) {
        QListWidgetItem* listItem = new QListWidgetItem();
        
        QString icon = getNotificationIcon(item.type);
        QString typeText = getNotificationTypeText(item.type);
        QString timestamp = item.timestamp.toString("MM/dd/yyyy hh:mm AP");
        QString readStatus = item.isRead ? "" : " 🔴";
        
        QString displayText = QString("%1 %2%3\n%4\n%5")
            .arg(icon)
            .arg(typeText)
            .arg(readStatus)
            .arg(item.message)
            .arg(timestamp);
        
        listItem->setText(displayText);
        listItem->setData(Qt::UserRole, item.transactionId);
        
        if (!item.isRead) {
            QFont font = listItem->font();
            font.setBold(true);
            listItem->setFont(font);
        }
        
        m_notificationList->addItem(listItem);
    }
}

QString NotificationHistoryDialog::getNotificationIcon(EnhancedNotifications::NotificationType type) const
{
    switch (type) {
        case EnhancedNotifications::IncomingSHAH: return "💰";
        case EnhancedNotifications::StakingReward: return "🎯";
        case EnhancedNotifications::NFTMinted: return "🖼️";
        case EnhancedNotifications::NFTTransferred: return "🔄";
        case EnhancedNotifications::TokenCreated: return "🪙";
        case EnhancedNotifications::MiningReward: return "⛏️";
        case EnhancedNotifications::TransactionSent: return "📤";
        case EnhancedNotifications::TransactionFailed: return "❌";
        default: return "📢";
    }
}

QString NotificationHistoryDialog::getNotificationTypeText(EnhancedNotifications::NotificationType type) const
{
    switch (type) {
        case EnhancedNotifications::IncomingSHAH: return tr("Incoming SHAH");
        case EnhancedNotifications::StakingReward: return tr("Staking Reward");
        case EnhancedNotifications::NFTMinted: return tr("NFT Minted");
        case EnhancedNotifications::NFTTransferred: return tr("NFT Transferred");
        case EnhancedNotifications::TokenCreated: return tr("Token Created");
        case EnhancedNotifications::MiningReward: return tr("Mining Reward");
        case EnhancedNotifications::TransactionSent: return tr("Transaction Sent");
        case EnhancedNotifications::TransactionFailed: return tr("Transaction Failed");
        default: return tr("Notification");
    }
}

void NotificationHistoryDialog::onClearAllClicked()
{
    if (QMessageBox::question(this, tr("Clear All Notifications"),
        tr("Are you sure you want to clear all notifications?"),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        m_notificationList->clear();
        // Emit signal to clear notifications in main system
        Q_EMIT clearAllNotifications();
    }
}

void NotificationHistoryDialog::onMarkAllReadClicked()
{
    for (int i = 0; i < m_notificationList->count(); ++i) {
        QListWidgetItem* item = m_notificationList->item(i);
        QFont font = item->font();
        font.setBold(false);
        item->setFont(font);
        
        // Update text to remove unread indicator
        QString text = item->text();
        text.remove(" 🔴");
        item->setText(text);
    }
}

void NotificationHistoryDialog::onExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Notifications"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/shahcoin_notifications.csv",
        tr("CSV Files (*.csv)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Export Failed"), tr("Could not open file for writing."));
        return;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // Write header
    out << "Type,Title,Message,Details,Timestamp,Transaction ID,Address,Amount\n";
    
    // Write data
    for (const EnhancedNotifications::NotificationItem& item : m_notifications) {
        QString typeText = getNotificationTypeText(item.type);
        QString timestamp = item.timestamp.toString(Qt::ISODate);
        
        out << QString("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",\"%6\",\"%7\",\"%8\"\n")
            .arg(typeText)
            .arg(item.title)
            .arg(item.message)
            .arg(item.details)
            .arg(timestamp)
            .arg(item.transactionId)
            .arg(item.address)
            .arg(item.amount);
    }
    
    file.close();
    QMessageBox::information(this, tr("Export Successful"), 
        tr("Notifications exported to %1").arg(fileName));
}
