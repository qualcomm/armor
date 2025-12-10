// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "node.hpp"
#include "clang/AST/ASTContext.h"
#include <cstddef>
#include <llvm-14/llvm/ADT/SmallVector.h>
#include <llvm-14/llvm/ADT/StringMap.h>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <llvm-14/llvm/ADT/StringSet.h>
#include <memory>

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
namespace beta{

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
     * @return True if the node was inserted, false if a node with that key already existed.
     */
    void addNode(llvm::StringRef key,const std::shared_ptr<APINode> node);

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

    llvm::StringSet<> excludeNodes;
    llvm::StringMap<std::shared_ptr<APINode>> usrNodeMap;

private:
    llvm::StringMap<llvm::SmallVector<std::shared_ptr<APINode>,16>> apiNodesMap;
    llvm::SmallVector<std::shared_ptr<const APINode>,64> apiNodes;

    clang::ASTContext* clangContext;
};

}