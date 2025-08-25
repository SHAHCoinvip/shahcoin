// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_QT_SHAHCOINADDRESSVALIDATOR_H
#define SHAHCOIN_QT_SHAHCOINADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class ShahcoinAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit ShahcoinAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

/** Shahcoin address widget validator, checks for a valid shahcoin address.
 */
class ShahcoinAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit ShahcoinAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

#endif // SHAHCOIN_QT_SHAHCOINADDRESSVALIDATOR_H
