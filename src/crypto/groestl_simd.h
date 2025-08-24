// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_CRYPTO_GROESTL_SIMD_H
#define SHAHCOIN_CRYPTO_GROESTL_SIMD_H

#include <cstdint>
#include <cstddef>

// SIMD-optimized Groestl implementation
// This provides significant performance improvements on modern CPUs

#ifdef __AVX2__
#define GROESTL_USE_AVX2 1
#endif

#ifdef __SSE4_1__
#define GROESTL_USE_SSE4 1
#endif

// SIMD-optimized Groestl-256 hash function
void groestl_256_hash_simd(const unsigned char* input, size_t len, unsigned char* output);

// Auto-detect best available SIMD implementation
void groestl_256_hash_auto(const unsigned char* input, size_t len, unsigned char* output);

// Check if SIMD is available
bool groestl_simd_available();

// Performance benchmark function
double groestl_benchmark_simd(size_t iterations);

#endif // SHAHCOIN_CRYPTO_GROESTL_SIMD_H
