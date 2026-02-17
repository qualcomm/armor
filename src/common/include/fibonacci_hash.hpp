// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include "llvm/ADT/StringRef.h"

namespace clang {
    class SourceManager;
    class SourceRange;
}

/**
 * FibonacciHash - A fast, non-cryptographic hash function based on the golden ratio.
 * 
 * This implementation uses the Fibonacci hashing technique which multiplies by
 * a constant derived from the golden ratio (φ = 1.618...) to achieve excellent
 * hash distribution with minimal collisions for typical string data.
 * 
 * The algorithm is optimized for speed over security and provides good avalanche
 * properties for text hashing applications.
 */
class FibonacciHash {

public:
    /**
     * Compute hash for std::string
     */
    static uint64_t hash(const std::string& str);
    
    /**
     * Compute hash for std::string_view
     */
    static uint64_t hash(std::string_view str);
    
    /**
     * Compute hash for llvm::StringRef
     */
    static uint64_t hash(const llvm::StringRef& str);
    
    /**
     * Compute normalized hash from source range using Clang Lexer.
     * Excludes comments, normalizes whitespace, and hashes only tokens.
     * 
     * @param SM Source manager for accessing source text
     * @param Range Source range to hash
     * @return Hash value, or 0 if range is invalid
     */
    static uint64_t hashFromSourceRange(clang::SourceManager* SM, clang::SourceRange Range);
    
    /**
     * Compute normalized hash from file offsets using Clang Lexer.
     * Excludes comments, normalizes whitespace, and hashes only tokens.
     * 
     * @param SM Source manager for accessing source text
     * @param startOffset Starting byte offset in the main file
     * @param endOffset Ending byte offset in the main file
     * @return Hash value, or 0 if offsets are invalid
     */
    static uint64_t hashFromOffsets(clang::SourceManager* SM, unsigned startOffset, unsigned endOffset);

private:
    /**
     * Core hashing algorithm implementation
     */
    static uint64_t fibonacci_hash_impl(const uint8_t* data, size_t length);

    // Golden ratio constant: 2^64 / φ where φ = (1 + √5) / 2
    static constexpr uint64_t GOLDEN_RATIO_64 = 0x9E3779B97F4A7C15ULL;
};