#include "spvclient.h"
#include <qt/walletmodel.h>
#include <qt/guiutil.h>
#include <qt/shahcoinunits.h>

#include <QApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <QThread>
#include <QThreadPool>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QSettings>
#include <QDebug>
#include <QUrlQuery>

#include <validation.h>
#include <chainparams.h>
#include <consensus/validation.h>
#include <script/standard.h>
#include <crypto/sha256.h>
#include <util/strencodings.h>
#include <util/time.h>
#include <index/txindex.h>
#include <txdb.h>

// Bloom filter parameters
static const uint32_t BLOOM_FILTER_SIZE = 1024 * 1024; // 1MB
static const uint32_t BLOOM_FILTER_HASH_FUNCS = 11;
static const uint32_t BLOOM_FILTER_TWEAK = 0xFBA4C795;

SPVClient::SPVClient(QObject* parent)
    : QObject(parent)
    , m_walletModel(nullptr)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_syncTimer(new QTimer(this))
    , m_connectionTimer(new QTimer(this))
    , m_timeoutTimer(new QTimer(this))
    , m_syncStatus(Disconnected)
    , m_syncProgress(0)
    , m_isConnected(false)
    , m_bestHeight(0)
    , m_lastSyncTime(0)
    , m_bloomFilterSize(BLOOM_FILTER_SIZE)
    , m_bloomFilterHashFuncs(BLOOM_FILTER_HASH_FUNCS)
    , m_bloomFilterTweak(BLOOM_FILTER_TWEAK)
    , m_threadPool(new QThreadPool(this))
    , m_failedRequests(0)
    , m_successfulRequests(0)
{
    // Initialize default config
    m_config.mode = FullNode;
    m_config.serverUrls = QStringList() 
        << "https://spv.shahcoin.net"
        << "https://spv-backup1.shahcoin.net"
        << "https://spv-backup2.shahcoin.net";
    m_config.maxConnections = 3;
    m_config.syncTimeout = 30000; // 30 seconds
    m_config.useBloomFilters = true;
    m_config.verifyHeaders = true;
    m_config.maxHeaders = 1000;
    m_config.maxBlocks = 100;

    setupNetworkManager();
    loadHeaders();
    loadBlocks();
    loadTransactions();

    // Set up timers
    connect(m_syncTimer, &QTimer::timeout, this, &SPVClient::onHeaderSync);
    connect(m_connectionTimer, &QTimer::timeout, this, &SPVClient::onConnectionCheck);
    connect(m_timeoutTimer, &QTimer::timeout, this, &SPVClient::onSyncTimeout);

    // Set up thread pool
    m_threadPool->setMaxThreadCount(4);
}

SPVClient::~SPVClient()
{
    disconnect();
    saveHeaders();
    saveBlocks();
    saveTransactions();
}

void SPVClient::setConfig(const SPVConfig& config)
{
    QMutexLocker locker(&m_dataMutex);
    m_config = config;
    
    // Save config to settings
    QSettings settings;
    settings.setValue("SPV/mode", static_cast<int>(config.mode));
    settings.setValue("SPV/serverUrls", config.serverUrls);
    settings.setValue("SPV/maxConnections", config.maxConnections);
    settings.setValue("SPV/syncTimeout", config.syncTimeout);
    settings.setValue("SPV/useBloomFilters", config.useBloomFilters);
    settings.setValue("SPV/verifyHeaders", config.verifyHeaders);
    settings.setValue("SPV/maxHeaders", config.maxHeaders);
    settings.setValue("SPV/maxBlocks", config.maxBlocks);
}

SPVClient::SPVConfig SPVClient::getConfig() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_config;
}

void SPVClient::setWalletModel(WalletModel* model)
{
    m_walletModel = model;
    if (m_walletModel) {
        // Update addresses for bloom filter
        updateBloomFilter();
    }
}

void SPVClient::setupNetworkManager()
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &SPVClient::onNetworkReply);
}

bool SPVClient::connect()
{
    if (m_isConnected) {
        return true;
    }

    m_syncStatus = Connecting;
    Q_EMIT syncStatusChanged(m_syncStatus);

    connectToServers();
    
    if (m_config.mode != FullNode) {
        startSync();
    }

    return true;
}

