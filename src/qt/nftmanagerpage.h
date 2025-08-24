// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_NFTMANAGERPAGE_H
#define SHAHCOIN_QT_NFTMANAGERPAGE_H

#include <QWidget>
#include <QTimer>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTabWidget>

class ClientModel;
class WalletModel;
class PlatformStyle;

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QFormLayout;
class QGroupBox;
class QTableWidget;
class QTableWidgetItem;
class QLabel;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QComboBox;
class QTabWidget;
class QPixmap;
QT_END_NAMESPACE

/**
 * @brief The NFTManagerPage class provides a comprehensive NFT management interface for SHAHCOIN Core
 * 
 * This page allows users to:
 * - Mint new NFTs with metadata and images
 * - View and manage their NFT collection
 * - Transfer NFTs to other addresses
 * - Browse and search NFTs
 */
class NFTManagerPage : public QWidget
{
    Q_OBJECT

public:
    explicit NFTManagerPage(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~NFTManagerPage();

    void setClientModel(ClientModel* clientModel);
    void setWalletModel(WalletModel* walletModel);

    void showOutOfSyncWarning(bool fShow);

public Q_SLOTS:
    void updateNFTList();
    void onMintNFT();
    void onTransferNFT();
    void onBrowseImage();
    void onClearForm();
    void onRefreshNFTs();
    void onPreviewUpdate();
    void onFilterChanged();
    void onSearchChanged();

Q_SIGNALS:
    void message(const QString& title, const QString& message, unsigned int style);

private Q_SLOTS:
    void updateDisplayUnit();
    void onTabChanged(int index);

private:
    void setupUI();
    void connectSignals();
    void updateMintFee();
    void updateTransferFee();
    void updatePreview();
    void loadNFTList();
    void populateNFTTable();
    void populateTransferComboBox();
    void showNFTError(const QString& message);
    void showNFTSuccess(const QString& message);
    bool validateMintForm();
    bool validateTransferForm();
    QPixmap loadImageFromPath(const QString& path);

    // UI Components - Mint Tab
    QLineEdit* lineEdit_Name;
    QTextEdit* textEdit_Description;
    QLineEdit* lineEdit_ImagePath;
    QPushButton* pushButton_BrowseImage;
    QComboBox* comboBox_Collection;
    QLabel* label_MintFeeValue;
    QLabel* label_PreviewImage;
    QLabel* label_PreviewName;
    QLabel* label_PreviewDescription;
    QLabel* label_PreviewCollection;
    QPushButton* pushButton_MintNFT;
    QPushButton* pushButton_ClearForm;

    // UI Components - My NFTs Tab
    QComboBox* comboBox_Filter;
    QLineEdit* lineEdit_Search;
    QPushButton* pushButton_RefreshNFTs;
    QTableWidget* tableWidget_MyNFTs;

    // UI Components - Transfer Tab
    QComboBox* comboBox_SelectNFT;
    QLineEdit* lineEdit_RecipientAddress;
    QLabel* label_TransferFeeValue;
    QPushButton* pushButton_TransferNFT;

    // Tab Widget
    QTabWidget* tabWidget;

    // Models
    ClientModel* clientModel{nullptr};
    WalletModel* walletModel{nullptr};
    const PlatformStyle* platformStyle;

    // Timer for periodic updates
    QTimer* updateTimer;

    // NFT data structures
    struct NFTData {
        QString tokenId;
        QString name;
        QString description;
        QString imagePath;
        QString collection;
        QString owner;
        QDateTime mintDate;
        QString metadata;
    };

    QList<NFTData> userNFTs;
    QList<NFTData> allNFTs;

    // Constants
    static constexpr CAmount NFT_MINT_FEE = 100 * COIN;
    static constexpr CAmount NFT_TRANSFER_FEE = 1000; // 0.001 SHAH
    static constexpr int UPDATE_INTERVAL_MS = 60000; // 1 minute
    static constexpr int MAX_IMAGE_SIZE = 1024 * 1024; // 1MB
    static constexpr int PREVIEW_SIZE = 200;
};

#endif // SHAHCOIN_QT_NFTMANAGERPAGE_H
