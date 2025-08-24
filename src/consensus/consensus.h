// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_CONSENSUS_CONSENSUS_H
#define SHAHCOIN_CONSENSUS_CONSENSUS_H

#include <cstdlib>
#include <stdint.h>

/** The maximum allowed size for a serialized block, in bytes (only for buffer size limits) */
static const unsigned int MAX_BLOCK_SERIALIZED_SIZE = 1000000; // Reduced to 1MB for security
/** The maximum allowed weight for a block, see BIP 141 (network rule) */
static const unsigned int MAX_BLOCK_WEIGHT = 1000000; // Reduced to 1MB for security
/** The maximum allowed number of signature check operations in a block (network rule) */
static const int64_t MAX_BLOCK_SIGOPS_COST = 80000;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;

static const int WITNESS_SCALE_FACTOR = 4;

static const size_t MIN_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 60; // 60 is the lower bound for the size of a valid serialized CTransaction
static const size_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 10; // 10 is the lower bound for the size of a serialized CTransaction

/** Maximum transaction size in bytes */
static const unsigned int MAX_TRANSACTION_SIZE = 100000; // 100KB limit for security

/** SHAHCOIN Core multi-algorithm mining support */
enum MiningAlgorithm {
    ALGO_SHA256D = 0,  // SHA256d (Shahcoin-style, ASIC-friendly)
    ALGO_SCRYPT = 1,   // Scrypt (Litecoin-style, GPU-friendly)
    ALGO_GROESTL = 2,  // Groestl (CPU/GPU-friendly)
    ALGO_COUNT = 3     // Total number of algorithms
};

/** SHAHCOIN Core algorithm weights for difficulty adjustment (33.3% each) */
static const int ALGO_WEIGHTS[ALGO_COUNT] = {33, 33, 34}; // Total 100%

/** SHAHCOIN Core algorithm rotation interval in blocks */
static const int ALGO_ROTATION_INTERVAL = 2016;

/** SHAHCOIN Core Proof of Stake (PoS) consensus parameters */
static const int64_t POS_MIN_STAKE_AMOUNT = 333 * 100000000; // 333 SHAH minimum stake (in shahi)
static const int64_t POS_MIN_STAKE_AGE = 3600; // 1 hour minimum stake age (in seconds)
static const int64_t POS_BLOCK_REWARD = 5 * 100000000; // 5 SHAH PoS block reward (in shahi)
static const int POS_BLOCK_INTERVAL = 10; // Every 10th block is PoS
static const int POS_DIFFICULTY_ADJUSTMENT_INTERVAL = 2016; // PoS difficulty adjustment interval

/** SHAHCOIN Core Token Creation parameters */
static const int64_t TOKEN_CREATION_FEE = 100 * 100000000; // 100 SHAH token creation fee (in shahi)
static const int64_t MAX_TOKEN_SUPPLY = 1000000000 * 100000000; // 1 billion max token supply (in shahi)
static const int MAX_TOKEN_DECIMALS = 18; // Maximum token decimals
static const int MAX_TOKEN_NAME_LENGTH = 32; // Maximum token name length
static const int MAX_TOKEN_SYMBOL_LENGTH = 8; // Maximum token symbol length

/** SHAHCOIN Core DEX/ShahSwap parameters */
static const int64_t DEX_SWAP_FEE_RATE = 10; // 0.1% swap fee (in basis points)
static const int64_t DEX_LIQUIDITY_FEE_RATE = 30; // 0.3% liquidity fee (in basis points)
static const int64_t MIN_LIQUIDITY_AMOUNT = 1000 * 100000000; // 1000 SHAH minimum liquidity

/** SHAHCOIN Core NFT parameters */
static const int64_t NFT_MINT_FEE = 10 * 100000000; // 10 SHAH NFT mint fee (in shahi)
static const int MAX_NFT_METADATA_SIZE = 1024; // Maximum NFT metadata size in bytes
static const int MAX_NFT_NAME_LENGTH = 64; // Maximum NFT name length
static const int MAX_NFT_DESCRIPTION_LENGTH = 256; // Maximum NFT description length

/** Block types for SHAHCOIN Core hybrid consensus */
enum BlockType {
    BLOCK_TYPE_POW = 0,  // Proof of Work block
    BLOCK_TYPE_POS = 1,  // Proof of Stake block
    BLOCK_TYPE_COUNT = 2
};

/** Transaction types for SHAHCOIN Core */
enum TransactionType {
    TX_TYPE_STANDARD = 0,    // Standard transaction
    TX_TYPE_TOKEN_CREATE = 1, // Token creation
    TX_TYPE_TOKEN_TRANSFER = 2, // Token transfer
    TX_TYPE_NFT_MINT = 3,    // NFT mint
    TX_TYPE_NFT_TRANSFER = 4, // NFT transfer
    TX_TYPE_DEX_SWAP = 5,    // DEX swap
    TX_TYPE_DEX_LIQUIDITY = 6, // DEX liquidity add/remove
    TX_TYPE_COUNT = 7
};

/** SHAHCOIN Core treasury address for fees */
static const char* TREASURY_ADDRESS = "SXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

/** SHAHCOIN Core maximum reorg depth for security */
static const int MAX_REORG_DEPTH = 100;

/** SHAHCOIN Core minimum relay fee in SHAH per KB */
static const int64_t MIN_RELAY_TX_FEE = 1000; // 0.001 SHAH per KB

/** Flags for nSequence and nLockTime locks */
/** Interpret sequence numbers as relative lock-time constraints. */
static constexpr unsigned int LOCKTIME_VERIFY_SEQUENCE = (1 << 0);

#endif // SHAHCOIN_CONSENSUS_CONSENSUS_H
