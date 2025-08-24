#ifndef SHAHCOIN_QT_COLDSTAKINGSETUP_H
#define SHAHCOIN_QT_COLDSTAKINGSETUP_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QProgressBar>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDateTimeEdit>
#include <QCalendarWidget>
#include <QTextBrowser>
#include <QScrollArea>

class WalletModel;
class PlatformStyle;

/**
 * @brief Cold Wallet Staking Setup
 *
 * Provides secure cold wallet staking delegation:
 * - Delegate stake from offline cold wallet to hot staking node
 * - QR code export for cold wallet signing
 * - Secure transaction flow with offline signing
 * - Delegation management and monitoring
 * - Instructions and confirmation dialogs
 */
class ColdStakingSetup : public QWidget
{
    Q_OBJECT

public:
    enum SetupStep {
        Welcome = 0,
        ColdWalletSelection,
        HotNodeConfiguration,
        DelegationAmount,
        QRCodeGeneration,
        Confirmation,
        SetupComplete
    };

    enum DelegationStatus {
        NotDelegated = 0,
        Pending,
        Active,
        Expired,
        Revoked
    };

    struct DelegationInfo {
        QString coldWalletAddress;
        QString hotNodeAddress;
        double delegatedAmount;
        QDateTime delegationDate;
        QDateTime expiryDate;
        DelegationStatus status;
        double rewardsEarned;
        QString transactionId;
    };

    struct HotNodeInfo {
        QString address;
        QString name;
        QString description;
        double feePercentage;
        double minimumStake;
        double maximumStake;
        bool isOnline;
        double uptime;
        int totalDelegators;
    };

    explicit ColdStakingSetup(WalletModel* walletModel, QWidget* parent = nullptr);
    ~ColdStakingSetup();

    // Main setup methods
    void startSetup();
    void generateDelegationQR();
    void processSignedTransaction(const QString& signedTx);
    void monitorDelegationStatus();

    // Delegation management
    void createDelegation(const DelegationInfo& delegation);
    void revokeDelegation(const QString& delegationId);
    void claimRewards(const QString& delegationId);

    // Settings
    void saveSettings();
    void loadSettings();

signals:
    void setupCompleted(const DelegationInfo& delegation);
    void delegationStatusChanged(const QString& delegationId, DelegationStatus status);
    void rewardsClaimed(const QString& delegationId, double amount);

public Q_SLOTS:
    void onNextStepClicked();
    void onPreviousStepClicked();
    void onGenerateQRClicked();
    void onImportSignedTxClicked();
    void onConfirmDelegationClicked();
    void onRevokeDelegationClicked();
    void onClaimRewardsClicked();
    void onShowInstructionsClicked();
    void onExportDelegationClicked();

private Q_SLOTS:
    void onUpdateTimer();
    void onDelegationItemClicked(QTableWidgetItem* item);
    void onContextMenuRequested(const QPoint& pos);

private:
    void setupUI();
    void connectSignals();
    void applyTheme();

    // Setup flow management
    void showStep(SetupStep step);
    void updateStepProgress();
    void validateCurrentStep();
    bool canProceedToNextStep();

    // QR code generation
    void generateDelegationQRCode(const DelegationInfo& delegation);
    QString createDelegationPayload(const DelegationInfo& delegation);
    bool validateSignedTransaction(const QString& signedTx);

    // Delegation management
    QList<DelegationInfo> loadDelegations();
    void saveDelegations(const QList<DelegationInfo>& delegations);
    void updateDelegationStatus(const QString& delegationId, DelegationStatus status);
    double calculateDelegationRewards(const DelegationInfo& delegation);

    // Hot node discovery
    QList<HotNodeInfo> discoverHotNodes();
    bool validateHotNode(const HotNodeInfo& node);
    void testHotNodeConnection(const QString& nodeAddress);

    // UI helpers
    void updateDelegationDisplay();
    void updateHotNodeDisplay();
    void showConfirmationDialog(const DelegationInfo& delegation);
    void showInstructionsDialog();
    void animateStepTransition();
    void showNotification(const QString& message, const QString& type = "info");

    // Data persistence
    void saveDelegationHistory();
    void loadDelegationHistory();
    void exportDelegationData();

    // Members
    WalletModel* m_walletModel;
    PlatformStyle* m_platformStyle;

    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Setup Progress
    QGroupBox* m_progressGroup;
    QHBoxLayout* m_progressLayout;
    QProgressBar* m_stepProgressBar;
    QLabel* m_stepLabel;
    QPushButton* m_previousButton;
    QPushButton* m_nextButton;

    // Step Content Area
    QScrollArea* m_stepContentArea;
    QWidget* m_stepContentWidget;
    QVBoxLayout* m_stepContentLayout;

    // Welcome Step
    QWidget* m_welcomeStep;
    QLabel* m_welcomeTitle;
    QTextBrowser* m_welcomeText;
    QPushButton* m_startSetupButton;

