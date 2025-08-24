// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_CRYPTO_SCRYPT_H
#define SHAHCOIN_CRYPTO_SCRYPT_H

#include <cstdint>
#include <cstdlib>

/** Scrypt hashing algorithm implementation */
class CScrypt
{
public:
    static const size_t OUTPUT_SIZE = 32;

    CScrypt();
    ~CScrypt();

    void Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    void Reset();

    // Static convenience methods
    static void Hash(const unsigned char* input, size_t len, unsigned char output[OUTPUT_SIZE]);
    static void Hash(const unsigned char* input, size_t len, unsigned char* output, size_t outlen);

private:
    void* pimpl;
};

#endif // SHAHCOIN_CRYPTO_SCRYPT_H
