// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "astnormalizer.hpp"
#include "session.hpp"
#include "preprocesor.hpp"

#include "clang/Frontend/CompilerInstance.h"

namespace armor { namespace alpha {

// --- ASTNormalizeConsumer ---

ASTNormalizeConsumer::ASTNormalizeConsumer(armor::APISession* session, armor::ASTNormalizedContext* context)
    : session(session), context(context) {}

void ASTNormalizeConsumer::HandleTranslationUnit(clang::ASTContext &clangContext) {
    context->addClangASTContext(&clangContext);
    // Alpha is parse-error detection only: no AST traversal, no node building.
    // Fatal include failures are already collected by ASTNormalizerPreprocessor.
}


// --- NormalizeAction ---

NormalizeAction::NormalizeAction(armor::APISession* session, armor::ASTNormalizedContext* context)
    : session(session), context(context) {}

std::unique_ptr<clang::ASTConsumer> NormalizeAction::CreateASTConsumer(
    clang::CompilerInstance &CI, clang::StringRef)
{
    auto preprocessor = std::make_unique<ASTNormalizerPreprocessor>(
        &CI.getSourceManager(), context);
    CI.getPreprocessor().addPPCallbacks(std::move(preprocessor));
    return std::make_unique<ASTNormalizeConsumer>(session, context);
}


// --- NormalizeActionFactory ---

NormalizeActionFactory::NormalizeActionFactory(armor::APISession* session, const std::string& fileName)
    : session(session), fileName(fileName) {}

std::unique_ptr<clang::FrontendAction> NormalizeActionFactory::create() {
    armor::ASTNormalizedContext* contextForThisFile = session->getAlphaContext(fileName);
    if (!contextForThisFile) {
        throw std::runtime_error("No armor::ASTNormalizedContext was created for file: " + fileName);
    }
    return std::make_unique<NormalizeAction>(session, contextForThisFile);
}

std::unique_ptr<clang::tooling::FrontendActionFactory>
createNormalizeActionFactory(armor::APISession* session, const std::string& fileName) {
    return std::make_unique<NormalizeActionFactory>(session, fileName);
}

} } // namespace armor::alpha
