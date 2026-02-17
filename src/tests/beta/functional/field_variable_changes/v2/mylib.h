// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <stdint.h>

struct Device {
    int id;
    int max_connections;
    int status;
    int cache_value;
    int control_register;
    float temperature;
    char name[32];
    uint8_t* buffer;
    const char* const firmware_version;
    static const int VERSION = 1;
    int MAX_BUFFER_SIZE = 1024;
    float DEFAULT_TEMP = 25.0f;
};

inline constexpr int INLINE_CONSTANT = 200;
static constexpr double PI = 3.14159265359;
thread_local int thread_counter;
int inline_global = 42;
int inline_const_global = 99;
int array_size = 10;
