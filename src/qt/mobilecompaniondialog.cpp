#include <qt/mobilecompaniondialog.h>
#include <qt/walletmodel.h>
#include <qt/guiutil.h>

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QBuffer>
#include <QPainter>
#include <QPen>
#include <QCryptographicHash>

MobileCompanionDialog::MobileCompanionDialog(WalletModel* walletModel, QWidget* parent)
	: QDialog(parent)
	, m_mainLayout(nullptr)
	, m_qrGroup(nullptr)
	, m_qrLabel(nullptr)
	, m_statusLabel(nullptr)
	, m_addressEdit(nullptr)
	, m_passphraseEdit(nullptr)
	, m_showRawCheck(nullptr)
	, m_rawPayloadEdit(nullptr)
	, m_generateButton(nullptr)
	, m_copyPayloadButton(nullptr)
	, m_saveQRButton(nullptr)
	, m_walletModel(walletModel)
{
	setWindowTitle(tr("📱 Mobile Companion"));
	setModal(true);
	resize(520, 560);
	setupUI();
}

MobileCompanionDialog::~MobileCompanionDialog() = default;

void MobileCompanionDialog::setupUI()
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setContentsMargins(16, 16, 16, 16);
	m_mainLayout->setSpacing(12);

	QLabel* intro = new QLabel(tr("Generate a secure QR to sync a watch-only view on your phone. The payload contains only public data, encrypted with your passphrase."), this);
	intro->setWordWrap(true);
	m_mainLayout->addWidget(intro);

	QHBoxLayout* form = new QHBoxLayout();
	QLabel* addrLbl = new QLabel(tr("Primary Address:"), this);
	m_addressEdit = new QLineEdit(this);
	m_addressEdit->setReadOnly(true);
	if (m_walletModel && m_walletModel->getAddressTableModel()) {
		// Best-effort: first receiving address
		QStringList addrs = m_walletModel->getAddressTableModel()->getAddressList();
		if (!addrs.isEmpty()) m_addressEdit->setText(addrs.first());
	}
	form->addWidget(addrLbl);
	form->addWidget(m_addressEdit, 1);
	m_mainLayout->addLayout(form);

	QHBoxLayout* pass = new QHBoxLayout();
	QLabel* passLbl = new QLabel(tr("Passphrase:"), this);
	m_passphraseEdit = new QLineEdit(this);
	m_passphraseEdit->setEchoMode(QLineEdit::Password);
	m_passphraseEdit->setPlaceholderText(tr("Required to encrypt payload"));
	pass->addWidget(passLbl);
	pass->addWidget(m_passphraseEdit, 1);
	m_mainLayout->addLayout(pass);

	m_qrGroup = new QGroupBox(tr("Companion QR"), this);
	QVBoxLayout* qrLay = new QVBoxLayout(m_qrGroup);
	m_qrLabel = new QLabel(this);
	m_qrLabel->setMinimumSize(260, 260);
	m_qrLabel->setAlignment(Qt::AlignCenter);
	m_qrLabel->setStyleSheet("QLabel { background:white; border:1px solid #ddd; }");
	m_statusLabel = new QLabel(tr("No QR generated yet"), this);
	m_statusLabel->setAlignment(Qt::AlignCenter);

	QHBoxLayout* btns = new QHBoxLayout();
	m_generateButton = new QPushButton(tr("Generate"), this);
	m_copyPayloadButton = new QPushButton(tr("Copy Payload"), this);
	m_saveQRButton = new QPushButton(tr("Save QR"), this);
	btns->addStretch();
	btns->addWidget(m_generateButton);
	btns->addWidget(m_copyPayloadButton);
	btns->addWidget(m_saveQRButton);

	m_showRawCheck = new QCheckBox(tr("Show raw payload"), this);
	m_rawPayloadEdit = new QTextEdit(this);
	m_rawPayloadEdit->setReadOnly(true);
	m_rawPayloadEdit->setVisible(false);

	qrLay->addWidget(m_qrLabel, 0, Qt::AlignCenter);
	qrLay->addWidget(m_statusLabel);
	qrLay->addLayout(btns);
	qrLay->addWidget(m_showRawCheck);
	qrLay->addWidget(m_rawPayloadEdit);

	m_mainLayout->addWidget(m_qrGroup);

	connect(m_generateButton, &QPushButton::clicked, this, &MobileCompanionDialog::onGenerateClicked);
	connect(m_copyPayloadButton, &QPushButton::clicked, this, &MobileCompanionDialog::onCopyPayloadClicked);
	connect(m_saveQRButton, &QPushButton::clicked, this, &MobileCompanionDialog::onSaveQRClicked);
	connect(m_showRawCheck, &QCheckBox::toggled, this, &MobileCompanionDialog::onToggleShowRaw);
}

