#ifndef SHAHCOIN_QT_SUBSCRIPTIONDIALOG_H
#define SHAHCOIN_QT_SUBSCRIPTIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QDateTime>

class MonetizationManager;
class WalletModel;

class SubscriptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SubscriptionDialog(MonetizationManager* monetization, WalletModel* walletModel = nullptr, QWidget* parent = nullptr);

Q_SIGNALS:
    void paymentRecorded(const QString& method, double amountUsd, const QString& transactionId, const QString& planType, const QString& invoiceUrl);

private Q_SLOTS:
    void onSubscribeClicked();
    void onCancelSubscriptionClicked();
    void onRenewNowClicked();
    void onMethodChanged();

private:
    void setupUI();
    void updateStatus();

    MonetizationManager* m_monetization;
    WalletModel* m_walletModel;

    QVBoxLayout* m_layout;
    QLabel* m_statusLabel;
    QLabel* m_priceLabel;
    QLabel* m_dueLabel;
    QGroupBox* m_methodGroup;
    QRadioButton* m_shahRadio;
    QRadioButton* m_shahTokenRadio;
    QRadioButton* m_stripeRadio;
    QPushButton* m_subscribeButton;
    QPushButton* m_cancelButton;
    QPushButton* m_renewButton;
};

#endif // SHAHCOIN_QT_SUBSCRIPTIONDIALOG_H


