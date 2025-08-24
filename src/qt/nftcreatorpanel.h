// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_NFTCREATORPANEL_H
#define SHAHCOIN_QT_NFTCREATORPANEL_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QTableWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class WalletModel;
class PlatformStyle;

/**
 * NFT Creator Panel Widget
 *
 * Allows users to create native Shahcoin NFTs with:
 * - NFT name, description, image
 * - Custom attributes (key/value pairs)
 * - Dynamic fee calculation
 * - Validation and confirmation
 */
class NFTCreatorPanel : public QWidget
{
    Q_OBJECT
public:
    explicit NFTCreatorPanel(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~NFTCreatorPanel();

    void setWalletModel(WalletModel* walletModel);

private Q_SLOTS:
    void onNFTNameChanged();
    void onImageUploadClicked();
    void onAddAttributeClicked();
    void onRemoveAttributeClicked();
    void onAttributeChanged();
    void onTierChanged();
    void onPayVerifyClicked();
    void onMintNFTClicked();
    void onClearClicked();
    void onPaymentMethodChanged(int);

private:
    void setupUI();
    void connectSignals();
    void applyTheme();
    void refreshData();
    void updateFeeDisplay();
    void updateMintButton();
    bool validateInputs();
    void showConfirmationDialog();
    void mintNFT();
    void showSuccessDialog(const QString& nftId);
    void showErrorDialog(const QString& error);
    QString encodeImageToBase64(const QString& filePath);
    QString generateAttributesJSON();
    void updateAttributesTable();
    void clearForm();

    void updateTierBenefits();
    void setPaymentVerified(bool ok);

    // Payment helpers
    void startPaymentVerification();
    void startStripePayment(const QString& tier);
    void requestShahInvoice(const QString& tier, const QString& method);
    void startInvoicePolling();
    void pollInvoiceStatus();

private:
    WalletModel* m_walletModel;
    const PlatformStyle* m_platformStyle;

    // UI Elements
    QVBoxLayout* m_mainLayout{nullptr};
    QGroupBox* m_nftDetailsGroup{nullptr};
    QLineEdit* m_nftNameEdit{nullptr};
    QTextEdit* m_descriptionEdit{nullptr};

    QGroupBox* m_imageGroup{nullptr};
    QLabel* m_imagePreview{nullptr};
    QPushButton* m_imageUploadButton{nullptr};
    QPushButton* m_imageClearButton{nullptr};
    QString m_imageFilePath;

    QGroupBox* m_attributesGroup{nullptr};
    QTableWidget* m_attributesTable{nullptr};
    QLineEdit* m_attributeKeyEdit{nullptr};
    QLineEdit* m_attributeValueEdit{nullptr};
    QPushButton* m_addAttributeButton{nullptr};
    QPushButton* m_removeAttributeButton{nullptr};
    QMap<QString, QString> m_attributes;

    QGroupBox* m_feeGroup{nullptr};
    QComboBox* m_tierCombo{nullptr};
    QLabel* m_tierBenefits{nullptr};
    QLabel* m_feeLabel{nullptr};
    QLabel* m_balanceLabel{nullptr};
    QProgressBar* m_balanceProgress{nullptr};

    // Payment UI
    QHBoxLayout* m_paymentLayout{nullptr};
    QComboBox* m_paymentMethodCombo{nullptr};
    QLabel* m_paymentStatusLabel{nullptr};

    QHBoxLayout* m_buttonLayout{nullptr};
    QPushButton* m_payVerifyButton{nullptr};
    QPushButton* m_mintButton{nullptr};
    QPushButton* m_clearButton{nullptr};

    // State
    bool m_nameValid;
    bool m_imageValid;
    bool m_hasSufficientBalance;
    bool m_paymentVerified{false};
    qint64 m_creationFee; // in satoshis of SHAH
    qint64 m_currentBalance;

    // Network/payment state
    QNetworkAccessManager m_net;
    QTimer m_invoicePollTimer;
    QString m_invoiceId;
    QString m_apiBaseUrl;
};

#endif // SHAHCOIN_QT_NFTCREATORPANEL_H
