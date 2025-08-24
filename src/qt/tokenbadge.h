// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_TOKENBADGE_H
#define SHAHCOIN_QT_TOKENBADGE_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QPropertyAnimation;
QT_END_NAMESPACE

namespace Ui {
class TokenBadge;
}

/**
 * Token Badge Types for SHI-20 and SHI-30
 */
enum class TokenBadgeType {
    UNKNOWN,
    SHI_20,
    SHI_30,
    VERIFIED
};

/**
 * Token Badge Sizes
 */
enum class TokenBadgeSize {
    SMALL,
    MEDIUM,
    LARGE
};

/**
 * Token Badge Data Structure
 */
struct TokenBadgeData {
    TokenBadgeType type = TokenBadgeType::UNKNOWN;
    QString icon;
    QString label;
    QString tooltip;
    QString color;
    QString bgColor;
    QString qtIcon;
    QString qtStyle;
    bool verified = false;
    bool loading = false;
    QString error;
};

/**
 * Token Badge Widget
 * 
 * Displays SHI-20 and SHI-30 token badges in Qt Wallet interface
 */
class TokenBadge : public QWidget
{
    Q_OBJECT

public:
    explicit TokenBadge(QWidget *parent = nullptr);
    explicit TokenBadge(const QVariantMap& metadata, QWidget *parent = nullptr);
    explicit TokenBadge(const QString& contractAddress, QWidget *parent = nullptr);
    ~TokenBadge();

    // Badge configuration
    void setMetadata(const QVariantMap& metadata);
    void setContractAddress(const QString& contractAddress);
    void setBadgeSize(TokenBadgeSize size);
    void setShowLabel(bool show);
    void setShowIcon(bool show);
    void setClickable(bool clickable);

    // Badge data getters
    TokenBadgeData getBadgeData() const;
    TokenBadgeType getBadgeType() const;
    bool isVerified() const;
    bool isLoading() const;
    QString getError() const;

    // Badge styling
    void setCustomStyle(const QString& style);
    void setCustomIcon(const QString& iconPath);
    void setCustomColors(const QString& color, const QString& bgColor);

    // Badge interaction
    void setOnClickCallback(std::function<void()> callback);
    void setTooltipText(const QString& tooltip);

    // Static badge creation
    static TokenBadge* createSHI20Badge(QWidget *parent = nullptr);
    static TokenBadge* createSHI30Badge(QWidget *parent = nullptr);
    static TokenBadge* createVerifiedBadge(QWidget *parent = nullptr);
    static TokenBadge* createUnknownBadge(QWidget *parent = nullptr);

    // Badge detection utilities
    static TokenBadgeType detectBadgeType(const QVariantMap& metadata);
    static TokenBadgeData createBadgeData(TokenBadgeType type);
    static QString getBadgeIcon(TokenBadgeType type);
    static QString getBadgeLabel(TokenBadgeType type);
    static QString getBadgeTooltip(TokenBadgeType type);
    static QString getBadgeColor(TokenBadgeType type);
    static QString getBadgeBgColor(TokenBadgeType type);
    static QString getQtIconPath(TokenBadgeType type);
    static QString getQtStyleSheet(TokenBadgeType type);

signals:
    void badgeClicked();
    void badgeHovered();
    void badgeDataChanged(const TokenBadgeData& data);
    void badgeError(const QString& error);

public slots:
    void refreshBadge();
    void setLoading(bool loading);
    void setError(const QString& error);
    void clearError();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onMetadataReceived();
    void onNetworkError(QNetworkReply::NetworkError error);
    void onBadgeClicked();
    void onBadgeHovered();

private:
    Ui::TokenBadge *ui;
    TokenBadgeData m_badgeData;
    TokenBadgeSize m_badgeSize;
    bool m_showLabel;
    bool m_showIcon;
    bool m_clickable;
    QString m_contractAddress;
    QVariantMap m_metadata;
    QNetworkAccessManager *m_networkManager;
    std::function<void()> m_onClickCallback;
    QPropertyAnimation *m_hoverAnimation;

    void setupUI();
    void setupConnections();
    void updateBadgeDisplay();
    void detectBadgeFromMetadata();
    void fetchMetadataFromAddress();
    void applyBadgeStyle();
    void createHoverAnimation();
    void updateBadgeGeometry();

