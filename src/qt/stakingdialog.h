// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_STAKINGDIALOG_H
#define SHAHCOIN_QT_STAKINGDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QComboBox>

#include <wallet/wallet.h>
#include <wallet/staking.h>

class WalletModel;
class ClientModel;

QT_BEGIN_NAMESPACE
class QTimer;
class QTableWidget;
class QTableWidgetItem;
class QLineEdit;
class QPushButton;
class QLabel;
class QProgressBar;
class QCheckBox;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QGroupBox;
class QTabWidget;
class QTextEdit;
class QComboBox;
QT_END_NAMESPACE

/**
 * Staking Dialog
 * Provides a user interface for managing staking operations
 */
class StakingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StakingDialog(QWidget* parent = nullptr);
    ~StakingDialog();

    void setModel(WalletModel* model);
    void setClientModel(ClientModel* model);

private Q_SLOTS:
    // Staking control
    void onEnableStakingClicked();
    void onDisableStakingClicked();
    
    // Stake management
    void onAddStakeClicked();
    void onRemoveStakeClicked();
    void onUpdateStakeClicked();
    
    // UI updates
    void updateStakingInfo();
    void updateStakesTable();
    void updateValidatorsTable();
    void updateStatistics();
    
    // Timer slots
    void onUpdateTimer();

private:
    // UI setup
    void setupUI();
    void setupStakingControlTab();
    void setupStakesTab();
    void setupValidatorsTab();
    void setupStatisticsTab();
    
    // Data helpers
    void refreshStakingInfo();
    void refreshStakes();
    void refreshValidators();
    void refreshStatistics();
    
    // Validation
    bool validateStakeAmount(const QString& amount);
    bool validateAddress(const QString& address);

private:
    WalletModel* m_walletModel;
    ClientModel* m_clientModel;
    
    // UI components
    QTabWidget* m_tabWidget;
    QTimer* m_updateTimer;
    
    // Staking control tab
    QGroupBox* m_stakingControlGroup;
    QCheckBox* m_enableStakingCheckBox;
    QPushButton* m_enableStakingButton;
    QPushButton* m_disableStakingButton;
    QLabel* m_stakingStatusLabel;
    QLabel* m_totalStakeLabel;
    QLabel* m_availableStakeLabel;
    QLabel* m_validatorCountLabel;
    QLabel* m_expectedTimeLabel;
    
    // Stakes tab
    QGroupBox* m_stakesGroup;
    QTableWidget* m_stakesTable;
    QLineEdit* m_addStakeAddressEdit;
    QLineEdit* m_addStakeAmountEdit;
    QPushButton* m_addStakeButton;
    QPushButton* m_removeStakeButton;
    QPushButton* m_updateStakeButton;
    
    // Validators tab
    QGroupBox* m_validatorsGroup;
    QTableWidget* m_validatorsTable;
    QPushButton* m_refreshValidatorsButton;
    
    // Statistics tab
    QGroupBox* m_statisticsGroup;
    QLabel* m_blocksCreatedLabel;
    QLabel* m_blocksSubmittedLabel;
    QLabel* m_blocksAcceptedLabel;
    QLabel* m_totalRewardsLabel;
    QLabel* m_lastBlockTimeLabel;
    QProgressBar* m_stakingProgressBar;
    QTextEdit* m_stakingLogText;
    
    // Data
    CStakingInfo m_stakingInfo;
    std::vector<CStakeValidator> m_stakes;
    std::vector<CStakeValidator> m_validators;
};

/**
 * Stakes Table Widget
 * Displays and manages stakes in a table format
 */
class StakesTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit StakesTableWidget(QWidget* parent = nullptr);
    
    void setStakes(const std::vector<CStakeValidator>& stakes);
    std::vector<CStakeValidator> getSelectedStakes() const;

private:
    void setupTable();
    void updateTable();
    
private:
    std::vector<CStakeValidator> m_stakes;
};

/**
 * Validators Table Widget
 * Displays network validators in a table format
 */
class ValidatorsTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit ValidatorsTableWidget(QWidget* parent = nullptr);
    
    void setValidators(const std::vector<CStakeValidator>& validators);

private:
    void setupTable();
    void updateTable();
    
private:
    std::vector<CStakeValidator> m_validators;
};

/**
 * Staking Statistics Widget
 * Displays staking statistics and performance metrics
 */
class StakingStatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StakingStatisticsWidget(QWidget* parent = nullptr);
    
    void updateStatistics(const CStakingInfo& info);

private:
    void setupUI();
    
private:
    QLabel* m_totalStakeLabel;
    QLabel* m_availableStakeLabel;
    QLabel* m_validatorCountLabel;
    QLabel* m_expectedTimeLabel;
    QLabel* m_stakeDifficultyLabel;
    QLabel* m_blockRewardLabel;
    QProgressBar* m_stakingProgressBar;
};

/**
 * Add Stake Dialog
 * Dialog for adding new stakes
 */
class AddStakeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddStakeDialog(QWidget* parent = nullptr);
    
    QString getAddress() const;
    CAmount getAmount() const;

private Q_SLOTS:
    void onAddressChanged();
    void onAmountChanged();
    void onAddClicked();
    void onCancelClicked();

private:
    void setupUI();
    void validateInputs();

private:
    QLineEdit* m_addressEdit;
    QLineEdit* m_amountEdit;
    QPushButton* m_addButton;
    QPushButton* m_cancelButton;
    QLabel* m_errorLabel;
    
    bool m_addressValid;
    bool m_amountValid;
};

/**
 * Update Stake Dialog
 * Dialog for updating existing stakes
 */
class UpdateStakeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateStakeDialog(const CStakeValidator& stake, QWidget* parent = nullptr);
    
    CAmount getNewAmount() const;

private Q_SLOTS:
    void onAmountChanged();
    void onUpdateClicked();
    void onCancelClicked();

private:
    void setupUI();
    void validateInputs();

private:
    CStakeValidator m_stake;
    QLineEdit* m_amountEdit;
    QPushButton* m_updateButton;
    QPushButton* m_cancelButton;
    QLabel* m_errorLabel;
    
    bool m_amountValid;
};

#endif // SHAHCOIN_QT_STAKINGDIALOG_H
