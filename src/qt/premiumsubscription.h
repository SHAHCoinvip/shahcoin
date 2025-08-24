#ifndef SHAHCOIN_QT_PREMIUMSUBSCRIPTION_H
#define SHAHCOIN_QT_PREMIUMSUBSCRIPTION_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QProgressBar>
#include <QTableWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QSpinBox>
#include <QTimer>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QCryptographicHash>

class WalletModel;
class PlatformStyle;
class PriceOracle;

/**
 * Premium Subscription Plans
 */
enum class PremiumPlan {
    FREE = 0,
    MONTHLY = 1,
    YEARLY = 2
};

/**
 * Payment Methods for Premium Subscriptions
 */
enum class PaymentMethod {
    SHAH_NATIVE = 0,
    SHAH_TOKEN = 1,
    STRIPE_CARD = 2
};

/**
 * Premium Features Enum
 */
enum class PremiumFeature {
    AI_WALLET_ASSISTANT = 0,
    AI_INSIGHTS_PANEL = 1,
    AI_FEE_OPTIMIZER = 2,
    AI_TROUBLESHOOTER = 3,
    MULTILINGUAL_ASSISTANT = 4,
    AI_ART_BOOSTER = 5,
    NFT_COLLECTION_DROPS = 6,
    AUCTION_MODULE = 7,
    AI_PORTFOLIO_ANALYZER = 8,
    NFT_FLOOR_PRICE_ALERTS = 9,
    USAGE_DASHBOARD = 10,
    EMAIL_CONFIRMATION_TOGGLE = 11
};

/**
 * Subscription Status Tracking
 */
struct SubscriptionData {
    QString subscriptionId;
    QString walletAddress;
    PremiumPlan plan;
    PaymentMethod paymentMethod;
    QDateTime subscriptionDate;
    QDateTime expirationDate;
    bool isActive;
    bool autoRenew;
    double amountPaidUSD;
    qint64 amountPaidSatoshis;
    QString transactionId;
    QString stripeSessionId;
    QDateTime lastPaymentDate;
    int renewalCount;
    QString emailAddress;
    bool emailConfirmationEnabled;
};

/**
 * Payment Transaction Record
 */
struct PaymentTransaction {
    QString transactionId;
    QString subscriptionId;
    QString walletAddress;
    PremiumPlan plan;
    PaymentMethod paymentMethod;
    QDateTime timestamp;
    double amountUSD;
    qint64 amountSatoshis;
    QString status; // "pending", "confirmed", "failed", "refunded"
    QString blockHash;
    int blockHeight;
    QString stripePaymentIntentId;
    QString notes;
};

/**
 * Core Premium Subscription Manager
 */
class PremiumSubscriptionManager : public QObject
{
    Q_OBJECT

public:
    explicit PremiumSubscriptionManager(QObject* parent = nullptr);
    ~PremiumSubscriptionManager();

    // Subscription Management
    bool isFeatureUnlocked(PremiumFeature feature) const;
    bool isPremiumActive() const;
    PremiumPlan getCurrentPlan() const;
    SubscriptionData getCurrentSubscription() const;
    QDateTime getExpirationDate() const;
    int getDaysRemaining() const;

    // Payment Processing
    bool initiatePurchase(PremiumPlan plan, PaymentMethod method, const QString& emailAddress = "");
    bool processShahPayment(PremiumPlan plan, qint64 amountSatoshis);
    bool processStripePayment(PremiumPlan plan, double amountUSD, const QString& emailAddress);
    
    // Subscription Control
    bool activateSubscription(const SubscriptionData& data);
    bool renewSubscription();
    bool cancelSubscription();
    bool upgradeSubscription(PremiumPlan newPlan);
    
    // Pricing
    double getPlanPriceUSD(PremiumPlan plan) const;
    qint64 getPlanPriceSatoshis(PremiumPlan plan) const;
    QString getPlanDisplayName(PremiumPlan plan) const;
    QStringList getPlanFeatures(PremiumPlan plan) const;
    
    // Payment History
    QList<PaymentTransaction> getPaymentHistory() const;
    void addPaymentTransaction(const PaymentTransaction& transaction);
    
    // Settings
    void setWalletModel(WalletModel* walletModel);
    void setPriceOracle(PriceOracle* priceOracle);
    void setEmailConfirmationEnabled(bool enabled);
    bool isEmailConfirmationEnabled() const;

    // External Integration
    void syncWithAPI();
    void validateSubscriptionStatus();

Q_SIGNALS:
    void subscriptionStatusChanged();
    void paymentProcessed(bool success, const QString& message);
    void featureAccessChanged(PremiumFeature feature, bool unlocked);
    void subscriptionExpiring(int daysRemaining);
    void subscriptionExpired();

private Q_SLOTS:
    void onPriceOracleUpdated();
    void onSubscriptionCheckTimer();
    void onApiSyncReply();

private:
    void setupSubscriptionTimer();
    void loadSubscriptionData();
    void saveSubscriptionData();
    void checkSubscriptionExpiry();
    void sendExpirationNotification();
    bool validatePaymentTransaction(const QString& txid, qint64 expectedAmount);
    QString generateSubscriptionId();
    void updateFeatureAccess();

