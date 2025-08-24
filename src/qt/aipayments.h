#ifndef SHAHCOIN_QT_AIPAYMENTS_H
#define SHAHCOIN_QT_AIPAYMENTS_H

#include <QWidget>
#include <QDateTime>
#include <QVector>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class PaymentHistoryModel : public QObject
{
    Q_OBJECT
public:
    struct PaymentRecord {
        QDateTime date;
        double amount;          // numeric amount
        QString currency;       // "SHAH", "SHAHtoken", "USD"
        QString method;         // "SHAH", "SHAHtoken", "Credit Card"
        QString transactionId;  // tx hash or Stripe invoice ID
        QString invoiceUrl;     // optional external invoice URL (Stripe)
        QString planType;       // "Monthly", "One-Time", "Trial"
    };

    explicit PaymentHistoryModel(QObject* parent = nullptr);

    const QVector<PaymentRecord>& records() const { return m_records; }
    void setRecords(const QVector<PaymentRecord>& r);
    void addRecord(const PaymentRecord& r);
    void clear();
    int size() const { return m_records.size(); }

    bool save();
    bool load();
    bool exportCsv(const QString& filePath) const;

    static QString storageFilePath();

private:
    QVector<PaymentRecord> m_records;
};

class PaymentHistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PaymentHistoryWidget(QWidget* parent = nullptr);

    void refresh();
    void addRecord(const PaymentHistoryModel::PaymentRecord& r);
    int recordCount() const;

private Q_SLOTS:
    void onExportCsv();
    void onClearLocal();
    void onViewInvoiceClicked();

private:
    void setupUI();
    void populateTable();
    void showInvoiceDialog(const PaymentHistoryModel::PaymentRecord& r);

    PaymentHistoryModel m_model;
    QVBoxLayout* m_mainLayout{nullptr};
    QTableWidget* m_table{nullptr};
    QPushButton* m_exportButton{nullptr};
    QPushButton* m_clearButton{nullptr};
    QLabel* m_hintLabel{nullptr};
};

#endif // SHAHCOIN_QT_AIPAYMENTS_H


