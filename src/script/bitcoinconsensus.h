// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Shahcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_SCRIPT_SHAHCOINCONSENSUS_H
#define SHAHCOIN_SCRIPT_SHAHCOINCONSENSUS_H

#include <stdint.h>

#if defined(BUILD_SHAHCOIN_INTERNAL) && defined(HAVE_CONFIG_H)
#include <config/shahcoin-config.h>
  #if defined(_WIN32)
    #if defined(HAVE_DLLEXPORT_ATTRIBUTE)
      #define EXPORT_SYMBOL __declspec(dllexport)
    #else
      #define EXPORT_SYMBOL
    #endif
  #elif defined(HAVE_DEFAULT_VISIBILITY_ATTRIBUTE)
    #define EXPORT_SYMBOL __attribute__ ((visibility ("default")))
  #endif
#elif defined(MSC_VER) && !defined(STATIC_LIBSHAHCOINCONSENSUS)
  #define EXPORT_SYMBOL __declspec(dllimport)
#endif

#ifndef EXPORT_SYMBOL
  #define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SHAHCOINCONSENSUS_API_VER 2

typedef enum shahcoinconsensus_error_t
{
    shahcoinconsensus_ERR_OK = 0,
    shahcoinconsensus_ERR_TX_INDEX,
    shahcoinconsensus_ERR_TX_SIZE_MISMATCH,
    shahcoinconsensus_ERR_TX_DESERIALIZE,
    shahcoinconsensus_ERR_AMOUNT_REQUIRED,
    shahcoinconsensus_ERR_INVALID_FLAGS,
    shahcoinconsensus_ERR_SPENT_OUTPUTS_REQUIRED,
    shahcoinconsensus_ERR_SPENT_OUTPUTS_MISMATCH
} shahcoinconsensus_error;

/** Script verification flags */
enum
{
    shahcoinconsensus_SCRIPT_FLAGS_VERIFY_NONE                = 0,
    shahcoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH                = (1U << 0), // evaluate P2SH (BIP16) subscripts
    shahcoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG              = (1U << 2), // enforce strict DER (BIP66) compliance
    shahcoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY           = (1U << 4), // enforce NULLDUMMY (BIP147)
    shahcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9), // enable CHECKLOCKTIMEVERIFY (BIP65)
    shahcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10), // enable CHECKSEQUENCEVERIFY (BIP112)
    shahcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS             = (1U << 11), // enable WITNESS (BIP141)
    shahcoinconsensus_SCRIPT_FLAGS_VERIFY_TAPROOT             = (1U << 17), // enable TAPROOT (BIPs 341 & 342)
    shahcoinconsensus_SCRIPT_FLAGS_VERIFY_ALL                 = shahcoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH | shahcoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG |
                                                               shahcoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY | shahcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY |
                                                               shahcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY | shahcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS |
                                                               shahcoinconsensus_SCRIPT_FLAGS_VERIFY_TAPROOT
};

typedef struct {
    const unsigned char *scriptPubKey;
    unsigned int scriptPubKeySize;
    int64_t value;
} UTXO;

/// Returns 1 if the input nIn of the serialized transaction pointed to by
/// txTo correctly spends the scriptPubKey pointed to by scriptPubKey under
/// the additional constraints specified by flags.
/// If not nullptr, err will contain an error/success code for the operation
EXPORT_SYMBOL int shahcoinconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                                 const unsigned char *txTo        , unsigned int txToLen,
                                                 unsigned int nIn, unsigned int flags, shahcoinconsensus_error* err);

EXPORT_SYMBOL int shahcoinconsensus_verify_script_with_amount(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, shahcoinconsensus_error* err);

EXPORT_SYMBOL int shahcoinconsensus_verify_script_with_spent_outputs(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    const UTXO *spentOutputs, unsigned int spentOutputsLen,
                                    unsigned int nIn, unsigned int flags, shahcoinconsensus_error* err);

EXPORT_SYMBOL unsigned int shahcoinconsensus_version();

#ifdef __cplusplus
} // extern "C"
#endif

#undef EXPORT_SYMBOL

#endif // SHAHCOIN_SCRIPT_SHAHCOINCONSENSUS_H
