// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include  <set>
#include <bits/stdc++.h>

        #ifndef       MACRO_TEST_H       
         #define               MACRO_TEST_H         

/* ============================================================================
 * BASIC MACROS
 * ============================================================================ */

/** Simple constant macro - defines a numeric constant */
#define SIMPLE_MACRO 42

/** Function-like macro that adds two values with proper parenthesization */
#define MACRO_WITH_ARGS(x, y) ((x) + (y))

/* ============================================================================
 * STRING AND TOKEN MANIPULATION MACROS
 * ============================================================================ */

/** Converts a token to a string literal using the stringification operator */
#define STRINGIFY(x) #x

/** Concatenates two tokens using the token-pasting operator */
#define CONCAT(x, y) x##y

/** Expands the argument before stringifying (useful for macro expansion) */
#define STRINGIFY_EXPANDED(x) STRINGIFY(x)

/* ============================================================================
 * DEBUG AND LOGGING MACROS
 * ============================================================================ */

/** Conditional debug printing - only active when DEBUG is defined */
#ifdef DEBUG
#define DEBUG_PRINT(x) printf("DEBUG: %s = %d\n", #x, (x))  /* Print variable name and value */
#else
#define DEBUG_PRINT(x) do {} while(0)  /* No-op when DEBUG is not defined */
#endif

/** Variadic macro for error logging with optional arguments */
#define LOG_ERROR(format, ...) fprintf(stderr, "ERROR: " format "\n", ##__VA_ARGS__)

/* ============================================================================
 * NESTED AND RECURSIVE MACROS
 * ============================================================================ */

/** Demonstrates macro nesting - OUTER calls INNER */
#define OUTER(x) INNER(x)
/** Helper macro that doubles the input value */
#define INNER(x) (x * 2)

/** Recursive macro that calculates sum from 1 to x (factorial-like behavior) */
#define RECURSIVE_MACRO(x) ((x) > 0 ? RECURSIVE_MACRO_IMPL(x) : 0)
/** Implementation helper for recursive macro to avoid infinite recursion */
#define RECURSIVE_MACRO_IMPL(x) ((x) + RECURSIVE_MACRO((x) - 1))

/** 
 * Multi-line macro using do-while(0) idiom for safe statement-like behavior
 * Compares two values and prints their names using stringification
 */
