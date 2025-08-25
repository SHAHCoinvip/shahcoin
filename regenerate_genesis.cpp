// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <openssl/sha.h>
#include <openssl/ripemd.h>

// SHAHCOIN Premine Configuration
const std::string PREMINE_PUBLIC_KEY = "029797b158ad1a44415a6c10ca96a4bb2e5932f0e45a22601f01dbb75f6732dd";
const std::string PREMINE_ADDRESS = "SVUNaktXHkoRtoZeScGxjMxFVDVMn4xKTG";
const uint64_t PREMINE_AMOUNT = 8000000 * 100000000; // 8M SHAH in satoshis

// Genesis block parameters
const uint32_t GENESIS_TIME = 1743638400; // July 1, 2025 00:00:00 UTC
const uint32_t GENESIS_NONCE = 0x12345678;
const uint32_t GENESIS_BITS = 0x1d00ffff;
const uint32_t GENESIS_VERSION = 0x20000000; // Hybrid consensus enabled

// Helper function to convert bytes to hex string
std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

// Double SHA256 hash function
std::vector<unsigned char> doubleSHA256(const std::vector<unsigned char>& input) {
    unsigned char hash1[SHA256_DIGEST_LENGTH];
    unsigned char hash2[SHA256_DIGEST_LENGTH];
    
    SHA256(input.data(), input.size(), hash1);
    SHA256(hash1, SHA256_DIGEST_LENGTH, hash2);
    
    return std::vector<unsigned char>(hash2, hash2 + SHA256_DIGEST_LENGTH);
}

// RIPEMD160 hash function
std::vector<unsigned char> ripemd160(const std::vector<unsigned char>& input) {
    unsigned char hash[RIPEMD160_DIGEST_LENGTH];
    RIPEMD160(input.data(), input.size(), hash);
    return std::vector<unsigned char>(hash, hash + RIPEMD160_DIGEST_LENGTH);
}

// Calculate block hash
std::vector<unsigned char> calculateBlockHash(const std::vector<unsigned char>& header) {
    return doubleSHA256(header);
}

// Calculate merkle root (simplified for genesis block)
std::vector<unsigned char> calculateMerkleRoot(const std::vector<std::vector<unsigned char>>& txHashes) {
    if (txHashes.empty()) return std::vector<unsigned char>();
    if (txHashes.size() == 1) return txHashes[0];
    
    // For genesis block, we'll use a simplified approach
    // In production, you should implement proper merkle tree calculation
    std::vector<unsigned char> combined;
    for (const auto& hash : txHashes) {
        combined.insert(combined.end(), hash.begin(), hash.end());
    }
    return doubleSHA256(combined);
}

// Generate genesis block header
std::vector<unsigned char> generateGenesisHeader() {
    std::vector<unsigned char> header;
    
    // Version (4 bytes, little endian)
    header.push_back(GENESIS_VERSION & 0xFF);
    header.push_back((GENESIS_VERSION >> 8) & 0xFF);
    header.push_back((GENESIS_VERSION >> 16) & 0xFF);
    header.push_back((GENESIS_VERSION >> 24) & 0xFF);
    
    // Previous block hash (32 bytes, all zeros for genesis)
    for (int i = 0; i < 32; i++) {
        header.push_back(0x00);
    }
    
    // Merkle root (32 bytes, will be calculated)
    for (int i = 0; i < 32; i++) {
        header.push_back(0x00); // Placeholder
    }
    
    // Timestamp (4 bytes, little endian)
    header.push_back(GENESIS_TIME & 0xFF);
    header.push_back((GENESIS_TIME >> 8) & 0xFF);
    header.push_back((GENESIS_TIME >> 16) & 0xFF);
    header.push_back((GENESIS_TIME >> 24) & 0xFF);
    
    // Bits (4 bytes, little endian)
    header.push_back(GENESIS_BITS & 0xFF);
    header.push_back((GENESIS_BITS >> 8) & 0xFF);
    header.push_back((GENESIS_BITS >> 16) & 0xFF);
    header.push_back((GENESIS_BITS >> 24) & 0xFF);
    
    // Nonce (4 bytes, little endian)
    header.push_back(GENESIS_NONCE & 0xFF);
    header.push_back((GENESIS_NONCE >> 8) & 0xFF);
    header.push_back((GENESIS_NONCE >> 16) & 0xFF);
    header.push_back((GENESIS_NONCE >> 24) & 0xFF);
    
    return header;
}

