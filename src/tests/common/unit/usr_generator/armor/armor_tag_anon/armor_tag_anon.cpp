// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

struct {
} alpha;

struct alpha2{
    int x;
} alpha2;

struct alpah3{
    int x;
} alpha4;

enum {
    x,
} alpha_enum;

enum alpha_eum2{
    y,
} alpha_enum2;

enum alpha_eum3{
    z,
} alpha_enum4;


struct armor{

    struct{
        int x;
        struct {
            int y;
        };
    };

    enum{
        x1,
    };

    enum{
        y1,
    };

};


struct { int x; } *const*volatile*&& field = nullptr;

struct {int y;} *** gamma[30];

struct SomeClass;
struct { int x; } SomeClass::* class_member_ptr;

enum { I } *****const**volatile** beta = nullptr;

struct {
  int a;
} zeta;

struct zeta{
  int a;
  int b;
} alpha1;

typedef struct {
  int a;
  int b;
} *****const**********alpha6[100];

alpha6 ok;

typedef enum {
  a,
  b
} *****const**********alpha7[100];
