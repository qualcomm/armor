// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <algorithm>
#include <string>
#include <complex>
#include <iostream>

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

static auto complex_processor = [](const auto& input) -> decltype(auto) {

  if constexpr (std::is_arithmetic_v<std::decay_t<decltype(input)>>) {

    return input * 2;

  } else if constexpr (std::is_same_v<std::decay_t<decltype(input)>, std::string>) {

    return input + "_processed";

  } else {

    return input;

  }

};

class ComplexNumber {
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

static ComplexNumber complex_unit_i(0.0, 1.0);
static ComplexNumber complex_golden_ratio(1.618, 0.0);
static ComplexNumber complex_euler(2.718, 3.14159);

static auto specialized_int_default = Factorial<7>::value;

struct NestedConfig {
    int priority = 1;
    std::string name = "default";
    std::vector<double> weights = {1.0, 0.5, 0.25};
    
    NestedConfig() = default;
    NestedConfig(int p, const std::string& n, std::vector<double> w) 
        : priority(p), name(n), weights(std::move(w)) {}
};