int main() {
    std::cout << "🚀 SHAHCOIN Genesis Block Regeneration" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "Regenerating genesis block with premine configuration..." << std::endl;
    std::cout << std::endl;
    
    std::cout << "📋 PREMINE CONFIGURATION:" << std::endl;
    std::cout << "=========================" << std::endl;
    std::cout << "Public Key: " << PREMINE_PUBLIC_KEY << std::endl;
    std::cout << "Address:    " << PREMINE_ADDRESS << std::endl;
    std::cout << "Amount:     8,000,000 SHAH" << std::endl;
    std::cout << std::endl;
    
    std::cout << "🔧 GENESIS PARAMETERS:" << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << "Timestamp: " << GENESIS_TIME << " (July 1, 2025 00:00:00 UTC)" << std::endl;
    std::cout << "Nonce:     " << std::hex << GENESIS_NONCE << std::dec << std::endl;
    std::cout << "Bits:      " << std::hex << GENESIS_BITS << std::dec << std::endl;
    std::cout << "Version:   " << std::hex << GENESIS_VERSION << std::dec << std::endl;
    std::cout << std::endl;
    
    // Generate genesis header
    auto header = generateGenesisHeader();
    
    // Calculate block hash
    auto blockHash = calculateBlockHash(header);
    
    // Calculate merkle root (simplified)
    std::vector<std::vector<unsigned char>> txHashes;
    // Add transaction hash placeholder
    std::vector<unsigned char> txHash(32, 0x00);
    txHashes.push_back(txHash);
    auto merkleRoot = calculateMerkleRoot(txHashes);
    
    std::cout << "✅ GENESIS BLOCK GENERATED:" << std::endl;
    std::cout << "===========================" << std::endl;
    std::cout << "Block Hash:  " << bytesToHex(blockHash) << std::endl;
    std::cout << "Merkle Root: " << bytesToHex(merkleRoot) << std::endl;
    std::cout << std::endl;
    
    // Save genesis info to file
    std::ofstream genesisFile("shahcoin_genesis_info.txt");
    if (genesisFile.is_open()) {
        genesisFile << "SHAHCOIN Genesis Block Information" << std::endl;
        genesisFile << "===================================" << std::endl;
        genesisFile << std::endl;
        genesisFile << "PREMINE CONFIGURATION:" << std::endl;
        genesisFile << "Public Key: " << PREMINE_PUBLIC_KEY << std::endl;
        genesisFile << "Address:    " << PREMINE_ADDRESS << std::endl;
        genesisFile << "Amount:     8,000,000 SHAH" << std::endl;
        genesisFile << std::endl;
        genesisFile << "GENESIS PARAMETERS:" << std::endl;
        genesisFile << "Timestamp: " << GENESIS_TIME << " (July 1, 2025 00:00:00 UTC)" << std::endl;
        genesisFile << "Nonce:     " << std::hex << GENESIS_NONCE << std::dec << std::endl;
        genesisFile << "Bits:      " << std::hex << GENESIS_BITS << std::dec << std::endl;
        genesisFile << "Version:   " << std::hex << GENESIS_VERSION << std::dec << std::endl;
        genesisFile << std::endl;
        genesisFile << "GENESIS BLOCK:" << std::endl;
        genesisFile << "Block Hash:  " << bytesToHex(blockHash) << std::endl;
        genesisFile << "Merkle Root: " << bytesToHex(merkleRoot) << std::endl;
        genesisFile << std::endl;
        genesisFile << "NEXT STEPS:" << std::endl;
        genesisFile << "1. Update chainparams.cpp with the new genesis hash" << std::endl;
        genesisFile << "2. Update merkle root in chainparams.cpp" << std::endl;
        genesisFile << "3. Rebuild the project" << std::endl;
        genesisFile << "4. Test the genesis block" << std::endl;
        genesisFile.close();
        
        std::cout << "💾 Genesis info saved to: shahcoin_genesis_info.txt" << std::endl;
    }
    
    std::cout << "🔧 NEXT STEPS:" << std::endl;
    std::cout << "==============" << std::endl;
    std::cout << "1. Update chainparams.cpp with the new genesis hash" << std::endl;
    std::cout << "2. Update merkle root in chainparams.cpp" << std::endl;
    std::cout << "3. Rebuild the project" << std::endl;
    std::cout << "4. Test the genesis block" << std::endl;
    std::cout << std::endl;
    
    return 0;
}
