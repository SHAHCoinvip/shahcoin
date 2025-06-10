// Copyright (c) 2021-2022 The Shahcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//! @file
//! @brief Common init functions shared by shahcoin-node, shahcoin-wallet, etc.

#ifndef SHAHCOIN_INIT_COMMON_H
#define SHAHCOIN_INIT_COMMON_H

#include <util/result.h>

class ArgsManager;

namespace init {
void AddLoggingArgs(ArgsManager& args);
void SetLoggingOptions(const ArgsManager& args);
[[nodiscard]] util::Result<void> SetLoggingCategories(const ArgsManager& args);
[[nodiscard]] util::Result<void> SetLoggingLevel(const ArgsManager& args);
bool StartLogging(const ArgsManager& args);
void LogPackageVersion();
} // namespace init

#endif // SHAHCOIN_INIT_COMMON_H
