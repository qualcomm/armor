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
#include "debug_config.hpp"
#include "user_print.hpp"


void alpha::APISession::createNormalizedASTContext(const std::string& key){
    const auto pair = m_contexts.try_emplace(key, std::make_unique<ASTNormalizedContext>());
    if (!pair.second) {
        throw std::runtime_error("AST context already exists for key: " + key);
    }
}

PARSING_STATUS alpha::APISession::processFile(std::string fileName, std::unique_ptr<clang::tooling::FixedCompilationDatabase> m_compDB) {
    const std::string diagLogPath = std::string("debug_output/logs/diagnostics.log");

    if (!diagLogPath.empty()) {
        llvm::SmallString<256> p(diagLogPath);
        llvm::StringRef dir = llvm::sys::path::parent_path(p);
        if (!dir.empty()) {
            llvm::sys::fs::create_directories(dir);
        }
    }

    // Prefer the global/shared sink if already set by the entry point
    llvm::raw_ostream* sink = DebugConfig::instance().getSink();

    // Fallback: create our own file stream if no sink yet
    static std::unique_ptr<llvm::raw_fd_ostream> sDiagStream;
    if (!sink) {
        if (!sDiagStream) {
            std::error_code EC;
            auto stream = std::make_unique<llvm::raw_fd_ostream>(
                diagLogPath, EC, llvm::sys::fs::OF_Text | llvm::sys::fs::OF_Append);
            if (EC) {
                USER_ERROR(std::string("Warning: cannot open diagnostics log '") +
                           diagLogPath + "': " + EC.message() +
                           " (using stderr for Clang diagnostics)");
            } else {
                sDiagStream = std::move(stream);
            }
        }
        sink = sDiagStream ? static_cast<llvm::raw_ostream*>(sDiagStream.get())
                           : static_cast<llvm::raw_ostream*>(&llvm::errs());
        // Also let DebugConfig share it so logs and diagnostics stay unified
        DebugConfig::instance().setSink(sink);
    }

    // Diagnostic options
    static llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> sDiagOpts;
    if (!sDiagOpts) {
        sDiagOpts = llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions>(new clang::DiagnosticOptions());
        sDiagOpts->ShowColors = 0; // cleaner logs
    }

    // Now it is safe to do any logging before/after this point
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
        DebugConfig::instance().log(
            std::string("Error while processing ") + fileName + ".",
            DebugConfig::Level::ERROR
        );
        if (sDiagStream) sDiagStream->flush();
        return rc == 1 ? FATAL_ERRORS : NO_FATAL_ERRORS;
    }
    if (sDiagStream) sDiagStream->flush();

    return NO_FATAL_ERRORS;

}

alpha::ASTNormalizedContext* alpha::APISession::getContext(const std::string& fileName) const {
    auto it = m_contexts.find(fileName);
    if (it != m_contexts.end())  return it->second.get();
    else {
        throw std::out_of_range("AST context does not exist for file: " + fileName);
    }
}

alpha::ASTNormalizedContext* alpha::APISession::getContext(llvm::StringRef fileName) const {
    auto it = m_contexts.find(fileName);
    if (it != m_contexts.end())  return it->second.get();
    else {
        throw std::out_of_range("AST context does not exist for file: " + fileName.str());
    }
}
