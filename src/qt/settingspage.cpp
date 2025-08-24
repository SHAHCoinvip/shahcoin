// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "settingspage.h"
#include "ui_settingspage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "guiconstants.h"
#include "optionsmodel.h"
#include "qt/guiutil.h"
#include "thememanager.h"

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QTimer>
#include <QStyleFactory>

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettingsPage)
    , clientModel(nullptr)
    , walletModel(nullptr)
    , settings(new QSettings("Shahcoin", "Shahcoin-Qt"))
    , m_themeManager(new ThemeManager(this))
{
    setupUI();
    connectSignals();
    loadSettings();
    updateVersionInfo();
}

SettingsPage::~SettingsPage()
{
    delete ui;
}

void SettingsPage::setupUI()
{
    ui->setupUi(this);
    
    // Get references to UI elements
    comboBox_MiningAlgorithm = ui->comboBox_MiningAlgorithm;
    checkBox_AutoSwitch = ui->checkBox_AutoSwitch;
    doubleSpinBox_CustomFee = ui->doubleSpinBox_CustomFee;
    checkBox_DynamicFees = ui->checkBox_DynamicFees;
    comboBox_Language = ui->comboBox_Language;
    comboBox_Theme = ui->comboBox_Theme;
    checkBox_StartMinimized = ui->checkBox_StartMinimized;
    checkBox_RPCLocalhost = ui->checkBox_RPCLocalhost;
    checkBox_WarnLargeAmount = ui->checkBox_WarnLargeAmount;
    pushButton_Save = ui->pushButton_Save;
    pushButton_Reset = ui->pushButton_Reset;
    pushButton_Website = ui->pushButton_Website;
    label_VersionValue = ui->label_VersionValue;
    label_Founder = ui->label_Founder;
}

void SettingsPage::connectSignals()
{
    // Connect button signals
    connect(pushButton_Save, &QPushButton::clicked, this, &SettingsPage::onSaveSettings);
    connect(pushButton_Reset, &QPushButton::clicked, this, &SettingsPage::onResetToDefaults);
    connect(pushButton_Website, &QPushButton::clicked, this, &SettingsPage::onWebsiteClicked);
    
    // Connect mining preferences
    connect(comboBox_MiningAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPage::onMiningAlgorithmChanged);
    connect(checkBox_AutoSwitch, &QCheckBox::toggled, this, &SettingsPage::onAutoSwitchToggled);
    
    // Connect transaction fees
    connect(doubleSpinBox_CustomFee, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SettingsPage::onCustomFeeChanged);
    connect(checkBox_DynamicFees, &QCheckBox::toggled, this, &SettingsPage::onDynamicFeesToggled);
    
    // Connect display preferences
    connect(comboBox_Language, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPage::onLanguageChanged);
    connect(comboBox_Theme, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPage::onThemeChanged);
    
    // Connect performance & security
    connect(checkBox_StartMinimized, &QCheckBox::toggled, this, &SettingsPage::onStartMinimizedToggled);
    connect(checkBox_RPCLocalhost, &QCheckBox::toggled, this, &SettingsPage::onRPCLocalhostToggled);
    connect(checkBox_WarnLargeAmount, &QCheckBox::toggled, this, &SettingsPage::onWarnLargeAmountToggled);
}

void SettingsPage::setClientModel(ClientModel* model)
{
    clientModel = model;
}

void SettingsPage::setWalletModel(WalletModel* model)
{
    walletModel = model;
}

