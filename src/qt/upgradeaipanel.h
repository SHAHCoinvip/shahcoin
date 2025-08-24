#ifndef SHAHCOIN_QT_UPGRADEAIPANEL_H
#define SHAHCOIN_QT_UPGRADEAIPANEL_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QComboBox>
#include <QStackedWidget>
#include <QProgressBar>

class WalletModel;
class MonetizationManager;

/**
 * UpgradeAIPanel
 *
 * Modal dialog to unlock AI Wallet features:
 * - Pay with SHAH (via ShahSwap conversion from $9.99)
 * - Pay with SHAHtoken (SHI-20 style)
 * - Pay with Stripe (credit card)
 * Also offers Buy SHAH with card (Stripe → bridge → ShahSwap → deposit).
 */
class UpgradeAIPanel : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeAIPanel(WalletModel* walletModel, MonetizationManager* monetization, QWidget* parent = nullptr);

signals:
    void premiumUnlocked();
    void creditsToppedUp(int newBalance);
    void paymentRecorded(const QString& method, double amountUsd, const QString& transactionId, const QString& planType, const QString& invoiceUrl);

private slots:
    void onOptionChanged();
    void onPurchaseClicked();
    void onBuyShahClicked();
    void onCancelClicked();

private:
    void setupUI();
    void updatePriceLabels();
    void showProcessing(const QString& text);
    void hideProcessing();

    // Simulated integrations (to be wired to real backends)
    bool processStripePayment(double amountUsd, QString* receiptOut);
    bool processShahPayment(double amountUsd, QString* txidOut); // via ShahSwap auto-convert
    bool processShahTokenPayment(double amountUsd, QString* txidOut);
    bool bridgeStripeToShah(double amountUsd, QString* shahTxidOut);

private:
    WalletModel* m_walletModel;
    MonetizationManager* m_monetization;

    // UI elements
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QLabel* m_descLabel;
    QLabel* m_statusLabel;
    QProgressBar* m_progress;

    QGroupBox* m_optionsGroup;
    QRadioButton* m_payShahRadio;
    QRadioButton* m_payShahTokenRadio;
    QRadioButton* m_payStripeRadio;
    QLabel* m_priceLabel;
    QPushButton* m_purchaseButton;
    QPushButton* m_buyShahButton;
    QPushButton* m_cancelButton;
};

#endif // SHAHCOIN_QT_UPGRADEAIPANEL_H


