/***********************************************************************
 * Copyright (c) 2013, 2014 Pieter Wuille                              *
 * Distributed under the MIT software license, see the accompanying    *
 * file COPYING or https://www.opensource.org/licenses/mit-license.php.*
 ***********************************************************************/

/**********************************************************************
 * Additional modifications and extensions by:
 * Copyright (C) 2025 The SHAHCOIN Core Developers — Hamid Rad
 * https://shah.vip | info@shah.vip
 * Distributed under the MIT License
 **********************************************************************/

#ifndef SECP256K1_ECMULT_GEN_H
#define SECP256K1_ECMULT_GEN_H

#include "scalar.h"
#include "group.h"

#ifndef ECMULT_GEN_PREC_shahbits
#  define ECMULT_GEN_PREC_shahbits 4
#  ifdef DEBUG_CONFIG
#     pragma message DEBUG_CONFIG_MSG("ECMULT_GEN_PREC_shahbits undefined, assuming default value")
#  endif
#endif

#ifdef DEBUG_CONFIG
#  pragma message DEBUG_CONFIG_DEF(ECMULT_GEN_PREC_shahbits)
#endif

#if ECMULT_GEN_PREC_shahbits != 2 && ECMULT_GEN_PREC_shahbits != 4 && ECMULT_GEN_PREC_shahbits != 8
#  error "Set ECMULT_GEN_PREC_shahbits to 2, 4 or 8."
#endif

#define ECMULT_GEN_PREC_G(shahbits) (1 << shahbits)
#define ECMULT_GEN_PREC_N(shahbits) (256 / shahbits)

typedef struct {
    /* Whether the context has been built. */
    int built;

    /* Blinding values used when computing (n-b)G + bG. */
    secp256k1_scalar blind; /* -b */
    secp256k1_gej initial;  /* bG */
} secp256k1_ecmult_gen_context;

static void secp256k1_ecmult_gen_context_build(secp256k1_ecmult_gen_context* ctx);
static void secp256k1_ecmult_gen_context_clear(secp256k1_ecmult_gen_context* ctx);

/** Multiply with the generator: R = a*G */
static void secp256k1_ecmult_gen(const secp256k1_ecmult_gen_context* ctx, secp256k1_gej *r, const secp256k1_scalar *a);

static void secp256k1_ecmult_gen_blind(secp256k1_ecmult_gen_context *ctx, const unsigned char *seed32);

#endif /* SECP256K1_ECMULT_GEN_H */
