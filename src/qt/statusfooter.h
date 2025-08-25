// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_STATUSFOOTER_H
#define SHAHCOIN_QT_STATUSFOOTER_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QFrame>
#include <QMouseEvent>

class ClientModel;
class WalletModel;
class PlatformStyle;
class DiagnosticsPanel;

/**
 * Status Footer Widget
 * 
 * Displays real-time information at the bottom of the Shahcoin GUI:
 * - Chain Sync status
 * - Mining status and algorithm
 * - Staking status and amount
 * - Peer count
 * - Alerts and warnings
 */
class StatusFooter : public QFrame
{
    Q_OBJECT

public:
    explicit StatusFooter(const PlatformStyle* platformStyle, QWidget* parent = nullptr);
    ~StatusFooter();

    void setClientModel(ClientModel* clientModel);
    void setWalletModel(WalletModel* walletModel);

private Q_SLOTS:
    void openDiagnosticsPanel();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

public Q_SLOTS:
    void updateStatus();

private Q_SLOTS:
    void onChainSyncChanged();
    void onMiningStatusChanged();
    void onStakingStatusChanged();
    void onPeerCountChanged();
    void onAlertStatusChanged();

private:
    void setupUI();
    void connectSignals();
    void updateChainSyncDisplay();
    void updateMiningDisplay();
    void updateStakingDisplay();
    void updatePeerDisplay();
    void updateAlertDisplay();
    
    QString formatSyncStatus() const;
    QString formatMiningStatus() const;
    QString formatStakingStatus() const;
    QString formatPeerStatus() const;
    QString formatAlertStatus() const;

    // UI Components
    QHBoxLayout* m_layout;
    
    // Chain Sync Section
    QLabel* m_chainSyncIcon;
    QLabel* m_chainSyncLabel;
    QFrame* m_chainSyncFrame;
    
    // Mining Section
    QLabel* m_miningIcon;
    QLabel* m_miningLabel;
    QFrame* m_miningFrame;
    
    // Staking Section
    QLabel* m_stakingIcon;
    QLabel* m_stakingLabel;
    QFrame* m_stakingFrame;
    
    // Peer Section
    QLabel* m_peerIcon;
    QLabel* m_peerLabel;
    QFrame* m_peerFrame;
    
    // Alert Section
    QLabel* m_alertIcon;
    QLabel* m_alertLabel;
    QFrame* m_alertFrame;
    
    // Models
    ClientModel* m_clientModel;
    WalletModel* m_walletModel;
    const PlatformStyle* m_platformStyle;
    
    // Update Timer
    QTimer* m_updateTimer;
    
    // Cached values for efficient updates
    int m_cachedBlockHeight;
    int m_cachedHeaderHeight;
    bool m_cachedIsSyncing;
    int m_cachedPeerCount;
    bool m_cachedMiningActive;
    QString m_cachedMiningAlgorithm;
    bool m_cachedStakingActive;
    CAmount m_cachedStakedAmount;
    QString m_cachedAlertMessage;
    bool m_cachedHasAlert;

    // Diagnostics Panel
    DiagnosticsPanel* m_diagnosticsPanel;
};

#endif // SHAHCOIN_QT_STATUSFOOTER_H
