// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

struct FieldOwner {
    struct NeverDefined* fptr;
};

struct Outer {
    struct Middle {
        struct Inner {
            struct DeepNeverDefined* fptr;
        };
    };
};

struct RetOwner;
struct RetOwner* retFn();
struct RetOwner {
    int val;
};

struct alpha{
    int a;
    int b;
};

struct beta {

    struct alpha;

    struct alpha{
        int a;
        int b;
    };

    struct alpha* a;
    struct alpha* b;

};

struct beta2{

    struct alpha1;
    struct alpha1* a;
    struct alpha1* b;

};

struct beta2::alpha1{
    int a;
    int b;
};

struct beta3{
    struct alpha3* a;
    struct alpha3* b;
};

struct alpha4{
    int a;
    int b;
};

struct beta4{
    struct alpha4* a;
    struct alpha4* b;
};

struct alpha5{
    struct alpha6;
};

struct beta5{
    struct alpha5::alpha6* a;
};

struct beta6{
    struct beta7{
        struct beta8{
            struct alpha7* a;
            struct alpha7* b;
            struct alpha8;
        };
    };
};

struct alpha7;

struct alpha3{
    int a;
    int b;
};
