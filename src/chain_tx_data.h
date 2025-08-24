// src/chain_tx_data.h
#ifndef SHAHCOIN_CHAIN_TX_DATA_H
#define SHAHCOIN_CHAIN_TX_DATA_H

#include <cstdint>

struct ChainTxData {
    int64_t nTime;
    int64_t nTxCount;
    double dTxRate;
};

#endif // SHAHCOIN_CHAIN_TX_DATA_H
