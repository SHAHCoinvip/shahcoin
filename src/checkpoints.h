// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see COPYING file.

#ifndef SHAHCOIN_CHECKPOINTS_H
#define SHAHCOIN_CHECKPOINTS_H

#include <uint256.h>
#include <map>
#include "chain_tx_data.h"
#include "checkpoint_data.h"

class CBlockIndex;

typedef std::map<int, uint256> MapCheckpoints;

#endif // SHAHCOIN_CHECKPOINTS_H
