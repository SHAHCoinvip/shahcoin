// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "tokenbadge.h"
#include "ui_tokenbadge.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTimer>
#include <QToolTip>
#include <QStyle>
#include <QStyleOption>

// Badge constants
const QString SHI_20_STANDARD = "shahcoin.token.standard.v1";
const QString SHI_30_STANDARD = "shahcoin.nft.standard.v1";
const QString SHAHFACTORY_CREATOR = "ShahFactory";

// Badge icons (Unicode)
const QString SHI_20_ICON = "🪙";
const QString SHI_30_ICON = "💎";
const QString VERIFIED_ICON = "✅";
const QString UNKNOWN_ICON = "❓";
const QString LOADING_ICON = "⏳";

// Badge labels
const QString SHI_20_LABEL = "Official Shahcoin Token (SHI-20)";
const QString SHI_30_LABEL = "Official Shahcoin NFT (SHI-30)";
const QString VERIFIED_LABEL = "Verified by SHAH Admin Team";
const QString UNKNOWN_LABEL = "Unknown Token";
const QString LOADING_LABEL = "Loading...";

// Badge tooltips
const QString SHI_20_TOOLTIP = "Created via ShahFactory - Official Shahcoin Standard";
const QString SHI_30_TOOLTIP = "Created via ShahFactory - Official Shahcoin Standard";
const QString VERIFIED_TOOLTIP = "This asset has been verified by the Shahcoin Admin Team";
const QString UNKNOWN_TOOLTIP = "Token type could not be determined";

// Badge colors
const QString SHI_20_COLOR = "#6B7280";
const QString SHI_20_BG_COLOR = "#F3F4F6";
const QString SHI_30_COLOR = "#F59E0B";
const QString SHI_30_BG_COLOR = "#FEF3C7";
const QString VERIFIED_COLOR = "#10B981";
const QString VERIFIED_BG_COLOR = "#D1FAE5";
const QString UNKNOWN_COLOR = "#9CA3AF";
const QString UNKNOWN_BG_COLOR = "#F9FAFB";

TokenBadge::TokenBadge(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TokenBadge)
    , m_badgeSize(TokenBadgeSize::MEDIUM)
    , m_showLabel(true)
    , m_showIcon(true)
    , m_clickable(false)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_hoverAnimation(nullptr)
{
    setupUI();
    setupConnections();
    createHoverAnimation();
}

TokenBadge::TokenBadge(const QVariantMap& metadata, QWidget *parent)
    : TokenBadge(parent)
{
    setMetadata(metadata);
}

TokenBadge::TokenBadge(const QString& contractAddress, QWidget *parent)
    : TokenBadge(parent)
{
    setContractAddress(contractAddress);
}

TokenBadge::~TokenBadge()
{
    delete ui;
}

void TokenBadge::setupUI()
{
    ui->setupUi(this);
    
    // Set default properties
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover);
    
    // Initialize badge data
    m_badgeData = createBadgeData(TokenBadgeType::UNKNOWN);
    updateBadgeDisplay();
}

void TokenBadge::setupConnections()
{
    // Connect network signals
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &TokenBadge::onMetadataReceived);
}

