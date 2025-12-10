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
using IntVector = std::vector<int>;
using StringCallback = std::function<void(const std::string&)>;

// Template type aliases
template<typename T>
using Vector = std::vector<T>;

template<typename K, typename V>
using Dictionary = std::map<K, V>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

// Templated variables with templated types
template<typename T>
inline Vector<T> empty_vector{};

template<typename K, typename V>
inline Dictionary<K, V> empty_dictionary{};

template<typename T, template<typename> class Container = Vector>
inline Container<T> empty_container{};

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

// Regular function declarations
void printMessage(const char* message);

// Function template
template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

// Function template with multiple parameters
template<typename T, typename U>
auto add(T a, U b) -> decltype(a + b) {
    return a + b;
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
    void setX(double x);
    void setY(double y);
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
    void remove(size_t index);
    T& get(size_t index);
    size_t size() const;
    bool isEmpty() const;
};

// Class template implementation
template<typename T>
void Container<T>::add(const T& element) {
    elements.push_back(element);
}

template<typename T>
void Container<T>::remove(size_t index) {
    if (index < elements.size()) {
        elements.erase(elements.begin() + index);
    }
}

template<typename T>
T& Container<T>::get(size_t index) {
    return elements.at(index);
}

template<typename T>
size_t Container<T>::size() const {
    return elements.size();
}

template<typename T>
bool Container<T>::isEmpty() const {
    return elements.empty();
}

// Template with default parameter
template<typename T = int>
class NumericValue {
private:
    T value;

public:
    NumericValue(T val = T{}) : value(val) {}
    T getValue() const { return value; }
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
    void add(bool element);
    bool get(size_t index) const;
    size_t size() const;
};

// Templated variable
template<typename T>
constexpr T DEFAULT_VALUE = T{};

// Specialized templated variables
template<>
constexpr int DEFAULT_VALUE<int> = 0;

template<>
constexpr double DEFAULT_VALUE<double> = 0.0;

template<>
constexpr char DEFAULT_VALUE<char> = '\0';

int add(int a, int b);

// Templated variable with templated type
template<typename T>
inline Vector<T> DEFAULT_COLLECTION = Vector<T>(3, DEFAULT_VALUE<T>);

// Templated variable with multiple template parameters
template<typename K, typename V>
inline Dictionary<K, V> DEFAULT_MAP = {{DEFAULT_VALUE<K>, DEFAULT_VALUE<V>}};

// Class with templated fields
template<typename T, typename U>
class Pair {
public:
    T first;
    U second;
    
    Pair() : first(DEFAULT_VALUE<T>), second(DEFAULT_VALUE<U>) {}
    Pair(T f, U s) : first(f), second(s) {}
    
    // Template method within a template class
    template<typename V>
    V convert() const {
        return static_cast<V>(first + second);
    }
};

struct alpha{
    int a;
};


// Variable template for compile-time type checking
template<typename T>
constexpr bool is_numeric_v = std::is_arithmetic<T>::value;

// Template variable that depends on the type's properties
template<typename T>
constexpr size_t type_size = sizeof(T);

// Templated variable with nested template types
template<typename T, typename U = T>
inline std::pair<Vector<T>, Dictionary<T, U>> default_data_structure = {
    Vector<T>(2, DEFAULT_VALUE<T>),
    {{DEFAULT_VALUE<T>, DEFAULT_VALUE<U>}}
};

int add(int a, int b, int c);

// Templated variable with conditional type
template<typename T>
inline std::conditional_t<is_numeric_v<T>, Vector<T>, Dictionary<std::string, T>> 
    type_appropriate_container = {};

double calculateArea(double radius);

#endif // MYLIB_H