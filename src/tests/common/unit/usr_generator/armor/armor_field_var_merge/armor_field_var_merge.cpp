// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <memory>

template<typename T>
struct CustomTraits {
    static constexpr bool is_custom = false;
    static constexpr size_t size = sizeof(T);
    using promoted_type = T;
};

template<>
struct CustomTraits<char> {
    static constexpr bool is_custom = true;
    static constexpr size_t size = 1;
    using promoted_type = int;  // Char promotes to int
};

template<>
struct CustomTraits<float> {
    static constexpr bool is_custom = true;
    static constexpr size_t size = sizeof(float);
    using promoted_type = double;  // Float promotes to double
};


template<typename T>
struct StaticSpecialization {
    static T defaultValue;
    
    static T getDefault() {
        return defaultValue;
    }
};

static constexpr bool is_custom = true;
