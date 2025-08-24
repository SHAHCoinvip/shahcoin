// SPDX-License-Identifier: Apache-2.0
#include "pow_dispatch.h"
#include "crypto/scrypt.h"
#include "crypto/groestl.h"
#include <openssl/sha.h>

static void sha256d(const unsigned char* in, size_t len, unsigned char* out) {
    unsigned char tmp[32];
    SHA256(in, len, tmp);
    SHA256(tmp, 32, out);
}

void GetPoWHash(const std::vector<unsigned char>& header, AlgoType algo, unsigned char* out32) {
    switch (algo) {
        case AlgoType::SHA256D:
            sha256d(header.data(), header.size(), out32);
            break;
        case AlgoType::SCRYPT:
            CScrypt::Hash(header.data(), header.size(), out32);
            break;
        case AlgoType::GROESTL:
            CGroestl::Hash(header.data(), header.size(), out32);
            break;
        case AlgoType::POS:
            for (int i = 0; i < 32; ++i) out32[i] = 0; // PoS uses kernel checks, no PoW hash
            break;
    }
}


