#ifndef SHAHCOIN_QT_NFTGALLERY_H
#define SHAHCOIN_QT_NFTGALLERY_H

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

class WalletModel;
class PlatformStyle;

/**
 * @brief NFT Gallery Widget - Visual NFT Manager
 * 
 * Displays user-owned NFTs in a visual gallery format with:
 * - Thumbnail images loaded from metadata
 * - Name and description
 * - Send/Burn action buttons
 * - Filtering options (owned, created, received)
 */
class NFTGallery : public QWidget
{
    Q_OBJECT

public:
    explicit NFTGallery(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~NFTGallery();

    void setWalletModel(WalletModel* walletModel);
    void refreshGallery();

public Q_SLOTS:
    void onFilterChanged(int index);
    void onSearchChanged(const QString& text);
    void onRefreshClicked();
    void onSendNFTClicked(const QString& nftId);
    void onBurnNFTClicked(const QString& nftId);
    void onViewNFTClicked(const QString& nftId);

Q_SIGNALS:
    void message(const QString& title, const QString& message, unsigned int style);

private Q_SLOTS:
    void updateDisplayUnit();
    void onNFTDataUpdated();

private:
    void setupUI();
    void setupControls();
    void setupGallery();
    void connectSignals();
    void applyTheme();
    void loadNFTData();
    void populateGallery();
    void clearGallery();
    void filterGallery();
    void showNFTError(const QString& message);
    void showNFTSuccess(const QString& message);
    QPixmap loadNFTImage(const QString& imagePath);
    QPixmap createPlaceholderImage(const QString& text, const QSize& size);
    QString formatNFTId(const QString& nftId);
    QString formatDate(const QDateTime& date);

    // UI Components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_controlsLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_galleryWidget;
    QGridLayout* m_galleryLayout;
    
    // Controls
    QComboBox* m_filterCombo;
    QLineEdit* m_searchEdit;
    QPushButton* m_refreshButton;
    QLabel* m_statusLabel;
    
    // Models
    WalletModel* m_walletModel{nullptr};
    const PlatformStyle* m_platformStyle;
    
    // Timer for periodic updates
    QTimer* m_updateTimer;
    
    // NFT data structures
    struct NFTItem {
        QString nftId;
        QString name;
        QString description;
        QString imagePath;
        QString collection;
        QString owner;
        QString creator;
        QDateTime mintDate;
        QString metadata;
        bool isOwned;
        bool isCreated;
        bool isReceived;
    };
    
    QList<NFTItem> m_allNFTs;
    QList<NFTItem> m_filteredNFTs;
    
    // Gallery item widgets
    struct GalleryItemWidget {
        QFrame* container;
        QLabel* imageLabel;
        QLabel* nameLabel;
        QLabel* descriptionLabel;
        QLabel* collectionLabel;
        QLabel* dateLabel;
        QPushButton* sendButton;
        QPushButton* burnButton;
        QPushButton* viewButton;
        QString nftId;
    };
    
    QList<GalleryItemWidget> m_galleryItems;
    
    // Filter options
    enum FilterType {
        AllNFTs = 0,
        OwnedNFTs,
        CreatedNFTs,
        ReceivedNFTs
    };
    
    FilterType m_currentFilter;
    QString m_searchText;
    
    // Constants
    static constexpr int UPDATE_INTERVAL_MS = 30000; // 30 seconds
    static constexpr int THUMBNAIL_SIZE = 200;
    static constexpr int ITEMS_PER_ROW = 3;
    static constexpr int ITEM_SPACING = 20;
    static constexpr int ITEM_BORDER_RADIUS = 8;
};

/**
 * @brief Individual NFT Gallery Item Widget
 */
class NFTGalleryItem : public QFrame
{
    Q_OBJECT

public:
    explicit NFTGalleryItem(const QString& nftId, const QString& name, const QString& description,
                           const QString& imagePath, const QString& collection, const QDateTime& mintDate,
                           bool isOwned, bool isCreated, const PlatformStyle* platformStyle, QWidget* parent = nullptr);

    QString getNFTId() const { return m_nftId; }
    void setImage(const QPixmap& pixmap);
    void setHighlighted(bool highlighted);

public Q_SLOTS:
    void onSendClicked();
    void onBurnClicked();
    void onViewClicked();
    void onMouseEnter();
    void onMouseLeave();

Q_SIGNALS:
    void sendRequested(const QString& nftId);
    void burnRequested(const QString& nftId);
    void viewRequested(const QString& nftId);

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();
    void applyTheme();
    void updateButtonStates();

    QString m_nftId;
    QString m_name;
    QString m_description;
    QString m_collection;
    QDateTime m_mintDate;
    bool m_isOwned;
    bool m_isCreated;
    const PlatformStyle* m_platformStyle;
    
    QVBoxLayout* m_layout;
    QLabel* m_imageLabel;
    QLabel* m_nameLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_collectionLabel;
    QLabel* m_dateLabel;
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_sendButton;
    QPushButton* m_burnButton;
    QPushButton* m_viewButton;
    
    QPropertyAnimation* m_hoverAnimation;
    QGraphicsOpacityEffect* m_opacityEffect;
    QGraphicsDropShadowEffect* m_shadowEffect;
    bool m_isHighlighted;
};

#endif // SHAHCOIN_QT_NFTGALLERY_H
