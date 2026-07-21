// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Path.h"

#include "comm_def.hpp"
#include "header_processor_utils.hpp"
#include "logger.hpp"
#include "report_generator.hpp"

namespace fs = std::filesystem;

namespace armor {

std::vector<std::string> getClangFlags(const std::vector<std::string>& includePaths,
                                       const std::vector<std::string>& macroFlags,
                                       const LANG_OPTIONS lang) {
    std::vector<std::string> flags;

    const char* rawFlags;
    switch (lang) {
        case LANG_OPTIONS::C:
            rawFlags = CLANG_FLAGS_C;
            break;
        case LANG_OPTIONS::CPP:
            rawFlags = CLANG_FLAGS_CPP;
            break;
        default:
            rawFlags = CLANG_FLAGS_CPP;
            break;
    }

    std::istringstream iss(rawFlags);
    std::string flag;
    while (iss >> flag) {
        flags.emplace_back(std::move(flag));
    }

    // Add runtime include paths
    for (const auto& path : includePaths) {
        flags.emplace_back("-I" + path);
    }

    // Add full macro flags directly
    for (const auto& macro : macroFlags) {
        flags.emplace_back(macro);
    }

    return flags;
}

std::vector<std::string> resolveInternalIncludePaths(const std::vector<std::string>& internalPaths,
                                                     const std::string& workspacePath) {

    std::vector<std::string> resolvedPaths;
    for (const auto& path : internalPaths) {
        resolvedPaths.push_back(workspacePath + "/" + path);
    }
    return resolvedPaths;
}

std::vector<std::string> generateIncludePaths(const std::string& projectPath, const std::string& headerPath) {
    std::vector<std::string> includePaths;

    // Ensure the header path starts with the project path
    if (headerPath.find(projectPath) != 0) {
        armor::user_error() << "Warning: Header file " << headerPath << " is not within project path " << projectPath << "\n";
        return includePaths;
    }

    // Get the directory of the header file
    llvm::SmallString<256> headerDir(headerPath);
    llvm::sys::path::remove_filename(headerDir);

    // Start with the directory containing the header file
    llvm::SmallString<256> currentPath = headerDir;

    // Add all parent directories up to and including the project path
    while (llvm::StringRef(currentPath).str().length() >= projectPath.length()) {
        includePaths.push_back("-I" + llvm::StringRef(currentPath).str());

        // Move up one directory
        llvm::sys::path::remove_filename(currentPath);

        // If we've reached the root directory, break
        if (currentPath.empty()) {
            break;
        }
    }

    // Make sure the project path itself is included
    if (std::find(includePaths.begin(), includePaths.end(), "-I" + projectPath) == includePaths.end()) {
        includePaths.push_back("-I" + projectPath);
    }

    return includePaths;
}

void emitHeaderDiffReport(const nlohmann::json& diffResult,
                          const std::string& file1,
                          const std::string& project1,
                          const std::string& reportFormat,
                          PARSER parser) {

    std::string headerName = std::filesystem::path(file1).filename().string();

    std::string dumpDir = "debug_output/ast_diffs";
    std::filesystem::create_directories(dumpDir);
    std::string outputFile = dumpDir + "/ast_diff_output_" + headerName + ".json";

    try {
        if (!diffResult.empty()) {
            std::ofstream out(outputFile, std::ios::trunc);
            out << diffResult.dump(4);
            out.close();
        }
    }
    catch (const std::exception& e) {
        armor::user_error() << "Error generating AST diff: " << e.what() << "\n";
    }

    std::string reportDir = "armor_reports/html_reports";
    std::filesystem::create_directories(reportDir);
    std::string htmlReportFile = reportDir + "/api_diff_report_" + headerName + ".html";

    if (!diffResult.empty()) {
        bool generate_json = (reportFormat == "json");
        std::string jsonReportFile;
        if (generate_json) {
            std::string jsonReportDir = "armor_reports/json_reports";
            std::filesystem::create_directories(jsonReportDir);
            jsonReportFile = jsonReportDir + "/api_diff_report_" + headerName + ".json";
        }
        fs::path relative_path = fs::relative(file1, project1);
        std::string trimmed_path = relative_path.string();
        report_generator(outputFile, trimmed_path, htmlReportFile, jsonReportFile, parser, generate_json);
    }
    else {
        try {
            armor::user_print() << "HTML report generated at: " << htmlReportFile << "\n";
        } catch (const std::exception& e) {
            armor::user_error() << "Failed to generate HTML report: " << e.what() << "\n";
        }
    }
}

} // namespace armor