    WalletModel* m_walletModel;
    PriceOracle* m_priceOracle;
    SubscriptionData m_currentSubscription;
    QList<PaymentTransaction> m_paymentHistory;
    QTimer* m_subscriptionTimer;
    QNetworkAccessManager* m_networkManager;
    QSettings m_settings;
    QString m_apiBaseUrl;
    
    // Feature access cache
    QMap<PremiumFeature, bool> m_featureAccess;
    
    // Pricing constants
    static constexpr double MONTHLY_PRICE_USD = 4.99;
    static constexpr double YEARLY_PRICE_USD = 49.99;
};

/**
 * Premium Purchase Dialog
 */
class PremiumPurchaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PremiumPurchaseDialog(PremiumSubscriptionManager* manager, QWidget* parent = nullptr);

private Q_SLOTS:
    void onPlanChanged();
    void onPaymentMethodChanged();
    void onPurchaseClicked();
    void onCancelClicked();
    void onPriceUpdated();

private:
    void setupUI();
    void updatePricing();
    void updatePlanComparison();
    void processPayment();

    PremiumSubscriptionManager* m_subscriptionManager;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Plan Selection Tab
    QWidget* m_planTab;
    QVBoxLayout* m_planLayout;
    QGroupBox* m_planGroup;
    QComboBox* m_planCombo;
    QLabel* m_planDescription;
    QLabel* m_planFeatures;
    QLabel* m_planPrice;
    QCheckBox* m_autoRenewCheck;
    
    // Payment Tab
    QWidget* m_paymentTab;
    QVBoxLayout* m_paymentLayout;
    QGroupBox* m_paymentGroup;
    QComboBox* m_paymentMethodCombo;
    QLabel* m_paymentDescription;
    QLineEdit* m_emailEdit;
    QCheckBox* m_emailReceiptsCheck;
    
    // Summary Tab
    QWidget* m_summaryTab;
    QVBoxLayout* m_summaryLayout;
    QLabel* m_summaryDetails;
    QLabel* m_totalAmount;
    QPushButton* m_purchaseButton;
    QPushButton* m_cancelButton;
    
    // Current selections
    PremiumPlan m_selectedPlan;
    PaymentMethod m_selectedPaymentMethod;
};

/**
 * Premium Status Widget for main wallet
 */
class PremiumStatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PremiumStatusWidget(PremiumSubscriptionManager* manager, QWidget* parent = nullptr);

public Q_SLOTS:
    void updateStatus();

private Q_SLOTS:
    void onUpgradeClicked();
    void onManageSubscriptionClicked();

private:
    void setupUI();

    PremiumSubscriptionManager* m_subscriptionManager;
    QHBoxLayout* m_layout;
    QLabel* m_statusIcon;
    QLabel* m_statusText;
    QPushButton* m_upgradeButton;
    QPushButton* m_manageButton;
};

/**
 * Subscription Management Dialog
 */
class SubscriptionManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SubscriptionManagementDialog(PremiumSubscriptionManager* manager, QWidget* parent = nullptr);

private Q_SLOTS:
    void onRenewClicked();
    void onCancelClicked();
    void onUpgradeClicked();
    void onPaymentHistoryClicked();
    void onEmailSettingsChanged();

private:
    void setupUI();
    void updateSubscriptionInfo();
    void populatePaymentHistory();

    PremiumSubscriptionManager* m_subscriptionManager;
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Subscription Info Tab
    QWidget* m_infoTab;
    QLabel* m_planLabel;
    QLabel* m_statusLabel;
    QLabel* m_expirationLabel;
    QLabel* m_renewalLabel;
    QPushButton* m_renewButton;
    QPushButton* m_cancelButton;
    QPushButton* m_upgradeButton;
    
    // Payment History Tab
    QWidget* m_historyTab;
    QTableWidget* m_historyTable;
    QPushButton* m_exportButton;
    
    // Settings Tab
    QWidget* m_settingsTab;
    QCheckBox* m_autoRenewCheck;
    QCheckBox* m_emailConfirmationCheck;
    QLineEdit* m_emailEdit;
    QPushButton* m_saveSettingsButton;
};

/**
 * Feature Gate Widget - shows lock icon for premium features
 */
class FeatureGateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FeatureGateWidget(PremiumFeature feature, PremiumSubscriptionManager* manager, QWidget* parent = nullptr);
    
    void setGatedWidget(QWidget* widget);
    bool isFeatureUnlocked() const;

private Q_SLOTS:
    void onFeatureAccessChanged();
    void onUpgradeClicked();

private:
    void setupUI();
    void updateGateStatus();

    PremiumFeature m_feature;
    PremiumSubscriptionManager* m_subscriptionManager;
    QWidget* m_gatedWidget;
    
    QVBoxLayout* m_layout;
    QWidget* m_lockOverlay;
    QLabel* m_lockIcon;
    QLabel* m_lockMessage;
    QPushButton* m_upgradeButton;
};

#endif // SHAHCOIN_QT_PREMIUMSUBSCRIPTION_H
