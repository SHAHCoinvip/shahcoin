#include "thememanager.h"

#include <QApplication>
#include <QWidget>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QSettings>
#include <QDebug>
#include <QTimer>

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
    , m_settings(new QSettings("Shahcoin", "Shahcoin-Qt"))
    , m_currentTheme(DEFAULT_THEME)
{
    setupThemes();
    loadThemeFromSettings();
}

ThemeManager::~ThemeManager()
{
    delete m_settings;
}

void ThemeManager::setupThemes()
{
    m_availableThemes = {"Light", "Dark", "Monochrome", "Retro"};
    
    m_themeNames[Light] = "Light";
    m_themeNames[Dark] = "Dark";
    m_themeNames[Monochrome] = "Monochrome";
    m_themeNames[Retro] = "Retro";
    
    m_themeDescriptions[Light] = "Clean and modern light theme with Shahi Blue accents";
    m_themeDescriptions[Dark] = "Elegant dark theme with Shahi Blue highlights";
    m_themeDescriptions[Monochrome] = "Minimalist black and white theme";
    m_themeDescriptions[Retro] = "Nostalgic retro-style theme";
}

void ThemeManager::setTheme(ThemeType theme)
{
    if (m_currentTheme == theme) return;
    
    m_currentTheme = theme;
    applyThemeToApplication();
    saveThemeToSettings();
    
    emit themeChanged(theme);
    emit themeApplied(getCurrentThemeName());
}

void ThemeManager::setTheme(const QString& themeName)
{
    for (auto it = m_themeNames.begin(); it != m_themeNames.end(); ++it) {
        if (it.value() == themeName) {
            setTheme(it.key());
            return;
        }
    }
    
    // Fallback to default theme
    qWarning() << "Unknown theme:" << themeName << "falling back to default";
    setTheme(DEFAULT_THEME);
}

QString ThemeManager::getCurrentThemeName() const
{
    return m_themeNames.value(m_currentTheme, "Light Theme");
}

QStringList ThemeManager::getAvailableThemes() const
{
    return m_availableThemes;
}

QString ThemeManager::getThemeDescription(ThemeType theme) const
{
    return m_themeDescriptions.value(theme, "No description available");
}

bool ThemeManager::isDarkTheme() const
{
    return m_currentTheme == Dark || m_currentTheme == Monochrome;
}

void ThemeManager::applyThemeToApplication()
{
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) return;
    
    switch (m_currentTheme) {
        case Light:
            applyLightTheme();
            break;
        case Dark:
            applyDarkTheme();
            break;
        case Monochrome:
            applyMonochromeTheme();
            break;
        case Retro:
            applyRetroTheme();
            break;
    }
}

void ThemeManager::applyThemeToWidget(QWidget* widget)
{
    if (!widget) return;
    
    // Apply theme-specific styling to individual widgets
    QString stylesheet = generateStylesheet(m_currentTheme);
    widget->setStyleSheet(stylesheet);
}

void ThemeManager::loadThemeFromSettings()
{
    QString savedTheme = m_settings->value(SETTINGS_KEY, "Light Theme").toString();
    setTheme(savedTheme);
}

void ThemeManager::saveThemeToSettings()
{
    m_settings->setValue(SETTINGS_KEY, getCurrentThemeName());
    m_settings->sync();
}

void ThemeManager::resetToDefaultTheme()
{
    setTheme(DEFAULT_THEME);
}

void ThemeManager::onThemeChanged(int themeIndex)
{
    if (themeIndex >= 0 && themeIndex < m_availableThemes.size()) {
        setTheme(m_availableThemes[themeIndex]);
    }
}

void ThemeManager::onThemeChanged(const QString& themeName)
{
    setTheme(themeName);
}

void ThemeManager::applyLightTheme()
{
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) return;
    
    // Reset to default light theme
    app->setStyleSheet("");
    
    // Set light palette
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(255, 255, 255));
    palette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    palette.setColor(QPalette::Text, QColor(0, 0, 0));
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    palette.setColor(QPalette::Link, QColor(0, 0, 255));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    
    updateApplicationPalette(palette);
}

