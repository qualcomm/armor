// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#ifndef MYLIB_H
#define MYLIB_H

#include "lib.h"

// Only compile this library if advanced features are enabled
#ifdef ENABLE_ADVANCED_FEATURES

// Basic structures using lib.h types
#if defined(VERSION_2) || defined(VERSION_3)
struct ApplicationConfig : public BaseConfig {
    extended_int_t max_users;
    extended_float_t performance_threshold;
    
    #ifdef ENABLE_LOGGING
    LogLevel default_log_level;
    #endif
};

// Declare without initialization to avoid inheritance initialization issues
struct ApplicationConfig app_config;
#endif

// Data processing structures
#ifdef ENABLE_DATA_PROCESSING
// new comments
struct DataBuffer {
    char data[MAX_BUFFER_SIZE];
    
    
    // new comments
    buffer_size_t size;
    buffer_size_t capacity;
    bool is_compressed;
    
    #ifdef DEBUG_MODE
    extended_int_t access_count;
    char debug_info[64];
    #endif
};

// Array of buffers
// Declare without initialization to avoid buffer_size_t narrowing issues
struct DataBuffer buffers[MAX_CONNECTIONS];
#endif

// Mathematical processing
#ifdef ENABLE_MATH_UTILS
struct Calculator {
    MathOperation operations[5];
    int operation_count;
    extended_float_t precision;
} calculator_instance = {
    .operation_count = 0,
    .precision = 0.0001
};

// Function using conditional compilation
INLINE extended_float_t calculate(extended_int_t a, extended_int_t b) {
    DEBUG_PRINT("Calculating with values: %lld, %lld", a, b);
    return (extended_float_t)(a + b);
}
#endif

// Network communication structures
#ifdef ENABLE_NETWORKING
struct Connection {
    int socket_fd;
    ConnectionType type;
    NetworkConfig config;
    bool is_active;
    
    #ifdef ENABLE_SSL
    bool ssl_enabled;
    #endif
};

// Connection pool
struct Connection connections[MAX_CONNECTIONS];

void initialize_connections() {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        connections[i] = (struct Connection){
            .socket_fd = -1,
            .type = CONN_TCP,
            .config = {
                .host = "localhost",
                .port = 8080,
                .timeout = 30,
                #ifdef ENABLE_SSL
                .use_ssl = false,
                #endif
            },
            .is_active = false,
            #ifdef ENABLE_SSL
            .ssl_enabled = false,
            #endif
        };
    }
}
#endif

// Legacy support structures (can be undefined with -ULEGACY_MODE)
#if defined(LEGACY_MODE) && defined(LEGACY_API_VERSION) && defined(LEGACY_BUFFER_SIZE)
struct LegacyBuffer {
    legacy_char_t data[LEGACY_BUFFER_SIZE];
    int size;
    int legacy_version;
} legacy_buffer = {
    .data = "legacy",
    .size = 6,
    .legacy_version = LEGACY_API_VERSION
};

struct LegacyConfig legacy_configs[4] = {
    {1, "config1"},
    {2, "config2"},
    {3, "config3"}
};
#endif

// Old API support (can be undefined with -UOLD_API_VERSION)
#ifdef OLD_API_VERSION
struct OldHandle {
    old_handle_t handle;
    char name[16];
    int api_version;
} old_handles[OLD_HANDLE_COUNT] = {
    {1, "handle1", 1},
    {2, "handle2", 1},
    {3, "handle3", 1}
};

struct OldApiData old_api_instances[3] = {
    {100, 1},
    {200, 1},
    {300, 1}
};
#endif

// Experimental features (can be undefined with -UEXPERIMENTAL_FEATURES)
#ifdef EXPERIMENTAL_FEATURES
struct ExperimentalData {
    experimental_type_t value;
    bool is_experimental;
    char experiment_name[32];
} experimental_instances[2] = {
    {3.14159, true, "pi_experiment"},
    {2.71828, true, "e_experiment"}
};

struct ExperimentalConfig experimental_config = {
    .value = 1.41421,
    .is_experimental = true
};
#endif

// Beta functions support (can be undefined with -UBETA_FUNCTIONS)
#ifdef BETA_FUNCTIONS
struct BetaConfig {
    beta_type_t beta_value;
    char beta_name[32];
    int beta_version;
} beta_configs[3] = {
    {1.0f, "beta_test_1", BETA_VERSION},
    {2.0f, "beta_test_2", BETA_VERSION},
    {3.0f, "beta_test_3", BETA_VERSION}
};

struct BetaData beta_data_instance = {
    .beta_value = 42.0f,
    .beta_name = "main_beta"
};
#endif

// Deprecated support (can be undefined with -UDEPRECATED_SUPPORT)
#ifdef DEPRECATED_SUPPORT
struct DeprecatedBuffer {
    char old_data[DEPRECATED_BUFFER_SIZE];
    deprecated_int_t old_size;
} deprecated_buffers[2] = {
    {"old_data_1", 10},
    {"old_data_2", 10}
};

struct DeprecatedStruct deprecated_instance = {
    .old_value = 999,
    .old_name = "old"
};
#endif

// Function declarations
#ifdef ENABLE_API_FUNCTIONS
int initialize_system(void);
void shutdown_system(void);

#ifdef ENABLE_DATA_PROCESSING
int process_data(struct DataBuffer* buffer);
#endif

#ifdef ENABLE_NETWORKING
int create_connection(const char* host, int port);
void close_connection(int connection_id);
#endif

#ifdef ENABLE_LOGGING
void log_message(LogLevel level, const char* message);
#endif

// Legacy API functions (can be undefined with -ULEGACY_MODE)
#if defined(LEGACY_MODE) && defined(LEGACY_API_VERSION) && defined(LEGACY_BUFFER_SIZE)
int legacy_init(int legacy_id);
void legacy_cleanup(void);
struct LegacyBuffer* get_legacy_buffer(void);
#endif

// Old API functions (can be undefined with -UOLD_API_VERSION)
#ifdef OLD_API_VERSION
old_handle_t old_api_create_handle(void);
void old_api_destroy_handle(old_handle_t handle);
int old_api_process(struct OldApiData* data);
#endif

// Experimental API functions (can be undefined with -UEXPERIMENTAL_FEATURES)
#ifdef EXPERIMENTAL_FEATURES
experimental_type_t experimental_calculate(experimental_type_t input);
void experimental_process(struct ExperimentalData* data);
#endif

// Beta API functions (can be undefined with -UBETA_FUNCTIONS)
#ifdef BETA_FUNCTIONS
void beta_function(void);
int beta_process(struct BetaConfig* config);
beta_type_t beta_calculate(beta_type_t input);
#endif

// Deprecated API functions (can be undefined with -UDEPRECATED_SUPPORT)
#ifdef DEPRECATED_SUPPORT

// new comment
int deprecated_function(deprecated_int_t value);
void deprecated_cleanup(struct DeprecatedStruct* data);
#endif

#endif // ENABLE_API_FUNCTIONS

#else
// Fallback when advanced features are disabled
#warning "Advanced features are disabled. Limited functionality available."

struct SimpleConfig {
    int id;
    char name[32];
} simple_config = {1, "BasicApp"};

int basic_function(int a, int b) {
    return a + b;
}

#endif // ENABLE_ADVANCED_FEATURES

#endif // MYLIB_H