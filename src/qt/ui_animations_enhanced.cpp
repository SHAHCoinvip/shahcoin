#include "ui_animations.h"
#include <QDebug>
#include <QApplication>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QTimer>
#include <QSettings>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsPolygonItem>
#include <QPolygonF>
#include <QTransform>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QSoundEffect>
#include <QMediaPlayer>

// Enhanced UI Animation Manager Implementation
class EnhancedUIAnimationManager : public QObject
{
    Q_OBJECT

public:
    static EnhancedUIAnimationManager* instance();
    
    // Core Animation Methods
    void startNFTRevealAnimation(QWidget* target, bool isRare = false, const QString& nftName = "");
    void startTokenCreationSparkline(QWidget* target, const QString& tokenName = "");
    void startPortfolioLoadAnimation(QWidget* target);
    void startShahDroppedAnimation(QWidget* target, double amount, const QString& source = "");
    
    // Settings
    void setAnimationsEnabled(bool enabled);
    void setLowResourceMode(bool enabled);
    bool areAnimationsEnabled() const;
    bool isLowResourceMode() const;

private:
    explicit EnhancedUIAnimationManager(QObject* parent = nullptr);
    ~EnhancedUIAnimationManager();
    
    // Animation Creation Methods
    QPropertyAnimation* createNFTRevealCardFlip(QWidget* target, bool isRare);
    QPropertyAnimation* createNFTRevealConfetti(QWidget* target);
    QPropertyAnimation* createTokenSparklineEffect(QWidget* target);
    QPropertyAnimation* createPortfolioSlideIn(QWidget* target);
    QPropertyAnimation* createPortfolioBalanceGlow(QWidget* target);
    QPropertyAnimation* createShahCoinDrop(QWidget* target, double amount);
    QPropertyAnimation* createShahSparkleEffect(QWidget* target);
    
    // Helper Methods
    void setupConfettiParticles(QWidget* target);
    void setupSparklineWaveform(QWidget* target);
    void setupCoinDropPhysics(QWidget* target, double amount);
    void playAnimationSound(const QString& soundType);
    
    // Settings
    QSettings m_settings;
    bool m_animationsEnabled;
    bool m_lowResourceMode;
    bool m_soundEnabled;
    
    // Animation Groups
    QMap<QWidget*, QParallelAnimationGroup*> m_runningAnimations;
};

// Singleton Instance
EnhancedUIAnimationManager* EnhancedUIAnimationManager::instance()
{
    static EnhancedUIAnimationManager* instance = nullptr;
    if (!instance) {
        instance = new EnhancedUIAnimationManager(QApplication::instance());
    }
    return instance;
}

EnhancedUIAnimationManager::EnhancedUIAnimationManager(QObject* parent)
    : QObject(parent)
    , m_settings("Shahcoin", "UIAnimations")
    , m_animationsEnabled(true)
    , m_lowResourceMode(false)
    , m_soundEnabled(true)
{
    loadSettings();
}

EnhancedUIAnimationManager::~EnhancedUIAnimationManager()
{
    // Clean up running animations
    for (auto group : m_runningAnimations.values()) {
        if (group) {
            group->stop();
            group->deleteLater();
        }
    }
    m_runningAnimations.clear();
}

void EnhancedUIAnimationManager::loadSettings()
{
    m_animationsEnabled = m_settings.value("animationsEnabled", true).toBool();
    m_lowResourceMode = m_settings.value("lowResourceMode", false).toBool();
    m_soundEnabled = m_settings.value("soundEnabled", true).toBool();
}