void ThemeManager::applyDarkTheme()
{
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) return;
    
    // Apply dark theme stylesheet
    QString darkStylesheet = R"(
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            border: none;
        }
        
        QMainWindow {
            background-color: #2b2b2b;
        }
        
        QMenuBar {
            background-color: #3c3c3c;
            color: #ffffff;
            border-bottom: 1px solid #555555;
        }
        
        QMenuBar::item {
            background-color: transparent;
            padding: 4px 8px;
        }
        
        QMenuBar::item:selected {
            background-color: #555555;
        }
        
        QMenu {
            background-color: #3c3c3c;
            color: #ffffff;
            border: 1px solid #555555;
        }
        
        QMenu::item:selected {
            background-color: #555555;
        }
        
        QPushButton {
            background-color: #4a4a4a;
            color: #ffffff;
            border: 1px solid #666666;
            border-radius: 4px;
            padding: 6px 12px;
        }
        
        QPushButton:hover {
            background-color: #5a5a5a;
        }
        
        QPushButton:pressed {
            background-color: #3a3a3a;
        }
        
        QLineEdit, QTextEdit, QPlainTextEdit {
            background-color: #3c3c3c;
            color: #ffffff;
            border: 1px solid #666666;
            border-radius: 4px;
            padding: 4px;
        }
        
        QComboBox {
            background-color: #3c3c3c;
            color: #ffffff;
            border: 1px solid #666666;
            border-radius: 4px;
            padding: 4px;
        }
        
        QComboBox::drop-down {
            border: none;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #ffffff;
        }
        
        QTableWidget, QTreeWidget, QListWidget {
            background-color: #3c3c3c;
            color: #ffffff;
            gridline-color: #555555;
            border: 1px solid #666666;
        }
        
        QHeaderView::section {
            background-color: #4a4a4a;
            color: #ffffff;
            border: 1px solid #666666;
            padding: 4px;
        }
        
        QTabWidget::pane {
            border: 1px solid #666666;
            background-color: #2b2b2b;
        }
        
        QTabBar::tab {
            background-color: #4a4a4a;
            color: #ffffff;
            border: 1px solid #666666;
            padding: 8px 16px;
        }
        
        QTabBar::tab:selected {
            background-color: #2b2b2b;
        }
        
        QScrollBar:vertical {
            background-color: #3c3c3c;
            width: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical {
            background-color: #666666;
            border-radius: 6px;
            min-height: 20px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #777777;
        }
        
        QDockWidget {
            titlebar-close-icon: url(close.png);
            titlebar-normal-icon: url(undock.png);
        }
        
        QDockWidget::title {
            background-color: #4a4a4a;
            color: #ffffff;
            padding: 4px;
            border: 1px solid #666666;
        }
    )";
    
    applyCustomStylesheet(darkStylesheet);
}

void ThemeManager::applyMonochromeTheme()
{
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) return;
    
    // Apply monochrome stylesheet
    QString monochromeStylesheet = R"(
        QWidget {
            background-color: #ffffff;
            color: #000000;
            border: 1px solid #000000;
        }
        
        QMainWindow {
            background-color: #ffffff;
        }
        
        QMenuBar {
            background-color: #ffffff;
            color: #000000;
            border-bottom: 2px solid #000000;
        }
        
        QMenuBar::item {
            background-color: transparent;
            padding: 4px 8px;
        }
        
        QMenuBar::item:selected {
            background-color: #000000;
            color: #ffffff;
        }
        
        QMenu {
            background-color: #ffffff;
            color: #000000;
            border: 2px solid #000000;
        }
        
        QMenu::item:selected {
            background-color: #000000;
            color: #ffffff;
        }
        
        QPushButton {
            background-color: #ffffff;
            color: #000000;
            border: 2px solid #000000;
            padding: 6px 12px;
        }
        
        QPushButton:hover {
            background-color: #000000;
            color: #ffffff;
        }
        
        QPushButton:pressed {
            background-color: #ffffff;
            color: #000000;
        }
        
        QLineEdit, QTextEdit, QPlainTextEdit {
            background-color: #ffffff;
            color: #000000;
            border: 2px solid #000000;
            padding: 4px;
        }
        
        QComboBox {
            background-color: #ffffff;
            color: #000000;
            border: 2px solid #000000;
            padding: 4px;
        }
        
        QComboBox::drop-down {
            border: none;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #000000;
        }
        
        QTableWidget, QTreeWidget, QListWidget {
            background-color: #ffffff;
            color: #000000;
            gridline-color: #000000;
            border: 2px solid #000000;
        }
        
        QHeaderView::section {
            background-color: #ffffff;
            color: #000000;
            border: 2px solid #000000;
            padding: 4px;
        }
        
        QTabWidget::pane {
            border: 2px solid #000000;
            background-color: #ffffff;
        }
        
        QTabBar::tab {
            background-color: #ffffff;
            color: #000000;
            border: 2px solid #000000;
            padding: 8px 16px;
        }
        
        QTabBar::tab:selected {
            background-color: #000000;
            color: #ffffff;
        }
        
        QScrollBar:vertical {
            background-color: #ffffff;
            width: 12px;
            border: 1px solid #000000;
        }
        
        QScrollBar::handle:vertical {
            background-color: #000000;
            min-height: 20px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #333333;
        }
    )";
    
    applyCustomStylesheet(monochromeStylesheet);
}

