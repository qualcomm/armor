// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstddef>
#include <cstdint>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <memory>

#include "comm_def.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "clang/Basic/SourceLocation.h"

// Main Node structure
// string can be optimized by SmallString

namespace beta {

struct APINode {
    NodeKind kind = NodeKind::Unknown;
    std::string qualifiedName;
    std::string dataType;         // datatype of variables as written .... (int/float/...)
    std::string caonicalType;     // underlying datatype of variable after parsing through typedef/typealias chain
    bool isInclined = false;
    bool isConstExpr = false;
    AccessSpec access = AccessSpec::None;
    APINodeStorageClass storage = APINodeStorageClass::None;
    VirtualQualifier virtualQualifier = VirtualQualifier::None;

    std::string USR;
    std::string NSR;
    llvm::SmallVector<uint64_t,4> stmtHashes;
    std::unique_ptr<llvm::SmallVector<std::shared_ptr<const APINode>,16>> children;

    nlohmann::json diff(const std::shared_ptr<const APINode>& other) const;

};

struct Range{
    unsigned startOffset;
    unsigned endOffset;
    uint64_t hash;
    bool isActive;
    
    Range() : startOffset(0), endOffset(0), hash(-1), isActive(false) {}
    
    Range(unsigned start, unsigned end, uint64_t hash, bool active) 
        : startOffset(start), endOffset(end), hash(hash), isActive(active) {}
};

}