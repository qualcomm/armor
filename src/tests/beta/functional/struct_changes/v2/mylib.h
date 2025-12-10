// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

// - Removed status field
// - Added timestamp field
// - Changed order of fields
// - Changed type of id from int to long
// - Removed int diag_uart_tbl_t
struct Record{

    float value;         // moved
    long id;             // changed type
    double timestamp;    // added
    
    struct diag_uart_tbl_t {
        int proc_type;
        float (*cb_func_ptr)(unsigned char *, int len, void *context_data);
        void *context_data;
    };

};

// - Changed Order
struct Vector{
    int y;
    int x;
};


//Changed Packing
struct Data{
    int a;
    float b;
};