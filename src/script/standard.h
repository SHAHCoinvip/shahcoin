#ifndef SHAHCOIN_SCRIPT_STANDARD_H
#define SHAHCOIN_SCRIPT_STANDARD_H

#include <script/script.h>
#include <script/interpreter.h>
#include <uint256.h>

enum txnouttype
{
    TX_NONSTANDARD,
    TX_PUBKEY,
    TX_PUBKEYHASH,
    TX_SCRIPTHASH,
    TX_MULTISIG,
    TX_NULL_DATA,
    TX_WITNESS_V0_KEYHASH,
    TX_WITNESS_V0_SCRIPTHASH,
};

#endif // SHAHCOIN_SCRIPT_STANDARD_H
