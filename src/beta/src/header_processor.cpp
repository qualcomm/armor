// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <nlohmann/json.hpp>

#include "clang/Tooling/CompilationDatabase.h"
#include "llvm/Support/raw_ostream.h"

#include "comm_def.hpp"
#include "diffengine.hpp"
#include "logger.hpp"
#include "header_processor.hpp"
#include "header_processor_utils.hpp"
#include "session.hpp"
#include "astnormalizer.hpp"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

namespace armor { namespace beta {

PARSING_STATUS processHeaderPairBeta(const std::string& project1,
                       const std::string& file1,
                       const std::string& project2,
                       const std::string& file2,
                       const std::string& reportFormat,
                       const std::vector<std::string>& IncludePaths,
                       const std::vector<std::string>& macroFlags,
                       const LANG_OPTIONS lang) {

    if (!DebugConfig::getInstance().initialize()) {
        armor::user_error() << "Failed to open diagnostics log <" << LOG_FILE_PATH << ">, using stderr\n";
    }

    std::vector<std::string> inclusion_paths1 = generateIncludePaths(project1, file1);
    std::vector<std::string> inclusion_paths2 = generateIncludePaths(project2, file2);

    std::vector<std::string> inclusionPaths1 = resolveInternalIncludePaths(IncludePaths, project1);
    std::vector<std::string> inclusionPaths2 = resolveInternalIncludePaths(IncludePaths, project2);

    std::vector<std::string> Flags1 = getClangFlags(inclusionPaths1, macroFlags, lang);
    std::vector<std::string> Flags2 = getClangFlags(inclusionPaths2, macroFlags, lang);

    Flags1.insert(Flags1.end(), inclusion_paths1.begin(), inclusion_paths1.end());
    Flags2.insert(Flags2.end(), inclusion_paths2.begin(), inclusion_paths2.end());

    // 1. Set up the Session
    auto compDB1 = std::make_unique<FixedCompilationDatabase>(project1, Flags1);
    auto compDB2 = std::make_unique<FixedCompilationDatabase>(project2, Flags2);
    auto session = std::make_unique<armor::APISession>();

    armor::info() << "Processing File1 : " << file1 << "\n";
    for (auto& x : Flags1) {
        armor::info() << "Clang search path : " << x << "\n";
    }

    // 2. Process the files. The session handles the tools and contexts.
    PARSING_STATUS header1ParsingStatus = session->processFileBeta(
        file1, std::move(compDB1), createNormalizeActionFactory(session.get(), file1));

    armor::info() << "Processing File2 : " << file2 << "\n";
    for (auto& x : Flags2) {
        armor::info() << "Clang search path : " << x << "\n";
    }

    PARSING_STATUS header2ParsingStatus = session->processFileBeta(
        file2, std::move(compDB2), createNormalizeActionFactory(session.get(), file2));

    // 3. Retrieve the results from the session
    armor::ASTNormalizedContext* context1 = session->getBetaContext(file1);
    armor::ASTNormalizedContext* context2 = session->getBetaContext(file2);

    if (!context1 || !context2) {
        armor::user_error() << "Failed to retrieve processing results from session\n";
        return FATAL_ERRORS;
    }

    // 4. Perform the diff using the retrieved contexts
    nlohmann::json diffResult = diffTrees(context1, context2);

    armor::emitHeaderDiffReport(diffResult, file1, project1, reportFormat, BETA_PARSER);

    DebugConfig::getInstance().flush();

    return  header1ParsingStatus == header2ParsingStatus ? header1ParsingStatus : FATAL_ERRORS;

}

} } // namespace armor::beta
