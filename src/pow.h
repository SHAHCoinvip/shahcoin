// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_POW_H
#define SHAHCOIN_POW_H

#include <consensus/params.h>
#include <consensus/consensus.h>
#include <consensus/hybrid.h>
#include <pow_dispatch.h>

#include <stdint.h>

class CBlockHeader;
class CBlockIndex;
class uint256;

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params&);
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params&);

// SHAHCOIN Core multi-algorithm mining support
unsigned int GetNextWorkRequiredMultiAlgo(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params&);
unsigned int CalculateNextWorkRequiredMultiAlgo(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params&, MiningAlgorithm algo);

// Hybrid consensus: Per-algo work required functions
unsigned int GetNextWorkRequiredSHA256(const CBlockIndex* pindexLast, const Consensus::Params&);
unsigned int GetNextWorkRequiredScrypt(const CBlockIndex* pindexLast, const Consensus::Params&);
unsigned int GetNextWorkRequiredGroestl(const CBlockIndex* pindexLast, const Consensus::Params&);
unsigned int GetNextStakeTarget(const CBlockIndex* pindexLast, const Consensus::Params&);

// Hybrid consensus: Overload for AlgoType
uint256 GetPoWHash(const CBlockHeader& hdr);

/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits */
bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params&);

/** Check whether a block hash satisfies the proof-of-work requirement for SHAHCOIN Core multi-algorithm mining */
bool CheckProofOfWorkMultiAlgo(uint256 hash, unsigned int nBits, const Consensus::Params&, MiningAlgorithm algo);

/**
 * Return false if the proof-of-work requirement specified by new_nbits at a
 * given height is not possible, given the proof-of-work on the prior block as
 * specified by old_nbits.
 *
 * This function only checks that the new value is within a factor of 4 of the
 * old value for blocks at the difficulty adjustment interval, and otherwise
 * requires the values to be the same.
 *
 * Always returns true on networks where min difficulty blocks are allowed,
 * such as regtest/testnet.
 */
bool PermittedDifficultyTransition(const Consensus::Params& params, int64_t height, uint32_t old_nbits, uint32_t new_nbits);

#endif // SHAHCOIN_POW_H
