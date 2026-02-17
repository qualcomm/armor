// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "node.hpp"
#include <nlohmann/json.hpp>
#include "ast_normalized_context.hpp"
#include "comm_def.hpp"

/**
 * @brief Computes the difference between two AST contexts and returns a structured JSON result.
 * 
 * The returned JSON has the following structure:
 * {
 *   "parsed_status": <status_code>,
 *   "unparsed_status": <status_code>,
 *   "headerResolutionFailures": [...],
 *   "astDiff": [...]
 * }
 * 
 * Status codes:
 * - UNSUPPORTED (1): At least one change is not supported
 * - SUPPORTED (2): All changes are supported
 * - COMMENTS_UPDATED (3): Only comments or whitespace/formatting were modified
 * - WHITESPACES_UPDATED (5): No code changes
 * 
 * @param context1 The first (old) AST context
 * @param context2 The second (new) AST context
 * @return nlohmann::json A structured JSON object containing diff results and status codes
 */
nlohmann::json diffTrees(
    beta::ASTNormalizedContext* context1,
    beta::ASTNormalizedContext* context2
);