void TokenBadge::createHoverAnimation()
{
    m_hoverAnimation = new QPropertyAnimation(this, "geometry");
    m_hoverAnimation->setDuration(150);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void TokenBadge::setMetadata(const QVariantMap& metadata)
{
    m_metadata = metadata;
    detectBadgeFromMetadata();
}

void TokenBadge::setContractAddress(const QString& contractAddress)
{
    m_contractAddress = contractAddress;
    if (!contractAddress.isEmpty()) {
        fetchMetadataFromAddress();
    }
}

void TokenBadge::setBadgeSize(TokenBadgeSize size)
{
    m_badgeSize = size;
    updateBadgeDisplay();
}

void TokenBadge::setShowLabel(bool show)
{
    m_showLabel = show;
    updateBadgeDisplay();
}

void TokenBadge::setShowIcon(bool show)
{
    m_showIcon = show;
    updateBadgeDisplay();
}

void TokenBadge::setClickable(bool clickable)
{
    m_clickable = clickable;
    updateBadgeDisplay();
}

TokenBadgeData TokenBadge::getBadgeData() const
{
    return m_badgeData;
}

TokenBadgeType TokenBadge::getBadgeType() const
{
    return m_badgeData.type;
}

bool TokenBadge::isVerified() const
{
    return m_badgeData.verified;
}

bool TokenBadge::isLoading() const
{
    return m_badgeData.loading;
}

QString TokenBadge::getError() const
{
    return m_badgeData.error;
}

void TokenBadge::setCustomStyle(const QString& style)
{
    setStyleSheet(style);
}

void TokenBadge::setCustomIcon(const QString& iconPath)
{
    m_badgeData.qtIcon = iconPath;
    updateBadgeDisplay();
}

void TokenBadge::setCustomColors(const QString& color, const QString& bgColor)
{
    m_badgeData.color = color;
    m_badgeData.bgColor = bgColor;
    applyBadgeStyle();
}

void TokenBadge::setOnClickCallback(std::function<void()> callback)
{
    m_onClickCallback = callback;
    setClickable(true);
}

void TokenBadge::setTooltipText(const QString& tooltip)
{
    m_badgeData.tooltip = tooltip;
    setToolTip(tooltip);
}

void TokenBadge::refreshBadge()
{
    if (!m_contractAddress.isEmpty()) {
        fetchMetadataFromAddress();
    } else if (!m_metadata.isEmpty()) {
        detectBadgeFromMetadata();
    }
}

void TokenBadge::setLoading(bool loading)
{
    m_badgeData.loading = loading;
    if (loading) {
        m_badgeData.icon = LOADING_ICON;
        m_badgeData.label = LOADING_LABEL;
        m_badgeData.tooltip = "Loading token information...";
    }
    updateBadgeDisplay();
}

void TokenBadge::setError(const QString& error)
{
    m_badgeData.error = error;
    m_badgeData.type = TokenBadgeType::UNKNOWN;
    m_badgeData.icon = UNKNOWN_ICON;
    m_badgeData.label = UNKNOWN_LABEL;
    m_badgeData.tooltip = "Error: " + error;
    updateBadgeDisplay();
    emit badgeError(error);
}

void TokenBadge::clearError()
{
    m_badgeData.error.clear();
    updateBadgeDisplay();
}

void TokenBadge::detectBadgeFromMetadata()
{
    if (m_metadata.isEmpty()) {
        setError("No metadata provided");
        return;
    }

    if (!validateMetadata(m_metadata)) {
        setError("Invalid metadata format");
        return;
    }

    // Check if verified
    bool verified = m_metadata.value("verified", false).toBool();
    if (verified) {
        m_badgeData = createBadgeData(TokenBadgeType::VERIFIED);
        m_badgeData.verified = true;
        updateBadgeDisplay();
        return;
    }

    // Detect badge type
    TokenBadgeType type = detectBadgeType(m_metadata);
    m_badgeData = createBadgeData(type);
    m_badgeData.verified = false;
    m_badgeData.loading = false;
    m_badgeData.error.clear();

    updateBadgeDisplay();
    emit badgeDataChanged(m_badgeData);
}

void TokenBadge::fetchMetadataFromAddress()
{
    if (m_contractAddress.isEmpty()) {
        setError("No contract address provided");
        return;
    }

    setLoading(true);
    
    // For now, use mock data - in real implementation, this would fetch from blockchain
    QTimer::singleShot(100, [this]() {
        QVariantMap mockMetadata = getMockMetadata(m_contractAddress);
        if (!mockMetadata.isEmpty()) {
            setMetadata(mockMetadata);
        } else {
            setError("Failed to fetch metadata");
        }
    });
}

void TokenBadge::updateBadgeDisplay()
{
    // Update UI elements
    if (ui->iconLabel) {
        ui->iconLabel->setText(m_showIcon ? m_badgeData.icon : "");
        ui->iconLabel->setVisible(m_showIcon);
    }
    
    if (ui->labelLabel) {
        ui->labelLabel->setText(m_showLabel ? m_badgeData.label : "");
        ui->labelLabel->setVisible(m_showLabel);
    }

    // Apply styling
    applyBadgeStyle();
    
    // Update tooltip
    setToolTip(m_badgeData.tooltip);
    
    // Update cursor
    setCursor(m_clickable ? Qt::PointingHandCursor : Qt::ArrowCursor);
    
    // Update geometry
    updateBadgeGeometry();
    
    // Force repaint
    update();
}

void TokenBadge::applyBadgeStyle()
{
    QString styleSheet = generateStyleSheet();
    setStyleSheet(styleSheet);
}

void TokenBadge::updateBadgeGeometry()
{
    // Adjust size based on content and badge size
    QSize contentSize = sizeHint();
    QSize minSize;
    
    switch (m_badgeSize) {
        case TokenBadgeSize::SMALL:
            minSize = QSize(60, 20);
            break;
        case TokenBadgeSize::MEDIUM:
            minSize = QSize(80, 24);
            break;
        case TokenBadgeSize::LARGE:
            minSize = QSize(100, 28);
            break;
    }
    
    QSize finalSize = contentSize.expandedTo(minSize);
    resize(finalSize);
}

bool TokenBadge::validateMetadata(const QVariantMap& metadata)
{
    QStringList requiredFields = {"standard", "type", "name", "createdBy", "verified"};
    
    for (const QString& field : requiredFields) {
        if (!metadata.contains(field)) {
            return false;
        }
    }
    
    // Validate standard
    QString standard = metadata.value("standard").toString();
    if (standard != SHI_20_STANDARD && standard != SHI_30_STANDARD) {
        return false;
    }
    
    // Validate type
    QString type = metadata.value("type").toString();
    if (type != "SHI-20" && type != "SHI-30") {
        return false;
    }
    
    return true;
}

bool TokenBadge::isShahFactoryCreated(const QVariantMap& metadata)
{
    QString createdBy = metadata.value("createdBy").toString();
    return createdBy == SHAHFACTORY_CREATOR;
}

TokenBadgeType TokenBadge::smartFallbackDetection(const QVariantMap& metadata)
{
    // Check if created by ShahFactory
    if (isShahFactoryCreated(metadata)) {
        // Infer from other properties
        if (metadata.contains("decimals") || metadata.contains("totalSupply")) {
            return TokenBadgeType::SHI_20;
        }
        if (metadata.contains("attributes") || metadata.contains("image")) {
            return TokenBadgeType::SHI_30;
        }
    }
    
    // Check for ERC-20 like properties
    if (metadata.contains("decimals") && metadata.contains("totalSupply")) {
        return TokenBadgeType::SHI_20;
    }
    
    // Check for NFT like properties
    if (metadata.contains("attributes") || metadata.contains("image") || metadata.contains("description")) {
        return TokenBadgeType::SHI_30;
    }
    
    return TokenBadgeType::UNKNOWN;
}

QVariantMap TokenBadge::getMockMetadata(const QString& contractAddress)
{
    // Mock implementation - in real app, this would fetch from blockchain
    QVariantMap mockData;
    
    if (contractAddress == "0x1234567890123456789012345678901234567890") {
        mockData = {
            {"standard", SHI_20_STANDARD},
            {"type", "SHI-20"},
            {"name", "Mock Token"},
            {"symbol", "MTK"},
            {"createdBy", SHAHFACTORY_CREATOR},
            {"verified", false}
        };
    } else if (contractAddress == "0x0987654321098765432109876543210987654321") {
        mockData = {
            {"standard", SHI_30_STANDARD},
            {"type", "SHI-30"},
            {"name", "Mock NFT"},
            {"symbol", "MNFT"},
            {"createdBy", SHAHFACTORY_CREATOR},
            {"verified", true}
        };
    }
    
    return mockData;
}

QString TokenBadge::generateStyleSheet() const
{
    QString styleSheet = QString(
        "TokenBadge {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "  padding: 4px 8px;"
        "  font-size: 12px;"
        "  font-weight: 600;"
        "  line-height: 1;"
        "  white-space: nowrap;"
        "  transition: all 0.2s ease-in-out;"
        "}"
        "TokenBadge:hover {"
        "  background-color: %3;"
        "  border-color: %4;"
        "  transform: translateY(-1px);"
        "  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);"
        "}"
    ).arg(m_badgeData.bgColor, m_badgeData.color, 
          getHoverBgColor(m_badgeData.bgColor), getHoverColor(m_badgeData.color));
    
    return styleSheet;
}

QString TokenBadge::getHoverBgColor(const QString& bgColor) const
{
    // Simple hover color calculation
    if (bgColor == SHI_20_BG_COLOR) return "#E5E7EB";
    if (bgColor == SHI_30_BG_COLOR) return "#FDE68A";
    if (bgColor == VERIFIED_BG_COLOR) return "#A7F3D0";
    if (bgColor == UNKNOWN_BG_COLOR) return "#F3F4F6";
    return bgColor;
}

QString TokenBadge::getHoverColor(const QString& color) const
{
    // Simple hover color calculation
    if (color == SHI_20_COLOR) return "#9CA3AF";
    if (color == SHI_30_COLOR) return "#F59E0B";
    if (color == VERIFIED_COLOR) return "#10B981";
    if (color == UNKNOWN_COLOR) return "#9CA3AF";
    return color;
}

// Static methods
TokenBadge* TokenBadge::createSHI20Badge(QWidget *parent)
{
    TokenBadge *badge = new TokenBadge(parent);
    badge->m_badgeData = createBadgeData(TokenBadgeType::SHI_20);
    badge->updateBadgeDisplay();
    return badge;
}

TokenBadge* TokenBadge::createSHI30Badge(QWidget *parent)
{
    TokenBadge *badge = new TokenBadge(parent);
    badge->m_badgeData = createBadgeData(TokenBadgeType::SHI_30);
    badge->updateBadgeDisplay();
    return badge;
}

TokenBadge* TokenBadge::createVerifiedBadge(QWidget *parent)
{
    TokenBadge *badge = new TokenBadge(parent);
    badge->m_badgeData = createBadgeData(TokenBadgeType::VERIFIED);
    badge->updateBadgeDisplay();
    return badge;
}

TokenBadge* TokenBadge::createUnknownBadge(QWidget *parent)
{
    TokenBadge *badge = new TokenBadge(parent);
    badge->m_badgeData = createBadgeData(TokenBadgeType::UNKNOWN);
    badge->updateBadgeDisplay();
    return badge;
}

TokenBadgeType TokenBadge::detectBadgeType(const QVariantMap& metadata)
{
    QString standard = metadata.value("standard").toString();
    QString type = metadata.value("type").toString();
    
    if (standard == SHI_20_STANDARD || type == "SHI-20") {
        return TokenBadgeType::SHI_20;
    }
    
    if (standard == SHI_30_STANDARD || type == "SHI-30") {
        return TokenBadgeType::SHI_30;
    }
    
    return TokenBadgeType::UNKNOWN;
}

TokenBadgeData TokenBadge::createBadgeData(TokenBadgeType type)
{
    TokenBadgeData data;
    data.type = type;
    
    switch (type) {
        case TokenBadgeType::SHI_20:
            data.icon = SHI_20_ICON;
            data.label = SHI_20_LABEL;
            data.tooltip = SHI_20_TOOLTIP;
            data.color = SHI_20_COLOR;
            data.bgColor = SHI_20_BG_COLOR;
            data.qtIcon = ":/icons/token_silver";
            break;
            
        case TokenBadgeType::SHI_30:
            data.icon = SHI_30_ICON;
            data.label = SHI_30_LABEL;
            data.tooltip = SHI_30_TOOLTIP;
            data.color = SHI_30_COLOR;
            data.bgColor = SHI_30_BG_COLOR;
            data.qtIcon = ":/icons/nft_gold";
            break;
            
        case TokenBadgeType::VERIFIED:
            data.icon = VERIFIED_ICON;
            data.label = VERIFIED_LABEL;
            data.tooltip = VERIFIED_TOOLTIP;
            data.color = VERIFIED_COLOR;
            data.bgColor = VERIFIED_BG_COLOR;
            data.qtIcon = ":/icons/verified_check";
            data.verified = true;
            break;
            
        case TokenBadgeType::UNKNOWN:
        default:
            data.icon = UNKNOWN_ICON;
            data.label = UNKNOWN_LABEL;
            data.tooltip = UNKNOWN_TOOLTIP;
            data.color = UNKNOWN_COLOR;
            data.bgColor = UNKNOWN_BG_COLOR;
            data.qtIcon = ":/icons/token_unknown";
            break;
    }
    
    return data;
}

QString TokenBadge::getBadgeIcon(TokenBadgeType type)
{
    switch (type) {
        case TokenBadgeType::SHI_20: return SHI_20_ICON;
        case TokenBadgeType::SHI_30: return SHI_30_ICON;
        case TokenBadgeType::VERIFIED: return VERIFIED_ICON;
        case TokenBadgeType::UNKNOWN:
        default: return UNKNOWN_ICON;
    }
}

QString TokenBadge::getBadgeLabel(TokenBadgeType type)
{
    switch (type) {
        case TokenBadgeType::SHI_20: return SHI_20_LABEL;
        case TokenBadgeType::SHI_30: return SHI_30_LABEL;
        case TokenBadgeType::VERIFIED: return VERIFIED_LABEL;
        case TokenBadgeType::UNKNOWN:
        default: return UNKNOWN_LABEL;
    }
}

QString TokenBadge::getBadgeTooltip(TokenBadgeType type)
{
    switch (type) {
        case TokenBadgeType::SHI_20: return SHI_20_TOOLTIP;
        case TokenBadgeType::SHI_30: return SHI_30_TOOLTIP;
        case TokenBadgeType::VERIFIED: return VERIFIED_TOOLTIP;
        case TokenBadgeType::UNKNOWN:
        default: return UNKNOWN_TOOLTIP;
    }
}

QString TokenBadge::getBadgeColor(TokenBadgeType type)
{
    switch (type) {
        case TokenBadgeType::SHI_20: return SHI_20_COLOR;
        case TokenBadgeType::SHI_30: return SHI_30_COLOR;
        case TokenBadgeType::VERIFIED: return VERIFIED_COLOR;
        case TokenBadgeType::UNKNOWN:
        default: return UNKNOWN_COLOR;
    }
}

QString TokenBadge::getBadgeBgColor(TokenBadgeType type)
{
    switch (type) {
        case TokenBadgeType::SHI_20: return SHI_20_BG_COLOR;
        case TokenBadgeType::SHI_30: return SHI_30_BG_COLOR;
        case TokenBadgeType::VERIFIED: return VERIFIED_BG_COLOR;
        case TokenBadgeType::UNKNOWN:
        default: return UNKNOWN_BG_COLOR;
    }
}

QString TokenBadge::getQtIconPath(TokenBadgeType type)
{
    switch (type) {
        case TokenBadgeType::SHI_20: return ":/icons/token_silver";
        case TokenBadgeType::SHI_30: return ":/icons/nft_gold";
        case TokenBadgeType::VERIFIED: return ":/icons/verified_check";
        case TokenBadgeType::UNKNOWN:
        default: return ":/icons/token_unknown";
    }
}

QString TokenBadge::getQtStyleSheet(TokenBadgeType type)
{
    QString color = getBadgeColor(type);
    QString bgColor = getBadgeBgColor(type);
    
    return QString(
        "background-color: %1;"
        "color: %2;"
        "border: 1px solid %2;"
        "border-radius: 4px;"
        "padding: 2px 6px;"
        "font-size: 10px;"
        "font-weight: bold;"
    ).arg(bgColor, color);
}

// Event handlers
void TokenBadge::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_clickable) {
        emit badgeClicked();
        if (m_onClickCallback) {
            m_onClickCallback();
        }
    }
    QWidget::mousePressEvent(event);
}

