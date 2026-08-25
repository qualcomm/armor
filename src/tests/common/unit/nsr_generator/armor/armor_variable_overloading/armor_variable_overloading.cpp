// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <string>

// Variable templates (C++14) with overloading
template<typename T>
constexpr static T pi = T(3.1415926535897932385);

template<>
constexpr static float pi<float> = 3.14159f;

template<>
constexpr static int pi<int> = 3;

// Overloaded variable templates with different types
template<typename T>
constexpr static bool is_small = sizeof(T) <= 4;

template<typename T>
constexpr static const char* type_name = "unknown";

template<>
constexpr static const char* type_name<int> = "int";

template<>
constexpr static const char* type_name<double> = "double";

template<>
constexpr static const char* type_name<std::string> = "string";

// Type traits with overloaded values
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

// Template struct with static member specialization
template<typename T>
struct StaticSpecialization {
    static T defaultValue;
    
    static T getDefault() {
        return defaultValue;
    }
};

// Overloaded type aliases and typedefs
template<typename T>
struct NestedTypes {
    struct Inner {
        T value;
    };
    
    using InnerRef = Inner&;
    using ValueType = T;
};

template<>
struct NestedTypes<bool> {
    struct Inner {
        bool value;
        size_t count;  // Additional field for bool specialization
    };
    
    using InnerRef = const Inner&;  // Different reference type
    using ValueType = int;  // Different value type
};

typedef struct DALSYSPropertyVar DALSYSPropertyVar;

struct DALSYSPropertyVar
{
  int dwType;
  int dwLen;
  union
  {
    int *pbVal;
    char *pszVal;
    int dwVal;
    int *pdwVal;
    const void *pStruct;
    int qwVal;      /* ADDED: 64-bit value */
    float fVal;       /* ADDED: Float value */
  }Val;
  int reserved[2];     /* ADDED: Reserved array */
};
