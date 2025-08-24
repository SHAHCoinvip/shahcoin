#include "nftgallery.h"
#include "walletmodel.h"
#include "guiutil.h"
#include "shahcoinunits.h"

#include <QApplication>
#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QFrame>
#include <QTimer>
#include <QSettings>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QScreen>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

NFTGallery::NFTGallery(const PlatformStyle* platformStyle, QWidget* parent)
    : QWidget(parent)
    , m_platformStyle(platformStyle)
    , m_currentFilter(AllNFTs)
    , m_updateTimer(nullptr)
{
    setupUI();
    connectSignals();
    applyTheme();
    
    // Setup update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &NFTGallery::onNFTDataUpdated);
    m_updateTimer->start(UPDATE_INTERVAL_MS);
}

NFTGallery::~NFTGallery()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
}

void NFTGallery::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);

    setupControls();
    setupGallery();
}

void NFTGallery::setupControls()
{
    m_controlsLayout = new QHBoxLayout();
    m_controlsLayout->setSpacing(15);

    // Filter combo box
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem(tr("All NFTs"), AllNFTs);
    m_filterCombo->addItem(tr("Owned by Me"), OwnedNFTs);
    m_filterCombo->addItem(tr("Created by Me"), CreatedNFTs);
    m_filterCombo->addItem(tr("Received"), ReceivedNFTs);
    m_filterCombo->setCurrentIndex(0);
    m_filterCombo->setStyleSheet(
        "QComboBox { background-color: white; border: 1px solid #CCCCCC; border-radius: 4px; padding: 8px; }"
        "QComboBox:hover { border-color: #2E86AB; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid #666666; }"
    );

    // Search edit
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search NFTs..."));
    m_searchEdit->setStyleSheet(
        "QLineEdit { background-color: white; border: 1px solid #CCCCCC; border-radius: 4px; padding: 8px; }"
        "QLineEdit:focus { border-color: #2E86AB; }"
    );

    // Refresh button
    m_refreshButton = new QPushButton(tr("Refresh"), this);
    m_refreshButton->setStyleSheet(
        "QPushButton { background-color: #2E86AB; color: white; border: none; padding: 8px 16px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #1E6B8B; }"
    );

    // Status label
    m_statusLabel = new QLabel(tr("Loading NFTs..."), this);
    m_statusLabel->setStyleSheet("QLabel { color: #666666; font-size: 12px; }");

    m_controlsLayout->addWidget(m_filterCombo);
    m_controlsLayout->addWidget(m_searchEdit);
    m_controlsLayout->addWidget(m_refreshButton);
    m_controlsLayout->addStretch();
    m_controlsLayout->addWidget(m_statusLabel);

    m_mainLayout->addLayout(m_controlsLayout);
}

void NFTGallery::setupGallery()
{
    // Create scroll area for gallery
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setStyleSheet(
        "QScrollArea { border: none; background-color: transparent; }"
        "QScrollBar:vertical { background-color: #F0F0F0; width: 12px; border-radius: 6px; }"
        "QScrollBar::handle:vertical { background-color: #CCCCCC; border-radius: 6px; min-height: 20px; }"
        "QScrollBar::handle:vertical:hover { background-color: #999999; }"
    );

    // Create gallery widget
    m_galleryWidget = new QWidget(m_scrollArea);
    m_galleryLayout = new QGridLayout(m_galleryWidget);
    m_galleryLayout->setSpacing(ITEM_SPACING);
    m_galleryLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea->setWidget(m_galleryWidget);
    m_mainLayout->addWidget(m_scrollArea);
}

void NFTGallery::connectSignals()
{
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NFTGallery::onFilterChanged);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &NFTGallery::onSearchChanged);
    connect(m_refreshButton, &QPushButton::clicked, this, &NFTGallery::onRefreshClicked);
}

void NFTGallery::applyTheme()
{
    setStyleSheet(
        "NFTGallery { background-color: #F8F9FA; }"
    );
}

void NFTGallery::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    if (m_walletModel) {
        connect(m_walletModel, &WalletModel::balanceChanged, this, &NFTGallery::onNFTDataUpdated);
        loadNFTData();
    }
}

void NFTGallery::refreshGallery()
{
    loadNFTData();
}

void NFTGallery::onFilterChanged(int index)
{
    m_currentFilter = static_cast<FilterType>(m_filterCombo->itemData(index).toInt());
    filterGallery();
}

void NFTGallery::onSearchChanged(const QString& text)
{
    m_searchText = text;
    filterGallery();
}

