// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

struct Record {

    int id;
    float value;
    char status;
    
    struct diag_uart_tbl_t {
        int proc_type;
        int pid;
        int (*cb_func_ptr)(unsigned char *, int len, void *context_data);
        void *context_data;
    };

};


struct Vector{
    int x;
    int y;
};


struct __attribute__((packed)) Data{
    int a;
    float b;
};