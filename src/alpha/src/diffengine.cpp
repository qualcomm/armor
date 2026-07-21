// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "diffengine.hpp"
#include "diff_utils.hpp"
#include "logger.hpp"

using json = nlohmann::json;

namespace armor { namespace alpha {

namespace {

json createHeaderResolutionFailures(const armor::SourceRangeTracker& tracker) {
    json failures = json::array();
    for (const auto& directive : tracker.getFatalDirectives()) {
        json failure;
        failure[HEADER] = directive.Header;
        failure[FILE_PATH]   = directive.File;
        failures.emplace_back(failure);
    }
    return failures;
}

} // namespace


json diffTrees(
    const armor::ASTNormalizedContext* context1,
    const armor::ASTNormalizedContext* context2)
{
    json report;

    report[AST_DIFF] = {};
    report[PARSED_STATUS]  = ParsedDiffStatus::FATAL_ERRORS;
    report[UNPARSED_STATUS] = nullptr;

    json headerFailures = json::array();

    json failures1 = createHeaderResolutionFailures(context1->getSourceRangeTracker());
    if (!failures1.empty()) {
        headerFailures.insert(headerFailures.end(), failures1.begin(), failures1.end());
    }

    json failures2 = createHeaderResolutionFailures(context2->getSourceRangeTracker());
    if (!failures2.empty()) {
        headerFailures.insert(headerFailures.end(), failures2.begin(), failures2.end());
    }

    report[HEADER_RESOLUTION_FAILURES] = headerFailures;
    report[AST_DIFF] = json::array();

    return report;
}

} } // namespace armor::alpha
