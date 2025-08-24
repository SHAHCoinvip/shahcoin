#ifndef SHAHCOIN_QT_MARKETPLACELISTINGDIALOG_H
#define SHAHCOIN_QT_MARKETPLACELISTINGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

class WalletModel;

class MarketplaceListingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MarketplaceListingDialog(QWidget* parent = nullptr);
    void setWalletModel(WalletModel* model);

    // Input getters
    QString getSelectedNFTId() const;
    QString getTitle() const;
    QString getDescription() const;
    QString getCategory() const;
    QString getTags() const;
    double getPriceUSD() const;
    qint64 getPriceSatoshis() const;
    bool getSubmitForVerification() const;

    // Pre-fill
    void presetNFT(const QString& nftId, const QString& defaultTitle);

signals:
    void listingSubmitted(const QString& nftId,
                          const QString& title,
                          const QString& description,
                          const QString& category,
                          const QString& tags,
                          qint64 priceSatoshis,
                          double priceUSD,
                          bool submitForVerification);

private slots:
    void onPriceChanged();
    void onBrowseNFTClicked();
    void onSubmitClicked();
    void onCancelClicked();

private:
    void setupUI();
    void connectSignals();
    void updatePriceConversion();

    WalletModel* m_walletModel { nullptr };

    QLineEdit* m_nftIdEdit { nullptr };
    QPushButton* m_browseNFTButton { nullptr };
    QLineEdit* m_titleEdit { nullptr };
    QTextEdit* m_descriptionEdit { nullptr };
    QComboBox* m_categoryCombo { nullptr };
    QLineEdit* m_tagsEdit { nullptr };
    QLineEdit* m_priceShahEdit { nullptr };
    QLineEdit* m_priceUsdEdit { nullptr };
    QLabel* m_priceHint { nullptr };
    QCheckBox* m_submitForVerification { nullptr };
    QPushButton* m_submitButton { nullptr };
    QPushButton* m_cancelButton { nullptr };
};

#endif // SHAHCOIN_QT_MARKETPLACELISTINGDIALOG_H
