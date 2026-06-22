// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
class Base {
public:
    virtual void virtualMethod() {}
    
    virtual void pureVirtualMethod() = 0;
    
    virtual void overriddenMethod() {}
    
    virtual void finalInDerived() {}
    
    static void staticMethod() {}
    
    static void staticConstMethod() {}
};

class Derived : public Base {
public:
    void overriddenMethod() override {}
    
    void finalInDerived() final {}
    
    void virtualMethod() override final {}
    
    void pureVirtualMethod() override {}
};

class TestQualifiers {
public:
    void constMethod() const {}
    
    void volatileMethod() volatile {}
    
    void constVolatileMethod() const volatile {}
    
    explicit TestQualifiers(int x) {}
    
    TestQualifiers(const TestQualifiers&) = delete;
    
    TestQualifiers(TestQualifiers&&) = default;

    TestQualifiers(int, float);
    
    explicit operator bool() const { return true; }
    
    static void staticClassMethod() {}
    
    static int staticWithImpl() {}
};


TestQualifiers::TestQualifiers(int x, float y){
    int ans = x + y;
}

class FriendTest {
private:
    int privateData;
    
public:
    friend void friendFunction(FriendTest& ft);
    
    friend class FriendClass;
};

void friendFunction(FriendTest& ft) {
    ft.privateData = 42;
}

class FriendClass {
public:
    void accessPrivateData(FriendTest& ft) {
        ft.privateData = 100;
    }
};

typedef void (*FunctionPointerType)(int, double);

extern int globalVariable;

static int staticGlobalVariable = 100;

static const int staticConstGlobalVariable = 200;

enum TestEnum {
    VALUE1,
    VALUE2,
    VALUE3
};

typedef enum {
    ANON_VALUE1,
    ANON_VALUE2
} AnonEnum;

struct TestStruct {
    int field1;
    double field2;
    
    static void staticStructMethod() {}
};

union TestUnion {
    int intValue;
    float floatValue;
};

class FinalClass {
public:
    void finalClassMethod() {}
};