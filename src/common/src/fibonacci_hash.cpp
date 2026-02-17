// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "fibonacci_hash.hpp"
#include "logger.hpp"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/LangOptions.h"
#include <llvm-14/llvm/Support/raw_ostream.h>

uint64_t FibonacciHash::hash(const std::string& str) {
    return fibonacci_hash_impl(reinterpret_cast<const uint8_t*>(str.data()), str.length());
}

uint64_t FibonacciHash::hash(std::string_view str) {
    return fibonacci_hash_impl(reinterpret_cast<const uint8_t*>(str.data()), str.length());
}

uint64_t FibonacciHash::hash(const llvm::StringRef& str) {
    return fibonacci_hash_impl(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

uint64_t FibonacciHash::fibonacci_hash_impl(const uint8_t* data, size_t length) {
    uint64_t hash = 0x9E3779B97F4A7C15ULL; // Initial seed based on golden ratio
    
    // Process bytes while skipping whitespace
    uint64_t chunk = 0;
    int chunkPos = 0;
    
    for (size_t i = 0; i < length; ++i) {
        uint8_t byte = data[i];
        
        // Skip all whitespace characters (optimized with single comparison range check)
        // ASCII whitespace: space(32), tab(9), LF(10), VT(11), FF(12), CR(13)
        if (byte <= 32 && (byte == 32 || byte == 9 || (byte >= 10 && byte <= 13))) {
            continue;
        }
        
        // Add byte to current chunk in little-endian order (reproducible)
        chunk |= static_cast<uint64_t>(byte) << (chunkPos * 8);
        chunkPos++;
        
        // Process chunk when we have 8 bytes
        if (chunkPos == 8) {
            hash ^= chunk;
            hash *= GOLDEN_RATIO_64;
            hash ^= hash >> 32; // Avalanche effect
            
            chunk = 0;
            chunkPos = 0;
        }
    }
    
    // Process remaining bytes (0-7 bytes)
    if (chunkPos > 0) {
        hash ^= chunk;
        hash *= GOLDEN_RATIO_64;
        hash ^= hash >> 32;
    }
    
    // Final avalanche mixing for better distribution
    hash ^= hash >> 16;
    hash *= 0x85EBCA6B;
    hash ^= hash >> 13;
    hash *= 0xC2B2AE35;
    hash ^= hash >> 16;
    
    return hash;
}

uint64_t FibonacciHash::hashFromSourceRange(clang::SourceManager* SM, clang::SourceRange Range) {
    if (!SM || !Range.isValid()) return 0;
    
    clang::SourceLocation StartLoc = Range.getBegin();
    clang::SourceLocation EndLoc = Range.getEnd();
    
    if (!StartLoc.isValid() || !EndLoc.isValid()){
        TEST_LOG << "Invalid SourceRange\n";
        return 0;
    }
    
    unsigned startOffset = SM->getFileOffset(StartLoc);
    unsigned endOffset = SM->getFileOffset(EndLoc);

    if(startOffset == endOffset){
        TEST_LOG << "Get the actual end location (after the last token) startOffset == endOffset \n";
        EndLoc = clang::Lexer::getLocForEndOfToken(
        EndLoc, 0, *SM, clang::LangOptions());
        endOffset = SM->getFileOffset(EndLoc);
    }
    
    return hashFromOffsets(SM, startOffset, endOffset);
}

uint64_t FibonacciHash::hashFromOffsets(clang::SourceManager* SM, unsigned startOffset, unsigned endOffset) {
    if (!SM || startOffset >= endOffset){
        if(startOffset >= endOffset) TEST_LOG << "Error while computing Hash  startOffset >= endOffset \n";
        return 0;
    }
    
    clang::FileID FID = SM->getMainFileID();
    llvm::StringRef buffer = SM->getBufferData(FID);
    
    if (endOffset > buffer.size()){
        TEST_LOG << "Error while computing Hash\n";
        return 0;
    }
    
    // Hash tokens directly without building intermediate string
    uint64_t hash = 0x9E3779B97F4A7C15ULL; // Initial seed
    uint64_t chunk = 0;
    int chunkPos = 0;
    
    clang::SourceLocation currentLoc = SM->getLocForStartOfFile(FID).getLocWithOffset(startOffset);
    clang::Token tok;
    
    // Static LangOptions to avoid repeated construction
    static const clang::LangOptions langOpts;
    
    // Cache to avoid repeated getFileOffset calls
    unsigned currentOffset;
    
    while (currentLoc.isValid()) {
        currentOffset = SM->getFileOffset(currentLoc);
        if (currentOffset >= endOffset) break;
        
        // Try to get the next token
        if (clang::Lexer::getRawToken(currentLoc, tok, *SM, langOpts)) {
            currentLoc = currentLoc.getLocWithOffset(1);
            continue;
        }
        
        // Get token kind once
        clang::tok::TokenKind kind = tok.getKind();
        
        // Skip comments and EOF (combined check)
        if (kind == clang::tok::comment || kind == clang::tok::eof) {
            currentLoc = tok.getEndLoc();
            continue;
        }
        
        // Get token location and length
        clang::SourceLocation tokLoc = tok.getLocation();
        unsigned tokOffset = SM->getFileOffset(tokLoc);
        
        // Early exit if token is beyond range
        if (tokOffset >= endOffset) break;
        
        unsigned tokLen = tok.getLength();
        if (tokLen > 0) {
            // Get token data pointer
            bool invalid = false;
            const uint8_t* data = reinterpret_cast<const uint8_t*>(
                SM->getCharacterData(tokLoc, &invalid));
            
            if (!invalid) {
                // Truncate token if it exceeds range
                unsigned tokEndOffset = tokOffset + tokLen;
                if (tokEndOffset > endOffset) {
                    tokLen = endOffset - tokOffset;
                }
                
                // Hash token bytes with unrolled loop for better performance
                const uint8_t* end = data + tokLen;
                const uint8_t* ptr = data;
                
                // Process 8 bytes at a time when possible
                while (ptr + 8 <= end && chunkPos == 0) {
                    // Fast path: directly load 8 bytes when chunk is empty
                    chunk = static_cast<uint64_t>(ptr[0]) |
                           (static_cast<uint64_t>(ptr[1]) << 8) |
                           (static_cast<uint64_t>(ptr[2]) << 16) |
                           (static_cast<uint64_t>(ptr[3]) << 24) |
                           (static_cast<uint64_t>(ptr[4]) << 32) |
                           (static_cast<uint64_t>(ptr[5]) << 40) |
                           (static_cast<uint64_t>(ptr[6]) << 48) |
                           (static_cast<uint64_t>(ptr[7]) << 56);
                    
                    hash ^= chunk;
                    hash *= GOLDEN_RATIO_64;
                    hash ^= hash >> 32;
                    
                    ptr += 8;
                    chunk = 0;
                }
                
                // Process remaining bytes
                while (ptr < end) {
                    chunk |= static_cast<uint64_t>(*ptr) << (chunkPos * 8);
                    chunkPos++;
                    ptr++;
                    
                    if (chunkPos == 8) {
                        hash ^= chunk;
                        hash *= GOLDEN_RATIO_64;
                        hash ^= hash >> 32;
                        chunk = 0;
                        chunkPos = 0;
                    }
                }
                
                // Early exit if we've processed beyond range
                if (tokEndOffset > endOffset) break;
            }
        }
        
        // Move to end of current token
        currentLoc = tok.getEndLoc();
    }
    
    // Process remaining bytes (0-7 bytes)
    if (chunkPos > 0) {
        hash ^= chunk;
        hash *= GOLDEN_RATIO_64;
        hash ^= hash >> 32;
    }
    
    // Final avalanche mixing
    hash ^= hash >> 16;
    hash *= 0x85EBCA6B;
    hash ^= hash >> 13;
    hash *= 0xC2B2AE35;
    hash ^= hash >> 16;
    
    return hash;
}