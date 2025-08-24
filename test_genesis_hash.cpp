#include <iostream>
#include <string>
#include "src/chainparams.h"
#include "src/primitives/block.h"

int main() {
    CChainParams* params = CreateChainParams(CBaseChainParams::REGTEST);
    CBlock genesis = params->GenesisBlock();
    
    std::cout << "Genesis block hash: " << genesis.GetHash().ToString() << std::endl;
    std::cout << "Genesis merkle root: " << genesis.hashMerkleRoot.ToString() << std::endl;
    
    return 0;
} 