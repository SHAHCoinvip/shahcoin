// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

#ifndef VAULTMANAGER_H
#define VAULTMANAGER_H

#include <QWidget>
#include <QTimer>
#include <QTableWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QTabWidget>
#include <QWebEngineView>

class ClientModel;
class WalletModel;

/**
 * VaultManager - DevKit / Tools Interface
 * 
 * Provides tools to interact with:
 * - Token/NFT factory deployment
 * - SHI-20/SHI-30 token creation
 * - NFT minting and management
 * - Smart contract interaction
 * - Development utilities
 */
class VaultManager : public QWidget
{
    Q_OBJECT

public:
    explicit VaultManager(QWidget *parent = nullptr);
    ~VaultManager();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);

private slots:
    void onTokenFactoryClicked();
    void onNFTFactoryClicked();
    void onDeployContractClicked();
    void onInteractContractClicked();
    void onGenerateSDKClicked();
    void onTestNetworkClicked();
    void onDocumentationClicked();

private:
    void setupUI();
    void setupTokenFactory();
    void setupNFTFactory();
    void setupContractDeployment();
    void setupContractInteraction();
    void setupSDKGenerator();
    void setupTestingTools();
    void connectSignals();

    // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // Token Factory Tab
    QWidget *m_tokenFactoryTab;
    QVBoxLayout *m_tokenFactoryLayout;
    QGroupBox *m_tokenCreationGroup;
    QGridLayout *m_tokenCreationLayout;
    QLineEdit *m_tokenNameEdit;
    QLineEdit *m_tokenSymbolEdit;
    QSpinBox *m_tokenDecimalsSpin;
    QDoubleSpinBox *m_tokenSupplySpin;
    QComboBox *m_tokenStandardCombo;
    QPushButton *m_createTokenButton;
    QTableWidget *m_tokenListTable;
    
    // NFT Factory Tab
    QWidget *m_nftFactoryTab;
    QVBoxLayout *m_nftFactoryLayout;
    QGroupBox *m_nftCreationGroup;
    QGridLayout *m_nftCreationLayout;
    QLineEdit *m_nftNameEdit;
    QLineEdit *m_nftSymbolEdit;
    QLineEdit *m_nftMetadataEdit;
    QComboBox *m_nftStandardCombo;
    QPushButton *m_createNFTButton;
    QTableWidget *m_nftListTable;
    
    // Contract Deployment Tab
    QWidget *m_contractDeploymentTab;
    QVBoxLayout *m_contractDeploymentLayout;
    QGroupBox *m_contractDeploymentGroup;
    QGridLayout *m_contractDeploymentLayout2;
    QTextEdit *m_contractSourceEdit;
    QComboBox *m_contractTypeCombo;
    QPushButton *m_deployContractButton;
    QLabel *m_deploymentStatusLabel;
    QProgressBar *m_deploymentProgressBar;
    
    // Contract Interaction Tab
    QWidget *m_contractInteractionTab;
    QVBoxLayout *m_contractInteractionLayout;
    QGroupBox *m_contractInteractionGroup;
    QGridLayout *m_contractInteractionLayout2;
    QLineEdit *m_contractAddressEdit;
    QComboBox *m_contractFunctionCombo;
    QTextEdit *m_functionParametersEdit;
    QPushButton *m_callFunctionButton;
    QTextEdit *m_functionResultEdit;
    
    // SDK Generator Tab
    QWidget *m_sdkGeneratorTab;
    QVBoxLayout *m_sdkGeneratorLayout;
    QGroupBox *m_sdkGeneratorGroup;
    QGridLayout *m_sdkGeneratorLayout2;
    QComboBox *m_sdkLanguageCombo;
    QCheckBox *m_includeTokensCheck;
    QCheckBox *m_includeNFTsCheck;
    QCheckBox *m_includeStakingCheck;
    QPushButton *m_generateSDKButton;
    QLabel *m_sdkStatusLabel;
    
    // Testing Tools Tab
    QWidget *m_testingToolsTab;
    QVBoxLayout *m_testingToolsLayout;
    QGroupBox *m_testingToolsGroup;
    QGridLayout *m_testingToolsLayout2;
    QPushButton *m_testNetworkButton;
    QPushButton *m_testTokensButton;
    QPushButton *m_testNFTsButton;
    QPushButton *m_testStakingButton;
    QLabel *m_testingStatusLabel;
    
    // Documentation Tab
    QWidget *m_documentationTab;
    QVBoxLayout *m_documentationLayout;
    QWebEngineView *m_documentationView;
    
    // Models
    ClientModel *m_clientModel;
    WalletModel *m_walletModel;
    
    // Data storage
    struct TokenData {
        QString name;
        QString symbol;
        QString address;
        int decimals;
        double supply;
        QString standard;
    };
    
    struct NFTData {
        QString name;
        QString symbol;
        QString address;
        QString metadata;
        QString standard;
    };
    
    QList<TokenData> m_tokens;
    QList<NFTData> m_nfts;
};

#endif // VAULTMANAGER_H
