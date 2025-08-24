#ifndef SHAHCOIN_QT_COLDSTAKINGPANEL_H
#define SHAHCOIN_QT_COLDSTAKINGPANEL_H

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
#include <QTableWidget>
#include <QHeaderView>
#include <QProgressBar>
#include <QCheckBox>
#include <QTextEdit>
#include <QTimer>
#include <QMessageBox>
#include <QDialog>
#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileDialog>
#include <QTabWidget>
#include <QMenu>

class WalletModel;
class PlatformStyle;

/**
 * Cold Staking Panel
 * 
 * Provides a secure interface for setting up cold staking delegation,
 * allowing users to stake SHAH using offline addresses while delegating
 * staking work to online wallets.
 */
class ColdStakingPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ColdStakingPanel(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~ColdStakingPanel();
    void setWalletModel(WalletModel* walletModel);

public Q_SLOTS:
    void refreshData();

private Q_SLOTS:
    void onStartColdStakingClicked();
    void onUnstakeClicked();
    void onChangeDelegateClicked();
    void onExportConfigClicked();
    void onRefreshClicked();
    void onColdStakingRowClicked(int row, int column);
    void onColdStakingContextMenu(const QPoint& pos);
    void onViewColdStakingDetails();
    void onExportColdStakingData();
    void onHotAddressChanged();
    void onColdAddressChanged();
    void onAmountChanged();
    void onUnderstandCheckboxToggled();
    void onConfirmColdStaking();
    void onCancelColdStaking();

private:
    void setupUI();
    void setupColdStakingSetup();
    void setupColdStakingList();
    void setupManagementTools();
    void connectSignals();
    void applyTheme();
    void loadColdStakingData();
    void validateInputs();
    void showColdStakingConfirmation();
    void executeColdStaking();
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void refreshColdStakingList();
    void exportColdStakingConfig();
    QString formatStakeAmount(qint64 amount);
    QString formatAddress(const QString& address);
    bool isValidShahcoinAddress(const QString& address);
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Setup Tab
    QWidget* m_setupTab;
    QVBoxLayout* m_setupLayout;
    QGroupBox* m_setupGroup;
    QFormLayout* m_setupForm;
    QLineEdit* m_hotAddressEdit;
    QLineEdit* m_coldAddressEdit;
    QLineEdit* m_amountEdit;
    QCheckBox* m_understandCheckbox;
    QPushButton* m_startColdStakingButton;
    QPushButton* m_refreshButton;
    
    // Cold Staking List Tab
    QWidget* m_listTab;
    QVBoxLayout* m_listLayout;
    QTableWidget* m_coldStakingTable;
    QPushButton* m_listRefreshButton;
    
    // Management Tab
    QWidget* m_managementTab;
    QVBoxLayout* m_managementLayout;
    QGroupBox* m_managementGroup;
    QVBoxLayout* m_managementForm;
    QPushButton* m_unstakeButton;
    QPushButton* m_changeDelegateButton;
    QPushButton* m_exportConfigButton;
    
    // Confirmation Dialog
    QDialog* m_confirmDialog;
    QVBoxLayout* m_confirmLayout;
    QLabel* m_confirmTitleLabel;
    QLabel* m_confirmDetailsLabel;
    QTextEdit* m_confirmDetailsText;
    QHBoxLayout* m_confirmButtonLayout;
    QPushButton* m_confirmButton;
    QPushButton* m_cancelButton;
    
    // Data
    WalletModel* m_walletModel;
    const PlatformStyle* m_platformStyle;
    QTimer* m_refreshTimer;
    
    // Cold Staking Data
    struct ColdStakingInfo {
        QString id;
        QString hotAddress;
        QString coldAddress;
        qint64 stakeAmount;
        qint64 rewardsEarned;
        qint64 startTime;
        QString status;
        int confirmations;
    };
    QList<ColdStakingInfo> m_coldStakingList;
    
    // Current Setup Data
    QString m_hotAddress;
    QString m_coldAddress;
    qint64 m_stakeAmount;
    bool m_understandChecked;
    
    // Settings
    QSettings* m_settings;
    
    // Context Menu
    QMenu* m_coldStakingContextMenu;
    QString m_selectedColdStakingId;
};

#endif // SHAHCOIN_QT_COLDSTAKINGPANEL_H
