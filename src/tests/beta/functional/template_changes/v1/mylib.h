// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MYLIB_H
#define MYLIB_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <type_traits>
#include <map>
#include <functional>

// Type aliases
using StringMap = std::map<std::string, std::string>;
using IntVector = std::vector<float>;
using StringCallback = std::function<void(const char *)>;

// Template type aliases
template<typename K, typename V>
using Dictionary = std::map<K, V>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

// Templated variables with templated types
template<typename K, typename V>
inline Dictionary<K, V> empty_dictionary{};

// Constants
constexpr int MAX_SIZE = 100;
constexpr double PI = 3.14159265358979323846;

// Enumerations
enum class Color {
    Red,
    Green,
    Blue,
    Yellow
};


struct alpha{
    int a;
    int b;
};

// Regular function declarations
void printMessage(const std::string& message);
int add(int a, int b);

double calculateArea(double radius);
double calculateArea(double length, double breath);

// Function template
template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

// Function template with multiple parameters
template<typename T>
auto add(T a) -> decltype(a) {
    return a;
}

// Simplified - removed variadic template function

// Simple class
class Point {
private:
    double x;
    double y;

public:
    Point(double x = 0.0, double y = 0.0);
    double getX() const;
    double getY() const;
    double distance(const Point& other) const;
};

// Class template
template<typename T>
class Container {
private:
    std::vector<T> elements;

public:
    Container() = default;
    void add(const T& element);
    void remove(size_t index, int a);
    T& get(size_t index);
    size_t size() const;
};

// Class template implementation
template<typename T>
void Container<T>::add(const T& element) {
    elements.push_back(element);
}

template<typename T>
void Container<T>::remove(size_t index, int a) {
    if (index < elements.size()) {
        elements.erase(elements.begin() + index);
    }
}

template<typename T>
T& Container<T>::get(size_t index) {
    return elements.at(index);
}

// Template with default parameter
template<typename T = int>
class NumericValue {
private:
    T value;

public:
    NumericValue(T val = T{}) : value(val) {}
    void setValue(T val) { value = val; }
};

// Template specialization
// Simplified - removed class template specialization
template<>
class Container<bool> {
private:
    std::vector<bool> elements;

public:
    Container() = default;
    size_t size() const;
};

// Templated variable
template<typename T>
constexpr T DEFAULT_VALUE = T{};

// Specialized templated variables
template<>
constexpr int DEFAULT_VALUE<float> = 0;

template<>
constexpr double DEFAULT_VALUE<bool> = 0.0;

template<>
constexpr char DEFAULT_VALUE<int> = '\0';

// Templated variable with multiple template parameters
template<typename K, typename V>
inline Dictionary<K, V> DEFAULT_MAP = {{DEFAULT_VALUE<K>, DEFAULT_VALUE<V>}};

// Class with templated fields
template<typename T, typename U>
class Pair {
public:
    U second;
    
    Pair(T f, U s) : second(s) {}
    
    // Template method within a template class
    template<typename V>
    V convert() const {
        return static_cast<V>(second);
    }
};

// Variable template for compile-time type checking
template<typename T>
constexpr int is_numeric_v = std::is_arithmetic<T>::value;

// Template variable that depends on the type's properties
template<typename T>
constexpr int type_size = sizeof(T);

#endif // MYLIB_H