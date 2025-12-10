// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include "node.hpp"
#include "ast_normalized_context.hpp"
#include <llvm-14/llvm/ADT/SmallVector.h>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <memory>
#include <utility>

beta::ASTNormalizedContext::ASTNormalizedContext() = default;

void beta::ASTNormalizedContext::addNode(llvm::StringRef key, std::shared_ptr<beta::APINode> node) {
    apiNodesMap[key].emplace_back(std::move(node));
}

void beta::ASTNormalizedContext::addRootNode(std::shared_ptr<const beta::APINode> rootNode) {
    if (rootNode) {
        apiNodes.push_back(std::move(rootNode));
    }
}

const llvm::StringMap<llvm::SmallVector<std::shared_ptr<beta::APINode>,16>>& beta::ASTNormalizedContext::getTree() const {
    return apiNodesMap;
}

const llvm::SmallVector<std::shared_ptr<const beta::APINode>,64>& beta::ASTNormalizedContext::getRootNodes() const {
    return apiNodes;
}

bool beta::ASTNormalizedContext::empty() const {
    return apiNodesMap.empty() && apiNodes.empty();
}

void beta::ASTNormalizedContext::clear() {
    apiNodesMap.clear();
    apiNodes.clear();
}

void beta::ASTNormalizedContext::addClangASTContext(clang::ASTContext *ASTContext){
    clangContext = ASTContext;
}

clang::ASTContext* beta::ASTNormalizedContext::getClangASTContext() const { 
    return clangContext; 
}