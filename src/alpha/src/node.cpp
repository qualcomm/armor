// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include "diff_utils.hpp"
#include "node.hpp"

nlohmann::json alpha::APINode::diff(const std::shared_ptr<const alpha::APINode>& other) const {
    nlohmann::json result, removed, added;

    // Helper to add metadata to a diff JSON node
    auto appendNodeMetadata  = [&](nlohmann::json& node) {
        node[NODE_TYPE] = serialize(kind);
        node[QUALIFIED_NAME] = qualifiedName;
    };

    // Define a lambda function to compare fields
    auto compare = [&](const std::string& field, const auto &lhs, const auto &rhs, const auto &emptyValue) {
        if (lhs != rhs) {
            if (lhs != emptyValue) {
                removed[field] = serialize(lhs);
            }
            if (rhs != emptyValue) {
                added[field] = serialize(rhs);
            }
        }
    };

    // Compare fields
    if(dataType != DATA_TYPE_PLACE_HOLDER && other->dataType != DATA_TYPE_PLACE_HOLDER){
        compare(DATA_TYPE, dataType, other->dataType, std::string{});
    }
    compare(
        STORAGE_QUALIFIER,
        storage,
        other->storage,
        APINodeStorageClass::None
    );
    compare(
        CONST_QUALIFIER,
        constQualifier,
        other->constQualifier,
        ConstQualifier::None
    );
    
    // If there are any changes
    if (!removed.empty() || !added.empty()) {
        // Helper to process removed and added changes
        auto processChanges = [&](nlohmann::json& parent) {
            if (!removed.empty()) {
                removed[TAG] = REMOVED;
                appendNodeMetadata(removed);
                parent.emplace_back(std::move(removed));
            }
            if (!added.empty()) {
                added[TAG] = ADDED;
                appendNodeMetadata(added);
                parent.emplace_back(std::move(added));
            }
        };

        if (this->children == nullptr) {
            nlohmann::json children = nlohmann::json::array();
            processChanges(children);

            if (!children.empty()) {
                result[CHILDREN] = std::move(children);
                appendNodeMetadata(result);
                result[TAG] = MODIFIED;
            }
        }
        else {
            result = nlohmann::json::array();
            processChanges(result);
        }
    }

    return result;
}