#include "fiatvaluedisplay.h"
#include "walletmodel.h"
#include "shahcoinunits.h"
#include "guiutil.h"

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QDebug>
#include <QRandomGenerator>

FiatValueDisplay::FiatValueDisplay(QObject* parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_priceUpdateTimer(nullptr)
    , m_shahPrice(MOCK_SHAH_PRICE)
    , m_currentBalance(0)
    , m_fiatEnabled(false)
    , m_priceUpdateInProgress(false)
{
    setupPriceUpdate();
    loadSettings();
}

FiatValueDisplay::~FiatValueDisplay()
{
    saveSettings();
    if (m_priceUpdateTimer) {
        m_priceUpdateTimer->stop();
    }
}

void FiatValueDisplay::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    if (m_walletModel) {
        connect(m_walletModel, &WalletModel::balanceChanged, this, &FiatValueDisplay::onBalanceChanged);
        onBalanceChanged();
    }
}

void FiatValueDisplay::setBalanceLabel(QLabel* balanceLabel)
{
    m_balanceLabel = balanceLabel;
    updateDisplay();
}

void FiatValueDisplay::setFiatToggle(QCheckBox* fiatToggle)
{
    m_fiatToggle = fiatToggle;
    if (m_fiatToggle) {
        m_fiatToggle->setChecked(m_fiatEnabled);
        connect(m_fiatToggle, &QCheckBox::toggled, this, &FiatValueDisplay::onFiatToggleChanged);
    }
}

QString FiatValueDisplay::getFormattedBalance() const
{
    return formatShahAmount(m_currentBalance);
}

QString FiatValueDisplay::getFormattedFiatValue() const
{
    double fiatValue = calculateFiatValue(m_currentBalance);
    return formatFiatAmount(fiatValue);
}

void FiatValueDisplay::onFiatToggleChanged(bool enabled)
{
    m_fiatEnabled = enabled;
    saveSettings();
    updateDisplay();
    emit displayFormatChanged();
}

void FiatValueDisplay::onBalanceChanged()
{
    if (m_walletModel) {
        m_currentBalance = m_walletModel->getBalance();
        updateDisplay();
    }
}

void FiatValueDisplay::onPriceUpdated()
{
    updateDisplay();
    emit priceUpdated(m_shahPrice);
    
    double fiatValue = calculateFiatValue(m_currentBalance);
    emit fiatValueChanged(formatFiatAmount(fiatValue));
}

void FiatValueDisplay::refreshPrice()
{
    if (!m_priceUpdateInProgress) {
        fetchPriceFromAPI();
    }
}

void FiatValueDisplay::onNetworkReplyFinished(QNetworkReply* reply)
{
    m_priceUpdateInProgress = false;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            
            // Try different API response formats
            if (obj.contains("price")) {
                m_shahPrice = obj["price"].toDouble();
            } else if (obj.contains("data") && obj["data"].isObject()) {
                QJsonObject dataObj = obj["data"].toObject();
                if (dataObj.contains("price")) {
                    m_shahPrice = dataObj["price"].toDouble();
                }
            } else if (obj.contains("shahcoin") && obj["shahcoin"].isObject()) {
                QJsonObject shahObj = obj["shahcoin"].toObject();
                if (shahObj.contains("usd")) {
                    m_shahPrice = shahObj["usd"].toDouble();
                }
            }
            
            if (m_shahPrice > 0) {
                onPriceUpdated();
            }
        }
    } else {
        qDebug() << "Price API error:" << reply->errorString();
        // Fallback to mock price
        updatePriceFromMock();
    }
    
    reply->deleteLater();
}

void FiatValueDisplay::onPriceUpdateTimeout()
{
    if (!m_priceUpdateInProgress) {
        updatePriceFromMock();
    }
}

void FiatValueDisplay::setupPriceUpdate()
{
    // Setup network manager
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &FiatValueDisplay::onNetworkReplyFinished);
    
    // Setup price update timer
    m_priceUpdateTimer = new QTimer(this);
    connect(m_priceUpdateTimer, &QTimer::timeout, this, &FiatValueDisplay::onPriceUpdateTimeout);
    m_priceUpdateTimer->start(PRICE_UPDATE_INTERVAL_MS);
    
    // Initial price fetch
    fetchPriceFromAPI();
}

void FiatValueDisplay::loadSettings()
{
    QSettings settings;
    m_fiatEnabled = settings.value("fiat/displayEnabled", false).toBool();
    m_shahPrice = settings.value("fiat/shahPrice", MOCK_SHAH_PRICE).toDouble();
}

void FiatValueDisplay::saveSettings()
{
    QSettings settings;
    settings.setValue("fiat/displayEnabled", m_fiatEnabled);
    settings.setValue("fiat/shahPrice", m_shahPrice);
}

void FiatValueDisplay::updateDisplay()
{
    if (m_balanceLabel) {
        QString displayText = formatShahAmount(m_currentBalance);
        
        if (m_fiatEnabled && m_shahPrice > 0) {
            double fiatValue = calculateFiatValue(m_currentBalance);
            displayText += QString(" (≈ $%1)").arg(formatFiatAmount(fiatValue));
        }
        
        m_balanceLabel->setText(displayText);
    }
}

