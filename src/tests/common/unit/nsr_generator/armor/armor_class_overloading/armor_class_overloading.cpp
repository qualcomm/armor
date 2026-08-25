// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>

// Basic class with template methods
class BasicContainer {
public:
    // Template method overloading with different types
    template<typename T>
    void store(T value);

    // Template specialization for int
    template<>
    void store(int value);
    
    // Template specialization for std::string
    template<>
    void store(std::string value);
    
    // Non-template overload
    void store(double value);

};

// Template class with overloaded methods
template<typename T>
class DataContainer {
public:
    DataContainer() = default;
    explicit DataContainer(T initialValue) : data(initialValue) {}
    
    // Const and non-const method overloading
    const T& getData() const;
    
    T& getData();
    
    // Method overloading with different parameter counts
    void calculate(int a);
    
    void calculate(int a, int b);
    
private:
    T data;
    
    // Private overload
    void calculate(int a, int b, int c);

};

// Template specialization for std::string
template<>
class DataContainer<std::string> {
public:
    DataContainer() = default;
    explicit DataContainer(const std::string& initialValue) : data(initialValue) {}
    
    // Different return types for specialized class
    const std::string& getData() const;
    
    std::string& getData();
    
    // Additional methods specific to string specialization
    size_t getLength() const;
    
    bool isEmpty() const;
    
private:
    std::string data;
};

// Template class with partial specialization for pointers
template<typename T>
class SmartContainer {
public:
    SmartContainer() : data() {}
    explicit SmartContainer(const T& value) : data(value) {}
    
    void process();
    
    T& getValue() { return data; }
    const T& getValue() const { return data; }
    
private:
    T data;
};

// Partial specialization for pointer types
template<typename T>
class SmartContainer<T*> {
public:
    SmartContainer() : data(nullptr), ownsPointer(false) {}
    
    // Constructor overloading
    explicit SmartContainer(T* ptr) : data(ptr), ownsPointer(false) {}
    SmartContainer(T* ptr, bool takeOwnership) : data(ptr), ownsPointer(takeOwnership) {}
    
    ~SmartContainer() {
        if (ownsPointer && data) {
            delete data;
        }
    }
    
    void process();
    
    T* getValue() { return data; }
    const T* getValue() const { return data; }
    
private:
    T* data;
    bool ownsPointer;
};

// Template class with template methods
template<typename T, typename U = T>
class MultiTypeContainer {
public:
    MultiTypeContainer() = default;
    
    // Template method with default template parameter
    template<typename V = T>
    void process(V value);
    // Template method with explicit template parameter
    template<typename V>
    void convert(V value);
    // Specialization for int within class template
    template<>
    void process<int>(int value);
};

// CRTP (Curiously Recurring Template Pattern) example
template<typename Derived>
class Base {
public:
    void implementation();
};

class Derived1 : public Base<Derived1> {
public:
    // Override the implementation
    void implementation();
};

class Derived2 : public Base<Derived2> {
public:
    // This one uses the base implementation
};