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
    enum {
        MODE_OFF,
        MODE_ON,
        MODE_STANDBY
    } mode;

    enum {
        MODE_POWER_SAVING,
        MODE_PERFORMANCE,
    } mode2;

    float y;

    // Nested struct
    struct {
        int year;
        int month;
        int day;
    } manufactureDate;

    double x;

    struct test{
        int a;
        char b;
    };

} Device;

// Struct with nested enum and struct
typedef struct {
    char systemName[60];
    int deviceCount;

    struct {
        Device devices[5];

        enum {
            STATUS_OK,
            STATUS_FAIL
        } systemStatus;

        struct {
            int a;
            int b;

            struct beta{
                int a;
                int b;
            };

            enum{
                alpha,
                beta
            };

            struct{
                int x;
                double y;
            };

            struct{
                int a;
                int b;
                typedef struct{
                    int a;
                    int b;
                } omega;
            } gamma;

        } alpha;

        typedef Device myDevice;
        
        typedef struct{
            int a;
            float b;
        } gamma;

    } systemDetails;

    int x;

    struct {
        int ok;
    };

    int y;

    enum{
        alpha,
        beta
    };

} System;


enum{
    alpha,
    beta
};


struct{
    int a;
    float b;
} alpha_beta;

typedef Device i;

typedef i k;

int (*********** funcPtr        )(      float  , float   );

Device *****   *const*  **           y;

Device **          *const        **          a [30][10][200][20];

int ***const*** omega;

typedef Device******const******** mydefptr;

typedef int myInt;

typedef myInt asMyInt;