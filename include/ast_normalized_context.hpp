// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "node.hpp"
#include "clang/AST/ASTContext.h"
#include <llvm-14/llvm/ADT/StringSet.h>

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
     * @return True if the node was inserted, false if a node with that key already existed.
     */
    bool addNode(const std::string& key,const std::shared_ptr<APINode> node);

    /**
     * @brief Adds or updates a node in the normalized tree.
     *
     * @param key The unique string identifier for the node (e.g., USR).
     * @param node A shared pointer to the APINode.
     */
    void addOrUpdateNode(const std::string& key,const std::shared_ptr<APINode> node);


    /**
     * @brief Retrieves a node from the normalized tree by its key.
     *
     * @param key The unique string identifier for the node.
     * @return A shared_ptr to the APINode if found, otherwise an empty shared_ptr.
     */
    std::shared_ptr<APINode> getNode(const std::string& key) const;

    /**
     * @brief Adds a node to the list of root API nodes.
     *
     * @param rootNode A const shared pointer to the APINode.
     */
    void addRootNode(const std::shared_ptr<const APINode> rootNode);

    /**
     * @brief Returns a const reference to the entire normalized tree map.
     */
    const normalizedTree& getTree() const;

    /**
     * @brief Returns a const reference to the list of root API nodes.
     */
    const rootApiNodes& getRootNodes() const;

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

private:
    normalizedTree m_normalizedTree;
    rootApiNodes m_rootApiNodes;

    clang::ASTContext* clangContext;
};
