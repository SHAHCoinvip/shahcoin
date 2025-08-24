#include "premiumsubscription.h"
#include "walletmodel.h"
#include "priceoracle.h"
#include "guiutil.h"
#include "shahcoinunits.h"
#include "util/translation.h"

#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QPixmap>
#include <QIcon>
#include <QHeaderView>
#include <QTextStream>
#include <QUuid>
#include <QCryptographicHash>

// PremiumSubscriptionManager Implementation

PremiumSubscriptionManager::PremiumSubscriptionManager(QObject* parent)
    : QObject(parent)
    , m_walletModel(nullptr)
    , m_priceOracle(nullptr)
    , m_subscriptionTimer(new QTimer(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_settings("Shahcoin", "PremiumSubscription")
{
    m_apiBaseUrl = "https://api.shah.vip/premium";
    
    // Initialize subscription data
    loadSubscriptionData();
    updateFeatureAccess();
    
    // Setup periodic subscription check (every hour)
    setupSubscriptionTimer();
    
    // Connect signals
    connect(m_subscriptionTimer, &QTimer::timeout, this, &PremiumSubscriptionManager::onSubscriptionCheckTimer);
}

PremiumSubscriptionManager::~PremiumSubscriptionManager()
{
    saveSubscriptionData();
}

bool PremiumSubscriptionManager::isFeatureUnlocked(PremiumFeature feature) const
{
    return m_featureAccess.value(feature, false);
}

bool PremiumSubscriptionManager::isPremiumActive() const
{
    return m_currentSubscription.isActive && 
           m_currentSubscription.expirationDate > QDateTime::currentDateTime();
}

PremiumPlan PremiumSubscriptionManager::getCurrentPlan() const
{
    return isPremiumActive() ? m_currentSubscription.plan : PremiumPlan::FREE;
}

SubscriptionData PremiumSubscriptionManager::getCurrentSubscription() const
{
    return m_currentSubscription;
}

QDateTime PremiumSubscriptionManager::getExpirationDate() const
{
    return m_currentSubscription.expirationDate;
}

int PremiumSubscriptionManager::getDaysRemaining() const
{
    if (!isPremiumActive()) return 0;
    
    QDateTime now = QDateTime::currentDateTime();
    QDateTime expiry = m_currentSubscription.expirationDate;
    
    return now.daysTo(expiry);
}

bool PremiumSubscriptionManager::initiatePurchase(PremiumPlan plan, PaymentMethod method, const QString& emailAddress)
{
    if (plan == PremiumPlan::FREE) {
        return false;
    }
    
    double priceUSD = getPlanPriceUSD(plan);
    qint64 priceSatoshis = getPlanPriceSatoshis(plan);
    
    PaymentTransaction transaction;
    transaction.transactionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    transaction.subscriptionId = generateSubscriptionId();
    transaction.walletAddress = m_walletModel ? m_walletModel->getWalletAddress() : "";
    transaction.plan = plan;
    transaction.paymentMethod = method;
    transaction.timestamp = QDateTime::currentDateTime();
    transaction.amountUSD = priceUSD;
    transaction.amountSatoshis = priceSatoshis;
    transaction.status = "pending";
    
    bool success = false;
    
    switch (method) {
        case PaymentMethod::SHAH_NATIVE:
        case PaymentMethod::SHAH_TOKEN:
            success = processShahPayment(plan, priceSatoshis);
            break;
        case PaymentMethod::STRIPE_CARD:
            success = processStripePayment(plan, priceUSD, emailAddress);
            break;
    }
    
    if (success) {
        transaction.status = "confirmed";
        addPaymentTransaction(transaction);
        
        // Activate subscription
        SubscriptionData newSubscription;
        newSubscription.subscriptionId = transaction.subscriptionId;
        newSubscription.walletAddress = transaction.walletAddress;
        newSubscription.plan = plan;
        newSubscription.paymentMethod = method;
        newSubscription.subscriptionDate = QDateTime::currentDateTime();
        
        // Set expiration date
        if (plan == PremiumPlan::MONTHLY) {
            newSubscription.expirationDate = newSubscription.subscriptionDate.addMonths(1);
        } else if (plan == PremiumPlan::YEARLY) {
            newSubscription.expirationDate = newSubscription.subscriptionDate.addYears(1);
        }
        
        newSubscription.isActive = true;
        newSubscription.autoRenew = true;
        newSubscription.amountPaidUSD = priceUSD;
        newSubscription.amountPaidSatoshis = priceSatoshis;
        newSubscription.transactionId = transaction.transactionId;
        newSubscription.lastPaymentDate = QDateTime::currentDateTime();
        newSubscription.renewalCount = 0;
        newSubscription.emailAddress = emailAddress;
        newSubscription.emailConfirmationEnabled = !emailAddress.isEmpty();
        
        activateSubscription(newSubscription);
    } else {
        transaction.status = "failed";
        addPaymentTransaction(transaction);
    }
    
    emit paymentProcessed(success, success ? tr("Subscription activated successfully!") : tr("Payment failed. Please try again."));
    return success;
}

bool PremiumSubscriptionManager::processShahPayment(PremiumPlan plan, qint64 amountSatoshis)
{
    if (!m_walletModel) {
        return false;
    }
    
    // Create payment transaction to premium subscription address
    QString premiumAddress = "SHAHPremium1234567890abcdefghijklmnop"; // Premium subscription address
    
    try {
        // Use wallet model to create and send transaction
        QString txid = m_walletModel->sendCoins(premiumAddress, amountSatoshis, "Premium Subscription Payment");
        
        if (!txid.isEmpty()) {
            // Transaction created successfully
            return true;
        }
    } catch (...) {
        // Transaction failed
    }
    
    return false;
}

bool PremiumSubscriptionManager::processStripePayment(PremiumPlan plan, double amountUSD, const QString& emailAddress)
{
    // Create Stripe checkout session
    QJsonObject sessionData;
    sessionData["plan"] = static_cast<int>(plan);
    sessionData["amount"] = amountUSD;
    sessionData["currency"] = "USD";
    sessionData["email"] = emailAddress;
    sessionData["wallet_address"] = m_walletModel ? m_walletModel->getWalletAddress() : "";
    
    QJsonDocument doc(sessionData);
    QByteArray data = doc.toJson();
    
    QNetworkRequest request(QUrl(m_apiBaseUrl + "/stripe/create-session"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->post(request, data);
    
    // For demonstration, we'll simulate success
    // In real implementation, this would redirect to Stripe checkout
    QTimer::singleShot(2000, [this]() {
        // Simulate successful Stripe payment
        emit paymentProcessed(true, tr("Stripe payment processed successfully!"));
    });
    
    return true;
}

bool PremiumSubscriptionManager::activateSubscription(const SubscriptionData& data)
{
    m_currentSubscription = data;
    updateFeatureAccess();
    saveSubscriptionData();
    
    emit subscriptionStatusChanged();
    return true;
}

bool PremiumSubscriptionManager::renewSubscription()
{
    if (!isPremiumActive()) {
        return false;
    }
    
    // Process renewal payment
    bool success = initiatePurchase(m_currentSubscription.plan, m_currentSubscription.paymentMethod, m_currentSubscription.emailAddress);
    
    if (success) {
        m_currentSubscription.renewalCount++;
        m_currentSubscription.lastPaymentDate = QDateTime::currentDateTime();
        
        // Extend expiration date
        if (m_currentSubscription.plan == PremiumPlan::MONTHLY) {
            m_currentSubscription.expirationDate = m_currentSubscription.expirationDate.addMonths(1);
        } else if (m_currentSubscription.plan == PremiumPlan::YEARLY) {
            m_currentSubscription.expirationDate = m_currentSubscription.expirationDate.addYears(1);
        }
        
        saveSubscriptionData();
        emit subscriptionStatusChanged();
    }
    
    return success;
}

bool PremiumSubscriptionManager::cancelSubscription()
{
    m_currentSubscription.isActive = false;
    m_currentSubscription.autoRenew = false;
    
    updateFeatureAccess();
    saveSubscriptionData();
    
    emit subscriptionStatusChanged();
    return true;
}

bool PremiumSubscriptionManager::upgradeSubscription(PremiumPlan newPlan)
{
    if (newPlan <= getCurrentPlan()) {
        return false; // Cannot downgrade or same plan
    }
    
    // Calculate prorated amount
    double currentPrice = getPlanPriceUSD(getCurrentPlan());
    double newPrice = getPlanPriceUSD(newPlan);
    double proratedAmount = newPrice - currentPrice;
    
    // Process upgrade payment
    return initiatePurchase(newPlan, m_currentSubscription.paymentMethod, m_currentSubscription.emailAddress);
}

double PremiumSubscriptionManager::getPlanPriceUSD(PremiumPlan plan) const
{
    switch (plan) {
        case PremiumPlan::MONTHLY:
            return MONTHLY_PRICE_USD;
        case PremiumPlan::YEARLY:
            return YEARLY_PRICE_USD;
        default:
            return 0.0;
    }
}

qint64 PremiumSubscriptionManager::getPlanPriceSatoshis(PremiumPlan plan) const
{
    double priceUSD = getPlanPriceUSD(plan);
    if (!m_priceOracle || priceUSD == 0.0) {
        return 0;
    }
    
    double shahPrice = m_priceOracle->getPrice();
    if (shahPrice == 0.0) {
        return 0;
    }
    
    double shahAmount = priceUSD / shahPrice;
    return static_cast<qint64>(shahAmount * 100000000); // Convert to satoshis
}

QString PremiumSubscriptionManager::getPlanDisplayName(PremiumPlan plan) const
{
    switch (plan) {
        case PremiumPlan::MONTHLY:
            return tr("Monthly Premium");
        case PremiumPlan::YEARLY:
            return tr("Yearly Premium");
        default:
            return tr("Free");
    }
}

QStringList PremiumSubscriptionManager::getPlanFeatures(PremiumPlan plan) const
{
    QStringList features;
    
    if (plan == PremiumPlan::FREE) {
        features << tr("Basic wallet features");
        features << tr("Standard transaction support");
        features << tr("Basic NFT browsing");
        return features;
    }
    
    // All premium plans include these features
    features << tr("🧠 AI Wallet Assistant");
    features << tr("📊 AI Insights Panel");
    features << tr("🧮 AI Fee Optimizer");
    features << tr("🛠️ AI Troubleshooter");
    features << tr("🌐 Multilingual Assistant");
    features << tr("🎨 AI Art Booster");
    features << tr("📦 NFT Collection Drops");
    features << tr("🕰️ Auction Module");
    features << tr("📈 AI Portfolio Analyzer");
    features << tr("🔔 NFT Floor Price Alerts");
    features << tr("🧾 Usage Dashboard");
    features << tr("📬 Email Confirmations");
    
    if (plan == PremiumPlan::YEARLY) {
        features << tr("💰 17% savings vs monthly");
        features << tr("🎁 Bonus premium support");
    }
    
    return features;
}

QList<PaymentTransaction> PremiumSubscriptionManager::getPaymentHistory() const
{
    return m_paymentHistory;
}

void PremiumSubscriptionManager::addPaymentTransaction(const PaymentTransaction& transaction)
{
    m_paymentHistory.append(transaction);
    
    // Keep only last 100 transactions
    if (m_paymentHistory.size() > 100) {
        m_paymentHistory.removeFirst();
    }
    
    saveSubscriptionData();
}

void PremiumSubscriptionManager::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
}

void PremiumSubscriptionManager::setPriceOracle(PriceOracle* priceOracle)
{
    if (m_priceOracle) {
        disconnect(m_priceOracle, nullptr, this, nullptr);
    }
    
    m_priceOracle = priceOracle;
    
    if (m_priceOracle) {
        connect(m_priceOracle, &PriceOracle::priceUpdated, this, &PremiumSubscriptionManager::onPriceOracleUpdated);
    }
}

void PremiumSubscriptionManager::setEmailConfirmationEnabled(bool enabled)
{
    m_currentSubscription.emailConfirmationEnabled = enabled;
    saveSubscriptionData();
}

bool PremiumSubscriptionManager::isEmailConfirmationEnabled() const
{
    return m_currentSubscription.emailConfirmationEnabled;
}

void PremiumSubscriptionManager::syncWithAPI()
{
    if (!m_walletModel) return;
    
    QString walletAddress = m_walletModel->getWalletAddress();
    
    QNetworkRequest request(QUrl(m_apiBaseUrl + "/subscription/" + walletAddress));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &PremiumSubscriptionManager::onApiSyncReply);
}

void PremiumSubscriptionManager::validateSubscriptionStatus()
{
    checkSubscriptionExpiry();
    updateFeatureAccess();
}

void PremiumSubscriptionManager::onPriceOracleUpdated()
{
    // Price updated, emit signal to update UI pricing
    emit subscriptionStatusChanged();
}

void PremiumSubscriptionManager::onSubscriptionCheckTimer()
{
    validateSubscriptionStatus();
    
    // Check for expiration warnings
    int daysRemaining = getDaysRemaining();
    if (daysRemaining > 0 && daysRemaining <= 7) {
        emit subscriptionExpiring(daysRemaining);
    }
    
    // Sync with API periodically
    syncWithAPI();
}

void PremiumSubscriptionManager::onApiSyncReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        
        // Update subscription data from API
        if (obj.contains("subscription")) {
            QJsonObject subObj = obj["subscription"].toObject();
            
            SubscriptionData newData;
            newData.subscriptionId = subObj["id"].toString();
            newData.walletAddress = subObj["wallet_address"].toString();
            newData.plan = static_cast<PremiumPlan>(subObj["plan"].toInt());
            newData.paymentMethod = static_cast<PaymentMethod>(subObj["payment_method"].toInt());
            newData.subscriptionDate = QDateTime::fromString(subObj["subscription_date"].toString(), Qt::ISODate);
            newData.expirationDate = QDateTime::fromString(subObj["expiration_date"].toString(), Qt::ISODate);
            newData.isActive = subObj["is_active"].toBool();
            newData.autoRenew = subObj["auto_renew"].toBool();
            newData.amountPaidUSD = subObj["amount_usd"].toDouble();
            newData.amountPaidSatoshis = subObj["amount_satoshis"].toVariant().toLongLong();
            newData.renewalCount = subObj["renewal_count"].toInt();
            newData.emailAddress = subObj["email"].toString();
            newData.emailConfirmationEnabled = subObj["email_confirmation"].toBool();
            
            // Update if different
            if (newData.subscriptionId != m_currentSubscription.subscriptionId ||
                newData.expirationDate != m_currentSubscription.expirationDate) {
                m_currentSubscription = newData;
                updateFeatureAccess();
                saveSubscriptionData();
                emit subscriptionStatusChanged();
            }
        }
    }
    
    reply->deleteLater();
}

