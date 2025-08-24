// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_CRYPTO_SCRYPT_OPTIMIZED_H
#define SHAHCOIN_CRYPTO_SCRYPT_OPTIMIZED_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <thread>

// Optimized Scrypt implementation with memory and performance improvements

// Memory-optimized Scrypt parameters
struct ScryptParams {
    uint32_t N;      // CPU/Memory cost parameter (must be power of 2)
    uint32_t r;      // Block size parameter
    uint32_t p;      // Parallelization parameter
    uint32_t dkLen;  // Derived key length
    
    ScryptParams(uint32_t n = 1024, uint32_t block_size = 1, uint32_t parallel = 1, uint32_t key_len = 32)
        : N(n), r(block_size), p(parallel), dkLen(key_len) {}
};

// Memory-optimized Scrypt implementation
void scrypt_1024_1_1_256_optimized(const char* input, char* output);

// Parallel Scrypt implementation
void scrypt_1024_1_1_256_parallel(const char* input, char* output, unsigned int num_threads = 0);

// Memory-efficient Scrypt with configurable parameters
void scrypt_hash_optimized(const char* input, size_t input_len, char* output, 
                          const ScryptParams& params = ScryptParams());

// Check available memory for Scrypt
size_t scrypt_get_available_memory();

// Auto-configure Scrypt parameters based on available memory
ScryptParams scrypt_auto_params(size_t max_memory_mb = 0);

// Performance benchmark
double scrypt_benchmark_optimized(size_t iterations);

// Memory usage estimation
size_t scrypt_memory_usage(const ScryptParams& params);

#endif // SHAHCOIN_CRYPTO_SCRYPT_OPTIMIZED_H
