// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <crypto/multihash.h>
#include <cstring>
#include <algorithm>

uint256 CMultiHash::Hash(const unsigned char* data, size_t len, MiningAlgorithm algo)
{
    unsigned char hash[32];
    
    switch (algo) {
        case ALGO_SHA256D: {
            // Double SHA256
            CSHA256 hasher1, hasher2;
            hasher1.Write(data, len);
            hasher1.Finalize(hash);
            hasher2.Write(hash, CSHA256::OUTPUT_SIZE);
            hasher2.Finalize(hash);
            break;
        }
        case ALGO_SCRYPT: {
            CScrypt::Hash(data, len, hash);
            break;
        }
        case ALGO_GROESTL: {
            CGroestl::Hash(data, len, hash);
            break;
        }
        default:
            // Fallback to SHA256d
            CSHA256 hasher1, hasher2;
            hasher1.Write(data, len);
            hasher1.Finalize(hash);
            hasher2.Write(hash, CSHA256::OUTPUT_SIZE);
            hasher2.Finalize(hash);
            break;
    }
    
    return uint256(hash);
}

uint256 CMultiHash::Hash(const uint256& input, MiningAlgorithm algo)
{
    return Hash(input.begin(), input.size(), algo);
}

uint256 CMultiHash::Hash(const std::vector<unsigned char>& input, MiningAlgorithm algo)
{
    return Hash(input.data(), input.size(), algo);
}

std::string CMultiHash::GetAlgorithmName(MiningAlgorithm algo)
{
    switch (algo) {
        case ALGO_SHA256D: return "sha256d";
        case ALGO_SCRYPT: return "scrypt";
        case ALGO_GROESTL: return "groestl";
        default: return "unknown";
    }
}

MiningAlgorithm CMultiHash::GetAlgorithmFromString(const std::string& name)
{
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "sha256d" || lower == "sha256") return ALGO_SHA256D;
    if (lower == "scrypt") return ALGO_SCRYPT;
    if (lower == "groestl") return ALGO_GROESTL;
    
    return ALGO_SHA256D; // Default fallback
}

MiningAlgorithm CMultiHash::GetAlgorithmForHeight(int height)
{
    // Simple round-robin algorithm selection
    // In production, you might want a more sophisticated algorithm
    return static_cast<MiningAlgorithm>(height % ALGO_COUNT);
}

bool CMultiHash::IsValidAlgorithm(MiningAlgorithm algo)
{
    return algo >= 0 && algo < ALGO_COUNT;
}
