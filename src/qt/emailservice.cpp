#include "emailservice.h"
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QDateTime>

EmailService::EmailService(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_webhookUrl("https://api.shahcoin.com/email/payment-confirmation") // Placeholder URL
    , m_emailEnabled(false)
{
    // Load email preference from settings
    QSettings settings;
    m_emailEnabled = settings.value("aiwallet/send_payment_email", false).toBool();

    connect(m_networkManager, &QNetworkAccessManager::finished, this, &EmailService::onEmailRequestFinished);
}

EmailService::~EmailService()
{
}

void EmailService::sendPaymentConfirmation(const PaymentEmailData& data)
{
    if (!m_emailEnabled) {
        return; // Email notifications disabled
    }

    QJsonObject payload;
    payload["email"] = data.email;
    payload["plan"] = data.plan;
    payload["amount"] = data.amount;
    payload["method"] = data.method;
    payload["date"] = data.date;
    payload["tx_id"] = data.transactionId;

    sendWebhookRequest(payload);
}

bool EmailService::isEmailEnabled() const
{
    return m_emailEnabled;
}

void EmailService::setEmailEnabled(bool enabled)
{
    m_emailEnabled = enabled;
    QSettings settings;
    settings.setValue("aiwallet/send_payment_email", enabled);
}

void EmailService::sendWebhookRequest(const QJsonObject& payload)
{
    QNetworkRequest request(QUrl(m_webhookUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(payload);
    QByteArray data = doc.toJson();

    QNetworkReply* reply = m_networkManager->post(request, data);
    reply->setProperty("transactionId", payload["tx_id"].toString());
}

void EmailService::onEmailRequestFinished(QNetworkReply* reply)
{
    QString transactionId = reply->property("transactionId").toString();
    
    if (reply->error() == QNetworkReply::NoError) {
        Q_EMIT emailSent(transactionId);
    } else {
        QString error = reply->errorString();
        Q_EMIT emailFailed(transactionId, error);
    }

    reply->deleteLater();
}