void PremiumSubscriptionManager::setupSubscriptionTimer()
{
    m_subscriptionTimer->start(3600000); // Check every hour
}

void PremiumSubscriptionManager::loadSubscriptionData()
{
    m_settings.beginGroup("subscription");
    
    m_currentSubscription.subscriptionId = m_settings.value("id").toString();
    m_currentSubscription.walletAddress = m_settings.value("wallet_address").toString();
    m_currentSubscription.plan = static_cast<PremiumPlan>(m_settings.value("plan", 0).toInt());
    m_currentSubscription.paymentMethod = static_cast<PaymentMethod>(m_settings.value("payment_method", 0).toInt());
    m_currentSubscription.subscriptionDate = m_settings.value("subscription_date").toDateTime();
    m_currentSubscription.expirationDate = m_settings.value("expiration_date").toDateTime();
    m_currentSubscription.isActive = m_settings.value("is_active", false).toBool();
    m_currentSubscription.autoRenew = m_settings.value("auto_renew", true).toBool();
    m_currentSubscription.amountPaidUSD = m_settings.value("amount_usd", 0.0).toDouble();
    m_currentSubscription.amountPaidSatoshis = m_settings.value("amount_satoshis", 0).toLongLong();
    m_currentSubscription.transactionId = m_settings.value("transaction_id").toString();
    m_currentSubscription.stripeSessionId = m_settings.value("stripe_session_id").toString();
    m_currentSubscription.lastPaymentDate = m_settings.value("last_payment_date").toDateTime();
    m_currentSubscription.renewalCount = m_settings.value("renewal_count", 0).toInt();
    m_currentSubscription.emailAddress = m_settings.value("email").toString();
    m_currentSubscription.emailConfirmationEnabled = m_settings.value("email_confirmation", false).toBool();
    
    m_settings.endGroup();
    
    // Load payment history
    m_settings.beginGroup("payment_history");
    int historySize = m_settings.beginReadArray("transactions");
    
    for (int i = 0; i < historySize; ++i) {
        m_settings.setArrayIndex(i);
        
        PaymentTransaction transaction;
        transaction.transactionId = m_settings.value("transaction_id").toString();
        transaction.subscriptionId = m_settings.value("subscription_id").toString();
        transaction.walletAddress = m_settings.value("wallet_address").toString();
        transaction.plan = static_cast<PremiumPlan>(m_settings.value("plan").toInt());
        transaction.paymentMethod = static_cast<PaymentMethod>(m_settings.value("payment_method").toInt());
        transaction.timestamp = m_settings.value("timestamp").toDateTime();
        transaction.amountUSD = m_settings.value("amount_usd").toDouble();
        transaction.amountSatoshis = m_settings.value("amount_satoshis").toLongLong();
        transaction.status = m_settings.value("status").toString();
        transaction.blockHash = m_settings.value("block_hash").toString();
        transaction.blockHeight = m_settings.value("block_height").toInt();
        transaction.stripePaymentIntentId = m_settings.value("stripe_payment_intent").toString();
        transaction.notes = m_settings.value("notes").toString();
        
        m_paymentHistory.append(transaction);
    }
    
    m_settings.endArray();
    m_settings.endGroup();
}