void NFTGallery::onRefreshClicked()
{
    loadNFTData();
}

void NFTGallery::onSendNFTClicked(const QString& nftId)
{
    // TODO: Implement NFT sending functionality
    QMessageBox::information(this, tr("Send NFT"), tr("Send functionality for NFT %1 will be implemented soon.").arg(formatNFTId(nftId)));
}

void NFTGallery::onBurnNFTClicked(const QString& nftId)
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Burn NFT"),
        tr("Are you sure you want to burn NFT %1? This action cannot be undone.").arg(formatNFTId(nftId)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // TODO: Implement NFT burning functionality
        showNFTSuccess(tr("NFT %1 has been burned successfully.").arg(formatNFTId(nftId)));
    }
}

void NFTGallery::onViewNFTClicked(const QString& nftId)
{
    // TODO: Implement NFT details view
    QMessageBox::information(this, tr("NFT Details"), tr("Detailed view for NFT %1 will be implemented soon.").arg(formatNFTId(nftId)));
}

void NFTGallery::updateDisplayUnit()
{
    // Update display units if needed
}

void NFTGallery::onNFTDataUpdated()
{
    loadNFTData();
}

void NFTGallery::loadNFTData()
{
    m_allNFTs.clear();
    
    // TODO: Load actual NFT data from wallet/blockchain
    // For now, create sample data
    for (int i = 1; i <= 12; ++i) {
        NFTItem item;
        item.nftId = QString("NFT_%1").arg(i, 6, 10, QChar('0'));
        item.name = QString("Sample NFT %1").arg(i);
        item.description = QString("This is a sample NFT description for NFT %1. It showcases the gallery functionality.").arg(i);
        item.imagePath = QString(":/images/nft_placeholder_%1.png").arg((i % 4) + 1);
        item.collection = QString("Sample Collection %1").arg((i % 3) + 1);
        item.owner = "SAMPLE_OWNER_ADDRESS";
        item.creator = "SAMPLE_CREATOR_ADDRESS";
        item.mintDate = QDateTime::currentDateTime().addDays(-i * 7);
        item.metadata = QString("{\"rarity\": \"%1\", \"attributes\": [\"sample\", \"demo\"]}").arg(i % 3 == 0 ? "rare" : "common");
        item.isOwned = (i % 3 == 0);
        item.isCreated = (i % 4 == 0);
        item.isReceived = (i % 5 == 0);
        
        m_allNFTs.append(item);
    }
    
    filterGallery();
    m_statusLabel->setText(tr("Loaded %1 NFTs").arg(m_allNFTs.size()));
}

void NFTGallery::populateGallery()
{
    clearGallery();
    
    int row = 0;
    int col = 0;
    
    for (const NFTItem& item : m_filteredNFTs) {
        // Create gallery item widget
        NFTGalleryItem* galleryItem = new NFTGalleryItem(
            item.nftId, item.name, item.description, item.imagePath,
            item.collection, item.mintDate, item.isOwned, item.isCreated,
            m_platformStyle, m_galleryWidget
        );
        
        // Connect signals
        connect(galleryItem, &NFTGalleryItem::sendRequested, this, &NFTGallery::onSendNFTClicked);
        connect(galleryItem, &NFTGalleryItem::burnRequested, this, &NFTGallery::onBurnNFTClicked);
        connect(galleryItem, &NFTGalleryItem::viewRequested, this, &NFTGallery::onViewNFTClicked);
        
        // Set image
        QPixmap image = loadNFTImage(item.imagePath);
        galleryItem->setImage(image);
        
        // Add to layout
        m_galleryLayout->addWidget(galleryItem, row, col);
        
        // Update position
        col++;
        if (col >= ITEMS_PER_ROW) {
            col = 0;
            row++;
        }
    }
    
    // Add stretch to fill remaining space
    m_galleryLayout->setRowStretch(row + 1, 1);
    m_galleryLayout->setColumnStretch(ITEMS_PER_ROW, 1);
}

