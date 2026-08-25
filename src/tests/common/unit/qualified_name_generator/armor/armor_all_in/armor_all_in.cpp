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

using IntVector = std::vector<int>;

namespace mylib {

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

Vector<int> zeta;

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

// // Template with default parameter
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

// Templated variable with templated type
template<typename T>
inline Vector<T> DEFAULT_COLLECTION = Vector<T>(3, DEFAULT_VALUE<T>);

// Templated variable with multiple template parameters
template<typename K, typename V>
inline Dictionary<K, V> DEFAULT_MAP = {{DEFAULT_VALUE<K>, DEFAULT_VALUE<V>}};

// // Class with templated fields
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

template<>
int add<int>(int,int);

typedef struct{
    int a;
} anon_alpha;

typedef struct{
    int a;
} ******const***volatile********const const volatile anon_beta[30];

struct {
    int a;
} gamma;

struct {
    int a;
} ******const***volatile**const******&& anon_gamma = nullptr;


} // namespace mylib

double calculateArea(double radius);

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

int add(int,int);

typedef struct{
    int a;
} anon_alpha;

typedef struct{
    int a;
} ******const***volatile********const const volatile anon_beta[30];

struct {
    int a;
} gamma;

struct {
    int a;
} ******const***volatile**const******&& anon_gamma = nullptr;


typedef int (*fun) (int,int);

template<>
class mylib::Container<fun> {
    int fun();
};

// First, let's create a template class that will be used as a parameter
template<typename T = int>
class InnerTemplate {
public:
    T value;
    
    InnerTemplate() : value(T()) {}
    explicit InnerTemplate(T val) : value(val) {}
    
    T getValue() const { return value; }
};

// Now, let's create the Container class that accepts a template class as its parameter
template<template<typename> class T, template<typename> class U>
class ok {
public:
    // Using the template parameter to create an instance with int
    T<int> intInstance;
    
    // Using the template parameter to create an instance with double
    T<double> doubleInstance;
    
    int fun() {
        return static_cast<int>(intInstance.getValue() + doubleInstance.getValue());
    }
};

typedef InnerTemplate<> alpha;

// You can also specialize Container if needed
template<>
class ok<InnerTemplate, InnerTemplate> {
public:
    InnerTemplate<int> specializedInstance;
    
    int fun() {
        return specializedInstance.getValue() * 2;
    }
};

// Basic template class with type parameter
template<typename T>
class SimpleTemplate {
public:
    T value;
    SimpleTemplate(T val) : value(val) {}
};

// Template with multiple parameters including a non-type parameter
template<typename T, int N>
class ArrayTemplate {
public:
    T array[N];
};

// Template with a template template parameter
template<template<typename> class TemplateParam>
class MetaTemplate {
public:
    TemplateParam<int> instance;
    TemplateParam<double> doubleInstance;
};

// Template with parameter pack
template<typename... Args>
class VariadicTemplate {
public:
    template<typename T>
    void method(T value, Args... args) {}
};

// Template with template template parameter that is itself a parameter pack
template<template<typename...> class... Templates>
class VariadicMetaTemplate {};

// Nested template example
template<typename T>
class Outer {
public:
    template<typename U>
    class Inner {
    public:
        T outerValue;
        U innerValue;
    };
    
    Inner<int> innerInstance;
};

// Function template
template<typename T>
T add(T a, T b) {
    return a + b;
}

// Function template with non-type parameter
template<int N>
int multiply(int value) {
    return value * N;
}

// Variable template (C++14)
template<typename T>
constexpr T pi = T(3.1415926535897932385);


template<typename T, int N, template<typename> class TT>
class ComplexTemplate {
public:
    T value;
    T array[N];
    TT<T> templateInstance;
};

template<template<typename, int> class TemplateParam>
class AdvancedMetaTemplate {
public:
    TemplateParam<int, 5> instance;
};

// Basic declarations to test VisitNamedDecl
namespace test_namespace {
    int simple_var = 42;
    
    void simple_function() {}
    
