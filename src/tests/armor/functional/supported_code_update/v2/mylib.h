// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MYLIB_H
#define MYLIB_H

// Simple enum for color representation
// new comments
enum Color {
    RED,
    GREEN,
    BLUE,
    YELLOW
};

// Struct representing a 2D point
struct Point {
    int x;
    int y;
};

// Struct representing a rectangle
struct Rectangle {
    struct Point topLeft;
};

// Namespace for geometry utilities
namespace Geometry {
    // Enum for shape types
    enum ShapeType {
        CIRCLE,
        SQUARE,
        TRIANGLE
    };
    
    // Struct for circle properties
    struct Circle {
        struct Point center;
        double radius;
    };
    
    // Function to calculate area (declaration only)
    double calculateArea(enum ShapeType type, double dimension);
}

// Namespace for data structures
namespace DataStructures {
    // Class template for a simple container
    template<typename T>
    class Container {
    public:
        T value;
        
        // Constructor
        Container(T val) : value(val) {}
        
        // Get the stored value
        T getValue() const {
            return value;
        }
        
        // Set a new value
        void setValue(T val) {
            value = val;
            // may be use a tmp var
        }
    };
    
    // Class template for a pair of values
    template<typename T1, typename T2>
    class Pair {
    private:
        T1 first;


        T2 second;
        
    public:
        // Constructor new
        Pair(T1 f,          T2 s) : first(f), second(s) {}
        
        // Get first element
        T1 getFirst() const { return first; }
        
        // Get second element
        T2 getSecond() const { return second; }
    };
}

// Nested namespace example
namespace Math {
    namespace Constants {
        // Mathematical constants
        const double PI = 3.14159265359;


        const double E = 2.71828182846;
    }
    
    // Enum for mathematical operations
    enum Operation {
        ADD,SUBTRACT,MULTIPLY,DIVIDE
    };
}

// Global function declarations
void initializeLibrary();

#endif // MYLIB_H