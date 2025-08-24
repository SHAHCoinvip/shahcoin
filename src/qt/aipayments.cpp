#include "aipayments.h"

#include <QHeaderView>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>

PaymentHistoryModel::PaymentHistoryModel(QObject* parent)
    : QObject(parent)
{
    load();
}

void PaymentHistoryModel::setRecords(const QVector<PaymentRecord>& r)
{
    m_records = r;
}

void PaymentHistoryModel::addRecord(const PaymentRecord& r)
{
    m_records.push_back(r);
    save();
}

void PaymentHistoryModel::clear()
{
    m_records.clear();
    save();
}

bool PaymentHistoryModel::save()
{
    QJsonArray arr;
    for (const auto& r : m_records) {
        QJsonObject o;
        o["date"] = r.date.toSecsSinceEpoch();
        o["amount"] = r.amount;
        o["currency"] = r.currency;
        o["method"] = r.method;
        o["transactionId"] = r.transactionId;
        o["invoiceUrl"] = r.invoiceUrl;
        o["planType"] = r.planType;
        arr.append(o);
    }
    QJsonDocument doc(arr);
    QFile f(storageFilePath());
    QDir().mkpath(QFileInfo(f).dir().absolutePath());
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(doc.toJson(QJsonDocument::Compact));
    return true;
}

bool PaymentHistoryModel::load()
{
    QFile f(storageFilePath());
    if (!f.exists()) return true;
    if (!f.open(QIODevice::ReadOnly)) return false;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray()) return false;
    QJsonArray arr = doc.array();
    m_records.clear();
    for (const auto& v : arr) {
        if (!v.isObject()) continue;
        QJsonObject o = v.toObject();
        PaymentRecord r;
        r.date = QDateTime::fromSecsSinceEpoch(o.value("date").toVariant().toLongLong());
        r.amount = o.value("amount").toDouble();
        r.currency = o.value("currency").toString();
        r.method = o.value("method").toString();
        r.transactionId = o.value("transactionId").toString();
        r.invoiceUrl = o.value("invoiceUrl").toString();
        r.planType = o.value("planType").toString();
        m_records.push_back(r);
    }
    return true;
}

bool PaymentHistoryModel::exportCsv(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "Date,Amount Paid,Payment Method,Currency,Transaction ID,Plan Type,Invoice URL\n";
    for (const auto& r : m_records) {
        out << r.date.toString(Qt::ISODate) << ","
            << QString::number(r.amount, 'f', 4) << ","
            << r.method << ","
            << r.currency << ","
            << r.transactionId << ","
            << r.planType << ","
            << r.invoiceUrl << "\n";
    }
    return true;
}

QString PaymentHistoryModel::storageFilePath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty()) dir = QDir::homePath() + "/.shahcoin";
    return dir + "/aiwallet_payments.json";
}

PaymentHistoryWidget::PaymentHistoryWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    refresh();
}

void PaymentHistoryWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_hintLabel = new QLabel(tr("AI Wallet Payment History"), this);
    m_mainLayout->addWidget(m_hintLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    QStringList headers;
    headers << tr("Date") << tr("Amount Paid") << tr("Payment Method") << tr("Transaction ID") << tr("Invoice") << tr("Plan Type");
    m_table->setHorizontalHeaderLabels(headers);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_mainLayout->addWidget(m_table);

    QHBoxLayout* btns = new QHBoxLayout();
    m_exportButton = new QPushButton(tr("Export CSV"), this);
    m_clearButton = new QPushButton(tr("Clear Local"), this);
    btns->addWidget(m_exportButton);
    btns->addWidget(m_clearButton);
    btns->addStretch();
    m_mainLayout->addLayout(btns);

    connect(m_exportButton, &QPushButton::clicked, this, &PaymentHistoryWidget::onExportCsv);
    connect(m_clearButton, &QPushButton::clicked, this, &PaymentHistoryWidget::onClearLocal);
}

void PaymentHistoryWidget::populateTable()
{
    m_table->setRowCount(0);
    const auto& recs = m_model.records();
    m_table->setRowCount(recs.size());
    for (int i = 0; i < recs.size(); ++i) {
        const auto& r = recs[i];
        m_table->setItem(i, 0, new QTableWidgetItem(r.date.toString("yyyy-MM-dd HH:mm")));
        m_table->setItem(i, 1, new QTableWidgetItem(QString("%1 %2").arg(QString::number(r.amount, 'f', 4), r.currency)));
        m_table->setItem(i, 2, new QTableWidgetItem(r.method));
        m_table->setItem(i, 3, new QTableWidgetItem(r.transactionId));
        QPushButton* viewBtn = new QPushButton(tr("View Invoice"), this);
        viewBtn->setProperty("row", i);
        connect(viewBtn, &QPushButton::clicked, this, &PaymentHistoryWidget::onViewInvoiceClicked);
        m_table->setCellWidget(i, 4, viewBtn);
        m_table->setItem(i, 5, new QTableWidgetItem(r.planType));
    }
}

void PaymentHistoryWidget::refresh()
{
    m_model.load();
    populateTable();
}

void PaymentHistoryWidget::addRecord(const PaymentHistoryModel::PaymentRecord& r)
{
    m_model.addRecord(r);
    refresh();
}

int PaymentHistoryWidget::recordCount() const
{
    return m_model.size();
}

void PaymentHistoryWidget::onExportCsv()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export Payment History"), QDir::homePath() + "/aiwallet_payments.csv", tr("CSV Files (*.csv)"));
    if (filePath.isEmpty()) return;
    if (!m_model.exportCsv(filePath)) {
        QMessageBox::critical(this, tr("Export Failed"), tr("Could not write the CSV file."));
    } else {
        QMessageBox::information(this, tr("Export Successful"), tr("Exported to %1").arg(filePath));
    }
}

void PaymentHistoryWidget::onClearLocal()
{
    if (QMessageBox::question(this, tr("Clear Local History"), tr("Remove local payment records? This does not affect on-chain or Stripe receipts.")) == QMessageBox::Yes) {
        m_model.clear();
        refresh();
    }
}

void PaymentHistoryWidget::onViewInvoiceClicked()
{
    QObject* s = sender();
    bool ok;
    int row = s->property("row").toInt(&ok);
    if (!ok) return;
    if (row < 0 || row >= m_model.records().size()) return;
    showInvoiceDialog(m_model.records().at(row));
}

void PaymentHistoryWidget::showInvoiceDialog(const PaymentHistoryModel::PaymentRecord& r)
{
    // For Stripe: open external URL if available
    if (!r.invoiceUrl.isEmpty()) {
        QDesktopServices::openUrl(QUrl(r.invoiceUrl));
        return;
    }
    // For SHAH / SHAHtoken: show a simple detail popup
    QString details = tr("Date: %1\nAmount: %2 %3\nMethod: %4\nTransaction ID: %5\nPlan: %6")
        .arg(r.date.toString("yyyy-MM-dd HH:mm"))
        .arg(QString::number(r.amount, 'f', 4))
        .arg(r.currency)
        .arg(r.method)
        .arg(r.transactionId)
        .arg(r.planType);
    QMessageBox::information(this, tr("Invoice"), details);
}


