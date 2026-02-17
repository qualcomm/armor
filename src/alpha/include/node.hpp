// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <llvm-14/llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>

#include "comm_def.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

// Main Node structure
// string can be optimized by string_view powered by stringRef

namespace alpha{

struct APINode {
    NodeKind kind = NodeKind::Unknown;
    std::string hash;
    std::string qualifiedName;
    std::string dataType;         // Underlying datatype of variables .... (int/float/...)
    bool isInclined = false;
    bool isConstExpr = false;
    APINodeStorageClass storage = APINodeStorageClass::None;

    std::unique_ptr<llvm::SmallVector<std::shared_ptr<const APINode>,16>> children;

    nlohmann::json diff(const std::shared_ptr<const APINode>& other) const;
};

}