// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "ast_normalized_context.hpp"
#include "comm_def.hpp"

#include "clang/Tooling/CompilationDatabase.h"

// Forward declarations to avoid including heavy Clang headers here
namespace clang { namespace tooling { class CompilationDatabase; } }

/**
 * @class APISession
 * @brief Manages a session for comparing multiple source files.
 *
 * This class orchestrates the process of parsing and normalizing ASTs from
 * one or more source files. It holds:
 *
 * 1. A map from filenames to their corresponding `ASTNormalizedContext`.
 * 2. A central string pool to unify and reduce memory usage for common strings.
 * 3. A reference to the Clang Compilation Database used for parsing.
 */
namespace beta{

class APISession {
public:
    /**
     * @brief Constructs an APISession.
     * @param compDB A unique_ptr to the compilation database to be used for all parsing.
     *        The session takes ownership of the database.
     */
    /**
     * @brief Processes a source file, normalizing its AST and storing the context.
     *
     * Runs a Clang tool on the specified file, populates an ASTNormalizedContext,
     * and stores it in the session, mapped by the filename.
     *
     * @param filename The path to the source file to process.
     */
    PARSING_STATUS processFile(std::string fileName, std::unique_ptr<clang::tooling::FixedCompilationDatabase> m_compDB);

    /**
     * @brief Retrieves the normalized context for a previously processed file.
     * @param filename The path to the source file.
     * @return A pointer to the ASTNormalizedContext if the file was processed,
     *         otherwise nullptr.
     */
    beta::ASTNormalizedContext* getContext(llvm::StringRef fileName) const;

    beta::ASTNormalizedContext* getContext(const std::string& fileName) const;


    void createNormalizedASTContext(const std::string& key);

private:
    // A map from a filename to its fully normalized AST context
    llvm::StringMap<std::unique_ptr<beta::ASTNormalizedContext>> m_contexts;
};

}