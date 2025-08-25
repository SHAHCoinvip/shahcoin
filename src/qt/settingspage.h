// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_SETTINGSPAGE_H
#define SHAHCOIN_QT_SETTINGSPAGE_H

#include <QWidget>
#include <QSettings>

class ClientModel;
class WalletModel;
class QComboBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QLabel;

namespace Ui {
    class SettingsPage;
}

/**
 * @brief The SettingsPage class provides a comprehensive settings interface for SHAHCOIN Core
 * 
 * This page allows users to configure:
 * - Mining preferences (algorithm selection, auto-switching)
 * - Transaction fees (custom fees, dynamic fees)
 * - Display preferences (language, theme)
 * - Performance & security settings
 * - About information
 */
class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);
    ~SettingsPage();

    void setClientModel(ClientModel* clientModel);
    void setWalletModel(WalletModel* walletModel);

Q_SIGNALS:
    void message(const QString& title, const QString& message, unsigned int style);

private Q_SLOTS:
    void onSaveSettings();
    void onResetToDefaults();
    void onWebsiteClicked();
    void onMiningAlgorithmChanged(int index);
    void onLanguageChanged(int index);
    void onThemeChanged(int index);
    void onCustomFeeChanged(double value);
    void onDynamicFeesToggled(bool checked);
    void onAutoSwitchToggled(bool checked);
    void onStartMinimizedToggled(bool checked);
    void onRPCLocalhostToggled(bool checked);
    void onWarnLargeAmountToggled(bool checked);

private:
    void setupUI();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    void resetToDefaults();
    void showSettingsSuccess(const QString& message);
    void showSettingsError(const QString& message);
    void updateVersionInfo();

    Ui::SettingsPage* ui;
    ClientModel* clientModel;
    WalletModel* walletModel;
    QSettings* settings;

    // UI Elements
    QComboBox* comboBox_MiningAlgorithm;
    QCheckBox* checkBox_AutoSwitch;
    QDoubleSpinBox* doubleSpinBox_CustomFee;
    QCheckBox* checkBox_DynamicFees;
    QComboBox* comboBox_Language;
    QComboBox* comboBox_Theme;
    QCheckBox* checkBox_StartMinimized;
    QCheckBox* checkBox_RPCLocalhost;
    QCheckBox* checkBox_WarnLargeAmount;
    QPushButton* pushButton_Save;
    QPushButton* pushButton_Reset;
    QPushButton* pushButton_Website;
    QLabel* label_VersionValue;
    QLabel* label_Founder;
    
    // Enhanced theme management
    class ThemeManager* m_themeManager;
};

#endif // SHAHCOIN_QT_SETTINGSPAGE_H