void TokenBadge::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}

void TokenBadge::enterEvent(QEvent *event)
{
    emit badgeHovered();
    QWidget::enterEvent(event);
}

void TokenBadge::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
}

void TokenBadge::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

// Private slots
void TokenBadge::onMetadataReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QVariantMap metadata = parseMetadataResponse(data);
        if (!metadata.isEmpty()) {
            setMetadata(metadata);
        } else {
            setError("Invalid metadata response");
        }
    } else {
        setError("Network error: " + reply->errorString());
    }
    
    reply->deleteLater();
}

void TokenBadge::onNetworkError(QNetworkReply::NetworkError error)
{
    setError("Network error: " + QString::number(error));
}

void TokenBadge::onBadgeClicked()
{
    // Handle badge click
}

void TokenBadge::onBadgeHovered()
{
    // Handle badge hover
}

QVariantMap TokenBadge::parseMetadataResponse(const QByteArray& response)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return QVariantMap();
    }
    
    return doc.object().toVariantMap();
}

// TokenBadgeGroup implementation
TokenBadgeGroup::TokenBadgeGroup(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QHBoxLayout(this))
{
    setupLayout();
}

TokenBadgeGroup::~TokenBadgeGroup()
{
}

void TokenBadgeGroup::setupLayout()
{
    m_layout->setSpacing(4);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void TokenBadgeGroup::addBadge(TokenBadge *badge)
{
    if (badge && !m_badges.contains(badge)) {
        m_badges.append(badge);
        m_layout->addWidget(badge);
        emit badgeAdded(badge);
    }
}

void TokenBadgeGroup::removeBadge(TokenBadge *badge)
{
    if (badge && m_badges.contains(badge)) {
        m_badges.removeOne(badge);
        m_layout->removeWidget(badge);
        badge->setParent(nullptr);
        emit badgeRemoved(badge);
    }
}

void TokenBadgeGroup::clearBadges()
{
    for (TokenBadge *badge : m_badges) {
        m_layout->removeWidget(badge);
        badge->setParent(nullptr);
    }
    m_badges.clear();
}

void TokenBadgeGroup::setSpacing(int spacing)
{
    m_layout->setSpacing(spacing);
}

void TokenBadgeGroup::setAlignment(Qt::Alignment alignment)
{
    m_layout->setAlignment(alignment);
}

QList<TokenBadge*> TokenBadgeGroup::getBadges() const
{
    return m_badges;
}

int TokenBadgeGroup::getBadgeCount() const
{
    return m_badges.size();
}

// TokenBadgeList implementation
TokenBadgeList::TokenBadgeList(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout(this))
    , m_badgeSize(TokenBadgeSize::MEDIUM)
{
    setupLayout();
}

TokenBadgeList::~TokenBadgeList()
{
}

void TokenBadgeList::setupLayout()
{
    m_layout->setSpacing(8);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setAlignment(Qt::AlignTop);
}

void TokenBadgeList::addToken(const QString& name, const QVariantMap& metadata, const QString& contractAddress)
{
    TokenBadge *badge = new TokenBadge(this);
    badge->setBadgeSize(m_badgeSize);
    badge->setMetadata(metadata);
    if (!contractAddress.isEmpty()) {
        badge->setContractAddress(contractAddress);
    }
    
    addToken(name, badge);
}

void TokenBadgeList::addToken(const QString& name, TokenBadge *badge)
{
    if (badge) {
        m_tokens.append(qMakePair(name, badge));
        createTokenItem(name, badge);
    }
}

void TokenBadgeList::removeToken(const QString& name)
{
    for (int i = 0; i < m_tokens.size(); ++i) {
        if (m_tokens[i].first == name) {
            TokenBadge *badge = m_tokens[i].second;
            m_layout->removeWidget(badge);
            badge->setParent(nullptr);
            m_tokens.removeAt(i);
            break;
        }
    }
}

void TokenBadgeList::clearTokens()
{
    for (const auto& pair : m_tokens) {
        m_layout->removeWidget(pair.second);
        pair.second->setParent(nullptr);
    }
    m_tokens.clear();
}

void TokenBadgeList::setBadgeSize(TokenBadgeSize size)
{
    m_badgeSize = size;
    for (const auto& pair : m_tokens) {
        pair.second->setBadgeSize(size);
    }
}

QList<QPair<QString, TokenBadge*>> TokenBadgeList::getTokens() const
{
    return m_tokens;
}

int TokenBadgeList::getTokenCount() const
{
    return m_tokens.size();
}

void TokenBadgeList::createTokenItem(const QString& name, TokenBadge *badge)
{
    QWidget *itemWidget = new QWidget(this);
    QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
    
    QLabel *nameLabel = new QLabel(name, itemWidget);
    nameLabel->setStyleSheet("font-weight: bold; margin-right: 8px;");
    
    itemLayout->addWidget(nameLabel);
    itemLayout->addWidget(badge);
    itemLayout->addStretch();
    itemLayout->setContentsMargins(0, 0, 0, 0);
    
    m_layout->addWidget(itemWidget);
    
    // Connect badge click to token click
    connect(badge, &TokenBadge::badgeClicked, [this, name, badge]() {
        emit tokenClicked(name, badge);
    });
}

// TokenBadgeManager implementation
TokenBadgeManager* TokenBadgeManager::s_instance = nullptr;

TokenBadgeManager* TokenBadgeManager::instance()
{
    if (!s_instance) {
        s_instance = new TokenBadgeManager();
    }
    return s_instance;
}

TokenBadgeManager::TokenBadgeManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    loadCachedMetadata();
}