void ThemeManager::applyRetroTheme()
{
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) return;
    
    // Apply retro 80s-style theme
    QString retroStylesheet = R"(
        QWidget {
            background-color: #000080;
            color: #00ff00;
            border: 2px solid #ff00ff;
            font-family: "Courier New", monospace;
        }
        
        QMainWindow {
            background-color: #000080;
        }
        
        QMenuBar {
            background-color: #800080;
            color: #ffff00;
            border-bottom: 3px solid #ff00ff;
        }
        
        QMenuBar::item {
            background-color: transparent;
            padding: 4px 8px;
        }
        
        QMenuBar::item:selected {
            background-color: #ff00ff;
            color: #000000;
        }
        
        QMenu {
            background-color: #800080;
            color: #ffff00;
            border: 3px solid #ff00ff;
        }
        
        QMenu::item:selected {
            background-color: #ff00ff;
            color: #000000;
        }
        
        QPushButton {
            background-color: #ff0000;
            color: #ffffff;
            border: 3px solid #ffff00;
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #ff8080;
            border-color: #ffffff;
        }
        
        QPushButton:pressed {
            background-color: #800000;
        }
        
        QLineEdit, QTextEdit, QPlainTextEdit {
            background-color: #000000;
            color: #00ff00;
            border: 3px solid #00ffff;
            padding: 6px;
            font-family: "Courier New", monospace;
        }
        
        QComboBox {
            background-color: #800080;
            color: #ffff00;
            border: 3px solid #ff00ff;
            border-radius: 8px;
            padding: 6px;
        }
        
        QComboBox::drop-down {
            border: none;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 6px solid transparent;
            border-right: 6px solid transparent;
            border-top: 6px solid #ffff00;
        }
        
        QTableWidget, QTreeWidget, QListWidget {
            background-color: #000000;
            color: #00ff00;
            gridline-color: #00ffff;
            border: 3px solid #00ffff;
        }
        
        QHeaderView::section {
            background-color: #800080;
            color: #ffff00;
            border: 2px solid #ff00ff;
            padding: 6px;
            font-weight: bold;
        }
        
        QTabWidget::pane {
            border: 3px solid #ff00ff;
            background-color: #000080;
        }
        
        QTabBar::tab {
            background-color: #800080;
            color: #ffff00;
            border: 3px solid #ff00ff;
            border-radius: 8px 8px 0 0;
            padding: 10px 20px;
            font-weight: bold;
        }
        
        QTabBar::tab:selected {
            background-color: #000080;
            color: #00ff00;
        }
        
        QScrollBar:vertical {
            background-color: #800080;
            width: 16px;
            border: 2px solid #ff00ff;
            border-radius: 8px;
        }
        
        QScrollBar::handle:vertical {
            background-color: #ff00ff;
            border-radius: 8px;
            min-height: 30px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #ff80ff;
        }
        
        QLabel {
            color: #00ff00;
            font-weight: bold;
        }
        
        QGroupBox {
            border: 3px solid #00ffff;
            border-radius: 8px;
            margin-top: 10px;
            font-weight: bold;
            color: #00ff00;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            background-color: #000080;
        }
    )";
    
    applyCustomStylesheet(retroStylesheet);
}

