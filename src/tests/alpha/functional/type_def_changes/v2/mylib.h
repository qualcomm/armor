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

typedef struct telemetry telemetry;

struct ok{
    int a;
    int b;
};

typedef ok alias_ok;


typedef struct _qurt_thread_attr {
    unsigned short priority;
    unsigned char  autostack:1;
    unsigned char  group_id:6;
    unsigned short timetest_id;
    unsigned int   stack_size;
    void *stack_addr;
    unsigned short detach_state;
} qurt_thread_attr_t;


DIAGPKT_REQ_DEFINE(DIAG_PUT_PERM_PROPERTY_F)
  diag_guid_type guid;
  int           name[0];
  int           size;
DIAGPKT_REQ_END


typedef struct DALSYSPropertyVar _DALSYSPropertyVar;

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
  }***********Val[20];
  int reserved[2];     /* ADDED: Reserved array */
};


struct alpha{
    int a;
    int b;
} ***********alpha2[20];


struct {
    int a;
    int b;
} *************beta[30];


typedef struct zeta{
    int b;
} ******const*** _zeta[20];
