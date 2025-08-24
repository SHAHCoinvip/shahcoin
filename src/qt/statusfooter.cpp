// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/statusfooter.h>

#include <qt/clientmodel.h>
#include <qt/walletmodel.h>
#include <qt/platformstyle.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>
#include <qt/diagnosticspanel.h>

#include <interfaces/node.h>
#include <interfaces/wallet.h>
#include <util/translation.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>
#include <QToolTip>
#include <QVBoxLayout>

StatusFooter::StatusFooter(const PlatformStyle* platformStyle, QWidget* parent)
    : QFrame(parent)
    , m_clientModel(nullptr)
    , m_walletModel(nullptr)
    , m_platformStyle(platformStyle)
    , m_cachedBlockHeight(-1)
    , m_cachedHeaderHeight(-1)
    , m_cachedIsSyncing(false)
    , m_cachedPeerCount(-1)
    , m_cachedMiningActive(false)
    , m_cachedMiningAlgorithm("")
    , m_cachedStakingActive(false)
    , m_cachedStakedAmount(0)
    , m_cachedAlertMessage("")
    , m_cachedHasAlert(false)
    , m_diagnosticsPanel(nullptr)
{
    setupUI();
    connectSignals();
    
    // Set up update timer (every 5 seconds)
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &StatusFooter::updateStatus);
    m_updateTimer->start(5000); // 5 seconds
    
    // Initial update
    updateStatus();
}

bool StatusFooter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            if (obj == m_chainSyncFrame) {
                QToolTip::showText(mouseEvent->globalPos(), formatSyncStatus(), this);
                return true;
            } else if (obj == m_miningFrame) {
                QToolTip::showText(mouseEvent->globalPos(), formatMiningStatus(), this);
                return true;
            } else if (obj == m_stakingFrame) {
                QToolTip::showText(mouseEvent->globalPos(), formatStakingStatus(), this);
                return true;
            } else if (obj == m_peerFrame) {
                QToolTip::showText(mouseEvent->globalPos(), formatPeerStatus(), this);
                return true;
            } else if (obj == this) {
                // Click on the footer itself opens diagnostics panel
                openDiagnosticsPanel();
                return true;
            }
        }
    }
    return QFrame::eventFilter(obj, event);
}

StatusFooter::~StatusFooter()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
}

void StatusFooter::setClientModel(ClientModel* clientModel)
{
    m_clientModel = clientModel;
    if (m_clientModel) {
        // Connect to client model signals for real-time updates
        connect(m_clientModel, &ClientModel::numConnectionsChanged, this, &StatusFooter::onPeerCountChanged);
        connect(m_clientModel, &ClientModel::numBlocksChanged, this, &StatusFooter::onChainSyncChanged);
        connect(m_clientModel, &ClientModel::strMiscWarningChanged, this, &StatusFooter::onAlertStatusChanged);
    }
    updateStatus();
}

void StatusFooter::setWalletModel(WalletModel* walletModel)
{
    m_walletModel = walletModel;
    if (m_walletModel) {
        // Connect to wallet model signals for staking updates
        connect(m_walletModel, &WalletModel::balanceChanged, this, &StatusFooter::onStakingStatusChanged);
    }
    updateStatus();
}