TokenBadgeManager::~TokenBadgeManager()
{
    saveCachedMetadata();
}

TokenBadge* TokenBadgeManager::createBadge(const QVariantMap& metadata, QWidget *parent)
{
    return new TokenBadge(metadata, parent);
}

TokenBadge* TokenBadgeManager::createBadge(const QString& contractAddress, QWidget *parent)
{
    return new TokenBadge(contractAddress, parent);
}

TokenBadge* TokenBadgeManager::createBadge(TokenBadgeType type, QWidget *parent)
{
    TokenBadge *badge = new TokenBadge(parent);
    badge->m_badgeData = TokenBadge::createBadgeData(type);
    badge->updateBadgeDisplay();
    return badge;
}

void TokenBadgeManager::storeMetadata(const QString& contractAddress, const QVariantMap& metadata)
{
    m_metadataCache[contractAddress] = metadata;
    emit metadataStored(contractAddress);
}

QVariantMap TokenBadgeManager::getMetadata(const QString& contractAddress)
{
    return m_metadataCache.value(contractAddress);
}

void TokenBadgeManager::updateVerificationStatus(const QString& contractAddress, bool verified)
{
    if (m_metadataCache.contains(contractAddress)) {
        m_metadataCache[contractAddress]["verified"] = verified;
        emit verificationStatusChanged(contractAddress, verified);
    }
}