void PremiumSubscriptionManager::saveSubscriptionData()
{
    m_settings.beginGroup("subscription");
    
    m_settings.setValue("id", m_currentSubscription.subscriptionId);
    m_settings.setValue("wallet_address", m_currentSubscription.walletAddress);
    m_settings.setValue("plan", static_cast<int>(m_currentSubscription.plan));
    m_settings.setValue("payment_method", static_cast<int>(m_currentSubscription.paymentMethod));
    m_settings.setValue("subscription_date", m_currentSubscription.subscriptionDate);
    m_settings.setValue("expiration_date", m_currentSubscription.expirationDate);
    m_settings.setValue("is_active", m_currentSubscription.isActive);
    m_settings.setValue("auto_renew", m_currentSubscription.autoRenew);
    m_settings.setValue("amount_usd", m_currentSubscription.amountPaidUSD);
    m_settings.setValue("amount_satoshis", m_currentSubscription.amountPaidSatoshis);
    m_settings.setValue("transaction_id", m_currentSubscription.transactionId);
    m_settings.setValue("stripe_session_id", m_currentSubscription.stripeSessionId);
    m_settings.setValue("last_payment_date", m_currentSubscription.lastPaymentDate);
    m_settings.setValue("renewal_count", m_currentSubscription.renewalCount);
    m_settings.setValue("email", m_currentSubscription.emailAddress);
    m_settings.setValue("email_confirmation", m_currentSubscription.emailConfirmationEnabled);
    
    m_settings.endGroup();
    
    // Save payment history
    m_settings.beginGroup("payment_history");
    m_settings.beginWriteArray("transactions");
    
    for (int i = 0; i < m_paymentHistory.size(); ++i) {
        m_settings.setArrayIndex(i);
        const PaymentTransaction& transaction = m_paymentHistory[i];
        
        m_settings.setValue("transaction_id", transaction.transactionId);
        m_settings.setValue("subscription_id", transaction.subscriptionId);
        m_settings.setValue("wallet_address", transaction.walletAddress);
        m_settings.setValue("plan", static_cast<int>(transaction.plan));
        m_settings.setValue("payment_method", static_cast<int>(transaction.paymentMethod));
        m_settings.setValue("timestamp", transaction.timestamp);
        m_settings.setValue("amount_usd", transaction.amountUSD);
        m_settings.setValue("amount_satoshis", transaction.amountSatoshis);
        m_settings.setValue("status", transaction.status);
        m_settings.setValue("block_hash", transaction.blockHash);
        m_settings.setValue("block_height", transaction.blockHeight);
        m_settings.setValue("stripe_payment_intent", transaction.stripePaymentIntentId);
        m_settings.setValue("notes", transaction.notes);
    }
    
    m_settings.endArray();
    m_settings.endGroup();
}

