// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#define SQUARE(x) ((x) * (x))

int diag_lsm_comm_open(void);

int diag_lsm_open(void);

int diag_lsm_comm_write(int fd, unsigned char *buf, int bytes, int flags);

int diag_lsm_comm_ioctl(int fd, unsigned long request, void *buf, unsigned int len);

double diag_lsm_ioctl(int fd, unsigned long request, void *buf);

int extern sum(int a, int b);

int inline multiply(int a, int b);

int static divide(int a, int b);

// Default call convetion is 
int __stdcall call_add(int a, int b);

float __fastcall dot_product(float a, float b);

int __attribute__((swiftcall)) swift_func(int x);

void diag_send_data(unsigned char *, int, unsigned int *buf[10]);

int**** diag_get_data(int, int, unsigned int *buf[10]);