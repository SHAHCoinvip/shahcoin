// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license.

#include <blockencodings.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <util/strencodings.h>
#include <util/system.h>
#include <hash.h>
#include <logging.h>
#include <tinyformat.h>
#include <consensus/consensus.h>
#include <consensus/validation.h>
#include <boost/multi_index/hashed_index.hpp> // ✅ Boost include restored
#include <random.h>
#include <streams.h>
#include <crypto/siphash.h>
#include <txmempool.h>
#include <validation.h>
#include <chainparams.h>

bool CTxCompressor::CompressScript(const CScript& script, std::vector<unsigned char>& out)
{
    out = std::vector<unsigned char>(script.begin(), script.end());
    return true;
}

bool CTxCompressor::DecompressScript(CScript& script, const std::vector<unsigned char>& in)
{
    script = CScript(in.begin(), in.end());
    return true;
}

CBlockHeaderAndShortTxIDs::CBlockHeaderAndShortTxIDs(const CBlock& block) :
    nonce(GetRand(std::numeric_limits<uint64_t>::max())),
    shorttxids(block.vtx.size() - 1), // exclude coinbase
    prefilledtxn(1)
{
    prefilledtxn[0] = {0, block.vtx[0]};
    for (size_t i = 1; i < block.vtx.size(); i++) {
        const CTransaction& tx = *block.vtx[i];
        shorttxids[i - 1] = GetShortID(tx.GetHash());
    }
}

void CBlockHeaderAndShortTxIDs::FillShortTxIDSelector() const
{
    CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
    stream << header << nonce;
    CSHA256 hasher;
    hasher.Write((unsigned char*)&(*stream.begin()), stream.end() - stream.begin());
    uint256 shorttxidhash;
    hasher.Finalize(shorttxidhash.begin());
    shorttxidk0 = shorttxidhash.GetUint64(0);
    shorttxidk1 = shorttxidhash.GetUint64(1);
}

uint64_t CBlockHeaderAndShortTxIDs::GetShortID(const uint256& txhash) const
{
    static_assert(SHORTTXIDS_LENGTH == 6, "shorttxids calculation assumes 6-byte shorttxids");
    return SipHashUint256(shorttxidk0, shorttxidk1, txhash) & 0xffffffffffffL;
}

ReadStatus PartiallyDownloadedBlock::InitData(const CBlockHeaderAndShortTxIDs& cmpctblock, const std::vector<std::pair<uint256, CTransactionRef>>& extra_txn)
{
    if (cmpctblock.header.IsNull() || (cmpctblock.BlockTxCount() == 0))
        return READ_STATUS_INVALID;

    if (cmpctblock.BlockTxCount() > MAX_BLOCK_WEIGHT / MIN_SERIALIZABLE_TRANSACTION_WEIGHT)
        return READ_STATUS_INVALID;

    assert(header.IsNull() && txn_available.empty());
    header = cmpctblock.header;
    txn_available.resize(cmpctblock.BlockTxCount(), nullptr);
    txn_available[0] = cmpctblock.prefilledtxn[0].tx; // coinbase is always available
    prefilled_count = cmpctblock.prefilledtxn.size();
    // count how many transactions are pre-filled (coinbase + prefilled)
    size_t filled_count = prefilled_count;
    for (size_t i = 0; i < cmpctblock.shorttxids.size(); i++) {
        uint16_t index = cmpctblock.prefilledtxn[prefilled_count - 1].index + (uint16_t)(i + 1);
        if (index >= cmpctblock.BlockTxCount())
            return READ_STATUS_INVALID;
        filled_count++;
    }
    // At least one transaction must be pre-filled
    if (filled_count == 0)
        return READ_STATUS_INVALID;

    // Calculate map of txids -> positions and check mempool to see what we have (or don't)
    // Because well-formed cmpctblock messages will have a (relatively) uniform distribution
    // of short IDs, any highly-uneven distribution of elements can be safely treated as a
    // READ_STATUS_FAILED.
    std::unordered_map<uint64_t, uint16_t> shorttxids(cmpctblock.shorttxids.size());
    std::vector<bool> have_txn(txn_available.size());
    {
        LOCK(pool->cs);
        const std::vector<TxMempoolInfo>& mempool_txn = pool->infoAll();
        for (size_t i = 0; i < mempool_txn.size(); i++) {
            uint64_t shortid = cmpctblock.GetShortID(mempool_txn[i].tx->GetHash());
            if (shortid != 0) { // ignore collisions
                shorttxids[shortid] = i;
            }
        }
        for (size_t i = 0; i < cmpctblock.shorttxids.size(); i++) {
            uint64_t shortid = cmpctblock.shorttxids[i];
            uint16_t index = cmpctblock.prefilledtxn[prefilled_count - 1].index + (uint16_t)(i + 1);
            if (index >= cmpctblock.BlockTxCount())
                return READ_STATUS_INVALID;
            if (shorttxids.count(shortid)) {
                if (!have_txn[index]) {
                    txn_available[index] = mempool_txn[shorttxids[shortid]].tx;
                    have_txn[index] = true;
                    mempool_count++;
                } else {
                    // If we find two mempool txn that match the short id, just
                    // request it.
                    txn_available[index] = nullptr;
                }
            }
            // Though ideally we'd continue scanning for the two-txn-match-shortid case,
            // the performance cost of doing so O(n^2) in the number of mempool txn
            // seems to outweigh the benefit of doing so.
        }
    }

    LogPrint(BCLog::NET, "Initialized PartiallyDownloadedBlock for block %s using a cmpctblock of size %lu\n", cmpctblock.header.GetHash().ToString(), GetSerializeSize(cmpctblock, PROTOCOL_VERSION));

    return READ_STATUS_OK;
}

bool PartiallyDownloadedBlock::IsTxAvailable(size_t index) const
{
    assert(!header.IsNull());
    assert(index < txn_available.size());
    return txn_available[index] != nullptr;
}

ReadStatus PartiallyDownloadedBlock::FillBlock(CBlock& block, const std::vector<CTransactionRef>& vtx_missing)
{
    assert(!header.IsNull());
    uint256 hash = header.GetHash();
    block = header;
    block.vtx.resize(txn_available.size());

    size_t tx_missing_offset = 0;
    for (size_t i = 0; i < txn_available.size(); i++) {
        if (txn_available[i] == nullptr) {
            if (tx_missing_offset >= vtx_missing.size())
                return READ_STATUS_INVALID;
            block.vtx[i] = vtx_missing[tx_missing_offset++];
        } else {
            block.vtx[i] = txn_available[i];
        }
    }

    // Make sure we can't call FillBlock again.
    header.SetNull();
    txn_available.clear();

    if (vtx_missing.size() != tx_missing_offset)
        return READ_STATUS_INVALID;

    BlockValidationState state;
    if (!CheckBlock(block, state, ::Params().GetConsensus())) {
        // TODO: We really want to just check merkle tree manually here,
        // but that is expensive, and CheckBlock is cheap enough for now.
        // Clean this up when security of this message is improved.
        return READ_STATUS_CHECKBLOCK_FAILED;
    }

    LogPrint(BCLog::NET, "Successfully reconstructed block %s with %lu txn prefilled, %lu txn from mempool (incl at least %lu from extra pool) and %lu txn requested\n", hash.ToString(), prefilled_count, mempool_count, extra_count, vtx_missing.size());

    return READ_STATUS_OK;
}
