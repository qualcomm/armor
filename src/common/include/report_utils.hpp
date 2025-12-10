// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "comm_def.hpp"

using json = nlohmann::json;

/**
 * @brief Preprocess API differences into a normalized list of change records.
 *
 * @param api_differences JSON array describing API changes (diff tree).
 * @param header_file_path Path to the header file being analyzed.
 * @return std::vector<json> Each element contains:
 *         {
 *           "headerfile": <string>,
 *           "name": <API name>,
 *           "description": <human-readable detail>,
 *           "changetype": "Functionality_changed" | "Compatibility_changed",
 *           "compatibility": "backward_compatible" | "backward_incompatible"
 *         }
 */
std::vector<json> preprocess_api_changes(const json& api_differences,
                                         const std::string& header_file_path);

/**
 * @brief Generate an HTML report from processed API changes.
 *
 * @param processed_data Vector of JSON records from preprocess_api_changes().
 * @param output_html_path Path to write the HTML file.
 */
void generate_html_report(const std::vector<json>& processed_data,
                          const std::string& output_html_path,
                          PARSER parser
                        );

/**
 * @brief Generate a JSON report from processed API changes.
 *
 * @param processed_data Vector of JSON records from preprocess_api_changes().
 * @param output_json_path Path to write the JSON file.
 */
void generate_json_report(const std::vector<json>& processed_data,
                          const std::string& output_json_path);

