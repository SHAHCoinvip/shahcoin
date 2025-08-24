// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_TOKENCREATIONDIALOG_H
#define SHAHCOIN_QT_TOKENCREATIONDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

namespace Ui {
class TokenCreationDialog;
}

class TokenCreationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TokenCreationDialog(QWidget *parent = nullptr);
    ~TokenCreationDialog();

    // Token configuration getters
    QString getTokenName() const;
    QString getTokenSymbol() const;
    int getDecimals() const;
    QString getMaxSupply() const;
    QString getInitialSupply() const;

    // Payment method getter
    QString getPaymentMethod() const;
    QString getTier() const;

    // Add-ons getters
    bool isCustomLogoSelected() const;
    bool isVerifiedBadgeSelected() const;
    bool isUpgradeableContractSelected() const;
    bool isAirdropToolSelected() const;

    // Terms acceptance
    bool isTermsAccepted() const;

    // Fee calculation
    double getCurrentShahPrice() const;
    double getTotalFeeUSD() const;
    double getTotalFeeSHAH() const;
    // Payment status
    void setPaymentVerified(bool verified);
    bool isPaymentVerified() const;

signals:
    void tokenCreationRequested(const QString& tokenName, const QString& tokenSymbol, 
                               int decimals, const QString& maxSupply, const QString& initialSupply,
                               const QString& paymentMethod, const QString& tier, const QStringList& addOns);

private slots:
    void onCalculateFeeClicked();
    void onConfirmCreateClicked();
    void onCancelClicked();
    void onShahPriceReceived();
    void onNetworkError(QNetworkReply::NetworkError error);
    void updateConfirmButtonState();
    void updateFeeDisplay();

private:
    Ui::TokenCreationDialog *ui;
    QNetworkAccessManager *m_networkManager;
    double m_currentShahPrice;
    double m_totalFeeUSD;
    double m_totalFeeSHAH;
    bool m_isCalculating;
    bool m_paymentVerified = false;
    // Payment integration
    QTimer *m_paymentPollTimer = nullptr;
    QString m_invoiceId;
    QString m_apiBaseUrl;
    QString m_factoryFeeAddress;

    void setupConnections();
    void setupFeeExplanation();
    void fetchShahPrice();
    void calculateTotalFee();
    double tierBaseUsd() const;
    QStringList getSelectedAddOns() const;
    void showError(const QString& title, const QString& message);
    void showSuccess(const QString& title, const QString& message);
    bool validateInputs() const;
    void updateFeeLabels();
    // Tier UI helpers
    void updateTierBenefits();
    // Payment helpers
    void startPaymentVerification();
    void startStripePayment();
    void requestShahInvoice();
    void startInvoicePolling();
    void pollInvoiceStatus();
};

#endif // SHAHCOIN_QT_TOKENCREATIONDIALOG_H
