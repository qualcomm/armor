// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <cassert>
#include <cstddef>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/DenseMap.h>
#include <string_view>
#include <utility>

#include "ast_normalized_context.hpp"
#include "diffengine.hpp"
#include "diff_utils.hpp"
#include "logger.hpp"
#include "node.hpp"
#include "comm_def.hpp"

using json = nlohmann::json;

const bool inline hasChildren(const std::shared_ptr<const beta::APINode>& node) {
    return node->children == nullptr ? false : !node->children->empty();
}

namespace{

    #ifdef TESTING_ENABLED
        void printDenseMap(const llvm::DenseMap<uint64_t, int>& map, const std::string_view& mapName) {
            TEST_LOG << mapName << "\n";
            if (map.empty()) {
                TEST_LOG << "(empty)" << "\n";
            } else {
                for (const auto& [x,y] : map) {
                    TEST_LOG << x << "  : " << y << "\n";
                }
            }
            TEST_LOG << "----------------------------------------\n";
        }
    #endif

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

    void reconcileUnhandledDeclHashes(beta::ASTNormalizedContext* context, const std::shared_ptr<const beta::APINode>& node){
        beta::SourceRangeTracker& tracker = context->getSourceRangeTracker();
        llvm::DenseMap<uint64_t, int>& unhandledDeclsHashMap = tracker.getUnhandledDeclsHashMap();
        if(node->stmtHashes.size()) TEST_LOG << "reconcileUnhandledDeclHashes\n" << node->qualifiedName << "\n";
        for(uint64_t stmtHash : node->stmtHashes ){
            auto it = unhandledDeclsHashMap.find(stmtHash);
            if (it != unhandledDeclsHashMap.end()) {
                it->second--;
                if (it->second <= 0) unhandledDeclsHashMap.erase(it);
                TEST_LOG << stmtHash << "\n----------------------------------------\n";
            }
        }
        if(hasChildren(node)) {
            for (const auto& childNode : *node->children) {
                reconcileUnhandledDeclHashes(context, childNode);
            }
        }
    }

    bool hasHashMapDifference(const llvm::DenseMap<uint64_t, int>& map1, 
                              const llvm::DenseMap<uint64_t, int>& map2) {
        for (const auto& [x,y] : map1) {
            auto itr = map2.find(x);
            if ( itr == map2.end()) {
                return true;
            }
            else{
                if( itr->second != y ) return true;
            }
        }
        
        return false;
    }

    ParsedDiffStatus determineStatus(bool hasASTDiff, bool hasCommentsDiff, bool hasUnhandledDeclsDiff) {

        if (hasUnhandledDeclsDiff) {
            return ParsedDiffStatus::UNSUPPORTED_UPDATES;
        }
        
        if (hasASTDiff) {
            return ParsedDiffStatus::SUPPORTED_UPDATES;
        }
        
        if (hasCommentsDiff) {
            return ParsedDiffStatus::COMMENTS_UPDATED;
        }

        return ParsedDiffStatus::NON_FUNCTIONAL_CHANGES ;
    }
}

