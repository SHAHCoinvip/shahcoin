// SPDX-License-Identifier: Apache-2.0
#ifndef SHAHCOIN_POW_DISPATCH_H
#define SHAHCOIN_POW_DISPATCH_H

#include <stdint.h>
#include <vector>
#include "consensus/hybrid.h"

// Compute 32-byte PoW hash for the given header bytes using the selected algo.
void GetPoWHash(const std::vector<unsigned char>& header, AlgoType algo, unsigned char* out32);

#endif // SHAHCOIN_POW_DISPATCH_H


