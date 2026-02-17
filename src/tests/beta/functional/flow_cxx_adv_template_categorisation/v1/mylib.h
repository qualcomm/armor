// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#ifndef LIB5_H
#define LIB5_H

#include <memory>
#include <math.h>
#include <functional>
#include <exception>
#include <variant>
#include <optional>
#include <type_traits>
#include <iostream>

// ============================================================================
// UNIONS
// ============================================================================

// Basic union
union BasicUnion {
    int i;
    float f;
    char c;
};

// Tagged union with enum
enum class DataType { INT, FLOAT, STRING };

union TypedData {
    int int_val;
    float float_val;
    char* string_val;
    
    TypedData() : int_val(0) {}
    ~TypedData() {} // Note: unions with non-trivial members need careful handling
};

struct TaggedUnion {
    DataType type;
    TypedData data;
    
    TaggedUnion(int val) : type(DataType::INT) { data.int_val = val; }
    TaggedUnion(float val) : type(DataType::FLOAT) { data.float_val = val; }
    TaggedUnion(const char* val) : type(DataType::STRING) { data.string_val = const_cast<char*>(val); }
};

// Anonymous union
struct AnonymousUnionExample {
    enum { INT_TYPE, FLOAT_TYPE } type;
    union {  // Anonymous union
        int int_value;
        float float_value;
    };
    
    AnonymousUnionExample(int val) : type(INT_TYPE), int_value(val) {}
    AnonymousUnionExample(float val) : type(FLOAT_TYPE), float_value(val) {}
};

// ============================================================================
// BIT FIELDS
// ============================================================================

struct BitFieldExample {
    unsigned int flag1 : 1;    // 1 bit
    unsigned int flag2 : 1;    // 1 bit
    unsigned int value : 6;    // 6 bits
    unsigned int type : 3;     // 3 bits
    unsigned int : 5;          // 5 unnamed bits (padding)
    unsigned int status : 16;  // 16 bits
};

struct PackedFlags {
    bool enabled : 1;
    bool visible : 1;
    bool active : 1;
    unsigned int priority : 5;  // 0-31 priority levels
    unsigned int reserved : 24; // Reserved for future use
};

// ============================================================================
// VIRTUAL FUNCTIONS & INHERITANCE
// ============================================================================

// Abstract base class
class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;  // Pure virtual
    virtual double perimeter() const = 0;  // Pure virtual
    virtual void draw() const { std::cout << "Drawing shape\n"; }  // Virtual with default
    
    // Non-virtual function
    void info() const { std::cout << "This is a shape\n"; }
};

// Derived class
class Rectangle : public Shape {
private:
    double width_, height_;
    
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}
    
    double area() const override { return width_ * height_; }
    double perimeter() const override { return 2 * (width_ + height_); }
    void draw() const override { std::cout << "Drawing rectangle\n"; }
    
    // Additional method
    double diagonal() const { return std::sqrt(width_ * width_ + height_ * height_); }
};

// Multiple inheritance
class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void render() const = 0;
};

class Serializable {
public:
    virtual ~Serializable() = default;
    virtual std::string serialize() const = 0;
    virtual void deserialize(const std::string& data) = 0;
};

class DrawableRectangle : public Rectangle, public Drawable, public Serializable {
public:
    DrawableRectangle(double w, double h) : Rectangle(w, h) {}
    
    void render() const override { draw(); }
    
    std::string serialize() const override {
        return "Rectangle(" + std::to_string(width_) + "," + std::to_string(height_) + ")";
    }
    
    void deserialize(const std::string& data) override {
        // Simple parsing logic would go here
    }
    
private:
    double width_, height_;  // Redeclared for access
};

// ============================================================================
// EXCEPTION HANDLING
// ============================================================================

// Custom exception hierarchy
class BaseException : public std::exception {
protected:
    std::string message_;
    
public:
    explicit BaseException(const std::string& msg) : message_(msg) {}
    const char* what() const noexcept override { return message_.c_str(); }
};

class ValidationError : public BaseException {
public:
    explicit ValidationError(const std::string& msg) 
        : BaseException("Validation Error: " + msg) {}
};

class NetworkError : public BaseException {
private:
    int error_code_;
    
public:
    NetworkError(const std::string& msg, int code) 
        : BaseException("Network Error: " + msg), error_code_(code) {}
    
    int getErrorCode() const { return error_code_; }
};

// Function demonstrating exception handling
template<typename T>
T safe_divide(T a, T b) {
    if (b == T{}) {
        throw ValidationError("Division by zero");
    }
    return a / b;
}

// RAII class with exception safety
class ResourceManager {
private:
    int* resource_;
    
public:
    explicit ResourceManager(size_t size) : resource_(new int[size]) {
        if (!resource_) {
            throw std::bad_alloc();
        }
    }
    
    ~ResourceManager() noexcept {
        delete[] resource_;
    }
    
    // Move constructor
    ResourceManager(ResourceManager&& other) noexcept : resource_(other.resource_) {
        other.resource_ = nullptr;
    }
    
    // Move assignment
    ResourceManager& operator=(ResourceManager&& other) noexcept {
        if (this != &other) {
            delete[] resource_;
            resource_ = other.resource_;
            other.resource_ = nullptr;
        }
        return *this;
    }
    
    // Delete copy operations
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    int* get() const { return resource_; }
};

// ============================================================================
// TEMPLATE TEMPLATE PARAMETERS
// ============================================================================

template<template<typename> class Container, typename T>
class ContainerWrapper {
private:
    Container<T> container_;
    
public:
    void add(const T& item) { container_.push_back(item); }
    size_t size() const { return container_.size(); }
    
