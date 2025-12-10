// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <cstddef>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <memory>

#include "comm_def.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

// Main Node structure
// string can be optimized by SmallString

namespace beta {

struct APINode {
    NodeKind kind = NodeKind::Unknown;
    std::string qualifiedName;
    std::string typeName;         // To handle typdef of built-in/CxxRecordDecl/EnumDecl
    std::string dataType;         // datatype of variables as written .... (int/float/...)
    std::string caonicalType;     // underlying datatype of variable after parsing through typedef/typealias chain
    AccessSpec access = AccessSpec::None;
    APINodeStorageClass storage = APINodeStorageClass::None;
    ConstQualifier constQualifier = ConstQualifier::None;
    VirtualQualifier virtualQualifier = VirtualQualifier::None;

    std::string USR;
    std::string NSR;
    std::unique_ptr<llvm::SmallVector<std::shared_ptr<const APINode>,16>> children;

    nlohmann::json diff(const std::shared_ptr<const APINode>& other) const;

};

}