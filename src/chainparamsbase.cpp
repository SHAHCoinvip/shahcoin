// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparamsbase.h>
#include <util/strencodings.h>
#include <tinyformat.h>
#include <util/chaintype.h>
#include <common/args.h>

#include <assert.h>
#include <stdexcept>

#include <string>
#include <vector>

static std::unique_ptr<CBaseChainParams> globalChainBaseParams;

const std::vector<std::string> AvailableNetworks()
{
    return {"main"}; // Only mainnet is available
}

std::unique_ptr<CBaseChainParams> CreateBaseChainParams(ChainType chain)
{
    switch (chain) {
    case ChainType::MAIN:
        return std::make_unique<CBaseChainParams>();
    default:
        throw std::runtime_error(strprintf("%s: Chain type not supported. Only mainnet is available.", __func__));
    }
}

const CBaseChainParams& BaseParams()
{
    assert(globalChainBaseParams);
    return *globalChainBaseParams;
}

void SelectBaseParams(const std::string& chain)
{
    globalChainBaseParams = std::make_unique<CBaseChainParams>();

    if (chain == "main") {
        globalChainBaseParams->SetRPCPort(8368);
        globalChainBaseParams->SetDataDir("main");
    } else {
        throw std::runtime_error(strprintf("%s: Chain %s is not supported. Only mainnet is available.", __func__, chain));
    }
}

bool AreBaseParamsConfigured()
{
    return globalChainBaseParams != nullptr;
}

void SetupChainParamsBaseOptions(ArgsManager& argsman)
{
    argsman.AddArg("-chain=<chain>", "Use the chain <chain> (default: main). Only mainnet is supported.", ArgsManager::ALLOW_ANY, OptionsCategory::CHAINPARAMS);
    // Testnet and regtest are disabled - only mainnet is available
}