    // Metadata validation
    bool validateMetadata(const QVariantMap& metadata);
    bool isShahFactoryCreated(const QVariantMap& metadata);
    TokenBadgeType smartFallbackDetection(const QVariantMap& metadata);

    // Network utilities
    QNetworkReply* requestMetadata(const QString& contractAddress);
    QVariantMap parseMetadataResponse(const QByteArray& response);
    QVariantMap getMockMetadata(const QString& contractAddress);

    // Style utilities
    QString generateStyleSheet() const;
    QString getSizeClass() const;
    QString getTypeClass() const;
    QString getAnimationClass() const;
};

/**
 * Token Badge Group Widget
 * 
 * Displays multiple badges together (e.g., SHI-20 + Verified)
 */
class TokenBadgeGroup : public QWidget
{
    Q_OBJECT

public:
    explicit TokenBadgeGroup(QWidget *parent = nullptr);
    ~TokenBadgeGroup();

    void addBadge(TokenBadge *badge);
    void removeBadge(TokenBadge *badge);
    void clearBadges();
    void setSpacing(int spacing);
    void setAlignment(Qt::Alignment alignment);

    QList<TokenBadge*> getBadges() const;
    int getBadgeCount() const;

signals:
    void badgeAdded(TokenBadge *badge);
    void badgeRemoved(TokenBadge *badge);

private:
    QHBoxLayout *m_layout;
    QList<TokenBadge*> m_badges;

    void setupLayout();
};

/**
 * Token Badge List Widget
 * 
 * Displays a list of tokens with their badges
 */
class TokenBadgeList : public QWidget
{
    Q_OBJECT

public:
    explicit TokenBadgeList(QWidget *parent = nullptr);
    ~TokenBadgeList();

    void addToken(const QString& name, const QVariantMap& metadata, const QString& contractAddress = QString());
    void addToken(const QString& name, TokenBadge *badge);
    void removeToken(const QString& name);
    void clearTokens();
    void setBadgeSize(TokenBadgeSize size);

    QList<QPair<QString, TokenBadge*>> getTokens() const;
    int getTokenCount() const;

signals:
    void tokenClicked(const QString& name, TokenBadge *badge);

private:
    QVBoxLayout *m_layout;
    QList<QPair<QString, TokenBadge*>> m_tokens;
    TokenBadgeSize m_badgeSize;

    void setupLayout();
    void createTokenItem(const QString& name, TokenBadge *badge);
};

/**
 * Token Badge Manager
 * 
 * Manages badge creation, caching, and metadata storage
 */
class TokenBadgeManager : public QObject
{
    Q_OBJECT

public:
    static TokenBadgeManager* instance();
    
    TokenBadge* createBadge(const QVariantMap& metadata, QWidget *parent = nullptr);
    TokenBadge* createBadge(const QString& contractAddress, QWidget *parent = nullptr);
    TokenBadge* createBadge(TokenBadgeType type, QWidget *parent = nullptr);

    // Metadata management
    void storeMetadata(const QString& contractAddress, const QVariantMap& metadata);
    QVariantMap getMetadata(const QString& contractAddress);
    void updateVerificationStatus(const QString& contractAddress, bool verified);
    QList<QString> getVerifiedTokens();
    QList<QString> getTokensByType(TokenBadgeType type);

    // Cache management
    void clearCache();
    void preloadMetadata(const QStringList& contractAddresses);

signals:
    void metadataStored(const QString& contractAddress);
    void metadataUpdated(const QString& contractAddress);
    void verificationStatusChanged(const QString& contractAddress, bool verified);

private:
    explicit TokenBadgeManager(QObject *parent = nullptr);
    ~TokenBadgeManager();

    static TokenBadgeManager* s_instance;
    QHash<QString, QVariantMap> m_metadataCache;
    QHash<QString, TokenBadge*> m_badgeCache;
    QNetworkAccessManager *m_networkManager;

    void loadCachedMetadata();
    void saveCachedMetadata();
    QString getCacheFilePath() const;
};

#endif // SHAHCOIN_QT_TOKENBADGE_H