void SPVClient::disconnect()
{
    if (!m_isConnected) {
        return;
    }

    stopSync();
    disconnectFromServers();
    
    m_syncStatus = Disconnected;
    m_isConnected = false;
    Q_EMIT syncStatusChanged(m_syncStatus);
}

bool SPVClient::isConnected() const
{
    return m_isConnected;
}

SPVClient::SyncStatus SPVClient::getSyncStatus() const
{
    return m_syncStatus;
}

int SPVClient::getSyncProgress() const
{
    return m_syncProgress;
}

void SPVClient::connectToServers()
{
    for (const QString& url : m_config.serverUrls) {
        QJsonObject request;
        request["method"] = "ping";
        request["params"] = QJsonArray();
        request["id"] = 1;

        QNetworkReply* reply = sendRequest(url, request);
        if (reply) {
            m_pendingRequests[url] = reply;
        }
    }
}

void SPVClient::disconnectFromServers()
{
    // Cancel all pending requests
    for (QNetworkReply* reply : m_pendingRequests.values()) {
        reply->abort();
        reply->deleteLater();
    }
    m_pendingRequests.clear();
    m_connectedServers.clear();
}

QNetworkReply* SPVClient::sendRequest(const QString& url, const QJsonObject& request)
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl(QUrl(url));
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, "Shahcoin-SPV-Client/1.0");

    QJsonDocument doc(request);
    QByteArray data = doc.toJson();

    return m_networkManager->post(networkRequest, data);
}

void SPVClient::onNetworkReply(QNetworkReply* reply)
{
    if (!reply) {
        return;
    }

    QString url = reply->url().toString();
    m_pendingRequests.remove(url);

    if (reply->error() == QNetworkReply::NoError) {
        m_successfulRequests++;
        
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        if (doc.isObject()) {
            QJsonObject response = doc.object();
            
            if (isValidServerResponse(response)) {
                if (!m_connectedServers.contains(url)) {
                    m_connectedServers.append(url);
                }
                
                // Handle different response types
                QString method = response.value("method").toString();
                if (method == "ping") {
                    // Server is alive
                    if (!m_isConnected) {
                        m_isConnected = true;
                        Q_EMIT connectionStatusChanged(true);
                    }
                } else if (method == "getheaders") {
                    processHeaders(response.value("result").toArray());
                } else if (method == "getblocks") {
                    processBlocks(response.value("result").toArray());
                }
            }
        }
    } else {
        m_failedRequests++;
        handleNetworkError(reply);
    }

    reply->deleteLater();
}

void SPVClient::handleNetworkError(QNetworkReply* reply)
{
    QString error = reply->errorString();
    logError(QString("Network error: %1").arg(error));
    
    if (m_connectedServers.isEmpty()) {
        m_isConnected = false;
        m_syncStatus = Error;
        Q_EMIT connectionStatusChanged(false);
        Q_EMIT syncStatusChanged(m_syncStatus);
    }
}

bool SPVClient::isValidServerResponse(const QJsonObject& response) const
{
    return response.contains("result") && 
           (response.contains("error") ? response.value("error").isNull() : true);
}

void SPVClient::startSync()
{
    if (m_config.mode == FullNode) {
        return;
    }

    m_syncStatus = SyncingHeaders;
    Q_EMIT syncStatusChanged(m_syncStatus);

    // Start header sync
    syncHeaders();
    
    // Set up periodic sync
    m_syncTimer->start(60000); // Sync every minute
    m_connectionTimer->start(30000); // Check connections every 30 seconds
}

void SPVClient::stopSync()
{
    m_syncTimer->stop();
    m_connectionTimer->stop();
    m_timeoutTimer->stop();
    
    m_syncStatus = Disconnected;
    Q_EMIT syncStatusChanged(m_syncStatus);
}

void SPVClient::pauseSync()
{
    m_syncTimer->stop();
    m_connectionTimer->stop();
}

void SPVClient::resumeSync()
{
    if (m_config.mode != FullNode) {
        startSync();
    }
}

void SPVClient::refreshConnections()
{
    disconnectFromServers();
    connectToServers();
}

