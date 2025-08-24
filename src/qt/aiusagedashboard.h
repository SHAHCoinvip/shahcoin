#ifndef SHAHCOIN_QT_AIUSAGEDASHBOARD_H
#define SHAHCOIN_QT_AIUSAGEDASHBOARD_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QTimer>
#include <QFrame>
#include <QChartView>
#include <QPieSeries>

QT_CHARTS_USE_NAMESPACE

class WalletModel;
class MonetizationManager;

class AIUsageDashboard : public QWidget
{
    Q_OBJECT
public:
    explicit AIUsageDashboard(QWidget* parent = nullptr);

    void setWalletModel(WalletModel* model);
    void setMonetization(MonetizationManager* monetization);

public slots:
    void refresh();
    void onRenewClicked();
    void onUpgradeClicked();

private:
    void setupUI();
    void updateOverlay();
    QString formatDuration(qint64 secs) const;
    void updateChart(int insightsCount, int chatCount, int alertsCount);

    // Models
    WalletModel* m_walletModel{nullptr};
    MonetizationManager* m_monetization{nullptr};

    // UI Elements
    QVBoxLayout* m_mainLayout{nullptr};
    QLabel* m_statusLabel{nullptr};
    QLabel* m_timeRemainingLabel{nullptr};
    QLabel* m_messagesLabel{nullptr};
    QLabel* m_chatsLabel{nullptr};
    QLabel* m_notificationsLabel{nullptr};
    QLabel* m_languageLabel{nullptr};
    QLabel* m_savingsLabel{nullptr};
    QChartView* m_chartView{nullptr};
    QPushButton* m_upgradeButton{nullptr};
    QPushButton* m_extendButton{nullptr};

    // Overlay for expired
    QFrame* m_overlay{nullptr};
    QLabel* m_overlayText{nullptr};
    QPushButton* m_overlayRenewButton{nullptr};
};

#endif // SHAHCOIN_QT_AIUSAGEDASHBOARD_H


