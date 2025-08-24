#ifndef SHAHCOIN_QT_SEEDVAULT_H
#define SHAHCOIN_QT_SEEDVAULT_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QTimer>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QCheckBox;
class QGroupBox;
class QMessageBox;
class WalletModel;

/**
 * @brief Secure Seed Vault Dialog for Shahcoin Qt Wallet
 * 
 * Provides secure access to BIP39 seed phrases with:
 * - Password confirmation required
 * - Secure display of seed words
 * - Copy to clipboard functionality
 * - Export to file option
 * - Warning for wallets without seeds
 */
class SeedVault : public QDialog
{
    Q_OBJECT

public:
    explicit SeedVault(WalletModel* walletModel, QWidget* parent = nullptr);
    ~SeedVault();

    // Seed management
    bool hasSeedPhrase() const;
    QStringList getSeedPhrase() const;
    bool validateSeedPhrase(const QStringList& words) const;
    
    // Security
    bool isPasswordValid(const QString& password) const;
    void setPassword(const QString& password);
    
    // Export functionality
    bool exportSeedToFile(const QString& filePath) const;
    QString generateSeedBackupText() const;

public Q_SLOTS:
    void onShowSeedClicked();
    void onCopySeedClicked();
    void onExportSeedClicked();
    void onPasswordChanged();
    void onShowPasswordToggled(bool checked);
    void onAcceptClicked();
    void onCancelClicked();

Q_SIGNALS:
    void seedExported(const QString& filePath);
    void seedCopied();
    void accessDenied();

private:
    void setupUI();
    void setupSecurityGroup();
    void setupSeedDisplayGroup();
    void setupButtonGroup();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    void showSeedPhrase();
    void hideSeedPhrase();
    void validateInputs();
    void showWarning(const QString& title, const QString& message);
    void showSuccess(const QString& title, const QString& message);
    void showError(const QString& title, const QString& message);
    QString getWalletPassword() const;
    bool checkWalletPassword(const QString& password) const;
    QStringList generateMockSeedPhrase() const; // For demo purposes
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Security Group
    QGroupBox* m_securityGroup;
    QLabel* m_passwordLabel;
    QLineEdit* m_passwordEdit;
    QCheckBox* m_showPasswordCheckbox;
    QPushButton* m_showSeedButton;
    
    // Seed Display Group
    QGroupBox* m_seedDisplayGroup;
    QLabel* m_seedInstructionsLabel;
    QTextEdit* m_seedDisplayEdit;
    QHBoxLayout* m_seedButtonLayout;
    QPushButton* m_copySeedButton;
    QPushButton* m_exportSeedButton;
    
    // Button Group
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_acceptButton;
    QPushButton* m_cancelButton;
    
    // Data
    WalletModel* m_walletModel;
    QStringList m_seedPhrase;
    QString m_walletPassword;
    bool m_seedVisible;
    bool m_passwordValid;
    
    // Settings
    QSettings* m_settings;
    
    // Security
    static constexpr int MAX_PASSWORD_ATTEMPTS = 3;
    int m_passwordAttempts;
    QTimer* m_lockoutTimer;
    
    // Constants
    static constexpr const char* SETTINGS_GROUP = "SeedVault";
    static constexpr const char* PASSWORD_ATTEMPTS_KEY = "PasswordAttempts";
    static constexpr const char* LAST_ACCESS_KEY = "LastAccess";
    static constexpr int LOCKOUT_DURATION_MS = 300000; // 5 minutes
};

#endif // SHAHCOIN_QT_SEEDVAULT_H