QList<QString> TokenBadgeManager::getVerifiedTokens()
{
    QList<QString> verifiedTokens;
    for (auto it = m_metadataCache.begin(); it != m_metadataCache.end(); ++it) {
        if (it.value().value("verified", false).toBool()) {
            verifiedTokens.append(it.key());
        }
    }
    return verifiedTokens;
}

QList<QString> TokenBadgeManager::getTokensByType(TokenBadgeType type)
{
    QList<QString> tokens;
    for (auto it = m_metadataCache.begin(); it != m_metadataCache.end(); ++it) {
        if (TokenBadge::detectBadgeType(it.value()) == type) {
            tokens.append(it.key());
        }
    }
    return tokens;
}

void TokenBadgeManager::clearCache()
{
    m_metadataCache.clear();
    m_badgeCache.clear();
}

void TokenBadgeManager::preloadMetadata(const QStringList& contractAddresses)
{
    for (const QString& address : contractAddresses) {
        if (!m_metadataCache.contains(address)) {
            // In real implementation, this would fetch from blockchain
            QVariantMap mockMetadata = TokenBadge::getMockMetadata(address);
            if (!mockMetadata.isEmpty()) {
                storeMetadata(address, mockMetadata);
            }
        }
    }
}

void TokenBadgeManager::loadCachedMetadata()
{
    QString cacheFile = getCacheFilePath();
    QFile file(cacheFile);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QVariantMap cache = doc.object().toVariantMap();
        m_metadataCache = cache;
    }
}

void TokenBadgeManager::saveCachedMetadata()
{
    QString cacheFile = getCacheFilePath();
    QDir().mkpath(QFileInfo(cacheFile).absolutePath());
    
    QFile file(cacheFile);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc = QJsonDocument::fromVariant(m_metadataCache);
        file.write(doc.toJson());
    }
}

QString TokenBadgeManager::getCacheFilePath() const
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return cacheDir + "/shahcoin/token_metadata_cache.json";
}
