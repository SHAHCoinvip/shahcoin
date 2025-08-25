#include <iostream>
#include <stdint.h>
#include "primitives/block.h"
#include "consensus/merkle.h"
#include "util/time.h"
#include "util/system.h"
#include "chainparams.h"
#include "util/strencodings.h"
#include "arith_uint256.h"

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nshahbits,
                                 int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Shahcoin begins - July 2025";
    const CScript genesisOutputScript = CScript() << ParseHex("04ffff001d0104") << OP_CHECKSIG;

    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nshahbits    = nshahbits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(txNew));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

int main()
{
    const uint32_t nGenesisTime = 1719868800;  // Replace with your desired timestamp
    const uint32_t nshahbits = 0x1e0ffff0;
    const int32_t nVersion = 1;
    const CAmount genesisReward = 50 * COIN;

    for (uint32_t nonce = 0; nonce < UINT32_MAX; ++nonce)
    {
        CBlock genesis = CreateGenesisBlock(nGenesisTime, nonce, nshahbits, nVersion, genesisReward);
        arith_uint256 hashTarget = arith_uint256().SetCompact(nshahbits);
        if (UintToArith256(genesis.GetHash()) <= hashTarget)
        {
            std::cout << "✅ Genesis block found!" << std::endl;
            std::cout << "Nonce: " << nonce << std::endl;
            std::cout << "Hash: " << genesis.GetHash().ToString() << std::endl;
            std::cout << "Merkle Root: " << genesis.hashMerkleRoot.ToString() << std::endl;
            break;
        }
    }
    return 0;
}
