// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "node.hpp"
#include <nlohmann/json.hpp>

#include <llvm-14/llvm/ADT/StringSet.h>

nlohmann::json diffTrees(
    const rootApiNodes& roots1, 
    const rootApiNodes& roots2, 
    const normalizedTree& tree1, 
    const normalizedTree& tree2,
    const llvm::StringSet<>& excludeNodes1,
    const llvm::StringSet<>& excludeNodes2
);

nlohmann::json diffNodes(
    const std::shared_ptr<APINode> & a, 
    const std::shared_ptr<APINode> & b, 
    const normalizedTree & treeA, 
    const normalizedTree & treeB
);