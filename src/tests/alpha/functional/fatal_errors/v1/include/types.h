// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include "utils.h"

// Basic type aliases
typedef unsigned char byte_t;
typedef unsigned int uint_t;
typedef unsigned long ulong_t;

// Pointer types
typedef int* int_ptr_t;
typedef char* string_t;
typedef void* generic_ptr_t;

// Function pointer types
typedef int (*operation_t)(int, int);
typedef void (*callback_t)(void);
typedef float (*math_func_t)(float);

// Array types
typedef int int_array_t[10];
typedef char buffer_t[256];

// Struct typedef
typedef struct {
    int id;
    char name[32];
} record_t;

// Enum typedef
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_PENDING = 2
} status_t;