#ifndef SHAHCOIN_QT_ONBOARDINGTOUR_H
#define SHAHCOIN_QT_ONBOARDINGTOUR_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QTimer>
#include <QSettings>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>

class QMainWindow;
class QAction;
class QMenu;

/**
 * @brief Interactive Onboarding Tour for Shahcoin Qt Wallet
 * 
 * Provides a guided tour of key wallet features on first launch:
 * - Create wallet / Load wallet
 * - Stake SHAH
 * - Create NFT or Token
 * - Use ShahSwap
 */
class OnboardingTour : public QObject
{
    Q_OBJECT

public:
    enum TourStep {
        Welcome = 0,
        CreateWallet,
        LoadWallet,
        StakeShah,
        CreateNFT,
        CreateToken,
        UseShahSwap,
        Complete
    };

    explicit OnboardingTour(QMainWindow* mainWindow, QObject* parent = nullptr);
    ~OnboardingTour();

    void startTour();
    void showStep(TourStep step);
    bool shouldShowTour() const;
    void setTourCompleted();

public Q_SLOTS:
    void onNextStep();
    void onPreviousStep();
    void onSkipTour();
    void onCompleteTour();
    void onDontShowAgainToggled(bool checked);

Q_SIGNALS:
    void tourCompleted();
    void tourSkipped();

private Q_SLOTS:
    void onActionTriggered();
    void onMenuAboutToShow();

private:
    void setupTourSteps();
    void createTooltipWidget();
    void showTooltip(const QString& title, const QString& description, 
                    const QPoint& position, QWidget* targetWidget = nullptr);
    void hideTooltip();
    void highlightWidget(QWidget* widget);
    void removeHighlight();
    void connectToActions();
    void disconnectFromActions();
    QPoint calculateTooltipPosition(QWidget* targetWidget, const QString& position = "bottom");
    void animateTooltip(const QPoint& from, const QPoint& to);
    void saveTourProgress();
    void loadTourProgress();

    // Tour step data
    struct TourStepData {
        QString title;
        QString description;
        QString actionName;
        QString menuPath;
        QString position;
        bool requiresAction;
    };

    QMainWindow* m_mainWindow;
    QWidget* m_tooltipWidget;
    QLabel* m_tooltipTitle;
    QLabel* m_tooltipDescription;
    QPushButton* m_previousButton;
    QPushButton* m_nextButton;
    QPushButton* m_skipButton;
    QCheckBox* m_dontShowAgainCheckbox;
    QPropertyAnimation* m_tooltipAnimation;
    QGraphicsOpacityEffect* m_tooltipOpacity;
    QGraphicsDropShadowEffect* m_tooltipShadow;

    QList<TourStepData> m_tourSteps;
    int m_currentStepIndex;
    QWidget* m_currentHighlightedWidget;
    QGraphicsOpacityEffect* m_highlightEffect;
    
    QTimer* m_autoAdvanceTimer;
    bool m_tourInProgress;
    bool m_dontShowAgain;

    // Connected actions for tracking
    QList<QAction*> m_connectedActions;
    QList<QMenu*> m_connectedMenus;
};

/**
 * @brief Tooltip widget for tour steps
 */
class TourTooltipWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TourTooltipWidget(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setPosition(const QString& position);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    QString m_title;
    QString m_description;
    QString m_position;
    QColor m_backgroundColor;
    QColor m_borderColor;
    QColor m_textColor;
    QColor m_titleColor;
    int m_cornerRadius;
    int m_borderWidth;
    int m_padding;
    int m_arrowSize;
    int m_arrowOffset;
};

#endif // SHAHCOIN_QT_ONBOARDINGTOUR_H
