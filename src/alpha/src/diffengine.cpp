// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include <iostream>
#include <llvm/ADT/SmallVector.h>

#include "diffengine.hpp"
#include "diff_utils.hpp"
#include "debug_config.hpp"

using json = nlohmann::json;

auto byHash = [](const std::shared_ptr<const alpha::APINode>& node) -> const std::string& {
    return node->hash;
};

template<typename KeyFunc>
std::vector<std::pair<std::shared_ptr<const alpha::APINode>, std::shared_ptr<const alpha::APINode>>> intersection(
    const llvm::SmallVector<std::shared_ptr<const alpha::APINode>, 16>& a,
    const llvm::SmallVector<std::shared_ptr<const alpha::APINode>, 16>& b,
    KeyFunc&& keyFunc
) {
    std::unordered_multimap<std::string, std::shared_ptr<const alpha::APINode>> map_b;
    map_b.reserve(b.size());
    for (const auto& node : b) {
        map_b.emplace(keyFunc(node), node);
    }

    std::vector<std::pair<std::shared_ptr<const alpha::APINode>, std::shared_ptr<const alpha::APINode>>> result;
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
std::vector<std::shared_ptr<const alpha::APINode>> difference(
    const llvm::SmallVector<std::shared_ptr<const alpha::APINode>, 16>& a,
    const llvm::SmallVector<std::shared_ptr<const alpha::APINode>, 16>& b,
    KeyFunc&& keyFunc
) {
    std::unordered_multimap<std::string, std::shared_ptr<const alpha::APINode>> map_b;
    map_b.reserve(b.size());
    for (const auto& node : b) {
        map_b.emplace(keyFunc(node), node);
    }

    std::vector<std::shared_ptr<const alpha::APINode>> result;
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

const bool inline checkLayoutChange(const  std::shared_ptr<const alpha::APINode> node){
    return node->kind != NodeKind::Enum;
}

const bool inline hasChildren(const std::shared_ptr<const alpha::APINode>& node) {
    return node->children == nullptr ? false : !node->children->empty();
}

namespace{
    const json toJson(const std::shared_ptr<const alpha::APINode>& node) {
    
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

    const json get_json_from_node(const std::shared_ptr<const alpha::APINode> node, const std::string& tag) {
        json json_node = toJson(node);
        json_node[TAG] = tag;
        return json_node;
    }
}


json diffNodes(
    const std::shared_ptr<const alpha::APINode>& a, 
    const std::shared_ptr<const alpha::APINode>& b
){
    
    // Any node can have children.

    if ( hasChildren(a) && hasChildren(b) ) {
        
        json childrenDiff = json::array();

        const std::vector<std::shared_ptr<const alpha::APINode>> removed_nodes = difference(
            *a->children, 
            *b->children,
            byHash
        );
        const std::vector<std::shared_ptr<const alpha::APINode>> added_nodes = difference(
            *b->children, 
            *a->children,
            byHash
        );

        const std::vector<std::pair<std::shared_ptr<const alpha::APINode>, std::shared_ptr<const alpha::APINode>>> common_nodes = intersection(
            *a->children, 
            *b->children, 
            byHash
        );

        for (const auto& removedNode : removed_nodes) {
            childrenDiff.emplace_back(get_json_from_node(removedNode, REMOVED));
        }

        for (const auto& addedNode : added_nodes) {
            childrenDiff.emplace_back(get_json_from_node(addedNode, ADDED));
        }

        for (const auto& commonNodePair : common_nodes) {
            /*
                Comparing nodes of same scope. No name conflicts for const alpha::APINodes in same scope.
                Here scope can be Main Header file or inside a CXXRecordDecl, EnumDecl, FunctionDecl
            */
            json sameScopeDiff = diffNodes(commonNodePair.first,commonNodePair.second);
            
            if(!sameScopeDiff.is_null() && !sameScopeDiff.empty()){
                if(sameScopeDiff.is_array()){
                    childrenDiff.insert(childrenDiff.end(), sameScopeDiff.begin(), sameScopeDiff.end());
                }
                else childrenDiff.emplace_back(sameScopeDiff);
            }
        }

        // For functions,  we check return type and for other future use-cases.
    
        json APINodeDiff = a->diff(b);
        if (!APINodeDiff.empty()) {
            if(APINodeDiff.is_array()){
                childrenDiff.insert(childrenDiff.end(), APINodeDiff.begin(), APINodeDiff.end());
            }
            else childrenDiff.emplace_back(APINodeDiff);
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
    const alpha::ASTNormalizedContext* context1,
    const alpha::ASTNormalizedContext* context2
) {

    json diffs = json::array();
    llvm::StringMap<std::shared_ptr<const alpha::APINode>> tree1 = context1->getTree();
    llvm::StringMap<std::shared_ptr<const alpha::APINode>> tree2 = context2->getTree();

    for (auto const &rootNode1 : context1->getRootNodes()) {

        if(context1->excludeNodes.count(rootNode1->hash) || context2->excludeNodes.count(rootNode1->hash)){
            DebugConfig::instance().log("Excluding : " + rootNode1->hash, DebugConfig::Level::INFO);
            continue;
        }

        if (tree2.find(rootNode1->hash) == tree2.end()) {
            diffs.emplace_back(get_json_from_node(rootNode1, REMOVED));
        }
        else {
            const std::shared_ptr<const alpha::APINode> rootNode2 = tree2.find(rootNode1->hash)->second;
            json sameScopeDiff = diffNodes(rootNode1, rootNode2);
            /*
                Comparing nodes of same scope. No name conflicts for alpha::APINodes in same scope.
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

    for (const auto & rootNode2 : context2->getRootNodes()) {

        if(context1->excludeNodes.count(rootNode2->hash) || context2->excludeNodes.count(rootNode2->hash)){
            DebugConfig::instance().log("Excluding : " + rootNode2->hash, DebugConfig::Level::INFO);
            continue;
        }

        if (tree1.find(rootNode2->hash) == tree1.end()) {
            diffs.emplace_back(get_json_from_node(rootNode2, ADDED));
        }
    }

    return diffs;
}