#ifndef SHAHCOIN_QT_ASSETCREATORPANEL_H
#define SHAHCOIN_QT_ASSETCREATORPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QTabWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <QPixmap>
#include <QBuffer>
#include <QByteArray>
#include <QComboBox>
#include <QCheckBox>
#include <QProgressBar>

class WalletModel;
class PlatformStyle;

/**
 * Asset Creator Panel
 * 
 * Provides a unified interface for creating native tokens and NFTs
 * on the Shahcoin blockchain directly from the desktop wallet.
 */
class AssetCreatorPanel : public QWidget
{
    Q_OBJECT
public:
    explicit AssetCreatorPanel(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~AssetCreatorPanel();
    void setWalletModel(WalletModel* walletModel);

public Q_SLOTS:
    void refreshData();

private Q_SLOTS:
    // Token Creation Slots
    void onTokenNameChanged();
    void onTokenSymbolChanged();
    void onTokenSupplyChanged();
    void onTokenDecimalsChanged();
    void onTokenDescriptionChanged();
    void onTokenLogoUploadClicked();
    void onCreateTokenClicked();
    void onClearTokenFormClicked();
    
    // NFT Creation Slots
    void onNFTNameChanged();
    void onNFTDescriptionChanged();
    void onNFTImageUploadClicked();
    void onNFTCollectionChanged();
    void onNFTAttributesChanged();
    void onMintNFTClicked();
    void onClearNFTFormClicked();
    
    // Shared Slots
    void onTabChanged(int index);
    void onConfirmTokenCreation();
    void onCancelTokenCreation();
    void onConfirmNFTCreation();
    void onCancelNFTCreation();
    void onFeeCalculationTimer();

private:
    void setupUI();
    void setupTokenMintingTab();
    void setupNFTMintingTab();
    void connectSignals();
    void applyTheme();
    void calculateTokenCreationFee();
    void calculateNFTCreationFee();
    void validateTokenInputs();
    void validateNFTInputs();
    void showTokenCreationConfirmation();
    void showNFTCreationConfirmation();
    void executeTokenCreation();
    void executeNFTCreation();
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void refreshBalances();
    void loadCollections();
    QString encodeImageToBase64(const QString& filePath);
    bool isValidTokenName(const QString& name);
    bool isValidTokenSymbol(const QString& symbol);
    bool isValidNFTName(const QString& name);
    QString formatAmount(qint64 amount);
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Token Minting Tab
    QWidget* m_tokenTab;
    QVBoxLayout* m_tokenLayout;
    QGroupBox* m_tokenGroup;
    QFormLayout* m_tokenForm;
    QLineEdit* m_tokenNameEdit;
    QLineEdit* m_tokenSymbolEdit;
    QSpinBox* m_tokenSupplySpinBox;
    QSpinBox* m_tokenDecimalsSpinBox;
    QTextEdit* m_tokenDescriptionEdit;
    QLineEdit* m_tokenLogoPathEdit;
    QPushButton* m_tokenLogoUploadButton;
    QLabel* m_tokenCreationFeeLabel;
    QLabel* m_tokenBalanceLabel;
    QHBoxLayout* m_tokenButtonLayout;
    QPushButton* m_createTokenButton;
    QPushButton* m_clearTokenButton;
    
    // NFT Minting Tab
    QWidget* m_nftTab;
    QVBoxLayout* m_nftLayout;
    QGroupBox* m_nftGroup;
    QFormLayout* m_nftForm;
    QLineEdit* m_nftNameEdit;
    QTextEdit* m_nftDescriptionEdit;
    QLineEdit* m_nftImagePathEdit;
    QPushButton* m_nftImageUploadButton;
    QComboBox* m_nftCollectionCombo;
    QTextEdit* m_nftAttributesEdit;
    QLabel* m_nftCreationFeeLabel;
    QLabel* m_nftBalanceLabel;
    QHBoxLayout* m_nftButtonLayout;
    QPushButton* m_mintNFTButton;
    QPushButton* m_clearNFTButton;
    
    // Token Creation Confirmation Dialog
    QDialog* m_tokenConfirmDialog;
    QVBoxLayout* m_tokenConfirmLayout;
    QLabel* m_tokenConfirmTitleLabel;
    QLabel* m_tokenConfirmDetailsLabel;
    QTextEdit* m_tokenConfirmDetailsText;
    QHBoxLayout* m_tokenConfirmButtonLayout;
    QPushButton* m_tokenConfirmButton;
    QPushButton* m_tokenCancelButton;
    
    // NFT Creation Confirmation Dialog
    QDialog* m_nftConfirmDialog;
    QVBoxLayout* m_nftConfirmLayout;
    QLabel* m_nftConfirmTitleLabel;
    QLabel* m_nftConfirmDetailsLabel;
    QTextEdit* m_nftConfirmDetailsText;
    QHBoxLayout* m_nftConfirmButtonLayout;
    QPushButton* m_nftConfirmButton;
    QPushButton* m_nftCancelButton;
    
    // Data
    WalletModel* m_walletModel;
    const PlatformStyle* m_platformStyle;
    QTimer* m_feeCalculationTimer;
    
    // Token Creation Data
    QString m_tokenName;
    QString m_tokenSymbol;
    qint64 m_tokenSupply;
    int m_tokenDecimals;
    QString m_tokenDescription;
    QString m_tokenLogoPath;
    qint64 m_tokenCreationFee;
    
    // NFT Creation Data
    QString m_nftName;
    QString m_nftDescription;
    QString m_nftImagePath;
    QString m_nftCollection;
    QString m_nftAttributes;
    qint64 m_nftCreationFee;
    
    // Current Balances
    qint64 m_currentBalance;
    
    // Settings
    QSettings* m_settings;
    
    // Collections (simulated)
    QStringList m_collections;
    
    // Fee calculation data (simulated)
    double m_shahUsdPrice;
};

#endif // SHAHCOIN_QT_ASSETCREATORPANEL_H
