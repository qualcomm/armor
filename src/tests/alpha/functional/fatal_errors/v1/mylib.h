// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include "types.h"
#include "comm_def.h"

// Simple struct using typedefs from types.h
struct Point {
    int x;
    int y;
};

// Struct with different types using typedefs
struct Person {
    string_t name;  // Using typedef from types.h
    uint_t age;     // Using typedef from types.h
    float height;
};

// Simple functions using basic typedefs
int add(int a, int b);

float multiply(float x, float y);

void print_point(struct Point p);

struct Person create_person(string_t name, uint_t age, float height);

// Functions using typedef types
status_t process_data(byte_t* data, uint_t size);

record_t get_record(uint_t id);

void execute_callback(callback_t cb);

float apply_math(math_func_t func, float value);

// Function using function pointer typedef
int perform_operation(operation_t op, int x, int y);
