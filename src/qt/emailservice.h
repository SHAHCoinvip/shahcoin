#ifndef SHAHCOIN_QT_EMAILSERVICE_H
#define SHAHCOIN_QT_EMAILSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>

class EmailService : public QObject
{
    Q_OBJECT

public:
    struct PaymentEmailData {
        QString email;
        QString plan;
        QString amount;
        QString method;
        QString date;
        QString transactionId;
    };

    explicit EmailService(QObject* parent = nullptr);
    ~EmailService();

    void sendPaymentConfirmation(const PaymentEmailData& data);
    bool isEmailEnabled() const;
    void setEmailEnabled(bool enabled);

Q_SIGNALS:
    void emailSent(const QString& transactionId);
    void emailFailed(const QString& transactionId, const QString& error);

private Q_SLOTS:
    void onEmailRequestFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_networkManager;
    QString m_webhookUrl; // URL to backend email service
    bool m_emailEnabled;

    void sendWebhookRequest(const QJsonObject& payload);
};

#endif // SHAHCOIN_QT_EMAILSERVICE_H
