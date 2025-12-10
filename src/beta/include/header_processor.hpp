// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <vector>
#include "session.hpp"

PARSING_STATUS processHeaderPairBeta(const std::string& projectRoot1,
                       const std::string& file1,
                       const std::string& projectRoot2,
                       const std::string& file2,
                       const std::string& reportFormat,
                       const std::vector<std::string>& IncludePaths,
                       const std::vector<std::string>& macroFlags);
