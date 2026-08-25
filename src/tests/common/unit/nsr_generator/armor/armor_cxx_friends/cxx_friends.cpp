// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

class ClassA; // Forward declaration

class ClassC;

class ClassB {
public:
    void accessClassA(const ClassA& obj) {}
    int help();
};

class ClassA {
private:
    int privateData;
    
public:
    ClassA(int data) : privateData(data) {}

    void ok();
    
    // Declare a specific member function of ClassB as a friend
    friend void ClassB::accessClassA(const ClassA& obj);
    friend void ok();
    friend ClassC;
};

void ok();

union b;

struct a{
    friend void ok();
    friend b;
};

union b{
    friend void ok();
    friend a;
};

template <typename T> class A {
   friend int foo(T);
   friend class B;
   friend T; // only in C++0x
   template <typename U> friend class C;
   template <typename U> friend A& operator+=(A&, const U&) {}
};

// Forward declarations with complex template patterns
template<typename... Ts> class VariadicTemplate;
template<typename T, typename U = void> class DefaultTemplate;
template<typename T, template<typename> class Container = std::vector> class TemplateTemplate;

// Type aliases
using IntAlias = int;
using StringVector = std::vector<std::string>;
template<typename T> using Ptr = std::shared_ptr<T>;

// Complex template function with SFINAE
template<typename T, typename = T>
T complexTemplateFunction(T value) {
    return value * value;
}

// Variadic template function
template<typename... Args>
void variadicFunction(Args... args) {
    (std::cout << ... << args) << std::endl;
}

// Class with type alias friends
class ClassWithTypeAliasFriends {
private:
    int privateData = 100;

public:
    // Type alias for a function pointer
    using FunctionPtr = void(*)(ClassWithTypeAliasFriends&);
    
    // Type alias for a member function pointer
    using MemberFunctionPtr = void(ClassWithTypeAliasFriends::*)();
    
    // Friend the function pointer type
    friend FunctionPtr;
    
    // Friend a specific function that matches the alias
    friend void functionMatchingAlias(ClassWithTypeAliasFriends& obj) {
        std::cout << "Function matching alias accessing private data: " << obj.privateData << std::endl;
    }
};

// Class with complex template friends
template<typename T>
class ComplexClass {
private:
    T data;
    
    // Private nested type
    struct PrivateNested {
        T nestedData;
    };
    
    PrivateNested nested;

public:
    ComplexClass(T val) : data(val), nested{val} {}
    
    // Friend function with SFINAE
    template<typename U, typename = T>
    friend U accessWithSFINAE(const ComplexClass<U>& obj) {
        return obj.data;
    }
    
    // Friend variadic template class
    template<typename... Us>
    friend class VariadicTemplate;
    
    // Friend template template class
    template<typename U, template<typename> class Container>
    friend class TemplateTemplate;
    
    // Friend function with decltype and auto return type
    template<typename U>
    friend auto complexFriendFunction(const ComplexClass<U>& obj) -> decltype(obj.data) {
        return obj.data;
    }
    
    // Friend class template with default parameter
    template<typename U, typename V>
    friend class DefaultTemplate;
};

// Variadic template class implementation
template<typename... Ts>
class VariadicTemplate {
private:
    std::tuple<Ts...> data;
    
public:
    VariadicTemplate(Ts... values) : data(values...) {}
    
    // Method to access ComplexClass private data
    template<typename T>
    void accessComplexClassData(const ComplexClass<T>& obj) {
        std::cout << "VariadicTemplate accessing ComplexClass data: " << obj.data << std::endl;
        std::cout << "VariadicTemplate accessing ComplexClass nested data: " << obj.nested.nestedData << std::endl;
    }
};

// Template template class implementation
template<typename T, template<typename> class Container>
class TemplateTemplate {
private:
    Container<T> container;
    
public:
    // Method to access ComplexClass private data
    void accessComplexClassData(const ComplexClass<T>& obj) {
        std::cout << "TemplateTemplate accessing ComplexClass data: " << obj.data << std::endl;
    }
};

// Default template class implementation
template<typename T, typename U>
class DefaultTemplate {
public:
    // Method to access ComplexClass private data
    void accessComplexClassData(const ComplexClass<T>& obj) {
        std::cout << "DefaultTemplate accessing ComplexClass data: " << obj.data << std::endl;
    }
};

// Class with conditional friends using SFINAE
template<typename T>
class ConditionalFriends {
private:
    T data;

public:
    ConditionalFriends(T val) : data(val) {}
    
