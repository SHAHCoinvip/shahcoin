// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_UI_ANIMATIONS_H
#define SHAHCOIN_QT_UI_ANIMATIONS_H

#include <QObject>
#include <QWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QApplication>
#include <QScreen>

class UIAnimations : public QObject
{
    Q_OBJECT

public:
    explicit UIAnimations(QObject* parent = nullptr);
    ~UIAnimations();

    // Animation types
    enum AnimationType {
        FadeIn,
        FadeOut,
        SlideIn,
        SlideOut,
        ScaleIn,
        ScaleOut,
        Bounce,
        Pulse,
        Shake,
        Rotate,
        Sparkle,
        NFTReveal,
        SHAHDropped,
        LoadIn,
        TypingEffect
    };

    // Animation presets
    enum AnimationPreset {
        NFTRevealAnimation,
        SHAHDroppedAnimation,
        SparklineAnimation,
        LoadInAnimation,
        ButtonHoverAnimation,
        CardFlipAnimation,
        ProgressBarAnimation,
        NotificationAnimation,
        MenuSlideAnimation,
        TabSwitchAnimation
    };

    // Main animation functions
    static void animateWidget(QWidget* widget, AnimationType type, int duration = 300);
    static void animateWidget(QWidget* widget, AnimationPreset preset);
    static void animateProperty(QWidget* widget, const QByteArray& property, const QVariant& startValue, const QVariant& endValue, int duration = 300);
    
    // Specific animation functions
    static void fadeIn(QWidget* widget, int duration = 300);
    static void fadeOut(QWidget* widget, int duration = 300);
    static void slideIn(QWidget* widget, Qt::Edge edge = Qt::TopEdge, int duration = 300);
    static void slideOut(QWidget* widget, Qt::Edge edge = Qt::TopEdge, int duration = 300);
    static void scaleIn(QWidget* widget, int duration = 300);
    static void scaleOut(QWidget* widget, int duration = 300);
    static void bounce(QWidget* widget, int duration = 500);
    static void pulse(QWidget* widget, int duration = 1000);
    static void shake(QWidget* widget, int duration = 500);
    static void rotate(QWidget* widget, int degrees = 360, int duration = 1000);
    static void sparkle(QWidget* widget, int duration = 2000);
    
    // Special animations
    static void nftRevealAnimation(QWidget* widget);
    static void shahDroppedAnimation(QWidget* widget, const QPoint& startPos, const QPoint& endPos);
    static void sparklineAnimation(QWidget* widget);
    static void loadInAnimation(QWidget* widget);
    static void buttonHoverAnimation(QWidget* widget, bool hovered);
    static void cardFlipAnimation(QWidget* widget);
    static void progressBarAnimation(QWidget* widget, int progress);
    static void notificationAnimation(QWidget* widget, const QString& message);
    static void menuSlideAnimation(QWidget* widget, bool show);
    static void tabSwitchAnimation(QWidget* widget, int direction = 1);
    
    // Typing effect
    static void typingEffect(QLabel* label, const QString& text, int speed = 50);
    
    // Animation groups
    static QParallelAnimationGroup* createParallelGroup();
    static QSequentialAnimationGroup* createSequentialGroup();
    
    // Utility functions
    static void setAnimationEnabled(bool enabled);
    static bool isAnimationEnabled();
    static void setAnimationSpeed(double speed);
    static double getAnimationSpeed();
    
    // Settings
    static void loadAnimationSettings();
    static void saveAnimationSettings();

private:
    static bool m_animationsEnabled;
    static double m_animationSpeed;
    static QTimer* m_globalTimer;
    
    // Helper functions
    static QPropertyAnimation* createPropertyAnimation(QWidget* widget, const QByteArray& property, const QVariant& startValue, const QVariant& endValue, int duration);
    static QGraphicsOpacityEffect* getOrCreateOpacityEffect(QWidget* widget);
    static QGraphicsDropShadowEffect* getOrCreateShadowEffect(QWidget* widget);
    static void applyEasingCurve(QPropertyAnimation* animation, AnimationType type);
    static QEasingCurve getEasingCurve(AnimationType type);
    
    // Animation presets implementation
    static void applyNFTRevealPreset(QWidget* widget);
    static void applySHAHDroppedPreset(QWidget* widget, const QPoint& startPos, const QPoint& endPos);
    static void applySparklinePreset(QWidget* widget);
    static void applyLoadInPreset(QWidget* widget);
    static void applyButtonHoverPreset(QWidget* widget, bool hovered);
    static void applyCardFlipPreset(QWidget* widget);
    static void applyProgressBarPreset(QWidget* widget, int progress);
    static void applyNotificationPreset(QWidget* widget, const QString& message);
    static void applyMenuSlidePreset(QWidget* widget, bool show);
    static void applyTabSwitchPreset(QWidget* widget, int direction);
};

// Animation manager for global animation control
class AnimationManager : public QObject
{
    Q_OBJECT

public:
    static AnimationManager* instance();
    
    void enableAnimations(bool enabled);
    bool animationsEnabled() const;
    
    void setGlobalAnimationSpeed(double speed);
    double globalAnimationSpeed() const;
    
    void pauseAllAnimations();
    void resumeAllAnimations();
    void stopAllAnimations();
    
    void registerAnimation(QPropertyAnimation* animation);
    void unregisterAnimation(QPropertyAnimation* animation);
    
    void setThemeBasedAnimations(bool enabled);
    bool themeBasedAnimations() const;

signals:
    void animationStarted(QPropertyAnimation* animation);
    void animationFinished(QPropertyAnimation* animation);
    void animationsEnabledChanged(bool enabled);
    void animationSpeedChanged(double speed);

private:
    explicit AnimationManager(QObject* parent = nullptr);
    ~AnimationManager();
    
    static AnimationManager* m_instance;
    bool m_animationsEnabled;
    double m_animationSpeed;
    bool m_themeBasedAnimations;
    QList<QPropertyAnimation*> m_activeAnimations;
    
    void loadSettings();
    void saveSettings();
};

// Typing effect timer
class TypingEffectTimer : public QTimer
{
    Q_OBJECT

public:
    explicit TypingEffectTimer(QLabel* label, const QString& text, int speed = 50, QObject* parent = nullptr);
    
    void startTyping();
    void stopTyping();
    void reset();

signals:
    void typingFinished();

private slots:
    void onTimeout();

private:
    QLabel* m_label;
    QString m_fullText;
    QString m_currentText;
    int m_currentIndex;
    int m_speed;
};

#endif // SHAHCOIN_QT_UI_ANIMATIONS_H