void SettingsPage::loadSettings()
{
    // Load theme settings first to ensure proper theming
    m_themeManager->loadThemeFromSettings();
    // Load mining preferences
    QString miningAlgo = settings->value("Mining/Algorithm", "SHA256d").toString();
    int algoIndex = comboBox_MiningAlgorithm->findText(miningAlgo);
    if (algoIndex >= 0) {
        comboBox_MiningAlgorithm->setCurrentIndex(algoIndex);
    }
    
    bool autoSwitch = settings->value("Mining/AutoSwitch", false).toBool();
    checkBox_AutoSwitch->setChecked(autoSwitch);
    
    // Load transaction fees
    double customFee = settings->value("Fees/CustomFee", 0.001).toDouble();
    doubleSpinBox_CustomFee->setValue(customFee);
    
    bool dynamicFees = settings->value("Fees/DynamicFees", true).toBool();
    checkBox_DynamicFees->setChecked(dynamicFees);
    
    // Load display preferences
    QString language = settings->value("Display/Language", "English").toString();
    int langIndex = comboBox_Language->findText(language);
    if (langIndex >= 0) {
        comboBox_Language->setCurrentIndex(langIndex);
    }
    
    // Load theme using ThemeManager
    QStringList availableThemes = m_themeManager->getAvailableThemes();
    comboBox_Theme->clear();
    comboBox_Theme->addItems(availableThemes);
    
    QString currentTheme = m_themeManager->getCurrentThemeName();
    int themeIndex = comboBox_Theme->findText(currentTheme);
    if (themeIndex >= 0) {
        comboBox_Theme->setCurrentIndex(themeIndex);
    }
    
    // Apply current theme to ensure consistency
    m_themeManager->applyThemeToApplication();
    
    // Connect theme manager signals
    connect(m_themeManager, &ThemeManager::themeChanged, this, [this](ThemeManager::ThemeType theme) {
        // Update combo box to reflect the change
        QString themeName = m_themeManager->getCurrentThemeName();
        int index = comboBox_Theme->findText(themeName);
        if (index >= 0) {
            comboBox_Theme->setCurrentIndex(index);
        }
        
        // Apply theme to this widget
        m_themeManager->applyThemeToWidget(this);
    });
    
    // Load performance & security
    bool startMinimized = settings->value("Performance/StartMinimized", false).toBool();
    checkBox_StartMinimized->setChecked(startMinimized);
    
    bool rpcLocalhost = settings->value("Security/RPCLocalhost", true).toBool();
    checkBox_RPCLocalhost->setChecked(rpcLocalhost);
    
    bool warnLargeAmount = settings->value("Security/WarnLargeAmount", true).toBool();
    checkBox_WarnLargeAmount->setChecked(warnLargeAmount);
}

void SettingsPage::saveSettings()
{
    // Save mining preferences
    settings->setValue("Mining/Algorithm", comboBox_MiningAlgorithm->currentText());
    settings->setValue("Mining/AutoSwitch", checkBox_AutoSwitch->isChecked());
    
    // Save transaction fees
    settings->setValue("Fees/CustomFee", doubleSpinBox_CustomFee->value());
    settings->setValue("Fees/DynamicFees", checkBox_DynamicFees->isChecked());
    
    // Save display preferences
    settings->setValue("Display/Language", comboBox_Language->currentText());
    // Theme is saved by ThemeManager
    m_themeManager->saveThemeToSettings();
    
    // Apply current theme to ensure consistency
    m_themeManager->applyThemeToApplication();
    m_themeManager->applyThemeToWidget(this);
    
    // Save performance & security
    settings->setValue("Performance/StartMinimized", checkBox_StartMinimized->isChecked());
    settings->setValue("Security/RPCLocalhost", checkBox_RPCLocalhost->isChecked());
    settings->setValue("Security/WarnLargeAmount", checkBox_WarnLargeAmount->isChecked());
    
    settings->sync();
}

void SettingsPage::resetToDefaults()
{
    // Reset mining preferences
    comboBox_MiningAlgorithm->setCurrentText("SHA256d");
    checkBox_AutoSwitch->setChecked(false);
    
    // Reset transaction fees
    doubleSpinBox_CustomFee->setValue(0.001);
    checkBox_DynamicFees->setChecked(true);
    
    // Reset display preferences
    comboBox_Language->setCurrentText("English");
    comboBox_Theme->setCurrentText("Light Theme");
    m_themeManager->resetToDefaultTheme();
    
    // Apply the reset theme immediately
    m_themeManager->applyThemeToApplication();
    m_themeManager->applyThemeToWidget(this);
    
    // Reset performance & security
    checkBox_StartMinimized->setChecked(false);
    checkBox_RPCLocalhost->setChecked(true);
    checkBox_WarnLargeAmount->setChecked(true);
}

void SettingsPage::updateVersionInfo()
{
    // Set version information
    label_VersionValue->setText("SHAHCOIN Core v21.0.0");
    label_Founder->setText("Hamid Rad — Founder of Shahcoin");
}