    // Friend only if T is integral
    template<typename U>
    friend U 
    integralOnlyFriend(const ConditionalFriends<U>& obj) {
        std::cout << "Integral-only friend accessing data: " << obj.data << std::endl;
    }
    
    // Friend only if T is floating point
    template<typename U>
    friend void
    floatOnlyFriend(const ConditionalFriends<U>& obj) {
        std::cout << "Float-only friend accessing data: " << obj.data << std::endl;
    }
};

// Class with recursive friend relationship
template<typename T, size_t N>
class RecursiveTemplate {
private:
    T data[N];

public:
    RecursiveTemplate(T val) {
        for (size_t i = 0; i < N; ++i) {
            data[i] = val;
        }
    }
    
    // Friend the next smaller size
    friend class RecursiveTemplate<T, N-1>;
    
    // Method to access smaller template's data
    void accessSmallerTemplate(const RecursiveTemplate<T, N-1>& smaller) {
        std::cout << "Accessing smaller template's first element: " << smaller.data[0] << std::endl;
    }
};

// Base case for recursive template
template<typename T>
class RecursiveTemplate<T, 0> {
private:
    T dummy;

public:
    RecursiveTemplate(T val) : dummy(val) {}
    
    // Friend the next larger size
    friend class RecursiveTemplate<T, 1>;
};

// Class with friend function that uses perfect forwarding
class PerfectForwardingClass {
private:
    int data = 42;

public:
    // Friend function with perfect forwarding
    template<typename... Args>
    friend auto perfectForwardingFriend(PerfectForwardingClass& obj, Args&&... args) {
        std::cout << "Perfect forwarding friend accessing data: " << obj.data << std::endl;
        return std::make_tuple(obj.data, std::forward<Args>(args)...);
    }
};

// Class with friend operators
template<typename T>
class OperatorFriendClass {
private:
    T value;

public:
    OperatorFriendClass(T val) : value(val) {}
    
    // Friend operator overloads
    template<typename U>
    friend OperatorFriendClass<U> operator+(const OperatorFriendClass<U>& lhs, const OperatorFriendClass<U>& rhs) {
        return OperatorFriendClass<U>(lhs.value + rhs.value);
    }
    
    template<typename U>
    friend std::ostream& operator<<(std::ostream& os, const OperatorFriendClass<U>& obj) {
        return os << "OperatorFriendClass{" << obj.value << "}";
    }
    
    // Friend comparison operators with SFINAE
    template<typename U>
    friend bool
    operator==(const OperatorFriendClass<U>& lhs, const OperatorFriendClass<U>& rhs) {
        return lhs.value == rhs.value;
    }
};

// Class with type traits and concept-like friends (C++17 style)
template<typename T>
class TypeTraitsFriendClass {
private:
    T data;

public:
    TypeTraitsFriendClass(T val) : data(val) {}
    
    // Friend only arithmetic types
    template<typename U>
    friend U 
    getArithmeticData(const TypeTraitsFriendClass<U>& obj) {
        return obj.data;
    }
    
    // Friend only class types
    template<typename U>
    friend const U& 
    getClassData(const TypeTraitsFriendClass<U>& obj) {
        return obj.data;
    }
};

// Class with alias template friends
template<typename T>
class AliasTemplateFriendClass {
private:
    T data;

public:
    AliasTemplateFriendClass(T val) : data(val) {}
     
    // Method to demonstrate usage
    void showData() const {
        std::cout << "AliasTemplateFriendClass data: " << data << std::endl;
    }
};

// Function to demonstrate alias template friendship
template<typename T>
void demonstrateAliasTemplateFriendship() {
    AliasTemplateFriendClass<T> obj(42);
    Ptr<AliasTemplateFriendClass<T>> ptr = std::make_shared<AliasTemplateFriendClass<T>>(obj);
    std::cout << "Accessing through alias template friend: " << ptr->data << std::endl;
}

// Specialized template for demonstration
class SpecializedFriendBase {
private:
    int secretData = 100;

public:
    // Friend a specific specialization
    template<typename T> friend class SpecializedFriend;
};

// Primary template
template<typename T>
class SpecializedFriend {
public:
    void doSomething() {
        std::cout << "Primary template" << std::endl;
    }
};

// Explicit specialization (not dependent)
template<>
class SpecializedFriend<int> {
public:
    void accessBaseData(SpecializedFriendBase& base) {
        std::cout << "Specialized friend accessing base data: " << base.secretData << std::endl;
    }
};
