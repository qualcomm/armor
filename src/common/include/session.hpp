// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "ast_normalized_context.hpp"
#include "comm_def.hpp"
#include "clang/Tooling/CompilationDatabase.h"

namespace clang { namespace tooling {
    class FixedCompilationDatabase;
    class FrontendActionFactory;
} }

namespace armor {

/**
 * @class APISession
 * @brief Unified session managing both alpha and beta parser contexts per file.
 *
 * - processFileAlpha: runs the alpha preprocessor-only pass (detects fatal includes),
 *   using the caller-supplied FrontendActionFactory (armor::alpha::createNormalizeActionFactory).
 * - processFileBeta:  runs the full beta AST + node-tree building pass,
 *   using the caller-supplied FrontendActionFactory (armor::beta::createNormalizeActionFactory).
 *
 * Each file gets two independent ASTNormalizedContext entries, one per parser.
 * Alpha contexts only contain SourceRangeTracker data.
 * Beta contexts contain the full node tree plus source range data.
 */
class APISession {
public:
    PARSING_STATUS processFileAlpha(std::string fileName,
                                    std::unique_ptr<clang::tooling::FixedCompilationDatabase> compDB,
                                    std::unique_ptr<clang::tooling::FrontendActionFactory> factory);

    PARSING_STATUS processFileBeta(std::string fileName,
                                   std::unique_ptr<clang::tooling::FixedCompilationDatabase> compDB,
                                   std::unique_ptr<clang::tooling::FrontendActionFactory> factory);

    ASTNormalizedContext* getAlphaContext(const std::string& fileName) const;
    ASTNormalizedContext* getAlphaContext(llvm::StringRef fileName) const;

    ASTNormalizedContext* getBetaContext(const std::string& fileName) const;
    ASTNormalizedContext* getBetaContext(llvm::StringRef fileName) const;

    void createAlphaContext(const std::string& key);
    void createBetaContext(const std::string& key);

private:
    llvm::StringMap<std::unique_ptr<ASTNormalizedContext>> m_alphaContexts;
    llvm::StringMap<std::unique_ptr<ASTNormalizedContext>> m_betaContexts;
};

} // namespace armor
