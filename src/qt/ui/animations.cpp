#include "animations.h"
#include <QDebug>
#include <QApplication>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QSettings>

// UIAnimationManager Singleton
UIAnimationManager* UIAnimationManager::m_instance = nullptr;

UIAnimationManager* UIAnimationManager::instance()
{
    if (!m_instance) {
        m_instance = new UIAnimationManager();
    }
    return m_instance;
}

UIAnimationManager::UIAnimationManager(QObject* parent)
    : QObject(parent)
    , m_settings("Shahcoin", "UIAnimations")
    , m_allAnimationsEnabled(true)
    , m_lowResourceMode(false)
{
    setupDefaultConfigs();
    loadSettings();
}

UIAnimationManager::~UIAnimationManager()
{
    saveSettings();
    stopAllAnimations();
}

void UIAnimationManager::setupDefaultConfigs()
{
    // Setup default animation configurations
    AnimationConfig config;
    config.enabled = true;
    config.duration = 1000;
    config.easingCurve = QEasingCurve::OutCubic;
    config.repeat = false;
    config.repeatCount = 1;
    config.autoReverse = false;
    config.primaryColor = QColor(255, 215, 0);
    config.secondaryColor = QColor(255, 255, 255);
    config.opacity = 1.0;
    config.offset = QPoint(0, 0);
    config.scale = QSize(1, 1);
    
    m_animationConfigs[AnimationType::NFT_REVEAL] = config;
    m_animationConfigs[AnimationType::TOKEN_CREATION_SPARKLINE] = config;
    m_animationConfigs[AnimationType::PORTFOLIO_LOAD_IN] = config;
    m_animationConfigs[AnimationType::SHAH_DROPPED] = config;
}

void UIAnimationManager::loadSettings()
{
    m_allAnimationsEnabled = m_settings.value("allAnimationsEnabled", true).toBool();
    m_lowResourceMode = m_settings.value("lowResourceMode", false).toBool();
}

void UIAnimationManager::saveSettings()
{
    m_settings.setValue("allAnimationsEnabled", m_allAnimationsEnabled);
    m_settings.setValue("lowResourceMode", m_lowResourceMode);
}

void UIAnimationManager::startAnimation(AnimationType type, QWidget* target, const QVariantMap& params)
{
    if (!target || shouldSkipAnimation(type)) {
        return;
    }
    
    createAnimation(type, target, params);
    emit animationStarted(type, target);
}

void UIAnimationManager::createAnimation(AnimationType type, QWidget* target, const QVariantMap& params)
{
    QPair<AnimationType, QWidget*> key = qMakePair(type, target);
    
    // Clean up existing animation
    if (m_runningAnimations.contains(key)) {
        cleanupAnimation(type, target);
    }
    
    QObject* animation = nullptr;
    
    switch (type) {
        case AnimationType::NFT_REVEAL:
            animation = createNFTRevealAnimation(target, params);
            break;
        case AnimationType::TOKEN_CREATION_SPARKLINE:
            animation = createTokenSparklineAnimation(target, params);
            break;
        case AnimationType::PORTFOLIO_LOAD_IN:
            animation = createPortfolioLoadAnimation(target, params);
            break;
        case AnimationType::SHAH_DROPPED:
            animation = createShahDroppedAnimation(target, params);
            break;
        default:
            animation = createGenericAnimation(target, type, params);
            break;
    }
    
    if (animation) {
        m_runningAnimations[key] = animation;
        connect(animation, &QObject::destroyed, this, [this, key]() {
            m_runningAnimations.remove(key);
        });
    }
}

QObject* UIAnimationManager::createNFTRevealAnimation(QWidget* target, const QVariantMap& params)
{
    bool isRare = params.value("isRare", false).toBool();
    
    // Create opacity animation
    QPropertyAnimation* opacityAnim = new QPropertyAnimation(target, "windowOpacity", this);
    opacityAnim->setDuration(800);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);
    opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // Create scale animation
    QPropertyAnimation* scaleAnim = new QPropertyAnimation(target, "geometry", this);
    scaleAnim->setDuration(600);
    scaleAnim->setEasingCurve(QEasingCurve::OutBack);
    
    // Create glow effect for rare NFTs
    if (isRare) {
        QGraphicsDropShadowEffect* glowEffect = new QGraphicsDropShadowEffect(target);
        glowEffect->setBlurRadius(20);
        glowEffect->setColor(QColor(255, 215, 0));
        glowEffect->setOffset(0, 0);
        target->setGraphicsEffect(glowEffect);
    }
    
    // Group animations
    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
    group->addAnimation(opacityAnim);
    group->addAnimation(scaleAnim);
    
    group->start();
    return group;
}

