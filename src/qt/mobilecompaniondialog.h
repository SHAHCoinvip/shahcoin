#ifndef SHAHCOIN_QT_MOBILECOMPANIONDIALOG_H
#define SHAHCOIN_QT_MOBILECOMPANIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QTimer>
#include <QPixmap>

class WalletModel;

/**
 * Mobile Companion Sync Dialog
 *
 * Generates a QR payload containing encrypted public information
 * (primary address and optional xpub/descriptor) that a mobile app
 * can scan to set up a watch-only wallet.
 */
class MobileCompanionDialog : public QDialog
{
	Q_OBJECT

public:
	explicit MobileCompanionDialog(WalletModel* walletModel, QWidget* parent = nullptr);
	~MobileCompanionDialog();

private Q_SLOTS:
	void onGenerateClicked();
	void onCopyPayloadClicked();
	void onSaveQRClicked();
	void onToggleShowRaw(bool checked);

private:
	void setupUI();
	QString buildPayloadJson() const;
	QString deriveKeyFromPassphrase(const QString& passphrase) const;
	QString encryptPayload(const QString& plaintext, const QString& key) const; // simple XOR-based MVP
	void renderPlaceholderQR(const QString& text);

private:
	// UI
	QVBoxLayout* m_mainLayout;
	QGroupBox* m_qrGroup;
	QLabel* m_qrLabel;
	QLabel* m_statusLabel;
	QLineEdit* m_addressEdit;
	QLineEdit* m_passphraseEdit;
	QCheckBox* m_showRawCheck;
	QTextEdit* m_rawPayloadEdit;
	QPushButton* m_generateButton;
	QPushButton* m_copyPayloadButton;
	QPushButton* m_saveQRButton;

	// Data
	WalletModel* m_walletModel;
	QString m_currentPayload;
	QPixmap m_qrPixmap;
};

#endif // SHAHCOIN_QT_MOBILECOMPANIONDIALOG_H


