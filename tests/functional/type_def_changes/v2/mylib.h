// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
// Normal enum
typedef enum {
    POWER_LOW,
    POWER_MEDIUM,
    POWER_HIGH
} PowerLevel;

// Normal struct
typedef struct {
    int voltage;
    PowerLevel level;
} PowerConfig;

// Struct with nested types
typedef struct {
    int id;
    char label[32];

    // Nested enum
    typedef enum {
        MODE_OFF,
        MODE_ON,
        MODE_STANDBY
    } mode;

    // Nested struct
    typedef struct {
        int year;
        int month;
        int day;
    } manufactureDate;

} Device;

// Struct with nested enum and struct
typedef struct {
    char systemName[63];
    int deviceCount;

    typedef struct {
        Device devices[11];

        typedef enum {
            STATUS_OK,
            STATUS_FAIL
        } systemStatus;

    } systemDetails;

} System;