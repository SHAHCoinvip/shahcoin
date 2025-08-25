// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_CRYPTO_MULTIHASH_H
#define SHAHCOIN_CRYPTO_MULTIHASH_H

#include <crypto/sha256.h>
#include <crypto/scrypt.h>
#include <crypto/groestl.h>
#include <consensus/consensus.h>
#include <uint256.h>

/** SHAHCOIN Core multi-algorithm hashing utility */
class CMultiHash
{
public:
    static uint256 Hash(const unsigned char* data, size_t len, MiningAlgorithm algo);
    static uint256 Hash(const uint256& input, MiningAlgorithm algo);
    static uint256 Hash(const std::vector<unsigned char>& input, MiningAlgorithm algo);
    
    // Get the algorithm name as string
    static std::string GetAlgorithmName(MiningAlgorithm algo);
    
    // Get the algorithm from string
    static MiningAlgorithm GetAlgorithmFromString(const std::string& name);
    
    // Get the algorithm for a specific block height (for rotation)
    static MiningAlgorithm GetAlgorithmForHeight(int height);
    
    // Check if an algorithm is valid
    static bool IsValidAlgorithm(MiningAlgorithm algo);
};

#endif // SHAHCOIN_CRYPTO_MULTIHASH_H
