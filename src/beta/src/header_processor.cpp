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
#include "debug_config.hpp"
#include "header_processor.hpp"
#include "user_print.hpp"
#include "session.hpp"

//one shared stream for the whole run (lives for process lifetime)
static std::unique_ptr<llvm::raw_fd_ostream> gSharedLog;

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
            USER_ERROR("Warning: Header file " + headerPath + " is not within project path " + projectPath );
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

    // === Initialize the shared log sink BEFORE any logging ===
    if (!gSharedLog) {
        const char* envLog = std::getenv("CLANG_DIAG_LOG");
        const std::string logPath = (envLog && *envLog)
            ? std::string(envLog)
            : std::string("debug_output/logs/diagnostics.log");

        if (!logPath.empty()) {
            llvm::SmallString<256> p(logPath);
            llvm::StringRef dir = llvm::sys::path::parent_path(p);
            if (!dir.empty()) {
                llvm::sys::fs::create_directories(dir);
            }
        }

        std::error_code ec;
        auto stream = std::make_unique<llvm::raw_fd_ostream>(
            logPath, ec, llvm::sys::fs::OF_Text | llvm::sys::fs::OF_Append);

        if (ec) {
            // Fallback to stderr so nothing is lost
            DebugConfig::instance().setSink(&llvm::errs());
            USER_ERROR(std::string("[WARN] Failed to open diagnostics log '") +
           logPath + "': " + ec.message());
        } else {
            gSharedLog = std::move(stream);
            DebugConfig::instance().setSink(gSharedLog.get());
        }
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

    DebugConfig::instance().log("Processing File1 : " + file1, DebugConfig::Level::INFO);
    for (auto& x : Flags1) {
        DebugConfig::instance().log("Clang search path : " + x, DebugConfig::Level::INFO);
    }

    // 2. Process the files. The session handles the tools and contexts.
    PARSING_STATUS header1ParsingStatus = session->processFile(file1, std::move(compDB1));

    DebugConfig::instance().log("Processing File2 : " + file2, DebugConfig::Level::INFO);
    for (auto& x : Flags2) {
        DebugConfig::instance().log("Clang search path : " + x, DebugConfig::Level::INFO);
    }

    PARSING_STATUS header2ParsingStatus = session->processFile(file2, std::move(compDB2));

    // 3. Retrieve the results from the session
    const beta::ASTNormalizedContext* context1 = session->getContext(file1);
    const beta::ASTNormalizedContext* context2 = session->getContext(file2);

    if (!context1 || !context2) {
        USER_ERROR("Failed to retrieve processing results from session");
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
        USER_ERROR(std::string("Error generating AST diff: ") + e.what());
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
            generate_html_report(emptyData, htmlReportFile, BETA_PARSER);
            USER_PRINT(std::string("HTML report generated at: ") + htmlReportFile);
        }
        catch (const std::exception& e) {
            USER_ERROR(std::string("Failed to generate HTML report: ") + e.what());
        }
    }

    return  header1ParsingStatus == header2ParsingStatus ? header1ParsingStatus : FATAL_ERRORS;

}
