// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <cstdlib>
#include <system_error>
#include <string>

#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "session.hpp"
#include "astnormalizer.hpp"
#include "ast_normalized_context.hpp"
#include "logger.hpp"

void beta::APISession::createNormalizedASTContext(const std::string& key){
    const auto pair = m_contexts.try_emplace(key, std::make_unique<ASTNormalizedContext>());
    if (!pair.second) {
        throw std::runtime_error("AST context already exists for key: " + key);
    }
}

PARSING_STATUS beta::APISession::processFile(std::string fileName, std::unique_ptr<clang::tooling::FixedCompilationDatabase> m_compDB) {
    // Initialize DebugConfig2 with diagnostics log path if not already initialized
    DebugConfig& debugConfig = DebugConfig::getInstance();

    // Get the sink from DebugConfig2 (handles file creation and fallback)
    llvm::raw_ostream* sink = debugConfig.getSink();

    // Diagnostic options
    static llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> sDiagOpts;
    if (!sDiagOpts) {
        sDiagOpts = llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions>(new clang::DiagnosticOptions());
        sDiagOpts->ShowColors = 0; // cleaner logs
    }

    createNormalizedASTContext(fileName);

    clang::tooling::ClangTool tool(*m_compDB, {fileName});

    // Build the diagnostic consumer using the shared sink
    std::unique_ptr<clang::DiagnosticConsumer> diagPrinter =
        std::make_unique<clang::TextDiagnosticPrinter>(*sink, &*sDiagOpts);

    // Hand ownership of the consumer to the tool
    tool.setDiagnosticConsumer(diagPrinter.release());

    // Make logged diagnostics clean and informative
    tool.appendArgumentsAdjuster(
        clang::tooling::getInsertArgumentAdjuster("-fno-color-diagnostics"));
    tool.appendArgumentsAdjuster(
        clang::tooling::getInsertArgumentAdjuster("-fno-caret-diagnostics"));
    tool.appendArgumentsAdjuster(
        clang::tooling::getInsertArgumentAdjuster("-fdiagnostics-show-note-include-stack"));
    tool.appendArgumentsAdjuster(
        clang::tooling::getInsertArgumentAdjuster("-fdiagnostics-absolute-paths"));

    //suppress ClangTool'son stderr
    tool.setPrintErrorMessage(false);
    int rc = tool.run(new NormalizeActionFactory(this, fileName));
    if (rc != 0) {
        armor::error() << "Error while processing " << fileName << "." << "\n";
        debugConfig.flush();
        return rc == 1 ? FATAL_ERRORS : NO_FATAL_ERRORS;
    }
    debugConfig.flush();

    return NO_FATAL_ERRORS;

}

beta::ASTNormalizedContext* beta::APISession::getContext(const std::string& fileName) const {
    auto it = m_contexts.find(fileName);
    if (it != m_contexts.end())  return it->second.get();
    else {
        throw std::out_of_range("AST context does not exist for file: " + fileName);
    }
}

beta::ASTNormalizedContext* beta::APISession::getContext(llvm::StringRef fileName) const {
    auto it = m_contexts.find(fileName);
    if (it != m_contexts.end())  return it->second.get();
    else {
        throw std::out_of_range("AST context does not exist for file: " + fileName.str());
    }
}
