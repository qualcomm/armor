// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "node.hpp"
#include "ast_normalized_context.hpp"

nlohmann::json diffTrees(
    const alpha::ASTNormalizedContext* context1,
    const alpha::ASTNormalizedContext* context2
);