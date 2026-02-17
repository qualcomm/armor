// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "node.hpp"
#include "ast_normalized_context.hpp"

alpha::ASTNormalizedContext::ASTNormalizedContext() = default;

void alpha::ASTNormalizedContext::addNode(llvm::StringRef key, std::shared_ptr<const alpha::APINode> node) {
    apiNodesMap.try_emplace(key, std::move(node));
}

void alpha::ASTNormalizedContext::addRootNode(std::shared_ptr<const alpha::APINode> rootNode) {
    if (rootNode) {
        apiNodes.emplace_back(std::move(rootNode));
    }
}

const llvm::StringMap<std::shared_ptr<const alpha::APINode>>& alpha::ASTNormalizedContext::getTree() const {
    return apiNodesMap;
}

const llvm::SmallVector<std::shared_ptr<const alpha::APINode>,64>& alpha::ASTNormalizedContext::getRootNodes() const {
    return apiNodes;
}

bool alpha::ASTNormalizedContext::empty() const {
    return apiNodesMap.empty() && apiNodes.empty();
}

void alpha::ASTNormalizedContext::clear() {
    apiNodesMap.clear();
    apiNodes.clear();
    sourceRangeTracker.clear();
}

void alpha::ASTNormalizedContext::addClangASTContext(clang::ASTContext *ASTContext){
    clangContext = ASTContext;
}

clang::ASTContext* alpha::ASTNormalizedContext::getClangASTContext() const { 
    return clangContext; 
}

alpha::SourceRangeTracker& alpha::ASTNormalizedContext::getSourceRangeTracker() {
    return sourceRangeTracker;
}

const alpha::SourceRangeTracker& alpha::ASTNormalizedContext::getSourceRangeTracker() const {
    return sourceRangeTracker;
}

void alpha::SourceRangeTracker::addFatalDirective(llvm::StringRef header, llvm::StringRef file) {
    fatalDirectives.emplace_back(header, file);
}

const llvm::SmallVector<alpha::SourceRangeTracker::FatalDirectiveRef,8>& alpha::SourceRangeTracker::getFatalDirectives() const {
    return fatalDirectives;
}

void alpha::SourceRangeTracker::clear() {
    fatalDirectives.clear();
}

bool alpha::SourceRangeTracker::empty() const {
    return fatalDirectives.empty();
}