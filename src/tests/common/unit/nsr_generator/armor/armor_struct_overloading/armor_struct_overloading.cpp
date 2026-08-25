// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <tuple>

// Forward declarations for template friends
template<typename T> struct TemplateStruct;
template<typename T> struct TemplateStructFriend;

// Overloaded struct with template parameters
template<typename T>
struct TemplateStruct {
    T value;
    
    // Constructor overloading
    TemplateStruct() : value{} {}
    explicit TemplateStruct(T val) : value(val) {}
    
    // Method overloading
    void set(T val);
    
    T get() const;
    
    // Friend declaration
    friend struct TemplateStructFriend<T>;
};

// Template specialization for pointer types
template<typename T>
struct TemplateStruct<T*> {
    T* value;
    bool ownsPointer;
    
    // Constructor overloading
    TemplateStruct() : value(nullptr), ownsPointer(false) {}
    explicit TemplateStruct(T* val, bool takeOwnership = false) 
        : value(val), ownsPointer(takeOwnership) {}
    
    ~TemplateStruct();
    
    // Method overloading
    void set(T* val, bool takeOwnership = false);
    
    T* get() const;
    
    // Friend declaration
    friend struct TemplateStructFriend<T*>;
};

// Complete specialization for int
template<>
struct TemplateStruct<int> {
    int value;
    
    TemplateStruct() : value(0) {}
    explicit TemplateStruct(int val) : value(val) {}
    
    void set(int val);
    
    // Overloaded method specific to int specialization
    bool isEven() const;

    int get() const;
    
    // Friend declaration
    friend struct TemplateStructFriend<int>;
};

// Friend struct template
template<typename T>
struct TemplateStructFriend {
    void inspect(const TemplateStruct<T>& ts);
};

// Variadic template struct
template<typename... Ts>
struct VariadicStruct {
    std::tuple<Ts...> data;
    
    VariadicStruct(Ts... args) : data(args...) {}
    
    // Overloaded methods with parameter packs
    template<size_t Index>
    auto get() const;
    
    template<typename T>
    bool contains() const;
};

// Struct with tag dispatch overloading
struct IntTag {};
struct FloatTag {};
struct StringTag {};

struct TagDispatcher {
    template<typename T>
    static auto dispatch(T value);
    
    static void process(int value, IntTag);
    
    static void process(float value, FloatTag);
};

// Struct with std::variant and overloaded pattern
template<typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

// Struct with virtual method overloading
struct BaseVirtual {
    virtual void method(int value);
    
    virtual void method(double value);
    
    virtual void method(const std::string& value);
    
    virtual ~BaseVirtual() = default;
};

struct DerivedVirtual : BaseVirtual {
    void method(int value) override;
    
    void method(double value) override;

    // Additional overload not in base
    void method(bool value);
};

// Storage policies
struct DirectStorage {
    using ValueType = int;
    using StorageType = int;
    
    static void store(StorageType& storage, const ValueType& value);
    
    static ValueType retrieve(const StorageType& storage) {
        return storage;
    }
};

struct PointerStorage {
    using ValueType = int;
    using StorageType = std::unique_ptr<int>;
    
    static void store(StorageType& storage, const ValueType& value) {
        storage = std::make_unique<int>(value);
    }
    
    static ValueType retrieve(const StorageType& storage) {
        return *storage;
    }
};

// Recursive template struct
template<unsigned N>
struct Factorial {
    static constexpr unsigned value = N * Factorial<N-1>::value;
};

template<>
struct Factorial<0> {
    static constexpr unsigned value = 1;
};

// Template struct with non-type template parameters
template<typename T, size_t Size>
struct FixedArray {
    T data[Size];
    
    // Constructor overloading
    FixedArray();
    
    explicit FixedArray(const T& defaultValue);
};

// Specialization for bool arrays
template<size_t Size>
struct FixedArray<bool, Size> {
    
    FixedArray();
    
    explicit FixedArray(bool defaultValue);

    // Overloaded operators
    int operator[](size_t index);
    bool operator[](size_t index) const;
};

// Recursive template struct with specializations
template<typename T, size_t Depth = 0>
struct NestedContainer {
    using type = std::vector<typename NestedContainer<T, Depth-1>::type>;
};

template<typename T>
struct NestedContainer<T, 0> {
    using type = T;
};

// Template struct with explicit instantiation control
template<typename T>
struct ExplicitControl {
    T value;
    
    explicit ExplicitControl(T val) : value(val) {}
    
    void display() const;
    
};

// Template struct with friend function templates
template<typename T>
struct WithFriendTemplates {
    T value;
    
    WithFriendTemplates(T val) : value(val) {}
    
private:
    // Friend function template
    template<typename U>
    friend WithFriendTemplates<U> transform(const WithFriendTemplates<U>& obj, U (*func)(U));
};

// Template struct with static member specialization
template<typename T>
struct StaticSpecialization {
    static T defaultValue;
    
    static T getDefault() {
        return defaultValue;
    }
};

// Type traits with overloaded values
template<typename T>
struct CustomTraits {
    static constexpr bool is_custom = false;
    static constexpr size_t size = sizeof(T);
    using promoted_type = T;
};

template<>
struct CustomTraits<char> {
    static constexpr bool is_custom = true;
    static constexpr size_t size = 1;
    using promoted_type = int;  // Char promotes to int
};

template<>
struct CustomTraits<float> {
    static constexpr bool is_custom = true;
    static constexpr size_t size = sizeof(float);
    using promoted_type = double;  // Float promotes to double
};

// Struct with overloaded nested types
template<typename T>
struct NestedTypes {
    struct Inner {
        T value;
    };
    
    using InnerRef = Inner&;
    using ValueType = T;
};

template<>
struct NestedTypes<bool> {
    struct Inner {
        bool value;
        size_t count;  // Additional field for bool specialization
    };
    
    using InnerRef = const Inner&;  // Different reference type
    using ValueType = int;  // Different value type
};


struct { int x; } *const*volatile*&& field = nullptr;

struct {int y;} *** gamma[30];

struct SomeClass;
struct { int x; } SomeClass::* class_member_ptr;

enum { I } *****const**volatile** beta = nullptr;

struct {
  int a;
} zeta;

struct zeta{
  int a;
  int b;
} alpha1;

typedef struct {
  int a;
  int b;
} *****const**********alpha6[100];

alpha6 ok;

typedef enum {
  a,
  b
} *****const**********alpha7[100];