    template<typename Predicate>
    void remove_if(Predicate pred) {
        container_.erase(
            std::remove_if(container_.begin(), container_.end(), pred),
            container_.end()
        );
    }
};

// Template template parameter with multiple parameters
template<template<typename, typename> class Container, typename T, typename Allocator = std::allocator<T>>
class AdvancedContainerWrapper {
private:
    Container<T, Allocator> container_;
    
public:
    using value_type = T;
    using allocator_type = Allocator;
    using iterator = typename Container<T, Allocator>::iterator;
    
    void insert(const T& value) { container_.insert(container_.end(), value); }
    iterator begin() { return container_.begin(); }
    iterator end() { return container_.end(); }
};

// ============================================================================
// VARIADIC TEMPLATES
// ============================================================================

// Variadic template function with perfect forwarding
template<typename... Args>
void print_all(Args&&... args) {
    ((std::cout << std::forward<Args>(args) << " "), ...);  // C++17 fold expression
    std::cout << std::endl;
}

// Variadic template class
template<typename... Types>
class TypeList {
public:
    static constexpr size_t size = sizeof...(Types);
    
    template<size_t N>
    using type_at = std::tuple_element_t<N, std::tuple<Types...>>;
    
    // Recursive template to check if type exists
    template<typename T>
    static constexpr bool contains() {
        return (std::is_same_v<T, Types> || ...);  // C++17 fold expression
    }
};

// Variadic template with parameter pack expansion
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_with_args(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// ============================================================================
// MODERN C++ FEATURES
// ============================================================================

// std::variant usage
using Value = std::variant<int, double, std::string>;

class VariantProcessor {
public:
    static void process(const Value& v) {
        std::visit([](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int>) {
                std::cout << "Integer: " << value << std::endl;
            } else if constexpr (std::is_same_v<T, double>) {
                std::cout << "Double: " << value << std::endl;
            } else if constexpr (std::is_same_v<T, std::string>) {
                std::cout << "String: " << value << std::endl;
            }
        }, v);
    }
};

// std::optional usage
template<typename T>
std::optional<T> safe_cast(const std::string& str) {
    try {
        if constexpr (std::is_same_v<T, int>) {
            return std::stoi(str);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::stod(str);
        }
        return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}

// ============================================================================
// FUNCTION OBJECTS & LAMBDAS
// ============================================================================

// Function object (functor)
struct Multiplier {
    int factor;
    
    explicit Multiplier(int f) : factor(f) {}
    
    int operator()(int value) const {
        return value * factor;
    }
    
    template<typename T>
    T operator()(T value) const {
        return value * static_cast<T>(factor);
    }
};

// Generic lambda and function utilities
class FunctionUtils {
public:
    template<typename Container, typename Func>
    static auto transform_container(const Container& container, Func func) {
        std::vector<decltype(func(*container.begin()))> result;
        std::transform(container.begin(), container.end(), 
                      std::back_inserter(result), func);
        return result;
    }
    
    // Higher-order function
    template<typename Func>
    static auto compose(Func f) {
        return [f](auto x) { return f(x); };
    }
    
    template<typename Func1, typename Func2>
    static auto compose(Func1 f1, Func2 f2) {
        return [f1, f2](auto x) { return f1(f2(x)); };
    }
};

// ============================================================================
// CONCEPTS (C++20)
// ============================================================================

#if __cplusplus >= 202002L
#include <concepts>

// Custom concepts
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept Printable = requires(T t) {
    std::cout << t;
};

template<typename T>
concept Container = requires(T t) {
    t.begin();
    t.end();
    t.size();
};

// Function using concepts
template<Numeric T>
T add(T a, T b) {
    return a + b;
}

template<Container C>
void print_container(const C& container) {
    for (const auto& item : container) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

#endif // C++20

// ============================================================================
// COROUTINES (C++20)
// ============================================================================

#if __cplusplus >= 202002L && __has_include(<coroutine>)
#include <coroutine>

// Simple generator
template<typename T>
struct Generator {
    struct promise_type {
        T current_value;
        
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() {}
        
        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }
        
        void return_void() {}
    };
    
    std::coroutine_handle<promise_type> coro;
    
    Generator(std::coroutine_handle<promise_type> h) : coro(h) {}
    ~Generator() { if (coro) coro.destroy(); }
    
    bool next() {
        coro.resume();
        return !coro.done();
    }
    
    T value() {
        return coro.promise().current_value;
    }
};

// Coroutine function
Generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        auto temp = a;
        a = b;
        b = temp + b;
    }
}

#endif // C++20 coroutines

// ============================================================================
// GLOBAL UTILITY FUNCTIONS
// ============================================================================

// Demonstration function that uses multiple features
template<typename T>
void demonstrate_features() {
    try {
        // Union usage
        BasicUnion u;
        u.i = 42;
        std::cout << "Union int value: " << u.i << std::endl;
        
        // Bit field usage
        BitFieldExample bf{};
        bf.flag1 = 1;
        bf.value = 63;
        bf.status = 0xFFFF;
        
        // Exception handling
        auto result = safe_divide(10.0, 2.0);
        std::cout << "Division result: " << result << std::endl;
        
        // Variant usage
        Value v = std::string("Hello");
        VariantProcessor::process(v);
        
        // Optional usage
        auto opt_int = safe_cast<int>("123");
        if (opt_int) {
            std::cout << "Parsed int: " << *opt_int << std::endl;
        }
        
    } catch (const BaseException& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
}

template<template<typename, typename> class Container>
typename AdvancedContainerWrapper<Container, int>::iterator global_iterator;

#endif // LIB5_H