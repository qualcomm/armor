// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#ifndef LIB_H
#define LIB_H

// Conditional compilation based on compiler flags
#ifdef ENABLE_ADVANCED_FEATURES

// Basic data types with macro-controlled features
#ifdef ENABLE_EXTENDED_TYPES
typedef long long extended_int_t;
typedef long double extended_float_t;
#else
typedef int extended_int_t;
typedef double extended_float_t;
#endif

// Version-dependent structures
#if defined(VERSION_2) || defined(VERSION_3)
struct BaseConfig {
    int id;
    char name[32];
    #ifdef VERSION_3
    extended_float_t precision;
    bool is_optimized;
    #endif
};
#endif

// Feature-specific enums
#ifdef ENABLE_LOGGING
enum LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
};
#endif

// Platform-specific definitions
#ifdef PLATFORM_EMBEDDED
    #define MAX_BUFFER_SIZE 256
    #define MAX_CONNECTIONS 4
    typedef unsigned char buffer_size_t;
#else
    #define MAX_BUFFER_SIZE 1024
    #define MAX_CONNECTIONS 16
    typedef unsigned short buffer_size_t;
#endif

// Debug macros
#ifdef DEBUG_MODE
    #include <stdio.h>
    #define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)
#endif

// Network-related structures
#ifdef ENABLE_NETWORKING
struct NetworkConfig {
    char host[64];
    int port;
    buffer_size_t timeout;
    
    #ifdef ENABLE_SSL
    bool use_ssl;
    #endif
};

enum ConnectionType {
    CONN_TCP = 1,
    CONN_UDP = 2
};
#endif

// Mathematical utilities
#ifdef ENABLE_MATH_UTILS
union MathValue {
    extended_int_t integer;
    extended_float_t floating;
};

struct MathOperation {
    enum {
        OP_ADD = 1,
        OP_SUB = 2,
        OP_MUL = 3,
        OP_DIV = 4
    } operation;
    
    MathValue operand1;
    MathValue operand2;
    MathValue result;
};
#endif

// Utility macros for different build configurations
#ifdef RELEASE_BUILD
    #define INLINE inline
#else
    #define INLINE
#endif

// Legacy support (can be undefined with -ULEGACY_MODE)
#ifdef LEGACY_MODE
    #define LEGACY_BUFFER_SIZE 128
    #define LEGACY_API_VERSION 1
    typedef char legacy_char_t;
    
    // Legacy-specific structures
    struct LegacyConfig {
        int legacy_id;
        legacy_char_t legacy_name[16];
    };
#endif

// Old API support (can be undefined with -UOLD_API_VERSION)
#ifdef OLD_API_VERSION
    #define OLD_HANDLE_COUNT 8
    typedef int old_handle_t;
    
    struct OldApiData {
        old_handle_t handle;
        int version;
    };
#endif

// Experimental features (can be undefined with -UEXPERIMENTAL_FEATURES)
#ifdef EXPERIMENTAL_FEATURES
    #define EXPERIMENTAL_BUFFER_SIZE 2048
    typedef double experimental_type_t;
    
    struct ExperimentalConfig {
        experimental_type_t value;
        bool is_experimental;
    };
#endif

// Beta functions (can be undefined with -UBETA_FUNCTIONS)
#ifdef BETA_FUNCTIONS
    #define BETA_VERSION 2
    typedef float beta_type_t;
    
    struct BetaData {
        beta_type_t beta_value;
        char beta_name[32];
    };
#endif

// Deprecated features (can be undefined with -UDEPRECATED_SUPPORT)
#ifdef DEPRECATED_SUPPORT
    #define DEPRECATED_BUFFER_SIZE 64
    typedef short deprecated_int_t;
    
    struct DeprecatedStruct {
        deprecated_int_t old_value;
        char old_name[8];
    };
#endif

#endif // ENABLE_ADVANCED_FEATURES

#endif // LIB_H