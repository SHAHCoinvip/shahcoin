#include "aboutdialog.h"

#include <qt/networkstyle.h>
#include <qt/guiutil.h>
#include <qt/guiconstants.h>

#include <clientversion.h>
#include <util/system.h>
#include <util/translation.h>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QUrl>
#include <QScreen>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QStyle>
#include <QStyleFactory>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About SHAHCOIN Core"));
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    setFixedSize(600, 500);
    
    setupUI();
    setupBranding();
    setupVersionInfo();
    setupCredits();
    setupLicense();
    applyTheme();
    
    // Center on screen
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);
    
    // Branding section
    QHBoxLayout* brandingLayout = new QHBoxLayout();
    brandingLayout->setSpacing(20);
    
    m_logo_label = new QLabel(this);
    m_logo_label->setFixedSize(80, 80);
    m_logo_label->setAlignment(Qt::AlignCenter);
    
    QVBoxLayout* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(5);
    
    m_title_label = new QLabel(tr("SHAHCOIN Core"), this);
    m_title_label->setStyleSheet("QLabel { color: #1E3A8A; font-size: 24px; font-weight: bold; }");
    
    m_tagline_label = new QLabel(tr("Sovereign Crypto for a New Era"), this);
    m_tagline_label->setStyleSheet("QLabel { color: #6B7280; font-size: 14px; font-style: italic; }");
    
    titleLayout->addWidget(m_title_label);
    titleLayout->addWidget(m_tagline_label);
    titleLayout->addStretch();
    
    brandingLayout->addWidget(m_logo_label);
    brandingLayout->addLayout(titleLayout);
    brandingLayout->addStretch();
    
    // Version info section
    QHBoxLayout* versionLayout = new QHBoxLayout();
    versionLayout->setSpacing(10);
    
    m_version_label = new QLabel(this);
    m_version_label->setStyleSheet("QLabel { color: #374151; font-size: 12px; }");
    
    m_build_label = new QLabel(this);
    m_build_label->setStyleSheet("QLabel { color: #9CA3AF; font-size: 11px; }");
    
    m_copy_version_button = new QPushButton(tr("Copy"), this);
    m_copy_version_button->setFixedSize(60, 25);
    m_copy_version_button->setStyleSheet(
        "QPushButton {"
        "   background-color: #1E3A8A;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3B82F6;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1E40AF;"
        "}"
    );
    
    versionLayout->addWidget(m_version_label);
    versionLayout->addWidget(m_build_label);
    versionLayout->addStretch();
    versionLayout->addWidget(m_copy_version_button);
    
    // Action buttons
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    
    m_github_button = new QPushButton(tr("GitHub"), this);
    m_github_button->setFixedSize(80, 30);
    m_github_button->setStyleSheet(
        "QPushButton {"
        "   background-color: #374151;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #4B5563;"
        "}"
    );
    
    m_docs_button = new QPushButton(tr("Documentation"), this);
    m_docs_button->setFixedSize(100, 30);
    m_docs_button->setStyleSheet(
        "QPushButton {"
        "   background-color: #059669;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #10B981;"
        "}"
    );
    
    m_license_button = new QPushButton(tr("License"), this);
    m_license_button->setFixedSize(80, 30);
    m_license_button->setStyleSheet(
        "QPushButton {"
        "   background-color: #DC2626;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #EF4444;"
        "}"
    );
    
    actionLayout->addWidget(m_github_button);
    actionLayout->addWidget(m_docs_button);
    actionLayout->addWidget(m_license_button);
    actionLayout->addStretch();
    
    // Tab widget for credits and license
    m_tab_widget = new QTabWidget(this);
    m_tab_widget->setStyleSheet(
        "QTabWidget::pane {"
        "   border: 1px solid #E5E7EB;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "}"
        "QTabBar::tab {"
        "   background-color: #F8FAFC;"
        "   color: #6B7280;"
        "   padding: 8px 16px;"
        "   border: 1px solid #E5E7EB;"
        "   border-bottom: none;"
        "   border-top-left-radius: 4px;"
        "   border-top-right-radius: 4px;"
        "}"
        "QTabBar::tab:selected {"
        "   background-color: white;"
        "   color: #1E3A8A;"
        "   border-bottom: 1px solid white;"
        "}"
    );
    
    m_credits_browser = new QTextBrowser(this);
    m_credits_browser->setOpenExternalLinks(true);
    m_credits_browser->setStyleSheet(
        "QTextBrowser {"
        "   border: none;"
        "   background-color: white;"
        "   color: #374151;"
        "   font-size: 12px;"
        "   line-height: 1.5;"
        "}"
    );
    
    m_license_browser = new QTextBrowser(this);
    m_license_browser->setOpenExternalLinks(true);
    m_license_browser->setStyleSheet(
        "QTextBrowser {"
        "   border: none;"
        "   background-color: white;"
        "   color: #374151;"
        "   font-size: 12px;"
        "   line-height: 1.5;"
        "}"
    );
    
    m_tab_widget->addTab(m_credits_browser, tr("Credits"));
    m_tab_widget->addTab(m_license_browser, tr("License"));
    
    // Close button
    m_close_button = new QPushButton(tr("Close"), this);
    m_close_button->setFixedSize(80, 35);
    m_close_button->setStyleSheet(
        "QPushButton {"
        "   background-color: #1E3A8A;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3B82F6;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1E40AF;"
        "}"
    );
    
    QHBoxLayout* closeLayout = new QHBoxLayout();
    closeLayout->addStretch();
    closeLayout->addWidget(m_close_button);
    
    // Add all layouts to main layout
    mainLayout->addLayout(brandingLayout);
    mainLayout->addLayout(versionLayout);
    mainLayout->addLayout(actionLayout);
    mainLayout->addWidget(m_tab_widget);
    mainLayout->addLayout(closeLayout);
    
    // Connect signals
    connect(m_github_button, &QPushButton::clicked, this, &AboutDialog::onGitHubClicked);
    connect(m_docs_button, &QPushButton::clicked, this, &AboutDialog::onDocumentationClicked);
    connect(m_license_button, &QPushButton::clicked, this, &AboutDialog::onLicenseClicked);
    connect(m_copy_version_button, &QPushButton::clicked, this, &AboutDialog::onCopyVersionClicked);
    connect(m_close_button, &QPushButton::clicked, this, &QDialog::accept);
}