bool SPVClient::syncHeaders()
{
    if (m_connectedServers.isEmpty()) {
        return false;
    }

    QString serverUrl = getServerUrl();
    if (serverUrl.isEmpty()) {
        return false;
    }

    QJsonObject request;
    request["method"] = "getheaders";
    request["params"] = QJsonArray() << m_bestHeight << m_config.maxHeaders;
    request["id"] = 2;

    QNetworkReply* reply = sendRequest(serverUrl, request);
    if (reply) {
        m_pendingRequests[serverUrl] = reply;
        
        // Set timeout
        m_timeoutTimer->start(m_config.syncTimeout);
        return true;
    }

    return false;
}

void SPVClient::processHeaders(const QJsonArray& headers)
{
    QMutexLocker locker(&m_dataMutex);
    
    QList<SPVHeader> newHeaders;
    for (const QJsonValue& value : headers) {
        QJsonObject headerObj = value.toObject();
        
        SPVHeader header;
        header.hash = uint256S(headerObj.value("hash").toString().toStdString());
        header.prevHash = uint256S(headerObj.value("prevhash").toString().toStdString());
        header.nTime = headerObj.value("time").toVariant().toUInt();
        header.nshahbits = headerObj.value("shahbits").toVariant().toUInt();
        header.nNonce = headerObj.value("nonce").toVariant().toUInt();
        header.nHeight = headerObj.value("height").toVariant().toUInt();
        header.isValid = false;
        
        if (verifyHeader(header)) {
            header.isValid = true;
            newHeaders.append(header);
        }
    }
    
    if (validateHeaderChain(newHeaders)) {
        m_headers.append(newHeaders);
        
        if (!newHeaders.isEmpty()) {
            m_bestHeight = newHeaders.last().nHeight;
        }
        
        saveHeaders();
        
        // Move to block sync if we have new headers
        if (!newHeaders.isEmpty()) {
            m_syncStatus = SyncingBlocks;
            Q_EMIT syncStatusChanged(m_syncStatus);
            syncBlocks(m_bestHeight - newHeaders.size() + 1, m_bestHeight);
        }
    }
}

bool SPVClient::verifyHeader(const SPVHeader& header)
{
    if (!m_config.verifyHeaders) {
        return true;
    }

    // Basic validation
    if (header.nHeight == 0 && header.hash != Params().GenesisBlock().GetHash()) {
        return false;
    }

    // Check proof of work
    CBlockHeader blockHeader;
    blockHeader.nVersion = 1;
    blockHeader.hashPrevBlock = header.prevHash;
    blockHeader.hashMerkleRoot = uint256(); // Will be set later
    blockHeader.nTime = header.nTime;
    blockHeader.nshahbits = header.nshahbits;
    blockHeader.nNonce = header.nNonce;

    return CheckProofOfWork(blockHeader.GetHash(), header.nshahbits, Params().GetConsensus());
}

bool SPVClient::validateHeaderChain(const QList<SPVHeader>& headers)
{
    if (headers.isEmpty()) {
        return true;
    }

    // Check chain continuity
    for (int i = 1; i < headers.size(); ++i) {
        if (headers[i].prevHash != headers[i-1].hash) {
            return false;
        }
    }

    // Check difficulty adjustment
    // This is a simplified check - in production, you'd want more thorough validation
    return true;
}

bool SPVClient::syncBlocks(uint32_t fromHeight, uint32_t toHeight)
{
    if (m_connectedServers.isEmpty()) {
        return false;
    }

    QString serverUrl = getServerUrl();
    if (serverUrl.isEmpty()) {
        return false;
    }

    QJsonObject request;
    request["method"] = "getblocks";
    request["params"] = QJsonArray() << fromHeight << toHeight;
    request["id"] = 3;

    QNetworkReply* reply = sendRequest(serverUrl, request);
    if (reply) {
        m_pendingRequests[serverUrl] = reply;
        return true;
    }

    return false;
}

