// Copyright (c) 2011-2022 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/customfeeselectiondialog.h>
#include <qt/platformstyle.h>
#include <qt/walletmodel.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>
#include <qt/optionsmodel.h>

#include <QApplication>
#include <QButtonGroup>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

#include <wallet/wallet.h>
#include <policy/fees.h>
#include <validation.h>
#include <wallet/coincontrol.h>

CustomFeeSelectionDialog::CustomFeeSelectionDialog(const PlatformStyle *_platformStyle, QWidget *parent)
    : QDialog(parent, GUIUtil::dialog_flags)
    , platformStyle(_platformStyle)
    , model(nullptr)
    , m_currentMode(Standard)
    , m_currentCustomFee(0)
    , m_currentConfirmationTarget(25)
    , m_standardFeeRate(0)
    , m_highPriorityFeeRate(0)
    , m_ecoModeFeeRate(0)
    , m_estimatedTransactionSize(250) // Default estimate
    , m_minRelayFee(0)
    , m_coin_control(std::make_unique<CCoinControl>())
{
    setupUI();
    setupBasicMode();
    setupAdvancedMode();
    
    // Set default values
    m_standardFeeRadio->setChecked(true);
    m_advancedModeCheckBox->setChecked(false);
    m_advancedModeGroup->setVisible(false);
    
    // Connect signals
    connect(m_standardFeeRadio, &QRadioButton::toggled, this, &CustomFeeSelectionDialog::onModeChanged);
    connect(m_highPriorityRadio, &QRadioButton::toggled, this, &CustomFeeSelectionDialog::onModeChanged);
    connect(m_ecoModeRadio, &QRadioButton::toggled, this, &CustomFeeSelectionDialog::onModeChanged);
    connect(m_customFeeRadio, &QRadioButton::toggled, this, &CustomFeeSelectionDialog::onModeChanged);
    connect(m_customFeeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &CustomFeeSelectionDialog::onCustomFeeChanged);
    connect(m_confirmationTargetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CustomFeeSelectionDialog::onConfirmationTargetChanged);
    connect(m_advancedModeCheckBox, &QCheckBox::toggled, this, &CustomFeeSelectionDialog::onAdvancedModeToggled);
    
    // Set window properties
    setWindowTitle(tr("Custom Fee Selection"));
    setModal(true);
    resize(500, 400);
    
    // Update fee estimates after a short delay to allow UI to be ready
    QTimer::singleShot(100, this, &CustomFeeSelectionDialog::updateFeeEstimates);
}

CustomFeeSelectionDialog::~CustomFeeSelectionDialog()
{
    // Save user preferences
    QSettings settings;
    settings.setValue("CustomFeeSelection/AdvancedMode", m_advancedModeCheckBox->isChecked());
    settings.setValue("CustomFeeSelection/LastMode", static_cast<int>(m_currentMode));
    settings.setValue("CustomFeeSelection/LastCustomFee", m_currentCustomFee);
}

void CustomFeeSelectionDialog::setModel(WalletModel *_model)
{
    model = _model;
    updateFeeEstimates();
}

void CustomFeeSelectionDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Advanced Mode Toggle
    m_advancedModeCheckBox = new QCheckBox(tr("Advanced Mode"), this);
    m_advancedModeCheckBox->setToolTip(tr("Enable advanced fee options for custom control"));
    mainLayout->addWidget(m_advancedModeCheckBox);
    
    // Basic Mode Group
    m_basicModeGroup = new QGroupBox(tr("Basic Mode"), this);
    mainLayout->addWidget(m_basicModeGroup);
    
    // Advanced Mode Group
    m_advancedModeGroup = new QGroupBox(tr("Advanced Mode"), this);
    mainLayout->addWidget(m_advancedModeGroup);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* resetButton = new QPushButton(tr("Reset to Defaults"), this);
    QPushButton* okButton = new QPushButton(tr("OK"), this);
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
    
    connect(resetButton, &QPushButton::clicked, this, &CustomFeeSelectionDialog::onResetToDefaults);
    connect(okButton, &QPushButton::clicked, this, &CustomFeeSelectionDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &CustomFeeSelectionDialog::reject);
    
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Load saved preferences
    QSettings settings;
    m_advancedModeCheckBox->setChecked(settings.value("CustomFeeSelection/AdvancedMode", false).toBool());
    m_currentMode = static_cast<FeeMode>(settings.value("CustomFeeSelection/LastMode", Standard).toInt());
    m_currentCustomFee = settings.value("CustomFeeSelection/LastCustomFee", 0).toLongLong();
}

