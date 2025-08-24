// src/chain_type.h
#ifndef SHAHCOIN_CHAIN_TYPE_H
#define SHAHCOIN_CHAIN_TYPE_H

#include <string>

enum class ChainType {
    MAIN,
    TESTNET,
    REGTEST
};

inline std::string ChainTypeToString(ChainType type) {
    switch (type) {
        case ChainType::MAIN: return "main";
        case ChainType::TESTNET: return "test";
        case ChainType::REGTEST: return "regtest";
        default: return "unknown";
    }
}

#endif // SHAHCOIN_CHAIN_TYPE_H
