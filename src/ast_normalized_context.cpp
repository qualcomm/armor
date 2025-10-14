// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "node.hpp"
#include "ast_normalized_context.hpp"

ASTNormalizedContext::ASTNormalizedContext() = default;

bool ASTNormalizedContext::addNode(const std::string& key, std::shared_ptr<APINode> node) {
    return m_normalizedTree.try_emplace(key, std::move(node)).second;
}

void ASTNormalizedContext::addOrUpdateNode(const std::string& key, std::shared_ptr<APINode> node) {
    m_normalizedTree[key] = std::move(node);
}

std::shared_ptr<APINode> ASTNormalizedContext::getNode(const std::string& key) const {
    auto it = m_normalizedTree.find(key);
    if (it != m_normalizedTree.end()) {
        return it->getValue();
    }
    return nullptr;
}

void ASTNormalizedContext::addRootNode(std::shared_ptr<const APINode> rootNode) {
    if (rootNode) {
        m_rootApiNodes.push_back(std::move(rootNode));
    }
}

const normalizedTree& ASTNormalizedContext::getTree() const {
    return m_normalizedTree;
}

const rootApiNodes& ASTNormalizedContext::getRootNodes() const {
    return m_rootApiNodes;
}

bool ASTNormalizedContext::empty() const {
    return m_normalizedTree.empty() && m_rootApiNodes.empty();
}

void ASTNormalizedContext::clear() {
    m_normalizedTree.clear();
    m_rootApiNodes.clear();
}

void ASTNormalizedContext::addClangASTContext(clang::ASTContext *ASTContext){
    clangContext = ASTContext;
}

clang::ASTContext* ASTNormalizedContext::getClangASTContext() const { 
    return clangContext; 
}