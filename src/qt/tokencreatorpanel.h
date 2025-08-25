// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_TOKENCREATORPANEL_H
#define SHAHCOIN_QT_TOKENCREATORPANEL_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QProgressBar>

class WalletModel;
class PlatformStyle;

/**
 * Token Creator Panel Widget
 *
 * Allows users to create native Shahcoin tokens with:
 * - Token name, symbol, supply, decimals
 * - Description and optional logo
 * - Dynamic fee calculation
 * - Validation and confirmation
 */
class TokenCreatorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TokenCreatorPanel(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~TokenCreatorPanel();

    void setWalletModel(WalletModel* walletModel);

public Q_SLOTS:
    void refreshData();

private Q_SLOTS:
    void onCreateTokenClicked();
    void onClearClicked();
    void onLogoUploadClicked();
    void onTokenNameChanged();
    void onTokenSymbolChanged();
    void onSupplyChanged();
    void onDecimalsChanged();

private:
    void setupUI();
    void connectSignals();
    void applyTheme();
    void updateFeeDisplay();
    void updateCreateButton();
    bool validateInputs();
    void showConfirmationDialog();
    void showSuccessDialog(const QString& tokenId);
    void showErrorDialog(const QString& error);
    QString encodeLogoToBase64(const QString& filePath);
    void clearForm();

    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Token Details Group
    QGroupBox* m_tokenDetailsGroup;
    QLineEdit* m_tokenNameEdit;
    QLineEdit* m_tokenSymbolEdit;
    QSpinBox* m_supplySpinBox;
    QSpinBox* m_decimalsSpinBox;
    QTextEdit* m_descriptionEdit;
    
    // Logo Group
    QGroupBox* m_logoGroup;
    QLabel* m_logoPreview;
    QPushButton* m_logoUploadButton;
    QPushButton* m_logoClearButton;
    QString m_logoFilePath;
    
    // Fee and Summary Group
    QGroupBox* m_feeGroup;
    QLabel* m_feeLabel;
    QLabel* m_balanceLabel;
    QProgressBar* m_balanceProgress;
    
    // Action Buttons
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_createButton;
    QPushButton* m_clearButton;
    
    // Models
    WalletModel* m_walletModel;
    const PlatformStyle* m_platformStyle;
    
    // Validation state
    bool m_nameValid;
    bool m_symbolValid;
    bool m_supplyValid;
    bool m_decimalsValid;
    bool m_hasSufficientBalance;
    
    // Cached values
    CAmount m_creationFee;
    CAmount m_currentBalance;
};

#endif // SHAHCOIN_QT_TOKENCREATORPANEL_H
