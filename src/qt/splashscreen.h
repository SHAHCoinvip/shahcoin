// Copyright (c) 2011-2022 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_SPLASHSCREEN_H
#define SHAHCOIN_QT_SPLASHSCREEN_H

#include <QSplashScreen>
#include <QTimer>
#include <QPropertyAnimation>
#include <QLabel>
#include <QVBoxLayout>
#include <QProgressBar>

class QPixmap;

/** Class for the splashscreen with information of the running client.
 *
 * @note this is intentionally not a QSplashScreen. SHAHCOIN Core initialization
 * can take a long time, and in that case a progress window that cannot be
 * moved around and doesn't show any mouse cursor must be used to prevent the
 * mouse cursor from disappearing into an infinite loop.
 */
class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(Qt::WindowFlags f, const NetworkStyle *networkStyle);
    ~SplashScreen();

protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

public Q_SLOTS:
    /** Hide the splash screen window and schedule the splash screen object for deletion */
    void finish();

    /** Show message and progress */
    void showMessage(const QString &message, int alignment, const QColor &color);

    /** Sets the break action */
    void setBreakAction(const std::function<void(void)> &action);

    void setProgress(int progress);

private:
    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();
    void connectWallet(WalletModel* walletModel);
    void connectBlockchain();
    void showProgress(const QString &title, int nProgress);
    void setVisible(bool visible);
    void alignWidget(QWidget *widget, const QWidget *host, int alignment, int dx, int dy);

    QLabel *m_icon_label = nullptr;
    QLabel *m_title_label = nullptr;
    QLabel *m_tagline_label = nullptr;
    QLabel *m_version_label = nullptr;
    QLabel *m_network_label = nullptr;
    QProgressBar *m_progress_bar = nullptr;
    QLabel *m_status_label = nullptr;
    QLabel *m_loading_label = nullptr;
    
    QPropertyAnimation *m_fade_animation = nullptr;
    QPropertyAnimation *m_progress_animation = nullptr;
    QTimer *m_loading_timer = nullptr;
    
    int m_progress = 0;
    int m_loading_dots = 0;
    
    std::function<void(void)> breakAction;
    const NetworkStyle *m_network_style;
    bool m_show = false;
    bool m_closed = false;
    std::list<WalletModel*> m_connected_wallets;
    std::list<WalletModel*> m_connected_wallets_wait;
    
    int curAlignment = Qt::AlignLeft | Qt::AlignTop;
    QColor curColor = Qt::white;

Q_SIGNALS:
    void clicked();
};

#endif // SHAHCOIN_QT_SPLASHSCREEN_H
