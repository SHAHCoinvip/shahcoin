// Copyright (c) 2018-2020 The Shahcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_TEST_ADDRESSBOOKTESTS_H
#define SHAHCOIN_QT_TEST_ADDRESSBOOKTESTS_H

#include <QObject>
#include <QTest>

namespace interfaces {
class Node;
} // namespace interfaces

class AddressBookTests : public QObject
{
public:
    explicit AddressBookTests(interfaces::Node& node) : m_node(node) {}
    interfaces::Node& m_node;

    Q_OBJECT

private Q_SLOTS:
    void addressBookTests();
};

#endif // SHAHCOIN_QT_TEST_ADDRESSBOOKTESTS_H
