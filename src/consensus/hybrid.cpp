// SPDX-License-Identifier: Apache-2.0
#include "consensus/hybrid.h"

AlgoType SelectNextAlgo(int height) {
    // 33/33/34 rotation for SHA256d/Scrypt/Groestl. PoS interleave to be handled elsewhere.
    int m = height % 3;
    if (m == 0) return AlgoType::SHA256D;
    if (m == 1) return AlgoType::SCRYPT;
    return AlgoType::GROESTL;
}

const char* AlgoName(AlgoType a) {
    switch (a) {
        case AlgoType::SHA256D: return "sha256d";
        case AlgoType::SCRYPT:  return "scrypt";
        case AlgoType::GROESTL: return "groestl";
        case AlgoType::POS:     return "pos";
    }
    return "unknown";
}

// LWMA (Linear Weighted Moving Average) difficulty adjustment
// This provides better responsiveness to hash rate changes

unsigned int GetNextWorkRequiredSHA256(int height) {
    // LWMA difficulty adjustment for SHA256d
    const int N = 144; // 24 hours worth of blocks
    const int T = 600; // 10 minute target spacing
    
    if (height < N) return 0x1d00ffff; // Genesis difficulty
    
    // Calculate weighted average of solve times
    uint64_t weightedSum = 0;
    uint64_t weightSum = 0;
    
    for (int i = 0; i < N; i++) {
        int weight = N - i;
        // TODO: Get actual solve time from block at height - i
        uint64_t solveTime = T; // Placeholder
        weightedSum += solveTime * weight;
        weightSum += weight;
    }
    
    uint64_t avgSolveTime = weightedSum / weightSum;
    uint64_t targetSpacing = T;
    
    // Adjust difficulty based on solve time ratio
    uint64_t newTarget = (avgSolveTime * 0x1d00ffff) / targetSpacing;
    
    // Clamp to reasonable bounds
    if (newTarget > 0x1d00ffff * 4) newTarget = 0x1d00ffff * 4;
    if (newTarget < 0x1d00ffff / 4) newTarget = 0x1d00ffff / 4;
    
    return static_cast<unsigned int>(newTarget);
}

unsigned int GetNextWorkRequiredScrypt(int height) {
    // LWMA difficulty adjustment for Scrypt
    const int N = 144; // 24 hours worth of blocks
    const int T = 600; // 10 minute target spacing
    
    if (height < N) return 0x1d00ffff; // Genesis difficulty
    
    // Calculate weighted average of solve times
    uint64_t weightedSum = 0;
    uint64_t weightSum = 0;
    
    for (int i = 0; i < N; i++) {
        int weight = N - i;
        // TODO: Get actual solve time from block at height - i
        uint64_t solveTime = T; // Placeholder
        weightedSum += solveTime * weight;
        weightSum += weight;
    }
    
    uint64_t avgSolveTime = weightedSum / weightSum;
    uint64_t targetSpacing = T;
    
    // Adjust difficulty based on solve time ratio
    uint64_t newTarget = (avgSolveTime * 0x1d00ffff) / targetSpacing;
    
    // Clamp to reasonable bounds
    if (newTarget > 0x1d00ffff * 4) newTarget = 0x1d00ffff * 4;
    if (newTarget < 0x1d00ffff / 4) newTarget = 0x1d00ffff / 4;
    
    return static_cast<unsigned int>(newTarget);
}

unsigned int GetNextWorkRequiredGroestl(int height) {
    // LWMA difficulty adjustment for Groestl
    const int N = 144; // 24 hours worth of blocks
    const int T = 600; // 10 minute target spacing
    
    if (height < N) return 0x1d00ffff; // Genesis difficulty
    
    // Calculate weighted average of solve times
    uint64_t weightedSum = 0;
    uint64_t weightSum = 0;
    
    for (int i = 0; i < N; i++) {
        int weight = N - i;
        // TODO: Get actual solve time from block at height - i
        uint64_t solveTime = T; // Placeholder
        weightedSum += solveTime * weight;
        weightSum += weight;
    }
    
    uint64_t avgSolveTime = weightedSum / weightSum;
    uint64_t targetSpacing = T;
    
    // Adjust difficulty based on solve time ratio
    uint64_t newTarget = (avgSolveTime * 0x1d00ffff) / targetSpacing;
    
    // Clamp to reasonable bounds
    if (newTarget > 0x1d00ffff * 4) newTarget = 0x1d00ffff * 4;
    if (newTarget < 0x1d00ffff / 4) newTarget = 0x1d00ffff / 4;
    
    return static_cast<unsigned int>(newTarget);
}

unsigned int GetNextStakeTarget(int height) {
    // PoS difficulty adjustment
    const int N = 144; // 24 hours worth of blocks
    const int T = 150; // 2.5 minute target spacing for PoS
    
    if (height < N) return 0x1d00ffff; // Genesis difficulty
    
    // Calculate weighted average of solve times
    uint64_t weightedSum = 0;
    uint64_t weightSum = 0;
    
    for (int i = 0; i < N; i++) {
        int weight = N - i;
        // TODO: Get actual solve time from PoS block at height - i
        uint64_t solveTime = T; // Placeholder
        weightedSum += solveTime * weight;
        weightSum += weight;
    }
    
    uint64_t avgSolveTime = weightedSum / weightSum;
    uint64_t targetSpacing = T;
    
    // Adjust difficulty based on solve time ratio
    uint64_t newTarget = (avgSolveTime * 0x1d00ffff) / targetSpacing;
    
    // Clamp to reasonable bounds
    if (newTarget > 0x1d00ffff * 4) newTarget = 0x1d00ffff * 4;
    if (newTarget < 0x1d00ffff / 4) newTarget = 0x1d00ffff / 4;
    
    return static_cast<unsigned int>(newTarget);
}


