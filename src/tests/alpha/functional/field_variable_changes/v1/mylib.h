// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <stdint.h>

struct Device {
    int id;
    const int max_connections;
    volatile int status;
    mutable int cache_value;
    const volatile int control_register;
    float temperature;
    char name[32];
    uint8_t* buffer;
    const char* const firmware_version;
    static const int VERSION = 1;
    static constexpr int MAX_BUFFER_SIZE = 1024;
    static constexpr float DEFAULT_TEMP = 25.0f;
};

inline constexpr int INLINE_CONSTANT = 200;
static constexpr double PI = 3.14159265359;
extern thread_local int thread_counter;
inline int inline_global = 42;
inline const int inline_const_global = 99;
constexpr int array_size = 10;
