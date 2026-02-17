// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <utility>
#include <vector>
#include <sstream>
#include <system_error>
#include <cstdlib>
#include <nlohmann/json.hpp>

#include "clang/Tooling/CompilationDatabase.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "comm_def.hpp"
#include "report_generator.hpp"
#include "report_utils.hpp"
#include "diffengine.hpp"
#include "logger.hpp"
#include "header_processor.hpp"
#include "session.hpp"

namespace fs = std::filesystem;
using namespace clang;
using namespace clang::tooling;
using namespace llvm;

#ifndef CLANG_FLAGS
#define CLANG_FLAGS ""
#endif

namespace {

    std::vector<std::string> getClangFlags(const std::vector<std::string>& includePaths,
                                           const std::vector<std::string>& macroFlags) {
        std::vector<std::string> flags;

        std::istringstream iss(CLANG_FLAGS);
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
}

PARSING_STATUS processHeaderPairBeta(const std::string& project1,
                       const std::string& file1,
                       const std::string& project2,
                       const std::string& file2,
                       const std::string& reportFormat,
                       const std::vector<std::string>& IncludePaths,
                       const std::vector<std::string>& macroFlags) {

    if (!DebugConfig::getInstance().initialize()) {
        armor::user_error() << "Failed to open diagnostics log <" << LOG_FILE_PATH << ">, using stderr\n";
    }

    std::vector<std::string> inclusion_paths1 = generateIncludePaths(project1, file1);
    std::vector<std::string> inclusion_paths2 = generateIncludePaths(project2, file2);

    std::vector<std::string> inclusionPaths1 = resolveInternalIncludePaths(IncludePaths, project1);
    std::vector<std::string> inclusionPaths2 = resolveInternalIncludePaths(IncludePaths, project2);

    std::vector<std::string> Flags1 = getClangFlags(inclusionPaths1, macroFlags);
    std::vector<std::string> Flags2 = getClangFlags(inclusionPaths2, macroFlags);

    Flags1.insert(Flags1.end(), inclusion_paths1.begin(), inclusion_paths1.end());
    Flags2.insert(Flags2.end(), inclusion_paths2.begin(), inclusion_paths2.end());

    // 1. Set up the Session
    auto compDB1 = std::make_unique<FixedCompilationDatabase>(project1, Flags1);
    auto compDB2 = std::make_unique<FixedCompilationDatabase>(project2, Flags2);
    auto session = std::make_unique<beta::APISession>();

    std::string headerName = std::filesystem::path(file1).filename().c_str();

    armor::info() << "Processing File1 : " << file1 << "\n";
    for (auto& x : Flags1) {
        armor::info() << "Clang search path : " << x << "\n";
    }

    // 2. Process the files. The session handles the tools and contexts.
    PARSING_STATUS header1ParsingStatus = session->processFile(file1, std::move(compDB1));

    armor::info() << "Processing File2 : " << file2 << "\n";
    for (auto& x : Flags2) {
        armor::info() << "Clang search path : " << x << "\n";
    }

    PARSING_STATUS header2ParsingStatus = session->processFile(file2, std::move(compDB2));

    // 3. Retrieve the results from the session
    beta::ASTNormalizedContext* context1 = session->getContext(file1);
    beta::ASTNormalizedContext* context2 = session->getContext(file2);

    if (!context1 || !context2) {
        armor::user_error() << "Failed to retrieve processing results from session\n";
        return FATAL_ERRORS;
    }

    // 4. Perform the diff using the retrieved contexts
    nlohmann::json diffResult;

    diffResult = diffTrees(
        context1,
        context2
    );

    std::string dumpDir = "debug_output/ast_diffs";
    std::filesystem::create_directories(dumpDir);
    std::string outputFile = dumpDir + "/ast_diff_output_" + headerName + ".json";

    try {
        if (!diffResult.empty()) {
            std::ofstream out(outputFile,std::ios::trunc);
            out << diffResult.dump(4);
            out.close();
        } 
    } catch (const std::exception& e) {
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
        report_generator(outputFile, trimmed_path, htmlReportFile, jsonReportFile, BETA_PARSER, generate_json);
    }
    else {
        try {
            std::vector<json> emptyData;
            // generate_html_report(emptyData, htmlReportFile, BETA_PARSER);
            armor::user_print() << "HTML report generated at: " << htmlReportFile << "\n";
        }
        catch (const std::exception& e) {
            armor::user_error() << "Failed to generate HTML report: " << e.what() << "\n";
        }
    }

    DebugConfig::getInstance().flush();

    return  header1ParsingStatus == header2ParsingStatus ? header1ParsingStatus : FATAL_ERRORS;

}