// 🎉 NFT Reveal Animation Implementation
void EnhancedUIAnimationManager::startNFTRevealAnimation(QWidget* target, bool isRare, const QString& nftName)
{
    if (!m_animationsEnabled || m_lowResourceMode) return;
    
    qDebug() << "[EnhancedUIAnimationManager] Starting NFT Reveal Animation for:" << nftName;
    
    // Create main animation group
    QParallelAnimationGroup* mainGroup = new QParallelAnimationGroup(this);
    
    // 1. Card Flip Animation
    QPropertyAnimation* cardFlip = createNFTRevealCardFlip(target, isRare);
    mainGroup->addAnimation(cardFlip);
    
    // 2. Confetti Effect for Rare NFTs
    if (isRare) {
        QPropertyAnimation* confetti = createNFTRevealConfetti(target);
        mainGroup->addAnimation(confetti);
        
        // Play rare NFT sound
        if (m_soundEnabled) {
            playAnimationSound("rare_nft");
        }
    }
    
    // 3. Glow Effect
    QGraphicsDropShadowEffect* glowEffect = new QGraphicsDropShadowEffect(target);
    glowEffect->setBlurRadius(20);
    glowEffect->setColor(isRare ? QColor(255, 215, 0) : QColor(255, 255, 255));
    glowEffect->setOffset(0, 0);
    target->setGraphicsEffect(glowEffect);
    
    // Glow animation
    QPropertyAnimation* glowAnim = new QPropertyAnimation(glowEffect, "blurRadius", this);
    glowAnim->setDuration(1500);
    glowAnim->setStartValue(5);
    glowAnim->setEndValue(25);
    glowAnim->setEasingCurve(QEasingCurve::OutCubic);
    mainGroup->addAnimation(glowAnim);
    
    // Store animation group
    m_runningAnimations[target] = mainGroup;
    
    // Connect finished signal
    connect(mainGroup, &QParallelAnimationGroup::finished, this, [this, target, glowEffect]() {
        target->setGraphicsEffect(nullptr);
        delete glowEffect;
        m_runningAnimations.remove(target);
    });
    
    mainGroup->start();
}

QPropertyAnimation* EnhancedUIAnimationManager::createNFTRevealCardFlip(QWidget* target, bool isRare)
{
    // Create 3D flip effect using geometry animation
    QPropertyAnimation* flipAnim = new QPropertyAnimation(target, "geometry", this);
    flipAnim->setDuration(800);
    flipAnim->setEasingCurve(QEasingCurve::OutBack);
    
    QRect startRect = target->geometry();
    QRect endRect = startRect;
    
    // Scale effect for flip
    endRect.setWidth(startRect.width() * 1.1);
    endRect.setHeight(startRect.height() * 1.1);
    endRect.moveCenter(startRect.center());
    
    flipAnim->setStartValue(startRect);
    flipAnim->setEndValue(endRect);
    
    return flipAnim;
}

QPropertyAnimation* EnhancedUIAnimationManager::createNFTRevealConfetti(QWidget* target)
{
    // Create confetti scene
    QGraphicsScene* confettiScene = new QGraphicsScene(this);
    QGraphicsView* confettiView = new QGraphicsView(confettiScene, target);
    confettiView->setRenderHint(QPainter::Antialiasing);
    confettiView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    confettiView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    confettiView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    confettiView->setStyleSheet("background: transparent; border: none;");
    confettiView->setFrameStyle(0);
    
    // Create confetti particles
    for (int i = 0; i < 50; ++i) {
        QGraphicsEllipseItem* particle = new QGraphicsEllipseItem();
        particle->setRect(0, 0, 4, 4);
        
        // Random colors for confetti
        QColor colors[] = {
            QColor(255, 215, 0),  // Gold
            QColor(255, 255, 255), // White
            QColor(255, 0, 0),     // Red
            QColor(0, 255, 0),     // Green
            QColor(0, 0, 255),     // Blue
            QColor(255, 165, 0)    // Orange
        };
        particle->setBrush(colors[i % 6]);
        particle->setPen(Qt::NoPen);
        
        // Random position
        particle->setPos(
            qrand() % target->width(),
            qrand() % target->height()
        );
        
        confettiScene->addItem(particle);
        
        // Animate each particle
        QPropertyAnimation* particleAnim = new QPropertyAnimation(particle, "pos", this);
        particleAnim->setDuration(2000 + qrand() % 1000);
        particleAnim->setStartValue(particle->pos());
        particleAnim->setEndValue(QPointF(
            particle->pos().x() + (qrand() % 200 - 100),
            particle->pos().y() + 200
        ));
        particleAnim->setEasingCurve(QEasingCurve::OutBounce);
        particleAnim->start();
        
        // Clean up particle after animation
        connect(particleAnim, &QPropertyAnimation::finished, [particle, particleAnim]() {
            delete particle;
            delete particleAnim;
        });
    }
    
    confettiView->setGeometry(target->rect());
    confettiView->show();
    
    // Create animation for confetti view
    QPropertyAnimation* confettiAnim = new QPropertyAnimation(confettiView, "windowOpacity", this);
    confettiAnim->setDuration(3000);
    confettiAnim->setStartValue(1.0);
    confettiAnim->setEndValue(0.0);
    confettiAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // Clean up confetti view after animation
    connect(confettiAnim, &QPropertyAnimation::finished, [confettiView, confettiScene]() {
        delete confettiView;
        delete confettiScene;
    });
    
    return confettiAnim;
}

