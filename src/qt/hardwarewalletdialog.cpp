#include <qt/hardwarewalletdialog.h>
#include <qt/walletmodel.h>
#include <qt/guiutil.h>

#include <QHeaderView>
#include <QMessageBox>
#include <QOverload>

HardwareWalletDialog::HardwareWalletDialog(WalletModel* walletModel, QWidget* parent)
	: QDialog(parent)
	, m_mainLayout(nullptr)
	, m_deviceCombo(nullptr)
	, m_addressTable(nullptr)
	, m_detectButton(nullptr)
	, m_importButton(nullptr)
	, m_statusLabel(nullptr)
	, m_walletModel(walletModel)
{
	setWindowTitle(tr("🔐 Import from Hardware Wallet"));
	setModal(true);
	resize(640, 520);
	setupUI();
	populateMockDevices();
}

HardwareWalletDialog::~HardwareWalletDialog() = default;

void HardwareWalletDialog::setupUI()
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setContentsMargins(16, 16, 16, 16);
	m_mainLayout->setSpacing(12);

	QHBoxLayout* top = new QHBoxLayout();
	QLabel* deviceLbl = new QLabel(tr("Device:"), this);
	m_deviceCombo = new QComboBox(this);
	m_detectButton = new QPushButton(tr("Detect"), this);
	connect(m_detectButton, &QPushButton::clicked, this, &HardwareWalletDialog::onDetectClicked);
	connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HardwareWalletDialog::onDeviceChanged);
	top->addWidget(deviceLbl);
	top->addWidget(m_deviceCombo, 1);
	top->addWidget(m_detectButton);
	m_mainLayout->addLayout(top);

	m_addressTable = new QTableWidget(this);
	m_addressTable->setColumnCount(3);
	m_addressTable->setHorizontalHeaderLabels({tr("Select"), tr("Address"), tr("Path")});
	m_addressTable->horizontalHeader()->setStretchLastSection(true);
	m_addressTable->verticalHeader()->setVisible(false);
	m_addressTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_addressTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_mainLayout->addWidget(m_addressTable, 1);

	QHBoxLayout* bottom = new QHBoxLayout();
	m_statusLabel = new QLabel(tr(""), this);
	m_importButton = new QPushButton(tr("Import Selected"), this);
	connect(m_importButton, &QPushButton::clicked, this, &HardwareWalletDialog::onImportSelectedClicked);
	bottom->addWidget(m_statusLabel, 1);
	bottom->addWidget(m_importButton);
	m_mainLayout->addLayout(bottom);
}

void HardwareWalletDialog::populateMockDevices()
{
	m_deviceCombo->clear();
	m_deviceCombo->addItem("Trezor Model T (mock)");
	m_deviceCombo->addItem("Ledger Nano X (mock)");
	populateMockAddresses();
}

void HardwareWalletDialog::onDetectClicked()
{
	// Placeholder detection. Real code would enumerate HID/USB.
	populateMockDevices();
	m_statusLabel->setText(tr("Devices refreshed"));
}

void HardwareWalletDialog::onDeviceChanged(int)
{
	populateMockAddresses();
}

void HardwareWalletDialog::populateMockAddresses()
{
	m_addressTable->setRowCount(0);
	QStringList addrs = {
		"S1HwAddr1abcdef1234567890",
		"S1HwAddr2abcdef1234567890",
		"S1HwAddr3abcdef1234567890"
	};
	QStringList paths = {
		"m/44'/0'/0'/0/0",
		"m/44'/0'/0'/0/1",
		"m/44'/0'/0'/0/2"
	};
	m_addressTable->setRowCount(addrs.size());
	for (int i = 0; i < addrs.size(); ++i) {
		QTableWidgetItem* selItem = new QTableWidgetItem();
		selItem->setCheckState(Qt::Unchecked);
		m_addressTable->setItem(i, 0, selItem);
		m_addressTable->setItem(i, 1, new QTableWidgetItem(addrs.at(i)));
		m_addressTable->setItem(i, 2, new QTableWidgetItem(paths.at(i)));
	}
}

void HardwareWalletDialog::onImportSelectedClicked()
{
	// MVP: simply count selected and notify. Real implementation would add watch-only entries.
	int imported = 0;
	for (int i = 0; i < m_addressTable->rowCount(); ++i) {
		QTableWidgetItem* sel = m_addressTable->item(i, 0);
		if (sel && sel->checkState() == Qt::Checked) {
			imported++;
			// TODO: Use WalletModel API to import as watch-only and tag as hardware
		}
	}
	QMessageBox::information(this, tr("Hardware Wallet"), tr("Imported %1 addresses as watch-only (mock)." ).arg(imported));
}


