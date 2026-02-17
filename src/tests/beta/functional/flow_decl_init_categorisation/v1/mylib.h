// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#ifndef LIB4_H
#define LIB4_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <functional>
#include <complex>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <typeindex>
#include <type_traits>
#include <stdexcept>

// Forward declarations
template<typename T, size_t N = 10>
class AdvancedContainer;

// Complex templated variables initialization
template<typename T>
static T global_default_value = T{};

template<>
static int global_default_value<int> = 42;

template<>
static std::string global_default_value<std::string> = "default_string";

template<>
static double global_default_value<double> = 3.14159;

// Complex static variables with initialization
static std::map<std::string, std::function<int(int, int)>> operation_map = {
    {"add", [](int a, int b) { return a + b; }},
    {"multiply", [](int a, int b) { return a * b; }},
    {"subtract", [](int a, int b) { return a - b; }},
    {"divide", [](int a, int b) { return b != 0 ? a / b : 0; }}
};

static std::unordered_map<std::string, std::vector<std::complex<double>>> complex_data = {
    {"signal1", {{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}}},
    {"signal2", {{2.0, 1.0}, {4.0, 3.0}, {6.0, 5.0}}},
    {"noise", {{0.1, 0.2}, {0.3, 0.4}, {0.5, 0.6}}}
};

// Complex templated class with inline implementation
template<typename T, size_t N>
class AdvancedContainer {
private:
    std::array<T, N> data_;
    size_t size_;
    std::function<bool(const T&, const T&)> comparator_;

public:
    // Constructor with initializer list
    AdvancedContainer(std::initializer_list<T> init_list = {}) 
        : size_(0), comparator_([](const T& a, const T& b) { return a < b; }) {
        for (const auto& item : init_list) {
            if (size_ < N) {
                data_[size_++] = item;
            }
        }
    }

    // Template member function
    template<typename Predicate>
    std::vector<T> filter(Predicate pred) const {
        std::vector<T> result;
        for (size_t i = 0; i < size_; ++i) {
            if (pred(data_[i])) {
                result.push_back(data_[i]);
            }
        }
        return result;
    }

    // Complex member function with lambda
    void sort_custom() {
        std::sort(data_.begin(), data_.begin() + size_, comparator_);
    }

    // Operator overloading
    T& operator[](size_t index) {
        if (index >= size_) throw std::out_of_range("Index out of range");
        return data_[index];
    }

    const T& operator[](size_t index) const {
        if (index >= size_) throw std::out_of_range("Index out of range");
        return data_[index];
    }

    // Iterator support
    T* begin() { return data_.data(); }
    T* end() { return data_.data() + size_; }
    const T* begin() const { return data_.data(); }
    const T* end() const { return data_.data() + size_; }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    void push_back(const T& value) {
        if (size_ < N) {
            data_[size_++] = value;
        }
    }

    void set_comparator(std::function<bool(const T&, const T&)> comp) {
        comparator_ = comp;
    }
};

// Complex struct with nested types and initialization
struct ComplexDataStructure {
    struct NestedConfig {
        int priority = 1;
        std::string name = "default";
        std::vector<double> weights = {1.0, 0.5, 0.25};
        
        NestedConfig() = default;
        NestedConfig(int p, const std::string& n, std::vector<double> w) 
            : priority(p), name(n), weights(std::move(w)) {}
    };

    std::map<std::string, NestedConfig> configurations = {
        {"high_performance", {10, "high_perf", {2.0, 1.5, 1.0}}},
        {"balanced", {5, "balanced", {1.0, 1.0, 1.0}}},
        {"low_power", {1, "low_power", {0.5, 0.3, 0.1}}}
    };

    std::unique_ptr<std::vector<std::string>> dynamic_data = 
        std::make_unique<std::vector<std::string>>(
            std::initializer_list<std::string>{"item1", "item2", "item3"}
        );

    mutable std::function<void(const std::string&)> logger = 
        [](const std::string& msg) { 
            std::cout << "[LOG]: " << msg << std::endl; 
        };

    // Complex member function
    template<typename Callback>
    void process_configurations(Callback callback) const {
        for (const auto& [key, config] : configurations) {
            callback(key, config);
            logger("Processed configuration: " + key);
        }
    }

    // Function with complex logic
    std::vector<std::string> get_sorted_config_names() const {
        std::vector<std::string> names;
        for (const auto& [key, config] : configurations) {
            names.push_back(key);
        }
        
        std::sort(names.begin(), names.end(), 
                  [this](const std::string& a, const std::string& b) {
                      return configurations.at(a).priority > configurations.at(b).priority;
                  });
        
        return names;
    }
};

