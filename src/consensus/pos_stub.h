// SPDX-License-Identifier: Apache-2.0
#ifndef SHAHCOIN_CONSENSUS_POS_STUB_H
#define SHAHCOIN_CONSENSUS_POS_STUB_H

#include <cstdint>
#include <string>

struct StakeInputRef {
    std::string txid;
    uint32_t vout;
    uint64_t amount;      // satoshis
    uint32_t nTime;       // UTXO time
};

struct PosKernel {
    uint64_t stakeModifier;
    uint32_t nTimeTx;
};

bool CheckProofOfStake(const StakeInputRef& input, const PosKernel& kernel, unsigned int stakeTarget);
uint64_t ComputeStakeModifier(uint64_t prevModifier, const std::string& seed);
bool IsValidCoinstakeTimestamp(uint32_t nTimeTx);

#endif // SHAHCOIN_CONSENSUS_POS_STUB_H


