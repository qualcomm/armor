// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "node.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceLocation.h"
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

namespace armor {

/**
 * @class SourceRangeTracker
 * @brief Unified tracker for source range information from both parsers.
 *
 * Alpha uses only the fatal directives interface.
 * Beta uses the full interface (comments, inactive PP directives, hash maps).
 */
class SourceRangeTracker {
public:
    SourceRangeTracker() = default;

    struct FatalDirectiveRef {
        std::string Header;
        std::string File;
        FatalDirectiveRef(llvm::StringRef header, llvm::StringRef file)
            : Header(header), File(file) {}
    };

    // --- Alpha: fatal include directives ---
    void addFatalDirective(llvm::StringRef header, llvm::StringRef file);
    const llvm::SmallVector<FatalDirectiveRef, 8>& getFatalDirectives() const;

    // --- Beta: comments ---
    const llvm::SmallVector<armor::Range, 32>& getComments() const;
    void moveComments(llvm::SmallVector<armor::Range, 32>& ranges);
    llvm::DenseMap<uint64_t, int>& getCommentsHashMap();
    const llvm::DenseMap<uint64_t, int>& getCommentsHashMap() const;
    void moveCommentsHashMap(llvm::DenseMap<uint64_t, int>& hashMap);

    // --- Beta: inactive PP directives ---
    const std::map<unsigned, armor::Range>& getInactivePPDirectives() const;
    std::map<unsigned, armor::Range>& getInactivePPDirectives();
    void moveInactivePPDirectives(std::map<unsigned, armor::Range>& map);

    // --- Beta: unhandled decl hashes ---
    void addUnhandledDeclHash(uint64_t hash);
    llvm::DenseMap<uint64_t, int>& getUnhandledDeclsHashMap();
    const llvm::DenseMap<uint64_t, int>& getUnhandledDeclsHashMap() const;
    void moveInactiveUnhandledDeclsHashMap(llvm::DenseMap<uint64_t, int>& hashMap);
    llvm::DenseMap<uint64_t, int>& getInactiveUnhandledDeclsHashMap();
    const llvm::DenseMap<uint64_t, int>& getInactiveUnhandledDeclsHashMap() const;

    void clear();
    bool empty() const;

private:
    llvm::SmallVector<FatalDirectiveRef, 8> fatalDirectives;

    llvm::SmallVector<armor::Range, 32> comments;
    std::map<unsigned, armor::Range> inactivePPDirectives;
    llvm::DenseMap<uint64_t, int> unhandledDeclsHashMap;
    llvm::DenseMap<uint64_t, int> commentsHashMap;
    llvm::DenseMap<uint64_t, int> inactiveUnhandledDeclsHashMap;
};

/**
 * @class ASTNormalizedContext
 * @brief Unified context for both alpha (parse-only) and beta (full tree) parsers.
 *
 * Alpha uses only: addClangASTContext, getSourceRangeTracker (fatal directives).
 * Beta uses the full interface: node maps, root nodes, source range tracker.
 */
class ASTNormalizedContext {
public:
    ASTNormalizedContext();

    // Node management (beta only)
    void addNode(llvm::StringRef key, const std::shared_ptr<APINode> node);
    void addRootNode(const std::shared_ptr<const APINode> rootNode);
    const llvm::StringMap<llvm::SmallVector<std::shared_ptr<APINode>, 16>>& getTree() const;
    const llvm::SmallVector<std::shared_ptr<const APINode>, 64>& getRootNodes() const;

    bool empty() const;
    void clear();

    void addClangASTContext(clang::ASTContext* ASTContext);
    clang::ASTContext* getClangASTContext() const;

    SourceRangeTracker& getSourceRangeTracker();
    const SourceRangeTracker& getSourceRangeTracker() const;

    // Beta-specific lookup maps
    llvm::StringMap<std::shared_ptr<APINode>> usrNodeMap;
    llvm::StringSet<> unSupportedUsrNodeMap;

private:
    llvm::StringMap<llvm::SmallVector<std::shared_ptr<APINode>, 16>> apiNodesMap;
    llvm::SmallVector<std::shared_ptr<const APINode>, 64> apiNodes;
    SourceRangeTracker sourceRangeTracker;
    clang::ASTContext* clangContext = nullptr;
};

} // namespace armor
