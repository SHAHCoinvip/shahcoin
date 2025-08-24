#include <qt/bulkoperationsdialog.h>
#include <qt/tokennftexplorer.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QDoubleValidator>
#include <QIntValidator>

BulkOperationsDialog::BulkOperationsDialog(const QList<TokenNFTExplorer::NFTData>& ownedNFTs, QWidget* parent)
    : QDialog(parent)
    , m_ownedNFTs(ownedNFTs)
    , m_selectedNFTs(ownedNFTs)
{
    setupUI();
    updateSelectedNFTs();
    validateOperation();
}

void BulkOperationsDialog::setupUI()
{
    setWindowTitle(tr("Bulk Operations - %1 NFTs").arg(m_ownedNFTs.size()));
    setMinimumSize(800, 600);
    
    m_mainLayout = new QVBoxLayout(this);
    
    // Operation type selection
    QGroupBox* operationGroup = new QGroupBox(tr("Operation Type"), this);
    QHBoxLayout* operationLayout = new QHBoxLayout(operationGroup);
    
    m_operationTypeCombo = new QComboBox(this);
    m_operationTypeCombo->addItems({
        tr("Batch List NFTs"),
        tr("Batch Price Update"),
        tr("Batch Delist NFTs")
    });
    
    operationLayout->addWidget(new QLabel(tr("Select Operation:")));
    operationLayout->addWidget(m_operationTypeCombo);
    operationLayout->addStretch();
    
    m_mainLayout->addWidget(operationGroup);
    
    // NFT selection table
    QGroupBox* selectionGroup = new QGroupBox(tr("NFT Selection"), this);
    QVBoxLayout* selectionLayout = new QVBoxLayout(selectionGroup);
    
    QHBoxLayout* selectionControls = new QHBoxLayout();
    m_selectAllButton = new QPushButton(tr("Select All"), this);
    m_deselectAllButton = new QPushButton(tr("Deselect All"), this);
    QLabel* countLabel = new QLabel(tr("Selected: %1/%2").arg(m_selectedNFTs.size()).arg(m_ownedNFTs.size()), this);
    
    selectionControls->addWidget(m_selectAllButton);
    selectionControls->addWidget(m_deselectAllButton);
    selectionControls->addStretch();
    selectionControls->addWidget(countLabel);
    
    selectionLayout->addLayout(selectionControls);
    
    m_nftSelectionTable = new QTableWidget(this);
    m_nftSelectionTable->setColumnCount(5);
    m_nftSelectionTable->setHorizontalHeaderLabels({
        tr("Select"), tr("Name"), tr("Category"), tr("Current Price"), tr("Tier")
    });
    m_nftSelectionTable->setAlternatingRowColors(true);
    m_nftSelectionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    selectionLayout->addWidget(m_nftSelectionTable);
    m_mainLayout->addWidget(selectionGroup);
    
    // Batch listing fields
    m_listingGroup = new QGroupBox(tr("Listing Details"), this);
    QGridLayout* listingLayout = new QGridLayout(m_listingGroup);
    
    m_categoryEdit = new QLineEdit(this);
    m_categoryEdit->setPlaceholderText(tr("e.g., Art, Game, Music"));
    
    m_tagsEdit = new QLineEdit(this);
    m_tagsEdit->setPlaceholderText(tr("e.g., rare, limited, exclusive"));
    
    m_verificationCheck = new QCheckBox(tr("Submit for verification"), this);
    
    listingLayout->addWidget(new QLabel(tr("Category:")), 0, 0);
    listingLayout->addWidget(m_categoryEdit, 0, 1);
    listingLayout->addWidget(new QLabel(tr("Tags:")), 1, 0);
    listingLayout->addWidget(m_tagsEdit, 1, 1);
    listingLayout->addWidget(m_verificationCheck, 2, 0, 1, 2);
    
    m_mainLayout->addWidget(m_listingGroup);
    
    // Price update fields
    m_priceGroup = new QGroupBox(tr("Price Settings"), this);
    QGridLayout* priceLayout = new QGridLayout(m_priceGroup);
    
    m_priceTypeCombo = new QComboBox(this);
    m_priceTypeCombo->addItems({
        tr("Percentage Change"),
        tr("Fixed Price (SHAH)"),
        tr("Fixed Price (USD)")
    });
    
    m_pricePercentEdit = new QLineEdit(this);
    m_pricePercentEdit->setPlaceholderText(tr("e.g., 10 for +10%, -5 for -5%"));
    m_pricePercentEdit->setValidator(new QDoubleValidator(-100, 1000, 2, this));
    
    m_priceFixedEdit = new QLineEdit(this);
    m_priceFixedEdit->setPlaceholderText(tr("Enter new price"));
    m_priceFixedEdit->setValidator(new QDoubleValidator(0, 1000000, 8, this));
    
    m_priceTable = new QTableWidget(this);
    m_priceTable->setColumnCount(4);
    m_priceTable->setHorizontalHeaderLabels({
        tr("NFT"), tr("Current Price"), tr("New Price"), tr("Change")
    });
    m_priceTable->setMaximumHeight(200);
    
    priceLayout->addWidget(new QLabel(tr("Price Type:")), 0, 0);
    priceLayout->addWidget(m_priceTypeCombo, 0, 1);
    priceLayout->addWidget(new QLabel(tr("Value:")), 1, 0);
    priceLayout->addWidget(m_pricePercentEdit, 1, 1);
    priceLayout->addWidget(m_priceFixedEdit, 1, 1);
    priceLayout->addWidget(m_priceTable, 2, 0, 1, 2);
    
    m_mainLayout->addWidget(m_priceGroup);
    
    // Action buttons
    QHBoxLayout* actionLayout = new QHBoxLayout();
    m_executeButton = new QPushButton(tr("Execute Operation"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: blue;");
    
    actionLayout->addWidget(m_executeButton);
    actionLayout->addWidget(m_cancelButton);
    actionLayout->addStretch();
    actionLayout->addWidget(m_progressBar);
    actionLayout->addWidget(m_statusLabel);
    
    m_mainLayout->addLayout(actionLayout);
    
    // Connect signals
    connect(m_operationTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BulkOperationsDialog::onOperationTypeChanged);
    connect(m_selectAllButton, &QPushButton::clicked, this, &BulkOperationsDialog::onSelectAllClicked);
    connect(m_deselectAllButton, &QPushButton::clicked, this, &BulkOperationsDialog::onDeselectAllClicked);
    connect(m_executeButton, &QPushButton::clicked, this, &BulkOperationsDialog::onExecuteClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    connect(m_priceTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BulkOperationsDialog::onPriceUpdateChanged);
    connect(m_pricePercentEdit, &QLineEdit::textChanged, this, &BulkOperationsDialog::onPriceUpdateChanged);
    connect(m_priceFixedEdit, &QLineEdit::textChanged, this, &BulkOperationsDialog::onPriceUpdateChanged);
    
    connect(m_categoryEdit, &QLineEdit::textChanged, this, &BulkOperationsDialog::onCategoryChanged);
    connect(m_tagsEdit, &QLineEdit::textChanged, this, &BulkOperationsDialog::onTagsChanged);
    connect(m_verificationCheck, &QCheckBox::toggled, this, &BulkOperationsDialog::onVerificationChanged);
    
    // Initialize operation type
    onOperationTypeChanged();
}

void BulkOperationsDialog::updateSelectedNFTs()
{
    m_nftSelectionTable->setRowCount(m_ownedNFTs.size());
    
    for (int i = 0; i < m_ownedNFTs.size(); ++i) {
        const auto& nft = m_ownedNFTs[i];
        
        // Checkbox for selection
        QCheckBox* checkbox = new QCheckBox();
        checkbox->setChecked(m_selectedNFTs.contains(nft));
        m_nftSelectionTable->setCellWidget(i, 0, checkbox);
        
        // Connect checkbox to update selection
        connect(checkbox, &QCheckBox::toggled, [this, i, checkbox](bool checked) {
            if (checked) {
                if (!m_selectedNFTs.contains(m_ownedNFTs[i])) {
                    m_selectedNFTs.append(m_ownedNFTs[i]);
                }
            } else {
                m_selectedNFTs.removeAll(m_ownedNFTs[i]);
            }
            validateOperation();
        });
        
        // NFT name
        m_nftSelectionTable->setItem(i, 1, new QTableWidgetItem(nft.name));
        
        // Category
        m_nftSelectionTable->setItem(i, 2, new QTableWidgetItem(nft.category));
        
        // Current price
        QString priceText = nft.isListed ? 
            QString("%1 SHAH").arg(nft.listedPriceSatoshis / 100000000.0, 0, 'f', 8) :
            tr("Not listed");
        m_nftSelectionTable->setItem(i, 3, new QTableWidgetItem(priceText));
        
        // Tier
        QString tier = nft.metadata.contains("\"tier\":\"Premium\"") ? tr("Premium") :
                      nft.metadata.contains("\"tier\":\"Pro\"") ? tr("Pro") : tr("Basic");
        m_nftSelectionTable->setItem(i, 4, new QTableWidgetItem(tier));
    }
    
    m_nftSelectionTable->resizeColumnsToContents();
}

void BulkOperationsDialog::updatePriceFields()
{
    int priceType = m_priceTypeCombo->currentIndex();
    
    // Show/hide appropriate price input
    m_pricePercentEdit->setVisible(priceType == 0);
    m_priceFixedEdit->setVisible(priceType == 1 || priceType == 2);
    
    // Update price table
    m_priceTable->setRowCount(m_selectedNFTs.size());
    
    for (int i = 0; i < m_selectedNFTs.size(); ++i) {
        const auto& nft = m_selectedNFTs[i];
        
        // NFT name
        m_priceTable->setItem(i, 0, new QTableWidgetItem(nft.name));
        
        // Current price
        QString currentPrice = nft.isListed ? 
            QString("%1 SHAH").arg(nft.listedPriceSatoshis / 100000000.0, 0, 'f', 8) :
            tr("Not listed");
        m_priceTable->setItem(i, 1, new QTableWidgetItem(currentPrice));
        
        // Calculate new price
        qint64 newPriceSatoshis = 0;
        QString newPriceText = tr("N/A");
        
        if (priceType == 0 && !m_pricePercentEdit->text().isEmpty()) {
            // Percentage change
            double percentChange = m_pricePercentEdit->text().toDouble();
            if (nft.isListed) {
                newPriceSatoshis = nft.listedPriceSatoshis * (1 + percentChange / 100.0);
                newPriceText = QString("%1 SHAH").arg(newPriceSatoshis / 100000000.0, 0, 'f', 8);
            }
        } else if (priceType == 1 && !m_priceFixedEdit->text().isEmpty()) {
            // Fixed SHAH price
            double shahPrice = m_priceFixedEdit->text().toDouble();
            newPriceSatoshis = shahPrice * 100000000;
            newPriceText = QString("%1 SHAH").arg(shahPrice, 0, 'f', 8);
        } else if (priceType == 2 && !m_priceFixedEdit->text().isEmpty()) {
            // Fixed USD price (would need price oracle)
            double usdPrice = m_priceFixedEdit->text().toDouble();
            // This would need price oracle integration
            newPriceText = QString("$%1 USD").arg(usdPrice, 0, 'f', 2);
        }
        
        m_priceTable->setItem(i, 2, new QTableWidgetItem(newPriceText));
        
        // Change
        QString changeText = tr("N/A");
        if (nft.isListed && newPriceSatoshis > 0) {
            double changePercent = ((double)newPriceSatoshis / nft.listedPriceSatoshis - 1) * 100;
            changeText = QString("%1%").arg(changePercent, 0, 'f', 1);
        }
        m_priceTable->setItem(i, 3, new QTableWidgetItem(changeText));
        
        // Store new price in operation
        if (newPriceSatoshis > 0) {
            m_currentOperation.newPrices[nft.id] = newPriceSatoshis;
        }
    }
    
    m_priceTable->resizeColumnsToContents();
}

void BulkOperationsDialog::validateOperation()
{
    bool isValid = true;
    QString errorMessage;
    
    // Check if NFTs are selected
    if (m_selectedNFTs.isEmpty()) {
        isValid = false;
        errorMessage = tr("Please select at least one NFT.");
    }
    
    // Check operation-specific requirements
    int operationType = m_operationTypeCombo->currentIndex();
    
    if (operationType == 0) { // Batch List
        if (m_categoryEdit->text().trimmed().isEmpty()) {
            isValid = false;
            errorMessage = tr("Please enter a category for listing.");
        }
        
        // Check if prices are set
        bool hasPrices = false;
        for (const auto& nft : m_selectedNFTs) {
            if (m_currentOperation.newPrices.contains(nft.id) && m_currentOperation.newPrices[nft.id] > 0) {
                hasPrices = true;
                break;
            }
        }
        
        if (!hasPrices) {
            isValid = false;
            errorMessage = tr("Please set prices for the NFTs to be listed.");
        }
    } else if (operationType == 1) { // Batch Price Update
        // Check if prices are set
        bool hasPrices = false;
        for (const auto& nft : m_selectedNFTs) {
            if (m_currentOperation.newPrices.contains(nft.id) && m_currentOperation.newPrices[nft.id] > 0) {
                hasPrices = true;
                break;
            }
        }
        
        if (!hasPrices) {
            isValid = false;
            errorMessage = tr("Please set new prices for the NFTs.");
        }
    }
    
    // Update UI
    m_executeButton->setEnabled(isValid);
    m_statusLabel->setText(errorMessage);
    m_statusLabel->setStyleSheet(isValid ? "color: green;" : "color: red;");
}

void BulkOperationsDialog::onOperationTypeChanged()
{
    int operationType = m_operationTypeCombo->currentIndex();
    
    // Show/hide appropriate groups
    m_listingGroup->setVisible(operationType == 0); // Batch List
    m_priceGroup->setVisible(operationType == 0 || operationType == 1); // Batch List or Price Update
    
    // Update operation type
    switch (operationType) {
        case 0: m_currentOperation.operationType = "batch_list"; break;
        case 1: m_currentOperation.operationType = "batch_price_update"; break;
        case 2: m_currentOperation.operationType = "batch_delist"; break;
    }
    
    validateOperation();
}

void BulkOperationsDialog::onSelectAllClicked()
{
    m_selectedNFTs = m_ownedNFTs;
    updateSelectedNFTs();
    validateOperation();
}

void BulkOperationsDialog::onDeselectAllClicked()
{
    m_selectedNFTs.clear();
    updateSelectedNFTs();
    validateOperation();
}

void BulkOperationsDialog::onExecuteClicked()
{
    // Confirm operation
    QString operationText;
    switch (m_operationTypeCombo->currentIndex()) {
        case 0: operationText = tr("list %1 NFTs").arg(m_selectedNFTs.size()); break;
        case 1: operationText = tr("update prices for %1 NFTs").arg(m_selectedNFTs.size()); break;
        case 2: operationText = tr("delist %1 NFTs").arg(m_selectedNFTs.size()); break;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Operation"),
        tr("Are you sure you want to %1?\n\nThis action cannot be undone.").arg(operationText),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Prepare operation
        m_currentOperation.nftIds.clear();
        for (const auto& nft : m_selectedNFTs) {
            m_currentOperation.nftIds.append(nft.id);
        }
        
        m_currentOperation.category = m_categoryEdit->text().trimmed();
        m_currentOperation.tags = m_tagsEdit->text().trimmed();
        m_currentOperation.submitForVerification = m_verificationCheck->isChecked();
        m_currentOperation.scheduledTime = QDateTime::currentDateTime();
        
        // Emit signal
        emit bulkOperationRequested(m_currentOperation);
        
        // Close dialog
        accept();
    }
}

void BulkOperationsDialog::onPriceUpdateChanged()
{
    updatePriceFields();
    validateOperation();
}

void BulkOperationsDialog::onCategoryChanged()
{
    m_currentOperation.category = m_categoryEdit->text().trimmed();
    validateOperation();
}

void BulkOperationsDialog::onTagsChanged()
{
    m_currentOperation.tags = m_tagsEdit->text().trimmed();
    validateOperation();
}

void BulkOperationsDialog::onVerificationChanged()
{
    m_currentOperation.submitForVerification = m_verificationCheck->isChecked();
    validateOperation();
}
