// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

union Data {
    int i;
    float f;
    char label[10];

    union nested_union {
        int code;
        double value;
    };
};


union Alpha{
    int x;
    int y;
    union{
        int a;
        int b;
    };
};