json diffNodes(
    beta::ASTNormalizedContext* contextA,
    beta::ASTNormalizedContext* contextB,
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
                        json sameScopeDiff = diffNodes(contextA, contextB,childNodeA, childNodeB);
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
                    json sameScopeDiff = diffNodes(contextA, contextB,childNodeA, childNodeB);
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
                reconcileUnhandledDeclHashes(contextB, childNodeB);
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
                        reconcileUnhandledDeclHashes(contextB, childNodeB);
                    }
                }
                // No else as we already computed if count1 + count 2 == 2 we do not have to compute it again.
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
            reconcileUnhandledDeclHashes(contextB, addedNode);
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
    beta::ASTNormalizedContext* context1,
    beta::ASTNormalizedContext* context2
) {
    
    json astDiff = json::array();
    llvm::StringMap<llvm::SmallVector<std::shared_ptr<beta::APINode>,16>> tree1 = context1->getTree();
    llvm::StringMap<llvm::SmallVector<std::shared_ptr<beta::APINode>,16>> tree2 = context2->getTree();

    for (auto const &rootNode1 : context1->getRootNodes()) {

        llvm::StringRef key = rootNode1->NSR;
        
        auto it = tree2.find(key);
        if (it == tree2.end()) {
            astDiff.emplace_back(get_json_from_node(rootNode1, REMOVED));
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
                    json sameScopeDiff = diffNodes(context1, context2, rootNode1, rootNode2);
                    if (!sameScopeDiff.is_null() && !sameScopeDiff.empty()){
                        if (sameScopeDiff.is_array()){ 
                            astDiff.insert(astDiff.end(), sameScopeDiff.begin(), sameScopeDiff.end());
                        }
                        else astDiff.emplace_back(sameScopeDiff);
                    }
                } 
                else astDiff.emplace_back(get_json_from_node(rootNode1, REMOVED));
            } 
            else {
                assert(count1+count2 == 2);
                const std::shared_ptr<const beta::APINode> rootNode2 = it->second[0];
                json sameScopeDiff = diffNodes(context1, context2, rootNode1, rootNode2);
                if (!sameScopeDiff.is_null() && !sameScopeDiff.empty()){
                    if (sameScopeDiff.is_array()){ 
                        astDiff.insert(astDiff.end(), sameScopeDiff.begin(), sameScopeDiff.end());
                    }
                    else astDiff.emplace_back(sameScopeDiff);
                }
            }
        }
    }

    for (const auto & rootNode2 : context2->getRootNodes()) {
        
        llvm::StringRef key = rootNode2->NSR;

        auto it = tree1.find(key);
        if (it == tree1.end()) {
            astDiff.emplace_back(get_json_from_node(rootNode2, ADDED));
            reconcileUnhandledDeclHashes(context2, rootNode2);
        }
        else{
            size_t count1 = tree1[key].size();
            size_t count2 = tree2[key].size();
            if (count1 + count2 > 2) {
                assert(!rootNode2->USR.empty());
                key = rootNode2->USR;
                auto usrIt = context1->usrNodeMap.find(key);
                if (usrIt == context1->usrNodeMap.end()){
                    astDiff.emplace_back(get_json_from_node(rootNode2, ADDED));
                    reconcileUnhandledDeclHashes(context2, rootNode2);
                }
            }
            // No else as we already computed if count1 + count 2 == 2 we do not have to compute it again.
        }
    }

    const beta::SourceRangeTracker& tracker1 = context1->getSourceRangeTracker();
    const beta::SourceRangeTracker& tracker2 = context2->getSourceRangeTracker();
    
    const llvm::DenseMap<uint64_t, int>& commentsHashMap1 = tracker1.getCommentsHashMap();
    const llvm::DenseMap<uint64_t, int>& commentsHashMap2 = tracker2.getCommentsHashMap();

    const llvm::DenseMap<uint64_t, int>& unhandledDeclsHashMap1 = tracker1.getUnhandledDeclsHashMap();
    const llvm::DenseMap<uint64_t, int>& unhandledDeclsHashMap2 = tracker2.getUnhandledDeclsHashMap();

    const llvm::DenseMap<uint64_t, int>& inactiveUnhandledDeclsHashMap1 = tracker1.getInactiveUnhandledDeclsHashMap();
    const llvm::DenseMap<uint64_t, int>& inactiveUnhandledDeclsHashMap2 = tracker2.getInactiveUnhandledDeclsHashMap();

    #ifdef TESTING_ENABLED
        TEST_LOG << "\n############ CONTEXT 1 MAPS ############" << "\n";
        printDenseMap(commentsHashMap1, "comments1");
        printDenseMap(unhandledDeclsHashMap1, "unhandledDecls1");
        printDenseMap(inactiveUnhandledDeclsHashMap1, "inactiveUnhandledDecls1");
        
        TEST_LOG << "\n############ CONTEXT 2 MAPS ############" << "\n";
        printDenseMap(commentsHashMap2, "comments2");
        printDenseMap(unhandledDeclsHashMap2, "unhandledDecls2");
        printDenseMap(inactiveUnhandledDeclsHashMap2, "inactiveUnhandledDecls2");
    #endif

    bool hasASTDiff = !astDiff.empty();
    
    bool hasCommentsDiff = hasHashMapDifference(commentsHashMap1, commentsHashMap2) || 
                           hasHashMapDifference(commentsHashMap2, commentsHashMap1);

    bool hasUnhandledDeclsDiff = hasHashMapDifference(unhandledDeclsHashMap1, unhandledDeclsHashMap2) || 
                                 hasHashMapDifference(unhandledDeclsHashMap2, unhandledDeclsHashMap1);

    bool hasInactiveUnhandledDeclsDiff = hasHashMapDifference(inactiveUnhandledDeclsHashMap1, inactiveUnhandledDeclsHashMap2) || 
                                         hasHashMapDifference(inactiveUnhandledDeclsHashMap2, inactiveUnhandledDeclsHashMap1);

    ParsedDiffStatus parsedStatus = determineStatus(hasASTDiff, hasCommentsDiff, hasUnhandledDeclsDiff);
    UnParsedDiffStatus unparsedStatus = hasInactiveUnhandledDeclsDiff ? UnParsedDiffStatus::CHANGED : UnParsedDiffStatus::UN_CHANGES;

    json result;
    result[PARSED_STATUS] = parsedStatus;
    result[UNPARSED_STATUS] = unparsedStatus;
    result[HEADER_RESOLUTION_FAILURES] = json::array();
    result[AST_DIFF] = astDiff;

    return result;
}