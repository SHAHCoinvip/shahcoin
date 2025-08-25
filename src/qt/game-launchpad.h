// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_GAME_LAUNCHPAD_H
#define SHAHCOIN_QT_GAME_LAUNCHPAD_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWebEngineView>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>

namespace Ui {
class GameLaunchpad;
}

/**
 * Game Launchpad Widget
 * 
 * Provides an in-wallet interface for discovering, installing, and launching
 * Shahcoin-powered games with automatic wallet connection and NFT integration.
 */
class GameLaunchpad : public QWidget
{
    Q_OBJECT

public:
    explicit GameLaunchpad(QWidget *parent = nullptr);
    ~GameLaunchpad();

    // Game management methods
    void discoverGames();
    void installGame(const QString& gameId);
    void launchGame(const QString& gameId);
    void uninstallGame(const QString& gameId);
    void updateGame(const QString& gameId);

    // Wallet integration methods
    void connectWalletToGame(const QString& gameId);
    void syncNFTInventory(const QString& gameId);
    void syncTokenBalances(const QString& gameId);

    // Data management
    void loadGameLibrary();
    void saveGameSettings();
    void exportGameData();

    // UI methods
    void updateGameList();
    void updateInstalledGames();
    void updateGameDetails();
    void refreshGameStore();

signals:
    void gameInstalled(const QString& gameId);
    void gameLaunched(const QString& gameId);
    void gameUninstalled(const QString& gameId);
    void walletConnected(const QString& gameId);
    void nftSynced(const QString& gameId, const QJsonArray& nfts);

public slots:
    void onGameInstalled();
    void onGameLaunched();
    void onGameUninstalled();
    void onWalletConnected();
    void onNFTsSynced();

private slots:
    void onDiscoverButtonClicked();
    void onInstallButtonClicked();
    void onLaunchButtonClicked();
    void onUninstallButtonClicked();
    void onUpdateButtonClicked();
    void onConnectWalletButtonClicked();
    void onSyncNFTsButtonClicked();
    void onRefreshButtonClicked();
    void onSettingsButtonClicked();
    void onSearchTextChanged(const QString& text);
    void onCategoryChanged(int index);
    void onSortByChanged(int index);
    void onFilterChanged();
    void onGameItemClicked(QListWidgetItem* item);
    void onGameItemDoubleClicked(QListWidgetItem* item);
    void onGameItemRightClicked(const QPoint& pos);
    void onNetworkResponse(QNetworkReply* reply);
    void onNetworkError(QNetworkReply::NetworkError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    Ui::GameLaunchpad *ui;
    QNetworkAccessManager *m_networkManager;
    QTimer *m_updateTimer;
    QProcess *m_gameProcess;
    
    // Data structures
    QJsonArray m_availableGames;
    QJsonArray m_installedGames;
    QJsonArray m_favoriteGames;
    QJsonObject m_gameSettings;
    QJsonObject m_walletConnections;
    QJsonObject m_nftInventories;
    
    // Configuration
    QString m_gamesDirectory;
    QString m_downloadDirectory;
    bool m_autoConnectWallet;
    bool m_autoSyncNFTs;
    bool m_autoUpdateGames;
    int m_maxConcurrentDownloads;
    
    // UI components
    QTabWidget *m_tabWidget;
    QListWidget *m_gameListWidget;
    QListWidget *m_installedGamesWidget;
    QListWidget *m_favoriteGamesWidget;
    QTableWidget *m_gameDetailsTable;
    QTextEdit *m_gameDescriptionText;
    QWebEngineView *m_gamePreviewView;
    QTreeWidget *m_nftInventoryTree;
    QTreeWidget *m_tokenBalancesTree;
    
    // Controls
    QLineEdit *m_searchEdit;
    QComboBox *m_categoryCombo;
    QComboBox *m_sortByCombo;
    QCheckBox *m_verifiedOnlyCheck;
    QCheckBox *m_freeOnlyCheck;
    QCheckBox *m_installedOnlyCheck;
    QPushButton *m_discoverButton;
    QPushButton *m_installButton;
    QPushButton *m_launchButton;
    QPushButton *m_uninstallButton;
    QPushButton *m_updateButton;
    QPushButton *m_connectWalletButton;
    QPushButton *m_syncNFTsButton;
    QPushButton *m_refreshButton;
    QPushButton *m_settingsButton;
    
    // Progress indicators
    QProgressBar *m_downloadProgress;
    QProgressBar *m_installProgress;
    QLabel *m_statusLabel;
    QLabel *m_downloadSpeedLabel;
    QLabel *m_remainingTimeLabel;
    
    // Game cards
    QList<QFrame*> m_gameCards;
    QString m_selectedGameId;
    bool m_isDownloading;
    bool m_isInstalling;
    bool m_isLaunching;

    // Setup methods
    void setupUI();
    void setupGameList();
    void setupInstalledGames();
    void setupGameDetails();
    void setupConnections();
    void setupNetworkManager();
    void setupGameProcess();
    
    // Game discovery methods
    void fetchGameStore();
    void fetchGameDetails(const QString& gameId);
    void fetchGameReviews(const QString& gameId);
    void fetchGameUpdates();
    void processGameStoreResponse(const QJsonArray& games);
    
    // Game installation methods
    void downloadGame(const QString& gameId);
    void installGameFiles(const QString& gameId);
    void verifyGameInstallation(const QString& gameId);
    void createGameShortcut(const QString& gameId);
    void updateGameRegistry(const QString& gameId);
    
    // Game launching methods
    void prepareGameLaunch(const QString& gameId);
    void launchGameProcess(const QString& gameId);
    void monitorGameProcess();
    void handleGameExit();
    
    // Wallet integration methods
    void establishWalletConnection(const QString& gameId);
    void sendWalletData(const QString& gameId);
    void receiveGameData(const QString& gameId);
    void syncGameState(const QString& gameId);
    
    // NFT integration methods
    void fetchNFTInventory(const QString& gameId);
    void syncNFTMetadata(const QString& gameId);
    void updateNFTBalances(const QString& gameId);
    void handleNFTTransactions(const QString& gameId);
    
    // UI update methods
    void createGameCard(const QJsonObject& game);
    void updateGameCard(const QString& gameId);
    void populateGameDetails(const QJsonObject& game);
    void updateDownloadProgress(int progress);
    void updateInstallProgress(int progress);
    void showGamePreview(const QString& gameId);
    
    // Network methods
    void sendGameStoreRequest();
    void sendGameDownloadRequest(const QString& gameId);
    void sendGameUpdateRequest(const QString& gameId);
    void sendWalletConnectionRequest(const QString& gameId);
    void sendNFTSyncRequest(const QString& gameId);
    
    // Utility methods
    QString getGamePath(const QString& gameId);
    QString getGameExecutable(const QString& gameId);
    bool isGameInstalled(const QString& gameId);
    bool isGameRunning(const QString& gameId);
    bool isGameUpToDate(const QString& gameId);
    QString formatFileSize(qint64 bytes);
    QString formatDownloadSpeed(qint64 bytesPerSecond);
    QString formatRemainingTime(qint64 remainingBytes, qint64 bytesPerSecond);
    QColor getGameStatusColor(const QString& status);
    QIcon getGameIcon(const QString& gameId);
    
    // Configuration methods
    void loadGameSettings();
    void saveGameSettings();
    void applyGameSettings();
    void resetGameSettings();
    
    // File management methods
    void createGameDirectories();
    void cleanupGameFiles(const QString& gameId);
    void backupGameData(const QString& gameId);
    void restoreGameData(const QString& gameId);
    
    // Security methods
    void verifyGameSignature(const QString& gameId);
    void validateGameFiles(const QString& gameId);
    void checkGamePermissions(const QString& gameId);
    void scanGameForMalware(const QString& gameId);
    
    // Performance methods
    void optimizeGameLaunch(const QString& gameId);
    void monitorGamePerformance(const QString& gameId);
    void adjustGameSettings(const QString& gameId);
    void cleanupGameResources(const QString& gameId);
};

/**
 * Game Information Structure
 */
struct GameInfo {
    QString id;
    QString name;
    QString description;
    QString developer;
    QString version;
    QString category;
    QString genre;
    QString platform;
    QString engine;
    QString size;
    QString price;
    QString currency;
    bool isFree;
    bool isVerified;
    bool isInstalled;
    bool isRunning;
    bool isUpToDate;
    QString status;
    QString downloadUrl;
    QString executablePath;
    QString installPath;
    QJsonObject metadata;
    QJsonArray screenshots;
    QJsonArray videos;
    QJsonArray reviews;
    QJsonArray requirements;
    QJsonObject walletIntegration;
    QJsonArray supportedNFTs;
    QJsonArray supportedTokens;
    double rating;
    int downloadCount;
    int installCount;
    QDateTime releaseDate;
    QDateTime lastUpdate;
    QDateTime lastPlayed;
};

/**
 * Game Installation Status
 */
enum GameInstallStatus {
    NotInstalled,
    Downloading,
    Installing,
    Installed,
    Updating,
    Error,
    Verifying
};

/**
 * Game Launch Status
 */
enum GameLaunchStatus {
    NotRunning,
    Starting,
    Running,
    Crashed,
    Exited
};

/**
 * Wallet Connection Status
 */
enum WalletConnectionStatus {
    Disconnected,
    Connecting,
    Connected,
    Syncing,
    Error
};

/**
 * NFT Sync Status
 */
enum NFTSyncStatus {
    NotSynced,
    Syncing,
    Synced,
    Error
};

#endif // SHAHCOIN_QT_GAME_LAUNCHPAD_H
