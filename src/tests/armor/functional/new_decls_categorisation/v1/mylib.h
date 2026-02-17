// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
struct alpha{
    int a;
    alpha(){
        a = 0;
    }
    alpha(int x, int y){
        a = x;
        a = y;
    }
};

alpha a;

alpha b(1,2);

// String array with initializers
const char* string_array[5] = {
    "first",
    "second",
    "third"    ,
    "fourth",
    nullptr
};
