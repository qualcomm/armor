// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/Tooling/Tooling.h"

#include "ast_normalized_context.hpp"
#include "session.hpp"
#include "preprocesor.hpp"

namespace armor { namespace alpha {

class ASTNormalizeConsumer : public clang::ASTConsumer {
    public:
        armor::APISession* session;
        armor::ASTNormalizedContext* context;
        ASTNormalizeConsumer(armor::APISession* session, armor::ASTNormalizedContext* context);
        void HandleTranslationUnit(clang::ASTContext &Context) override;
};


class NormalizeAction : public clang::ASTFrontendAction {
    public:
        armor::APISession* session;
        armor::ASTNormalizedContext* context;
        NormalizeAction(armor::APISession* session, armor::ASTNormalizedContext* context);
        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &, clang::StringRef) override;
};

class NormalizeActionFactory : public clang::tooling::FrontendActionFactory {
    public:
        armor::APISession* session;
        const std::string& fileName;
        explicit NormalizeActionFactory(armor::APISession* session, const std::string& fileName);
        std::unique_ptr<clang::FrontendAction> create() override;
};

std::unique_ptr<clang::tooling::FrontendActionFactory>
createNormalizeActionFactory(armor::APISession* session, const std::string& fileName);

} } // namespace armor::alpha
