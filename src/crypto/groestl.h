// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_CRYPTO_GROESTL_H
#define SHAHCOIN_CRYPTO_GROESTL_H

#include <cstdint>
#include <cstdlib>

/** Groestl hashing algorithm implementation */
class CGroestl
{
public:
    static const size_t OUTPUT_SIZE = 32;

    CGroestl();
    ~CGroestl();

    void Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    void Reset();

    // Static convenience methods
    static void Hash(const unsigned char* input, size_t len, unsigned char output[OUTPUT_SIZE]);
    static void Hash(const unsigned char* input, size_t len, unsigned char* output, size_t outlen);

private:
    void* pimpl;
};

#endif // SHAHCOIN_CRYPTO_GROESTL_H
