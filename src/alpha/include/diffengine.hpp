// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "ast_normalized_context.hpp"
#include <nlohmann/json.hpp>

namespace armor { namespace alpha {

nlohmann::json diffTrees(
    const armor::ASTNormalizedContext* context1,
    const armor::ASTNormalizedContext* context2
);

} } // namespace armor::alpha
