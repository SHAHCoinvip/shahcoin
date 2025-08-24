#ifndef SHAHCOIN_QT_WIZARDAI_H
#define SHAHCOIN_QT_WIZARDAI_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QProgressBar>
#include <QTimer>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QSettings>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class WalletModel;
class PlatformStyle;

/**
 * AI Wallet Wizard Dialog
 * 
 * Provides intelligent onboarding and guidance for Shahcoin Wallet users:
 * - AI-powered analysis of wallet state
 * - Progressive step-by-step walkthrough
 * - Personalized recommendations
 * - Security tips and best practices
 * - Test transaction guidance
 * - Staking setup assistance
 */
class WizardAI : public QDialog
{
    Q_OBJECT

public:
    enum WizardStep {
        Welcome = 0,
        WalletAnalysis,
        SecuritySetup,
        TestTransaction,
        StakingSetup,
        BackupReminder,
        Completion
    };

    explicit WizardAI(WalletModel* walletModel, QWidget* parent = nullptr);
    ~WizardAI();

    // Wizard control
    void startWizard();
    void showStep(WizardStep step);
    void nextStep();
    void previousStep();
    void completeWizard();
    
    // AI Analysis
    QJsonObject analyzeWalletState() const;
    QJsonObject generateRecommendations() const;
    QString getPersonalizedMessage() const;
    bool shouldShowStaking() const;
    bool shouldShowBackup() const;
    bool shouldShowTestTransaction() const;
    
    // Recommendations
    QStringList getSecurityRecommendations() const;
    QStringList getStakingRecommendations() const;
    QStringList getBackupRecommendations() const;
    QStringList getFeeRecommendations() const;

public Q_SLOTS:
    void onNextClicked();
    void onPreviousClicked();
    void onSkipClicked();
    void onCompleteClicked();
    void onHelpClicked();
    void onRetryAnalysis();
    void onShowDetails();
    void onHideDetails();

private Q_SLOTS:
    void onStepChanged(int step);
    void onAnalysisComplete();
    void onRecommendationGenerated();
    void onAnimationFinished();

private:
    void setupUI();
    void setupWelcomeStep();
    void setupAnalysisStep();
    void setupSecurityStep();
    void setupTestTransactionStep();
    void setupStakingStep();
    void setupBackupStep();
    void setupCompletionStep();
    void setupNavigation();
    void connectSignals();
    void applyTheme();
    
    // Step management
    void showCurrentStep();
    void hideAllSteps();
    void updateNavigationButtons();
    void animateStepTransition(WizardStep fromStep, WizardStep toStep);
    
    // AI Analysis methods
    void performWalletAnalysis();
    void analyzeBalance();
    void analyzeSecurity();
    void analyzeStaking();
    void analyzeBackup();
    void analyzeFees();
    void analyzeTransactions();
    
    // UI helpers
    void showRecommendation(const QString& title, const QString& description, const QString& action);
    void showTip(const QString& tip, const QString& icon = "💡");
    void showWarning(const QString& warning, const QString& icon = "⚠️");
    void showSuccess(const QString& success, const QString& icon = "✅");
    void showProgress(const QString& message);
    void hideProgress();
    
    // Data collection
    qint64 getWalletBalance() const;
    int getTransactionCount() const;
    bool hasStakingSetup() const;
    bool hasBackup() const;
    bool hasTestTransaction() const;
    QString getFeeSetting() const;
    QStringList getAddressList() const;
    
    // Settings
    void saveWizardProgress();
    void loadWizardProgress();
    void markStepCompleted(WizardStep step);
    bool isStepCompleted(WizardStep step) const;
    void setWizardCompleted();
    bool isWizardCompleted() const;

    // UI Components
    QVBoxLayout* m_mainLayout;
    QStackedWidget* m_stepStack;
    
    // Welcome Step
    QWidget* m_welcomeStep;
    QVBoxLayout* m_welcomeLayout;
    QLabel* m_welcomeTitle;
    QLabel* m_welcomeSubtitle;
    QLabel* m_welcomeIcon;
    QTextEdit* m_welcomeDescription;
    QCheckBox* m_autoAnalysisCheck;
    QPushButton* m_welcomeNextButton;
    
