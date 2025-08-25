// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ui_animations.h"
#include <QSettings>
#include <QDebug>
#include <QApplication>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>

// Static member initialization
bool UIAnimations::m_animationsEnabled = true;
double UIAnimations::m_animationSpeed = 1.0;
QTimer* UIAnimations::m_globalTimer = nullptr;

// AnimationManager singleton instance
AnimationManager* AnimationManager::m_instance = nullptr;

UIAnimations::UIAnimations(QObject* parent)
    : QObject(parent)
{
    if (!m_globalTimer) {
        m_globalTimer = new QTimer(this);
        m_globalTimer->setSingleShot(false);
        m_globalTimer->setInterval(16); // ~60 FPS
    }
    
    loadAnimationSettings();
}

UIAnimations::~UIAnimations()
{
    saveAnimationSettings();
}

void UIAnimations::animateWidget(QWidget* widget, AnimationType type, int duration)
{
    if (!widget || !m_animationsEnabled) return;
    
    duration = static_cast<int>(duration * m_animationSpeed);
    
    switch (type) {
        case FadeIn:
            fadeIn(widget, duration);
            break;
        case FadeOut:
            fadeOut(widget, duration);
            break;
        case SlideIn:
            slideIn(widget, Qt::TopEdge, duration);
            break;
        case SlideOut:
            slideOut(widget, Qt::TopEdge, duration);
            break;
        case ScaleIn:
            scaleIn(widget, duration);
            break;
        case ScaleOut:
            scaleOut(widget, duration);
            break;
        case Bounce:
            bounce(widget, duration);
            break;
        case Pulse:
            pulse(widget, duration);
            break;
        case Shake:
            shake(widget, duration);
            break;
        case Rotate:
            rotate(widget, 360, duration);
            break;
        case Sparkle:
            sparkle(widget, duration);
            break;
        case NFTReveal:
            nftRevealAnimation(widget);
            break;
        case SHAHDropped:
            shahDroppedAnimation(widget, QPoint(0, 0), QPoint(100, 100));
            break;
        case LoadIn:
            loadInAnimation(widget);
            break;
        default:
            break;
    }
}

