// Copyright (c) 2023 SHAHCOIN Core
// Distributed under the MIT software license

#ifndef SHAHCOIN_CHAINPARAMSBASE_H
#define SHAHCOIN_CHAINPARAMSBASE_H

#include <string>
#include <vector>
#include <memory>
#include "util/chaintype.h"

class ArgsManager;

/**
 * CBaseChainParams defines the base parameters (shared between shahcoind and shahcoin-qt)
 * of a given instance of the Shahcoin network.
 */
class CBaseChainParams {
public:
    enum Network {
        MAIN,
        TESTNET,
        REGTEST,
        MAX_NETWORK_TYPES
    };

    const std::string& DataDir() const { return strDataDir; }
    int RPCPort() const { return nRPCPort; }
    int OnionServiceTargetPort() const { return nRPCPort; }
    
    // Setters for configuration
    void SetRPCPort(int port) { nRPCPort = port; }
    void SetDataDir(const std::string& dir) { strDataDir = dir; }

public:
    CBaseChainParams() {}

protected:
    int nRPCPort;
    std::string strDataDir;
};

/**
 * Creates and returns base chain params for the given chain type.
 */
std::unique_ptr<CBaseChainParams> CreateBaseChainParams(ChainType chain);

/**
 * Returns base parameters for the given network name (main, testnet, regtest).
 * Raises a runtime_error if the network is unknown.
 */
const CBaseChainParams& BaseParams(const std::string& chain);

/**
 * Returns currently selected parameters.
 * This will not change after the app starts unless SelectBaseParams() is called.
 */
const CBaseChainParams& BaseParams();

/**
 * Sets base parameters according to the given network.
 */
void SelectBaseParams(const std::string& chain);

/**
 * Checks if base parameters are configured.
 */
bool AreBaseParamsConfigured();

/**
 * Setup chain params base options for ArgManager.
 */
void SetupChainParamsBaseOptions(ArgsManager& argsman);

#endif // SHAHCOIN_CHAINPARAMSBASE_H
