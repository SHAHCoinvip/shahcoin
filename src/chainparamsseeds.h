// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_CHAINPARAMSSEEDS_H
#define SHAHCOIN_CHAINPARAMSSEEDS_H

#include <stdint.h>

// Macro to get array length
#define ARRAYLEN(array) (sizeof(array)/sizeof(array[0]))

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

// Main network seed nodes
// These will need to be updated with actual Shahcoin mainnet nodes
static const SeedSpec6 pnSeed6_main[] = {
    // Example mainnet seeds - replace with actual Shahcoin nodes
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x7f,0x00,0x00,0x01}, 8369},
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x7f,0x00,0x00,0x01}, 8369},
};

// Test network seed nodes
// These will need to be updated with actual Shahcoin testnet nodes
static const SeedSpec6 pnSeed6_test[] = {
    // Example testnet seeds - replace with actual Shahcoin testnet nodes
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x7f,0x00,0x00,0x01}, 18369},
};

// Regtest network seed nodes (empty for regtest)
static const SeedSpec6 pnSeed6_regtest[] = {
    // Regtest doesn't use fixed seeds
};

#endif // SHAHCOIN_CHAINPARAMSSEEDS_H