QObject* UIAnimationManager::createTokenSparklineAnimation(QWidget* target, const QVariantMap& params)
{
    // Create sparkline effect
    QGraphicsDropShadowEffect* sparklineEffect = new QGraphicsDropShadowEffect(target);
    sparklineEffect->setBlurRadius(15);
    sparklineEffect->setColor(QColor(0, 255, 127));
    sparklineEffect->setOffset(0, 0);
    target->setGraphicsEffect(sparklineEffect);
    
    // Create animation
    QPropertyAnimation* anim = new QPropertyAnimation(sparklineEffect, "blurRadius", this);
    anim->setDuration(800);
    anim->setStartValue(5);
    anim->setEndValue(25);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    
    anim->start();
    return anim;
}

QObject* UIAnimationManager::createPortfolioLoadAnimation(QWidget* target, const QVariantMap& params)
{
    // Create slide and fade animation
    QPropertyAnimation* slideAnim = new QPropertyAnimation(target, "geometry", this);
    slideAnim->setDuration(600);
    slideAnim->setEasingCurve(QEasingCurve::OutQuart);
    
    QPropertyAnimation* opacityAnim = new QPropertyAnimation(target, "windowOpacity", this);
    opacityAnim->setDuration(400);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);
    opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // Create bounce effect
    QPropertyAnimation* bounceAnim = new QPropertyAnimation(target, "geometry", this);
    bounceAnim->setDuration(300);
    bounceAnim->setEasingCurve(QEasingCurve::OutBounce);
    
    // Group animations
    QParallelAnimationGroup* mainGroup = new QParallelAnimationGroup(this);
    mainGroup->addAnimation(slideAnim);
    mainGroup->addAnimation(opacityAnim);
    
    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
    group->addAnimation(mainGroup);
    group->addAnimation(bounceAnim);
    
    group->start();
    return group;
}

QObject* UIAnimationManager::createShahDroppedAnimation(QWidget* target, const QVariantMap& params)
{
    double amount = params.value("amount", 0.0).toDouble();
    
    // Create drop animation
    QPropertyAnimation* dropAnim = new QPropertyAnimation(target, "geometry", this);
    dropAnim->setDuration(800);
    dropAnim->setEasingCurve(QEasingCurve::OutBounce);
    
    // Create bounce animation
    QPropertyAnimation* bounceAnim = new QPropertyAnimation(target, "geometry", this);
    bounceAnim->setDuration(400);
    bounceAnim->setEasingCurve(QEasingCurve::OutElastic);
    
    // Create sparkle effect
    QGraphicsDropShadowEffect* sparkleEffect = new QGraphicsDropShadowEffect(target);
    sparkleEffect->setBlurRadius(15);
    sparkleEffect->setColor(QColor(255, 215, 0));
    sparkleEffect->setOffset(0, 0);
    target->setGraphicsEffect(sparkleEffect);
    
    // Group animations
    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
    group->addAnimation(dropAnim);
    group->addAnimation(bounceAnim);
    
    group->start();
    return group;
}

QObject* UIAnimationManager::createGenericAnimation(QWidget* target, AnimationType type, const QVariantMap& params)
{
    QPropertyAnimation* anim = new QPropertyAnimation(target, "geometry", this);
    anim->setDuration(getAnimationDuration(type));
    anim->setEasingCurve(getConfig(type).easingCurve);
    anim->start();
    return anim;
}

void UIAnimationManager::stopAnimation(AnimationType type, QWidget* target)
{
    QPair<AnimationType, QWidget*> key = qMakePair(type, target);
    
    if (m_runningAnimations.contains(key)) {
        cleanupAnimation(type, target);
    }
}

void UIAnimationManager::stopAllAnimations()
{
    for (auto it = m_runningAnimations.begin(); it != m_runningAnimations.end(); ++it) {
        if (it.value()) {
            it.value()->deleteLater();
        }
    }
    m_runningAnimations.clear();
}

void UIAnimationManager::cleanupAnimation(AnimationType type, QWidget* target)
{
    QPair<AnimationType, QWidget*> key = qMakePair(type, target);
    
    if (m_runningAnimations.contains(key)) {
        QObject* animation = m_runningAnimations[key];
        if (animation) {
            animation->deleteLater();
        }
        m_runningAnimations.remove(key);
    }
}

bool UIAnimationManager::shouldSkipAnimation(AnimationType type) const
{
    if (!m_allAnimationsEnabled) return true;
    if (m_lowResourceMode) return true;
    
    auto it = m_animationConfigs.find(type);
    if (it != m_animationConfigs.end()) {
        return !it.value().enabled;
    }
    
    return false;
}

