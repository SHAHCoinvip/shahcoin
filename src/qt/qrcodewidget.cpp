#include <qt/qrcodewidget.h>
#include <qt/walletmodel.h>
#include <qt/guiutil.h>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QBuffer>
#include <QByteArray>

// Note: In a real implementation, you would include a QR code library
// For now, we'll create a placeholder QR code display

QRCodeWidget::QRCodeWidget(QWidget* parent)
    : QWidget(parent)
    , m_address("")
    , m_amount(0.0)
    , m_walletModel(nullptr)
{
    setupUI();
}

void QRCodeWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // QR Code Group
    m_qrGroup = new QGroupBox(tr("📱 QR Code for Receiving SHAH"), this);
    QVBoxLayout* qrLayout = new QVBoxLayout(m_qrGroup);
    
    // QR Code Display
    m_qrCodeLabel = new QLabel(this);
    m_qrCodeLabel->setMinimumSize(200, 200);
    m_qrCodeLabel->setMaximumSize(300, 300);
    m_qrCodeLabel->setAlignment(Qt::AlignCenter);
    m_qrCodeLabel->setStyleSheet("QLabel { border: 2px solid #FFD700; border-radius: 10px; background-color: white; }");
    m_qrCodeLabel->setText(tr("QR Code will appear here"));
    
    // Address Display
    m_addressLabel = new QLabel(tr("Wallet Address:"), this);
    m_addressEdit = new QLineEdit(this);
    m_addressEdit->setReadOnly(true);
    m_addressEdit->setStyleSheet("QLineEdit { background-color: #f8f9fa; border: 1px solid #dee2e6; padding: 8px; }");
    
    // Amount Input
    QLabel* amountLabel = new QLabel(tr("Amount (SHAH):"), this);
    m_amountSpinBox = new QDoubleSpinBox(this);
    m_amountSpinBox->setRange(0.0, 1000000.0);
    m_amountSpinBox->setDecimals(8);
    m_amountSpinBox->setSuffix(" SHAH");
    m_amountSpinBox->setToolTip(tr("Enter the amount you want to receive (optional)"));
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_copyAddressButton = new QPushButton(tr("📋 Copy Address"), this);
    m_copyAddressButton->setToolTip(tr("Copy the wallet address to clipboard"));
    
    m_saveQRButton = new QPushButton(tr("💾 Save QR Code"), this);
    m_saveQRButton->setToolTip(tr("Save the QR code as an image file"));
    
    buttonLayout->addWidget(m_copyAddressButton);
    buttonLayout->addWidget(m_saveQRButton);
    
    // Layout
    qrLayout->addWidget(m_qrCodeLabel, 0, Qt::AlignCenter);
    qrLayout->addWidget(m_addressLabel);
    qrLayout->addWidget(m_addressEdit);
    qrLayout->addWidget(amountLabel);
    qrLayout->addWidget(m_amountSpinBox);
    qrLayout->addLayout(buttonLayout);
    
    m_mainLayout->addWidget(m_qrGroup);
    
    // Connect signals
    connect(m_amountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &QRCodeWidget::onAmountChanged);
    connect(m_copyAddressButton, &QPushButton::clicked,
        this, &QRCodeWidget::onCopyAddressClicked);
    connect(m_saveQRButton, &QPushButton::clicked,
        this, &QRCodeWidget::onSaveQRClicked);
}

void QRCodeWidget::setAddress(const QString& address)
{
    m_address = address;
    m_addressEdit->setText(address);
    updateQRCode();
}

void QRCodeWidget::setAmount(double amount)
{
    m_amount = amount;
    m_amountSpinBox->setValue(amount);
    updateQRCode();
}

void QRCodeWidget::generateQRCode()
{
    updateQRCode();
}

void QRCodeWidget::setWalletModel(WalletModel* model)
{
    m_walletModel = model;
    if (m_walletModel) {
        // Get the first receiving address from the wallet
        QStringList addresses = m_walletModel->getAddressTableModel()->getAddressList();
        if (!addresses.isEmpty()) {
            setAddress(addresses.first());
        }
    }
}

void QRCodeWidget::onAmountChanged(double amount)
{
    m_amount = amount;
    updateQRCode();
}

void QRCodeWidget::onCopyAddressClicked()
{
    if (!m_address.isEmpty()) {
        QApplication::clipboard()->setText(m_address);
        QMessageBox::information(this, tr("Address Copied"), tr("Wallet address has been copied to clipboard."));
    } else {
        QMessageBox::warning(this, tr("No Address"), tr("No wallet address available to copy."));
    }
}

void QRCodeWidget::onSaveQRClicked()
{
    if (m_qrPixmap.isNull()) {
        QMessageBox::warning(this, tr("No QR Code"),
            tr("Please generate a QR code first."));
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save QR Code"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/shahcoin_qr.png",
        tr("PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        if (m_qrPixmap.save(fileName)) {
            QMessageBox::information(this, tr("QR Code Saved"),
                tr("QR code has been saved to: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, tr("Save Failed"),
                tr("Failed to save QR code. Please try again."));
        }
    }
}

void QRCodeWidget::updateQRCode()
{
    if (m_address.isEmpty()) {
        m_qrCodeLabel->setText(tr("No address available"));
        return;
    }
    
    // Generate QR code string
    QString qrString = generateQRString();
    
    // Create a placeholder QR code (in real implementation, use a QR library)
    QPixmap qrPixmap(250, 250);
    qrPixmap.fill(Qt::white);
    
    QPainter painter(&qrPixmap);
    painter.setPen(QPen(Qt::black, 2));
    
    // Draw a simple placeholder pattern
    for (int i = 0; i < 25; ++i) {
        for (int j = 0; j < 25; ++j) {
            if ((i + j) % 2 == 0) {
                painter.fillRect(i * 10, j * 10, 10, 10, Qt::black);
            }
        }
    }
    
    // Add text overlay
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 8));
    painter.drawText(qrPixmap.rect(), Qt::AlignCenter, tr("QR Code\n%1").arg(qrString.left(20) + "..."));
    
    painter.end();
    
    m_qrPixmap = qrPixmap;
    m_qrCodeLabel->setPixmap(qrPixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QString QRCodeWidget::generateQRString() const
{
    if (m_address.isEmpty()) {
        return "";
    }
    
    QString qrString = QString("shah:%1").arg(m_address);
    
    if (m_amount > 0.0) {
        qrString += QString("?amount=%1").arg(m_amount, 0, 'f', 8);
    }
    
    return qrString;
}
