// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include <vector>

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

struct {
  int a;
} ***const****volatile*****&&alpha = nullptr;

struct alpha{
  int a;
  int b;
} const******volatile******alpha1[200][30];

typedef struct {
  int a;
  int b;
} *****const**********alpha2[100];


typedef struct alpha3 {
  int a;
  int b;
} ****const ******alpha4[120][10];


struct {
  int a;
} zeta;

struct zeta{
  int a;
  int b;
} zeta1;

struct a;

struct a;

struct a;

struct a;

struct a;

struct a;

struct a;

typedef alpha3 ok;


typedef alpha3 ok;


typedef alpha3 ok;


typedef alpha3 ok;


typedef alpha3 ok;


typedef alpha3 ok;


typedef alpha3 ok;

struct b{
  int a;
  int b;
};

// Struct with nested enum and struct
typedef struct {
    char systemName[64];
    int deviceCount;

    typedef struct {
        Device devices[10];

        typedef enum {
            STATUS_OK,
            STATUS_FAIL,
            STATUS_UNKNOWN
        } systemStatus;

    } systemDetails;

} ********const **volatile****System;

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


#define __CL_ANON_STRUCT__ __extension__
#define __CL_HAS_ANON_STRUCT__

typedef union
{
 __CL_ANON_STRUCT__ struct{ char x, y; };
 __CL_ANON_STRUCT__ struct{ char s0, s1; };
 __CL_ANON_STRUCT__ struct{ char lo, hi; };
}cl_uchar2;

typedef union
{
 __CL_ANON_STRUCT__ struct{ char x, y, z, w; };
 __CL_ANON_STRUCT__ struct{ char s0, s1, s2, s3; };
 __CL_ANON_STRUCT__ struct{ char lo, hi; };
}cl_uchar4;
