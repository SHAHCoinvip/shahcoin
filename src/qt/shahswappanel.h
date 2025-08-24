#ifndef SHAHCOIN_QT_SHAHSWAPPANEL_H
#define SHAHCOIN_QT_SHAHSWAPPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QProgressBar>
#include <QSlider>
#include <QCheckBox>
#include <QTextEdit>
#include <QTimer>
#include <QMessageBox>
#include <QDialog>
#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

class WalletModel;
class PlatformStyle;

/**
 * ShahSwap DEX Panel
 * 
 * Provides a decentralized exchange interface for swapping
 * native Shahcoin tokens within the wallet.
 */
class ShahSwapPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ShahSwapPanel(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~ShahSwapPanel();
    void setWalletModel(WalletModel* walletModel);

public Q_SLOTS:
    void refreshData();

private Q_SLOTS:
    void onTokenASelectionChanged();
    void onTokenBSelectionChanged();
    void onAmountAChanged();
    void onSwapClicked();
    void onSettingsClicked();
    void onRefreshClicked();
    void onHistoryClicked();
    void onPoolViewClicked();
    void onSlippageChanged();
    void onDeadlineChanged();
    void onAutoSlippageToggled();
    void onConfirmSwap();
    void onCancelSwap();

private:
    void setupUI();
    void setupSwapInterface();
    void setupSettings();
    void setupHistory();
    void setupPoolView();
    void connectSignals();
    void applyTheme();
    void loadTokenList();
    void calculateSwap();
    void updateTokenBalances();
    void updatePriceImpact();
    void updateSlippage();
    void validateSwap();
    void executeSwap();
    void showSwapConfirmation();
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void refreshSwapHistory();
    void refreshPoolData();
    QString formatTokenAmount(qint64 amount, int decimals);
    QString formatPrice(qint64 price);
    qint64 calculateMinimumReceived(qint64 inputAmount, double slippage);
    double calculatePriceImpact(qint64 inputAmount, qint64 outputAmount);
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Swap Tab
    QWidget* m_swapTab;
    QVBoxLayout* m_swapLayout;
    QGroupBox* m_swapGroup;
    QFormLayout* m_swapForm;
    QComboBox* m_tokenACombo;
    QComboBox* m_tokenBCombo;
    QLineEdit* m_amountAEdit;
    QLineEdit* m_amountBEdit;
    QLabel* m_balanceALabel;
    QLabel* m_balanceBLabel;
    QPushButton* m_swapButton;
    QPushButton* m_settingsButton;
    QPushButton* m_refreshButton;
    
    // Swap Details
    QGroupBox* m_detailsGroup;
    QVBoxLayout* m_detailsLayout;
    QLabel* m_priceImpactLabel;
    QLabel* m_minimumReceivedLabel;
    QLabel* m_liquidityFeeLabel;
    QLabel* m_slippageLabel;
    QLabel* m_routeLabel;
    
    // Settings Dialog
    QDialog* m_settingsDialog;
    QVBoxLayout* m_settingsLayout;
    QGroupBox* m_slippageGroup;
    QHBoxLayout* m_slippageLayout;
    QCheckBox* m_autoSlippageCheck;
    QDoubleSpinBox* m_slippageSpinBox;
    QSlider* m_slippageSlider;
    QLabel* m_slippageValueLabel;
    QGroupBox* m_deadlineGroup;
    QHBoxLayout* m_deadlineLayout;
    QSpinBox* m_deadlineSpinBox;
    QComboBox* m_deadlineUnitCombo;
    QPushButton* m_settingsSaveButton;
    QPushButton* m_settingsCancelButton;
    
    // History Tab
    QWidget* m_historyTab;
    QVBoxLayout* m_historyLayout;
    QTableWidget* m_historyTable;
    QPushButton* m_historyRefreshButton;
    
    // Pool View Tab
    QWidget* m_poolTab;
    QVBoxLayout* m_poolLayout;
    QTableWidget* m_poolTable;
    QPushButton* m_poolRefreshButton;
    
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
    QTimer* m_priceTimer;
    
    // Token Data
    struct TokenInfo {
        QString id;
        QString name;
        QString symbol;
        int decimals;
        qint64 balance;
        QString address;
    };
    QList<TokenInfo> m_tokenList;
    
    // Swap Data
    QString m_selectedTokenA;
    QString m_selectedTokenB;
    qint64 m_inputAmount;
    qint64 m_outputAmount;
    double m_currentSlippage;
    int m_transactionDeadline;
    bool m_autoSlippage;
    
    // Pool Data
    struct PoolInfo {
        QString tokenA;
        QString tokenB;
        qint64 reserveA;
        qint64 reserveB;
        qint64 totalSupply;
        double fee;
    };
    QList<PoolInfo> m_poolList;
    
    // History Data
    struct SwapHistory {
        QString txHash;
        QString tokenA;
        QString tokenB;
        qint64 amountA;
        qint64 amountB;
        qint64 timestamp;
        QString status;
    };
    QList<SwapHistory> m_swapHistory;
    
    // Settings
    QSettings* m_settings;
};

#endif // SHAHCOIN_QT_SHAHSWAPPANEL_H
