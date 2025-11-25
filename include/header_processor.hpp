// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#ifndef HEADER_PROCESSOR_HPP
#define HEADER_PROCESSOR_HPP

#include <string>
#include <vector>

void processHeaderPair(const std::string& projectRoot1,
                       const std::string& file1,
                       const std::string& projectRoot2,
                       const std::string& file2,
                       const std::string& reportFormat,
                       const std::vector<std::string>& IncludePaths,
                       const std::vector<std::string>& macroFlags);
std::vector<std::string> getClangFlags();

#endif