// 🔥 Token Creation Sparkline Effect Implementation
void EnhancedUIAnimationManager::startTokenCreationSparkline(QWidget* target, const QString& tokenName)
{
    if (!m_animationsEnabled || m_lowResourceMode) return;
    
    qDebug() << "[EnhancedUIAnimationManager] Starting Token Creation Sparkline for:" << tokenName;
    
    // Create sparkline effect
    QGraphicsDropShadowEffect* sparklineEffect = new QGraphicsDropShadowEffect(target);
    sparklineEffect->setBlurRadius(15);
    sparklineEffect->setColor(QColor(0, 255, 127)); // Green spark
    sparklineEffect->setOffset(0, 0);
    target->setGraphicsEffect(sparklineEffect);
    
    // Create sparkline animation
    QPropertyAnimation* sparklineAnim = createTokenSparklineEffect(target);
    
    // Create waveform animation
    QSequentialAnimationGroup* waveformGroup = new QSequentialAnimationGroup(this);
    
    // Multiple spark waves
    for (int i = 0; i < 3; ++i) {
        QPropertyAnimation* waveAnim = new QPropertyAnimation(sparklineEffect, "blurRadius", this);
        waveAnim->setDuration(300);
        waveAnim->setStartValue(5);
        waveAnim->setEndValue(25);
        waveAnim->setEasingCurve(QEasingCurve::OutCubic);
        waveformGroup->addAnimation(waveAnim);
        
        // Add delay between waves
        if (i < 2) {
            QPropertyAnimation* delayAnim = new QPropertyAnimation(this, "objectName", this);
            delayAnim->setDuration(100);
            waveformGroup->addAnimation(delayAnim);
        }
    }
    
    // Store animation
    m_runningAnimations[target] = waveformGroup;
    
    // Connect finished signal
    connect(waveformGroup, &QSequentialAnimationGroup::finished, this, [this, target, sparklineEffect]() {
        target->setGraphicsEffect(nullptr);
        delete sparklineEffect;
        m_runningAnimations.remove(target);
    });
    
    // Play success sound
    if (m_soundEnabled) {
        playAnimationSound("token_created");
    }
    
    waveformGroup->start();
}

