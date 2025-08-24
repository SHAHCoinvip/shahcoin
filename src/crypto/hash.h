#ifndef SHAHCOIN_CRYPTO_HASH_H
#define SHAHCOIN_CRYPTO_HASH_H

#include <span.h>
#include <uint256.h>
#include <cstdint>

// Declare the low-level hash function
uint256 Hash(const uint8_t* begin, const uint8_t* end);

// Template wrapper for any data type
template<typename T>
inline uint256 Hash(const T& data) {
    return Hash((const uint8_t*)&data, (const uint8_t*)&data + sizeof(T));
}

#endif // SHAHCOIN_CRYPTO_HASH_H