void StatusFooter::setupUI()
{
    // Main layout
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 4, 8, 4);
    m_layout->setSpacing(12);
    
    // Set background color (slightly darker than main window)
    setStyleSheet("StatusFooter { background-color: #f0f0f0; border-top: 1px solid #d0d0d0; }");
    
    // Chain Sync Section
    m_chainSyncFrame = new QFrame();
    m_chainSyncFrame->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout* chainSyncLayout = new QHBoxLayout(m_chainSyncFrame);
    chainSyncLayout->setContentsMargins(4, 2, 4, 2);
    chainSyncLayout->setSpacing(4);
    
    m_chainSyncIcon = new QLabel("⛓");
    m_chainSyncIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50;");
    m_chainSyncLabel = new QLabel("Syncing...");
    m_chainSyncLabel->setStyleSheet("font-size: 11px; color: #34495e; font-weight: 500;");
    
    chainSyncLayout->addWidget(m_chainSyncIcon);
    chainSyncLayout->addWidget(m_chainSyncLabel);
    
    // Mining Section
    m_miningFrame = new QFrame();
    m_miningFrame->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout* miningLayout = new QHBoxLayout(m_miningFrame);
    miningLayout->setContentsMargins(4, 2, 4, 2);
    miningLayout->setSpacing(4);
    
    m_miningIcon = new QLabel("⛏️");
    m_miningIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50;");
    m_miningLabel = new QLabel("Mining: Inactive");
    m_miningLabel->setStyleSheet("font-size: 11px; color: #34495e; font-weight: 500;");
    
    miningLayout->addWidget(m_miningIcon);
    miningLayout->addWidget(m_miningLabel);
    
    // Staking Section
    m_stakingFrame = new QFrame();
    m_stakingFrame->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout* stakingLayout = new QHBoxLayout(m_stakingFrame);
    stakingLayout->setContentsMargins(4, 2, 4, 2);
    stakingLayout->setSpacing(4);
    
    m_stakingIcon = new QLabel("🌱");
    m_stakingIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50;");
    m_stakingLabel = new QLabel("Staking: Inactive");
    m_stakingLabel->setStyleSheet("font-size: 11px; color: #34495e; font-weight: 500;");
    
    stakingLayout->addWidget(m_stakingIcon);
    stakingLayout->addWidget(m_stakingLabel);
    
    // Peer Section
    m_peerFrame = new QFrame();
    m_peerFrame->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout* peerLayout = new QHBoxLayout(m_peerFrame);
    peerLayout->setContentsMargins(4, 2, 4, 2);
    peerLayout->setSpacing(4);
    
    m_peerIcon = new QLabel("📶");
    m_peerIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50;");
    m_peerLabel = new QLabel("Peers: 0");
    m_peerLabel->setStyleSheet("font-size: 11px; color: #34495e; font-weight: 500;");
    
    peerLayout->addWidget(m_peerIcon);
    peerLayout->addWidget(m_peerLabel);
    
    // Alert Section
    m_alertFrame = new QFrame();
    m_alertFrame->setFrameStyle(QFrame::NoFrame);
    m_alertFrame->hide(); // Hidden by default
    QHBoxLayout* alertLayout = new QHBoxLayout(m_alertFrame);
    alertLayout->setContentsMargins(4, 2, 4, 2);
    alertLayout->setSpacing(4);
    
    m_alertIcon = new QLabel("⚠️");
    m_alertIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #e74c3c;");
    m_alertLabel = new QLabel("");
    m_alertLabel->setStyleSheet("font-size: 11px; color: #e74c3c; font-weight: 500;");
    
    alertLayout->addWidget(m_alertIcon);
    alertLayout->addWidget(m_alertLabel);
    
    // Add sections to main layout
    m_layout->addWidget(m_chainSyncFrame);
    m_layout->addWidget(m_miningFrame);
    m_layout->addWidget(m_stakingFrame);
    m_layout->addWidget(m_peerFrame);
    m_layout->addStretch(); // Push alert to the right
    m_layout->addWidget(m_alertFrame);
    
    // Set tooltips
    m_chainSyncFrame->setToolTip(tr("Blockchain synchronization status"));
    m_miningFrame->setToolTip(tr("Mining status and algorithm"));
    m_stakingFrame->setToolTip(tr("Proof of Stake status"));
    m_peerFrame->setToolTip(tr("Network peer connections"));
    m_alertFrame->setToolTip(tr("System alerts and warnings"));
}

void StatusFooter::connectSignals()
{
    // Set up clickable frames with tooltips
                    m_chainSyncFrame->setCursor(Qt::PointingHandCursor);
                m_miningFrame->setCursor(Qt::PointingHandCursor);
                m_stakingFrame->setCursor(Qt::PointingHandCursor);
                m_peerFrame->setCursor(Qt::PointingHandCursor);

                // Make the entire footer clickable for diagnostics
                setCursor(Qt::PointingHandCursor);
    
    // Install event filters for click handling
    m_chainSyncFrame->installEventFilter(this);
    m_miningFrame->installEventFilter(this);
    m_stakingFrame->installEventFilter(this);
    m_peerFrame->installEventFilter(this);

    // Install event filter for the entire footer
    installEventFilter(this);
}

