// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "CLI/CLI.hpp"
#include "llvm/Support/raw_ostream.h"
#include "options_handler.hpp"

#include <session.hpp>

#include "alpha/include/header_processor.hpp"
#include "beta/include/header_processor.hpp"
#include "user_print.hpp"

#ifndef TOOL_VERSION
#define TOOL_VERSION ""
#endif

bool filesAreDifferentUsingDiff(const std::string &file1, const std::string &file2) {
    std::string command = "diff -q " + file1 + " " + file2 + " > /dev/null";
    return std::system(command.c_str()) != 0;
}

bool runArmorTool(int argc, const char **argv) {
    CLI::App app{"ARMOR"};
    std::string projectRoot1;
    std::string projectRoot2;
    std::vector<std::string> headers;
    std::string headerSubDir;
    std::string reportFormat = "html";
    bool dumpAstDiff = false;
    std::string debugLevel = "";
    std::vector<std::string> IncludePaths;
    std::vector<std::string> macros;
    std::string macroFlags;
    auto fmt = std::make_shared<CLI::Formatter>();
    fmt->column_width(40);
    app.formatter(fmt);
    // Positional arguments
    app.add_option("projectroot1", projectRoot1, "Path to the project root dir of the older version")->required();
    app.add_option("projectroot2", projectRoot2, "Path to the project root dir of the newer version")->required();
    app.add_option("headers", headers,
        "List of header files to compare between the two versions.\n"
        "\n"
        "Header path interpretation depends on whether --header-dir is provided:\n"
        "\n"
        "  With --header-dir:\n"
        "      Headers are treated as basenames (e.g., \"foo.h\") under the specified subdirectory.\n"
        "      Example:\n"
        "        --header-dir include/api foo.h bar.hpp\n"
        "\n"
        "  • Without --header-dir:\n"
        "      Headers must be relative paths from the project root.\n"
        "      Example:\n"
        "        include/api/foo.h include/api/bar.hpp\n"
    );
    // Optional arguments
    app.add_option("--header-dir", headerSubDir, "Subdirectory under each project root containing headers");
    app.add_option("--report-format,-r", reportFormat, "Report format: html (default).\n"
                                                       "If json is provided, both html and json reports will be generated.")
        ->check(CLI::IsMember({"html", "json"}));
    app.add_flag("--dump-ast-diff", dumpAstDiff, "Dump AST diff JSON files for debugging");
    app.set_version_flag("--version,-v", TOOL_VERSION);
    app.add_option("--log-level", debugLevel, "Set debug log level: ERROR, LOG, INFO (default), DEBUG")
        ->check(CLI::IsMember({"ERROR", "LOG", "INFO", "DEBUG"}));

    app.add_option("-I,--include-paths", IncludePaths,
        "Include paths for header dependencies.\n"
        "Example: -I path/to/include1 -I path/to/include2");
    app.add_option("-m,--macro-flags", macroFlags,
        "Macro flags to be passed for headers.\n");
    CLI11_PARSE(app, argc, argv);
    std::istringstream iss(macroFlags);
    std::string flag;
    while (iss >> flag) {
        macros.push_back(flag);
    }
    // Set level and announce (now goes to the file)
    if (debugLevel == "DEBUG") {
        DebugConfig::instance().setLevel(DebugConfig::Level::DEBUG);
        DebugConfig::instance().log("Debug level set to DEBUG", DebugConfig::Level::INFO);
    } else if (debugLevel == "INFO") {
        DebugConfig::instance().setLevel(DebugConfig::Level::INFO);
        DebugConfig::instance().log("Debug level set to INFO", DebugConfig::Level::INFO);
    } else if (debugLevel == "LOG") {
        DebugConfig::instance().setLevel(DebugConfig::Level::LOG);
        DebugConfig::instance().log("Debug level set to LOG", DebugConfig::Level::INFO);
    } else if (debugLevel == "ERROR") {
        DebugConfig::instance().setLevel(DebugConfig::Level::ERROR);
        DebugConfig::instance().log("Debug level set to ERROR", DebugConfig::Level::INFO);
    }

    bool processed = false;
    std::vector<std::string> headersToCompare;
    if (!headers.empty()) {
        for (const auto &header : headers) {
            std::string file1, file2;
            if (!headerSubDir.empty()) {
                file1 = projectRoot1 + "/" + headerSubDir + "/" + header;
                file2 = projectRoot2 + "/" + headerSubDir + "/" + header;
            } else {
                file1 = projectRoot1 + "/" + header;
                file2 = projectRoot2 + "/" + header;
            }
            USER_PRINT(std::string("Processing files: ") + file1 + " " + file2);
            if (!std::filesystem::exists(file1)) {
                USER_ERROR(std::string("Missing header in older version: ") + file1);
            } else if (!std::filesystem::exists(file2)) {
                USER_ERROR(std::string("Missing header in newer version: ") + file2);
            } else if (filesAreDifferentUsingDiff(file1, file2)) {
                PARSING_STATUS parsingStatus = processHeaderPairAlpha(projectRoot1, file1, projectRoot2, file2, reportFormat,
                                IncludePaths, macros);
                switch (parsingStatus) {
                    case NO_FATAL_ERRORS:
                        DebugConfig::instance().log("Processing Headers again via v2", DebugConfig::Level::INFO);
                        processHeaderPairBeta(projectRoot1, file1, projectRoot2, file2, reportFormat,
                                    IncludePaths, macros);
                        break;
                    case FATAL_ERRORS:
                        DebugConfig::instance().log("Processing Headers stopped at v1", DebugConfig::Level::INFO);
                        break;
                }
                processed = true;
            } else {
                USER_PRINT(std::string("No differences found between: ") + file1 + " and " + file2);
            }
        }
    }
    else if (!headerSubDir.empty()) {
        std::string dir1 = projectRoot1 + "/" + headerSubDir;
        std::string dir2 = projectRoot2 + "/" + headerSubDir;
        for (const auto &entry : std::filesystem::directory_iterator(dir1)) {
            if (entry.path().extension() == ".h" || entry.path().extension() == ".hpp") {
                headersToCompare.push_back(entry.path().filename().string());
            }
        }
        USER_PRINT("List of headers to process:");
        for (const auto &h : headersToCompare) {
            USER_PRINT(std::string("  ") + h);
        }
        for (const auto &header : headersToCompare) {
            std::string file1 = dir1 + "/" + header;
            std::string file2 = dir2 + "/" + header;
            USER_PRINT(std::string("Processing files: ") + file1 + " " + file2);
            if (!std::filesystem::exists(file1)) {
                USER_ERROR(std::string("Missing header in older version: ") + file1);
            } else if (!std::filesystem::exists(file2)) {
                USER_ERROR(std::string("Missing header in newer version: ") + file2);
            } else if (filesAreDifferentUsingDiff(file1, file2)) {
                PARSING_STATUS parsingStatus = processHeaderPairAlpha(projectRoot1, file1, projectRoot2, file2, reportFormat,
                                IncludePaths, macros);
                switch (parsingStatus) {
                    case NO_FATAL_ERRORS:
                        DebugConfig::instance().log("Processing Headers again via v2", DebugConfig::Level::INFO);
                        processHeaderPairBeta(projectRoot1, file1, projectRoot2, file2, reportFormat,
                                    IncludePaths, macros);
                        break;
                    case FATAL_ERRORS:
                        DebugConfig::instance().log("Processing Headers stopped at v1", DebugConfig::Level::INFO);
                        break;
                }
                processed = true;
            } else {
                USER_PRINT(std::string("No differences found between: ") + file1 + " and " + file2);
            }
        }
    }
    if (processed && !dumpAstDiff) {
        try {
            std::filesystem::remove_all("debug_output/ast_diffs");
        } catch (const std::exception &e) {
            USER_ERROR(std::string("Failed to remove debug_output directory: ") + e.what());
        }
    }
    if (!processed && headers.empty() && headerSubDir.empty()) {
        const std::string argv0 = argv[0] ? std::string(argv[0]) : std::string("armor");
        USER_ERROR(
            std::string("Usage: ") + argv0 + " <projectroot1> <projectroot2> <header1> <header2> ...\n"
            "Or use --header-dir to compare all headers in a subdirectory.\n"
            "Try '" + argv0 + " --help' for more information."
        );
    }
    return processed;
}
