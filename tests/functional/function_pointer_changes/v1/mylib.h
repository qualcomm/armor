// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

typedef int(******const****cb_func_ptr)(unsigned char ********a[100][20], int len, void ********&&context_data, int (******************volatile************const*&&inner_ptr)(int *********&&a, int ***** arr[10]));

typedef cb_func_ptr ok;

cb_func_ptr fun();


typedef int (*fun_ptr)();

fun_ptr not_fun();

int(***         *volatile         ****           *const *fun_ptr_ok)(int                   a, int b);


typedef struct{
    cb_func_ptr ok;
    typedef int(**const**cb_func_ptr)(unsigned char **a, int len, void **&&context_data, int (**volatile***const*&&inner_ptr)(int **&&a, int **arr[10]));
} alpha;