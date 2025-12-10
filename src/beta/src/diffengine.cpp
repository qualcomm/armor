// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include <cassert>
#include <cstddef>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallVector.h>
#include <utility>

#include "diffengine.hpp"
#include "diff_utils.hpp"
#include "debug_config.hpp"
#include "node.hpp"

using json = nlohmann::json;

const bool inline hasChildren(const std::shared_ptr<const beta::APINode>& node) {
    return node->children == nullptr ? false : !node->children->empty();
}

namespace{
    const json toJson(const std::shared_ptr<const beta::APINode>& node) {
    
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

    const json get_json_from_node(const std::shared_ptr<const beta::APINode> node, const std::string& tag) {
        json json_node = toJson(node);
        json_node[TAG] = tag;
        return json_node;
    }
}

json diffNodes(
    const std::shared_ptr<const beta::APINode>& a, 
    const std::shared_ptr<const beta::APINode>& b) // we are not using the map for now.
{
    
    // Any node can have children.
    assert(a->kind == b->kind);

    if (hasChildren(a) && hasChildren(b)) {
        json childrenDiff = json::array();

        // Create StringMaps for a->children and b->children
        llvm::StringMap<llvm::SmallVector<std::shared_ptr<const beta::APINode>,16>> aNSRMap;
        llvm::StringMap<llvm::SmallVector<std::shared_ptr<const beta::APINode>,16>> bNSRMap;
        llvm::StringMap<std::shared_ptr<const beta::APINode>> aUSRMap;
        llvm::StringMap<std::shared_ptr<const beta::APINode>> bUSRMap;
        
        // Populate maps
        for (const auto& childNode : *a->children) {
            aNSRMap[childNode->NSR].emplace_back(std::move(childNode));
            if (!childNode->USR.empty()) {
                aUSRMap.try_emplace(childNode->USR,std::move(childNode));
            }
        }
        
        for (const auto& childNode : *b->children) {
            bNSRMap[childNode->NSR].emplace_back(childNode);
            if (!childNode->USR.empty()) {
                bUSRMap.try_emplace(childNode->USR,std::move(childNode));
            }
        }
        
        for (const auto& childNodeA : *a->children) {
            llvm::StringRef key = childNodeA->NSR;
            auto it = bNSRMap.find(key);
            if (it == bNSRMap.end()) {
                childrenDiff.emplace_back(get_json_from_node(childNodeA, REMOVED));
            } 
            else {
                size_t countA = aNSRMap[key].size();
                size_t countB = bNSRMap[key].size();
                if (countA + countB > 2) {
                    assert(!childNodeA->USR.empty());
                    key = childNodeA->USR;
                    auto usrIt = bUSRMap.find(key);
                    if (usrIt != bUSRMap.end()) {
                        const std::shared_ptr<const beta::APINode> childNodeB = usrIt->second;
                        json sameScopeDiff = diffNodes(childNodeA, childNodeB);
                        if (!sameScopeDiff.is_null() && !sameScopeDiff.empty()) {
                            if (sameScopeDiff.is_array()) {
                                childrenDiff.insert(childrenDiff.end(), sameScopeDiff.begin(), sameScopeDiff.end());
                            } 
                            else {
                                childrenDiff.emplace_back(sameScopeDiff);
                            }
                        }
                    } 
                    else {
                        childrenDiff.emplace_back(get_json_from_node(childNodeA, REMOVED));
                    }
                } 
                else {
                    assert(countA+countB == 2);
                    const std::shared_ptr<const beta::APINode> childNodeB = it->second[0];
                    json sameScopeDiff = diffNodes(childNodeA, childNodeB);
                    if (!sameScopeDiff.is_null() && !sameScopeDiff.empty()) {
                        if (sameScopeDiff.is_array()) {
                            childrenDiff.insert(childrenDiff.end(), sameScopeDiff.begin(), sameScopeDiff.end());
                        } 
                        else {
                            childrenDiff.emplace_back(sameScopeDiff);
                        }
                    }
                }
            }
        }
    
        for (const auto& childNodeB : *b->children) {

            llvm::StringRef key = childNodeB->NSR;

            auto it = aNSRMap.find(key);
            if (aNSRMap.find(childNodeB->NSR) == aNSRMap.end()) {
                childrenDiff.emplace_back(get_json_from_node(childNodeB, ADDED));
            }
            else{
                size_t count1 = aNSRMap[key].size();
                size_t count2 = bNSRMap[key].size();
                if (count1 + count2 > 2) {
                    assert(!childNodeB->USR.empty());
                    key = childNodeB->USR;
                    auto usrIt = aUSRMap.find(key);
                    if (usrIt == aUSRMap.end()){
                        childrenDiff.emplace_back(get_json_from_node(childNodeB, ADDED));
                    }
                }
            }
        }
        
        json APINodeDiff = a->diff(b);
        if (!APINodeDiff.empty()) {
            if (APINodeDiff.is_array()) {
                childrenDiff.insert(childrenDiff.end(), APINodeDiff.begin(), APINodeDiff.end());
            } else {
                childrenDiff.emplace_back(APINodeDiff);
            }
        }
        
        if (!childrenDiff.empty()) {
            json diff;
            diff[QUALIFIED_NAME] = a->qualifiedName;
            diff[NODE_TYPE] = serialize(a->kind);
            if (!childrenDiff.empty()) diff[CHILDREN] = childrenDiff;
            diff[TAG] = MODIFIED;
            return json::array().emplace_back(diff);
        }
    }
    else if(hasChildren(a)){

        json childrenDiff = json::array();

        for (const auto& removedNode : *a->children) {
            childrenDiff.emplace_back(get_json_from_node(removedNode, REMOVED));
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
    else if(hasChildren(b)){

        json childrenDiff = json::array();

        for (const auto& addedNode : *b->children) {
            childrenDiff.emplace_back(get_json_from_node(addedNode, ADDED));
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
    const beta::ASTNormalizedContext* context1,
    const beta::ASTNormalizedContext* context2
) {
    
    json diffs = json::array();
    llvm::StringMap<llvm::SmallVector<std::shared_ptr<beta::APINode>,16>> tree1 = context1->getTree();
    llvm::StringMap<llvm::SmallVector<std::shared_ptr<beta::APINode>,16>> tree2 = context2->getTree();

    for (auto const &rootNode1 : context1->getRootNodes()) {

        llvm::StringRef key = rootNode1->NSR;
        
        auto it = tree2.find(key);
        if (it == tree2.end()) {
            diffs.emplace_back(get_json_from_node(rootNode1, REMOVED));
        } 
        else {
            size_t count1 = tree1[key].size();
            size_t count2 = tree2[key].size();
            if (count1 + count2 > 2) {
                assert(!rootNode1->USR.empty());
                key = rootNode1->USR;
                auto usrIt = context2->usrNodeMap.find(key);
                if (usrIt != context2->usrNodeMap.end()) {
                    const std::shared_ptr<const beta::APINode> rootNode2 = usrIt->second;
                    json sameScopeDiff = diffNodes(rootNode1, rootNode2);
                    if (!sameScopeDiff.is_null() && !sameScopeDiff.empty()){
                        if (sameScopeDiff.is_array()){ 
                            diffs.insert(diffs.end(), sameScopeDiff.begin(), sameScopeDiff.end());
                        }
                        else diffs.emplace_back(sameScopeDiff);
                    }
                } 
                else diffs.emplace_back(get_json_from_node(rootNode1, REMOVED));
            } 
            else {
                assert(count1+count2 == 2);
                const std::shared_ptr<const beta::APINode> rootNode2 = it->second[0];
                json sameScopeDiff = diffNodes(rootNode1, rootNode2);
                if (!sameScopeDiff.is_null() && !sameScopeDiff.empty()){
                    if (sameScopeDiff.is_array()){ 
                        diffs.insert(diffs.end(), sameScopeDiff.begin(), sameScopeDiff.end());
                    }
                    else diffs.emplace_back(sameScopeDiff);
                }
            }
        }
    }

    for (const auto & rootNode2 : context2->getRootNodes()) {
        
        llvm::StringRef key = rootNode2->NSR;

        auto it = tree1.find(key);
        if (it == tree1.end()) {
            diffs.emplace_back(get_json_from_node(rootNode2, ADDED));
        }
        else{
            size_t count1 = tree1[key].size();
            size_t count2 = tree2[key].size();
            if (count1 + count2 > 2) {
                assert(!rootNode2->USR.empty());
                key = rootNode2->USR;
                auto usrIt = context1->usrNodeMap.find(key);
                if (usrIt == context1->usrNodeMap.end()){
                    diffs.emplace_back(get_json_from_node(rootNode2, ADDED));
                }
            }
        }
    }

    return diffs;
}