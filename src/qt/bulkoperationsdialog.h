#ifndef SHAHCOIN_QT_BULKOPERATIONSDIALOG_H
#define SHAHCOIN_QT_BULKOPERATIONSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QProgressBar>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QString>
#include <QDateTime>

// Forward declarations
class TokenNFTExplorer;

/**
 * Bulk Operations Dialog
 * 
 * Handles bulk operations for creators (batch listing, pricing updates, delisting)
 */
class BulkOperationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BulkOperationsDialog(const QList<TokenNFTExplorer::NFTData>& ownedNFTs, QWidget* parent = nullptr);

Q_SIGNALS:
    void bulkOperationRequested(const TokenNFTExplorer::BulkOperation& operation);

private Q_SLOTS:
    void onOperationTypeChanged();
    void onSelectAllClicked();
    void onDeselectAllClicked();
    void onExecuteClicked();
    void onCancelClicked();
    void onPriceUpdateChanged();
    void onCategoryChanged();
    void onTagsChanged();
    void onVerificationChanged();

private:
    void setupUI();
    void updateSelectedNFTs();
    void updatePriceFields();
    void validateOperation();
    
    QList<TokenNFTExplorer::NFTData> m_ownedNFTs;
    QList<TokenNFTExplorer::NFTData> m_selectedNFTs;
    TokenNFTExplorer::BulkOperation m_currentOperation;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QComboBox* m_operationTypeCombo;
    QTableWidget* m_nftSelectionTable;
    QPushButton* m_selectAllButton;
    QPushButton* m_deselectAllButton;
    
    // Batch listing fields
    QGroupBox* m_listingGroup;
    QLineEdit* m_categoryEdit;
    QLineEdit* m_tagsEdit;
    QCheckBox* m_verificationCheck;
    
    // Price update fields
    QGroupBox* m_priceGroup;
    QLineEdit* m_pricePercentEdit;
    QLineEdit* m_priceFixedEdit;
    QComboBox* m_priceTypeCombo;
    QTableWidget* m_priceTable;
    
    // Common fields
    QPushButton* m_executeButton;
    QPushButton* m_cancelButton;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
};

#endif // SHAHCOIN_QT_BULKOPERATIONSDIALOG_H