// Global complex variables with initialization
static ComplexDataStructure global_config;

static AdvancedContainer<int, 20> global_int_container{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

static AdvancedContainer<std::string, 15> global_string_container{
    "alpha", "beta", "gamma", "delta", "epsilon"
};

// Complex templated function with multiple template parameters
template<typename Container, typename Predicate, typename Transform>
auto complex_transform_filter(const Container& container, Predicate pred, Transform trans) 
    -> std::vector<decltype(trans(*container.begin()))> {
    
    using ResultType = decltype(trans(*container.begin()));
    std::vector<ResultType> result;
    
    for (const auto& item : container) {
        if (pred(item)) {
            result.push_back(trans(item));
        }
    }
    
    return result;
}

// Variadic template function
template<typename T, typename... Args>
std::unique_ptr<T> make_complex_unique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// Complex function with multiple overloads
inline std::string process_data(int value) {
    return "Processing integer: " + std::to_string(value);
}

inline std::string process_data(const std::string& value) {
    return "Processing string: " + value;
}

template<typename T>
std::string process_data(const std::vector<T>& values) {
    return "Processing vector of size: " + std::to_string(values.size());
}

// Complex lambda stored as static variable
static auto complex_processor = [](const auto& input) -> decltype(auto) {
    if constexpr (std::is_arithmetic_v<std::decay_t<decltype(input)>>) {
        return input * 2;
    } else if constexpr (std::is_same_v<std::decay_t<decltype(input)>, std::string>) {
        return input + "_processed";
    } else {
        return input;
    }
};

// Complex initialization with SFINAE
template<typename T>
struct ComplexInitializer {
    static T get_default() {
        if constexpr (std::is_arithmetic_v<T>) {
            return T{42};
        } else if constexpr (std::is_same_v<T, std::string>) {
            return std::string{"default_value"};
        } else {
            return T{};
        }
    }
};

// Static complex templated variables using the initializer
template<typename T>
static T complex_default = ComplexInitializer<T>::get_default();

// Specialized complex variables
static auto specialized_int_default = complex_default<int>;
static auto specialized_string_default = complex_default<std::string>;
static auto specialized_double_default = complex_default<double>;

// Complex nested namespace with inline variables
namespace detail {
    inline static std::map<std::type_index, std::string> type_names = {
        {std::type_index(typeid(int)), "integer"},
        {std::type_index(typeid(double)), "double"},
        {std::type_index(typeid(std::string)), "string"},
        {std::type_index(typeid(std::vector<int>)), "vector<int>"}
    };

    template<typename T>
    inline static std::string get_type_name() {
        auto it = type_names.find(std::type_index(typeid(T)));
        return it != type_names.end() ? it->second : "unknown";
    }
}

// Complex matrix class with advanced features
template<typename T, size_t Rows, size_t Cols>
class Matrix {
private:
    std::array<std::array<T, Cols>, Rows> data_;

public:
    Matrix() {
        for (auto& row : data_) {
            row.fill(T{});
        }
    }

    Matrix(std::initializer_list<std::initializer_list<T>> init) : Matrix() {
        size_t row = 0;
        for (const auto& row_data : init) {
            if (row >= Rows) break;
            size_t col = 0;
            for (const auto& value : row_data) {
                if (col >= Cols) break;
                data_[row][col] = value;
                ++col;
            }
            ++row;
        }
    }

    T& operator()(size_t row, size_t col) {
        if (row >= Rows || col >= Cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return data_[row][col];
    }

    const T& operator()(size_t row, size_t col) const {
        if (row >= Rows || col >= Cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return data_[row][col];
    }

    template<size_t OtherCols>
    Matrix<T, Rows, OtherCols> multiply(const Matrix<T, Cols, OtherCols>& other) const {
        Matrix<T, Rows, OtherCols> result;
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < OtherCols; ++j) {
                T sum = T{};
                for (size_t k = 0; k < Cols; ++k) {
                    sum += data_[i][k] * other(k, j);
                }
                result(i, j) = sum;
            }
        }
        return result;
    }

    Matrix<T, Rows, Cols> operator+(const Matrix<T, Rows, Cols>& other) const {
        Matrix<T, Rows, Cols> result;
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                result(i, j) = data_[i][j] + other(i, j);
            }
        }
        return result;
    }

    constexpr size_t rows() const { return Rows; }
    constexpr size_t cols() const { return Cols; }
};

// Global matrix instances with initialization
static Matrix<double, 3, 3> identity_matrix{
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
};

