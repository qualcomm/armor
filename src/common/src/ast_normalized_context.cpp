// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "node.hpp"
#include "ast_normalized_context.hpp"
#include <llvm-14/llvm/ADT/SmallVector.h>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <memory>
#include <utility>

// --- ASTNormalizedContext ---

armor::ASTNormalizedContext::ASTNormalizedContext() = default;

void armor::ASTNormalizedContext::addNode(llvm::StringRef key, std::shared_ptr<armor::APINode> node) {
    apiNodesMap[key].emplace_back(std::move(node));
}

void armor::ASTNormalizedContext::addRootNode(std::shared_ptr<const armor::APINode> rootNode) {
    if (rootNode) {
        apiNodes.push_back(std::move(rootNode));
    }
}

const llvm::StringMap<llvm::SmallVector<std::shared_ptr<armor::APINode>, 16>>&
armor::ASTNormalizedContext::getTree() const {
    return apiNodesMap;
}

const llvm::SmallVector<std::shared_ptr<const armor::APINode>, 64>&
armor::ASTNormalizedContext::getRootNodes() const {
    return apiNodes;
}

bool armor::ASTNormalizedContext::empty() const {
    return apiNodesMap.empty() && apiNodes.empty();
}

void armor::ASTNormalizedContext::clear() {
    apiNodesMap.clear();
    apiNodes.clear();
    sourceRangeTracker.clear();
}

void armor::ASTNormalizedContext::addClangASTContext(clang::ASTContext* ASTContext) {
    clangContext = ASTContext;
}

clang::ASTContext* armor::ASTNormalizedContext::getClangASTContext() const {
    return clangContext;
}

armor::SourceRangeTracker& armor::ASTNormalizedContext::getSourceRangeTracker() {
    return sourceRangeTracker;
}

const armor::SourceRangeTracker& armor::ASTNormalizedContext::getSourceRangeTracker() const {
    return sourceRangeTracker;
}

// --- SourceRangeTracker ---

void armor::SourceRangeTracker::addFatalDirective(llvm::StringRef header, llvm::StringRef file) {
    fatalDirectives.emplace_back(header, file);
}

const llvm::SmallVector<armor::SourceRangeTracker::FatalDirectiveRef, 8>&
armor::SourceRangeTracker::getFatalDirectives() const {
    return fatalDirectives;
}

const llvm::SmallVector<armor::Range, 32>& armor::SourceRangeTracker::getComments() const {
    return comments;
}

void armor::SourceRangeTracker::moveComments(llvm::SmallVector<armor::Range, 32>& ranges) {
    comments = std::move(ranges);
}

llvm::DenseMap<uint64_t, int>& armor::SourceRangeTracker::getCommentsHashMap() {
    return commentsHashMap;
}

const llvm::DenseMap<uint64_t, int>& armor::SourceRangeTracker::getCommentsHashMap() const {
    return commentsHashMap;
}

void armor::SourceRangeTracker::moveCommentsHashMap(llvm::DenseMap<uint64_t, int>& hashMap) {
    commentsHashMap = std::move(hashMap);
}

const std::map<unsigned, armor::Range>& armor::SourceRangeTracker::getInactivePPDirectives() const {
    return inactivePPDirectives;
}

std::map<unsigned, armor::Range>& armor::SourceRangeTracker::getInactivePPDirectives() {
    return inactivePPDirectives;
}

void armor::SourceRangeTracker::moveInactivePPDirectives(std::map<unsigned, armor::Range>& ranges) {
    inactivePPDirectives = std::move(ranges);
}

void armor::SourceRangeTracker::addUnhandledDeclHash(uint64_t hash) {
    unhandledDeclsHashMap[hash]++;
}

llvm::DenseMap<uint64_t, int>& armor::SourceRangeTracker::getUnhandledDeclsHashMap() {
    return unhandledDeclsHashMap;
}

const llvm::DenseMap<uint64_t, int>& armor::SourceRangeTracker::getUnhandledDeclsHashMap() const {
    return unhandledDeclsHashMap;
}

void armor::SourceRangeTracker::moveInactiveUnhandledDeclsHashMap(llvm::DenseMap<uint64_t, int>& hashMap) {
    inactiveUnhandledDeclsHashMap = std::move(hashMap);
}

llvm::DenseMap<uint64_t, int>& armor::SourceRangeTracker::getInactiveUnhandledDeclsHashMap() {
    return inactiveUnhandledDeclsHashMap;
}

const llvm::DenseMap<uint64_t, int>& armor::SourceRangeTracker::getInactiveUnhandledDeclsHashMap() const {
    return inactiveUnhandledDeclsHashMap;
}

void armor::SourceRangeTracker::clear() {
    fatalDirectives.clear();
    comments.clear();
    inactivePPDirectives.clear();
    unhandledDeclsHashMap.clear();
    inactiveUnhandledDeclsHashMap.clear();
    commentsHashMap.clear();
}

bool armor::SourceRangeTracker::empty() const {
    return fatalDirectives.empty() &&
           comments.empty() &&
           inactivePPDirectives.empty();
}