void CustomFeeSelectionDialog::setupBasicMode()
{
    QVBoxLayout* basicLayout = new QVBoxLayout(m_basicModeGroup);
    
    // Fee mode radio buttons
    m_standardFeeRadio = new QRadioButton(tr("Standard Fee"), this);
    m_standardFeeRadio->setToolTip(tr("Standard fee for normal transaction priority"));
    
    m_highPriorityRadio = new QRadioButton(tr("High Priority"), this);
    m_highPriorityRadio->setToolTip(tr("Higher fee for faster confirmation"));
    
    m_ecoModeRadio = new QRadioButton(tr("Eco Mode"), this);
    m_ecoModeRadio->setToolTip(tr("Lower fee for slower confirmation (cheaper)"));
    
    // Fee information labels
    QLabel* standardFeeLabel = new QLabel(tr("Estimated: %1 | Confirmation: ~%2 blocks"), this);
    QLabel* highPriorityLabel = new QLabel(tr("Estimated: %1 | Confirmation: ~%2 blocks"), this);
    QLabel* ecoModeLabel = new QLabel(tr("Estimated: %1 | Confirmation: ~%2 blocks"), this);
    
    // Add to layout
    basicLayout->addWidget(m_standardFeeRadio);
    basicLayout->addWidget(standardFeeLabel);
    basicLayout->addSpacing(10);
    
    basicLayout->addWidget(m_highPriorityRadio);
    basicLayout->addWidget(highPriorityLabel);
    basicLayout->addSpacing(10);
    
    basicLayout->addWidget(m_ecoModeRadio);
    basicLayout->addWidget(ecoModeLabel);
    basicLayout->addStretch();
    
    // Store labels for later updates
    m_standardFeeRadio->setProperty("infoLabel", QVariant::fromValue(standardFeeLabel));
    m_highPriorityRadio->setProperty("infoLabel", QVariant::fromValue(highPriorityLabel));
    m_ecoModeRadio->setProperty("infoLabel", QVariant::fromValue(ecoModeLabel));
}

void CustomFeeSelectionDialog::setupAdvancedMode()
{
    QVBoxLayout* advancedLayout = new QVBoxLayout(m_advancedModeGroup);
    
    // Custom fee radio button
    m_customFeeRadio = new QRadioButton(tr("Custom Fee"), this);
    m_customFeeRadio->setToolTip(tr("Set your own fee per kilobyte"));
    advancedLayout->addWidget(m_customFeeRadio);
    
    // Custom fee input
    QHBoxLayout* feeInputLayout = new QHBoxLayout();
    QLabel* feeLabel = new QLabel(tr("Fee per kilobyte:"), this);
    m_customFeeSpinBox = new QSpinBox(this);
    m_customFeeSpinBox->setRange(1, 1000000); // 1 satoshi to 1 SHAH per KB
    m_customFeeSpinBox->setSuffix(tr(" satoshis/kB"));
    m_customFeeSpinBox->setToolTip(tr("Enter the fee you want to pay per kilobyte of transaction data"));
    
    feeInputLayout->addWidget(feeLabel);
    feeInputLayout->addWidget(m_customFeeSpinBox);
    feeInputLayout->addStretch();
    advancedLayout->addLayout(feeInputLayout);
    
    // Confirmation target selector
    QHBoxLayout* targetLayout = new QHBoxLayout();
    QLabel* targetLabel = new QLabel(tr("Confirmation target:"), this);
    m_confirmationTargetCombo = new QComboBox(this);
    m_confirmationTargetCombo->addItem(tr("1 block"), 1);
    m_confirmationTargetCombo->addItem(tr("2 blocks"), 2);
    m_confirmationTargetCombo->addItem(tr("3 blocks"), 3);
    m_confirmationTargetCombo->addItem(tr("6 blocks"), 6);
    m_confirmationTargetCombo->addItem(tr("10 blocks"), 10);
    m_confirmationTargetCombo->addItem(tr("15 blocks"), 15);
    m_confirmationTargetCombo->addItem(tr("25 blocks"), 25);
    m_confirmationTargetCombo->addItem(tr("50 blocks"), 50);
    m_confirmationTargetCombo->addItem(tr("100 blocks"), 100);
    m_confirmationTargetCombo->setCurrentText(tr("25 blocks"));
    
    targetLayout->addWidget(targetLabel);
    targetLayout->addWidget(m_confirmationTargetCombo);
    targetLayout->addStretch();
    advancedLayout->addLayout(targetLayout);
    
    // Information labels
    m_estimatedConfirmationLabel = new QLabel(tr("Estimated confirmation time: ~%1"), this);
    m_transactionSizeLabel = new QLabel(tr("Estimated transaction size: %1 bytes"), this);
    m_totalFeeLabel = new QLabel(tr("Total fee: %1"), this);
    m_feeWarningLabel = new QLabel(this);
    m_feeWarningLabel->setStyleSheet("color: red;");
    
    advancedLayout->addWidget(m_estimatedConfirmationLabel);
    advancedLayout->addWidget(m_transactionSizeLabel);
    advancedLayout->addWidget(m_totalFeeLabel);
    advancedLayout->addWidget(m_feeWarningLabel);
    advancedLayout->addStretch();
}

