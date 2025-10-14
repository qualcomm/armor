// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <llvm/ADT/SmallVector.h>

#include "diffengine.hpp"
#include "diff_utils.hpp"
#include "debug_config.hpp"

using json = nlohmann::json;

auto byQualifiedName = [](const std::shared_ptr<const APINode>& node) -> const std::string& {
    return node->qualifiedName;
};

auto byDatatype =  [](const std::shared_ptr<const APINode>& node)  -> const std::string& {
    return node->dataType;
};


template<typename KeyFunc>
std::vector<std::pair<std::shared_ptr<const APINode>, std::shared_ptr<const APINode>>> intersection(
    const llvm::SmallVector<std::shared_ptr<const APINode>, 16>& a,
    const llvm::SmallVector<std::shared_ptr<const APINode>, 16>& b,
    KeyFunc&& keyFunc
) {
    std::unordered_multimap<std::string, std::shared_ptr<const APINode>> map_b;
    map_b.reserve(b.size());
    for (const auto& node : b) {
        map_b.emplace(keyFunc(node), node);
    }

    std::vector<std::pair<std::shared_ptr<const APINode>, std::shared_ptr<const APINode>>> result;
    result.reserve(std::min(a.size(), b.size()));

    for (const auto& node_a : a) {
        const auto& key = keyFunc(node_a);
        auto it = map_b.find(key);
        if (it != map_b.end()) {
            result.emplace_back(node_a, std::move(it->second));
            map_b.erase(it);
        }
    }

    return result;

}

template<typename KeyFunc>
std::vector<std::shared_ptr<const APINode>> difference(
    const llvm::SmallVector<std::shared_ptr<const APINode>, 16>& a,
    const llvm::SmallVector<std::shared_ptr<const APINode>, 16>& b,
    KeyFunc&& keyFunc
) {
    std::unordered_multimap<std::string, std::shared_ptr<const APINode>> map_b;
    map_b.reserve(b.size());
    for (const auto& node : b) {
        map_b.emplace(keyFunc(node), node);
    }

    std::vector<std::shared_ptr<const APINode>> result;
    result.reserve(std::min(a.size(), b.size()));

    for (const auto& node : a) {
        const auto& key = keyFunc(node);
        auto it = map_b.find(key);
        if (it != map_b.end()) {
            map_b.erase(it);  // remove one occurrence
        } else {
            result.emplace_back(node);
        }
    }

    return result;
}

const auto getKeyExtractor =  [](const std::shared_ptr<const APINode>& node) -> decltype(auto) {
    return (node->kind == NodeKind::Function) ? byDatatype  : byQualifiedName;
};

const bool inline checkLayoutChange(const  std::shared_ptr<const APINode> node){
    return node->kind != NodeKind::Enum;
}

const bool inline hasChildren(const std::shared_ptr<const APINode>& node) {
    return node->children == nullptr ? false : !node->children->empty();
}

const json toJson(const std::shared_ptr<const APINode>& node) {
    
    json json_node;
    
    if(!node->qualifiedName.empty()) json_node[QUALIFIED_NAME] = node->qualifiedName;    
    json_node[NODE_TYPE] = serialize(node->kind);

    if(hasChildren(node)) {
        json_node[CHILDREN] = json::array();
        for (const auto& childNode : *node->children) {
            json_node[CHILDREN].emplace_back(toJson(childNode));
        }
    }
    
    if(!node->dataType.empty()) json_node[DATA_TYPE] = node->dataType;

    return json_node;
}

const json get_json_from_node(const std::shared_ptr<const APINode> node, const std::string& tag) {
    json json_node = toJson(node);
    json_node[TAG] = tag;
    return json_node;
}


