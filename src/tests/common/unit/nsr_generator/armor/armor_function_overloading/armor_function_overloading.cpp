// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <string>
#include <vector>

// Basic function overloading with different parameter types
void print(int value);
void print(double value);
void print(const std::string& value);

// Function overloading with different number of parameters
void calculate(int a);
void calculate(int a, int b);
void calculate(int a, int b, int c);

// Function template overloading
template<typename T>
T add(T a, T b);

// Template specialization
template<>
std::string add<std::string>(std::string a, std::string b);

// Overloading with template and non-template functions
void process(int value);

template<typename T>
void process(T value);

// Template function with friend function templates
template<typename T>
struct WithFriendTemplates;

template<typename U>
WithFriendTemplates<U> transform(const WithFriendTemplates<U>& obj, U (*func)(U));

// Specialization for int
template<>
WithFriendTemplates<int> transform<int>(const WithFriendTemplates<int>& obj, int (*func)(int));
