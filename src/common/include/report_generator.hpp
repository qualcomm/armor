// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include "comm_def.hpp"

void report_generator(const std::string& diff_json_path,
                          const std::string& header_file_path,
                          const std::string& output_html_path,
                          const std::string& output_json_path,
                          PARSER parser,
                          bool generate_json = false
                        );