QPropertyAnimation* EnhancedUIAnimationManager::createTokenSparklineEffect(QWidget* target)
{
    // Create sparkline drawing
    QPixmap sparklinePixmap(target->size());
    sparklinePixmap.fill(Qt::transparent);
    
    QPainter painter(&sparklinePixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw sparkline waveform
    QPainterPath sparklinePath;
    sparklinePath.moveTo(0, target->height() / 2);
    
    for (int x = 0; x < target->width(); x += 10) {
        double y = target->height() / 2 + 
                   sin(x * 0.1) * 20 + 
                   sin(x * 0.05) * 10;
        sparklinePath.lineTo(x, y);
    }
    
    // Create gradient for sparkline
    QLinearGradient gradient(0, 0, target->width(), 0);
    gradient.setColorAt(0, QColor(0, 255, 127, 0));
    gradient.setColorAt(0.5, QColor(0, 255, 127, 255));
    gradient.setColorAt(1, QColor(0, 255, 127, 0));
    
    painter.setPen(QPen(gradient, 3));
    painter.drawPath(sparklinePath);
    
    // Create overlay widget
    QLabel* sparklineOverlay = new QLabel(target);
    sparklineOverlay->setPixmap(sparklinePixmap);
    sparklineOverlay->setAlignment(Qt::AlignCenter);
    sparklineOverlay->setGeometry(target->rect());
    sparklineOverlay->show();
    
    // Animate overlay
    QPropertyAnimation* overlayAnim = new QPropertyAnimation(sparklineOverlay, "windowOpacity", this);
    overlayAnim->setDuration(1000);
    overlayAnim->setStartValue(1.0);
    overlayAnim->setEndValue(0.0);
    overlayAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // Clean up overlay after animation
    connect(overlayAnim, &QPropertyAnimation::finished, [sparklineOverlay]() {
        delete sparklineOverlay;
    });
    
    return overlayAnim;
}

// 💼 Portfolio Load-in Animation Implementation
void EnhancedUIAnimationManager::startPortfolioLoadAnimation(QWidget* target)
{
    if (!m_animationsEnabled || m_lowResourceMode) return;
    
    qDebug() << "[EnhancedUIAnimationManager] Starting Portfolio Load Animation";
    
    // Create main animation group
    QSequentialAnimationGroup* mainGroup = new QSequentialAnimationGroup(this);
    
    // 1. Slide in animation
    QPropertyAnimation* slideAnim = createPortfolioSlideIn(target);
    mainGroup->addAnimation(slideAnim);
    
    // 2. Balance glow effect
    QPropertyAnimation* glowAnim = createPortfolioBalanceGlow(target);
    mainGroup->addAnimation(glowAnim);
    
    // Store animation
    m_runningAnimations[target] = mainGroup;
    
    // Connect finished signal
    connect(mainGroup, &QSequentialAnimationGroup::finished, this, [this, target]() {
        m_runningAnimations.remove(target);
    });
    
    mainGroup->start();
}

QPropertyAnimation* EnhancedUIAnimationManager::createPortfolioSlideIn(QWidget* target)
{
    // Store original geometry
    QRect originalRect = target->geometry();
    
    // Start from off-screen (left)
    QRect startRect = originalRect;
    startRect.moveLeft(-originalRect.width());
    
    // Create slide animation
    QPropertyAnimation* slideAnim = new QPropertyAnimation(target, "geometry", this);
    slideAnim->setDuration(600);
    slideAnim->setStartValue(startRect);
    slideAnim->setEndValue(originalRect);
    slideAnim->setEasingCurve(QEasingCurve::OutQuart);
    
    // Add fade effect
    QPropertyAnimation* fadeAnim = new QPropertyAnimation(target, "windowOpacity", this);
    fadeAnim->setDuration(400);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // Group slide and fade
    QParallelAnimationGroup* slideGroup = new QParallelAnimationGroup(this);
    slideGroup->addAnimation(slideAnim);
    slideGroup->addAnimation(fadeAnim);
    
    return slideGroup;
}

QPropertyAnimation* EnhancedUIAnimationManager::createPortfolioBalanceGlow(QWidget* target)
{
    // Create glow effect
    QGraphicsDropShadowEffect* glowEffect = new QGraphicsDropShadowEffect(target);
    glowEffect->setBlurRadius(20);
    glowEffect->setColor(QColor(255, 215, 0)); // Gold glow
    glowEffect->setOffset(0, 0);
    target->setGraphicsEffect(glowEffect);
    
    // Create glow animation
    QPropertyAnimation* glowAnim = new QPropertyAnimation(glowEffect, "blurRadius", this);
    glowAnim->setDuration(800);
    glowAnim->setStartValue(5);
    glowAnim->setEndValue(30);
    glowAnim->setEasingCurve(QEasingCurve::OutBounce);
    
    // Clean up effect after animation
    connect(glowAnim, &QPropertyAnimation::finished, [target, glowEffect]() {
        target->setGraphicsEffect(nullptr);
        delete glowEffect;
    });
    
    return glowAnim;
}

// 🪙 SHAH Dropped Animation Implementation
void EnhancedUIAnimationManager::startShahDroppedAnimation(QWidget* target, double amount, const QString& source)
{
    if (!m_animationsEnabled || m_lowResourceMode) return;
    
    qDebug() << "[EnhancedUIAnimationManager] Starting SHAH Dropped Animation for:" << amount << "SHAH from" << source;
    
    // Create main animation group
    QSequentialAnimationGroup* mainGroup = new QSequentialAnimationGroup(this);
    
    // 1. Coin drop animation
    QPropertyAnimation* dropAnim = createShahCoinDrop(target, amount);
    mainGroup->addAnimation(dropAnim);
    
    // 2. Sparkle effect
    QPropertyAnimation* sparkleAnim = createShahSparkleEffect(target);
    mainGroup->addAnimation(sparkleAnim);
    
    // Store animation
    m_runningAnimations[target] = mainGroup;
    
    // Connect finished signal
    connect(mainGroup, &QSequentialAnimationGroup::finished, this, [this, target]() {
        m_runningAnimations.remove(target);
    });
    
    // Play coin drop sound
    if (m_soundEnabled) {
        playAnimationSound("shah_dropped");
    }
    
    mainGroup->start();
}

QPropertyAnimation* EnhancedUIAnimationManager::createShahCoinDrop(QWidget* target, double amount)
{
    // Create coin icon
    QLabel* coinIcon = new QLabel(target);
    coinIcon->setText("🪙");
    coinIcon->setStyleSheet("font-size: 24px; color: #FFD700;");
    coinIcon->setAlignment(Qt::AlignCenter);
    coinIcon->setGeometry(0, 0, 50, 50);
    coinIcon->show();
    
    // Create drop animation
    QPropertyAnimation* dropAnim = new QPropertyAnimation(coinIcon, "geometry", this);
    dropAnim->setDuration(1000);
    dropAnim->setStartValue(QRect(0, -50, 50, 50));
    dropAnim->setEndValue(QRect(0, target->height() - 50, 50, 50));
    dropAnim->setEasingCurve(QEasingCurve::OutBounce);
    
    // Add bounce effect
    QPropertyAnimation* bounceAnim = new QPropertyAnimation(coinIcon, "geometry", this);
    bounceAnim->setDuration(300);
    bounceAnim->setStartValue(QRect(0, target->height() - 50, 50, 50));
    bounceAnim->setEndValue(QRect(0, target->height() - 80, 50, 50));
    bounceAnim->setEasingCurve(QEasingCurve::OutElastic);
    
    // Group drop and bounce
    QSequentialAnimationGroup* dropGroup = new QSequentialAnimationGroup(this);
    dropGroup->addAnimation(dropAnim);
    dropGroup->addAnimation(bounceAnim);
    
    // Clean up coin icon after animation
    connect(dropGroup, &QSequentialAnimationGroup::finished, [coinIcon]() {
        delete coinIcon;
    });
    
    return dropGroup;
}

QPropertyAnimation* EnhancedUIAnimationManager::createShahSparkleEffect(QWidget* target)
{
    // Create sparkle scene
    QGraphicsScene* sparkleScene = new QGraphicsScene(this);
    QGraphicsView* sparkleView = new QGraphicsView(sparkleScene, target);
    sparkleView->setRenderHint(QPainter::Antialiasing);
    sparkleView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    sparkleView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sparkleView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sparkleView->setStyleSheet("background: transparent; border: none;");
    sparkleView->setFrameStyle(0);
    
    // Create sparkle particles
    for (int i = 0; i < 20; ++i) {
        QGraphicsEllipseItem* sparkle = new QGraphicsEllipseItem();
        sparkle->setRect(0, 0, 3, 3);
        sparkle->setBrush(QColor(255, 215, 0)); // Gold sparkles
        sparkle->setPen(Qt::NoPen);
        
        // Random position around the target
        sparkle->setPos(
            qrand() % target->width(),
            qrand() % target->height()
        );
        
        sparkleScene->addItem(sparkle);
        
        // Animate each sparkle
        QPropertyAnimation* sparkleAnim = new QPropertyAnimation(sparkle, "pos", this);
        sparkleAnim->setDuration(1000 + qrand() % 500);
        sparkleAnim->setStartValue(sparkle->pos());
        sparkleAnim->setEndValue(QPointF(
            sparkle->pos().x() + (qrand() % 100 - 50),
            sparkle->pos().y() + (qrand() % 100 - 50)
        ));
        sparkleAnim->setEasingCurve(QEasingCurve::OutCubic);
        sparkleAnim->start();
        
        // Clean up sparkle after animation
        connect(sparkleAnim, &QPropertyAnimation::finished, [sparkle, sparkleAnim]() {
            delete sparkle;
            delete sparkleAnim;
        });
    }
    
    sparkleView->setGeometry(target->rect());
    sparkleView->show();
    
    // Create animation for sparkle view
    QPropertyAnimation* sparkleViewAnim = new QPropertyAnimation(sparkleView, "windowOpacity", this);
    sparkleViewAnim->setDuration(1500);
    sparkleViewAnim->setStartValue(1.0);
    sparkleViewAnim->setEndValue(0.0);
    sparkleViewAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // Clean up sparkle view after animation
    connect(sparkleViewAnim, &QPropertyAnimation::finished, [sparkleView, sparkleScene]() {
        delete sparkleView;
        delete sparkleScene;
    });
    
    return sparkleViewAnim;
}

// Sound Effects
void EnhancedUIAnimationManager::playAnimationSound(const QString& soundType)
{
    if (!m_soundEnabled) return;
    
    // Create sound effect based on type
    QSoundEffect* soundEffect = new QSoundEffect(this);
    
    QString soundFile;
    if (soundType == "rare_nft") {
        soundFile = ":/sounds/rare_nft.wav";
    } else if (soundType == "token_created") {
        soundFile = ":/sounds/token_created.wav";
    } else if (soundType == "shah_dropped") {
        soundFile = ":/sounds/coin_drop.wav";
    }
    
    if (!soundFile.isEmpty()) {
        soundEffect->setSource(QUrl::fromLocalFile(soundFile));
        soundEffect->setVolume(0.5);
        soundEffect->play();
        
        // Clean up sound effect after playing
        connect(soundEffect, &QSoundEffect::playingChanged, [soundEffect]() {
            if (!soundEffect->isPlaying()) {
                delete soundEffect;
            }
        });
    }
}

// Settings Methods
void EnhancedUIAnimationManager::setAnimationsEnabled(bool enabled)
{
    m_animationsEnabled = enabled;
    m_settings.setValue("animationsEnabled", enabled);
}

void EnhancedUIAnimationManager::setLowResourceMode(bool enabled)
{
    m_lowResourceMode = enabled;
    m_settings.setValue("lowResourceMode", enabled);
}

bool EnhancedUIAnimationManager::areAnimationsEnabled() const
{
    return m_animationsEnabled;
}

bool EnhancedUIAnimationManager::isLowResourceMode() const
{
    return m_lowResourceMode;
}

// Global Animation Functions (for easy access)
namespace UIAnimations {
    void startNFTReveal(QWidget* target, bool isRare, const QString& nftName) {
        EnhancedUIAnimationManager::instance()->startNFTRevealAnimation(target, isRare, nftName);
    }
    
    void startTokenCreationSparkline(QWidget* target, const QString& tokenName) {
        EnhancedUIAnimationManager::instance()->startTokenCreationSparkline(target, tokenName);
    }
    
    void startPortfolioLoad(QWidget* target) {
        EnhancedUIAnimationManager::instance()->startPortfolioLoadAnimation(target);
    }
    
    void startShahDropped(QWidget* target, double amount, const QString& source) {
        EnhancedUIAnimationManager::instance()->startShahDroppedAnimation(target, amount, source);
    }
    
    void setAnimationsEnabled(bool enabled) {
        EnhancedUIAnimationManager::instance()->setAnimationsEnabled(enabled);
    }
    
    void setLowResourceMode(bool enabled) {
        EnhancedUIAnimationManager::instance()->setLowResourceMode(enabled);
    }
    
    bool areAnimationsEnabled() {
        return EnhancedUIAnimationManager::instance()->areAnimationsEnabled();
    }
    
    bool isLowResourceMode() {
        return EnhancedUIAnimationManager::instance()->isLowResourceMode();
    }
}
