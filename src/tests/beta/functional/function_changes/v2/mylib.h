// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

// Replacing macros with functio
int SQUARE(int x);

//#same as original
int diag_lsm_open(void);

//#param type change
int diag_lsm_comm_write(float fd, unsigned char *buf[10], int bytes, int flags);

//return type change
double diag_lsm_comm_ioctl(int fd, unsigned long a, void *buf, unsigned int len);

// #param removal 
double diag_lsm_ioctl(int fd, unsigned long request);

// adding new function (pass)
int diag_lsm_comm_ioctl_v2(int fd, unsigned long request, void *buf, unsigned int len);

// removed return type modifier
int sum(int a, int b);

// modified return type modifier
int static multiply(int a, int b);

// modified return type modifier
int inline divide(int a, int b);

int __attribute__((fastcall)) call_add(int a, int b);

float __stdcall dot_product(float a, float b);

int __vectorcall swift_func(int x);


void diag_send_data(unsigned char *, int, int, unsigned int *buf[10]);

int****** diag_get_data(unsigned char *, int, int, unsigned int *buf[10]);