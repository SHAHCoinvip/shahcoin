// Copyright (c) 2009-2010 Shahi Nakamoto
// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>
#include <crypto/multihash.h>
#include <consensus/consensus.h>

uint256 CBlockHeader::GetHash() const
{
    // Use SHAHCOIN Core multi-algorithm hashing based on the block's algorithm
    MiningAlgorithm algo = GetAlgorithm();
    
    // Create a temporary header without the algorithm field for hashing
    // (to maintain compatibility with existing hash calculations)
    CHashWriter hasher(PROTOCOL_VERSION);
    hasher << nVersion << hashPrevBlock << hashMerkleRoot << nTime << nBits << nNonce;
    
    // Apply the appropriate algorithm
    return CMultiHash::Hash(hasher.GetHash(), algo);
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, algo=%s, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce, CMultiHash::GetAlgorithmName(GetAlgorithm()),
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
