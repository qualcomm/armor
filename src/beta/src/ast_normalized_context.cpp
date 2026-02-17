// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "node.hpp"
#include "ast_normalized_context.hpp"
#include <llvm-14/llvm/ADT/SmallVector.h>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <llvm-14/llvm/Support/raw_ostream.h>
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
    sourceRangeTracker.clear();
}

void beta::ASTNormalizedContext::addClangASTContext(clang::ASTContext *ASTContext){
    clangContext = ASTContext;
}

clang::ASTContext* beta::ASTNormalizedContext::getClangASTContext() const { 
    return clangContext; 
}

const llvm::SmallVector<beta::Range,32>& beta::SourceRangeTracker::getComments() const {
    return comments;
}

const std::map<unsigned, beta::Range>& beta::SourceRangeTracker::getInactivePPDirectives() const {
    return inactivePPDirectives;
}

std::map<unsigned, beta::Range>& beta::SourceRangeTracker::getInactivePPDirectives(){
    return inactivePPDirectives;
}

void beta::SourceRangeTracker::moveInactivePPDirectives(std::map<unsigned, beta::Range>& ranges) {
    inactivePPDirectives = std::move(ranges);
}

void beta::SourceRangeTracker::moveComments(llvm::SmallVector<beta::Range, 32> &ranges){
    comments = std::move(ranges);
}

void beta::SourceRangeTracker::moveInactiveUnhandledDeclsHashMap(llvm::DenseMap<uint64_t, int>& hashMap) {
    inactiveUnhandledDeclsHashMap = std::move(hashMap);
}

void beta::SourceRangeTracker::moveCommentsHashMap(llvm::DenseMap<uint64_t, int>& hashMap) {
    commentsHashMap = std::move(hashMap);
}

llvm::DenseMap<uint64_t, int>& beta::SourceRangeTracker::getUnhandledDeclsHashMap() {
    return unhandledDeclsHashMap;
}

void beta::SourceRangeTracker::addUnhandledDeclHash(uint64_t hash) {
    unhandledDeclsHashMap[hash]++;
}

const llvm::DenseMap<uint64_t, int>& beta::SourceRangeTracker::getUnhandledDeclsHashMap() const {
    return unhandledDeclsHashMap;
}

llvm::DenseMap<uint64_t, int>& beta::SourceRangeTracker::getInactiveUnhandledDeclsHashMap() {
    return inactiveUnhandledDeclsHashMap;
}

const llvm::DenseMap<uint64_t, int>& beta::SourceRangeTracker::getInactiveUnhandledDeclsHashMap() const {
    return inactiveUnhandledDeclsHashMap;
}

llvm::DenseMap<uint64_t, int>& beta::SourceRangeTracker::getCommentsHashMap(){
    return commentsHashMap;
}

const llvm::DenseMap<uint64_t, int>& beta::SourceRangeTracker::getCommentsHashMap() const {
    return commentsHashMap;
}

void beta::SourceRangeTracker::clear() {
    comments.clear();
    inactivePPDirectives.clear();
    unhandledDeclsHashMap.clear();
    inactiveUnhandledDeclsHashMap.clear();
}

bool beta::SourceRangeTracker::empty() const {
    return comments.empty() && 
           inactivePPDirectives.empty();
}

beta::SourceRangeTracker& beta::ASTNormalizedContext::getSourceRangeTracker() {
    return sourceRangeTracker;
}

const beta::SourceRangeTracker& beta::ASTNormalizedContext::getSourceRangeTracker() const {
    return sourceRangeTracker;
}