void PremiumSubscriptionManager::checkSubscriptionExpiry()
{
    if (m_currentSubscription.isActive) {
        QDateTime now = QDateTime::currentDateTime();
        
        if (m_currentSubscription.expirationDate <= now) {
            // Subscription expired
            m_currentSubscription.isActive = false;
            updateFeatureAccess();
            saveSubscriptionData();
            
            emit subscriptionExpired();
            emit subscriptionStatusChanged();
        }
    }
}

void PremiumSubscriptionManager::sendExpirationNotification()
{
    // This would send email notification in real implementation
}

bool PremiumSubscriptionManager::validatePaymentTransaction(const QString& txid, qint64 expectedAmount)
{
    // In real implementation, this would verify the transaction on the blockchain
    return !txid.isEmpty() && expectedAmount > 0;
}

QString PremiumSubscriptionManager::generateSubscriptionId()
{
    QByteArray data = QUuid::createUuid().toByteArray();
    data.append(QDateTime::currentDateTime().toString().toUtf8());
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data);
    
    return hash.result().toHex().left(16); // 16 character ID
}

void PremiumSubscriptionManager::updateFeatureAccess()
{
    bool isPremium = isPremiumActive();
    
    // Update all feature access
    m_featureAccess[PremiumFeature::AI_WALLET_ASSISTANT] = isPremium;
    m_featureAccess[PremiumFeature::AI_INSIGHTS_PANEL] = isPremium;
    m_featureAccess[PremiumFeature::AI_FEE_OPTIMIZER] = isPremium;
    m_featureAccess[PremiumFeature::AI_TROUBLESHOOTER] = isPremium;
    m_featureAccess[PremiumFeature::MULTILINGUAL_ASSISTANT] = isPremium;
    m_featureAccess[PremiumFeature::AI_ART_BOOSTER] = isPremium;
    m_featureAccess[PremiumFeature::NFT_COLLECTION_DROPS] = isPremium;
    m_featureAccess[PremiumFeature::AUCTION_MODULE] = isPremium;
    m_featureAccess[PremiumFeature::AI_PORTFOLIO_ANALYZER] = isPremium;
    m_featureAccess[PremiumFeature::NFT_FLOOR_PRICE_ALERTS] = isPremium;
    m_featureAccess[PremiumFeature::USAGE_DASHBOARD] = isPremium;
    m_featureAccess[PremiumFeature::EMAIL_CONFIRMATION_TOGGLE] = isPremium;
    
    // Emit signals for each feature
    for (auto it = m_featureAccess.begin(); it != m_featureAccess.end(); ++it) {
        emit featureAccessChanged(it.key(), it.value());
    }
}

// PremiumPurchaseDialog Implementation

PremiumPurchaseDialog::PremiumPurchaseDialog(PremiumSubscriptionManager* manager, QWidget* parent)
    : QDialog(parent)
    , m_subscriptionManager(manager)
    , m_selectedPlan(PremiumPlan::MONTHLY)
    , m_selectedPaymentMethod(PaymentMethod::SHAH_NATIVE)
{
    setWindowTitle(tr("Upgrade to Shahcoin Premium"));
    setModal(true);
    resize(600, 500);
    
    setupUI();
    updatePricing();
    
    // Connect to subscription manager signals
    connect(m_subscriptionManager, &PremiumSubscriptionManager::paymentProcessed,
            this, [this](bool success, const QString& message) {
                if (success) {
                    QMessageBox::information(this, tr("Success"), message);
                    accept();
                } else {
                    QMessageBox::warning(this, tr("Payment Failed"), message);
                }
            });
}

void PremiumPurchaseDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    
    // Plan Selection Tab
    m_planTab = new QWidget();
    m_planLayout = new QVBoxLayout(m_planTab);
    
    // Header
    QLabel* headerLabel = new QLabel(tr("🌟 Choose Your Premium Plan"));
    headerLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 20px;");
    headerLabel->setAlignment(Qt::AlignCenter);
    m_planLayout->addWidget(headerLabel);
    
    // Plan selection group
    m_planGroup = new QGroupBox(tr("Subscription Plans"));
    QVBoxLayout* planGroupLayout = new QVBoxLayout(m_planGroup);
    
    m_planCombo = new QComboBox();
    m_planCombo->addItem(tr("Monthly Premium - $4.99/month"), static_cast<int>(PremiumPlan::MONTHLY));
    m_planCombo->addItem(tr("Yearly Premium - $49.99/year (Save 17%)"), static_cast<int>(PremiumPlan::YEARLY));
    planGroupLayout->addWidget(m_planCombo);
    
    m_planDescription = new QLabel();
    m_planDescription->setWordWrap(true);
    planGroupLayout->addWidget(m_planDescription);
    
    m_planFeatures = new QLabel();
    m_planFeatures->setWordWrap(true);
    planGroupLayout->addWidget(m_planFeatures);
    
    m_planPrice = new QLabel();
    m_planPrice->setStyleSheet("font-size: 16px; font-weight: bold; color: #2E8B57;");
    planGroupLayout->addWidget(m_planPrice);
    
    m_autoRenewCheck = new QCheckBox(tr("Enable automatic renewal"));
    m_autoRenewCheck->setChecked(true);
    planGroupLayout->addWidget(m_autoRenewCheck);
    
    m_planLayout->addWidget(m_planGroup);
    m_planLayout->addStretch();
    
    m_tabWidget->addTab(m_planTab, tr("📋 Plan"));
    
    // Payment Method Tab
    m_paymentTab = new QWidget();
    m_paymentLayout = new QVBoxLayout(m_paymentTab);
    
    QLabel* paymentHeaderLabel = new QLabel(tr("💳 Choose Payment Method"));
    paymentHeaderLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 20px;");
    paymentHeaderLabel->setAlignment(Qt::AlignCenter);
    m_paymentLayout->addWidget(paymentHeaderLabel);
    
    m_paymentGroup = new QGroupBox(tr("Payment Options"));
    QVBoxLayout* paymentGroupLayout = new QVBoxLayout(m_paymentGroup);
    
    m_paymentMethodCombo = new QComboBox();
    m_paymentMethodCombo->addItem(tr("💰 SHAH (Native)"), static_cast<int>(PaymentMethod::SHAH_NATIVE));
    m_paymentMethodCombo->addItem(tr("🪙 SHAHtoken (ERC-20)"), static_cast<int>(PaymentMethod::SHAH_TOKEN));
    m_paymentMethodCombo->addItem(tr("💳 Credit/Debit Card (Stripe)"), static_cast<int>(PaymentMethod::STRIPE_CARD));
    paymentGroupLayout->addWidget(m_paymentMethodCombo);
    
    m_paymentDescription = new QLabel();
    m_paymentDescription->setWordWrap(true);
    paymentGroupLayout->addWidget(m_paymentDescription);
    
    // Email for receipts (required for Stripe)
    QLabel* emailLabel = new QLabel(tr("Email Address (for receipts):"));
    paymentGroupLayout->addWidget(emailLabel);
    
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText(tr("Enter your email address"));
    paymentGroupLayout->addWidget(m_emailEdit);
    
    m_emailReceiptsCheck = new QCheckBox(tr("Send email confirmations for transactions"));
    m_emailReceiptsCheck->setChecked(true);
    paymentGroupLayout->addWidget(m_emailReceiptsCheck);
    
    m_paymentLayout->addWidget(m_paymentGroup);
    m_paymentLayout->addStretch();
    
    m_tabWidget->addTab(m_paymentTab, tr("💳 Payment"));
    
    // Summary Tab
    m_summaryTab = new QWidget();
    m_summaryLayout = new QVBoxLayout(m_summaryTab);
    
    QLabel* summaryHeaderLabel = new QLabel(tr("📄 Order Summary"));
    summaryHeaderLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 20px;");
    summaryHeaderLabel->setAlignment(Qt::AlignCenter);
    m_summaryLayout->addWidget(summaryHeaderLabel);
    
    m_summaryDetails = new QLabel();
    m_summaryDetails->setWordWrap(true);
    m_summaryDetails->setStyleSheet("background-color: #f5f5f5; padding: 15px; border-radius: 5px;");
    m_summaryLayout->addWidget(m_summaryDetails);
    
    m_totalAmount = new QLabel();
    m_totalAmount->setStyleSheet("font-size: 18px; font-weight: bold; color: #2E8B57; text-align: center; margin: 20px;");
    m_totalAmount->setAlignment(Qt::AlignCenter);
    m_summaryLayout->addWidget(m_totalAmount);
    
    // Action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_purchaseButton = new QPushButton(tr("🚀 Purchase Premium"));
    m_purchaseButton->setStyleSheet("background-color: #4CAF50; color: white; padding: 10px; font-weight: bold;");
    m_cancelButton = new QPushButton(tr("Cancel"));
    
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_purchaseButton);
    
    m_summaryLayout->addLayout(buttonLayout);
    
    m_tabWidget->addTab(m_summaryTab, tr("📄 Summary"));
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Connect signals
    connect(m_planCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PremiumPurchaseDialog::onPlanChanged);
    connect(m_paymentMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PremiumPurchaseDialog::onPaymentMethodChanged);
    connect(m_purchaseButton, &QPushButton::clicked, this, &PremiumPurchaseDialog::onPurchaseClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &PremiumPurchaseDialog::onCancelClicked);
    
    // Initial updates
    onPlanChanged();
    onPaymentMethodChanged();
}

void PremiumPurchaseDialog::onPlanChanged()
{
    int planIndex = m_planCombo->currentData().toInt();
    m_selectedPlan = static_cast<PremiumPlan>(planIndex);
    
    updatePricing();
    updatePlanComparison();
}

void PremiumPurchaseDialog::onPaymentMethodChanged()
{
    int methodIndex = m_paymentMethodCombo->currentData().toInt();
    m_selectedPaymentMethod = static_cast<PaymentMethod>(methodIndex);
    
    updatePricing();
    
    // Show/hide email field based on payment method
    bool requireEmail = (m_selectedPaymentMethod == PaymentMethod::STRIPE_CARD);
    m_emailEdit->setVisible(requireEmail);
    m_emailEdit->parentWidget()->layout()->itemAt(3)->widget()->setVisible(requireEmail); // Email label
    
    // Update payment description
    QString description;
    switch (m_selectedPaymentMethod) {
        case PaymentMethod::SHAH_NATIVE:
            description = tr("Pay with native SHAH from your wallet. Transaction will be processed on the Shahcoin blockchain.");
            break;
        case PaymentMethod::SHAH_TOKEN:
            description = tr("Pay with SHAHtoken (ERC-20). Transaction will be processed on Ethereum network.");
            break;
        case PaymentMethod::STRIPE_CARD:
            description = tr("Pay with credit/debit card through Stripe. USD will be converted to SHAH automatically.");
            break;
    }
    m_paymentDescription->setText(description);
}

