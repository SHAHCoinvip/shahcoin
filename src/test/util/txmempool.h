// Copyright (c) 2022 The Shahcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_TEST_UTIL_TXMEMPOOL_H
#define SHAHCOIN_TEST_UTIL_TXMEMPOOL_H

#include <txmempool.h>
#include <util/time.h>

namespace node {
struct NodeContext;
}

CTxMemPool::Options MemPoolOptionsForTest(const node::NodeContext& node);

struct TestMemPoolEntryHelper {
    // Default values
    CAmount nFee{0};
    NodeSeconds time{};
    unsigned int nHeight{1};
    uint64_t m_sequence{0};
    bool spendsCoinbase{false};
    unsigned int sigOpCost{4};
    LockPoints lp;

    CTxMemPoolEntry FromTx(const CMutableTransaction& tx) const;
    CTxMemPoolEntry FromTx(const CTransactionRef& tx) const;

    // Change the default value
    TestMemPoolEntryHelper& Fee(CAmount _fee) { nFee = _fee; return *this; }
    TestMemPoolEntryHelper& Time(NodeSeconds tp) { time = tp; return *this; }
    TestMemPoolEntryHelper& Height(unsigned int _height) { nHeight = _height; return *this; }
    TestMemPoolEntryHelper& Sequence(uint64_t _seq) { m_sequence = _seq; return *this; }
    TestMemPoolEntryHelper& SpendsCoinbase(bool _flag) { spendsCoinbase = _flag; return *this; }
    TestMemPoolEntryHelper& SigOpsCost(unsigned int _sigopsCost) { sigOpCost = _sigopsCost; return *this; }
};

#endif // SHAHCOIN_TEST_UTIL_TXMEMPOOL_H
