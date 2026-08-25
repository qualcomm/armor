// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
// test_template_specializations.cpp
#include <string>
#include <iostream>

// Primary template
template<typename T>
class Container {
public:
    T data;
    
    Container(T val);
    
    void print();
    
    T getValue() const;
};

template<>
class Container<int> {
public:
    int data;
    
    Container(int val);
    
    void print();
    
    int getValue() const;
    
    bool isEven() const;
};

// Explicit specialization for std::string
template<>
class Container<std::string> {
public:
    std::string data;
    
    Container(std::string val);
    
    void print();
    
    std::string getValue() const;
    
    int length() const;
};

// Partial specialization for pointer types
template<typename T>
class Container<T*> {
public:
    T* data;
    
    Container(T* val);
    
    void print();
    
    T* getValue() const;
    
    T& dereference() const;
};