void PremiumPurchaseDialog::onPurchaseClicked()
{
    // Validate inputs
    if (m_selectedPaymentMethod == PaymentMethod::STRIPE_CARD && m_emailEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Email Required"), tr("Please enter your email address for Stripe payments."));
        return;
    }
    
    processPayment();
}

void PremiumPurchaseDialog::onCancelClicked()
{
    reject();
}

void PremiumPurchaseDialog::onPriceUpdated()
{
    updatePricing();
}

void PremiumPurchaseDialog::updatePricing()
{
    double priceUSD = m_subscriptionManager->getPlanPriceUSD(m_selectedPlan);
    qint64 priceSatoshis = m_subscriptionManager->getPlanPriceSatoshis(m_selectedPlan);
    
    QString priceText;
    if (m_selectedPaymentMethod == PaymentMethod::STRIPE_CARD) {
        priceText = tr("$%1 USD").arg(priceUSD, 0, 'f', 2);
    } else {
        double shahAmount = priceSatoshis / 100000000.0;
        priceText = tr("%1 SHAH (~$%2 USD)").arg(shahAmount, 0, 'f', 8).arg(priceUSD, 0, 'f', 2);
    }
    
    m_planPrice->setText(tr("Price: %1").arg(priceText));
    m_totalAmount->setText(tr("Total: %1").arg(priceText));
    
    // Update summary
    QString planName = m_subscriptionManager->getPlanDisplayName(m_selectedPlan);
    QString paymentMethodName = m_paymentMethodCombo->currentText();
    
    QString summary = tr("Plan: %1\n").arg(planName);
    summary += tr("Payment Method: %1\n").arg(paymentMethodName);
    summary += tr("Auto-Renewal: %1\n").arg(m_autoRenewCheck->isChecked() ? tr("Enabled") : tr("Disabled"));
    
    if (!m_emailEdit->text().isEmpty()) {
        summary += tr("Email: %1\n").arg(m_emailEdit->text());
    }
    
    m_summaryDetails->setText(summary);
}

void PremiumPurchaseDialog::updatePlanComparison()
{
    QStringList features = m_subscriptionManager->getPlanFeatures(m_selectedPlan);
    QString featuresText = tr("Included Features:\n");
    for (const QString& feature : features) {
        featuresText += tr("✓ %1\n").arg(feature);
    }
    
    m_planFeatures->setText(featuresText);
    
    // Update description
    QString description;
    if (m_selectedPlan == PremiumPlan::MONTHLY) {
        description = tr("Perfect for trying out premium features. Billed monthly.");
    } else if (m_selectedPlan == PremiumPlan::YEARLY) {
        description = tr("Best value! Save 17% compared to monthly billing. Billed annually.");
    }
    
    m_planDescription->setText(description);
}

void PremiumPurchaseDialog::processPayment()
{
    QString email = m_emailEdit->text();
    
    // Show processing dialog
    QProgressDialog* progressDialog = new QProgressDialog(tr("Processing payment..."), tr("Cancel"), 0, 0, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->show();
    
    // Process payment
    bool success = m_subscriptionManager->initiatePurchase(m_selectedPlan, m_selectedPaymentMethod, email);
    
    progressDialog->close();
    progressDialog->deleteLater();
    
    if (!success) {
        QMessageBox::warning(this, tr("Payment Failed"), tr("Failed to process payment. Please try again."));
    }
}

// PremiumStatusWidget Implementation

PremiumStatusWidget::PremiumStatusWidget(PremiumSubscriptionManager* manager, QWidget* parent)
    : QWidget(parent)
    , m_subscriptionManager(manager)
{
    setupUI();
    updateStatus();
    
    // Connect to subscription manager signals
    connect(m_subscriptionManager, &PremiumSubscriptionManager::subscriptionStatusChanged,
            this, &PremiumStatusWidget::updateStatus);
}

void PremiumStatusWidget::setupUI()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(5);
    
    m_statusIcon = new QLabel();
    m_statusIcon->setFixedSize(24, 24);
    m_layout->addWidget(m_statusIcon);
    
    m_statusText = new QLabel();
    m_layout->addWidget(m_statusText);
    
    m_layout->addStretch();
    
    m_upgradeButton = new QPushButton(tr("Go Premium"));
    m_upgradeButton->setStyleSheet("background-color: #FF6B35; color: white; padding: 5px 15px; border-radius: 3px; font-weight: bold;");
    m_layout->addWidget(m_upgradeButton);
    
    m_manageButton = new QPushButton(tr("Manage"));
    m_manageButton->setStyleSheet("background-color: #4CAF50; color: white; padding: 5px 15px; border-radius: 3px;");
    m_layout->addWidget(m_manageButton);
    
    // Connect signals
    connect(m_upgradeButton, &QPushButton::clicked, this, &PremiumStatusWidget::onUpgradeClicked);
    connect(m_manageButton, &QPushButton::clicked, this, &PremiumStatusWidget::onManageSubscriptionClicked);
}

void PremiumStatusWidget::updateStatus()
{
    bool isPremium = m_subscriptionManager->isPremiumActive();
    
    if (isPremium) {
        m_statusIcon->setText("👑");
        QString planName = m_subscriptionManager->getPlanDisplayName(m_subscriptionManager->getCurrentPlan());
        int daysRemaining = m_subscriptionManager->getDaysRemaining();
        m_statusText->setText(tr("%1 (%2 days remaining)").arg(planName).arg(daysRemaining));
        m_upgradeButton->setVisible(false);
        m_manageButton->setVisible(true);
    } else {
        m_statusIcon->setText("🆓");
        m_statusText->setText(tr("Free Plan"));
        m_upgradeButton->setVisible(true);
        m_manageButton->setVisible(false);
    }
}