void AboutDialog::setupBranding()
{
    // Set logo (placeholder - will be replaced with actual Shahcoin logo)
    QPixmap logoPixmap(80, 80);
    logoPixmap.fill(Qt::transparent);
    
    QPainter painter(&logoPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw a simple placeholder logo with Shahi Blue
    QRadialGradient gradient(40, 40, 40);
    gradient.setColorAt(0, QColor(30, 58, 138));  // Shahi Blue
    gradient.setColorAt(1, QColor(59, 130, 246)); // Lighter Shahi Blue
    
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(5, 5, 70, 70);
    
    // Draw "S" in the center
    painter.setPen(QPen(Qt::white, 3));
    painter.setFont(QFont("Arial", 24, QFont::Bold));
    painter.drawText(logoPixmap.rect(), Qt::AlignCenter, "S");
    
    m_logo_label->setPixmap(logoPixmap);
}

void AboutDialog::setupVersionInfo()
{
    m_version_text = QString("Version %1").arg(QString::fromStdString(FormatFullVersion()));
    m_build_text = QString("Build: %1").arg(QString::fromStdString(FormatFullVersionWithCodename()));
    
    m_version_label->setText(m_version_text);
    m_build_label->setText(m_build_text);
}

void AboutDialog::setupCredits()
{
    m_credits_text = QString(
        "<h2 style='color: #1E3A8A;'>SHAHCOIN Core Contributors</h2>"
        "<p><strong>Built with love by SHAHCOIN Core Contributors</strong></p>"
        "<br>"
        "<h3>Core Development Team</h3>"
        "<ul>"
        "<li><strong>Lead Developers:</strong> SHAHCOIN Core Team</li>"
        "<li><strong>UI/UX Design:</strong> Shahcoin Design Team</li>"
        "<li><strong>Security Audits:</strong> Shahcoin Security Team</li>"
        "</ul>"
        "<br>"
        "<h3>Special Thanks</h3>"
        "<ul>"
        "<li>SHAHCOIN Core contributors for the foundation</li>"
        "<li>Qt Framework for the GUI toolkit</li>"
        "<li>Open source community for continuous support</li>"
        "</ul>"
        "<br>"
        "<h3>Community</h3>"
        "<p>Join our community:</p>"
        "<ul>"
        "<li><a href='https://github.com/shahcoin'>GitHub</a></li>"
        "<li><a href='https://shah.vip'>Official Website</a></li>"
        "<li><a href='https://shah.vip/help'>Documentation</a></li>"
        "</ul>"
    );
    
    m_credits_browser->setHtml(m_credits_text);
}

void AboutDialog::setupLicense()
{
    m_license_text = QString(
        "<h2 style='color: #1E3A8A;'>MIT License</h2>"
        "<p>Copyright (c) 2024 SHAHCOIN Core Contributors</p>"
        "<br>"
        "<p>Permission is hereby granted, free of charge, to any person obtaining a copy "
        "of this software and associated documentation files (the \"Software\"), to deal "
        "in the Software without restriction, including without limitation the rights "
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
        "copies of the Software, and to permit persons to whom the Software is "
        "furnished to do so, subject to the following conditions:</p>"
        "<br>"
        "<p>The above copyright notice and this permission notice shall be included in all "
        "copies or substantial portions of the Software.</p>"
        "<br>"
        "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
        "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
        "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
        "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
        "SOFTWARE.</p>"
    );
    
    m_license_browser->setHtml(m_license_text);
}

void AboutDialog::applyTheme()
{
    // Apply consistent styling
    setStyleSheet(
        "QDialog {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #E5E7EB;"
        "   border-radius: 8px;"
        "}"
    );
}

void AboutDialog::onGitHubClicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/shahcoin"));
}

void AboutDialog::onDocumentationClicked()
{
    QDesktopServices::openUrl(QUrl("https://shah.vip/help"));
}

void AboutDialog::onLicenseClicked()
{
    m_tab_widget->setCurrentIndex(1); // Switch to license tab
}

void AboutDialog::onCopyVersionClicked()
{
    QString versionInfo = QString("%1\n%2").arg(m_version_text, m_build_text);
    copyToClipboard(versionInfo);
}

void AboutDialog::copyToClipboard(const QString& text)
{
    QApplication::clipboard()->setText(text);
    
    // Show temporary feedback
    QString originalText = m_copy_version_button->text();
    m_copy_version_button->setText(tr("Copied!"));
    m_copy_version_button->setStyleSheet(
        "QPushButton {"
        "   background-color: #059669;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 11px;"
        "}"
    );
    
    QTimer::singleShot(2000, [this, originalText]() {
        m_copy_version_button->setText(originalText);
        m_copy_version_button->setStyleSheet(
            "QPushButton {"
            "   background-color: #1E3A8A;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 4px;"
            "   font-size: 11px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #3B82F6;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #1E40AF;"
            "}"
        );
    });
}