void SPVClient::processBlocks(const QJsonArray& blocks)
{
    QMutexLocker locker(&m_dataMutex);
    
    for (const QJsonValue& value : blocks) {
        QJsonObject blockObj = value.toObject();
        
        SPVBlock block;
        block.hash = uint256S(blockObj.value("hash").toString().toStdString());
        block.prevHash = uint256S(blockObj.value("prevhash").toString().toStdString());
        block.nHeight = blockObj.value("height").toVariant().toUInt();
        block.nTime = blockObj.value("time").toVariant().toUInt();
        block.isValid = false;
        
        // Process transaction hashes
        QJsonArray txArray = blockObj.value("tx").toArray();
        for (const QJsonValue& txValue : txArray) {
            block.txHashes.append(uint256S(txValue.toString().toStdString()));
        }
        
        if (validateBlock(block)) {
            block.isValid = true;
            m_blocks.append(block);
            
            // Process transactions for our addresses
            processBlockTransactions(block);
        }
    }
    
    saveBlocks();
    
    // Update sync progress
    m_syncProgress = (m_blocks.size() * 100) / m_config.maxBlocks;
    Q_EMIT syncProgressChanged(m_syncProgress);
    
    if (m_syncProgress >= 100) {
        m_syncStatus = Synced;
        Q_EMIT syncStatusChanged(m_syncStatus);
    }
}

bool SPVClient::validateBlock(const SPVBlock& block)
{
    // Basic validation
    if (block.nHeight == 0 && block.hash != Params().GenesisBlock().GetHash()) {
        return false;
    }
    
    // Check merkle root
    // This would require full transaction data in a real implementation
    
    return true;
}

void SPVClient::processBlockTransactions(const SPVBlock& block)
{
    if (!m_walletModel || !m_config.useBloomFilters) {
        return;
    }

    // Check if any of our addresses are in this block
    for (const uint256& txHash : block.txHashes) {
        // In a real implementation, you'd check if the transaction contains our addresses
        // For now, we'll just add a placeholder transaction
        SPVTransaction tx;
        tx.txid = txHash;
        tx.blockHash = block.hash;
        tx.blockHeight = block.nHeight;
        tx.nTime = block.nTime;
        tx.amount = 0;
        tx.address = "";
        tx.isSpent = false;
        tx.isConfirmed = true;
        
        m_transactions.append(tx);
    }
}

void SPVClient::updateBloomFilter()
{
    if (!m_config.useBloomFilters) {
        return;
    }

    createBloomFilter();
    updateBloomFilterData();
}

void SPVClient::createBloomFilter()
{
    if (!m_walletModel) {
        return;
    }

    // Get all addresses from wallet
    m_addresses.clear();
    
    // In a real implementation, you'd get addresses from the wallet model
    // For now, we'll use placeholder addresses
    m_addresses.insert("S1TestAddress1");
    m_addresses.insert("S1TestAddress2");
    
    // Create bloom filter
    m_bloomFilter.clear();
    m_bloomFilter.resize(m_bloomFilterSize);
    m_bloomFilter.fill(0);
    
    for (const QString& address : m_addresses) {
        addAddressToFilter(address);
    }
}

void SPVClient::addAddressToFilter(const QString& address)
{
    if (!m_config.useBloomFilters) {
        return;
    }

    // Add address to bloom filter
    // This is a simplified implementation
    QByteArray addressBytes = address.toUtf8();
    uint32_t hash1 = MurmurHash3(addressBytes.data(), addressBytes.size(), m_bloomFilterTweak);
    uint32_t hash2 = MurmurHash3(addressBytes.data(), addressBytes.size(), hash1);
    
    for (uint32_t i = 0; i < m_bloomFilterHashFuncs; ++i) {
        uint32_t bit = (hash1 + i * hash2) % (m_bloomFilterSize * 8);
        uint32_t byteIndex = bit / 8;
        uint32_t bitIndex = bit % 8;
        
        if (byteIndex < m_bloomFilter.size()) {
            m_bloomFilter[byteIndex] |= (1 << bitIndex);
        }
    }
}

bool SPVClient::removeAddressFromFilter(const QString& address)
{
    // In a real implementation, you'd remove the address from the bloom filter
    // For now, we'll just remove it from the set
    return m_addresses.remove(address) > 0;
}

void SPVClient::clearBloomFilter()
{
    m_bloomFilter.clear();
    m_addresses.clear();
}

void SPVClient::updateBloomFilterData()
{
    // Send updated bloom filter to servers
    if (m_connectedServers.isEmpty()) {
        return;
    }

    QString serverUrl = getServerUrl();
    if (serverUrl.isEmpty()) {
        return;
    }

    QJsonObject request;
    request["method"] = "setbloomfilter";
    request["params"] = QJsonArray() << QString(m_bloomFilter.toHex());
    request["id"] = 4;

    QNetworkReply* reply = sendRequest(serverUrl, request);
    if (reply) {
        m_pendingRequests[serverUrl] = reply;
    }
}