void CustomFeeSelectionDialog::onModeChanged()
{
    if (m_standardFeeRadio->isChecked()) {
        m_currentMode = Standard;
    } else if (m_highPriorityRadio->isChecked()) {
        m_currentMode = HighPriority;
    } else if (m_ecoModeRadio->isChecked()) {
        m_currentMode = EcoMode;
    } else if (m_customFeeRadio->isChecked()) {
        m_currentMode = Custom;
    }
    
    updateFeeDisplay();
    updateConfirmationTime();
}

void CustomFeeSelectionDialog::onCustomFeeChanged()
{
    m_currentCustomFee = m_customFeeSpinBox->value();
    validateCustomFee();
    updateFeeDisplay();
    updateConfirmationTime();
}

void CustomFeeSelectionDialog::onConfirmationTargetChanged()
{
    m_currentConfirmationTarget = m_confirmationTargetCombo->currentData().toInt();
    updateFeeEstimates();
    updateConfirmationTime();
}

void CustomFeeSelectionDialog::onAdvancedModeToggled(bool enabled)
{
    m_advancedModeGroup->setVisible(enabled);
    if (enabled && !m_customFeeRadio->isChecked()) {
        m_customFeeRadio->setChecked(true);
        m_currentMode = Custom;
    } else if (!enabled && m_customFeeRadio->isChecked()) {
        m_standardFeeRadio->setChecked(true);
        m_currentMode = Standard;
    }
    
    updateFeeDisplay();
}

void CustomFeeSelectionDialog::updateFeeEstimates()
{
    if (!model) return;
    
    // Get fee estimates from the wallet
    int returned_target;
    FeeReason reason;
    
    // Standard fee (25 blocks)
    CFeeRate standardRate = CFeeRate(model->wallet().getMinimumFee(1000, *m_coin_control, &returned_target, &reason));
    m_standardFeeRate = standardRate.GetFeePerK();
    
    // High priority fee (6 blocks)
    m_coin_control->m_confirm_target = 6;
    CFeeRate highPriorityRate = CFeeRate(model->wallet().getMinimumFee(1000, *m_coin_control, &returned_target, &reason));
    m_highPriorityFeeRate = highPriorityRate.GetFeePerK();
    
    // Eco mode fee (100 blocks)
    m_coin_control->m_confirm_target = 100;
    CFeeRate ecoRate = CFeeRate(model->wallet().getMinimumFee(1000, *m_coin_control, &returned_target, &reason));
    m_ecoModeFeeRate = ecoRate.GetFeePerK();
    
    // Reset to current confirmation target
    m_coin_control->m_confirm_target = m_currentConfirmationTarget;
    
    // Get minimum relay fee
    m_minRelayFee = ::minRelayTxFee.GetFeePerK();
    
    updateFeeDisplay();
    updateTransactionSize();
}

void CustomFeeSelectionDialog::updateFeeDisplay()
{
    if (!model || !model->getOptionsModel()) return;
    
    // Update basic mode labels
    QLabel* standardLabel = m_standardFeeRadio->property("infoLabel").value<QLabel*>();
    QLabel* highPriorityLabel = m_highPriorityRadio->property("infoLabel").value<QLabel*>();
    QLabel* ecoModeLabel = m_ecoModeRadio->property("infoLabel").value<QLabel*>();
    
    if (standardLabel) {
        CAmount fee = getEstimatedFee(Standard);
        QString feeStr = ShahcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), fee);
        standardLabel->setText(tr("Estimated: %1 | Confirmation: ~25 blocks").arg(feeStr));
    }
    
    if (highPriorityLabel) {
        CAmount fee = getEstimatedFee(HighPriority);
        QString feeStr = ShahcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), fee);
        highPriorityLabel->setText(tr("Estimated: %1 | Confirmation: ~6 blocks").arg(feeStr));
    }
    
    if (ecoModeLabel) {
        CAmount fee = getEstimatedFee(EcoMode);
        QString feeStr = ShahcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), fee);
        ecoModeLabel->setText(tr("Estimated: %1 | Confirmation: ~100 blocks").arg(feeStr));
    }
    
    // Update advanced mode labels
    updateTransactionSize();
}

void CustomFeeSelectionDialog::updateConfirmationTime()
{
    if (m_currentMode == Custom) {
        int blocks = getEstimatedConfirmationTime(Custom);
        QString timeStr = getEstimatedConfirmationTimeString(blocks);
        m_estimatedConfirmationLabel->setText(tr("Estimated confirmation time: %1").arg(timeStr));
    }
}

