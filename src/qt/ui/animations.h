#ifndef UI_ANIMATIONS_H
#define UI_ANIMATIONS_H

#include <QObject>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QSettings>
#include <QMap>
#include <QList>
#include <QString>
#include <QColor>
#include <QPoint>
#include <QSize>
#include <QPropertyAnimation>
#include <QEasingCurve>

// Animation types
enum class AnimationType {
    NFT_REVEAL,
    TOKEN_CREATION_SPARKLINE,
    PORTFOLIO_LOAD_IN,
    SHAH_DROPPED,
    BALANCE_GLOW,
    CARD_FLIP,
    FADE_IN,
    SLIDE_IN,
    BOUNCE,
    SPARKLE,
    CONFETTI
};

// Animation configuration
struct AnimationConfig {
    bool enabled;
    int duration;
    QEasingCurve easingCurve;
    bool repeat;
    int repeatCount;
    bool autoReverse;
    QColor primaryColor;
    QColor secondaryColor;
    double opacity;
    QPoint offset;
    QSize scale;
};

// NFT Reveal Animation
class NFTRevealAnimation : public QObject
{
    Q_OBJECT

public:
    explicit NFTRevealAnimation(QWidget* target, QObject* parent = nullptr);
    ~NFTRevealAnimation();

    void startAnimation(bool isRare = false);
    void stopAnimation();
    bool isRunning() const;

    void setCardFlip(bool enabled);
    void setConfettiEnabled(bool enabled);
    void setShineEffect(bool enabled);
    void setRarityColors(const QColor& common, const QColor& rare, const QColor& legendary);

signals:
    void animationStarted();
    void animationFinished();
    void cardFlipped();

private slots:
    void onCardFlipFinished();
    void onConfettiFinished();
    void onShineFinished();

private:
    QWidget* m_target;
    QPropertyAnimation* m_flipAnimation;
    QPropertyAnimation* m_opacityAnimation;
    QPropertyAnimation* m_scaleAnimation;
    QParallelAnimationGroup* m_mainGroup;
    QSequentialAnimationGroup* m_confettiGroup;
    QTimer* m_confettiTimer;
    QTimer* m_shineTimer;
    
    QGraphicsDropShadowEffect* m_shadowEffect;
    QGraphicsOpacityEffect* m_opacityEffect;
    
    bool m_isRunning;
    bool m_cardFlipEnabled;
    bool m_confettiEnabled;
    bool m_shineEnabled;
    
    QColor m_commonColor;
    QColor m_rareColor;
    QColor m_legendaryColor;
    
    void setupAnimations();
    void createConfettiParticles();
    void createShineEffect();
    void updateRarityColors(bool isRare);
};

// Token Creation Sparkline Animation
class TokenCreationSparklineAnimation : public QObject
{
    Q_OBJECT

public:
    explicit TokenCreationSparklineAnimation(QWidget* target, QObject* parent = nullptr);
    ~TokenCreationSparklineAnimation();

    void startAnimation();
    void stopAnimation();
    bool isRunning() const;

    void setSparklineColor(const QColor& color);
    void setWaveformEnabled(bool enabled);
    void setSparkleEffect(bool enabled);

signals:
    void animationStarted();
    void animationFinished();

private slots:
    void onSparklineFinished();
    void onWaveformFinished();
    void onSparkleFinished();

private:
    QWidget* m_target;
    QPropertyAnimation* m_sparklineAnimation;
    QPropertyAnimation* m_waveformAnimation;
    QParallelAnimationGroup* m_sparkleGroup;
    QTimer* m_sparkleTimer;
    
    QGraphicsDropShadowEffect* m_glowEffect;
    
    bool m_isRunning;
    bool m_waveformEnabled;
    bool m_sparkleEnabled;
    QColor m_sparklineColor;
    
    void setupAnimations();
    void createSparklineEffect();
    void createWaveformEffect();
    void createSparkleEffect();
};

// Portfolio Load-in Animation
class PortfolioLoadInAnimation : public QObject
{
    Q_OBJECT

public:
    explicit PortfolioLoadInAnimation(QWidget* target, QObject* parent = nullptr);
    ~PortfolioLoadInAnimation();

    void startAnimation();
    void stopAnimation();
    bool isRunning() const;

    void setSlideDirection(Qt::Orientation direction);
    void setBounceEffect(bool enabled);
    void setGlowEffect(bool enabled);

signals:
    void animationStarted();
    void animationFinished();

private slots:
    void onSlideFinished();
    void onBounceFinished();
    void onGlowFinished();

private:
    QWidget* m_target;
    QPropertyAnimation* m_slideAnimation;
    QPropertyAnimation* m_opacityAnimation;
    QPropertyAnimation* m_bounceAnimation;
    QPropertyAnimation* m_glowAnimation;
    QParallelAnimationGroup* m_mainGroup;
    QSequentialAnimationGroup* m_bounceGroup;
    
    QGraphicsOpacityEffect* m_opacityEffect;
    QGraphicsDropShadowEffect* m_glowEffect;
    
    bool m_isRunning;
    bool m_bounceEnabled;
    bool m_glowEnabled;
    Qt::Orientation m_slideDirection;
    
    void setupAnimations();
    void createSlideEffect();
    void createBounceEffect();
    void createGlowEffect();
};

// SHAH Dropped Animation
class ShahDroppedAnimation : public QObject
{
    Q_OBJECT

public:
    explicit ShahDroppedAnimation(QWidget* target, QObject* parent = nullptr);
    ~ShahDroppedAnimation();

