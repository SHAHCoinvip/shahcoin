// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see COPYING file.

#include <checkpoints.h>
#include <uint256.h>

// Define checkpoint at block 0 (genesis block)
static MapCheckpoints mapCheckpoints = {
    {0, uint256S("0000000004e79555daba236568b407582a5dedba3fcab929169e4a3e10db6c5")},
};

static const CCheckpointData data = {
    mapCheckpoints
};

