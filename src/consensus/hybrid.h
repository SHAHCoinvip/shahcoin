// SPDX-License-Identifier: Apache-2.0
#ifndef SHAHCOIN_CONSENSUS_HYBRID_H
#define SHAHCOIN_CONSENSUS_HYBRID_H

#include <cstdint>

enum class AlgoType : uint8_t {
    SHA256D = 0,
    SCRYPT  = 1,
    GROESTL = 2,
    POS     = 3
};

AlgoType SelectNextAlgo(int height);
const char* AlgoName(AlgoType a);

// Per‑algo DAA stubs
unsigned int GetNextWorkRequiredSHA256(int height);
unsigned int GetNextWorkRequiredScrypt(int height);
unsigned int GetNextWorkRequiredGroestl(int height);

// PoS target stub
unsigned int GetNextStakeTarget(int height);

#endif // SHAHCOIN_CONSENSUS_HYBRID_H