#define COMPLEX_EXPR(x, y) \
    do { \
        if ((x) > (y)) { \
            printf("%s is greater than %s\n", #x, #y); \
        } else { \
            printf("%s is less than or equal to %s\n", #x, #y); \
        } \
    } while(0)

/** Combines stringification and concatenation to merge two tokens as strings */
#define COMBINED(x, y) CONCAT(STRINGIFY(x), STRINGIFY(y))

/* ============================================================================
 * CONDITIONAL COMPILATION - FEATURE FLAGS
 * ============================================================================ */

/** 
 * Feature flag system demonstrating complex conditional logic
 * Priority: FEATURE_A (without FEATURE_B) > FEATURE_B > default
 */
            #if      defined(FEATURE_A) &&     !defined(FEATURE_B)           // first comments
#define FEATURE_FLAG 1  /* FEATURE_A has priority when FEATURE_B is not defined */
       #elif      defined(FEATURE_B)           
#define FEATURE_FLAG 2  /* FEATURE_B is secondary priority */
      #else        
#define FEATURE_FLAG 0  /* Default when no features are enabled */
      #endif     

/* ============================================================================
 * PLATFORM-SPECIFIC DEFINITIONS
 * ============================================================================ */

/** 
 * Platform detection and function mapping
 * Provides a unified interface that calls platform-specific implementations
 */
#ifdef _WIN32
#define PLATFORM_SPECIFIC_FUNCTION() win32_function()      /* Windows implementation */
  #elif    defined(__APPLE__)     
#define PLATFORM_SPECIFIC_FUNCTION() macos_function()      /* macOS implementation */
#elif defined(__linux__)
             #define PLATFORM_SPECIFIC_FUNCTION() linux_function()  /* Linux implementation */
#else
#define PLATFORM_SPECIFIC_FUNCTION() generic_function()    /* Fallback for unknown platforms */
#endif

/* ============================================================================
 * COMPILER-SPECIFIC OPTIMIZATIONS AND ATTRIBUTES
 * ============================================================================ */

/** Inline assembly support - only available on GCC-compatible compilers */
#ifdef __GNUC__
#define INLINE_ASM(asm_code) __asm__(asm_code)  /* GCC inline assembly syntax */
#else
#define INLINE_ASM(asm_code)                    /* No-op for unsupported compilers */
#endif

/** Cross-compiler deprecation warnings */
#ifdef __GNUC__
#define DEPRECATED(func) func __attribute__((deprecated))  /* GCC/Clang attribute */
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func       /* MSVC declspec */
#else
#define DEPRECATED(func) func                              /* No deprecation warning */
#endif

/** Branch prediction hints for performance optimization */
#ifdef __GNUC__
#define LIKELY(x) __builtin_expect(!!(x), 1)    /* Hint that condition is likely true */
#define UNLIKELY(x) __builtin_expect(!!(x), 0)  /* Hint that condition is likely false */
#else
#define LIKELY(x) (x)                           /* No optimization hints */
#define UNLIKELY(x) (x)                         /* No optimization hints */
#endif

/* ============================================================================
 * FEATURE DETECTION AND VERSION CHECKING
 * ============================================================================ */

/** Generic feature detection using token pasting */
#define HAS_FEATURE(x) (FEATURE_##x == 1)  /* Checks if FEATURE_x is enabled */

/** 
 * Semantic version comparison macro
 * Returns true if the given version is greater than or equal to the current version
 */
#define VERSION_CHECK(major, minor, patch) \
    ((major) > VERSION_MAJOR || \
     ((major) == VERSION_MAJOR && (minor) > VERSION_MINOR) || \
     ((major) == VERSION_MAJOR && (minor) == VERSION_MINOR && (patch) >= VERSION_PATCH))

/* ============================================================================
 * BIT MANIPULATION UTILITIES
 * ============================================================================ */

/** Creates a bitmask with bit n set (0-indexed from right) */
#define BIT(n) (1UL << (n))

/** Sets the nth bit in variable x to 1 */
#define SET_BIT(x, n) ((x) |= BIT(n))

/** Clears the nth bit in variable x (sets to 0) */
#define CLEAR_BIT(x, n) ((x) &= ~BIT(n))

/** Toggles the nth bit in variable x (0->1, 1->0) */
#define TOGGLE_BIT(x, n) ((x) ^= BIT(n))

/** Checks if the nth bit in variable x is set (returns 1 or 0) */
#define CHECK_BIT(x, n) (!!((x) & BIT(n)))

/* ============================================================================
 * UTILITY MACROS
 * ============================================================================ */

/** 
 * Error handling macro - evaluates expression and returns early if non-zero
 * Useful for functions that return error codes
 */
#define RETURN_IF_ERROR(expr) \
    do { \
        int result = (expr); \
        if (result != 0) { \
            return result; \
        } \
    } while(0)

/** Aligns a value x to the next boundary of alignment a (must be power of 2) */
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

/** Returns the smaller of two values */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/** Returns the larger of two values */
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/** Swaps two variables using GCC's typeof extension */
#define SWAP(a, b) do { typeof(a) temp = (a); (a) = (b); (b) = temp; } while(0)

/* ============================================================================
 * INACTIVE CODE BLOCKS AND CONDITIONAL DEFINITIONS
 * ============================================================================ */

/** 
 * Code block disabled at compile time using #if 0
 * Useful for temporarily disabling code without deleting it
 */
#if 0
void inactive_function() {
    int x = 42;
    printf("This function is in an inactive block\n");
}
#endif 

/** 
 * Code block that's only compiled if UNDEFINED_MACRO is defined
 * Since UNDEFINED_MACRO is not defined, this code is inactive
 */
#ifdef UNDEFINED_MACRO
void another_inactive_function() {
    printf("This function is also inactive\n");
}
#endif

/** Demonstrate macro definition and immediate undefinition */
#define TEMPORARY_MACRO 123  /* Define temporarily */
#undef TEMPORARY_MACRO       /* Immediately undefine */

/** 
 * Complex conditional check combining multiple conditions
 * Checks if SIMPLE_MACRO exists but TEMPORARY_MACRO doesn't
 */
#if defined(SIMPLE_MACRO) && \
 !defined(TEMPORARY_MACRO) 
#define CONDITION_MET 1  /* Condition is satisfied */
#else 
#define CONDITION_MET 0  /* Condition is not met */
#endif

/** Forward declaration of a function */
int fun();

/* ============================================================================
 * MODERN CONDITIONAL COMPILATION (C23 features)
 * ============================================================================ */

/** 
 * Demonstrates #ifndef/#elifndef usage (C23 feature)
 * Priority: NEW_FEATURE_X > LEGACY_FEATURE_Y fallback > disabled
 */
   #ifndef    NEW_FEATURE_X  
   #define NEW_FEATURE_X 100        /* Primary feature value */
#define FEATURE_X_ENABLED 1         /* Flag indicating feature is active */
  #elifndef   LEGACY_FEATURE_Y      /* C23: "else if not defined" */
#define NEW_FEATURE_X 200           /* Fallback value when legacy feature is undefined */
#define FEATURE_Y_FALLBACK 1        /* Flag indicating fallback mode */
   #else   
#define NEW_FEATURE_X 0             /* Disabled state */
  #endif  

/** 
 * Demonstrates #ifdef/#elifdef usage (C23 feature)
 * Mode selection: experimental > stable > default
 */
     #ifdef     EXPERIMENTAL_MODE     // Checking #ifdef
#define EXPERIMENTAL_VALUE 999      /* High value for experimental mode */
#define USE_EXPERIMENTAL 1          /* Flag for experimental features */
    #elifdef      STABLE_MODE       /* C23: "else if defined" */
#define EXPERIMENTAL_VALUE 500      /* Moderate value for stable mode */
#define USE_STABLE 1                /* Flag for stable features */
     #else    
#define EXPERIMENTAL_VALUE 0        /* Default/disabled value */
   #endif   

/* ============================================================================
 * NESTED CONDITIONAL COMPILATION
 * ============================================================================ */

/** Nested conditional compilation with multiple levels */
#ifdef LEVEL_1
    #ifdef LEVEL_2
        #ifdef LEVEL_3
            #define NESTED_LEVEL 3
        #else
            #define NESTED_LEVEL 2
        #endif
    #else
        #define NESTED_LEVEL 1
    #endif
#else
    #define NESTED_LEVEL 0
#endif

/* ============================================================================
 * ADVANCED MACRO TECHNIQUES
 * ============================================================================ */

/** Complex macro with nested conditionals */
/** 
 * Advanced macro demonstrating:
 * - Pragma directives within macros
 * - Compiler warning suppression
 * - Multi-line macro with complex logic
 */
#define COMPLEX_CONDITIONAL(x) \
    _Pragma("GCC diagnostic push") \                    /* Save current diagnostic state */ \
    _Pragma("GCC diagnostic ignored \"-Wunused-variable\"") \  /* Suppress unused variable warnings */ \
    do { \
        int temp = (x); \                               /* Store value to avoid multiple evaluation */ \
        if (temp > 100) { \
            printf("Large: %d\n", temp); \
        } else if (temp > 50) { \
            printf("Medium: %d\n", temp); \
        } else { \
            printf("Small: %d\n", temp); \
        } \
    } while(0) \
    _Pragma("GCC diagnostic pop")                       /* Restore diagnostic state */

/** Variadic macro with token pasting */
/** Standard variadic macro - passes all arguments to function */
#define CALL_FUNC(func, ...) func(__VA_ARGS__)

/** C++20 variadic macro using __VA_OPT__ for optional comma insertion */
#define CALL_FUNC_OPT(func, ...) func(__VA_OPT__(,) __VA_ARGS__)

/* ============================================================================
 * INCLUDE GUARDS AND MODULE VERSIONING
 * ============================================================================ */

/** Multiple includes with guards */
#ifndef SUBMODULE_A_H
#define SUBMODULE_A_H
#define SUBMODULE_A_VERSION 1
#endif

#ifndef SUBMODULE_B_H
#define SUBMODULE_B_H
#define SUBMODULE_B_VERSION 2
#endif

/* ============================================================================
 * COMPLEX INACTIVE CODE PATTERNS
 * ============================================================================ */

/** Deeply nested inactive code */
#if 0
    #if 1
        #define SHOULD_NOT_EXIST_1 1
    #endif
    #ifdef NEVER_DEFINED
        #define SHOULD_NOT_EXIST_2 2
    #endif
#endif

/* ============================================================================
 * COMPILER DETECTION SYSTEM
 * ============================================================================ */

/** Multiple elif chains */
#if defined(COMPILER_GCC)
    #define COMPILER_NAME "GCC"
    #define COMPILER_ID 1
#elif defined(COMPILER_CLANG)
    #define COMPILER_NAME "Clang"
    #define COMPILER_ID 2
#elif defined(COMPILER_MSVC)
    #define COMPILER_NAME "MSVC"
    #define COMPILER_ID 3
#elif defined(COMPILER_ICC)
    #define COMPILER_NAME "ICC"
    #define COMPILER_ID 4
#elif defined(COMPILER_UNKNOWN)
    #define COMPILER_NAME "Unknown"
    #define COMPILER_ID 0
#else
    #define COMPILER_NAME "Generic"
    #define COMPILER_ID -1
#endif

/* ============================================================================
 * MACRO REDEFINITION PATTERNS
 * ============================================================================ */

/** Macro redefinition with undef - demonstrates iterative value changes */
#define TEMP_VALUE 100  /* Initial value */
#undef                 TEMP_VALUE       /* Remove definition */
#define TEMP_VALUE 200  /* Redefine with new value */
#undef TEMP_VALUE       /* Remove again */
#define TEMP_VALUE 300  /* Final value */


/* ============================================================================
 * API VERSION AND FEATURE AVAILABILITY
 * ============================================================================ */

/** Complex version checking */
#if (defined(VERSION_MAJOR) && VERSION_MAJOR >= 2) || \
    (defined(VERSION_MAJOR) && VERSION_MAJOR == 1 && \
     defined(VERSION_MINOR) && VERSION_MINOR >= 5)
    #define API_V2_AVAILABLE 1
    #ifdef ENABLE_EXPERIMENTAL_API
        #define API_EXPERIMENTAL 1
    #else
        #define API_EXPERIMENTAL 0
    #endif
#else
    #define API_V2_AVAILABLE 0
    #define API_EXPERIMENTAL 0
#endif

/* ============================================================================
 * ARCHITECTURE DETECTION AND OPTIMIZATION
 * ============================================================================ */

/** Architecture-specific definitions */
#if defined(__x86_64__) || defined(_M_X64)
    #define ARCH_64BIT 1
    #define ARCH_NAME "x86_64"
    #ifdef __AVX2__
        #define HAS_AVX2 1
    #else
        #define HAS_AVX2 0
    #endif
#elif defined(__i386__) || defined(_M_IX86) // ok
    #define ARCH_64BIT 0
    #define ARCH_NAME "x86"
    #define HAS_AVX2 0
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARCH_64BIT 1
    #define ARCH_NAME "ARM64"
    #define HAS_AVX2 0
#else
    #define ARCH_64BIT 0
    #define ARCH_NAME "Unknown"
    #define HAS_AVX2 0
#endif

/* ============================================================================
 * CONDITIONAL FUNCTION ABSTRACTIONS
 * ============================================================================ */

/** Conditional function-like macros - memory allocation abstraction */
#ifdef USE_CUSTOM_ALLOCATOR
    #define ALLOC(size) custom_alloc(size)  /* Use custom memory allocator */
    #define FREE(ptr) custom_free(ptr)      /* Use custom memory deallocator */
#else
    #define ALLOC(size) malloc(size)        /* Use standard malloc */
    #define FREE(ptr) free(ptr)             /* Use standard free */
#endif

/* ============================================================================
 * ADVANCED STRING MANIPULATION
 * ============================================================================ */

/** Stringification with multiple levels */
/** Two-level stringification - expands macros before stringifying */
#define XSTR(s) STR(s)          /* First level: expand the macro */
#define STR(s) #s               /* Second level: stringify the result */

/** Creates a version string from numeric components */
#define MAKE_VERSION_STRING(maj, min, patch) \
    XSTR(maj) "." XSTR(min) "." XSTR(patch)  /* e.g., "1.2.3" */


/** Token concatenation chains - building complex identifiers */
#define CAT(a, b) a##b                      /* Concatenate two tokens */
#define CAT3(a, b, c) CAT(CAT(a, b), c)     /* Concatenate three tokens */
#define CAT4(a, b, c, d) CAT(CAT3(a, b, c), d)  /* Concatenate four tokens */

/* ============================================================================
 * MODULAR FEATURE SYSTEM
 * ============================================================================ */

/** Conditional includes (simulated) - hierarchical feature enabling */
#ifdef INCLUDE_EXTRA_FEATURES
    #define EXTRA_FEATURE_1 1           /* Enable first extra feature */
    #define EXTRA_FEATURE_2 1           /* Enable second extra feature */
    #ifdef INCLUDE_EXPERIMENTAL
        #define EXPERIMENTAL_FEATURE_1 1 /* Enable experimental features only if extras are enabled */
    #endif
#endif

/* ============================================================================
 * TEMPORARY MACRO CLEANUP
 * ============================================================================ */

/** Multiple undefines - cleanup pattern for temporary macros */
#define TEMP_A 1    /* Temporary macro A */
#define TEMP_B 2    /* Temporary macro B */
#define TEMP_C 3    /* Temporary macro C */
#undef TEMP_A       /* Clean up A */
#undef TEMP_B       /* Clean up B */
#undef TEMP_C       /* Clean up C */

/* ============================================================================
 * COMPILER PRAGMA ABSTRACTIONS
 * ============================================================================ */

/** Pragma-based conditionals - structure packing control */
#ifdef _MSC_VER
    #define PRAGMA_PACK_PUSH _Pragma("pack(push, 1)")  /* MSVC: Push current packing, set to 1 byte */
    #define PRAGMA_PACK_POP _Pragma("pack(pop)")        /* MSVC: Restore previous packing */
#else // new comment will it show up
    #define PRAGMA_PACK_PUSH                            /* No-op for other compilers */
    #define PRAGMA_PACK_POP                             /* No-op for other compilers */
#endif // comment near endif

#endif