    class SimpleClass {
    public:
        int field;
        static int static_field;
        void method() {}
        static void static_method() {}
    };
    
    // Test namespace alias
    namespace ns1 {
        int ns1_var = 1;
    }
    namespace ns_alias = ns1;
}

// Anonymous namespace to test VisitNamespaceDecl
namespace {
    int anon_var = 10;
}

// Test for anonymous struct in field declaration
struct ContainsAnonymousStruct {
    struct {
        int anonymous_field;
    } field_with_anonymous_struct;
    
    union {
        int i;
        float f;
    } field_with_anonymous_union;
    
    enum {
        VAL1,
        VAL2
    } field_with_anonymous_enum;
};

// Test for anonymous struct with typedef
typedef struct {
    int data;
} TypedefForAnonymousStruct;

// Test for embedded anonymous struct
struct OuterStruct {
    struct {
        int inner_field;
    };  // Anonymous struct directly embedded
};

// Test for function templates
template<typename T>
T template_function(T a, T b) {
    return a + b;
}

// Test for class templates
template<typename T>
class TemplateClass {
public:
    T value;
    TemplateClass(T val) : value(val) {}
};

// Test for template specialization
template<>
class TemplateClass<void*> {
public:
    void* ptr;
    TemplateClass(void* p) : ptr(p) {}
};

// Test for partial template specialization
template<typename T>
class TemplateClass<T*> {
public:
    T* ptr;
    TemplateClass(T* p) : ptr(p) {}
};

// Test for template with default parameter values
template<typename T = int, int N = 10>
class DefaultParamTemplate {
public:
    T values[N];
};

// Test for linkage specification
extern "C" {
    void c_function();
}

// Test for using declarations and directives
namespace other_ns {
    void other_function() {}
}
using other_ns::other_function;
using namespace other_ns;

// Test for binding declarations (C++17)
void test_binding() {
    auto [x, y] = std::pair<int, int>(1, 2);
}

// Test for UnresolvedUsingValueDecl and UnresolvedUsingTypenameDecl
template<typename T>
class Base {
public:
    void base_method() {}
    typedef int base_type;
};

template<typename T>
class Derived : public Base<T> {
public:
    using Base<T>::base_method;  // UnresolvedUsingValueDecl
    using typename Base<T>::base_type;  // UnresolvedUsingTypenameDecl
};

// Test for function with variadic arguments
void variadic_function(int first, ...) {}

// Test for function with parameter that has no name
void function_with_unnamed_param(int) {}

// Test for function pointer type
void function_with_function_pointer(void (*f)(void*)) {}

// Test for template specialization with template arguments
template<typename T>
struct TemplateWithSpecialization {
    T value;
};

template<>
struct TemplateWithSpecialization<int> {
    int specialized_value;
};

// Test for class template specialization with multiple template arguments
template<typename T, typename U>
class MultiArgTemplate {
public:
    T first;
    U second;
};

template<>
class MultiArgTemplate<int, double> {
public:
    int specialized_first;
    double specialized_second;
};

// Test for variable template specialization
template<typename T>
T static_var = T(0);

template<>
int static_var<int> = 42;

    // Instantiate templates to ensure they're processed
int result1 = template_function<int>(5, 10);
double result2 = template_function<double>(3.14, 2.71);

TemplateClass<int> tc1(42);
TemplateClass<double> tc2(3.14);
TemplateClass<int*> tc3(nullptr);
TemplateClass<void*> tc4(nullptr);

ArrayTemplate<double, 5> array_template;

Outer<float> outer;
Outer<double>::Inner<char> inner_explicit;

VariadicTemplate<int, float, double> variadic;

VariadicMetaTemplate<TemplateClass, VariadicTemplate> variadic_meta;

DefaultParamTemplate<> default_template;

AdvancedMetaTemplate<ArrayTemplate> advanced_meta;

TemplateWithSpecialization<int> specialized_template;

MultiArgTemplate<int, double> multi_arg_specialized;

double pi_value = pi<double>;
    

#endif // MYLIB_H