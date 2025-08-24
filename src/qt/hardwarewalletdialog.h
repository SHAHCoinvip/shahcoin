#ifndef SHAHCOIN_QT_HARDWAREWALLETDIALOG_H
#define SHAHCOIN_QT_HARDWAREWALLETDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTableWidget>
#include <QCheckBox>
#include <QTimer>

class WalletModel;

/**
 * Hardware Wallet Import Dialog (MVP)
 *
 * Simulates detection of Trezor/Ledger devices and imports
 * public addresses into a watch-only list. Real transport/HID
 * code can be wired behind these hooks later.
 */
class HardwareWalletDialog : public QDialog
{
	Q_OBJECT

public:
	explicit HardwareWalletDialog(WalletModel* walletModel, QWidget* parent = nullptr);
	~HardwareWalletDialog();

private Q_SLOTS:
	void onDetectClicked();
	void onImportSelectedClicked();
	void onDeviceChanged(int index);

private:
	void setupUI();
	void populateMockDevices();
	void populateMockAddresses();

private:
	// UI
	QVBoxLayout* m_mainLayout;
	QComboBox* m_deviceCombo;
	QTableWidget* m_addressTable;
	QPushButton* m_detectButton;
	QPushButton* m_importButton;
	QLabel* m_statusLabel;

	// Data
	WalletModel* m_walletModel;
};

#endif // SHAHCOIN_QT_HARDWAREWALLETDIALOG_H


