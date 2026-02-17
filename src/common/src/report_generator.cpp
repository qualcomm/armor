// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "report_generator.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "comm_def.hpp"
#include "report_utils.hpp"
#include "categorization.hpp"

#include "logger.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

static json load_json(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open JSON file: " + path);
    }
    json j;
    file >> j;
    return j;
}

static int read_status_safe(const json& obj, const char* key)
{
    if (!obj.is_object() || !obj.contains(key) || obj[key].is_null())
        return 0;

    const json& v = obj[key];
    if (v.is_number_integer())   return v.get<int>();
    if (v.is_number_unsigned())  return static_cast<int>(v.get<unsigned int>());
    if (v.is_number_float())     return static_cast<int>(v.get<double>());
    if (v.is_string()) {
        try { return std::stoi(v.get<std::string>()); }
        catch (...) { return 0; }
    }
    return 0;
}

static json extract_ast_diff(
        const json& root,
        int* parsed_status,
        int* unparsed_status,
        std::vector<std::string>* header_failures)
{
    if (parsed_status)   *parsed_status   = 0;
    if (unparsed_status) *unparsed_status = 0;
    if (header_failures) header_failures->clear();

    // Legacy: whole file IS the array
    if (root.is_array())
        return root;

    if (!root.is_object())
        throw std::runtime_error("Unexpected JSON root type: expected object or array");

    // Read statuses (tolerate number, string, or null)
    if (parsed_status) {
        *parsed_status = read_status_safe(root, "parsed_status");
    }
    if (unparsed_status) {
        // Prefer correct key; fall back to known-typo key
        int up = read_status_safe(root, "unparsed_status");
        if (up == 0 && root.contains("unparsed_staus")) {
            up = read_status_safe(root, "unparsed_staus");
        }
        *unparsed_status = up;
    }

    // Header failures (if present)
    if (header_failures && root.contains("headerResolutionFailures") &&
        root["headerResolutionFailures"].is_array())
    {
        for (const auto& it : root["headerResolutionFailures"]) {
            if (it.is_string()) {
                header_failures->push_back(it.get<std::string>());
            } else if (it.is_object()) {
                if (it.contains("message") && it["message"].is_string())
                    header_failures->push_back(it["message"].get<std::string>());
                else if (it.contains("reason") && it["reason"].is_string())
                    header_failures->push_back(it["reason"].get<std::string>());
                else
                    header_failures->push_back(it.dump());
            } else {
                header_failures->push_back(it.dump());
            }
        }
    }

    // New format must contain "astDiff"
    if (!root.contains("astDiff"))
        return json::array(); // treat as empty diff

    if (!root["astDiff"].is_array())
        throw std::runtime_error("Field 'astDiff' is not an array");

    return root["astDiff"];
}

void report_generator(const std::string& diff_json_path,
                      const std::string& header_file_path,
                      const std::string& output_html_path,
                      const std::string& output_json_path,
                      PARSER parser,
                      bool generate_json) {
    json root = load_json(diff_json_path);

    int parsed_status = 0, unparsed_status = 0;
    std::vector<std::string> header_failures;

    json diff_data =
        extract_ast_diff(root, &parsed_status, &unparsed_status, &header_failures);
    std::vector<json> processed = preprocess_api_changes(diff_data, header_file_path);

    // Determine compatibility
    bool hasBackwardIncompatible = false;
    for (auto& r : processed) {
        if (r.contains("compatibility") &&
            r["compatibility"].is_string() &&
            r["compatibility"].get<std::string>() == "backward_incompatible")
        {
            hasBackwardIncompatible = true;
            break;
        }
    }

    std::string aggCompatibility =
        hasBackwardIncompatible ? "backward_incompatible"
                                : "backward_compatible";

    // Overall category
    const char* overallStatus =
        getOverAllCategory((unsigned)parsed_status,
                           (unsigned)unparsed_status,
                           !hasBackwardIncompatible);

    const char* reason =
        getReasonForCategorization((unsigned)parsed_status,
                                   (unsigned)unparsed_status,
                                   !hasBackwardIncompatible);

    // HTML
    try {
        generate_html_report(processed, output_html_path, parser,
                             parsed_status, unparsed_status,
                             aggCompatibility, overallStatus, reason);

        armor::user_print() << "HTML report generated at: "
                            << output_html_path << "\n";
    }
    catch (const std::exception& e) {
        armor::user_error() << "Failed to generate HTML report: "
                            << e.what() << "\n";
    }

    // JSON
    if (generate_json) {
        try {
            std::string header_name =
                std::filesystem::path(header_file_path).filename().string();

            std::string json_report_dir = "armor_reports/json_reports";
            std::filesystem::create_directories(json_report_dir);

            std::string json_out =
                json_report_dir + "/api_diff_report_" + header_name + ".json";

            generate_json_report(processed, json_out,
                                 parsed_status, unparsed_status,
                                 aggCompatibility, overallStatus, reason);

            armor::user_print() << "JSON report generated at: "
                                << json_out << "\n";
        }
        catch (const std::exception& e) {
            armor::user_error() << "Failed to generate JSON report: "
                                << e.what() << "\n";
        }
    }
}