void StatusFooter::updateStatus()
{
    updateChainSyncDisplay();
    updateMiningDisplay();
    updateStakingDisplay();
    updatePeerDisplay();
    updateAlertDisplay();
}

void StatusFooter::onChainSyncChanged()
{
    updateChainSyncDisplay();
}

void StatusFooter::onMiningStatusChanged()
{
    updateMiningDisplay();
}

void StatusFooter::onStakingStatusChanged()
{
    updateStakingDisplay();
}

void StatusFooter::onPeerCountChanged()
{
    updatePeerDisplay();
}

void StatusFooter::onAlertStatusChanged()
{
    updateAlertDisplay();
}

void StatusFooter::updateChainSyncDisplay()
{
    if (!m_clientModel) {
        m_chainSyncLabel->setText("Not connected");
        m_chainSyncIcon->setText("❌");
        return;
    }
    
    int blockHeight = m_clientModel->getNumBlocks();
    int headerHeight = m_clientModel->getHeaderTipHeight();
    bool isSyncing = blockHeight < headerHeight;
    
    // Only update if values changed
    if (blockHeight != m_cachedBlockHeight || headerHeight != m_cachedHeaderHeight || isSyncing != m_cachedIsSyncing) {
        m_cachedBlockHeight = blockHeight;
        m_cachedHeaderHeight = headerHeight;
        m_cachedIsSyncing = isSyncing;
        
        if (isSyncing) {
            double syncProgress = (double)blockHeight / headerHeight * 100.0;
            m_chainSyncLabel->setText(QString("Syncing: %1%").arg(syncProgress, 0, 'f', 1));
            m_chainSyncIcon->setText("⬇️");
            m_chainSyncIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #f39c12;");
        } else {
            m_chainSyncLabel->setText("Synced");
            m_chainSyncIcon->setText("⛓");
            m_chainSyncIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #27ae60;");
        }
    }
}

void StatusFooter::updateMiningDisplay()
{
    // For now, we'll use placeholder data since mining status isn't directly exposed
    // In a real implementation, this would connect to the mining module
    bool miningActive = false;
    QString algorithm = "SHA256d";
    
    // Check if mining is active (this would be implemented based on actual mining status)
    if (m_clientModel) {
        // This is a placeholder - in real implementation, get from mining module
        miningActive = false; // g_miningManager->IsMiningActive();
        algorithm = "SHA256d"; // g_miningManager->GetCurrentAlgorithm();
    }
    
    if (miningActive != m_cachedMiningActive || algorithm != m_cachedMiningAlgorithm) {
        m_cachedMiningActive = miningActive;
        m_cachedMiningAlgorithm = algorithm;
        
        if (miningActive) {
            m_miningLabel->setText(QString("Mining: %1").arg(algorithm));
            m_miningIcon->setText("⛏️");
            m_miningIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #27ae60;");
        } else {
            m_miningLabel->setText("Mining: Inactive");
            m_miningIcon->setText("⛏️");
            m_miningIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #95a5a6;");
        }
    }
}

void StatusFooter::updateStakingDisplay()
{
    bool stakingActive = false;
    CAmount stakedAmount = 0;
    
    if (m_walletModel) {
        // Get staking information from wallet model
        stakedAmount = m_walletModel->getStakedBalance();
        stakingActive = stakedAmount > 0;
    }
    
    if (stakingActive != m_cachedStakingActive || stakedAmount != m_cachedStakedAmount) {
        m_cachedStakingActive = stakingActive;
        m_cachedStakedAmount = stakedAmount;
        
        if (stakingActive) {
            QString formattedAmount = ShahcoinUnits::formatWithUnit(
                m_walletModel ? m_walletModel->getOptionsModel()->getDisplayUnit() : ShahcoinUnit::SHAH,
                stakedAmount,
                false,
                ShahcoinUnits::SeparatorStyle::STANDARD
            );
            m_stakingLabel->setText(QString("Staking: %1").arg(formattedAmount));
            m_stakingIcon->setText("🌱");
            m_stakingIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #27ae60;");
        } else {
            m_stakingLabel->setText("Staking: Inactive");
            m_stakingIcon->setText("🌱");
            m_stakingIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #95a5a6;");
        }
    }
}

