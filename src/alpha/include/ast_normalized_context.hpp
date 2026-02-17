// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "node.hpp"
#include "clang/AST/ASTContext.h"
#include <llvm-14/llvm/ADT/SmallVector.h>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <llvm-14/llvm/ADT/StringSet.h>

namespace alpha{

/**
 * @class SourceRangeTracker
 * @brief Tracks fatal preprocessor directives found during AST traversal.
 *
 * This class manages a collection of fatal directive references that need to be
 * tracked for error reporting or analysis purposes.
 */
class SourceRangeTracker {
    /**
    * @struct FatalDirectiveRef
    * @brief References to a fatal preprocessor directive and its source file.
    */
    struct FatalDirectiveRef {
        std::string Header;  ///< The directive text (e.g., "#error message")
        std::string File;    ///< The source file containing the directive
        
        FatalDirectiveRef(llvm::StringRef header, llvm::StringRef file) : Header(header), File(file) {}
    };

    public:
    /**
     * @brief Constructs an empty SourceRangeTracker.
     */
    SourceRangeTracker() = default;

    /**
     * @brief Adds a fatal directive reference.
     * @param header The directive text.
     * @param file The source file path.
     */
    void addFatalDirective(llvm::StringRef header, llvm::StringRef file);

    /**
     * @brief Returns all fatal directive references.
     */
    const llvm::SmallVector<FatalDirectiveRef,8>& getFatalDirectives() const;

    /**
     * @brief Clears all tracked fatal directives.
     */
    void clear();

    /**
     * @brief Checks if the tracker is empty.
     * @return True if no fatal directives are tracked.
     */
    bool empty() const;

private:
    llvm::SmallVector<FatalDirectiveRef,8> fatalDirectives;
};

/**
 * @class ASTNormalizedContext
 * @brief Manages a collection of API nodes parsed from an Abstract Syntax Tree (AST).
 *
 * This class serves as the central repository for all unique API nodes found during
 * an AST traversal. It maintains two primary data structures:
 *
 * 1. A map (`normalizedTree`) from a unique identifier (like a USR) to the
 *    corresponding `APINode`. This ensures that each API entity is represented by
 *    a single, unique object, preventing duplication.
 *
 * 2. A vector (`rootApiNodes`) of nodes that are considered top-level or
 *    "root" elements of the API (e.g., free functions, global variables, or
 *    classes in the global namespace).
 */

class ASTNormalizedContext {
public:

    /**
     * @brief Constructs an empty ASTNormalizedContext.
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
    void addNode(llvm::StringRef key,const std::shared_ptr<const APINode> node);

    /**
     * @brief Adds a node to the list of root API nodes.
     *
     * @param rootNode A const shared pointer to the APINode.
     */
    void addRootNode(const std::shared_ptr<const APINode> rootNode);

    /**
     * @brief Returns a const reference to the entire normalized tree map.
     */
    const llvm::StringMap<std::shared_ptr<const APINode>>& getTree() const;

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
     * @brief Returns a reference to the source range tracker.
     */
    SourceRangeTracker& getSourceRangeTracker();

    /**
     * @brief Returns a const reference to the source range tracker.
     */
    const SourceRangeTracker& getSourceRangeTracker() const;

    llvm::StringSet<> excludeNodes;
    llvm::StringSet<> hashSet;

private:
    llvm::StringMap<std::shared_ptr<const APINode>> apiNodesMap;
    llvm::SmallVector<std::shared_ptr<const APINode>,64> apiNodes;

    SourceRangeTracker sourceRangeTracker;
    clang::ASTContext* clangContext;
};

}