void PremiumStatusWidget::onUpgradeClicked()
{
    PremiumPurchaseDialog dialog(m_subscriptionManager, this);
    dialog.exec();
}

void PremiumStatusWidget::onManageSubscriptionClicked()
{
    SubscriptionManagementDialog dialog(m_subscriptionManager, this);
    dialog.exec();
}

// SubscriptionManagementDialog Implementation

SubscriptionManagementDialog::SubscriptionManagementDialog(PremiumSubscriptionManager* manager, QWidget* parent)
    : QDialog(parent)
    , m_subscriptionManager(manager)
{
    setWindowTitle(tr("Manage Subscription"));
    setModal(true);
    resize(500, 400);
    
    setupUI();
    updateSubscriptionInfo();
    populatePaymentHistory();
}

void SubscriptionManagementDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget(this);
    
    // Subscription Info Tab
    m_infoTab = new QWidget();
    QVBoxLayout* infoLayout = new QVBoxLayout(m_infoTab);
    
    QLabel* headerLabel = new QLabel(tr("👑 Subscription Information"));
    headerLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 20px;");
    infoLayout->addWidget(headerLabel);
    
    m_planLabel = new QLabel();
    m_statusLabel = new QLabel();
    m_expirationLabel = new QLabel();
    m_renewalLabel = new QLabel();
    
    infoLayout->addWidget(m_planLabel);
    infoLayout->addWidget(m_statusLabel);
    infoLayout->addWidget(m_expirationLabel);
    infoLayout->addWidget(m_renewalLabel);
    
    infoLayout->addStretch();
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_renewButton = new QPushButton(tr("Renew Now"));
    m_renewButton->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px;");
    m_upgradeButton = new QPushButton(tr("Upgrade Plan"));
    m_upgradeButton->setStyleSheet("background-color: #2196F3; color: white; padding: 8px;");
    m_cancelButton = new QPushButton(tr("Cancel Subscription"));
    m_cancelButton->setStyleSheet("background-color: #f44336; color: white; padding: 8px;");
    
    buttonLayout->addWidget(m_renewButton);
    buttonLayout->addWidget(m_upgradeButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    
    infoLayout->addLayout(buttonLayout);
    
    m_tabWidget->addTab(m_infoTab, tr("📋 Info"));
    
    // Payment History Tab
    m_historyTab = new QWidget();
    QVBoxLayout* historyLayout = new QVBoxLayout(m_historyTab);
    
    QLabel* historyHeaderLabel = new QLabel(tr("💳 Payment History"));
    historyHeaderLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 20px;");
    historyLayout->addWidget(historyHeaderLabel);
    
    m_historyTable = new QTableWidget();
    m_historyTable->setColumnCount(5);
    m_historyTable->setHorizontalHeaderLabels({tr("Date"), tr("Plan"), tr("Amount"), tr("Method"), tr("Status")});
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    historyLayout->addWidget(m_historyTable);
    
    m_exportButton = new QPushButton(tr("Export History"));
    historyLayout->addWidget(m_exportButton);
    
    m_tabWidget->addTab(m_historyTab, tr("📊 History"));
    
    // Settings Tab
    m_settingsTab = new QWidget();
    QVBoxLayout* settingsLayout = new QVBoxLayout(m_settingsTab);
    
    QLabel* settingsHeaderLabel = new QLabel(tr("⚙️ Subscription Settings"));
    settingsHeaderLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 20px;");
    settingsLayout->addWidget(settingsHeaderLabel);
    
    m_autoRenewCheck = new QCheckBox(tr("Enable automatic renewal"));
    m_emailConfirmationCheck = new QCheckBox(tr("Send email confirmations"));
    
    settingsLayout->addWidget(m_autoRenewCheck);
    settingsLayout->addWidget(m_emailConfirmationCheck);
    
    QLabel* emailLabel = new QLabel(tr("Email Address:"));
    m_emailEdit = new QLineEdit();
    settingsLayout->addWidget(emailLabel);
    settingsLayout->addWidget(m_emailEdit);
    
    settingsLayout->addStretch();
    
    m_saveSettingsButton = new QPushButton(tr("Save Settings"));
    m_saveSettingsButton->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px;");
    settingsLayout->addWidget(m_saveSettingsButton);
    
    m_tabWidget->addTab(m_settingsTab, tr("⚙️ Settings"));
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Connect signals
    connect(m_renewButton, &QPushButton::clicked, this, &SubscriptionManagementDialog::onRenewClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SubscriptionManagementDialog::onCancelClicked);
    connect(m_upgradeButton, &QPushButton::clicked, this, &SubscriptionManagementDialog::onUpgradeClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &SubscriptionManagementDialog::onPaymentHistoryClicked);
    connect(m_saveSettingsButton, &QPushButton::clicked, this, &SubscriptionManagementDialog::onEmailSettingsChanged);
}

void SubscriptionManagementDialog::updateSubscriptionInfo()
{
    SubscriptionData subscription = m_subscriptionManager->getCurrentSubscription();
    
    QString planName = m_subscriptionManager->getPlanDisplayName(subscription.plan);
    QString status = subscription.isActive ? tr("Active") : tr("Inactive");
    
    m_planLabel->setText(tr("Plan: %1").arg(planName));
    m_statusLabel->setText(tr("Status: %1").arg(status));
    m_expirationLabel->setText(tr("Expires: %1").arg(subscription.expirationDate.toString()));
    m_renewalLabel->setText(tr("Auto-Renewal: %1").arg(subscription.autoRenew ? tr("Enabled") : tr("Disabled")));
    
    // Update settings
    m_autoRenewCheck->setChecked(subscription.autoRenew);
    m_emailConfirmationCheck->setChecked(subscription.emailConfirmationEnabled);
    m_emailEdit->setText(subscription.emailAddress);
}