static Matrix<int, 2, 3> sample_matrix{
    {1, 2, 3},
    {4, 5, 6}
};

// Complex function pointer type with initialization
using ComplexFunctionType = std::function<std::vector<double>(const std::vector<double>&, double)>;

static std::map<std::string, ComplexFunctionType> signal_processors = {
    {"amplify", [](const std::vector<double>& signal, double factor) {
        std::vector<double> result;
        result.reserve(signal.size());
        for (double val : signal) {
            result.push_back(val * factor);
        }
        return result;
    }},
    {"normalize", [](const std::vector<double>& signal, double) {
        std::vector<double> result = signal;
        if (!result.empty()) {
            double max_val = *std::max_element(result.begin(), result.end());
            if (max_val != 0.0) {
                for (double& val : result) {
                    val /= max_val;
                }
            }
        }
        return result;
    }},
    {"offset", [](const std::vector<double>& signal, double offset) {
        std::vector<double> result;
        result.reserve(signal.size());
        for (double val : signal) {
            result.push_back(val + offset);
        }
        return result;
    }}
};

// Complex function using all the above components
template<typename T>
void demonstrate_complex_functionality() {
    std::cout << "=== Complex Functionality Demo ===" << std::endl;
    
    // Use global templated variables
    std::cout << "Default value for type: " << global_default_value<T> << std::endl;
    
    // Use complex container
    AdvancedContainer<T, 5> container;
    if constexpr (std::is_same_v<T, int>) {
        container.push_back(10);
        container.push_back(5);
        container.push_back(15);
        
        // Demonstrate filtering
        auto filtered = container.filter([](const T& val) { return val > 7; });
        std::cout << "Filtered values count: " << filtered.size() << std::endl;
    }
    
    // Use complex data structure
    global_config.process_configurations([](const std::string& name, const auto& config) {
        std::cout << "Config: " << name << ", Priority: " << config.priority << std::endl;
    });
    
    // Use complex operations
    if (operation_map.count("add")) {
        std::cout << "Operation result: " << operation_map["add"](10, 20) << std::endl;
    }
    
    // Use complex processor
    if constexpr (std::is_arithmetic_v<T>) {
        auto result = complex_processor(T{5});
        std::cout << "Processed result: " << result << std::endl;
    }
    
    // Use matrix operations
    std::cout << "Identity matrix (0,0): " << identity_matrix(0, 0) << std::endl;
    std::cout << "Sample matrix (1,2): " << sample_matrix(1, 2) << std::endl;
    
    // Use signal processors
    std::vector<double> test_signal = {1.0, 2.0, 3.0, 4.0, 5.0};
    if (signal_processors.count("amplify")) {
        auto amplified = signal_processors["amplify"](test_signal, 2.0);
        std::cout << "Amplified signal size: " << amplified.size() << std::endl;
    }
    
    std::cout << "Type name: " << detail::get_type_name<T>() << std::endl;
}

// Complex recursive template for compile-time calculations
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N-1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
    int a = 10;
    int fun();
};

// Static constexpr variables using recursive templates
static constexpr int factorial_5 = Factorial<5>::value;
static constexpr int factorial_7 = Factorial<7>::value;

// Complex CRTP (Curiously Recurring Template Pattern) example
template<typename Derived>
class Printable {
public:
    void print() const {
        static_cast<const Derived*>(this)->print_impl();
    }
    
    void debug_info() const {
        std::cout << "Type: " << typeid(Derived).name() << std::endl;
        static_cast<const Derived*>(this)->print_impl();
    }
};

class ComplexNumber : public Printable<ComplexNumber> {
private:
    double real_, imag_;

public:
    ComplexNumber(double r = 0.0, double i = 0.0) : real_(r), imag_(i) {}
    
    void print_impl() const {
        std::cout << real_ << " + " << imag_ << "i" << std::endl;
    }
    
    ComplexNumber operator+(const ComplexNumber& other) const {
        return ComplexNumber(real_ + other.real_, imag_ + other.imag_);
    }
    
    ComplexNumber operator*(const ComplexNumber& other) const {
        return ComplexNumber(
            real_ * other.real_ - imag_ * other.imag_,
            real_ * other.imag_ + imag_ * other.real_
        );
    }
    
    double magnitude() const {
        return std::sqrt(real_ * real_ + imag_ * imag_);
    }
};

// Global complex number instances
static ComplexNumber complex_unit_i(0.0, 1.0);
static ComplexNumber complex_golden_ratio(1.618, 0.0);
static ComplexNumber complex_euler(2.718, 3.14159);

#endif // LIB4_H