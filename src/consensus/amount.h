// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_AMOUNT_H
#define SHAHCOIN_AMOUNT_H

#include <cstdint>  // Modern C++ version of <stdint.h>

/** Type aliases for coin amounts */
using CAmount = int64_t;

static constexpr CAmount COIN = 100000000;
static constexpr CAmount CENT = 1000000;

#endif // SHAHCOIN_AMOUNT_H