void SubscriptionManagementDialog::populatePaymentHistory()
{
    QList<PaymentTransaction> history = m_subscriptionManager->getPaymentHistory();
    
    m_historyTable->setRowCount(history.size());
    
    for (int i = 0; i < history.size(); ++i) {
        const PaymentTransaction& transaction = history[i];
        
        m_historyTable->setItem(i, 0, new QTableWidgetItem(transaction.timestamp.toString("yyyy-MM-dd")));
        m_historyTable->setItem(i, 1, new QTableWidgetItem(m_subscriptionManager->getPlanDisplayName(transaction.plan)));
        m_historyTable->setItem(i, 2, new QTableWidgetItem(tr("$%1").arg(transaction.amountUSD, 0, 'f', 2)));
        
        QString methodText;
        switch (transaction.paymentMethod) {
            case PaymentMethod::SHAH_NATIVE: methodText = tr("SHAH"); break;
            case PaymentMethod::SHAH_TOKEN: methodText = tr("SHAHtoken"); break;
            case PaymentMethod::STRIPE_CARD: methodText = tr("Credit Card"); break;
        }
        m_historyTable->setItem(i, 3, new QTableWidgetItem(methodText));
        m_historyTable->setItem(i, 4, new QTableWidgetItem(transaction.status));
    }
}

void SubscriptionManagementDialog::onRenewClicked()
{
    bool success = m_subscriptionManager->renewSubscription();
    if (success) {
        QMessageBox::information(this, tr("Success"), tr("Subscription renewed successfully!"));
        updateSubscriptionInfo();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to renew subscription. Please try again."));
    }
}

void SubscriptionManagementDialog::onCancelClicked()
{
    int result = QMessageBox::question(this, tr("Cancel Subscription"), 
                                     tr("Are you sure you want to cancel your subscription? You will lose access to premium features."),
                                     QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        bool success = m_subscriptionManager->cancelSubscription();
        if (success) {
            QMessageBox::information(this, tr("Success"), tr("Subscription cancelled successfully."));
            updateSubscriptionInfo();
        }
    }
}

void SubscriptionManagementDialog::onUpgradeClicked()
{
    PremiumPurchaseDialog dialog(m_subscriptionManager, this);
    dialog.exec();
    updateSubscriptionInfo();
}

void SubscriptionManagementDialog::onPaymentHistoryClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Payment History"), 
                                                   "payment_history.csv", tr("CSV Files (*.csv)"));
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "Date,Plan,Amount USD,Payment Method,Status\n";
            
            QList<PaymentTransaction> history = m_subscriptionManager->getPaymentHistory();
            for (const PaymentTransaction& transaction : history) {
                QString methodText;
                switch (transaction.paymentMethod) {
                    case PaymentMethod::SHAH_NATIVE: methodText = "SHAH"; break;
                    case PaymentMethod::SHAH_TOKEN: methodText = "SHAHtoken"; break;
                    case PaymentMethod::STRIPE_CARD: methodText = "Credit Card"; break;
                }
                
                stream << transaction.timestamp.toString("yyyy-MM-dd") << ","
                       << m_subscriptionManager->getPlanDisplayName(transaction.plan) << ","
                       << transaction.amountUSD << ","
                       << methodText << ","
                       << transaction.status << "\n";
            }
            
            QMessageBox::information(this, tr("Success"), tr("Payment history exported successfully!"));
        }
    }
}

void SubscriptionManagementDialog::onEmailSettingsChanged()
{
    // Save settings
    SubscriptionData subscription = m_subscriptionManager->getCurrentSubscription();
    subscription.autoRenew = m_autoRenewCheck->isChecked();
    subscription.emailConfirmationEnabled = m_emailConfirmationCheck->isChecked();
    subscription.emailAddress = m_emailEdit->text();
    
    m_subscriptionManager->activateSubscription(subscription);
    
    QMessageBox::information(this, tr("Success"), tr("Settings saved successfully!"));
}

// FeatureGateWidget Implementation

FeatureGateWidget::FeatureGateWidget(PremiumFeature feature, PremiumSubscriptionManager* manager, QWidget* parent)
    : QWidget(parent)
    , m_feature(feature)
    , m_subscriptionManager(manager)
    , m_gatedWidget(nullptr)
{
    setupUI();
    updateGateStatus();
    
    // Connect to subscription manager signals
    connect(m_subscriptionManager, &PremiumSubscriptionManager::featureAccessChanged,
            this, [this](PremiumFeature feature, bool unlocked) {
                if (feature == m_feature) {
                    onFeatureAccessChanged();
                }
            });
}

void FeatureGateWidget::setGatedWidget(QWidget* widget)
{
    m_gatedWidget = widget;
    if (widget) {
        m_layout->addWidget(widget);
    }
    updateGateStatus();
}

bool FeatureGateWidget::isFeatureUnlocked() const
{
    return m_subscriptionManager->isFeatureUnlocked(m_feature);
}

void FeatureGateWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    
    // Create lock overlay
    m_lockOverlay = new QWidget();
    m_lockOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.8); border-radius: 5px;");
    
    QVBoxLayout* overlayLayout = new QVBoxLayout(m_lockOverlay);
    overlayLayout->setAlignment(Qt::AlignCenter);
    
    m_lockIcon = new QLabel("🔒");
    m_lockIcon->setStyleSheet("font-size: 48px; color: white;");
    m_lockIcon->setAlignment(Qt::AlignCenter);
    overlayLayout->addWidget(m_lockIcon);
    
    m_lockMessage = new QLabel(tr("Premium Feature"));
    m_lockMessage->setStyleSheet("color: white; font-weight: bold; font-size: 14px;");
    m_lockMessage->setAlignment(Qt::AlignCenter);
    overlayLayout->addWidget(m_lockMessage);
    
    m_upgradeButton = new QPushButton(tr("🚀 Go Premium"));
    m_upgradeButton->setStyleSheet("background-color: #FF6B35; color: white; padding: 8px 16px; border-radius: 4px; font-weight: bold;");
    overlayLayout->addWidget(m_upgradeButton);
    
    m_layout->addWidget(m_lockOverlay);
    
    // Connect upgrade button
    connect(m_upgradeButton, &QPushButton::clicked, this, &FeatureGateWidget::onUpgradeClicked);
}

void FeatureGateWidget::updateGateStatus()
{
    bool unlocked = isFeatureUnlocked();
    
    m_lockOverlay->setVisible(!unlocked);
    
    if (m_gatedWidget) {
        m_gatedWidget->setEnabled(unlocked);
    }
}

void FeatureGateWidget::onFeatureAccessChanged()
{
    updateGateStatus();
}

void FeatureGateWidget::onUpgradeClicked()
{
    PremiumPurchaseDialog dialog(m_subscriptionManager, this);
    dialog.exec();
}

#include "premiumsubscription.moc"
