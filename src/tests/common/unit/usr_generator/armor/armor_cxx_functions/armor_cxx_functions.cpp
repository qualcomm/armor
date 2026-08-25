// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
// test_overloaded_functions.cpp
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <tuple>

template<typename T, typename U>
class ComplexContainer;

template<typename... Args>
struct VariadicStruct;

enum class ProcessingMode {
    Fast,
    Accurate,
    Balanced
};

// First overloaded function with complex parameters
template<typename T, typename U, int N>
std::tuple<T, U, bool> processData(
    const std::vector<T>& inputData,
    std::map<std::string, U>& configMap,
    const std::function<U(T, int)>& transformFunc,
    const ComplexContainer<T, U>* container,
    ProcessingMode mode = ProcessingMode::Balanced,
    const std::shared_ptr<VariadicStruct<T, double, std::string>>& extraData = nullptr);

// Second overloaded function with different complex parameters
template<typename T, typename U, int N, typename... Args>
std::pair<std::vector<T>, U> processData(
    const std::map<U, std::vector<T>>& inputMap,
    std::function<void(T&, Args...)> modifierFunc,
    const std::tuple<Args...>& additionalArgs,
    int priority,
    const ComplexContainer<U, T>& settings,
    ProcessingMode mode = ProcessingMode::Fast);

// Definition of the custom types used in the functions
template<typename T, typename U>
class ComplexContainer {
public:
    T primary;
    U secondary;
    std::vector<std::pair<T, U>> elements;
    
    ComplexContainer(T p, U s) : primary(p), secondary(s) {}
    
    void add(const T& t, const U& u);
    
    std::size_t size() const;
};

template<typename... Args>
struct VariadicStruct {
    std::tuple<Args...> data;
    
    template<typename... Ts>
    VariadicStruct(Ts&&... args);
};
