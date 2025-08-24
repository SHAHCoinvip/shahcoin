// Copyright (c) 2009-2010 Shahi Nakamoto
// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_PRIMITIVES_BLOCK_H
#define SHAHCOIN_PRIMITIVES_BLOCK_H

#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>
#include <util/time.h>
#include <consensus/consensus.h>
#include <consensus/hybrid.h>

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
    uint8_t nAlgorithm; // SHAHCOIN Core multi-algorithm mining support
    uint8_t nBlockType; // SHAHCOIN Core block type (PoW/PoS)
    
    // PoS staking data (only used for PoS blocks)
    uint256 hashStake;      // Stake transaction hash
    uint32_t nStakeTime;    // Stake timestamp
    uint256 hashStakeKernel; // Stake kernel hash for validation
    
    // Hybrid consensus: AlgoType field (encoded in version bits or extra field)
    AlgoType GetAlgoType() const {
        // Check if this is a hybrid consensus block (version >= 0x20000000)
        if (nVersion >= 0x20000000) {
            // Extract algorithm from version bits (bits 28-30)
            uint8_t algo_bits = (nVersion >> 28) & 0x07;
            return static_cast<AlgoType>(algo_bits);
        }
        
        // Legacy blocks: derive from height-based rotation
        // This is a fallback for blocks that don't have explicit algorithm encoding
        return AlgoType::SHA256D; // Default for legacy blocks
    }
    
    void SetAlgoType(AlgoType algo) {
        // Set version to indicate hybrid consensus block
        nVersion = 0x20000000 | (static_cast<uint8_t>(algo) << 28);
        nAlgorithm = static_cast<uint8_t>(algo);
    }

    CBlockHeader()
    {
        SetNull();
    }

    SERIALIZE_METHODS(CBlockHeader, obj) { 
        READWRITE(obj.nVersion, obj.hashPrevBlock, obj.hashMerkleRoot, obj.nTime, obj.nBits, obj.nNonce, obj.nAlgorithm, obj.nBlockType);
        if (obj.nBlockType == BLOCK_TYPE_POS) {
            READWRITE(obj.hashStake, obj.nStakeTime, obj.hashStakeKernel);
        }
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        nAlgorithm = ALGO_SHA256D; // SHAHCOIN Core default to SHA256d
        nBlockType = BLOCK_TYPE_POW; // SHAHCOIN Core default to PoW
        hashStake.SetNull();
        nStakeTime = 0;
        hashStakeKernel.SetNull();
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    NodeSeconds Time() const
    {
        return NodeSeconds{std::chrono::seconds{nTime}};
    }

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    // Get the SHAHCOIN Core mining algorithm for this block
    MiningAlgorithm GetAlgorithm() const
    {
        return static_cast<MiningAlgorithm>(nAlgorithm % ALGO_COUNT);
    }

    // Set the SHAHCOIN Core mining algorithm for this block
    void SetAlgorithm(MiningAlgorithm algo)
    {
        nAlgorithm = static_cast<uint8_t>(algo);
    }

    // Get the SHAHCOIN Core block type
    BlockType GetBlockType() const
    {
        return static_cast<BlockType>(nBlockType);
    }

    // Set the SHAHCOIN Core block type
    void SetBlockType(BlockType type)
    {
        nBlockType = static_cast<uint8_t>(type);
    }

    // Check if this is a PoS block
    bool IsProofOfStake() const
    {
        return nBlockType == BLOCK_TYPE_POS;
    }

    // Check if this is a PoW block
    bool IsProofOfWork() const
    {
        return nBlockType == BLOCK_TYPE_POW;
    }
    
    // Hybrid consensus: Check if this is a PoS block
    bool IsProofOfStake() const
    {
        return nBlockType == BLOCK_TYPE_POS;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransactionRef> vtx;

    // memory only
    mutable bool fChecked;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *(static_cast<CBlockHeader*>(this)) = header;
    }

    SERIALIZE_METHODS(CBlock, obj) { READWRITEAS(CBlockHeader, obj); READWRITE(obj.vtx); }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        fChecked = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        block.nAlgorithm     = nAlgorithm;
        block.nBlockType     = nBlockType;
        block.hashStake      = hashStake;
        block.nStakeTime     = nStakeTime;
        block.hashStakeKernel = hashStakeKernel;
        return block;
    }

    uint256 GetHash() const;
    uint256 GetPoWHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    // SHAHCOIN Core: Calculate block hash based on algorithm
    uint256 GetHash(MiningAlgorithm algo) const;

    // SHAHCOIN Core: Get the expected algorithm for this block height
    static MiningAlgorithm GetExpectedAlgorithm(int nHeight)
    {
        return static_cast<MiningAlgorithm>(nHeight % ALGO_COUNT);
    }

    // SHAHCOIN Core: Check if this block should be PoS based on height
    static bool ShouldBeProofOfStake(int nHeight)
    {
        return (nHeight % POS_BLOCK_INTERVAL) == 0;
    }
};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    /** Historically CBlockLocator's version field has been written to network
     * streams as the negotiated protocol version and to disk streams as the
     * client version, but the value has never been used.
     *
     * Hard-code to the highest protocol version ever written to a network stream.
     * SerParams can be used if the field requires any meaning in the future,
     **/
    static constexpr int DUMMY_VERSION = 70016;

    std::vector<uint256> vHave;

    CBlockLocator() {}

    explicit CBlockLocator(std::vector<uint256>&& have) : vHave(std::move(have)) {}

    SERIALIZE_METHODS(CBlockLocator, obj)
    {
        int nVersion = DUMMY_VERSION;
        READWRITE(nVersion);
        READWRITE(obj.vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

#endif // SHAHCOIN_PRIMITIVES_BLOCK_H
