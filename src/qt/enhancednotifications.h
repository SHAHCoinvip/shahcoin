#ifndef SHAHCOIN_QT_ENHANCEDNOTIFICATIONS_H
#define SHAHCOIN_QT_ENHANCEDNOTIFICATIONS_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QSound>
#include <QMediaPlayer>
#include <QSettings>
#include <QTimer>
#include <QQueue>
#include <QMutex>

class QWidget;
class QAction;

/**
 * @brief Enhanced Notification System for Shahcoin Qt Wallet
 * 
 * Provides comprehensive notification features including:
 * - Desktop notifications for various wallet events
 * - Sound alerts with customizable sounds
 * - Notification history and management
 * - Settings for notification preferences
 * - Toast notifications for immediate feedback
 */
class EnhancedNotifications : public QObject
{
    Q_OBJECT

public:
    enum NotificationType {
        IncomingTransaction,
        StakingReward,
        TokenCreated,
        NFTCreated,
        SwapCompleted,
        WalletEncrypted,
        BackupReminder,
        SecurityAlert,
        General
    };

    enum NotificationPriority {
        Low,
        Normal,
        High,
        Critical
    };

    struct NotificationData {
        NotificationType type;
        NotificationPriority priority;
        QString title;
        QString message;
        QString icon;
        QString sound;
        bool persistent;
        int timeout;
        QDateTime timestamp;
    };

    explicit EnhancedNotifications(QObject* parent = nullptr);
    ~EnhancedNotifications();

    // Notification methods
    void showNotification(NotificationType type, const QString& title, const QString& message, 
                         NotificationPriority priority = Normal);
    void showToastNotification(const QString& message, int duration = 3000);
    void showDesktopNotification(const QString& title, const QString& message, 
                                const QString& icon = QString());
    
    // Sound management
    void playSound(const QString& soundFile);
    void playNotificationSound(NotificationType type);
    void setSoundEnabled(bool enabled);
    bool isSoundEnabled() const;
    
    // Settings
    void loadSettings();
    void saveSettings();
    void setNotificationEnabled(NotificationType type, bool enabled);
    bool isNotificationEnabled(NotificationType type) const;
    
    // History
    QList<NotificationData> getNotificationHistory() const;
    void clearNotificationHistory();
    void exportNotificationHistory(const QString& filePath);

public Q_SLOTS:
    void onIncomingTransaction(const QString& amount, const QString& address);
    void onStakingReward(const QString& amount);
    void onTokenCreated(const QString& tokenName);
    void onNFTCreated(const QString& nftName);
    void onSwapCompleted(const QString& fromToken, const QString& toToken, const QString& amount);
    void onWalletEncrypted();
    void onBackupReminder();
    void onSecurityAlert(const QString& message);
    
    void onNotificationClicked();
    void onNotificationActionTriggered(QSystemTrayIcon::ActivationReason reason);
    void onSoundFinished();

Q_SIGNALS:
    void notificationShown(const NotificationData& notification);
    void notificationClicked(const NotificationData& notification);
    void soundPlayed(const QString& soundFile);
    void settingsChanged();

private:
    void setupSystemTray();
    void setupSounds();
    void setupSettings();
    void processNotificationQueue();
    void addToHistory(const NotificationData& notification);
    QString getDefaultSound(NotificationType type) const;
    QString getDefaultIcon(NotificationType type) const;
    QString getNotificationTitle(NotificationType type) const;
    
    // UI Components
    QSystemTrayIcon* m_trayIcon;
    QWidget* m_toastWidget;
    
    // Sound Components
    QMediaPlayer* m_soundPlayer;
    bool m_soundEnabled;
    QMap<NotificationType, QString> m_soundFiles;
    
    // Settings
    QSettings* m_settings;
    QMap<NotificationType, bool> m_notificationSettings;
    bool m_desktopNotificationsEnabled;
    bool m_toastNotificationsEnabled;
    int m_notificationTimeout;
    int m_maxHistorySize;
    
    // History
    QList<NotificationData> m_notificationHistory;
    QMutex m_historyMutex;
    
    // Queue
    QQueue<NotificationData> m_notificationQueue;
    QTimer* m_queueTimer;
    QMutex m_queueMutex;
    
    // Constants
    static constexpr int DEFAULT_TIMEOUT = 5000;
    static constexpr int MAX_HISTORY_SIZE = 100;
    static constexpr int QUEUE_PROCESS_INTERVAL = 100;
};

#endif // SHAHCOIN_QT_ENHANCEDNOTIFICATIONS_H
