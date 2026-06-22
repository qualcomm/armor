// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include "CLI/CLI.hpp"
#include "llvm/Support/raw_ostream.h"
#include "comm_def.hpp"
#include "options_handler.hpp"
#include "git_utils.hpp"
#include <nlohmann/json.hpp>

#include <session.hpp>

#include "alpha/include/header_processor.hpp"
#include "beta/include/header_processor.hpp"
#include "report_utils.hpp"
#include "diff_utils.hpp"
#include "logger.hpp"

#ifndef TOOL_VERSION
#define TOOL_VERSION ""
#endif

LANG_OPTIONS stringToLangOption(const std::string& lang) {
    // Convert to lowercase for case-insensitive comparison
    std::string lowerLang = lang;
    std::transform(lowerLang.begin(), lowerLang.end(), lowerLang.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (lowerLang == LANG_C) {
        return LANG_OPTIONS::C;
    } else if (lowerLang == LANG_CPP) {
        return LANG_OPTIONS::CPP;
    }
    return LANG_OPTIONS::CPP; // default to C++
}

bool filesAreDifferentUsingDiff(const std::string &file1, const std::string &file2) {
    std::string command = "diff -q " + file1 + " " + file2 + " > /dev/null";
    return std::system(command.c_str()) != 0;
}

void printHeaderSummary(const std::string& headerPath) {
    std::string headerName = std::filesystem::path(headerPath).filename().string();
    std::string jsonPath = "armor_reports/json_reports/api_diff_report_" + headerName + ".json";
    std::ifstream f(jsonPath);
    if (!f.is_open()) return;

    nlohmann::json report;
    try { f >> report; } catch (...) { return; }

    std::string compatibility = report.value("compatibility", "unknown");
    bool pass = (compatibility == "backward_compatible");

    if (pass) {
        llvm::outs() << headerPath << "  ->  BACKWARD_COMPATIBLE\n";
    } else {
        llvm::outs() << headerPath << "  ->  BACKWARD_INCOMPATIBLE\n";
        if (report.contains("api_diff")) {
            for (const auto& change : report["api_diff"]) {
                if (change.value("compatibility", "") == "backward_incompatible") {
                    std::string desc = change.value("description", "");
                    auto nl = desc.find('\n');
                    std::string line = (nl != std::string::npos) ? desc.substr(0, nl) : desc;
                    if (!line.empty())
                        llvm::outs() << "         - " << line << "\n";
                }
            }
        }
    }
    llvm::outs().flush();
}

bool runArmorTool(int argc, const char **argv) {
    // Pre-scan for --dev-mode so we can conditionally define positional args.
    // Without this, CLI11 greedily assigns the first header as projectroot2.
    bool gitDiffMode = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--dev-mode") {
            gitDiffMode = true;
            break;
        }
    }

    CLI::App app{"ARMOR - API backward compatibility checker.\n\n"
                 "Two modes of operation:\n\n"
                 "  Standard mode  (two project roots required):\n"
                 "    armor <root1> <root2> [headers...] [options]\n\n"
                 "  Git mode  (run from inside the repo, no paths needed):\n"
                 "    armor [headers...] --dev-mode [--git-ref=<ref>] [options]\n"};
    std::string projectRoot1;
    std::string projectRoot2;
    std::vector<std::string> headers;
    std::string headerSubDir;
    std::string reportFormat = "html";
    std::string language = LANG_CPP; // default to C++
    bool dumpAstDiff = false;
    std::string debugLevel = "";
    std::vector<std::string> IncludePaths;
    std::vector<std::string> macros;
    std::string macroFlags;
    bool gitDiff = false;
    std::string gitRef = "origin/main";
    std::string newRef = "";
    std::string detectedRepoRoot;
    auto fmt = std::make_shared<CLI::Formatter>();
    fmt->column_width(40);
    app.formatter(fmt);
    // Positional arguments — both roots are omitted in git-diff mode (CWD is used automatically)
    if (!gitDiffMode) {
        app.add_option("projectroot1", projectRoot1,
            "Path to the project root of the older (baseline) version."
        )->required();
        app.add_option("projectroot2", projectRoot2,
            "Path to the project root of the newer version.\n"
            "Required in standard mode. Omit this when using --dev-mode."
        )->required();
    }
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
    app.add_option("--lang,-l", language, "Language mode: cpp (default) or c.\n"
                                          "Use 'c' for C headers, 'cpp' for C++ headers.")
        ->transform(CLI::IsMember({LANG_C, LANG_CPP}, CLI::ignore_case));
    app.add_flag("--dump-ast-diff", dumpAstDiff, "Dump AST diff JSON files for debugging");
    app.set_version_flag("--version,-v", TOOL_VERSION);
    app.add_option("--log-level", debugLevel, "Set debug log level: ERROR, LOG, INFO (default), DEBUG")
        ->check(CLI::IsMember({"ERROR", "LOG", "INFO", "DEBUG"}));

    app.add_option("-I,--include-paths", IncludePaths,
        "Include paths for header dependencies.\n"
        "Example: -I path/to/include1 -I path/to/include2");
    app.add_option("-m,--macro-flags", macroFlags,
        "Macro flags to be passed for headers.\n");
    app.add_flag("--dev-mode", gitDiff,
        "Enable git mode: checks out the ref given by --git-ref via 'git worktree add'\n"
        "and compares it against the current working tree (including uncommitted changes).\n"
        "\n"
        "When this flag is used:\n"
        "  - No project roots are needed (CWD is used automatically).\n"
        "  - The base version is a full worktree checkout at --git-ref.\n"
        "  - The worktree is automatically removed after the run.");
    app.add_option("--git-ref", gitRef,
        "Git ref to check out as the base version when using --dev-mode.\n"
        "Accepts any ref: branch, tag, or commit expression (default: origin/main).\n"
        "  --git-ref origin/main   Base is the remote main branch (default)\n"
        "  --git-ref HEAD          Base is your last local commit\n"
        "  --git-ref HEAD~2        Base is two commits back")
        ->default_val("origin/main");
    app.add_option("--new-ref", newRef,
        "New git ref for pure git-to-git comparison (no working tree involved).\n"
        "When set, both sides come from git history — useful for CI commit-range checks.\n"
        "  --new-ref HEAD              New version is last commit\n"
        "  --new-ref feature-branch    New version is a branch tip\n"
        "Pair with --git-ref to compare any two refs:\n"
        "  --dev-mode --new-ref=HEAD --git-ref=origin/main  (HEAD vs origin/main)\n"
        "  --dev-mode --new-ref=HEAD --git-ref=HEAD~3        (last 3 commits)")
        ->default_val("");
    app.footer(
        "Examples:\n"
        "\n"
        "  Standard mode (two roots):\n"
        "    armor /path/v1 /path/v2 include/mylib.h\n"
        "    armor /path/v1 /path/v2 --header-dir include/api -r json\n"
        "\n"
        "  Git mode — working tree vs ref (default):\n"
        "    armor include/mylib.h --dev-mode\n"
        "    armor include/mylib.h --dev-mode --git-ref=HEAD~1\n"
        "\n"
        "  Git mode — two refs, no working tree (CI use):\n"
        "    armor include/mylib.h --dev-mode --new-ref=HEAD\n"
        "    armor include/mylib.h --dev-mode --new-ref=HEAD --git-ref=HEAD~3\n"
        "    armor --header-dir include/api --dev-mode --new-ref=HEAD\n"
    );
    CLI11_PARSE(app, argc, argv);
    std::istringstream iss(macroFlags);
    std::string flag;
    while (iss >> flag) {
        macros.push_back(flag);
    }

    GitWorktreeGuard worktreeGuard;
    GitWorktreeGuard newRefWorktreeGuard;
    if (gitDiff) {
        if (projectRoot1.empty())
            projectRoot1 = std::filesystem::current_path().string();
        armor::user_print() << "Using project root: " << projectRoot1 << "\n";
        if (!isGitRepo(projectRoot1)) {
            armor::user_error() << "Not a git repository: " << projectRoot1 << "\n";
            return false;
        }
        std::string repoRoot = getGitRepoRoot(projectRoot1);
        detectedRepoRoot = repoRoot;
        std::string refInfo = getRefInfo(repoRoot, gitRef);

        if (!newRef.empty()) {
            // Two-ref mode: both sides from git, working tree not involved
            std::string newRefInfo = getRefInfo(repoRoot, newRef);
            armor::user_print() << "Two-ref mode: comparing "
                                << newRef << (newRefInfo.empty() ? "" : " (" + newRefInfo + ")")
                                << " vs "
                                << gitRef << (refInfo.empty() ? "" : " (" + refInfo + ")")
                                << ". Run 'git fetch' if this looks stale.\n";
        } else {
            // Working tree mode: detect dirty state and inform user
            if (hasUncommittedChanges(repoRoot)) {
                armor::user_print() << "Dirty changes detected — comparing working tree vs "
                                    << gitRef
                                    << (refInfo.empty() ? "" : " (" + refInfo + ")")
                                    << ". Run 'git fetch' if this looks stale.\n";
            } else {
                armor::user_print() << "No uncommitted changes — comparing HEAD vs "
                                    << gitRef
                                    << (refInfo.empty() ? "" : " (" + refInfo + ")")
                                    << ". Run 'git fetch' if this looks stale.\n";
            }
        }

        std::string worktreePath = createGitWorktree(repoRoot, gitRef);
        if (worktreePath.empty()) {
            armor::user_error() << "Failed to create git worktree for ref '" << gitRef << "'.\n";
            return false;
        }
        worktreeGuard.repoRoot = repoRoot;
        worktreeGuard.worktreePath = worktreePath;
        worktreeGuard.active = true;

        // The worktree is a full repo checkout. If CWD is a subdirectory of the
        // repo, navigate to the same subdirectory inside the worktree.
        std::filesystem::path relPath = std::filesystem::canonical(projectRoot1)
                                            .lexically_relative(
                                                std::filesystem::canonical(repoRoot));
        projectRoot2 = projectRoot1;
        projectRoot1 = (std::filesystem::path(worktreePath) / relPath).string();

        if (!newRef.empty()) {
            // Two-ref mode: replace working tree with second git worktree
            std::string newWorktreePath = createGitWorktree(repoRoot, newRef);
            if (newWorktreePath.empty()) {
                armor::user_error() << "Failed to create git worktree for new ref '" << newRef << "'.\n";
                return false;
            }
            newRefWorktreeGuard.repoRoot = repoRoot;
            newRefWorktreeGuard.worktreePath = newWorktreePath;
            newRefWorktreeGuard.active = true;
            projectRoot2 = (std::filesystem::path(newWorktreePath) / relPath).string();
        }
    }

    // Set level and announce (now goes to the file)

    DebugConfig& debugConfig = DebugConfig::getInstance();
    debugConfig.initialize();

    if (debugLevel == "DEBUG") {
        debugConfig.setLevel(DebugConfig::Level::DEBUG);
        armor::info() << "Debug level set to DEBUG\n";
    } else if (debugLevel == "INFO") {
        debugConfig.setLevel(DebugConfig::Level::INFO);
        armor::info() << "Debug level set to INFO\n";
    } else if (debugLevel == "LOG") {
        debugConfig.setLevel(DebugConfig::Level::WARNING);
        armor::info() << "Debug level set to LOG (WARNING)\n";
    } else if (debugLevel == "ERROR") {
        debugConfig.setLevel(DebugConfig::Level::ERROR);
        armor::info() << "Debug level set to ERROR\n";
    }
    else{
        #ifdef TESTING_ENABLED
            llvm::outs()<<"Enabled Testing \n";
        #endif
        debugConfig.setLevel(DebugConfig::Level::NONE);
    }

    // Convert language string to LANG_OPTIONS enum
    LANG_OPTIONS langOption = stringToLangOption(language);
    armor::info() << "Language mode set to: " << language << "\n";

    bool processed = false;

    // In git mode: force JSON so printHeaderSummary can read it,
    // then auto-detect changed headers if none were specified.
    if (gitDiff) {
        reportFormat = "json";
        if (headers.empty() && headerSubDir.empty()) {
            headers = getChangedHeaders(detectedRepoRoot, gitRef, newRef);
            if (headers.empty()) {
                std::string msg = "No changed headers detected";
                if (!newRef.empty())
                    msg += " between " + gitRef + " and " + newRef;
                msg += ". Nothing to compare.";
                armor::user_print() << msg << "\n";
                return true;
            }
            armor::user_print() << "Auto-detected " << headers.size() << " changed header(s):\n";
            for (const auto& h : headers)
                armor::user_print() << "  " << h << "\n";
        }
    }
    std::vector<std::string> headersToCompare;
    if (!headers.empty()) {
        for (const auto &header : headers) {
            std::string file1, file2;
            if (!headerSubDir.empty()) {
                file1 = projectRoot1 + "/" + headerSubDir + "/" + header;
                file2 = projectRoot2 + "/" + headerSubDir + "/" + header;
            }
            else {
                file1 = projectRoot1 + "/" + header;
                file2 = projectRoot2 + "/" + header;
            }
            armor::user_print() << "Processing files: " << file1 << " " << file2 << "\n";
            if ( !std::filesystem::exists(file1) && !std::filesystem::exists(file2) ){
                armor::user_error() << "Missing old and new versions of header : \n" << file1 << "\n" << file2 << "\n";
                std::filesystem::create_directories("armor_reports/html_reports");
                std::filesystem::create_directories("armor_reports/json_reports");
            }
            else if (!std::filesystem::exists(file1)) {
                armor::user_error() << "Missing header in older version: " << file1 << "\n";
                std::string headerName = std::filesystem::path(file2).filename().string();
                const auto& [jsonReportFile, htmlReportFile] = prepare_report_output_dirs(headerName);
                generate_json_report(
                        {},
                          jsonReportFile,
                          static_cast<int>(ParsedDiffStatus::SUPPORTED_UPDATES),
                          static_cast<int>(UnParsedDiffStatus::UN_CHANGED),
                          "backward_compatible",
                          "BACKWARD_COMPATIBLE",
                          "Missing header in older version"
                          );
                generate_html_report(
                    {},
                          htmlReportFile,
                          NO_PARSER,
                          static_cast<int>(ParsedDiffStatus::SUPPORTED_UPDATES),
                          static_cast<int>(UnParsedDiffStatus::UN_CHANGED),
                          "backward_compatible",
                          "BACKWARD_COMPATIBLE",
                          "Missing header in older version",
                          {false, true}
                        );
            }
            else if (!std::filesystem::exists(file2)) {
                armor::user_error() << "Missing header in newer version: " << file2 << "\n";
                std::string headerName = std::filesystem::path(file1).filename().string();
                const auto& [jsonReportFile, htmlReportFile] = prepare_report_output_dirs(headerName);
                generate_json_report(
                        {},
                          jsonReportFile,
                          static_cast<int>(ParsedDiffStatus::SUPPORTED_UPDATES),
                          static_cast<int>(UnParsedDiffStatus::UN_CHANGED),
                          "backward_incompatible",
                          "BACKWARD_INCOMPATIBLE",
                          "Missing header in newer version"
                          );
                generate_html_report(
                    {},
                          htmlReportFile,
                          NO_PARSER,
                          static_cast<int>(ParsedDiffStatus::SUPPORTED_UPDATES),
                          static_cast<int>(UnParsedDiffStatus::UN_CHANGED),
                          "backward_incompatible",
                          "BACKWARD_INCOMPATIBLE",
                          "Missing header in newer version",
                          {true, false}
                        );
            }
            else{
                if (filesAreDifferentUsingDiff(file1, file2)) {
                    PARSING_STATUS parsingStatus = processHeaderPairAlpha(projectRoot1, file1, projectRoot2, file2, reportFormat,
                                    IncludePaths, macros, langOption);
                    switch (parsingStatus) {
                        case NO_FATAL_ERRORS:
                            armor::info() << "Processing Headers again via beta parser\n";
                            processHeaderPairBeta(projectRoot1, file1, projectRoot2, file2, reportFormat,
                                        IncludePaths, macros, langOption);
                            break;
                        case FATAL_ERRORS:
                            armor::info() << "Processing Headers stopped at alpha parser\n";
                            break;
                    }
                    processed = true;
                    if (gitDiff)
                        printHeaderSummary(header);
                }
                else {
                    armor::user_print() << "No differences found between: " << file1 << " and " << file2 << "\n";
                    return true;
                }
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
        armor::user_print() << "List of headers to process:\n";
        for (const auto &h : headersToCompare) {
            armor::user_print() << "  " << h << "\n";
        }
        for (const auto &header : headersToCompare) {
            std::string file1 = dir1 + "/" + header;
            std::string file2 = dir2 + "/" + header;
            armor::user_print() << "Processing files: " << file1 << " " << file2 << "\n";
            if ( !std::filesystem::exists(file1) && !std::filesystem::exists(file2) ){
                armor::user_error() << "Missing old and new versions of header : \n" << file1 << "\n" << file2 << "\n";
            }
            else if (!std::filesystem::exists(file1)) {
                armor::user_error() << "Missing header in older version: " << file1 << "\n";
                std::string headerName = std::filesystem::path(file2).filename().string();
                const auto& [jsonReportFile, htmlReportFile] = prepare_report_output_dirs(headerName);
                generate_json_report(
                        {},
                          jsonReportFile,
                          static_cast<int>(ParsedDiffStatus::SUPPORTED_UPDATES),
                          static_cast<int>(UnParsedDiffStatus::UN_CHANGED),
                          "backward_compatible",
                          "BACKWARD_COMPATIBLE",
                          "Missing header in older version"
                          );
                generate_html_report(
                    {},
                          htmlReportFile,
                          NO_PARSER,
                          static_cast<int>(ParsedDiffStatus::SUPPORTED_UPDATES),
                          static_cast<int>(UnParsedDiffStatus::UN_CHANGED),
                          "backward_compatible",
                          "BACKWARD_COMPATIBLE",
                          "Missing header in older version",
                          {false, true}
                        );
            } else if (!std::filesystem::exists(file2)) {
                armor::user_error() << "Missing header in newer version: " << file2 << "\n";
                std::string headerName = std::filesystem::path(file1).filename().string();
                const auto& [jsonReportFile, htmlReportFile] = prepare_report_output_dirs(headerName);
                generate_json_report(
                        {},
                          jsonReportFile,
                          static_cast<int>(ParsedDiffStatus::SUPPORTED_UPDATES),
                          static_cast<int>(UnParsedDiffStatus::UN_CHANGED),
                          "backward_incompatible",
                          "BACKWARD_INCOMPATIBLE",
                          "Missing header in newer version"
                          );
                generate_html_report(
                    {},
                          htmlReportFile,
                          NO_PARSER,
                          static_cast<int>(ParsedDiffStatus::SUPPORTED_UPDATES),
                          static_cast<int>(UnParsedDiffStatus::UN_CHANGED),
                          "backward_incompatible",
                          "BACKWARD_INCOMPATIBLE",
                          "Missing header in newer version",
                          {true, false}
                        );
            }
            else{
                if (filesAreDifferentUsingDiff(file1, file2)) {
                    PARSING_STATUS parsingStatus = processHeaderPairAlpha(projectRoot1, file1, projectRoot2, file2, reportFormat,
                                    IncludePaths, macros, langOption);
                    switch (parsingStatus) {
                        case NO_FATAL_ERRORS:
                            armor::info() << "Processing Headers again via v2\n";
                            processHeaderPairBeta(projectRoot1, file1, projectRoot2, file2, reportFormat,
                                        IncludePaths, macros, langOption);
                            break;
                        case FATAL_ERRORS:
                            armor::info() << "Processing Headers stopped at v1\n";
                            break;
                    }
                    processed = true;
                    if (gitDiff)
                        printHeaderSummary(header);
                }
                else {
                    llvm::outs()<<"No diff beta\n";
                    armor::user_print() << "No differences found between: " << file1 << " and " << file2 << "\n";
                    return true;
                }
            }
        }
    }
    if (processed && !dumpAstDiff) {
        try {
            std::filesystem::remove_all("debug_output/ast_diffs");
        } catch (const std::exception &e) {
            armor::user_error() << "Failed to remove debug_output directory: " << e.what() << "\n";
        }
    }
    if (!processed && headers.empty() && headerSubDir.empty()) {
        const std::string argv0 = argv[0] ? std::string(argv[0]) : std::string("armor");
        armor::user_error() << "Usage: " << argv0 << " <projectroot1> <projectroot2> <header1> <header2> ...\n"
            << "Or use --header-dir to compare all headers in a subdirectory.\n"
            << "Try '" << argv0 << " --help' for more information.\n";
    }
    return processed;
}
