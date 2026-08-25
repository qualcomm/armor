// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "stdint.h"

// ---------------------------------------------------------------------------
// 1. Plain typedef chain (unchanged in v2) — sanity baseline.
// ---------------------------------------------------------------------------
typedef int32_t   base_int_t;
typedef base_int_t mid_int_t;
typedef mid_int_t  top_int_t;

// ---------------------------------------------------------------------------
// 2. typedef chain in v1 -> using chain in v2, same resolved type each hop.
// ---------------------------------------------------------------------------
typedef uint32_t   handle_base_t;
typedef handle_base_t handle_mid_t;
typedef handle_mid_t  handle_t;

// ---------------------------------------------------------------------------
// 3. using chain in v1 -> typedef chain in v2, same resolved type each hop.
// ---------------------------------------------------------------------------
using token_base_t = uint16_t;
using token_mid_t  = token_base_t;
using token_t       = token_mid_t;

// ---------------------------------------------------------------------------
// 4. Mixed chain: typedef -> using -> typedef, same resolved type in v2
//    (order/kind of each hop flips, but the final canonical type is stable).
// ---------------------------------------------------------------------------
typedef int64_t     mixed_base_t;
using mixed_mid_t  = mixed_base_t;
typedef mixed_mid_t  mixed_top_t;

// ---------------------------------------------------------------------------
// 5. Function-pointer typedef chain -> using chain, same signature.
// ---------------------------------------------------------------------------
typedef void (*callback_base_fn)(int code, const char *msg);
typedef callback_base_fn callback_t;

// ---------------------------------------------------------------------------
// 6. Anonymous-enum-with-typedef-name -> named-enum-with-using-alias.
// ---------------------------------------------------------------------------
typedef enum
{
    STATUS_OK = 0,
    STATUS_FAIL,
    STATUS_PENDING
} StatusCode;

// ---------------------------------------------------------------------------
// 7. Chain that DOES change semantically at the far end (real incompatible
//    change) — the alias itself is untouched, but the base type widens.
// ---------------------------------------------------------------------------
typedef int16_t      size_base_t;
typedef size_base_t  size_mid_t;
typedef size_mid_t   size_t_alias;

// ---------------------------------------------------------------------------
// 8. Deep chain (5 hops) to exercise multi-level resolution both directions.
// ---------------------------------------------------------------------------
typedef uint8_t deep_l1_t;
typedef deep_l1_t deep_l2_t;
typedef deep_l2_t deep_l3_t;
typedef deep_l3_t deep_l4_t;
typedef deep_l4_t deep_l5_t;
