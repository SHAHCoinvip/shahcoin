#ifndef SHAHCOIN_QT_EASTEREGG_H
#define SHAHCOIN_QT_EASTEREGG_H

#include <QObject>
#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QKeySequence>
#include <QMap>
#include <QString>

class QLabel;
class QWidget;

/**
 * @brief Easter Egg System for Shahcoin Qt Wallet
 * 
 * Provides hidden features and fun interactions including:
 * - Hidden keyboard shortcuts
 * - Secret animations and effects
 * - Fun messages and surprises
 * - Konami code and other classic easter eggs
 * - Hidden settings and features
 */
class EasterEgg : public QObject
{
    Q_OBJECT

public:
    enum EasterEggType {
        KonamiCode,
        ShahiCrown,
        SecretMessage,
        HiddenAnimation,
        RetroMode,
        DeveloperMode,
        SecretSettings,
        FunFact,
        Achievement,
        Custom
    };

    struct EasterEggData {
        EasterEggType type;
        QString name;
        QString description;
        QString trigger;
        QString message;
        bool enabled;
        int cooldown;
        QDateTime lastTriggered;
    };

    explicit EasterEgg(QObject* parent = nullptr);
    ~EasterEgg();

    // Easter egg management
    void registerEasterEgg(EasterEggType type, const QString& name, const QString& description, 
                          const QString& trigger, const QString& message);
    void triggerEasterEgg(EasterEggType type);
    void triggerEasterEgg(const QString& name);
    bool isEasterEggEnabled(EasterEggType type) const;
    void setEasterEggEnabled(EasterEggType type, bool enabled);
    
    // Keyboard shortcuts
    void setupKeyboardShortcuts(QWidget* parent);
    void handleKeyPress(QKeyEvent* event);
    void handleKeySequence(const QKeySequence& sequence);
    
    // Special effects
    void showShahiCrown();
    void showSecretMessage(const QString& message);
    void playHiddenAnimation();
    void enableRetroMode();
    void enableDeveloperMode();
    void showFunFact();
    void unlockAchievement(const QString& achievement);
    
    // Settings
    void loadSettings();
    void saveSettings();
    void resetAllEasterEggs();

public Q_SLOTS:
    void onKonamiCodeDetected();
    void onShahiCrownTriggered();
    void onSecretMessageTriggered();
    void onHiddenAnimationTriggered();
    void onRetroModeTriggered();
    void onDeveloperModeTriggered();
    void onSecretSettingsTriggered();
    void onFunFactTriggered();
    void onAchievementUnlocked(const QString& achievement);

Q_SIGNALS:
    void easterEggTriggered(EasterEggType type, const QString& message);
    void achievementUnlocked(const QString& achievement);
    void secretModeEnabled(const QString& mode);
    void funFactShown(const QString& fact);

private:
    void setupDefaultEasterEggs();
    void setupKonamiCode();
    void setupShahiCrown();
    void setupSecretMessages();
    void setupHiddenAnimations();
    void setupRetroMode();
    void setupDeveloperMode();
    void setupSecretSettings();
    void setupFunFacts();
    void setupAchievements();
    
    void showCrownAnimation();
    void showMessageAnimation(const QString& message);
    void playAnimationEffect();
    void applyRetroStyling();
    void applyDeveloperStyling();
    void showSecretSettingsDialog();
    void showFunFactDialog();
    void showAchievementDialog(const QString& achievement);
    
    bool checkCooldown(EasterEggType type);
    void updateLastTriggered(EasterEggType type);
    QString getRandomFunFact() const;
    QString getRandomAchievement() const;
    
    // Data
    QMap<EasterEggType, EasterEggData> m_easterEggs;
    QMap<QString, EasterEggType> m_nameToType;
    QList<QKeyEvent*> m_konamiSequence;
    QTimer* m_konamiTimer;
    QTimer* m_cooldownTimer;
    
    // UI Components
    QLabel* m_crownLabel;
    QLabel* m_messageLabel;
    QWidget* m_animationWidget;
    QPropertyAnimation* m_crownAnimation;
    QPropertyAnimation* m_messageAnimation;
    QGraphicsOpacityEffect* m_crownOpacity;
    QGraphicsOpacityEffect* m_messageOpacity;
    
    // Settings
    QSettings* m_settings;
    bool m_easterEggsEnabled;
    bool m_konamiCodeEnabled;
    bool m_retroModeEnabled;
    bool m_developerModeEnabled;
    
    // State
    bool m_crownVisible;
    bool m_retroModeActive;
    bool m_developerModeActive;
    QSet<QString> m_unlockedAchievements;
    
    // Constants
    static constexpr int KONAMI_TIMEOUT = 3000;
    static constexpr int CROWN_DURATION = 2000;
    static constexpr int MESSAGE_DURATION = 3000;
    static constexpr int ANIMATION_DURATION = 1000;
    static constexpr int COOLDOWN_DURATION = 5000;
};

/**
 * @brief Crown Animation Widget
 * 
 * Displays animated crown emoji
 */
class CrownAnimationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CrownAnimationWidget(QWidget* parent = nullptr);

public Q_SLOTS:
    void showCrown();
    void hideCrown();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void setupAnimation();
    
    QPropertyAnimation* m_scaleAnimation;
    QPropertyAnimation* m_rotationAnimation;
    QPropertyAnimation* m_opacityAnimation;
    QGraphicsOpacityEffect* m_opacityEffect;
    bool m_visible;
    qreal m_scale;
    qreal m_rotation;
    qreal m_opacity;
};

/**
 * @brief Secret Message Widget
 * 
 * Displays secret messages with animations
 */
class SecretMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SecretMessageWidget(QWidget* parent = nullptr);

public Q_SLOTS:
    void showMessage(const QString& message);
    void hideMessage();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void setupAnimation();
    
    QPropertyAnimation* m_slideAnimation;
    QPropertyAnimation* m_opacityAnimation;
    QGraphicsOpacityEffect* m_opacityEffect;
    QString m_message;
    bool m_visible;
    qreal m_opacity;
    int m_slideOffset;
};

#endif // SHAHCOIN_QT_EASTEREGG_H
