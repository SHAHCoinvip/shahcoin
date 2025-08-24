#ifndef SHAHCOIN_QT_TOOLTIPENHANCEMENT_H
#define SHAHCOIN_QT_TOOLTIPENHANCEMENT_H

#include <QObject>
#include <QWidget>
#include <QToolTip>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class QLabel;
class QTextBrowser;

/**
 * @brief Enhanced Tooltip System for Shahcoin Qt Wallet
 * 
 * Provides smart tooltips with:
 * - Markdown support for rich formatting
 * - Contextual help based on widget state
 * - Animated tooltips with smooth transitions
 * - Customizable styling and positioning
 * - Help system integration
 */
class TooltipEnhancement : public QObject
{
    Q_OBJECT

public:
    enum TooltipType {
        Basic,
        Help,
        Warning,
        Info,
        Success,
        Error
    };

    struct TooltipData {
        QString text;
        QString markdown;
        TooltipType type;
        QString icon;
        int duration;
        bool persistent;
        QString helpUrl;
    };

    explicit TooltipEnhancement(QObject* parent = nullptr);
    ~TooltipEnhancement();

    // Tooltip management
    void setTooltip(QWidget* widget, const QString& text, TooltipType type = Basic);
    void setTooltip(QWidget* widget, const TooltipData& data);
    void removeTooltip(QWidget* widget);
    void showTooltip(QWidget* widget, const QString& text, int duration = 3000);
    void hideTooltip();
    
    // Markdown support
    QString parseMarkdown(const QString& markdown) const;
    void setMarkdownEnabled(bool enabled);
    bool isMarkdownEnabled() const;
    
    // Styling
    void setTooltipStyle(const QString& style);
    QString getTooltipStyle() const;
    void setTheme(const QString& theme);
    
    // Help system
    void setHelpUrl(QWidget* widget, const QString& url);
    void showHelpDialog(const QString& topic);

public Q_SLOTS:
    void onWidgetEntered();
    void onWidgetLeft();
    void onWidgetClicked();
    void onHelpRequested();

Q_SIGNALS:
    void tooltipShown(QWidget* widget, const TooltipData& data);
    void tooltipHidden(QWidget* widget);
    void helpRequested(const QString& topic);

private:
    void setupDefaultTooltips();
    void createTooltipWidget();
    void positionTooltip(QWidget* target);
    void animateTooltip(bool show);
    QString getDefaultTooltip(QWidget* widget) const;
    QString getTooltipIcon(TooltipType type) const;
    QString getTooltipStyle(TooltipType type) const;
    
    // UI Components
    QWidget* m_tooltipWidget;
    QLabel* m_iconLabel;
    QTextBrowser* m_textBrowser;
    QPropertyAnimation* m_fadeAnimation;
    QGraphicsOpacityEffect* m_opacityEffect;
    
    // Data
    QMap<QWidget*, TooltipData> m_tooltipData;
    QMap<QWidget*, QString> m_helpUrls;
    QWidget* m_currentWidget;
    QTimer* m_showTimer;
    QTimer* m_hideTimer;
    
    // Settings
    bool m_markdownEnabled;
    QString m_currentStyle;
    QString m_currentTheme;
    int m_defaultDuration;
    bool m_animationsEnabled;
    
    // Default tooltips
    QMap<QString, TooltipData> m_defaultTooltips;
    
    // Constants
    static constexpr int DEFAULT_DURATION = 3000;
    static constexpr int ANIMATION_DURATION = 200;
    static constexpr int SHOW_DELAY = 500;
};

/**
 * @brief Enhanced Tooltip Widget
 * 
 * Custom tooltip widget with rich formatting and animations
 */
class EnhancedTooltipWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EnhancedTooltipWidget(QWidget* parent = nullptr);
    ~EnhancedTooltipWidget();

    void setContent(const QString& text, const QString& icon = QString(), 
                   TooltipEnhancement::TooltipType type = TooltipEnhancement::Basic);
    void setMarkdown(const QString& markdown);
    void setStyle(const QString& style);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void setupUI();
    void applyStyle();
    void updateLayout();
    
    QLabel* m_iconLabel;
    QTextBrowser* m_textBrowser;
    QVBoxLayout* m_layout;
    QString m_currentStyle;
    TooltipEnhancement::TooltipType m_currentType;
};

#endif // SHAHCOIN_QT_TOOLTIPENHANCEMENT_H
