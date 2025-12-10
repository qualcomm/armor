// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include "report_generator.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "comm_def.hpp"
#include "report_utils.hpp"
#include "user_print.hpp"

using json = nlohmann::json;

static json load_json(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open JSON file: " + path);
    }
    json j;
    file >> j;
    return j;
}

void report_generator(const std::string& diff_json_path,
                      const std::string& header_file_path,
                      const std::string& output_html_path,
                      const std::string& output_json_path,
                      PARSER parser,
                      bool generate_json) {
    json diff_data = load_json(diff_json_path);
    std::vector<json> processed = preprocess_api_changes(diff_data, header_file_path);

    // Generate HTML report with error handling
    try {
        generate_html_report(processed, output_html_path, parser);
        USER_PRINT(std::string("HTML report generated at: ") + output_html_path);
    } catch (const std::exception& e) {
        USER_ERROR(std::string("Failed to generate HTML report: ") + e.what());
    }

    if (generate_json) {
        try {
            std::string header_name = std::filesystem::path(header_file_path).filename().string();
            std::string json_report_dir = "armor_reports/json_reports";
            std::filesystem::create_directories(json_report_dir);
            std::string json_output_path = json_report_dir + "/api_diff_report_" + header_name + ".json";
            generate_json_report(processed, json_output_path);
            USER_PRINT(std::string("JSON report generated at: ") + json_output_path);
        } catch (const std::exception& e) {
            USER_ERROR(std::string("Failed to generate JSON report: ") + e.what());
        }
    }
}