void FiatValueDisplay::fetchPriceFromAPI()
{
    m_priceUpdateInProgress = true;
    
    // Try multiple price sources
    QList<QUrl> priceUrls = {
        QUrl("https://api.coingecko.com/api/v3/simple/price?ids=shahcoin&vs_currencies=usd"),
        QUrl("https://api.shahswap.com/v1/price/shah/usd"),
        QUrl("https://api.example.com/shahcoin/price") // Placeholder
    };
    
    for (const QUrl& url : priceUrls) {
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setHeader(QNetworkRequest::UserAgentHeader, "Shahcoin-Wallet/1.0");
        
        QNetworkReply* reply = m_networkManager->get(request);
        connect(reply, &QNetworkReply::errorOccurred, [this, reply](QNetworkReply::NetworkError error) {
            if (error != QNetworkReply::NoError) {
                m_priceUpdateInProgress = false;
                updatePriceFromMock();
                reply->deleteLater();
            }
        });
    }
}

void FiatValueDisplay::updatePriceFromMock()
{
    // Simulate price volatility for demo purposes
    double volatility = (QRandomGenerator::global()->bounded(100) - 50) / 1000.0; // ±5%
    m_shahPrice = MOCK_SHAH_PRICE * (1.0 + volatility);
    
    if (m_shahPrice < 0.01) m_shahPrice = 0.01; // Minimum price
    
    onPriceUpdated();
}

QString FiatValueDisplay::formatShahAmount(qint64 amount) const
{
    return ShahcoinUnits::format(ShahcoinUnits::SHAH, amount, false, ShahcoinUnits::separatorAlways);
}

QString FiatValueDisplay::formatFiatAmount(double amount) const
{
    if (amount >= 1000000) {
        return QString::number(amount / 1000000.0, 'f', 2) + "M";
    } else if (amount >= 1000) {
        return QString::number(amount / 1000.0, 'f', 2) + "K";
    } else {
        return QString::number(amount, 'f', 2);
    }
}

double FiatValueDisplay::calculateFiatValue(qint64 shahAmount) const
{
    return (shahAmount / 100000000.0) * m_shahPrice; // Convert from satoshis
}

// FiatBalanceLabel implementation
FiatBalanceLabel::FiatBalanceLabel(QWidget* parent)
    : QLabel(parent)
    , m_shahAmount(0)
    , m_shahPrice(0.0)
    , m_fiatEnabled(false)
{
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setStyleSheet("QLabel { color: #2E86AB; font-weight: bold; }");
}

void FiatBalanceLabel::setShahAmount(qint64 amount)
{
    m_shahAmount = amount;
    updateDisplay();
}

void FiatBalanceLabel::setFiatEnabled(bool enabled)
{
    m_fiatEnabled = enabled;
    updateDisplay();
}

void FiatBalanceLabel::setShahPrice(double price)
{
    m_shahPrice = price;
    updateDisplay();
}

void FiatBalanceLabel::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);
}

void FiatBalanceLabel::updateDisplay()
{
    m_shahFormatted = formatShahAmount();
    m_fiatFormatted = formatFiatAmount();
    
    QString displayText = m_shahFormatted;
    if (m_fiatEnabled && m_shahPrice > 0) {
        displayText += QString(" (≈ $%1)").arg(m_fiatFormatted);
    }
    
    setText(displayText);
}

QString FiatBalanceLabel::formatShahAmount() const
{
    return ShahcoinUnits::format(ShahcoinUnits::SHAH, m_shahAmount, false, ShahcoinUnits::separatorAlways);
}

QString FiatBalanceLabel::formatFiatAmount() const
{
    double fiatValue = (m_shahAmount / 100000000.0) * m_shahPrice;
    
    if (fiatValue >= 1000000) {
        return QString::number(fiatValue / 1000000.0, 'f', 2) + "M";
    } else if (fiatValue >= 1000) {
        return QString::number(fiatValue / 1000.0, 'f', 2) + "K";
    } else {
        return QString::number(fiatValue, 'f', 2);
    }
}

// FiatValueToggle implementation
FiatValueToggle::FiatValueToggle(QWidget* parent)
    : QCheckBox(parent)
{
    setupUI();
    loadSettings();
    connect(this, &QCheckBox::toggled, this, &FiatValueToggle::onToggleChanged);
}

void FiatValueToggle::onToggleChanged(bool checked)
{
    saveSettings();
    emit fiatDisplayToggled(checked);
}

void FiatValueToggle::setupUI()
{
    setText(tr("Show SHAH in USD"));
    setToolTip(tr("Toggle to display wallet balance in both SHAH and USD"));
    setStyleSheet(
        "QCheckBox { color: #666666; font-size: 12px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; }"
        "QCheckBox::indicator:unchecked { border: 2px solid #CCCCCC; border-radius: 3px; background-color: white; }"
        "QCheckBox::indicator:checked { border: 2px solid #2E86AB; border-radius: 3px; background-color: #2E86AB; }"
        "QCheckBox::indicator:checked::after { content: '✓'; color: white; font-weight: bold; }"
    );
}

void FiatValueToggle::loadSettings()
{
    QSettings settings;
    setChecked(settings.value("fiat/displayEnabled", false).toBool());
}

void FiatValueToggle::saveSettings()
{
    QSettings settings;
    settings.setValue("fiat/displayEnabled", isChecked());
}
