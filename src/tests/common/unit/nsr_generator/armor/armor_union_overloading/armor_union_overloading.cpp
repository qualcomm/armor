// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <cstdint>

// Union with overloaded methods
union OverloadedUnion {
    int intValue;
    float floatValue;
    double doubleValue;
    
    // Unions can have constructors in C++11 and later
    OverloadedUnion() : intValue(0) {}
    
    // Overloaded methods to set values
    void setValue(int val) {
        intValue = val;
    }
    
    void setValue(float val) {
        floatValue = val;
    }
    
    void setValue(double val) {
        doubleValue = val;
    }
    
    // Overloaded methods to get values
    int getInt() const {
        return intValue;
    }
    
    float getFloat() const {
        return floatValue;
    }
    
    double getDouble() const {
        return doubleValue;
    }
};

// Template overloaded union with specializations
template<typename T>
union TemplateUnion {
    T value;
    int intRep;
    char buffer[sizeof(T)];
    
    TemplateUnion() : intRep(0) {}
    explicit TemplateUnion(T val) : value(val) {}
    
    T getValue() const { return value; }
    void setValue(T val) { value = val; }
    
    int getIntRep() const { return intRep; }
};

// Specialization for pointer types
template<typename T>
union TemplateUnion<T*> {
    T* value;
    uintptr_t ptrValue;
    void* voidPtr;
    
    TemplateUnion() : value(nullptr) {}
    explicit TemplateUnion(T* val) : value(val) {}
    
    T* getValue() const { return value; }
    void setValue(T* val) { value = val; }
    
    uintptr_t getAddress() const { return ptrValue; }
};

// Specialization for bool
template<>
union TemplateUnion<bool> {
    bool value;
    char charRep;
    
    TemplateUnion() : value(false) {}
    explicit TemplateUnion(bool val) : value(val) {}
    
    bool getValue() const { return value; }
    void setValue(bool val) { value = val; }
    
    char getCharRep() const { return charRep; }
};
