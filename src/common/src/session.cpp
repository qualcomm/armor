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
#include "ast_normalized_context.hpp"
#include "logger.hpp"

namespace {

void setupClangTool(clang::tooling::ClangTool& tool, llvm::raw_ostream* sink,
                    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagOpts) {
    std::unique_ptr<clang::DiagnosticConsumer> diagPrinter =
        std::make_unique<clang::TextDiagnosticPrinter>(*sink, &*diagOpts);
    tool.setDiagnosticConsumer(diagPrinter.release());
    tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster("-fno-color-diagnostics"));
    tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster("-fno-caret-diagnostics"));
    tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster("-fdiagnostics-show-note-include-stack"));
    tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster("-fdiagnostics-absolute-paths"));
    tool.setPrintErrorMessage(false);
}

} // namespace

static llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> sDiagOpts;

static void ensureDiagOpts() {
    if (!sDiagOpts) {
        sDiagOpts = llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions>(new clang::DiagnosticOptions());
        sDiagOpts->ShowColors = 0;
    }
}

void armor::APISession::createAlphaContext(const std::string& key) {
    const auto pair = m_alphaContexts.try_emplace(key, std::make_unique<ASTNormalizedContext>());
    if (!pair.second) {
        throw std::runtime_error("Alpha AST context already exists for key: " + key);
    }
}

void armor::APISession::createBetaContext(const std::string& key) {
    const auto pair = m_betaContexts.try_emplace(key, std::make_unique<ASTNormalizedContext>());
    if (!pair.second) {
        throw std::runtime_error("Beta AST context already exists for key: " + key);
    }
}

PARSING_STATUS armor::APISession::processFileAlpha(
    std::string fileName,
    std::unique_ptr<clang::tooling::FixedCompilationDatabase> compDB,
    std::unique_ptr<clang::tooling::FrontendActionFactory> factory)
{
    DebugConfig& debugConfig = DebugConfig::getInstance();
    llvm::raw_ostream* sink = debugConfig.getSink();
    ensureDiagOpts();

    createAlphaContext(fileName);

    clang::tooling::ClangTool tool(*compDB, {fileName});
    setupClangTool(tool, sink, sDiagOpts);

    int rc = tool.run(factory.get());
    if (rc != 0) {
        armor::error() << "Alpha: error while processing " << fileName << ".\n";
        debugConfig.flush();
        return rc == 1 ? FATAL_ERRORS : NO_FATAL_ERRORS;
    }
    debugConfig.flush();
    return NO_FATAL_ERRORS;
}

PARSING_STATUS armor::APISession::processFileBeta(
    std::string fileName,
    std::unique_ptr<clang::tooling::FixedCompilationDatabase> compDB,
    std::unique_ptr<clang::tooling::FrontendActionFactory> factory)
{
    DebugConfig& debugConfig = DebugConfig::getInstance();
    llvm::raw_ostream* sink = debugConfig.getSink();
    ensureDiagOpts();

    createBetaContext(fileName);

    clang::tooling::ClangTool tool(*compDB, {fileName});
    setupClangTool(tool, sink, sDiagOpts);

    int rc = tool.run(factory.get());
    if (rc != 0) {
        armor::error() << "Beta: error while processing " << fileName << ".\n";
        debugConfig.flush();
        return rc == 1 ? FATAL_ERRORS : NO_FATAL_ERRORS;
    }
    debugConfig.flush();
    return NO_FATAL_ERRORS;
}

armor::ASTNormalizedContext* armor::APISession::getAlphaContext(const std::string& fileName) const {
    auto it = m_alphaContexts.find(fileName);
    if (it != m_alphaContexts.end()) return it->second.get();
    throw std::out_of_range("Alpha AST context does not exist for file: " + fileName);
}

armor::ASTNormalizedContext* armor::APISession::getAlphaContext(llvm::StringRef fileName) const {
    auto it = m_alphaContexts.find(fileName);
    if (it != m_alphaContexts.end()) return it->second.get();
    throw std::out_of_range("Alpha AST context does not exist for file: " + fileName.str());
}

armor::ASTNormalizedContext* armor::APISession::getBetaContext(const std::string& fileName) const {
    auto it = m_betaContexts.find(fileName);
    if (it != m_betaContexts.end()) return it->second.get();
    throw std::out_of_range("Beta AST context does not exist for file: " + fileName);
}

armor::ASTNormalizedContext* armor::APISession::getBetaContext(llvm::StringRef fileName) const {
    auto it = m_betaContexts.find(fileName);
    if (it != m_betaContexts.end()) return it->second.get();
    throw std::out_of_range("Beta AST context does not exist for file: " + fileName.str());
}