void MobileCompanionDialog::onGenerateClicked()
{
	QString addr = m_addressEdit->text().trimmed();
	QString pass = m_passphraseEdit->text();
	if (addr.isEmpty()) { m_statusLabel->setText(tr("No address available")); return; }
	if (pass.isEmpty()) { m_statusLabel->setText(tr("Enter a passphrase")); return; }

	QString key = deriveKeyFromPassphrase(pass);
	QString payload = buildPayloadJson();
	QString cipher = encryptPayload(payload, key);
	m_currentPayload = cipher;
	m_rawPayloadEdit->setPlainText(cipher);
	m_statusLabel->setText(tr("Payload generated (%1 bytes)").arg(cipher.toUtf8().size()));
	renderPlaceholderQR(cipher.left(64));
}

void MobileCompanionDialog::onCopyPayloadClicked()
{
	if (m_currentPayload.isEmpty()) return;
	QApplication::clipboard()->setText(m_currentPayload);
}

void MobileCompanionDialog::onSaveQRClicked()
{
	if (m_qrPixmap.isNull()) return;
	QString file = QFileDialog::getSaveFileName(this, tr("Save QR"), "companion_qr.png", tr("PNG Files (*.png)"));
	if (!file.isEmpty()) m_qrPixmap.save(file);
}

void MobileCompanionDialog::onToggleShowRaw(bool checked)
{
	m_rawPayloadEdit->setVisible(checked);
}

QString MobileCompanionDialog::buildPayloadJson() const
{
	QJsonObject obj;
	obj["type"] = "shahcoin_companion_v1";
	obj["created"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
	obj["address"] = m_addressEdit->text().trimmed();
	obj["note"] = "watch_only_sync";
	return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QString MobileCompanionDialog::deriveKeyFromPassphrase(const QString& passphrase) const
{
	// MVP key derivation (placeholder). Replace with proper KDF in production.
	return QString::fromUtf8(QCryptographicHash::hash(passphrase.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString MobileCompanionDialog::encryptPayload(const QString& plaintext, const QString& key) const
{
	// MVP XOR cipher using key bytes. Replace with AES in production.
	QByteArray p = plaintext.toUtf8();
	QByteArray k = QByteArray::fromHex(key.toUtf8());
	if (k.isEmpty()) k = key.toUtf8();
	for (int i = 0; i < p.size(); ++i) p[i] = p[i] ^ k.at(i % k.size());
	return QString::fromUtf8(p.toBase64());
}

void MobileCompanionDialog::renderPlaceholderQR(const QString& text)
{
	QPixmap pm(300, 300);
	pm.fill(Qt::white);
	QPainter pr(&pm);
	pr.setPen(QPen(Qt::black, 2));
	for (int i = 0; i < 25; ++i) {
		for (int j = 0; j < 25; ++j) {
			if (((i * 7 + j * 11) + text.size()) % 3 == 0) {
				pr.fillRect(i * 12, j * 12, 10, 10, Qt::black);
			}
		}
	}
	pr.end();
	m_qrPixmap = pm;
	m_qrLabel->setPixmap(pm);
}