    // Analysis Step
    QWidget* m_analysisStep;
    QVBoxLayout* m_analysisLayout;
    QLabel* m_analysisTitle;
    QProgressBar* m_analysisProgress;
    QLabel* m_analysisStatus;
    QTextEdit* m_analysisResults;
    QPushButton* m_analysisNextButton;
    QPushButton* m_analysisRetryButton;
    
    // Security Step
    QWidget* m_securityStep;
    QVBoxLayout* m_securityLayout;
    QLabel* m_securityTitle;
    QGroupBox* m_securityRecommendations;
    QVBoxLayout* m_securityRecLayout;
    QPushButton* m_securityNextButton;
    QPushButton* m_securitySkipButton;
    
    // Test Transaction Step
    QWidget* m_testStep;
    QVBoxLayout* m_testLayout;
    QLabel* m_testTitle;
    QTextEdit* m_testDescription;
    QPushButton* m_testSendButton;
    QPushButton* m_testSkipButton;
    QPushButton* m_testNextButton;
    
    // Staking Step
    QWidget* m_stakingStep;
    QVBoxLayout* m_stakingLayout;
    QLabel* m_stakingTitle;
    QGroupBox* m_stakingRecommendations;
    QVBoxLayout* m_stakingRecLayout;
    QPushButton* m_stakingSetupButton;
    QPushButton* m_stakingSkipButton;
    QPushButton* m_stakingNextButton;
    
    // Backup Step
    QWidget* m_backupStep;
    QVBoxLayout* m_backupLayout;
    QLabel* m_backupTitle;
    QTextEdit* m_backupDescription;
    QPushButton* m_backupNowButton;
    QPushButton* m_backupLaterButton;
    QPushButton* m_backupNextButton;
    
    // Completion Step
    QWidget* m_completionStep;
    QVBoxLayout* m_completionLayout;
    QLabel* m_completionTitle;
    QLabel* m_completionIcon;
    QTextEdit* m_completionSummary;
    QPushButton* m_completionFinishButton;
    QPushButton* m_completionRestartButton;
    
    // Navigation
    QHBoxLayout* m_navigationLayout;
    QPushButton* m_previousButton;
    QPushButton* m_nextButton;
    QPushButton* m_skipButton;
    QPushButton* m_helpButton;
    QLabel* m_stepIndicator;
    
    // Help Dialog
    QDialog* m_helpDialog;
    QVBoxLayout* m_helpLayout;
    QLabel* m_helpTitle;
    QTextEdit* m_helpContent;
    QPushButton* m_helpCloseButton;
    
    // Data
    WalletModel* m_walletModel;
    const PlatformStyle* m_platformStyle;
    QTimer* m_analysisTimer;
    
    // Wizard state
    WizardStep m_currentStep;
    QList<WizardStep> m_completedSteps;
    QJsonObject m_analysisResults;
    QJsonObject m_recommendations;
    bool m_wizardCompleted;
    bool m_autoAnalysis;
    
    // UI state
    bool m_analysisInProgress;
    bool m_showDetails;
    QPropertyAnimation* m_stepAnimation;
    QGraphicsOpacityEffect* m_opacityEffect;
    QGraphicsDropShadowEffect* m_shadowEffect;
    
    // Settings
    QSettings* m_settings;
    QString m_lastAnalysisTime;
    int m_wizardRunCount;
    
    // Constants
    static constexpr int ANALYSIS_TIMEOUT_MS = 3000;
    static constexpr int ANIMATION_DURATION_MS = 300;
    static constexpr int MIN_BALANCE_FOR_STAKING = 1000000000; // 10 SHAH in satoshis
    static constexpr int MIN_BALANCE_FOR_TEST = 100000000; // 1 SHAH in satoshis
    static constexpr const char* SETTINGS_GROUP = "WizardAI";
    static constexpr const char* SETTINGS_COMPLETED = "Completed";
    static constexpr const char* SETTINGS_RUN_COUNT = "RunCount";
    static constexpr const char* SETTINGS_LAST_ANALYSIS = "LastAnalysis";
};

#endif // SHAHCOIN_QT_WIZARDAI_H
