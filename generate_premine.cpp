// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

// SHAHCOIN address prefix (mainnet)
const unsigned char SHAH_ADDRESS_PREFIX = 0x3F; // 'S' character
const unsigned char SHAH_SECRET_PREFIX = 0xBF;  // Private key WIF prefix

// Helper function to convert bytes to hex string
std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

// Helper function to convert hex string to bytes
std::vector<unsigned char> hexToBytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
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

// Base58 encoding
std::string base58Encode(const std::vector<unsigned char>& input) {
    const std::string alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    std::vector<unsigned char> digits((input.size() * 138 / 100) + 1);
    size_t digitslen = 1;
    
    for (size_t i = 0; i < input.size(); i++) {
        unsigned int carry = input[i];
        for (size_t j = 0; j < digitslen; j++) {
            carry += static_cast<unsigned int>(digits[j]) * 256;
            digits[j] = carry % 58;
            carry /= 58;
        }
        while (carry > 0) {
            digits[digitslen++] = carry % 58;
            carry /= 58;
        }
    }
    
    std::string result;
    for (size_t i = 0; i < input.size() && input[i] == 0; i++) {
        result += '1';
    }
    for (size_t i = 0; i < digitslen; i++) {
        result += alphabet[digits[digitslen - 1 - i]];
    }
    
    return result;
}

// Generate SHAHCOIN address from public key
std::string generateShahAddress(const std::vector<unsigned char>& publicKey) {
    // SHA256 + RIPEMD160 of public key
    auto sha256Hash = doubleSHA256(publicKey);
    auto ripemd160Hash = ripemd160(sha256Hash);
    
    // Add version byte
    std::vector<unsigned char> versionedHash;
    versionedHash.push_back(SHAH_ADDRESS_PREFIX);
    versionedHash.insert(versionedHash.end(), ripemd160Hash.begin(), ripemd160Hash.end());
    
    // Double SHA256 for checksum
    auto checksum = doubleSHA256(versionedHash);
    
    // Add first 4 bytes of checksum
    std::vector<unsigned char> addressBytes = versionedHash;
    addressBytes.insert(addressBytes.end(), checksum.begin(), checksum.begin() + 4);
    
    // Base58 encode
    return base58Encode(addressBytes);
}

// Generate WIF private key
std::string generateWIF(const std::vector<unsigned char>& privateKey) {
    // Add version byte
    std::vector<unsigned char> versionedKey;
    versionedKey.push_back(SHAH_SECRET_PREFIX);
    versionedKey.insert(versionedKey.end(), privateKey.begin(), privateKey.end());
    
    // Add compression flag (0x01 for compressed)
    versionedKey.push_back(0x01);
    
    // Double SHA256 for checksum
    auto checksum = doubleSHA256(versionedKey);
    
    // Add first 4 bytes of checksum
    std::vector<unsigned char> wifBytes = versionedKey;
    wifBytes.insert(wifBytes.end(), checksum.begin(), checksum.begin() + 4);
    
    // Base58 encode
    return base58Encode(wifBytes);
}

// Generate new keypair
void generateKeypair() {
    std::cout << "🔐 Generating SHAHCOIN Premine Keypair..." << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // Generate new EC key
    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!key) {
        std::cerr << "Error: Failed to create EC key" << std::endl;
        return;
    }
    
    // Generate keypair
    if (EC_KEY_generate_key(key) != 1) {
        std::cerr << "Error: Failed to generate keypair" << std::endl;
        EC_KEY_free(key);
        return;
    }
    
    // Get private key
    const BIGNUM* priv = EC_KEY_get0_private_key(key);
    if (!priv) {
        std::cerr << "Error: Failed to get private key" << std::endl;
        EC_KEY_free(key);
        return;
    }
    
    // Convert private key to bytes
    std::vector<unsigned char> privateKeyBytes(32);
    BN_bn2bin(priv, privateKeyBytes.data());
    
    // Get public key
    const EC_POINT* pub = EC_KEY_get0_public_key(key);
    if (!pub) {
        std::cerr << "Error: Failed to get public key" << std::endl;
        EC_KEY_free(key);
        return;
    }
    
    // Convert public key to bytes (compressed format)
    std::vector<unsigned char> publicKeyBytes(33);
    EC_POINT_point2oct(EC_KEY_get0_group(key), pub, POINT_CONVERSION_COMPRESSED, 
                       publicKeyBytes.data(), 33, nullptr);
    
    // Generate address and WIF
    std::string address = generateShahAddress(publicKeyBytes);
    std::string wif = generateWIF(privateKeyBytes);
    
    // Output results
    std::cout << std::endl;
    std::cout << "✅ SHAHCOIN Premine Wallet Generated Successfully!" << std::endl;
    std::cout << std::endl;
    std::cout << "📋 WALLET INFORMATION (SAVE SECURELY):" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "SHAH Address: " << address << std::endl;
    std::cout << "Public Key:   " << bytesToHex(publicKeyBytes) << std::endl;
    std::cout << "Private Key:  " << wif << std::endl;
    std::cout << std::endl;
    std::cout << "⚠️  SECURITY WARNING:" << std::endl;
    std::cout << "   - Store private key in secure cold storage" << std::endl;
    std::cout << "   - Never share private key with anyone" << std::endl;
    std::cout << "   - This keypair will receive 8,000,000 SHAH premine" << std::endl;
    std::cout << std::endl;
    
    // Clean up
    EC_KEY_free(key);
    
    // Save to file for reference
    std::ofstream walletFile("shahcoin_premine_wallet.txt");
    if (walletFile.is_open()) {
        walletFile << "SHAHCOIN Premine Wallet - Generated " << std::endl;
        walletFile << "=====================================" << std::endl;
        walletFile << "SHAH Address: " << address << std::endl;
        walletFile << "Public Key:   " << bytesToHex(publicKeyBytes) << std::endl;
        walletFile << "Private Key:  " << wif << std::endl;
        walletFile << std::endl;
        walletFile << "This wallet will receive 8,000,000 SHAH premine" << std::endl;
        walletFile << "Store private key securely in cold storage!" << std::endl;
        walletFile.close();
        std::cout << "💾 Wallet info saved to: shahcoin_premine_wallet.txt" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "🔧 Next Steps:" << std::endl;
    std::cout << "   1. Copy the public key above" << std::endl;
    std::cout << "   2. Update chainparams.cpp with the public key" << std::endl;
    std::cout << "   3. Regenerate genesis block" << std::endl;
    std::cout << "   4. Store private key securely" << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "🚀 SHAHCOIN Premine Setup Tool" << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << "This tool will generate a secure keypair for the SHAHCOIN premine." << std::endl;
    std::cout << "The generated wallet will receive 8,000,000 SHAH in the genesis block." << std::endl;
    std::cout << std::endl;
    
    generateKeypair();
    
    return 0;
}
