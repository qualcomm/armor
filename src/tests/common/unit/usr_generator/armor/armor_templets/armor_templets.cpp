// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <tuple>
#include<vector>
#include<string>
#include<map>
#include<memory>

template<typename... Args>
struct VariadicHolder {
    std::tuple<Args...> values;
};

VariadicHolder<int, double, std::string> variadic_instance;

template<template<typename, typename...> class OuterContainer,
         template<typename, typename...> class InnerContainer>
struct ComplexWrapper {
    OuterContainer<InnerContainer<int, double>, std::allocator<InnerContainer<int, double>>> nested;
    OuterContainer<int> simple;
    typename OuterContainer<int>::value_type value;
    
    template<typename T>
    using ContainerAlias = OuterContainer<InnerContainer<T, T>, std::allocator<InnerContainer<T, T>>>;
    
    ContainerAlias<float> aliased;
};

ComplexWrapper<std::vector, std::pair> complex_wrapper;
