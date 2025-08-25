// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/shahcoin-config.h>
#endif

#include "splashscreen.h"

#include <qt/networkstyle.h>
#include <qt/guiutil.h>
#include <qt/guiconstants.h>

#include <clientversion.h>
#include <interfaces/handler.h>
#include <interfaces/node.h>
#include <interfaces/wallet.h>
#include <util/system.h>
#include <util/translation.h>

#include <QApplication>
#include <QCloseEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QScreen>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QFontDatabase>
#include <QFontMetrics>

#include <memory>

SplashScreen::SplashScreen(Qt::WindowFlags f, const NetworkStyle *networkStyle) :
    QWidget(nullptr, f), curAlignment(0), curColor(0), m_node(nullptr), m_shutdown(false),
    m_network_style(networkStyle)
{
    // Set window properties
    setWindowTitle(tr("SHAHCOIN Core"));
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // Create layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);
    
    // Create UI elements
    m_icon_label = new QLabel(this);
    m_icon_label->setAlignment(Qt::AlignCenter);
    m_icon_label->setPixmap(m_network_style->getSplashIcon().pixmap(128, 128));
    
    m_title_label = new QLabel(tr("SHAHCOIN Core"), this);
    m_title_label->setAlignment(Qt::AlignCenter);
    m_title_label->setStyleSheet("QLabel { color: #1E3A8A; font-size: 32px; font-weight: bold; }");
    
    m_tagline_label = new QLabel(tr("Sovereign Crypto for a New Era"), this);
    m_tagline_label->setAlignment(Qt::AlignCenter);
    m_tagline_label->setStyleSheet("QLabel { color: #6B7280; font-size: 16px; font-style: italic; }");
    
    m_version_label = new QLabel(QString("v%1").arg(QString::fromStdString(FormatFullVersion())), this);
    m_version_label->setAlignment(Qt::AlignCenter);
    m_version_label->setStyleSheet("QLabel { color: #9CA3AF; font-size: 12px; }");
    
    m_network_label = new QLabel(m_network_style->getTitleAddText(), this);
    m_network_label->setAlignment(Qt::AlignCenter);
    m_network_label->setStyleSheet("QLabel { color: #F59E0B; font-size: 14px; font-weight: bold; }");
    
    m_progress_bar = new QProgressBar(this);
    m_progress_bar->setRange(0, 100);
    m_progress_bar->setValue(0);
    m_progress_bar->setTextVisible(false);
    m_progress_bar->setStyleSheet(
        "QProgressBar {"
        "   border: 2px solid #E5E7EB;"
        "   border-radius: 10px;"
        "   background-color: #F8FAFC;"
        "   height: 8px;"
        "}"
        "QProgressBar::chunk {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 #1E3A8A, stop:1 #3B82F6);"
        "   border-radius: 8px;"
        "}"
    );
    
    m_status_label = new QLabel(tr("Initializing..."), this);
    m_status_label->setAlignment(Qt::AlignCenter);
    m_status_label->setStyleSheet("QLabel { color: #6B7280; font-size: 14px; }");
    
    m_loading_label = new QLabel("", this);
    m_loading_label->setAlignment(Qt::AlignCenter);
    m_loading_label->setStyleSheet("QLabel { color: #9CA3AF; font-size: 12px; }");
    
    // Add widgets to layout
    mainLayout->addStretch();
    mainLayout->addWidget(m_icon_label);
    mainLayout->addWidget(m_title_label);
    mainLayout->addWidget(m_tagline_label);
    mainLayout->addWidget(m_version_label);
    mainLayout->addWidget(m_network_label);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_progress_bar);
    mainLayout->addWidget(m_status_label);
    mainLayout->addWidget(m_loading_label);
    mainLayout->addStretch();
    
    // Setup animations
    m_fade_animation = new QPropertyAnimation(this, "windowOpacity");
    m_fade_animation->setDuration(500);
    m_fade_animation->setStartValue(0.0);
    m_fade_animation->setEndValue(1.0);
    
    m_progress_animation = new QPropertyAnimation(m_progress_bar, "value");
    m_progress_animation->setDuration(300);
    
    // Setup loading timer
    m_loading_timer = new QTimer(this);
    connect(m_loading_timer, &QTimer::timeout, [this]() {
        m_loading_dots = (m_loading_dots + 1) % 4;
        QString dots = QString(".").repeated(m_loading_dots);
        m_loading_label->setText(dots);
    });
    m_loading_timer->start(500);
    
    // Center on screen
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
    
    // Start fade in animation
    m_fade_animation->start();
    
    // Subscribe to core signals
    subscribeToCoreSignals();
}

SplashScreen::~SplashScreen()
{
    unsubscribeFromCoreSignals();
}

void SplashScreen::finish()
{
    if (m_closed) return;
    
    m_closed = true;
    
    // Fade out animation
    QPropertyAnimation* fadeOut = new QPropertyAnimation(this, "windowOpacity");
    fadeOut->setDuration(300);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    
    connect(fadeOut, &QPropertyAnimation::finished, [this]() {
        close();
        deleteLater();
    });
    
    fadeOut->start();
}

void SplashScreen::showMessage(const QString &message, int alignment, const QColor &color)
{
    curMessage = message;
    curAlignment = alignment;
    curColor = color;
    m_status_label->setText(message);
    update();
}

void SplashScreen::setBreakAction(const std::function<void(void)> &action)
{
    breakAction = action;
}

void SplashScreen::setProgress(int progress)
{
    m_progress = progress;
    m_progress_animation->setStartValue(m_progress_bar->value());
    m_progress_animation->setEndValue(progress);
    m_progress_animation->start();
}

void SplashScreen::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    
    // Create gradient background
    QRadialGradient gradient(rect().center(), width() / 2);
    gradient.setColorAt(0, QColor(248, 250, 252));  // Light gray
    gradient.setColorAt(1, QColor(255, 255, 255));  // White
    
    painter.fillRect(rect(), gradient);
    
    // Draw border
    painter.setPen(QPen(QColor(229, 231, 235), 2));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 15, 15);
}

void SplashScreen::closeEvent(QCloseEvent *event)
{
    if (breakAction) {
        breakAction();
    }
    event->accept();
}

void SplashScreen::subscribeToCoreSignals()
{
    // Connect to core signals for progress updates
    // This will be implemented when connecting to the actual core
}

void SplashScreen::unsubscribeFromCoreSignals()
{
    // Disconnect from core signals
    // This will be implemented when connecting to the actual core
}

void SplashScreen::connectWallet(WalletModel* walletModel)
{
    // Connect wallet model signals
    // This will be implemented when connecting to the actual wallet model
}

void SplashScreen::connectBlockchain()
{
    // Connect blockchain signals
    // This will be implemented when connecting to the actual blockchain
}

void SplashScreen::showProgress(const QString &title, int nProgress)
{
    setProgress(nProgress);
    showMessage(title, Qt::AlignCenter | Qt::AlignHCenter, QColor(102, 102, 102));
}

void SplashScreen::setVisible(bool visible)
{
    m_show = visible;
    QWidget::setVisible(visible);
}

void SplashScreen::alignWidget(QWidget *widget, const QWidget *host, int alignment, int dx, int dy)
{
    // Align widget relative to host widget
    // This will be implemented if needed
}