void NFTGallery::clearGallery()
{
    // Remove all widgets from layout
    QLayoutItem* item;
    while ((item = m_galleryLayout->takeAt(0)) != nullptr) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void NFTGallery::filterGallery()
{
    m_filteredNFTs.clear();
    
    for (const NFTItem& item : m_allNFTs) {
        bool includeItem = true;
        
        // Apply filter
        switch (m_currentFilter) {
            case AllNFTs:
                break;
            case OwnedNFTs:
                if (!item.isOwned) includeItem = false;
                break;
            case CreatedNFTs:
                if (!item.isCreated) includeItem = false;
                break;
            case ReceivedNFTs:
                if (!item.isReceived) includeItem = false;
                break;
        }
        
        // Apply search
        if (includeItem && !m_searchText.isEmpty()) {
            QString searchLower = m_searchText.toLower();
            if (!item.name.toLower().contains(searchLower) &&
                !item.description.toLower().contains(searchLower) &&
                !item.collection.toLower().contains(searchLower)) {
                includeItem = false;
            }
        }
        
        if (includeItem) {
            m_filteredNFTs.append(item);
        }
    }
    
    populateGallery();
    m_statusLabel->setText(tr("Showing %1 of %2 NFTs").arg(m_filteredNFTs.size()).arg(m_allNFTs.size()));
}

void NFTGallery::showNFTError(const QString& message)
{
    emit this->message(tr("NFT Error"), message, 2); // Error style
}

void NFTGallery::showNFTSuccess(const QString& message)
{
    emit this->message(tr("NFT Success"), message, 0); // Info style
}

QPixmap NFTGallery::loadNFTImage(const QString& imagePath)
{
    QPixmap pixmap;
    
    // Try to load from resource
    if (pixmap.load(imagePath)) {
        return pixmap.scaled(THUMBNAIL_SIZE, THUMBNAIL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    // Try to load from file
    if (QFile::exists(imagePath)) {
        if (pixmap.load(imagePath)) {
            return pixmap.scaled(THUMBNAIL_SIZE, THUMBNAIL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
    
    // Create placeholder
    return createPlaceholderImage("NFT", QSize(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
}

QPixmap NFTGallery::createPlaceholderImage(const QString& text, const QSize& size)
{
    QPixmap pixmap(size);
    pixmap.fill(QColor(240, 240, 240));
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw border
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawRect(pixmap.rect().adjusted(0, 0, -1, -1));
    
    // Draw text
    painter.setPen(QColor(120, 120, 120));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);
    
    return pixmap;
}

QString NFTGallery::formatNFTId(const QString& nftId)
{
    if (nftId.length() > 12) {
        return nftId.left(6) + "..." + nftId.right(6);
    }
    return nftId;
}

QString NFTGallery::formatDate(const QDateTime& date)
{
    return date.toString("MMM dd, yyyy");
}

// NFTGalleryItem implementation
NFTGalleryItem::NFTGalleryItem(const QString& nftId, const QString& name, const QString& description,
                               const QString& imagePath, const QString& collection, const QDateTime& mintDate,
                               bool isOwned, bool isCreated, const PlatformStyle* platformStyle, QWidget* parent)
    : QFrame(parent)
    , m_nftId(nftId)
    , m_name(name)
    , m_description(description)
    , m_collection(collection)
    , m_mintDate(mintDate)
    , m_isOwned(isOwned)
    , m_isCreated(isCreated)
    , m_platformStyle(platformStyle)
    , m_isHighlighted(false)
{
    setupUI();
    applyTheme();
    updateButtonStates();
    
    // Setup hover animation
    m_hoverAnimation = new QPropertyAnimation(this, "geometry", this);
    m_hoverAnimation->setDuration(200);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // Setup effects
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(10);
    m_shadowEffect->setColor(QColor(0, 0, 0, 60));
    m_shadowEffect->setOffset(0, 2);
    
    setGraphicsEffect(m_shadowEffect);
}

void NFTGalleryItem::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(15, 15, 15, 15);
    m_layout->setSpacing(10);
    
    // Image label
    m_imageLabel = new QLabel(this);
    m_imageLabel->setFixedSize(200, 200);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("QLabel { background-color: #F8F9FA; border: 1px solid #E9ECEF; border-radius: 4px; }");
    m_layout->addWidget(m_imageLabel, 0, Qt::AlignCenter);
    
    // Name label
    m_nameLabel = new QLabel(m_name, this);
    m_nameLabel->setStyleSheet("QLabel { color: #2E86AB; font-size: 14px; font-weight: bold; }");
    m_nameLabel->setWordWrap(true);
    m_layout->addWidget(m_nameLabel);
    
    // Description label
    m_descriptionLabel = new QLabel(m_description, this);
    m_descriptionLabel->setStyleSheet("QLabel { color: #666666; font-size: 12px; }");
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setMaximumHeight(40);
    m_layout->addWidget(m_descriptionLabel);
    
    // Collection label
    m_collectionLabel = new QLabel(tr("Collection: %1").arg(m_collection), this);
    m_collectionLabel->setStyleSheet("QLabel { color: #999999; font-size: 11px; }");
    m_layout->addWidget(m_collectionLabel);
    
    // Date label
    m_dateLabel = new QLabel(tr("Minted: %1").arg(m_mintDate.toString("MMM dd, yyyy")), this);
    m_dateLabel->setStyleSheet("QLabel { color: #999999; font-size: 11px; }");
    m_layout->addWidget(m_dateLabel);
    
    // Button layout
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(8);
    
    m_viewButton = new QPushButton(tr("View"), this);
    m_sendButton = new QPushButton(tr("Send"), this);
    m_burnButton = new QPushButton(tr("Burn"), this);
    
    m_buttonLayout->addWidget(m_viewButton);
    m_buttonLayout->addWidget(m_sendButton);
    m_buttonLayout->addWidget(m_burnButton);
    
    m_layout->addLayout(m_buttonLayout);
    
    // Connect signals
    connect(m_viewButton, &QPushButton::clicked, this, &NFTGalleryItem::onViewClicked);
    connect(m_sendButton, &QPushButton::clicked, this, &NFTGalleryItem::onSendClicked);
    connect(m_burnButton, &QPushButton::clicked, this, &NFTGalleryItem::onBurnClicked);
}

void NFTGalleryItem::applyTheme()
{
    setStyleSheet(
        "NFTGalleryItem { background-color: white; border: 1px solid #E9ECEF; border-radius: 8px; }"
        "NFTGalleryItem:hover { border-color: #2E86AB; }"
    );
    
    m_viewButton->setStyleSheet(
        "QPushButton { background-color: #6C757D; color: white; border: none; padding: 6px 12px; border-radius: 4px; font-size: 11px; }"
        "QPushButton:hover { background-color: #5A6268; }"
    );
    
    m_sendButton->setStyleSheet(
        "QPushButton { background-color: #28A745; color: white; border: none; padding: 6px 12px; border-radius: 4px; font-size: 11px; }"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:disabled { background-color: #CCCCCC; }"
    );
    
    m_burnButton->setStyleSheet(
        "QPushButton { background-color: #DC3545; color: white; border: none; padding: 6px 12px; border-radius: 4px; font-size: 11px; }"
        "QPushButton:hover { background-color: #C82333; }"
        "QPushButton:disabled { background-color: #CCCCCC; }"
    );
}

void NFTGalleryItem::updateButtonStates()
{
    m_sendButton->setEnabled(m_isOwned);
    m_burnButton->setEnabled(m_isOwned || m_isCreated);
}

void NFTGalleryItem::setImage(const QPixmap& pixmap)
{
    m_imageLabel->setPixmap(pixmap);
}

void NFTGalleryItem::setHighlighted(bool highlighted)
{
    m_isHighlighted = highlighted;
    if (highlighted) {
        setStyleSheet(
            "NFTGalleryItem { background-color: #E3F2FD; border: 2px solid #2E86AB; border-radius: 8px; }"
        );
    } else {
        applyTheme();
    }
}

void NFTGalleryItem::onSendClicked()
{
    emit sendRequested(m_nftId);
}

void NFTGalleryItem::onBurnClicked()
{
    emit burnRequested(m_nftId);
}

void NFTGalleryItem::onViewClicked()
{
    emit viewRequested(m_nftId);
}

void NFTGalleryItem::onMouseEnter()
{
    m_hoverAnimation->setStartValue(geometry());
    m_hoverAnimation->setEndValue(geometry().adjusted(-2, -2, 2, 2));
    m_hoverAnimation->start();
    
    m_shadowEffect->setBlurRadius(15);
    m_shadowEffect->setOffset(0, 4);
}

void NFTGalleryItem::onMouseLeave()
{
    m_hoverAnimation->setStartValue(geometry());
    m_hoverAnimation->setEndValue(geometry().adjusted(2, 2, -2, -2));
    m_hoverAnimation->start();
    
    m_shadowEffect->setBlurRadius(10);
    m_shadowEffect->setOffset(0, 2);
}

void NFTGalleryItem::enterEvent(QEvent* event)
{
    QFrame::enterEvent(event);
    onMouseEnter();
}

void NFTGalleryItem::leaveEvent(QEvent* event)
{
    QFrame::leaveEvent(event);
    onMouseLeave();
}

void NFTGalleryItem::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
}
