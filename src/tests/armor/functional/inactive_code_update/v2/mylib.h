// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MYLIB_H_INCLUDED
#define MYLIB_H_INCLUDED

// ============================================================================
// PREPROCESSING CONFIGURATION
// ============================================================================

/// Version macros
#define MYLIB_VERSION_MAJOR 1
#define MYLIB_VERSION_MINOR 0
#define MYLIB_VERSION_PATCH 0

/// Stringify helpers
#define MYLIB_STRINGIFY(x) #x
#define MYLIB_TOSTRING(x) MYLIB_STRINGIFY(x)
#define MYLIB_VERSION_STRING MYLIB_TOSTRING(MYLIB_VERSION_MAJOR) "." \
                             MYLIB_TOSTRING(MYLIB_VERSION_MINOR) "." \
                             MYLIB_TOSTRING(MYLIB_VERSION_PATCH)

// ============================================================================
// PLATFORM DETECTION
// ============================================================================

#if defined(_WIN32) || defined(_WIN64)
    #define MYLIB_PLATFORM_WINDOWS
    #define MYLIB_EXPORT __declspec(dllexport)
#elif defined(__linux__)
    #define MYLIB_PLATFORM_LINUX
    #define MYLIB_EXPORT __attribute__((visibility("default")))
#elif defined(__APPLE__) && defined(__MACH__)
    #define MYLIB_PLATFORM_MACOS
    #define MYLIB_EXPORT __attribute__((visibility("default")))
#else
    #define MYLIB_PLATFORM_UNKNOWN
    #define MYLIB_EXPORT
#endif

// ============================================================================
// COMPILER DETECTION
// ============================================================================

#if defined(__clang__)
    #define MYLIB_COMPILER_CLANG
    #define MYLIB_FORCE_INLINE __attribute__((always_inline)) inline
#elif defined(__GNUC__)
    #define MYLIB_COMPILER_GCC
    #define MYLIB_FORCE_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
    #define MYLIB_COMPILER_MSVC
    #define MYLIB_FORCE_INLINE __forceinline
#else
    #define MYLIB_COMPILER_UNKNOWN
    #define MYLIB_FORCE_INLINE inline
#endif

// ============================================================================
// FEATURE FLAGS
// ============================================================================

#ifdef MYLIB_DEBUG
    #define MYLIB_DEBUG_MODE 1
    #include <cassert>
    #define MYLIB_ASSERT(cond, msg) assert((cond) && (msg))
    #define MYLIB_LOG(msg) printf("[LOG] %s\n", msg)
#else
    #define MYLIB_DEBUG_MODE 0
    #define MYLIB_ASSERT(cond, msg) ((void)0)
    #define MYLIB_LOG(msg) ((void)0)
#endif

#ifdef MYLIB_ENABLE_THREADING
    #define MYLIB_THREAD_SAFE 1
    #include <mutex>
#else
    #define MYLIB_THREAD_SAFE 0
#endif

// ============================================================================
// UTILITY MACROS
// ============================================================================

#define MYLIB_MAX(a, b) ((a) > (b) ? (a) : (b))
#define MYLIB_MIN(a, b) ((a) < (b) ? (a) : (b))
#define MYLIB_CLAMP(val, min, max) MYLIB_MIN(MYLIB_MAX(val, min), max)
#define MYLIB_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MYLIB_UNUSED(x) ((void)(x))

#ifdef MYLIB_COMPILER_MSVC
    #define MYLIB_ALIGN(n) __declspec(align(n))
#else
    #define MYLIB_ALIGN(n) __attribute__((aligned(n)))
#endif

// ============================================================================
// C++ INTERFACE
// ============================================================================

#ifdef __cplusplus

#include <cstdint>
#include <cstddef>
#include <string>

namespace mylib {

    // Type aliases
    using byte_t = uint8_t;
    using size_type = std::size_t;
    
    // Enumeration
    enum class ErrorCode : int32_t {
        SUCCESS = 0,
        FAILURE = -1,
        INVALID_ARGUMENT = -2
    };
    
    // Structure
    struct Point3D {
        float x, y, z;
        Point3D() : x(0.0f), y(0.0f), z(0.0f) {}
        Point3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    };
    
    // Union
    union DataUnion {
        int32_t i32;
        float f32;
        byte_t bytes[4];
        DataUnion() : i32(0) {}
    };

