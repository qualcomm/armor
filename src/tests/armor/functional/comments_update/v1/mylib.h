// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MYLIB_H
#define MYLIB_H

#include <iostream>
#include <vector>
#include <string>

// Platform-specific macros
#ifdef _WIN32  // Check if compiling on Windows
    #define MYLIB_PLATFORM "Windows"  // Set platform string for Windows
#elif __linux__  // Check if compiling on Linux
    #define MYLIB_PLATFORM "Linux"  // Set platform string for Linux
#else  // All other platforms
    #define MYLIB_PLATFORM "Unknown"  // Default platform string
#endif  // End platform detection

// Debug logging macro
#ifdef NDEBUG  // If in release mode (NDEBUG defined)
    #define MYLIB_LOG(msg)  // Empty macro - no logging in release
#else  // Debug mode
    // Log message to stdout with [LOG] prefix
    #define MYLIB_LOG(msg) std::cout << "[LOG] " << msg << std::endl
#endif  // End debug macro definition

// Version macros
#define MYLIB_VERSION "1.0.0"

/**
 * @namespace mylib
 * @brief Main library namespace
 */
namespace mylib {

// Forward declarations
template<typename T> class Container;
template<typename T, typename U> class Pair;
class ResourceManager;

/**
 * @namespace detail
 * @brief Internal implementation details
 */
namespace detail {
    // Helper for cleanup operations
    template<typename T>
    inline void cleanup(T* ptr) {
        // Log the cleanup operation (only in debug mode)
        MYLIB_LOG("Cleaning up resource");
        // Deallocate the memory pointed to by ptr
        delete ptr;
    }
} // namespace detail

/**
 * @brief Generic container template class
 * @tparam T Element type
 */
template<typename T>
class Container {
private:
    std::vector<T> data_;  // Internal storage

public:
    // Constructor
    explicit Container(size_t capacity = 10) {
        // Pre-allocate memory to avoid reallocations
        data_.reserve(capacity);
    }

    // Add element to container
    void add(const T& element) { 
        // Append element to the end of the vector
        data_.push_back(element); 
    }

    // Get element at index
    T& at(size_t index) { 
        // Return reference with bounds checking
        return data_.at(index); 
    }
    const T& at(size_t index) const { 
        // Const version - return const reference
        return data_.at(index); 
    }

    // Get size
    size_t size() const { 
        // Return number of elements currently stored
        return data_.size(); 
    }
};

/**
 * @brief Template pair class
 * @tparam T First element type
 * @tparam U Second element type
 */
template<typename T, typename U>
class Pair {
private:
    T first_;
    U second_;

public:
    Pair(const T& first, const U& second) 
        : first_(first),   // Initialize first member
          second_(second)  // Initialize second member
    {}

    T& first() { 
        // Return reference to first element
        return first_; 
    }
    U& second() { 
        // Return reference to second element
        return second_; 
    }
};

/**
 * @brief Find maximum of two values
 */
template<typename T>
inline T max(const T& a, const T& b) {
    // Compare and return the larger value
    return (a > b) ? a : b;  // Ternary operator for concise comparison
}

/**
 * @brief Square a numeric value
 */
template<typename T>
inline T square(T value) {
    // Multiply value by itself to get the square
    return value * value;  // Works for any type with operator*
}

/**
 * @class ResourceManager
 * @brief Manages resources with RAII
 */
class ResourceManager {
private:
    std::string name_;
    bool initialized_;

public:
    explicit ResourceManager(const std::string& name) 
        : name_(name),           // Store resource name
          initialized_(false)    // Start in uninitialized state
    {}

    ~ResourceManager() { 
        // Log destruction for debugging
        MYLIB_LOG("Resource destroyed"); 
    }

    // Non-copyable
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    bool initialize() { 
        // Mark resource as initialized
        initialized_ = true; 
        // Return success status
        return true; 
    }
    bool is_initialized() const { 
        // Check initialization status
        return initialized_; 
    }
};

/**
 * @namespace algorithms
 * @brief Algorithm implementations
 */
namespace algorithms {
    // Bubble sort implementation
    template<typename T>
    void sort(Container<T>& container) {
        // Get the number of elements to sort
        size_t n = container.size();
        // Outer loop - number of passes
        for (size_t i = 0; i < n - 1; ++i) {
            // Inner loop - compare adjacent elements
            for (size_t j = 0; j < n - i - 1; ++j) {
                // If current element is greater than next
                if (container.at(j) > container.at(j + 1)) {
                    // Swap them to move larger element forward
                    std::swap(container.at(j), container.at(j + 1));
                }
            }
        }
    }
} // namespace algorithms

// Utility functions
inline const char* get_version() { 
    // Return the library version string
    return MYLIB_VERSION; 
}
inline const char* get_platform() { 
    // Return the detected platform name
    return MYLIB_PLATFORM; 
}

} // namespace mylib

#endif // MYLIB_H
