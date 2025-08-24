// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <crypto/scrypt.h>
#include <crypto/sha256.h>
#include <cstring>

// SHAHCOIN Core scrypt implementation
// For now, we'll implement a simplified scrypt-like function using SHA256
// In production, you would link against a proper scrypt library like libscrypt

CScrypt::CScrypt() : pimpl(nullptr)
{
    Reset();
}

CScrypt::~CScrypt()
{
    if (pimpl) {
        free(pimpl);
    }
}

void CScrypt::Write(const unsigned char* data, size_t len)
{
    // Simplified implementation - in production use proper scrypt
    // This is a placeholder that uses SHA256 for compatibility
    if (!pimpl) {
        pimpl = malloc(CSHA256::OUTPUT_SIZE);
    }
    
    CSHA256 hasher;
    hasher.Write(data, len);
    hasher.Finalize((unsigned char*)pimpl);
}

void CScrypt::Finalize(unsigned char hash[OUTPUT_SIZE])
{
    if (pimpl) {
        memcpy(hash, pimpl, OUTPUT_SIZE);
    } else {
        memset(hash, 0, OUTPUT_SIZE);
    }
}

void CScrypt::Reset()
{
    if (pimpl) {
        free(pimpl);
        pimpl = nullptr;
    }
}

void CScrypt::Hash(const unsigned char* input, size_t len, unsigned char output[OUTPUT_SIZE])
{
    CScrypt hasher;
    hasher.Write(input, len);
    hasher.Finalize(output);
}

void CScrypt::Hash(const unsigned char* input, size_t len, unsigned char* output, size_t outlen)
{
    if (outlen >= OUTPUT_SIZE) {
        Hash(input, len, output);
    }
}