    void startAnimation(double amount = 0.0);
    void stopAnimation();
    bool isRunning() const;

    void setCoinDropEnabled(bool enabled);
    void setSparkleEffect(bool enabled);
    void setThrottleEnabled(bool enabled);
    void setThrottleDelay(int milliseconds);

signals:
    void animationStarted();
    void animationFinished();

private slots:
    void onCoinDropFinished();
    void onSparkleFinished();
    void onThrottleTimeout();

private:
    QWidget* m_target;
    QPropertyAnimation* m_dropAnimation;
    QPropertyAnimation* m_bounceAnimation;
    QPropertyAnimation* m_sparkleAnimation;
    QParallelAnimationGroup* m_sparkleGroup;
    QSequentialAnimationGroup* m_mainGroup;
    QTimer* m_sparkleTimer;
    QTimer* m_throttleTimer;
    
    QGraphicsDropShadowEffect* m_shadowEffect;
    QGraphicsOpacityEffect* m_opacityEffect;
    
    bool m_isRunning;
    bool m_coinDropEnabled;
    bool m_sparkleEnabled;
    bool m_throttleEnabled;
    int m_throttleDelay;
    double m_lastAmount;
    
    void setupAnimations();
    void createCoinDropEffect();
    void createSparkleEffect();
    void throttleAnimation();
};

// Main Animation Manager
class UIAnimationManager : public QObject
{
    Q_OBJECT

public:
    static UIAnimationManager* instance();
    
    explicit UIAnimationManager(QObject* parent = nullptr);
    ~UIAnimationManager();

    // Core functionality
    void initialize();
    void loadSettings();
    void saveSettings();

    // Animation controls
    void startAnimation(AnimationType type, QWidget* target, const QVariantMap& params = QVariantMap());
    void stopAnimation(AnimationType type, QWidget* target);
    void stopAllAnimations();
    bool isAnimationRunning(AnimationType type, QWidget* target) const;

    // Settings
    void setAnimationEnabled(AnimationType type, bool enabled);
    void setAllAnimationsEnabled(bool enabled);
    void setAnimationDuration(AnimationType type, int duration);
    void setAnimationEasing(AnimationType type, QEasingCurve::Type easing);
    void setLowResourceMode(bool enabled);

    bool isAnimationEnabled(AnimationType type) const;
    bool areAllAnimationsEnabled() const;
    int getAnimationDuration(AnimationType type) const;
    QEasingCurve::Type getAnimationEasing(AnimationType type) const;
    bool isLowResourceMode() const;

    // Specific animation methods
    void startNFTReveal(QWidget* target, bool isRare = false);
    void startTokenCreationSparkline(QWidget* target);
    void startPortfolioLoadIn(QWidget* target);
    void startShahDropped(QWidget* target, double amount = 0.0);
    void startBalanceGlow(QWidget* target);
    void startCardFlip(QWidget* target);
    void startFadeIn(QWidget* target);
    void startSlideIn(QWidget* target, Qt::Orientation direction = Qt::Horizontal);
    void startBounce(QWidget* target);
    void startSparkle(QWidget* target);
    void startConfetti(QWidget* target);

signals:
    void animationStarted(AnimationType type, QWidget* target);
    void animationFinished(AnimationType type, QWidget* target);
    void settingsChanged();

private slots:
    void onAnimationFinished();
    void onLowResourceModeChanged();

private:
    static UIAnimationManager* m_instance;
    
    QSettings m_settings;
    bool m_allAnimationsEnabled;
    bool m_lowResourceMode;
    
    QMap<AnimationType, AnimationConfig> m_animationConfigs;
    QMap<QPair<AnimationType, QWidget*>, QObject*> m_runningAnimations;
    
    NFTRevealAnimation* m_nftRevealAnim;
    TokenCreationSparklineAnimation* m_tokenSparklineAnim;
    PortfolioLoadInAnimation* m_portfolioLoadAnim;
    ShahDroppedAnimation* m_shahDroppedAnim;
    
    void setupDefaultConfigs();
    void createAnimation(AnimationType type, QWidget* target, const QVariantMap& params);
    void cleanupAnimation(AnimationType type, QWidget* target);
    AnimationConfig getConfig(AnimationType type) const;
    void setConfig(AnimationType type, const AnimationConfig& config);
    bool shouldSkipAnimation(AnimationType type) const;
    void logAnimation(const QString& message);
};

// Animation Widget Mixin
class AnimatedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnimatedWidget(QWidget* parent = nullptr);
    ~AnimatedWidget();

    // Animation methods
    void animateIn(AnimationType type = AnimationType::FADE_IN);
    void animateOut(AnimationType type = AnimationType::FADE_IN);
    void animateUpdate(AnimationType type = AnimationType::BOUNCE);
    
    // Specific animations
    void animateNFTReveal(bool isRare = false);
    void animateTokenCreation();
    void animatePortfolioLoad();
    void animateShahReceived(double amount = 0.0);
    void animateBalanceUpdate();
    void animateCardFlip();
    void animateSparkle();
    void animateConfetti();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    bool m_animateInOnShow;
    bool m_animateOutOnHide;
    AnimationType m_defaultInAnimation;
    AnimationType m_defaultOutAnimation;
    AnimationType m_defaultUpdateAnimation;
    
    void setupDefaultAnimations();
};

#endif // UI_ANIMATIONS_H