void SettingsPage::showSettingsSuccess(const QString& message)
{
    QMessageBox::information(this, tr("Settings"), message);
}

void SettingsPage::showSettingsError(const QString& message)
{
    QMessageBox::warning(this, tr("Settings Error"), message);
}

// Slot implementations
void SettingsPage::onSaveSettings()
{
    saveSettings();
    showSettingsSuccess(tr("Settings saved successfully!"));
    
    // Apply theme changes immediately if needed
    if (comboBox_Theme->currentText() != "Light") {
        // Apply theme (this would need to be implemented based on your theming system)
        QString themeName = comboBox_Theme->currentText();
        if (themeName == "Dark") {
            // Apply dark theme
            qApp->setStyleSheet("QWidget { background-color: #2b2b2b; color: #ffffff; }");
        } else if (themeName == "System Default") {
            // Reset to system default
            qApp->setStyleSheet("");
        }
    }
}

void SettingsPage::onResetToDefaults()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        tr("Reset Settings"), 
        tr("Are you sure you want to reset all settings to their default values?"),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        resetToDefaults();
        saveSettings();
        showSettingsSuccess(tr("Settings reset to defaults!"));
    }
}

void SettingsPage::onWebsiteClicked()
{
    QDesktopServices::openUrl(QUrl("https://shah.vip"));
}

void SettingsPage::onMiningAlgorithmChanged(int index)
{
    QString algorithm = comboBox_MiningAlgorithm->itemText(index);
    settings->setValue("Mining/Algorithm", algorithm);
    
    // Notify the mining system about the algorithm change
    if (clientModel) {
        // This would typically call a method on the client model to update mining settings
        // clientModel->setMiningAlgorithm(algorithm);
    }
}

void SettingsPage::onLanguageChanged(int index)
{
    QString language = comboBox_Language->itemText(index);
    settings->setValue("Display/Language", language);
    
    // Show restart message for language changes
    if (index > 0) { // Not English (default)
        QMessageBox::information(this, tr("Language Change"), 
            tr("Language will be applied after restarting SHAHCOIN Core."));
    }
}

void SettingsPage::onThemeChanged(int index)
{
    QString theme = comboBox_Theme->itemText(index);
    m_themeManager->setTheme(theme);
    
    // Show a brief message about theme change
    QMessageBox::information(this, tr("Theme Changed"), 
        tr("Theme changed to: %1").arg(theme));
    
    // Save theme setting immediately
    m_themeManager->saveThemeToSettings();
    
    // Apply theme to this widget
    m_themeManager->applyThemeToWidget(this);
}

void SettingsPage::onCustomFeeChanged(double value)
{
    settings->setValue("Fees/CustomFee", value);
    
    // Update wallet model if available
    if (walletModel) {
        // This would typically call a method on the wallet model to update fee settings
        // walletModel->setCustomFee(value);
    }
}

void SettingsPage::onDynamicFeesToggled(bool checked)
{
    settings->setValue("Fees/DynamicFees", checked);
    
    // Enable/disable custom fee input based on dynamic fees setting
    doubleSpinBox_CustomFee->setEnabled(!checked);
    
    if (walletModel) {
        // This would typically call a method on the wallet model to update fee settings
        // walletModel->setDynamicFees(checked);
    }
}

void SettingsPage::onAutoSwitchToggled(bool checked)
{
    settings->setValue("Mining/AutoSwitch", checked);
    
    if (clientModel) {
        // This would typically call a method on the client model to update mining settings
        // clientModel->setAutoSwitchMining(checked);
    }
}

void SettingsPage::onStartMinimizedToggled(bool checked)
{
    settings->setValue("Performance/StartMinimized", checked);
}

void SettingsPage::onRPCLocalhostToggled(bool checked)
{
    settings->setValue("Security/RPCLocalhost", checked);
    
    if (checked) {
        QMessageBox::information(this, tr("Security"), 
            tr("RPC access is now restricted to localhost only for enhanced security."));
    }
}

void SettingsPage::onWarnLargeAmountToggled(bool checked)
{
    settings->setValue("Security/WarnLargeAmount", checked);
}