    // Template Class
    template<typename T>
    class SmartPointer {
    private:
        T* ptr_;
    public:
        SmartPointer() : ptr_(nullptr) {}
        explicit SmartPointer(T* ptr) : ptr_(ptr) {}
        ~SmartPointer() { delete ptr_; }
        T* get() const { return ptr_; }
    };
    
    // Template Function
    template<typename T>
    MYLIB_FORCE_INLINE void swap(T& a, T& b) {
        T temp = a; a = b; b = temp;
    }
    
    // Function Declarations
    ErrorCode initializeLibrary();
    void* allocateAligned(size_type size, size_type alignment);
    
} // namespace mylib

// ============================================================================
// BIG FUNCTION WITH HEAVY CONDITIONAL COMPILATION
// ============================================================================

#ifdef __cplusplus

/// @brief Complex processing function with heavy conditional compilation
inline int processData(void* data, size_t size, int flags) {
    #ifdef MYLIB_DEBUG
        MYLIB_ASSERT(data != nullptr, "Data pointer is null");
        printf("[DEBUG] Processing %zu bytes, Flags: 0x%X\n", size, flags);
    #endif
    
    int result = 0;
    
    // Platform-specific memory allocation
    #ifdef MYLIB_PLATFORM_WINDOWS
        void* buffer = _aligned_malloc(size, 16);
        #ifdef MYLIB_COMPILER_MSVC
            __assume(size > 0);
        #endif
    #elif defined(MYLIB_PLATFORM_LINUX)
        void* buffer = aligned_alloc(16, size);
        #ifdef MYLIB_COMPILER_GCC
            __builtin_prefetch(buffer, 0, 3);
        #endif
    #elif defined(MYLIB_PLATFORM_MACOS)
        void* buffer = nullptr;
        posix_memalign(&buffer, 16, size);
    #else
        void* buffer = malloc(size);
    #endif
    
    if (!buffer) return -1;
    
    // Threading support
    #ifdef MYLIB_ENABLE_THREADING
        static std::mutex mtx;
        static std::atomic<int> count{0};
        std::lock_guard<std::mutex> lock(mtx);
        result = count.fetch_add(1, std::memory_order_relaxed);
    #else
        static int count = 0;
        result = ++count;
    #endif
    
    // Flag-based processing with compiler-specific code
    if (flags & 0x01) {
        #ifdef MYLIB_COMPILER_MSVC
            result += 100;
        #elif defined(MYLIB_COMPILER_GCC) || defined(MYLIB_COMPILER_CLANG)
            result += 200;
        #else
            result += 300;
        #endif
    }
    
    if (flags & 0x02) {
        #ifdef MYLIB_PLATFORM_WINDOWS
            result += 1000;
        #elif defined(MYLIB_PLATFORM_LINUX)
            result += 2000;
        #elif defined(MYLIB_PLATFORM_MACOS)
            result += 3000;
        #else
            result += 4000;
        #endif
    }
    
    #ifdef MYLIB_DEBUG
        #define NEW_LIB
        static size_t totalBytes = 0;
        totalBytes += size;
        printf("[DEBUG] Total processed: %zu bytes, Result: %d\n", totalBytes, result);
    #endif
    
    // Platform-specific cleanup
    #ifdef MYLIB_PLATFORM_WINDOWS
        _aligned_free(buffer);
    #else
        free(buffer);
    #endif
    
    return result;
}

#endif // __cplusplus

// ============================================================================
// COMPILER-SPECIFIC OPTIMIZATIONS
// ============================================================================

#ifdef MYLIB_COMPILER_MSVC
    #define MYLIB_LIKELY(x) (x)
    #define NEW_LIB
#elif defined(MYLIB_COMPILER_GCC) || defined(MYLIB_COMPILER_CLANG)
    #define MYLIB_LIKELY(x) __builtin_expect(!!(x), 1)
#else
    #define MYLIB_LIKELY(x) (x)
#endif

// ============================================================================
// C INTERFACE
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

MYLIB_EXPORT int mylib_init(void);
MYLIB_EXPORT const char* mylib_get_version(void);

#ifdef __cplusplus
}
#endif

#endif // MYLIB_H_INCLUDED

#endif
// End of mylib.h - Compact version with preprocessing, templates, classes, structs, unions
