#ifndef SHAHCOIN_SPV_SPVCLIENT_H
#define SHAHCOIN_SPV_SPVCLIENT_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>

#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/script.h>
#include <script/standard.h>
#include <crypto/sha256.h>
#include <util/time.h>
#include <chainparams.h>
#include <validation.h>
#include <txdb.h>
#include <index/txindex.h>

class WalletModel;
class CBlockHeader;
class CBlockIndex;
class CMerkleBlock;

/**
 * SPV Client for Simplified Payment Verification
 * 
 * Allows Shahcoin to run on mobile or low-resource devices without full node sync.
 * Implements lightweight peer verification with Bloom filters and Merkle proofs.
 */
class SPVClient : public QObject
{
    Q_OBJECT

public:
    enum SPVMode {
        FullNode,      // Full node mode (default)
        SPVLight,      // Light SPV mode
        SPVUltraLight  // Ultra-light SPV mode (mobile)
    };

    enum SyncStatus {
        Disconnected,
        Connecting,
        SyncingHeaders,
        SyncingBlocks,
        Synced,
        Error
    };

    struct SPVConfig {
        SPVMode mode;
        QStringList serverUrls;
        int maxConnections;
        int syncTimeout;
        bool useBloomFilters;
        bool verifyHeaders;
        int maxHeaders;
        int maxBlocks;
    };

    struct SPVHeader {
        uint256 hash;
        uint256 prevHash;
        uint32_t nTime;
        uint32_t nshahbits;
        uint32_t nNonce;
        uint32_t nHeight;
        bool isValid;
    };

    struct SPVBlock {
        uint256 hash;
        uint256 prevHash;
        uint32_t nHeight;
        uint32_t nTime;
        QList<uint256> txHashes;
        CMerkleBlock merkleBlock;
        bool isValid;
    };

    struct SPVTransaction {
        uint256 txid;
        uint256 blockHash;
        uint32_t blockHeight;
        uint32_t nTime;
        CAmount amount;
        QString address;
        bool isSpent;
        bool isConfirmed;
    };

    explicit SPVClient(QObject* parent = nullptr);
    ~SPVClient();

    // Configuration
    void setConfig(const SPVConfig& config);
    SPVConfig getConfig() const;
    void setWalletModel(WalletModel* model);

    // Connection and sync
    bool connect();
    void disconnect();
    bool isConnected() const;
    SyncStatus getSyncStatus() const;
    int getSyncProgress() const;

    // Header management
    bool syncHeaders();
    bool verifyHeader(const SPVHeader& header);
    QList<SPVHeader> getHeaders() const;
    uint32_t getBestHeight() const;

    // Block and transaction management
    bool syncBlocks(uint32_t fromHeight, uint32_t toHeight);
    bool verifyBlock(const SPVBlock& block);
    QList<SPVTransaction> getTransactions() const;
    QList<SPVTransaction> getTransactionsForAddress(const QString& address) const;

    // Bloom filter management
    void updateBloomFilter();
    bool addAddressToFilter(const QString& address);
    bool removeAddressFromFilter(const QString& address);
    void clearBloomFilter();

    // Merkle proof verification
    bool verifyMerkleProof(const uint256& txid, const uint256& blockHash, 
                          const std::vector<uint256>& merklePath, uint32_t index);

    // Settings
    void setServerUrls(const QStringList& urls);
    void setMaxConnections(int max);
    void setSyncTimeout(int timeout);
    void setUseBloomFilters(bool use);
    void setVerifyHeaders(bool verify);

public Q_SLOTS:
    void startSync();
    void stopSync();
    void pauseSync();
    void resumeSync();
    void refreshConnections();

private Q_SLOTS:
    void onNetworkReply(QNetworkReply* reply);
    void onSyncTimeout();
    void onConnectionCheck();
    void onHeaderSync();
    void onBlockSync();

private:
    // Network management
    void setupNetworkManager();
    void connectToServers();
    void disconnectFromServers();
    QNetworkReply* sendRequest(const QString& url, const QJsonObject& request);
    void handleNetworkError(QNetworkReply* reply);

    // Header sync
    void requestHeaders(uint32_t fromHeight, uint32_t toHeight);
    void processHeaders(const QJsonArray& headers);
    bool validateHeaderChain(const QList<SPVHeader>& headers);

    // Block sync
    void requestBlocks(uint32_t fromHeight, uint32_t toHeight);
    void processBlocks(const QJsonArray& blocks);
    bool validateBlock(const SPVBlock& block);

    // Bloom filter
    void createBloomFilter();
    void updateBloomFilterData();
    bool isAddressInFilter(const QString& address) const;

    // Storage
    void saveHeaders();
    void loadHeaders();
    void saveBlocks();
    void loadBlocks();
    void saveTransactions();
    void loadTransactions();

    // Utility
    QString getServerUrl() const;
    bool isValidServerResponse(const QJsonObject& response) const;
    void logError(const QString& error);
    void logInfo(const QString& info);

    // Data members
    SPVConfig m_config;
    WalletModel* m_walletModel;
    QNetworkAccessManager* m_networkManager;
    QTimer* m_syncTimer;
    QTimer* m_connectionTimer;
    QTimer* m_timeoutTimer;
    
    // State
    SyncStatus m_syncStatus;
    int m_syncProgress;
    bool m_isConnected;
    uint32_t m_bestHeight;
    uint32_t m_lastSyncTime;
    
    // Data storage
    QList<SPVHeader> m_headers;
    QList<SPVBlock> m_blocks;
    QList<SPVTransaction> m_transactions;
    QSet<QString> m_addresses;
    
    // Bloom filter
    QByteArray m_bloomFilter;
    uint32_t m_bloomFilterSize;
    uint32_t m_bloomFilterHashFuncs;
    uint32_t m_bloomFilterTweak;
    
    // Threading
    QMutex m_dataMutex;
    QThreadPool* m_threadPool;
    
    // Network state
    QStringList m_connectedServers;
    QMap<QString, QNetworkReply*> m_pendingRequests;
    int m_failedRequests;
    int m_successfulRequests;
};

/**
 * SPV Sync Worker
 * 
 * Handles background synchronization tasks
 */
class SPVSyncWorker : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit SPVSyncWorker(SPVClient* client, QObject* parent = nullptr);
    void run() override;

public Q_SLOTS:
    void syncHeaders();
    void syncBlocks();
    void verifyTransactions();

Q_SIGNALS:
    void syncProgress(int progress);
    void syncComplete();
    void syncError(const QString& error);

private:
    SPVClient* m_client;
    bool m_isRunning;
};

#endif // SHAHCOIN_SPV_SPVCLIENT_H