void ThemeManager::applyCustomStylesheet(const QString& stylesheet)
{
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) return;
    
    app->setStyleSheet(stylesheet);
}

void ThemeManager::updateApplicationPalette(const QPalette& palette)
{
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) return;
    
    app->setPalette(palette);
}

QString ThemeManager::generateStylesheet(ThemeType theme) const
{
    ColorScheme colors = getColorScheme(theme);
    
    QString stylesheet = QString(R"(
        QWidget {
            background-color: %1;
            color: %2;
        }
        
        QPushButton {
            background-color: %3;
            color: %4;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 6px 12px;
        }
        
        QPushButton:hover {
            background-color: %6;
        }
        
        QLineEdit {
            background-color: %7;
            color: %8;
            border: 1px solid %9;
            border-radius: 4px;
            padding: 4px;
        }
    )").arg(colors.background, colors.text, colors.primary, colors.text, 
            colors.border, colors.secondary, colors.surface, colors.text, colors.border);
    
    return stylesheet;
}

ThemeManager::ColorScheme ThemeManager::getColorScheme(ThemeType theme) const
{
    ColorScheme scheme;
    
    switch (theme) {
        case Light:
            scheme.primary = "#1E3A8A";      // Shahi Blue
            scheme.secondary = "#3B82F6";    // Lighter Shahi Blue
            scheme.accent = "#F59E0B";       // Gold accent
            scheme.background = "#FFFFFF";   // Pure white
            scheme.surface = "#F8FAFC";      // Light gray
            scheme.text = "#1F2937";         // Dark gray
            scheme.textSecondary = "#6B7280"; // Medium gray
            scheme.border = "#E5E7EB";       // Light border
            scheme.shadow = "#00000020";     // Subtle shadow
            scheme.success = "#10B981";      // Green
            scheme.warning = "#F59E0B";      // Orange
            scheme.error = "#EF4444";        // Red
            break;
            
        case Dark:
            scheme.primary = "#3B82F6";      // Shahi Blue
            scheme.secondary = "#60A5FA";    // Lighter Shahi Blue
            scheme.accent = "#FBBF24";       // Gold accent
            scheme.background = "#0F172A";   // Dark blue-gray
            scheme.surface = "#1E293B";      // Medium blue-gray
            scheme.text = "#F1F5F9";         // Light gray
            scheme.textSecondary = "#94A3B8"; // Medium gray
            scheme.border = "#334155";       // Dark border
            scheme.shadow = "#00000040";     // Dark shadow
            scheme.success = "#34D399";      // Green
            scheme.warning = "#FBBF24";      // Orange
            scheme.error = "#F87171";        // Red
            break;
            
        case Monochrome:
            scheme.primary = "#000000";      // Black
            scheme.secondary = "#333333";    // Dark gray
            scheme.accent = "#666666";       // Medium gray
            scheme.background = "#FFFFFF";   // White
            scheme.surface = "#F5F5F5";      // Light gray
            scheme.text = "#000000";         // Black
            scheme.textSecondary = "#666666"; // Medium gray
            scheme.border = "#CCCCCC";       // Light gray
            scheme.shadow = "#00000015";     // Subtle shadow
            scheme.success = "#000000";      // Black
            scheme.warning = "#666666";      // Medium gray
            scheme.error = "#333333";        // Dark gray
            break;
            
        case Retro:
            scheme.primary = "#8B4513";      // Saddle brown
            scheme.secondary = "#D2691E";    // Chocolate
            scheme.accent = "#FFD700";       // Gold
            scheme.background = "#F5DEB3";   // Wheat
            scheme.surface = "#DEB887";      // Burlywood
            scheme.text = "#654321";         // Dark brown
            scheme.textSecondary = "#8B7355"; // Tan
            scheme.border = "#CD853F";       // Peru
            scheme.shadow = "#8B451320";     // Brown shadow
            scheme.success = "#228B22";      // Forest green
            scheme.warning = "#FF8C00";      // Dark orange
            scheme.error = "#DC143C";        // Crimson
            break;
    }
    
    return scheme;
}
