#ifndef SHAHCOIN_AMOUNT_H
#define SHAHCOIN_AMOUNT_H

#include <stdint.h>
#include <string>

typedef int64_t CAmount;

static const CAmount COIN = 100000000;
static const CAmount CENT = 1000000;

bool MoneyRange(const CAmount& nValue);

#endif // SHAHCOIN_AMOUNT_H