json diffNodes(
    const std::shared_ptr<const APINode>& a, 
    const std::shared_ptr<const APINode>& b, 
    const normalizedTree& tree1, 
    const normalizedTree& tree2) // we are not using the map for now.
{
    
    // Any node can have children.

    if ( hasChildren(a) && hasChildren(b) ) {
        
        json childrenDiff = json::array();

        const std::vector<std::shared_ptr<const APINode>> removed_nodes = difference(
            *a->children, 
            *b->children,
            byQualifiedName
        );
        const std::vector<std::shared_ptr<const APINode>> added_nodes = difference(
            *b->children, 
            *a->children,
            byQualifiedName
        );

        const std::vector<std::pair<std::shared_ptr<const APINode>, std::shared_ptr<const APINode>>> common_nodes = intersection(
            *a->children, 
            *b->children, 
            byQualifiedName
        );

        for (const auto& removedNode : removed_nodes) {
            childrenDiff.emplace_back(get_json_from_node(removedNode, REMOVED));
        }

        for (const auto& addedNode : added_nodes) {
            childrenDiff.emplace_back(get_json_from_node(addedNode, ADDED));
        }

        for (const auto& commonNodePair : common_nodes) {
            /*
                Comparing nodes of same scope. No name conflicts for const APINodes in same scope.
                Here scope can be Main Header file or inside a CXXRecordDecl, EnumDecl, FunctionDecl
            */
            json sameScopeDiff = diffNodes(commonNodePair.first,commonNodePair.second,tree1,tree2);
            
            if(!sameScopeDiff.is_null() && !sameScopeDiff.empty()){
                if(sameScopeDiff.is_array()){
                    childrenDiff.insert(childrenDiff.end(), sameScopeDiff.begin(), sameScopeDiff.end());
                }
                else childrenDiff.emplace_back(sameScopeDiff);
            }
        }

        // For functions,  we check return type and for other future use-cases.
    
        json apiNodeDiff = a->diff(b);
        if (!apiNodeDiff.empty()) {
            if(apiNodeDiff.is_array()){
                childrenDiff.insert(childrenDiff.end(), apiNodeDiff.begin(), apiNodeDiff.end());
            }
            else childrenDiff.emplace_back(apiNodeDiff);
        }

        if(!childrenDiff.empty()){
            json diff;
            diff[QUALIFIED_NAME] = a->qualifiedName;
            diff[NODE_TYPE] = serialize(a->kind);
            if(!childrenDiff.empty()) diff[CHILDREN] = childrenDiff;
            diff[TAG] = MODIFIED;
            return json::array().emplace_back(diff);
        }

    }
    else return a->diff(b);

    return json::array();
    
}


json diffTrees(
    const rootApiNodes& roots1, 
    const rootApiNodes& roots2,
    const normalizedTree& tree1, 
    const normalizedTree& tree2,
    const llvm::StringSet<>& excludeNodes1,
    const llvm::StringSet<>& excludeNodes2
) {
    
    json diffs = json::array();

    for (auto const &rootNode1 : roots1) {

        if(excludeNodes1.count(rootNode1->qualifiedName)){
            DebugConfig::instance().log("Excluding : " + rootNode1->qualifiedName, DebugConfig::Level::INFO);
            continue;
        }

        if (tree2.find(rootNode1->qualifiedName) == tree2.end()) {
            diffs.emplace_back(get_json_from_node(rootNode1, REMOVED));
        } 
        else {
            const std::shared_ptr<const APINode> rootNode2 = tree2.find(rootNode1->qualifiedName)->second;
            json sameScopeDiff = diffNodes(rootNode1, rootNode2, tree1, tree2);
            /*
                Comparing nodes of same scope. No name conflicts for APINodes in same scope.
                Here scope can be Main Header file or inside a CXXRecordDecl
            */ 
            if (!sameScopeDiff.is_null() && !sameScopeDiff.empty()){
                if (sameScopeDiff.is_array()){ 
                    diffs.insert(diffs.end(), sameScopeDiff.begin(), sameScopeDiff.end());
                }
                else diffs.emplace_back(sameScopeDiff);
            }
        }

    }

    for (const auto & rootNode2 : roots2) {

        if(excludeNodes2.count(rootNode2->qualifiedName)){
            DebugConfig::instance().log("Excluding : " + rootNode2->qualifiedName, DebugConfig::Level::INFO);
            continue;
        }

        if (tree1.find(rootNode2->qualifiedName) == tree1.end()) {
            diffs.emplace_back(get_json_from_node(rootNode2, ADDED));
        }
    }

    return diffs;
}