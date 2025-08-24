#ifndef SHAHCOIN_QT_QRCODEWIDGET_H
#define SHAHCOIN_QT_QRCODEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>

class WalletModel;

class QRCodeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QRCodeWidget(QWidget* parent = nullptr);
    void setAddress(const QString& address);
    void setAmount(double amount);
    void generateQRCode();
    void setWalletModel(WalletModel* model);

public Q_SLOTS:
    void onAmountChanged(double amount);
    void onCopyAddressClicked();
    void onSaveQRClicked();

private:
    void setupUI();
    void updateQRCode();
    QString generateQRString() const;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_qrGroup;
    QLabel* m_qrCodeLabel;
    QLabel* m_addressLabel;
    QLineEdit* m_addressEdit;
    QDoubleSpinBox* m_amountSpinBox;
    QPushButton* m_copyAddressButton;
    QPushButton* m_saveQRButton;
    
                    // Data
                QString m_address;
                double m_amount;
                QPixmap m_qrPixmap;
                WalletModel* m_walletModel;
};

#endif // SHAHCOIN_QT_QRCODEWIDGET_H