void CustomFeeSelectionDialog::updateTransactionSize()
{
    if (!model || !model->getOptionsModel()) return;
    
    m_transactionSizeLabel->setText(tr("Estimated transaction size: %1 bytes").arg(m_estimatedTransactionSize));
    
    CAmount totalFee = 0;
    if (m_currentMode == Custom) {
        totalFee = (m_currentCustomFee * m_estimatedTransactionSize) / 1000;
    } else {
        totalFee = getEstimatedFee(m_currentMode);
    }
    
    QString feeStr = ShahcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), totalFee);
    m_totalFeeLabel->setText(tr("Total fee: %1").arg(feeStr));
}

void CustomFeeSelectionDialog::validateCustomFee()
{
    if (m_currentCustomFee < m_minRelayFee) {
        m_feeWarningLabel->setText(tr("⚠️ Warning: Fee is below minimum relay fee (%1 satoshis/kB)").arg(m_minRelayFee));
        m_feeWarningLabel->setVisible(true);
    } else {
        m_feeWarningLabel->setVisible(false);
    }
}

CAmount CustomFeeSelectionDialog::getEstimatedFee(FeeMode mode) const
{
    CAmount feeRate = 0;
    switch (mode) {
        case Standard:
            feeRate = m_standardFeeRate;
            break;
        case HighPriority:
            feeRate = m_highPriorityFeeRate;
            break;
        case EcoMode:
            feeRate = m_ecoModeFeeRate;
            break;
        case Custom:
            feeRate = m_currentCustomFee;
            break;
    }
    
    return (feeRate * m_estimatedTransactionSize) / 1000;
}

int CustomFeeSelectionDialog::getEstimatedConfirmationTime(FeeMode mode) const
{
    switch (mode) {
        case Standard:
            return 25;
        case HighPriority:
            return 6;
        case EcoMode:
            return 100;
        case Custom:
            return m_currentConfirmationTarget;
    }
    return 25;
}

QString CustomFeeSelectionDialog::getEstimatedConfirmationTimeString(int blocks) const
{
    if (blocks == 1) {
        return tr("~%1 block").arg(blocks);
    } else if (blocks < 60) {
        return tr("~%1 blocks").arg(blocks);
    } else {
        int hours = blocks / 6; // Assuming 10 minutes per block
        return tr("~%1 hours").arg(hours);
    }
}

void CustomFeeSelectionDialog::onResetToDefaults()
{
    m_standardFeeRadio->setChecked(true);
    m_currentMode = Standard;
    m_advancedModeCheckBox->setChecked(false);
    m_customFeeSpinBox->setValue(m_standardFeeRate);
    m_confirmationTargetCombo->setCurrentText(tr("25 blocks"));
    
    updateFeeDisplay();
    updateConfirmationTime();
}

// Getter methods
CustomFeeSelectionDialog::FeeMode CustomFeeSelectionDialog::getSelectedMode() const
{
    return m_currentMode;
}

CAmount CustomFeeSelectionDialog::getCustomFeePerKB() const
{
    return m_currentCustomFee;
}

int CustomFeeSelectionDialog::getConfirmationTarget() const
{
    return m_currentConfirmationTarget;
}

// Setter methods
void CustomFeeSelectionDialog::setCurrentMode(FeeMode mode)
{
    m_currentMode = mode;
    switch (mode) {
        case Standard:
            m_standardFeeRadio->setChecked(true);
            break;
        case HighPriority:
            m_highPriorityRadio->setChecked(true);
            break;
        case EcoMode:
            m_ecoModeRadio->setChecked(true);
            break;
        case Custom:
            m_customFeeRadio->setChecked(true);
            m_advancedModeCheckBox->setChecked(true);
            break;
    }
}

void CustomFeeSelectionDialog::setCurrentCustomFee(CAmount feePerKB)
{
    m_currentCustomFee = feePerKB;
    m_customFeeSpinBox->setValue(feePerKB);
}

void CustomFeeSelectionDialog::setCurrentConfirmationTarget(int target)
{
    m_currentConfirmationTarget = target;
    for (int i = 0; i < m_confirmationTargetCombo->count(); ++i) {
        if (m_confirmationTargetCombo->itemData(i).toInt() == target) {
            m_confirmationTargetCombo->setCurrentIndex(i);
            break;
        }
    }
}

void CustomFeeSelectionDialog::accept()
{
    // Validate custom fee if selected
    if (m_currentMode == Custom && m_currentCustomFee < m_minRelayFee) {
        QMessageBox::warning(this, tr("Low Fee Warning"),
            tr("The selected fee is below the minimum relay fee. Your transaction may not be accepted by the network."),
            QMessageBox::Ok);
    }
    
    QDialog::accept();
}

void CustomFeeSelectionDialog::reject()
{
    QDialog::reject();
}