// Specific animation methods
void UIAnimationManager::startNFTReveal(QWidget* target, bool isRare)
{
    QVariantMap params;
    params["isRare"] = isRare;
    startAnimation(AnimationType::NFT_REVEAL, target, params);
}

void UIAnimationManager::startTokenCreationSparkline(QWidget* target)
{
    startAnimation(AnimationType::TOKEN_CREATION_SPARKLINE, target);
}

void UIAnimationManager::startPortfolioLoadIn(QWidget* target)
{
    startAnimation(AnimationType::PORTFOLIO_LOAD_IN, target);
}

void UIAnimationManager::startShahDropped(QWidget* target, double amount)
{
    QVariantMap params;
    params["amount"] = amount;
    startAnimation(AnimationType::SHAH_DROPPED, target, params);
}

// Settings methods
void UIAnimationManager::setAnimationEnabled(AnimationType type, bool enabled)
{
    auto it = m_animationConfigs.find(type);
    if (it != m_animationConfigs.end()) {
        it.value().enabled = enabled;
        saveSettings();
        emit settingsChanged();
    }
}

void UIAnimationManager::setAllAnimationsEnabled(bool enabled)
{
    m_allAnimationsEnabled = enabled;
    saveSettings();
    emit settingsChanged();
}

void UIAnimationManager::setLowResourceMode(bool enabled)
{
    m_lowResourceMode = enabled;
    saveSettings();
    emit settingsChanged();
}

bool UIAnimationManager::isAnimationEnabled(AnimationType type) const
{
    auto it = m_animationConfigs.find(type);
    if (it != m_animationConfigs.end()) {
        return it.value().enabled;
    }
    return false;
}

bool UIAnimationManager::areAllAnimationsEnabled() const
{
    return m_allAnimationsEnabled;
}

bool UIAnimationManager::isLowResourceMode() const
{
    return m_lowResourceMode;
}

int UIAnimationManager::getAnimationDuration(AnimationType type) const
{
    auto it = m_animationConfigs.find(type);
    if (it != m_animationConfigs.end()) {
        return it.value().duration;
    }
    return 1000; // Default duration
}

AnimationConfig UIAnimationManager::getConfig(AnimationType type) const
{
    auto it = m_animationConfigs.find(type);
    if (it != m_animationConfigs.end()) {
        return it.value();
    }
    return AnimationConfig();
}

// AnimatedWidget Implementation
AnimatedWidget::AnimatedWidget(QWidget* parent)
    : QWidget(parent)
    , m_animateInOnShow(true)
    , m_animateOutOnHide(true)
    , m_defaultInAnimation(AnimationType::FADE_IN)
    , m_defaultOutAnimation(AnimationType::FADE_IN)
    , m_defaultUpdateAnimation(AnimationType::BOUNCE)
{
    setupDefaultAnimations();
}

AnimatedWidget::~AnimatedWidget()
{
}

void AnimatedWidget::setupDefaultAnimations()
{
    // Default animations can be customized here
}

void AnimatedWidget::animateIn(AnimationType type)
{
    UIAnimationManager::instance()->startAnimation(type, this);
}

void AnimatedWidget::animateOut(AnimationType type)
{
    UIAnimationManager::instance()->startAnimation(type, this);
}

void AnimatedWidget::animateUpdate(AnimationType type)
{
    UIAnimationManager::instance()->startAnimation(type, this);
}

void AnimatedWidget::animateNFTReveal(bool isRare)
{
    UIAnimationManager::instance()->startNFTReveal(this, isRare);
}

void AnimatedWidget::animateTokenCreation()
{
    UIAnimationManager::instance()->startTokenCreationSparkline(this);
}

void AnimatedWidget::animatePortfolioLoad()
{
    UIAnimationManager::instance()->startPortfolioLoadIn(this);
}

void AnimatedWidget::animateShahReceived(double amount)
{
    UIAnimationManager::instance()->startShahDropped(this, amount);
}

void AnimatedWidget::animateBalanceUpdate()
{
    UIAnimationManager::instance()->startAnimation(AnimationType::BALANCE_GLOW, this);
}

void AnimatedWidget::animateCardFlip()
{
    UIAnimationManager::instance()->startAnimation(AnimationType::CARD_FLIP, this);
}

void AnimatedWidget::animateSparkle()
{
    UIAnimationManager::instance()->startAnimation(AnimationType::SPARKLE, this);
}

void AnimatedWidget::animateConfetti()
{
    UIAnimationManager::instance()->startAnimation(AnimationType::CONFETTI, this);
}

void AnimatedWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    
    if (m_animateInOnShow) {
        animateIn(m_defaultInAnimation);
    }
}

void AnimatedWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    
    if (m_animateOutOnHide) {
        animateOut(m_defaultOutAnimation);
    }
}

void AnimatedWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}
