#ifndef SHAHCOIN_QT_THEMEMANAGER_H
#define SHAHCOIN_QT_THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QSettings>
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>

class QWidget;

/**
 * @brief Advanced Theme Manager for Shahcoin Qt Wallet
 * 
 * Provides comprehensive theming support with:
 * - Light Theme (default)
 * - Dark Theme
 * - Monochrome Theme
 * - Retro Theme (Easter egg)
 * - Custom color schemes
 * - Persistent theme storage
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum ThemeType {
        Light = 0,
        Dark,
        Monochrome,
        Retro
    };

    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager();

    // Theme management
    void setTheme(ThemeType theme);
    void setTheme(const QString& themeName);
    ThemeType getCurrentTheme() const { return m_currentTheme; }
    QString getCurrentThemeName() const;
    
    // Theme information
    QStringList getAvailableThemes() const;
    QString getThemeDescription(ThemeType theme) const;
    bool isDarkTheme() const;
    
    // Application-wide theming
    void applyThemeToApplication();
    void applyThemeToWidget(QWidget* widget);
    
    // Settings
    void loadThemeFromSettings();
    void saveThemeToSettings();
    void resetToDefaultTheme();

public Q_SLOTS:
    void onThemeChanged(int themeIndex);
    void onThemeChanged(const QString& themeName);

Q_SIGNALS:
    void themeChanged(ThemeType theme);
    void themeApplied(const QString& themeName);

private:
    void setupThemes();
    void applyLightTheme();
    void applyDarkTheme();
    void applyMonochromeTheme();
    void applyRetroTheme();
    void applyCustomStylesheet(const QString& stylesheet);
    void updateApplicationPalette(const QPalette& palette);
    QString generateStylesheet(ThemeType theme) const;
    
    // Theme-specific color schemes
    struct ColorScheme {
        QString primary;
        QString secondary;
        QString accent;
        QString background;
        QString surface;
        QString text;
        QString textSecondary;
        QString border;
        QString shadow;
        QString success;
        QString warning;
        QString error;
    };
    
    ColorScheme getColorScheme(ThemeType theme) const;

    QSettings* m_settings;
    ThemeType m_currentTheme;
    QList<QString> m_availableThemes;
    QMap<ThemeType, QString> m_themeNames;
    QMap<ThemeType, QString> m_themeDescriptions;
    
    // Constants
    static constexpr ThemeType DEFAULT_THEME = Light;
    static constexpr const char* SETTINGS_KEY = "Display/Theme";
};

#endif // SHAHCOIN_QT_THEMEMANAGER_H
