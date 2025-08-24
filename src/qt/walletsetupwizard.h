#ifndef SHAHCOIN_QT_WALLETSETUPWIZARD_H
#define SHAHCOIN_QT_WALLETSETUPWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QComboBox>
#include <QSlider>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

class QWizardPage;
class QLabel;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QProgressBar;
class QTextEdit;
class QComboBox;
class QSlider;
class QGroupBox;

/**
 * @brief Enhanced Wallet Setup Wizard for SHAHCOIN Core
 * 
 * Provides a comprehensive wallet creation flow with:
 * - Welcome and introduction
 * - Security settings and password strength meter
 * - Seed phrase generation and backup
 * - Wallet configuration options
 * - Final setup and verification
 * - Onboarding tooltips and help
 */
class WalletSetupWizard : public QWizard
{
    Q_OBJECT

public:
    enum PageId {
        WelcomePage = 0,
        SecurityPage,
        SeedPhrasePage,
        BackupPage,
        ConfigurationPage,
        FinalPage
    };

    explicit WalletSetupWizard(QWidget* parent = nullptr);
    ~WalletSetupWizard();

    // Wizard configuration
    void setWalletName(const QString& name);
    QString getWalletName() const;
    void setEncryptionEnabled(bool enabled);
    bool isEncryptionEnabled() const;
    QString getPassword() const;
    QString getSeedPhrase() const;
    
    // Security settings
    void setAutoLockTimeout(int minutes);
    int getAutoLockTimeout() const;
    void setBackupReminderEnabled(bool enabled);
    bool isBackupReminderEnabled() const;

public Q_SLOTS:
    void onPasswordChanged(const QString& password);
    void onSeedPhraseGenerated();
    void onBackupConfirmed();
    void onConfigurationChanged();
    void onHelpRequested();

Q_SIGNALS:
    void walletCreated(const QString& name, const QString& password, const QString& seedPhrase);
    void setupCancelled();
    void helpRequested(const QString& topic);

private:
    void setupPages();
    void setupConnections();
    void applyTheme();
    void validateCurrentPage();
    void updatePasswordStrength(const QString& password);
    void generateSeedPhrase();
    void showBackupInstructions();
    
    // Page creation methods
    QWizardPage* createWelcomePage();
    QWizardPage* createSecurityPage();
    QWizardPage* createSeedPhrasePage();
    QWizardPage* createBackupPage();
    QWizardPage* createConfigurationPage();
    QWizardPage* createFinalPage();
    
    // Data
    QString m_walletName;
    QString m_password;
    QString m_seedPhrase;
    bool m_encryptionEnabled;
    int m_autoLockTimeout;
    bool m_backupReminderEnabled;
    bool m_backupConfirmed;
    
    // UI Elements (shared across pages)
    QLineEdit* m_walletNameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_confirmPasswordEdit;
    QProgressBar* m_passwordStrengthBar;
    QLabel* m_passwordStrengthLabel;
    QCheckBox* m_encryptionCheckBox;
    QTextEdit* m_seedPhraseDisplay;
    QCheckBox* m_backupConfirmedCheckBox;
    QSlider* m_autoLockSlider;
    QLabel* m_autoLockLabel;
    QCheckBox* m_backupReminderCheckBox;
    QComboBox* m_themeComboBox;
    QComboBox* m_languageComboBox;
};

/**
 * @brief Welcome Page for Wallet Setup
 */
class WelcomePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget* parent = nullptr);

private:
    void setupUI();
    bool validatePage() override;
    
    QLabel* m_welcomeLabel;
    QLabel* m_descriptionLabel;
    QLineEdit* m_walletNameEdit;
    QLabel* m_walletNameLabel;
};

/**
 * @brief Security Page for Wallet Setup
 */
class SecurityPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SecurityPage(QWidget* parent = nullptr);

private Q_SLOTS:
    void onPasswordChanged(const QString& password);
    void onConfirmPasswordChanged(const QString& password);
    void onEncryptionToggled(bool checked);

private:
    void setupUI();
    void updatePasswordStrength(const QString& password);
    bool validatePage() override;
    
    QLineEdit* m_passwordEdit;
    QLineEdit* m_confirmPasswordEdit;
    QProgressBar* m_passwordStrengthBar;
    QLabel* m_passwordStrengthLabel;
    QCheckBox* m_encryptionCheckBox;
    QLabel* m_passwordRequirementsLabel;
    QLabel* m_confirmPasswordLabel;
};

/**
 * @brief Seed Phrase Page for Wallet Setup
 */
class SeedPhrasePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SeedPhrasePage(QWidget* parent = nullptr);

private Q_SLOTS:
    void onGenerateClicked();
    void onCopyClicked();
    void onShowHideToggled();

private:
    void setupUI();
    void generateSeedPhrase();
    bool validatePage() override;
    
    QTextEdit* m_seedPhraseDisplay;
    QPushButton* m_generateButton;
    QPushButton* m_copyButton;
    QCheckBox* m_showHideCheckBox;
    QLabel* m_warningLabel;
    QLabel* m_instructionLabel;
};

/**
 * @brief Backup Page for Wallet Setup
 */
class BackupPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit BackupPage(QWidget* parent = nullptr);

private Q_SLOTS:
    void onBackupInstructionsClicked();
    void onBackupConfirmedToggled(bool checked);

private:
    void setupUI();
    void showBackupInstructions();
    bool validatePage() override;
    
    QLabel* m_instructionLabel;
    QCheckBox* m_backupConfirmedCheckBox;
    QPushButton* m_backupInstructionsButton;
    QLabel* m_warningLabel;
    QGroupBox* m_backupOptionsGroup;
    QCheckBox* m_exportFileCheckBox;
    QCheckBox* m_printCheckBox;
    QCheckBox* m_cloudBackupCheckBox;
};

/**
 * @brief Configuration Page for Wallet Setup
 */
class ConfigurationPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ConfigurationPage(QWidget* parent = nullptr);

private Q_SLOTS:
    void onAutoLockChanged(int value);
    void onThemeChanged(int index);
    void onLanguageChanged(int index);

private:
    void setupUI();
    bool validatePage() override;
    
    QSlider* m_autoLockSlider;
    QLabel* m_autoLockLabel;
    QCheckBox* m_backupReminderCheckBox;
    QComboBox* m_themeComboBox;
    QComboBox* m_languageComboBox;
    QGroupBox* m_securityGroup;
    QGroupBox* m_appearanceGroup;
    QGroupBox* m_notificationsGroup;
};

/**
 * @brief Final Page for Wallet Setup
 */
class FinalPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit FinalPage(QWidget* parent = nullptr);

private:
    void setupUI();
    void initializePage() override;
    bool validatePage() override;
    
    QLabel* m_successLabel;
    QLabel* m_summaryLabel;
    QLabel* m_nextStepsLabel;
    QPushButton* m_openWalletButton;
    QPushButton* m_viewTutorialButton;
};

#endif // SHAHCOIN_QT_WALLETSETUPWIZARD_H
