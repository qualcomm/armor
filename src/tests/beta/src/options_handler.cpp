// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include "CLI/CLI.hpp"
#include "llvm/Support/raw_ostream.h"
#include "options_handler.hpp"
#include "header_processor.hpp"
#include "logger.hpp"

#ifndef TOOL_VERSION
#define TOOL_VERSION ""
#endif

LANG_OPTIONS stringToLangOption(const std::string& lang) {
    std::string lowerLang = lang;
    std::transform(lowerLang.begin(), lowerLang.end(), lowerLang.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    if (lowerLang == LANG_C)   return LANG_OPTIONS::C;
    if (lowerLang == LANG_CPP) return LANG_OPTIONS::CPP;
    return LANG_OPTIONS::CPP;
}

bool runArmorTool(int argc, const char **argv) {
    CLI::App app{"ARMOR beta — full AST diff test"};
    std::string projectRoot1;
    std::string projectRoot2;
    std::vector<std::string> headers;
    std::string reportFormat = "json";
    std::string language  = LANG_CPP;
    std::string debugLevel;
    std::vector<std::string> IncludePaths;
    std::string macroFlags;
    std::vector<std::string> macros;

    app.add_option("projectroot1",       projectRoot1, "Path to the baseline project root")->required();
    app.add_option("projectroot2",       projectRoot2, "Path to the newer project root")->required();
    app.add_option("headers",            headers,      "Header files (relative to project root) to compare");
    app.add_option("--report-format,-r", reportFormat, "Report format: html or json")
        ->check(CLI::IsMember({"html", "json"}));
    app.add_option("--lang,-l",          language,     "Language: cpp (default) or c")
        ->transform(CLI::IsMember({LANG_C, LANG_CPP}, CLI::ignore_case));
    app.add_option("--log-level",        debugLevel,   "Log level: ERROR, LOG, INFO, DEBUG")
        ->check(CLI::IsMember({"ERROR", "LOG", "INFO", "DEBUG"}));
    app.add_option("-I,--include-paths", IncludePaths, "Extra include paths");
    app.add_option("-m,--macro-flags",   macroFlags,   "Macro flags");
    app.set_version_flag("--version,-v", TOOL_VERSION);
    CLI11_PARSE(app, argc, argv);

    std::istringstream iss(macroFlags);
    std::string flag;
    while (iss >> flag) macros.push_back(flag);

    DebugConfig& debugConfig = DebugConfig::getInstance();
    debugConfig.initialize();

    if      (debugLevel == "DEBUG") debugConfig.setLevel(DebugConfig::Level::DEBUG);
    else if (debugLevel == "INFO")  debugConfig.setLevel(DebugConfig::Level::INFO);
    else if (debugLevel == "LOG")   debugConfig.setLevel(DebugConfig::Level::WARNING);
    else if (debugLevel == "ERROR") debugConfig.setLevel(DebugConfig::Level::ERROR);
    else                            debugConfig.setLevel(DebugConfig::Level::NONE);

    LANG_OPTIONS langOption = stringToLangOption(language);

    if (headers.empty()) {
        armor::user_error() << "No headers specified. Usage: beta <root1> <root2> <header...>\n";
        return false;
    }

    bool processed = false;
    for (const auto& header : headers) {
        std::string file1 = projectRoot1 + "/" + header;
        std::string file2 = projectRoot2 + "/" + header;

        armor::user_print() << "Processing: " << file1 << " vs " << file2 << "\n";

        if (!std::filesystem::exists(file1)) {
            armor::user_error() << "Missing header in older version: " << file1 << "\n";
            continue;
        }
        if (!std::filesystem::exists(file2)) {
            armor::user_error() << "Missing header in newer version: " << file2 << "\n";
            continue;
        }

        armor::beta::processHeaderPairBeta(projectRoot1, file1, projectRoot2, file2,
                              reportFormat, IncludePaths, macros, langOption);
        processed = true;
    }

    return processed;
}
