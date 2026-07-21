// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "comm_def.hpp"

namespace armor {

std::vector<std::string> getClangFlags(const std::vector<std::string>& includePaths,
                                       const std::vector<std::string>& macroFlags,
                                       LANG_OPTIONS lang);

std::vector<std::string> resolveInternalIncludePaths(const std::vector<std::string>& internalPaths,
                                                     const std::string& workspacePath);

std::vector<std::string> generateIncludePaths(const std::string& projectPath,
                                              const std::string& headerPath);

void emitHeaderDiffReport(const nlohmann::json& diffResult,
                          const std::string& file1,
                          const std::string& project1,
                          const std::string& reportFormat,
                          PARSER parser);

} // namespace armor