    // Cold Wallet Selection Step
    QWidget* m_coldWalletStep;
    QLabel* m_coldWalletLabel;
    QLineEdit* m_coldWalletInput;
    QPushButton* m_importColdWalletButton;
    QLabel* m_coldWalletBalanceLabel;
    QTextBrowser* m_coldWalletInstructions;

    // Hot Node Configuration Step
    QWidget* m_hotNodeStep;
    QComboBox* m_hotNodeCombo;
    QPushButton* m_refreshNodesButton;
    QLabel* m_nodeInfoLabel;
    QTextBrowser* m_nodeDetailsText;
    QPushButton* m_testConnectionButton;

    // Delegation Amount Step
    QWidget* m_amountStep;
    QLabel* m_amountLabel;
    QDoubleSpinBox* m_delegationAmountInput;
    QLabel* m_availableBalanceLabel;
    QLabel* m_feeLabel;
    QLabel* m_estimatedRewardsLabel;
    QSlider* m_amountSlider;

    // QR Code Generation Step
    QWidget* m_qrStep;
    QLabel* m_qrCodeLabel;
    QPushButton* m_generateQRButton;
    QPushButton* m_copyPayloadButton;
    QPushButton* m_saveQRButton;
    QTextBrowser* m_qrInstructions;

    // Confirmation Step
    QWidget* m_confirmationStep;
    QLabel* m_delegationSummaryLabel;
    QTextBrowser* m_delegationDetailsText;
    QPushButton* m_confirmDelegationButton;
    QPushButton* m_editDelegationButton;

    // Setup Complete Step
    QWidget* m_completeStep;
    QLabel* m_completionTitle;
    QTextBrowser* m_completionText;
    QPushButton* m_viewDelegationsButton;
    QPushButton* m_newDelegationButton;

    // Delegation Management
    QGroupBox* m_delegationsGroup;
    QVBoxLayout* m_delegationsLayout;
    QTableWidget* m_delegationsTable;
    QPushButton* m_revokeDelegationButton;
    QPushButton* m_claimRewardsButton;
    QPushButton* m_exportDelegationsButton;

    // Instructions Panel
    QGroupBox* m_instructionsGroup;
    QTextBrowser* m_instructionsText;
    QPushButton* m_showInstructionsButton;

    // State
    SetupStep m_currentStep;
    DelegationInfo m_currentDelegation;
    QList<DelegationInfo> m_delegations;
    QList<HotNodeInfo> m_hotNodes;
    bool m_setupInProgress;
    QString m_delegationQRData;

    // Network and timing
    QNetworkAccessManager* m_networkManager;
    QTimer* m_updateTimer;
    QTimer* m_animationTimer;
    QDateTime m_lastUpdate;

    // Data storage
    QSettings* m_settings;
    QList<QPair<QDateTime, double>> m_rewardsHistory;

    // Animation
    QPropertyAnimation* m_stepAnimation;
    QGraphicsOpacityEffect* m_stepOpacityEffect;

    // Constants
    static const int UPDATE_INTERVAL_MS = 60000; // 1 minute
    static const int ANIMATION_DURATION_MS = 300;
    static const QString SETTINGS_GROUP;
    static const QString SETTINGS_DELEGATIONS;
    static const QString SETTINGS_HOT_NODES;
    static const QString SETTINGS_LAST_UPDATE;
};

/**
 * @brief Delegation History Table Widget
 */
class DelegationHistoryTable : public QTableWidget
{
    Q_OBJECT

public:
    explicit DelegationHistoryTable(QWidget* parent = nullptr);
    void addDelegationRecord(const ColdStakingSetup::DelegationInfo& delegation);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private Q_SLOTS:
    void onCopyAddress();
    void onCopyTransactionId();
    void onExportDelegation();
};

/**
 * @brief Hot Node Discovery Widget
 */
class HotNodeDiscoveryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HotNodeDiscoveryWidget(QWidget* parent = nullptr);
    void refreshHotNodes();

signals:
    void hotNodeSelected(const ColdStakingSetup::HotNodeInfo& node);

private:
    void setupUI();
    void updateNodeList();

    QVBoxLayout* m_layout;
    QTableWidget* m_nodeTable;
    QPushButton* m_refreshButton;
    QList<ColdStakingSetup::HotNodeInfo> m_hotNodes;
};

/**
 * @brief Delegation Confirmation Dialog
 */
class DelegationConfirmationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DelegationConfirmationDialog(const ColdStakingSetup::DelegationInfo& delegation,
                                         QWidget* parent = nullptr);

signals:
    void delegationConfirmed(const ColdStakingSetup::DelegationInfo& delegation);
    void delegationCancelled();

private Q_SLOTS:
    void onConfirmClicked();
    void onCancelClicked();

private:
    void setupUI();
    void applyTheme();

    ColdStakingSetup::DelegationInfo m_delegation;
    QVBoxLayout* m_layout;
    QTextBrowser* m_summaryText;
    QPushButton* m_confirmButton;
    QPushButton* m_cancelButton;
};

#endif // SHAHCOIN_QT_COLDSTAKINGSETUP_H
