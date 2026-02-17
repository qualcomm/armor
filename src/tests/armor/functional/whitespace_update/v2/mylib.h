// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MYLIB_H
#define MYLIB_H

// Enum for status
enum Status { SUCCESS, FAILURE, PENDING };

// Global functions
int processData(struct Packet* packet);

void initializeLibrary();

// Union for data types

union Data {
    int intValue; float floatValue;
};

// Struct with union
struct Person {
    char name[50]; int age; union Data metadata;
};

// Namespace with enum and struct


namespace Utils {
    enum LogLevel { DEBUG, INFO, ERROR };
    struct Config {
        enum LogLevel level; int maxRetries;
    };
    void log(enum LogLevel level, const char* message);
}// namespace Utils


namespace Math {
    namespace Constants {
        const double PI = 3.14159;
    }
    enum Operation { ADD, SUBTRACT };
    template<typename T>
    T add(T a, T b) { return a + b; }
}


// namespace Math

// Template classes


namespace DataStructures {
    template<typename T>
    class Container {
        T* data; int size;
    public:
        Container(int cap);
        void add(T element);
        T get(int index) const;
    };
    
    template<typename T1, typename T2>
    class Pair {
        T1 first; T2 second;
    public:
        Pair(T1 f, T2 s) : first(f), second(s) {}
        T1 getFirst() const { 
            return first; 
        }
    };
}

// Namespace with templates


// Regular class
class Logger {
    enum Utils::LogLevel currentLevel;
public:
    Logger();
    void setLevel(enum Utils::LogLevel level);
    void log(const char* message);
};

// Struct with anonymous union
struct Packet {
    int id;
    union {
        int intPayload; float floatPayload;
    } payload;
};

// Typedefs
typedef void (*CallbackFunc)(int     code);

#endif // MYLIB_H