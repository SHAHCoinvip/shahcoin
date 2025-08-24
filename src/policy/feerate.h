// Copyright (c) 2009-2010 Shahi Nakamoto
// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_POLICY_FEERATE_H
#define SHAHCOIN_POLICY_FEERATE_H

#include <consensus/amount.h>
#include <serialize.h>


#include <cstdint>
#include <string>
#include <type_traits>

const std::string CURRENCY_UNIT = "SHAH"; // One formatted unit
const std::string CURRENCY_ATOM = "sat"; // One indivisible minimum value unit

/* Used to determine type of fee estimation requested */
enum class FeeEstimateMode {
    UNSET,        //!< Use default settings based on other criteria
    ECONOMICAL,   //!< Force estimateSmartFee to use non-conservative estimates
    CONSERVATIVE, //!< Force estimateSmartFee to use conservative estimates
    SHAH_KVB,      //!< Use SHAH/kvB fee rate unit
    SAT_VB,       //!< Use sat/vB fee rate unit
};

/**
 * Fee rate in shahis per kilovirtualbyte: CAmount / kvB
 */
class CFeeRate
{
private:
    /** Fee rate in sat/kvB (shahis per 1000 virtualbytes) */
    CAmount nshahisPerK;

public:
    /** Fee rate of 0 shahis per kvB */
    CFeeRate() : nshahisPerK(0) { }
    template<typename I>
    explicit CFeeRate(const I _nshahisPerK): nshahisPerK(_nshahisPerK) {
        // We've previously had bugs creep in from silent double->int conversion...
        static_assert(std::is_integral<I>::value, "CFeeRate should be used without floats");
    }

    /**
     * Construct a fee rate from a fee in shahis and a vsize in vB.
     *
     * param@[in]   nFeePaid    The fee paid by a transaction, in shahis
     * param@[in]   num_bytes   The vsize of a transaction, in vbytes
     */
    CFeeRate(const CAmount& nFeePaid, uint32_t num_bytes);

    /**
     * Return the fee in shahis for the given vsize in vbytes.
     * If the calculated fee would have fractional shahis, then the
     * returned fee will always be rounded up to the nearest shahi.
     */
    CAmount GetFee(uint32_t num_bytes) const;

    /**
     * Return the fee in shahis for a vsize of 1000 vbytes
     */
    CAmount GetFeePerK() const { return nshahisPerK; }
    friend bool operator<(const CFeeRate& a, const CFeeRate& b) { return a.nshahisPerK < b.nshahisPerK; }
    friend bool operator>(const CFeeRate& a, const CFeeRate& b) { return a.nshahisPerK > b.nshahisPerK; }
    friend bool operator==(const CFeeRate& a, const CFeeRate& b) { return a.nshahisPerK == b.nshahisPerK; }
    friend bool operator<=(const CFeeRate& a, const CFeeRate& b) { return a.nshahisPerK <= b.nshahisPerK; }
    friend bool operator>=(const CFeeRate& a, const CFeeRate& b) { return a.nshahisPerK >= b.nshahisPerK; }
    friend bool operator!=(const CFeeRate& a, const CFeeRate& b) { return a.nshahisPerK != b.nshahisPerK; }
    CFeeRate& operator+=(const CFeeRate& a) { nshahisPerK += a.nshahisPerK; return *this; }
    std::string ToString(const FeeEstimateMode& fee_estimate_mode = FeeEstimateMode::SHAH_KVB) const;

    SERIALIZE_METHODS(CFeeRate, obj) { READWRITE(obj.nshahisPerK); }
};

#endif // SHAHCOIN_POLICY_FEERATE_H