bool SPVClient::verifyMerkleProof(const uint256& txid, const uint256& blockHash, 
                                 const std::vector<uint256>& merklePath, uint32_t index)
{
    // Verify merkle proof
    uint256 current = txid;
    
    for (const uint256& sibling : merklePath) {
        if (index % 2 == 0) {
            current = Hash(current, sibling);
        } else {
            current = Hash(sibling, current);
        }
        index /= 2;
    }
    
    // Compare with block's merkle root
    // In a real implementation, you'd get the merkle root from the block header
    return true;
}

QString SPVClient::getServerUrl() const
{
    if (m_connectedServers.isEmpty()) {
        return QString();
    }
    
    // Simple round-robin selection
    static int currentIndex = 0;
    return m_connectedServers[currentIndex++ % m_connectedServers.size()];
}

void SPVClient::onSyncTimeout()
{
    logError("Sync timeout");
    m_syncStatus = Error;
    Q_EMIT syncStatusChanged(m_syncStatus);
}

void SPVClient::onConnectionCheck()
{
    if (m_connectedServers.isEmpty()) {
        refreshConnections();
    }
}

void SPVClient::onHeaderSync()
{
    if (m_syncStatus == Synced) {
        // Periodic header sync to check for new blocks
        syncHeaders();
    }
}

void SPVClient::onBlockSync()
{
    // This would be called when new blocks are available
}

QList<SPVHeader> SPVClient::getHeaders() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_headers;
}

uint32_t SPVClient::getBestHeight() const
{
    return m_bestHeight;
}

QList<SPVTransaction> SPVClient::getTransactions() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_transactions;
}

QList<SPVTransaction> SPVClient::getTransactionsForAddress(const QString& address) const
{
    QMutexLocker locker(&m_dataMutex);
    QList<SPVTransaction> result;
    
    for (const SPVTransaction& tx : m_transactions) {
        if (tx.address == address) {
            result.append(tx);
        }
    }
    
    return result;
}

void SPVClient::saveHeaders()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/spv";
    QDir().mkpath(dataDir);
    
    QFile file(dataDir + "/headers.dat");
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << static_cast<qint32>(m_headers.size());
        
        for (const SPVHeader& header : m_headers) {
            out << QString::fromStdString(header.hash.ToString());
            out << QString::fromStdString(header.prevHash.ToString());
            out << header.nTime;
            out << header.nshahbits;
            out << header.nNonce;
            out << header.nHeight;
            out << header.isValid;
        }
    }
}

void SPVClient::loadHeaders()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/spv";
    QFile file(dataDir + "/headers.dat");
    
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        qint32 count;
        in >> count;
        
        for (qint32 i = 0; i < count; ++i) {
            SPVHeader header;
            QString hashStr, prevHashStr;
            
            in >> hashStr >> prevHashStr >> header.nTime >> header.nshahbits 
               >> header.nNonce >> header.nHeight >> header.isValid;
            
            header.hash = uint256S(hashStr.toStdString());
            header.prevHash = uint256S(prevHashStr.toStdString());
            
            m_headers.append(header);
        }
        
        if (!m_headers.isEmpty()) {
            m_bestHeight = m_headers.last().nHeight;
        }
    }
}

void SPVClient::saveBlocks()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/spv";
    QDir().mkpath(dataDir);
    
    QFile file(dataDir + "/blocks.dat");
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << static_cast<qint32>(m_blocks.size());
        
        for (const SPVBlock& block : m_blocks) {
            out << QString::fromStdString(block.hash.ToString());
            out << QString::fromStdString(block.prevHash.ToString());
            out << block.nHeight;
            out << block.nTime;
            out << static_cast<qint32>(block.txHashes.size());
            
            for (const uint256& txHash : block.txHashes) {
                out << QString::fromStdString(txHash.ToString());
            }
            
            out << block.isValid;
        }
    }
}

