#ifndef SHAHCOIN_QT_CUSTOMFEESELECTIONDIALOG_H
#define SHAHCOIN_QT_CUSTOMFEESELECTIONDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>

class PlatformStyle;
class WalletModel;

namespace Ui {
    class CustomFeeSelectionDialog;
}

/**
 * Custom Fee Selection Dialog
 * Provides both Basic Mode (Standard, High Priority, Eco Mode) and Advanced Mode (custom fee per kilobyte)
 */
class CustomFeeSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    enum FeeMode {
        Standard,      // Standard fee (default)
        HighPriority,  // High priority (faster, more expensive)
        EcoMode,       // Eco mode (slower, cheaper)
        Custom         // Custom fee per kilobyte
    };

    explicit CustomFeeSelectionDialog(const PlatformStyle *platformStyle, QWidget *parent = nullptr);
    ~CustomFeeSelectionDialog();

    void setModel(WalletModel *model);
    
    // Get selected fee mode and value
    FeeMode getSelectedMode() const;
    CAmount getCustomFeePerKB() const;
    int getConfirmationTarget() const;
    
    // Set current values
    void setCurrentMode(FeeMode mode);
    void setCurrentCustomFee(CAmount feePerKB);
    void setCurrentConfirmationTarget(int target);

public Q_SLOTS:
    void accept() override;
    void reject() override;

private Q_SLOTS:
    void onModeChanged();
    void onCustomFeeChanged();
    void onConfirmationTargetChanged();
    void onAdvancedModeToggled(bool enabled);
    void updateFeeEstimates();
    void onResetToDefaults();

private:
    void setupUI();
    void setupBasicMode();
    void setupAdvancedMode();
    void updateFeeDisplay();
    void updateConfirmationTime();
    void updateTransactionSize();
    void validateCustomFee();
    
    // Calculate fee estimates
    CAmount getEstimatedFee(FeeMode mode) const;
    int getEstimatedConfirmationTime(FeeMode mode) const;
    QString getEstimatedConfirmationTimeString(int blocks) const;
    
    Ui::CustomFeeSelectionDialog *ui;
    const PlatformStyle *platformStyle;
    WalletModel* model;
    
    // UI Components
    QGroupBox* m_basicModeGroup;
    QGroupBox* m_advancedModeGroup;
    QCheckBox* m_advancedModeCheckBox;
    
    // Basic Mode
    QRadioButton* m_standardFeeRadio;
    QRadioButton* m_highPriorityRadio;
    QRadioButton* m_ecoModeRadio;
    
    // Advanced Mode
    QRadioButton* m_customFeeRadio;
    QSpinBox* m_customFeeSpinBox;
    QLabel* m_customFeeUnitLabel;
    QLabel* m_estimatedConfirmationLabel;
    QLabel* m_transactionSizeLabel;
    QLabel* m_totalFeeLabel;
    QLabel* m_feeWarningLabel;
    
    // Confirmation target selector
    QComboBox* m_confirmationTargetCombo;
    
    // Current values
    FeeMode m_currentMode;
    CAmount m_currentCustomFee;
    int m_currentConfirmationTarget;
    
    // Fee estimates (in satoshis per kilobyte)
    CAmount m_standardFeeRate;
    CAmount m_highPriorityFeeRate;
    CAmount m_ecoModeFeeRate;
    
    // Transaction info
    int m_estimatedTransactionSize; // in bytes
    CAmount m_minRelayFee;
    std::unique_ptr<CCoinControl> m_coin_control;
};

#endif // SHAHCOIN_QT_CUSTOMFEESELECTIONDIALOG_H
