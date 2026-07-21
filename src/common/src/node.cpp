// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "diff_utils.hpp"
#include "node.hpp"
#include <cassert>
#include <iostream>
#include <string>

nlohmann::json armor::APINode::diff(const std::shared_ptr<const armor::APINode>& other) const {
    nlohmann::json result, removed, added;

    auto appendNodeMetadata = [&](nlohmann::json& node) {
        node[NODE_TYPE] = serialize(kind);
        node[QUALIFIED_NAME] = qualifiedName;
    };

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

    if (dataType != other->dataType) {
        if (kind == NodeKind::FunctionPointer) {
            compare(DATA_TYPE, dataType, other->dataType, std::string{});
        } else {
            assert(!caonicalType.empty());
            assert(!other->caonicalType.empty());
            compare(DATA_TYPE, caonicalType, other->caonicalType, std::string{});
        }
    }

    compare(STORAGE_QUALIFIER, storage, other->storage, APINodeStorageClass::None);
    compare(VIRTUAL_QUALIFIER, virtualQualifier, other->virtualQualifier, VirtualQualifier::None);
    compare(INLINE, isInlined, other->isInlined, false);
    compare(CONST_EXPR, isConstExpr, other->isConstExpr, false);
    compare(ACCESS_SPECIFIER, access, other->access, AccessSpec::None);
    compare(IS_OVERRIDE, isOveride, other->isOveride, false);
    compare(IS_FINAL, isFinal, other->isFinal, false);
    compare(IS_DELETE, isDelete, other->isDelete, false);
    compare(IS_DEFAULT, isDefault, other->isDefault, false);
    compare(IS_EXPLICIT, isExplicit, other->isExplicit, false);
    compare(IS_VOLATILE, isVolatile, other->isVolatile, false);
    compare(IS_CONST, isConst, other->isConst, false);
    compare(IS_FRIEND, isFriend, other->isFriend, false);

    if (!removed.empty() || !added.empty()) {
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
        } else {
            result = nlohmann::json::array();
            processChanges(result);
        }
    }

    return result;
}
