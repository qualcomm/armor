// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
class Base {
public:
    virtual void virtualMethod() {}
    
    virtual void pureVirtualMethod() = 0;
    
    virtual void overriddenMethod() {}
    
    virtual void finalInDerived() const {}
    
    virtual void newVirtualMethod() {}
    
    virtual void volatileMethod() volatile {}
    
    static void staticConstMethod() {}

    static void staticMethod() {}
};

class Derived : public Base {
public:
    void overriddenMethod() override {}
    
    void finalInDerived() const final {}
    
    void virtualMethod() override {}
    
    void pureVirtualMethod() {}
    
    void newDerivedMethod() {}
};

class TestQualifiers {
public:
    void constMethod() {}
    
    void volatileMethod() const volatile {}
    
    void constVolatileMethod() const {}
    
    TestQualifiers(int x) {}
    
    TestQualifiers(const TestQualifiers&) = default;
    
    TestQualifiers(TestQualifiers&&) = delete;

    TestQualifiers(int, int);
    
    operator bool() const { return true; }
    
    void newMethod() {}
    
    static void staticClassMethod() { return; }
    
    static double staticWithImpl() { return 3.14; }
    
    static void newStaticMethod() {}
};

TestQualifiers::TestQualifiers(int x, int y){
    int ans = x + y;
}

class FriendTest {
private:
    int privateData;
    int newPrivateData;
    
public:
    friend void friendFunction(FriendTest& ft);
    
    friend class FriendClass;
    
    friend void newFriendFunction(FriendTest& ft);
    
    friend class NewFriendClass;
};


void friendFunction(FriendTest& ft) {
    ft.privateData = 100;
}


void newFriendFunction(FriendTest& ft) {
    ft.newPrivateData = 200;
}


class FriendClass {
public:
    void accessPrivateData(FriendTest& ft) {
        ft.privateData = 300;
        ft.newPrivateData = 400;
    }
};

class NewFriendClass {
public:
    void accessNewPrivateData(FriendTest& ft) {
        ft.newPrivateData = 500;
    }
};

typedef void (*FunctionPointerType)(int, double, char*);

typedef int (*NewFunctionPointerType)(const char*);

extern double globalVariable;

extern int newGlobalVariable;

static int staticGlobalVariable = 150;

static int staticConstGlobalVariable = 250;

static char staticNewGlobal = 'A';

class FinalClass final {
public:
    void finalClassMethod() {}
};