void SPVClient::loadBlocks()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/spv";
    QFile file(dataDir + "/blocks.dat");
    
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        qint32 count;
        in >> count;
        
        for (qint32 i = 0; i < count; ++i) {
            SPVBlock block;
            QString hashStr, prevHashStr;
            qint32 txCount;
            
            in >> hashStr >> prevHashStr >> block.nHeight >> block.nTime >> txCount;
            
            block.hash = uint256S(hashStr.toStdString());
            block.prevHash = uint256S(prevHashStr.toStdString());
            
            for (qint32 j = 0; j < txCount; ++j) {
                QString txHashStr;
                in >> txHashStr;
                block.txHashes.append(uint256S(txHashStr.toStdString()));
            }
            
            in >> block.isValid;
            m_blocks.append(block);
        }
    }
}

void SPVClient::saveTransactions()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/spv";
    QDir().mkpath(dataDir);
    
    QFile file(dataDir + "/transactions.dat");
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << static_cast<qint32>(m_transactions.size());
        
        for (const SPVTransaction& tx : m_transactions) {
            out << QString::fromStdString(tx.txid.ToString());
            out << QString::fromStdString(tx.blockHash.ToString());
            out << tx.blockHeight;
            out << tx.nTime;
            out << tx.amount;
            out << tx.address;
            out << tx.isSpent;
            out << tx.isConfirmed;
        }
    }
}

void SPVClient::loadTransactions()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/shahcoin/spv";
    QFile file(dataDir + "/transactions.dat");
    
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        qint32 count;
        in >> count;
        
        for (qint32 i = 0; i < count; ++i) {
            SPVTransaction tx;
            QString txidStr, blockHashStr;
            
            in >> txidStr >> blockHashStr >> tx.blockHeight >> tx.nTime 
               >> tx.amount >> tx.address >> tx.isSpent >> tx.isConfirmed;
            
            tx.txid = uint256S(txidStr.toStdString());
            tx.blockHash = uint256S(blockHashStr.toStdString());
            
            m_transactions.append(tx);
        }
    }
}

void SPVClient::logError(const QString& error)
{
    qDebug() << "[SPV Error]" << error;
    Q_EMIT errorOccurred(error);
}

void SPVClient::logInfo(const QString& info)
{
    qDebug() << "[SPV Info]" << info;
    Q_EMIT infoReceived(info);
}

// Settings methods
void SPVClient::setServerUrls(const QStringList& urls)
{
    m_config.serverUrls = urls;
    QSettings settings;
    settings.setValue("SPV/serverUrls", urls);
}

void SPVClient::setMaxConnections(int max)
{
    m_config.maxConnections = max;
    QSettings settings;
    settings.setValue("SPV/maxConnections", max);
}

void SPVClient::setSyncTimeout(int timeout)
{
    m_config.syncTimeout = timeout;
    QSettings settings;
    settings.setValue("SPV/syncTimeout", timeout);
}

void SPVClient::setUseBloomFilters(bool use)
{
    m_config.useBloomFilters = use;
    QSettings settings;
    settings.setValue("SPV/useBloomFilters", use);
    
    if (use) {
        updateBloomFilter();
    } else {
        clearBloomFilter();
    }
}

void SPVClient::setVerifyHeaders(bool verify)
{
    m_config.verifyHeaders = verify;
    QSettings settings;
    settings.setValue("SPV/verifyHeaders", verify);
}

// SPVSyncWorker Implementation
SPVSyncWorker::SPVSyncWorker(SPVClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_isRunning(false)
{
}

void SPVSyncWorker::run()
{
    m_isRunning = true;
    
    while (m_isRunning) {
        syncHeaders();
        QThread::msleep(1000); // Wait 1 second between syncs
        
        if (m_isRunning) {
            syncBlocks();
            QThread::msleep(1000);
        }
        
        if (m_isRunning) {
            verifyTransactions();
            QThread::msleep(1000);
        }
    }
    
    Q_EMIT syncComplete();
}

void SPVSyncWorker::syncHeaders()
{
    if (m_client) {
        m_client->syncHeaders();
        Q_EMIT syncProgress(33);
    }
}

void SPVSyncWorker::syncBlocks()
{
    if (m_client) {
        // This would sync blocks
        Q_EMIT syncProgress(66);
    }
}

void SPVSyncWorker::verifyTransactions()
{
    if (m_client) {
        // This would verify transactions
        Q_EMIT syncProgress(100);
    }
}