void UIAnimations::fadeIn(QWidget* widget, int duration)
{
    if (!widget) return;
    
    QGraphicsOpacityEffect* effect = getOrCreateOpacityEffect(widget);
    effect->setOpacity(0.0);
    widget->setGraphicsEffect(effect);
    
    QPropertyAnimation* animation = createPropertyAnimation(effect, "opacity", 0.0, 1.0, duration);
    applyEasingCurve(animation, FadeIn);
    
    connect(animation, &QPropertyAnimation::finished, [widget, effect]() {
        widget->setGraphicsEffect(nullptr);
        effect->deleteLater();
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::fadeOut(QWidget* widget, int duration)
{
    if (!widget) return;
    
    QGraphicsOpacityEffect* effect = getOrCreateOpacityEffect(widget);
    effect->setOpacity(1.0);
    widget->setGraphicsEffect(effect);
    
    QPropertyAnimation* animation = createPropertyAnimation(effect, "opacity", 1.0, 0.0, duration);
    applyEasingCurve(animation, FadeOut);
    
    connect(animation, &QPropertyAnimation::finished, [widget, effect]() {
        widget->setGraphicsEffect(nullptr);
        effect->deleteLater();
        widget->hide();
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::slideIn(QWidget* widget, Qt::Edge edge, int duration)
{
    if (!widget) return;
    
    QRect originalGeometry = widget->geometry();
    QRect startGeometry = originalGeometry;
    
    switch (edge) {
        case Qt::TopEdge:
            startGeometry.moveTop(-originalGeometry.height());
            break;
        case Qt::BottomEdge:
            startGeometry.moveTop(widget->parentWidget()->height());
            break;
        case Qt::LeftEdge:
            startGeometry.moveLeft(-originalGeometry.width());
            break;
        case Qt::RightEdge:
            startGeometry.moveLeft(widget->parentWidget()->width());
            break;
    }
    
    widget->setGeometry(startGeometry);
    widget->show();
    
    QPropertyAnimation* animation = createPropertyAnimation(widget, "geometry", startGeometry, originalGeometry, duration);
    applyEasingCurve(animation, SlideIn);
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::slideOut(QWidget* widget, Qt::Edge edge, int duration)
{
    if (!widget) return;
    
    QRect originalGeometry = widget->geometry();
    QRect endGeometry = originalGeometry;
    
    switch (edge) {
        case Qt::TopEdge:
            endGeometry.moveTop(-originalGeometry.height());
            break;
        case Qt::BottomEdge:
            endGeometry.moveTop(widget->parentWidget()->height());
            break;
        case Qt::LeftEdge:
            endGeometry.moveLeft(-originalGeometry.width());
            break;
        case Qt::RightEdge:
            endGeometry.moveLeft(widget->parentWidget()->width());
            break;
    }
    
    QPropertyAnimation* animation = createPropertyAnimation(widget, "geometry", originalGeometry, endGeometry, duration);
    applyEasingCurve(animation, SlideOut);
    
    connect(animation, &QPropertyAnimation::finished, [widget]() {
        widget->hide();
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::scaleIn(QWidget* widget, int duration)
{
    if (!widget) return;
    
    widget->setScale(0.0);
    widget->show();
    
    QPropertyAnimation* animation = createPropertyAnimation(widget, "scale", 0.0, 1.0, duration);
    applyEasingCurve(animation, ScaleIn);
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::scaleOut(QWidget* widget, int duration)
{
    if (!widget) return;
    
    QPropertyAnimation* animation = createPropertyAnimation(widget, "scale", 1.0, 0.0, duration);
    applyEasingCurve(animation, ScaleOut);
    
    connect(animation, &QPropertyAnimation::finished, [widget]() {
        widget->hide();
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::bounce(QWidget* widget, int duration)
{
    if (!widget) return;
    
    QSequentialAnimationGroup* group = createSequentialGroup();
    
    // Scale up
    QPropertyAnimation* scaleUp = createPropertyAnimation(widget, "scale", 1.0, 1.2, duration / 3);
    applyEasingCurve(scaleUp, Bounce);
    
    // Scale down
    QPropertyAnimation* scaleDown = createPropertyAnimation(widget, "scale", 1.2, 0.9, duration / 3);
    applyEasingCurve(scaleDown, Bounce);
    
    // Scale back to normal
    QPropertyAnimation* scaleNormal = createPropertyAnimation(widget, "scale", 0.9, 1.0, duration / 3);
    applyEasingCurve(scaleNormal, Bounce);
    
    group->addAnimation(scaleUp);
    group->addAnimation(scaleDown);
    group->addAnimation(scaleNormal);
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::pulse(QWidget* widget, int duration)
{
    if (!widget) return;
    
    QPropertyAnimation* animation = createPropertyAnimation(widget, "scale", 1.0, 1.1, duration);
    animation->setLoopCount(-1); // Infinite loop
    animation->setDirection(QAbstractAnimation::ForwardBackward);
    applyEasingCurve(animation, Pulse);
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::shake(QWidget* widget, int duration)
{
    if (!widget) return;
    
    QPoint originalPos = widget->pos();
    QSequentialAnimationGroup* group = createSequentialGroup();
    
    // Create shake sequence
    for (int i = 0; i < 5; ++i) {
        QPropertyAnimation* shakeAnim = createPropertyAnimation(widget, "pos", 
            originalPos, originalPos + QPoint((i % 2 == 0 ? 10 : -10), 0), duration / 10);
        group->addAnimation(shakeAnim);
    }
    
    // Return to original position
    QPropertyAnimation* returnAnim = createPropertyAnimation(widget, "pos", 
        widget->pos(), originalPos, duration / 10);
    group->addAnimation(returnAnim);
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::rotate(QWidget* widget, int degrees, int duration)
{
    if (!widget) return;
    
    QPropertyAnimation* animation = createPropertyAnimation(widget, "rotation", 0, degrees, duration);
    applyEasingCurve(animation, Rotate);
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::sparkle(QWidget* widget, int duration)
{
    if (!widget) return;
    
    // Create sparkle effect with multiple small animations
    QParallelAnimationGroup* group = createParallelGroup();
    
    for (int i = 0; i < 8; ++i) {
        QWidget* sparkle = new QWidget(widget);
        sparkle->setFixedSize(4, 4);
        sparkle->setStyleSheet("background-color: white; border-radius: 2px;");
        
        // Position sparkle around the widget
        int angle = i * 45;
        int radius = 20;
        QPoint center = widget->rect().center();
        QPoint pos = center + QPoint(radius * cos(angle * M_PI / 180), radius * sin(angle * M_PI / 180));
        sparkle->move(pos);
        
        // Animate sparkle
        QPropertyAnimation* sparkleAnim = createPropertyAnimation(sparkle, "geometry", 
            sparkle->geometry(), sparkle->geometry().adjusted(-2, -2, 2, 2), duration / 4);
        sparkleAnim->setLoopCount(4);
        
        connect(sparkleAnim, &QPropertyAnimation::finished, [sparkle]() {
            sparkle->deleteLater();
        });
        
        group->addAnimation(sparkleAnim);
    }
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::nftRevealAnimation(QWidget* widget)
{
    if (!widget) return;
    
    // NFT reveal with glow effect and scale
    QParallelAnimationGroup* group = createParallelGroup();
    
    // Scale animation
    QPropertyAnimation* scaleAnim = createPropertyAnimation(widget, "scale", 0.0, 1.0, 800);
    applyEasingCurve(scaleAnim, ScaleIn);
    
    // Glow effect
    QGraphicsDropShadowEffect* glowEffect = getOrCreateShadowEffect(widget);
    glowEffect->setColor(QColor(255, 215, 0, 150)); // Gold glow
    glowEffect->setBlurRadius(20);
    widget->setGraphicsEffect(glowEffect);
    
    QPropertyAnimation* glowAnim = createPropertyAnimation(glowEffect, "blurRadius", 0, 20, 800);
    applyEasingCurve(glowAnim, FadeIn);
    
    group->addAnimation(scaleAnim);
    group->addAnimation(glowAnim);
    
    connect(group, &QParallelAnimationGroup::finished, [widget, glowEffect]() {
        widget->setGraphicsEffect(nullptr);
        glowEffect->deleteLater();
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::shahDroppedAnimation(QWidget* widget, const QPoint& startPos, const QPoint& endPos)
{
    if (!widget) return;
    
    // SHAH coin dropping animation
    QSequentialAnimationGroup* group = createSequentialGroup();
    
    // Move to start position
    widget->move(startPos);
    widget->show();
    
    // Drop animation
    QPropertyAnimation* dropAnim = createPropertyAnimation(widget, "pos", startPos, endPos, 1000);
    applyEasingCurve(dropAnim, Bounce);
    
    // Scale bounce on landing
    QPropertyAnimation* bounceAnim = createPropertyAnimation(widget, "scale", 1.0, 1.2, 200);
    bounceAnim->setDirection(QAbstractAnimation::ForwardBackward);
    applyEasingCurve(bounceAnim, Bounce);
    
    group->addAnimation(dropAnim);
    group->addAnimation(bounceAnim);
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::sparklineAnimation(QWidget* widget)
{
    if (!widget) return;
    
    // Sparkline chart animation
    QPropertyAnimation* anim = createPropertyAnimation(widget, "geometry", 
        widget->geometry().adjusted(0, widget->height(), 0, 0), widget->geometry(), 600);
    applyEasingCurve(anim, SlideIn);
    
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::loadInAnimation(QWidget* widget)
{
    if (!widget) return;
    
    // Loading animation with fade and slide
    QParallelAnimationGroup* group = createParallelGroup();
    
    // Fade in
    QGraphicsOpacityEffect* effect = getOrCreateOpacityEffect(widget);
    effect->setOpacity(0.0);
    widget->setGraphicsEffect(effect);
    
    QPropertyAnimation* fadeAnim = createPropertyAnimation(effect, "opacity", 0.0, 1.0, 500);
    applyEasingCurve(fadeAnim, FadeIn);
    
    // Slide in from left
    QRect originalGeometry = widget->geometry();
    QRect startGeometry = originalGeometry;
    startGeometry.moveLeft(-originalGeometry.width());
    widget->setGeometry(startGeometry);
    
    QPropertyAnimation* slideAnim = createPropertyAnimation(widget, "geometry", startGeometry, originalGeometry, 500);
    applyEasingCurve(slideAnim, SlideIn);
    
    group->addAnimation(fadeAnim);
    group->addAnimation(slideAnim);
    
    connect(group, &QParallelAnimationGroup::finished, [widget, effect]() {
        widget->setGraphicsEffect(nullptr);
        effect->deleteLater();
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::buttonHoverAnimation(QWidget* widget, bool hovered)
{
    if (!widget) return;
    
    // Button hover effect
    QPropertyAnimation* anim = createPropertyAnimation(widget, "scale", 
        hovered ? 1.0 : 1.05, hovered ? 1.05 : 1.0, 150);
    applyEasingCurve(anim, hovered ? ScaleIn : ScaleOut);
    
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::cardFlipAnimation(QWidget* widget)
{
    if (!widget) return;
    
    // Card flip animation
    QSequentialAnimationGroup* group = createSequentialGroup();
    
    // Flip to 90 degrees
    QPropertyAnimation* flip1 = createPropertyAnimation(widget, "rotation", 0, 90, 200);
    applyEasingCurve(flip1, Rotate);
    
    // Flip to 180 degrees
    QPropertyAnimation* flip2 = createPropertyAnimation(widget, "rotation", 90, 180, 200);
    applyEasingCurve(flip2, Rotate);
    
    group->addAnimation(flip1);
    group->addAnimation(flip2);
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::progressBarAnimation(QWidget* widget, int progress)
{
    if (!widget) return;
    
    // Progress bar fill animation
    QPropertyAnimation* anim = createPropertyAnimation(widget, "maximumWidth", 0, progress, 1000);
    applyEasingCurve(anim, SlideIn);
    
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::notificationAnimation(QWidget* widget, const QString& message)
{
    if (!widget) return;
    
    // Notification popup animation
    QParallelAnimationGroup* group = createParallelGroup();
    
    // Scale in
    widget->setScale(0.0);
    QPropertyAnimation* scaleAnim = createPropertyAnimation(widget, "scale", 0.0, 1.0, 300);
    applyEasingCurve(scaleAnim, ScaleIn);
    
    // Fade in
    QGraphicsOpacityEffect* effect = getOrCreateOpacityEffect(widget);
    effect->setOpacity(0.0);
    widget->setGraphicsEffect(effect);
    
    QPropertyAnimation* fadeAnim = createPropertyAnimation(effect, "opacity", 0.0, 1.0, 300);
    applyEasingCurve(fadeAnim, FadeIn);
    
    group->addAnimation(scaleAnim);
    group->addAnimation(fadeAnim);
    
    // Auto-hide after 3 seconds
    QTimer::singleShot(3000, [widget, effect]() {
        QPropertyAnimation* fadeOut = createPropertyAnimation(effect, "opacity", 1.0, 0.0, 300);
        applyEasingCurve(fadeOut, FadeOut);
        
        connect(fadeOut, &QPropertyAnimation::finished, [widget, effect]() {
            widget->hide();
            widget->setGraphicsEffect(nullptr);
            effect->deleteLater();
        });
        
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::menuSlideAnimation(QWidget* widget, bool show)
{
    if (!widget) return;
    
    // Menu slide animation
    QRect originalGeometry = widget->geometry();
    QRect endGeometry = originalGeometry;
    
    if (show) {
        endGeometry.moveLeft(-originalGeometry.width());
        widget->setGeometry(endGeometry);
        widget->show();
        
        QPropertyAnimation* anim = createPropertyAnimation(widget, "geometry", endGeometry, originalGeometry, 300);
        applyEasingCurve(anim, SlideIn);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        endGeometry.moveLeft(-originalGeometry.width());
        
        QPropertyAnimation* anim = createPropertyAnimation(widget, "geometry", originalGeometry, endGeometry, 300);
        applyEasingCurve(anim, SlideOut);
        
        connect(anim, &QPropertyAnimation::finished, [widget]() {
            widget->hide();
        });
        
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void UIAnimations::tabSwitchAnimation(QWidget* widget, int direction)
{
    if (!widget) return;
    
    // Tab switch animation
    QRect originalGeometry = widget->geometry();
    QRect startGeometry = originalGeometry;
    startGeometry.moveLeft(direction > 0 ? -originalGeometry.width() : originalGeometry.width());
    
    widget->setGeometry(startGeometry);
    widget->show();
    
    QPropertyAnimation* anim = createPropertyAnimation(widget, "geometry", startGeometry, originalGeometry, 250);
    applyEasingCurve(anim, SlideIn);
    
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimations::typingEffect(QLabel* label, const QString& text, int speed)
{
    if (!label) return;
    
    TypingEffectTimer* timer = new TypingEffectTimer(label, text, speed, label);
    timer->startTyping();
    
    connect(timer, &TypingEffectTimer::typingFinished, [timer]() {
        timer->deleteLater();
    });
}

QParallelAnimationGroup* UIAnimations::createParallelGroup()
{
    return new QParallelAnimationGroup();
}

QSequentialAnimationGroup* UIAnimations::createSequentialGroup()
{
    return new QSequentialAnimationGroup();
}

void UIAnimations::setAnimationEnabled(bool enabled)
{
    m_animationsEnabled = enabled;
    AnimationManager::instance()->enableAnimations(enabled);
}

bool UIAnimations::isAnimationEnabled()
{
    return m_animationsEnabled;
}

void UIAnimations::setAnimationSpeed(double speed)
{
    m_animationSpeed = qBound(0.1, speed, 3.0);
    AnimationManager::instance()->setGlobalAnimationSpeed(m_animationSpeed);
}

double UIAnimations::getAnimationSpeed()
{
    return m_animationSpeed;
}

void UIAnimations::loadAnimationSettings()
{
    QSettings settings;
    m_animationsEnabled = settings.value("ui/animationsEnabled", true).toBool();
    m_animationSpeed = settings.value("ui/animationSpeed", 1.0).toDouble();
}

void UIAnimations::saveAnimationSettings()
{
    QSettings settings;
    settings.setValue("ui/animationsEnabled", m_animationsEnabled);
    settings.setValue("ui/animationSpeed", m_animationSpeed);
}

// Helper functions implementation
QPropertyAnimation* UIAnimations::createPropertyAnimation(QWidget* widget, const QByteArray& property, 
    const QVariant& startValue, const QVariant& endValue, int duration)
{
    QPropertyAnimation* animation = new QPropertyAnimation(widget, property);
    animation->setStartValue(startValue);
    animation->setEndValue(endValue);
    animation->setDuration(duration);
    
    AnimationManager::instance()->registerAnimation(animation);
    
    connect(animation, &QPropertyAnimation::finished, [animation]() {
        AnimationManager::instance()->unregisterAnimation(animation);
    });
    
    return animation;
}

QGraphicsOpacityEffect* UIAnimations::getOrCreateOpacityEffect(QWidget* widget)
{
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
    }
    return effect;
}

QGraphicsDropShadowEffect* UIAnimations::getOrCreateShadowEffect(QWidget* widget)
{
    QGraphicsDropShadowEffect* effect = qobject_cast<QGraphicsDropShadowEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsDropShadowEffect(widget);
        effect->setBlurRadius(10);
        effect->setColor(QColor(0, 0, 0, 80));
        effect->setOffset(0, 0);
    }
    return effect;
}

void UIAnimations::applyEasingCurve(QPropertyAnimation* animation, AnimationType type)
{
    animation->setEasingCurve(getEasingCurve(type));
}

QEasingCurve UIAnimations::getEasingCurve(AnimationType type)
{
    switch (type) {
        case FadeIn:
            return QEasingCurve::OutQuad;
        case FadeOut:
            return QEasingCurve::InQuad;
        case SlideIn:
            return QEasingCurve::OutBack;
        case SlideOut:
            return QEasingCurve::InBack;
        case ScaleIn:
            return QEasingCurve::OutElastic;
        case ScaleOut:
            return QEasingCurve::InElastic;
        case Bounce:
            return QEasingCurve::OutBounce;
        case Pulse:
            return QEasingCurve::InOutSine;
        case Shake:
            return QEasingCurve::InOutQuad;
        case Rotate:
            return QEasingCurve::OutCubic;
        case Sparkle:
            return QEasingCurve::OutQuart;
        default:
            return QEasingCurve::OutQuad;
    }
}

// Animation presets implementation
void UIAnimations::applyNFTRevealPreset(QWidget* widget)
{
    nftRevealAnimation(widget);
}

void UIAnimations::applySHAHDroppedPreset(QWidget* widget, const QPoint& startPos, const QPoint& endPos)
{
    shahDroppedAnimation(widget, startPos, endPos);
}

void UIAnimations::applySparklinePreset(QWidget* widget)
{
    sparklineAnimation(widget);
}

void UIAnimations::applyLoadInPreset(QWidget* widget)
{
    loadInAnimation(widget);
}

void UIAnimations::applyButtonHoverPreset(QWidget* widget, bool hovered)
{
    buttonHoverAnimation(widget, hovered);
}

void UIAnimations::applyCardFlipPreset(QWidget* widget)
{
    cardFlipAnimation(widget);
}

void UIAnimations::applyProgressBarPreset(QWidget* widget, int progress)
{
    progressBarAnimation(widget, progress);
}

void UIAnimations::applyNotificationPreset(QWidget* widget, const QString& message)
{
    notificationAnimation(widget, message);
}

void UIAnimations::applyMenuSlidePreset(QWidget* widget, bool show)
{
    menuSlideAnimation(widget, show);
}

void UIAnimations::applyTabSwitchPreset(QWidget* widget, int direction)
{
    tabSwitchAnimation(widget, direction);
}

// AnimationManager implementation
AnimationManager* AnimationManager::instance()
{
    if (!m_instance) {
        m_instance = new AnimationManager();
    }
    return m_instance;
}

AnimationManager::AnimationManager(QObject* parent)
    : QObject(parent)
    , m_animationsEnabled(true)
    , m_animationSpeed(1.0)
    , m_themeBasedAnimations(true)
{
    loadSettings();
}

AnimationManager::~AnimationManager()
{
    saveSettings();
}

void AnimationManager::enableAnimations(bool enabled)
{
    if (m_animationsEnabled != enabled) {
        m_animationsEnabled = enabled;
        emit animationsEnabledChanged(enabled);
        saveSettings();
    }
}

bool AnimationManager::animationsEnabled() const
{
    return m_animationsEnabled;
}

void AnimationManager::setGlobalAnimationSpeed(double speed)
{
    if (m_animationSpeed != speed) {
        m_animationSpeed = qBound(0.1, speed, 3.0);
        emit animationSpeedChanged(m_animationSpeed);
        saveSettings();
    }
}

double AnimationManager::globalAnimationSpeed() const
{
    return m_animationSpeed;
}

void AnimationManager::pauseAllAnimations()
{
    for (QPropertyAnimation* animation : m_activeAnimations) {
        animation->pause();
    }
}

void AnimationManager::resumeAllAnimations()
{
    for (QPropertyAnimation* animation : m_activeAnimations) {
        animation->resume();
    }
}

void AnimationManager::stopAllAnimations()
{
    for (QPropertyAnimation* animation : m_activeAnimations) {
        animation->stop();
    }
    m_activeAnimations.clear();
}

void AnimationManager::registerAnimation(QPropertyAnimation* animation)
{
    if (animation && !m_activeAnimations.contains(animation)) {
        m_activeAnimations.append(animation);
        emit animationStarted(animation);
        
        connect(animation, &QPropertyAnimation::finished, [this, animation]() {
            unregisterAnimation(animation);
        });
    }
}

void AnimationManager::unregisterAnimation(QPropertyAnimation* animation)
{
    if (animation && m_activeAnimations.contains(animation)) {
        m_activeAnimations.removeOne(animation);
        emit animationFinished(animation);
    }
}

void AnimationManager::setThemeBasedAnimations(bool enabled)
{
    m_themeBasedAnimations = enabled;
    saveSettings();
}

bool AnimationManager::themeBasedAnimations() const
{
    return m_themeBasedAnimations;
}

void AnimationManager::loadSettings()
{
    QSettings settings;
    m_animationsEnabled = settings.value("animations/enabled", true).toBool();
    m_animationSpeed = settings.value("animations/speed", 1.0).toDouble();
    m_themeBasedAnimations = settings.value("animations/themeBased", true).toBool();
}

void AnimationManager::saveSettings()
{
    QSettings settings;
    settings.setValue("animations/enabled", m_animationsEnabled);
    settings.setValue("animations/speed", m_animationSpeed);
    settings.setValue("animations/themeBased", m_themeBasedAnimations);
}

// TypingEffectTimer implementation
TypingEffectTimer::TypingEffectTimer(QLabel* label, const QString& text, int speed, QObject* parent)
    : QTimer(parent)
    , m_label(label)
    , m_fullText(text)
    , m_currentText("")
    , m_currentIndex(0)
    , m_speed(speed)
{
    connect(this, &QTimer::timeout, this, &TypingEffectTimer::onTimeout);
    setInterval(m_speed);
}

void TypingEffectTimer::startTyping()
{
    m_currentText = "";
    m_currentIndex = 0;
    m_label->setText("");
    start();
}

void TypingEffectTimer::stopTyping()
{
    stop();
    m_label->setText(m_fullText);
}

void TypingEffectTimer::reset()
{
    stop();
    m_currentText = "";
    m_currentIndex = 0;
    m_label->setText("");
}

void TypingEffectTimer::onTimeout()
{
    if (m_currentIndex < m_fullText.length()) {
        m_currentText += m_fullText[m_currentIndex];
        m_label->setText(m_currentText);
        m_currentIndex++;
    } else {
        stop();
        emit typingFinished();
    }
}
