// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "node.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceLocation.h"
#include <cstddef>
#include <cstdint>
#include <llvm-14/llvm/ADT/DenseMap.h>
#include <llvm-14/llvm/ADT/SmallString.h>
#include <llvm-14/llvm/ADT/SmallVector.h>
#include <llvm-14/llvm/ADT/StringMap.h>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <llvm-14/llvm/ADT/StringSet.h>
#include <map>
#include <memory>
#include <utility>

namespace beta{

/**
 * @class SourceRangeTracker
 * @brief Tracks various source ranges found during AST traversal.
 *
 * ⚠️ IMPORTANT LIFETIME CONSTRAINT:
 * SourceRange objects stored in this tracker are ONLY VALID while the
 * ASTContext is alive. After AST destruction, accessing SourceRange objects
 * results in undefined behavior. All processing using SourceRange objects
 * must complete before the AST is destroyed.
 *
 * Hash values (unhandledDeclsHash, inactiveUnhandledDeclsHash) remain
 * valid after AST destruction and can be safely used for comparison.
 *
 * This class manages collections of source ranges for different categories:
 * - Comments
 * - Preprocessor directives (active and inactive)
 * - Unhandled declarations
 *
 * Each category maintains both ranges (valid during AST lifetime) and
 * hash maps (valid after AST destruction) for efficient deduplication.
 */
class SourceRangeTracker {
public:
    /**
     * @brief Constructs an empty SourceRangeTracker.
     */
    SourceRangeTracker() = default;

    /**
     * @brief Returns all comment source ranges.
     */
    const llvm::SmallVector<beta::Range, 32>& getComments() const;

    /**
     * @brief Returns all inactive preprocessor directive source ranges.
     */
    const std::map<unsigned, beta::Range>& getInactivePPDirectives() const;

    std::map<unsigned, beta::Range>& getInactivePPDirectives();

    void moveInactivePPDirectives(std::map<unsigned int, beta::Range>& map);

    void moveComments(llvm::SmallVector<beta::Range, 32>& ranges);

    /**
     * @brief Moves inactive unhandled declarations hash map into the tracker.
     * @param hashMap The hash map to move from.
     */
    void moveInactiveUnhandledDeclsHashMap(llvm::DenseMap<uint64_t, int>& hashMap);

    /**
     * @brief Moves comments hash map into the tracker.
     * @param hashMap The hash map to move from.
     */
    void moveCommentsHashMap(llvm::DenseMap<uint64_t, int>& hashMap);

    /**
     * @brief Adds a hash to the unhandled declarations hash map.
     * @param hash The hash value to add/increment.
     */
    void addUnhandledDeclHash(uint64_t hash);

    /**
     * @brief Returns all unhandled declaration hashes (mutable).
     */
    llvm::DenseMap<uint64_t, int>& getUnhandledDeclsHashMap();

    /**
     * @brief Returns all unhandled declaration hashes (const).
     */
    const llvm::DenseMap<uint64_t, int>& getUnhandledDeclsHashMap() const;

    /**
     * @brief Returns all inactive unhandled declaration hashes (mutable).
     */
    llvm::DenseMap<uint64_t, int>& getInactiveUnhandledDeclsHashMap();

    /**
     * @brief Returns all inactive unhandled declaration hashes (const).
     */
    const llvm::DenseMap<uint64_t, int>& getInactiveUnhandledDeclsHashMap() const;

    /**
     * @brief Returns all comments hashes (mutable).
     */
    llvm::DenseMap<uint64_t, int>& getCommentsHashMap();

    /**
     * @brief Returns all comments hashes (const).
     */
    const llvm::DenseMap<uint64_t, int>& getCommentsHashMap() const;

    /**
     * @brief Clears all tracked source ranges.
     */
    void clear();

    /**
     * @brief Checks if the tracker is empty.
     * @return True if no source ranges are tracked.
     */
    bool empty() const;

private:
    llvm::SmallVector<beta::Range, 32> comments;
    std::map<unsigned,beta::Range> inactivePPDirectives;

    llvm::DenseMap<uint64_t, int> unhandledDeclsHashMap;
    llvm::DenseMap<uint64_t, int> commentsHashMap;
    llvm::DenseMap<uint64_t, int> inactiveUnhandledDeclsHashMap;
};

/**
 * @class ASTNormalizedContext
 * @brief Manages a collection of API nodes parsed from an Abstract Syntax Tree (AST).
 *
 * This class serves as the central repository for all unique API nodes found during
 * an AST traversal. It maintains two primary data structures:
 *
 * 1. A map (`apiNodesMap`) from a unique identifier (like a USR) to the
 *    corresponding `APINode`. This ensures that each API entity is represented by
 *    a single, unique object, preventing duplication.
 *
 * 2. A vector (`apiNodes`) of nodes that are considered top-level or
 *    "root" elements of the API (e.g., free functions, global variables, or
 *    classes in the global namespace).
 */
class ASTNormalizedContext {
public:

    /**
     * @brief Constructs an empty ASTNormalizedContext.
     * Reserves space for the usrNodeMap.
     */
    ASTNormalizedContext();

    /**
     * @brief Adds a new node to the normalized tree.
     *
     * If a node with the same key already exists, it is not replaced.
     * Use addOrUpdateNode if overwriting is desired.
     *
     * @param key The unique string identifier for the node (e.g., USR).
     * @param node A shared pointer to the APINode.
     */
    void addNode(llvm::StringRef key, const std::shared_ptr<APINode> node);

    /**
     * @brief Adds a node to the list of root API nodes.
     *
     * @param rootNode A const shared pointer to the APINode.
     */
    void addRootNode(const std::shared_ptr<const APINode> rootNode);

    /**
     * @brief Returns a const reference to the entire normalized tree map.
     */
    const llvm::StringMap<llvm::SmallVector<std::shared_ptr<APINode>,16>>& getTree() const;

    /**
     * @brief Returns a const reference to the list of root API nodes.
     */
    const llvm::SmallVector<std::shared_ptr<const APINode>,64>& getRootNodes() const;

    /**
     * @brief Checks if the context contains any nodes.
     * @return True if both the tree and root nodes list are empty, false otherwise.
     */
    bool empty() const;

    /**
     * @brief Clears all stored nodes, resetting the context to an empty state.
     */
    void clear();

    void addClangASTContext(clang::ASTContext *ASTContext);

    clang::ASTContext* getClangASTContext() const;

    /**
     * @brief Returns a reference to the source range tracker (mutable).
     */
    SourceRangeTracker& getSourceRangeTracker();

    /**
     * @brief Returns a const reference to the source range tracker.
     */
    const SourceRangeTracker& getSourceRangeTracker() const;

    llvm::StringMap<std::shared_ptr<APINode>> usrNodeMap;
    llvm::StringSet<> unSupportedUsrNodeMap;

private:
    llvm::StringMap<llvm::SmallVector<std::shared_ptr<APINode>,16>> apiNodesMap;
    llvm::SmallVector<std::shared_ptr<const APINode>,64> apiNodes;

    SourceRangeTracker sourceRangeTracker;
    clang::ASTContext* clangContext;
};

}