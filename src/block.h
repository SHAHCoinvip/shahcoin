// src/block.h
#ifndef SHAHCOIN_BLOCK_H
#define SHAHCOIN_BLOCK_H

#include <primitives/block.h>
#include <script/script.h>
#include <amount.h>

// Declaration only — the implementation is in chainparams.cpp
CBlock CreateGenesisBlock(
    const char* pszTimestamp,
    const CScript& outputScript,
    uint32_t nTime,
    uint32_t nNonce,
    uint32_t nshahbits,
    int32_t nVersion,
    const CAmount& genesisReward
);

#endif // SHAHCOIN_BLOCK_H
