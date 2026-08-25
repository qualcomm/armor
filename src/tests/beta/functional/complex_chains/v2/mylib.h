// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "stdint.h"

// ---------------------------------------------------------------------------
// 1. Plain typedef chain (unchanged) — sanity baseline.
// ---------------------------------------------------------------------------
typedef int32_t   base_int_t;
typedef base_int_t mid_int_t;
typedef mid_int_t  top_int_t;

// ---------------------------------------------------------------------------
// 2. typedef chain in v1 -> using chain in v2, same resolved type each hop.
// ---------------------------------------------------------------------------
using handle_base_t = uint32_t;
using handle_mid_t  = handle_base_t;
using handle_t        = handle_mid_t;

// ---------------------------------------------------------------------------
// 3. using chain in v1 -> typedef chain in v2, same resolved type each hop.
// ---------------------------------------------------------------------------
typedef uint16_t    token_base_t;
typedef token_base_t token_mid_t;
typedef token_mid_t   token_t;

// ---------------------------------------------------------------------------
// 4. Mixed chain: typedef -> using -> typedef in v1 becomes
//    using -> typedef -> using in v2, same resolved type throughout.
// ---------------------------------------------------------------------------
using mixed_base_t = int64_t;
typedef mixed_base_t mixed_mid_t;
using mixed_top_t   = mixed_mid_t;

// ---------------------------------------------------------------------------
// 5. Function-pointer typedef chain -> using chain, same signature.
// ---------------------------------------------------------------------------
using callback_base_fn = void (*)(int code, const char *msg);
using callback_t         = callback_base_fn;

// ---------------------------------------------------------------------------
// 6. Anonymous-enum-with-typedef-name -> named-enum-with-using-alias.
// ---------------------------------------------------------------------------
enum StatusCodeEnum
{
    STATUS_OK = 0,
    STATUS_FAIL,
    STATUS_PENDING
};
using StatusCode = StatusCodeEnum;

// ---------------------------------------------------------------------------
// 7. Chain that DOES change semantically at the far end (real incompatible
//    change) — the alias name is untouched, but the base type widens from
//    int16_t to int32_t, so size_t_alias's canonical type must differ.
// ---------------------------------------------------------------------------
typedef int32_t      size_base_t;
typedef size_base_t  size_mid_t;
typedef size_mid_t    size_t_alias;

// ---------------------------------------------------------------------------
// 8. Deep chain (5 hops), typedef chain flipped to using chain throughout.
// ---------------------------------------------------------------------------
using deep_l1_t = uint8_t;
using deep_l2_t = deep_l1_t;
using deep_l3_t = deep_l2_t;
using deep_l4_t = deep_l3_t;
using deep_l5_t = deep_l4_t;
