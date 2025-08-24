// src/checkpoint_data.h
#ifndef SHAHCOIN_CHECKPOINT_DATA_H
#define SHAHCOIN_CHECKPOINT_DATA_H

#include <uint256.h>
#include <map>

struct CCheckpointData {
    std::map<int, uint256> mapCheckpoints;
    
    int GetHeight() const {
        if (mapCheckpoints.empty()) {
            return 0;
        }
        return mapCheckpoints.rbegin()->first;
    }
};

#endif // SHAHCOIN_CHECKPOINT_DATA_H