void StatusFooter::updatePeerDisplay()
{
    if (!m_clientModel) {
        m_peerLabel->setText("Peers: 0");
        return;
    }
    
    int peerCount = m_clientModel->getNumConnections();
    
    if (peerCount != m_cachedPeerCount) {
        m_cachedPeerCount = peerCount;
        m_peerLabel->setText(QString("Peers: %1").arg(peerCount));
        
        // Color code based on peer count
        if (peerCount >= 8) {
            m_peerIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #27ae60;");
        } else if (peerCount >= 4) {
            m_peerIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #f39c12;");
        } else {
            m_peerIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #e74c3c;");
        }
    }
}

void StatusFooter::updateAlertDisplay()
{
    QString alertMessage = "";
    bool hasAlert = false;
    
    if (m_clientModel) {
        alertMessage = m_clientModel->getStatusBarWarnings();
        hasAlert = !alertMessage.isEmpty();
    }
    
    if (hasAlert != m_cachedHasAlert || alertMessage != m_cachedAlertMessage) {
        m_cachedHasAlert = hasAlert;
        m_cachedAlertMessage = alertMessage;
        
        if (hasAlert) {
            m_alertLabel->setText(alertMessage);
            m_alertFrame->show();
            m_alertIcon->setText("⚠️");
            m_alertIcon->setStyleSheet("font-size: 14px; font-weight: bold; color: #e74c3c;");
        } else {
            m_alertFrame->hide();
        }
    }
}

QString StatusFooter::formatSyncStatus() const
{
    if (!m_clientModel) {
        return tr("Not connected to network");
    }
    
    int blockHeight = m_clientModel->getNumBlocks();
    int headerHeight = m_clientModel->getHeaderTipHeight();
    
    if (blockHeight < headerHeight) {
        double progress = (double)blockHeight / headerHeight * 100.0;
        return tr("Syncing blockchain: %1 of %2 blocks (%3%)")
               .arg(blockHeight)
               .arg(headerHeight)
               .arg(progress, 0, 'f', 1);
    } else {
        return tr("Blockchain synchronized: %1 blocks").arg(blockHeight);
    }
}

QString StatusFooter::formatMiningStatus() const
{
    if (m_cachedMiningActive) {
        return tr("Mining active using %1 algorithm").arg(m_cachedMiningAlgorithm);
    } else {
        return tr("Mining inactive - use CLI or external miner");
    }
}

QString StatusFooter::formatStakingStatus() const
{
    if (m_cachedStakingActive) {
        QString formattedAmount = ShahcoinUnits::formatWithUnit(
            m_walletModel ? m_walletModel->getOptionsModel()->getDisplayUnit() : ShahcoinUnit::SHAH,
            m_cachedStakedAmount,
            false,
            ShahcoinUnits::SeparatorStyle::STANDARD
        );
        return tr("Staking active with %1 locked").arg(formattedAmount);
    } else {
        return tr("Staking inactive - minimum 333 SHAH required");
    }
}

QString StatusFooter::formatPeerStatus() const
{
    if (!m_clientModel) {
        return tr("Not connected to network");
    }
    
    int peerCount = m_clientModel->getNumConnections();
    return tr("Connected to %1 peers").arg(peerCount);
}

QString StatusFooter::formatAlertStatus() const
{
    if (m_cachedHasAlert) {
        return tr("Alert: %1").arg(m_cachedAlertMessage);
    } else {
        return tr("No active alerts");
    }
}

void StatusFooter::openDiagnosticsPanel()
{
    // Create diagnostics panel if it doesn't exist
    if (!m_diagnosticsPanel) {
        m_diagnosticsPanel = new DiagnosticsPanel(m_platformStyle, this);
        m_diagnosticsPanel->setWalletModel(m_walletModel);
    }

    // Show the diagnostics panel
    m_diagnosticsPanel->show();
    m_diagnosticsPanel->raise();
    m_diagnosticsPanel->activateWindow();
}
