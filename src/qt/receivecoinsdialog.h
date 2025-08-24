// Copyright (c) 2011-2021 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_RECEIVECOINSDIALOG_H
#define SHAHCOIN_QT_RECEIVECOINSDIALOG_H

#include <qt/guiutil.h>
#include <qt/enhancedaddressbook.h>

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QPixmap>
#include <QPainter>
#include <QHeaderView>
#include <QItemSelection>
#include <QKeyEvent>
#include <QMenu>
#include <QPoint>
#include <QVariant>

class PlatformStyle;
class WalletModel;

namespace Ui {
    class ReceiveCoinsDialog;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Dialog for requesting payment of shahcoins */
class ReceiveCoinsDialog : public QDialog
{
    Q_OBJECT

public:
    enum ColumnWidths {
        DATE_COLUMN_WIDTH = 130,
        LABEL_COLUMN_WIDTH = 120,
        AMOUNT_MINIMUM_COLUMN_WIDTH = 180,
        MINIMUM_COLUMN_WIDTH = 130
    };

    explicit ReceiveCoinsDialog(const PlatformStyle *platformStyle, QWidget *parent = nullptr);
    ~ReceiveCoinsDialog();

    void setModel(WalletModel *model);

public Q_SLOTS:
    void clear();
    void reject() override;
    void accept() override;

private:
    Ui::ReceiveCoinsDialog *ui;
    WalletModel* model{nullptr};
    QMenu *contextMenu;
    QAction* copyLabelAction;
    QAction* copyMessageAction;
    QAction* copyAmountAction;
    const PlatformStyle *platformStyle;
    
    // QR Code components
    QLabel* m_qrCodeLabel;
    QLineEdit* m_qrAmountEdit;
    QLineEdit* m_qrLabelEdit;
    QLineEdit* m_qrMessageEdit;
    QPushButton* m_copyAddressButton;
    QPushButton* m_copyURIButton;
    QPushButton* m_saveQRButton;
    QPixmap m_qrPixmap;

    QModelIndex selectedRow();
    void copyColumnToClipboard(int column);

private Q_SLOTS:
    void on_receiveButton_clicked();
    void on_showRequestButton_clicked();
    void on_removeRequestButton_clicked();
    void on_recentRequestsView_doubleClicked(const QModelIndex &index);
    void recentRequestsView_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void updateDisplayUnit();
    void showMenu(const QPoint &point);
    void copyURI();
    void copyAddress();
    void copyLabel();
    void copyMessage();
    void copyAmount();
    void on_manageAddressBookClicked();
    
    // QR Code methods
    void setupQRCodeSection();
    void updateQRCode();
    void generateQRCode();
    QString generateQRString() const;
    void onQRAmountChanged();
    void onQRLabelChanged();
    void onQRMessageChanged();
    void onCopyAddressClicked();
    void onCopyURIClicked();
    void onSaveQRClicked();
};

#endif // SHAHCOIN_QT_RECEIVECOINSDIALOG_H
