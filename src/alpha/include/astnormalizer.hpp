// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Tooling/Tooling.h"

#include "node.hpp"
#include "ast_normalized_context.hpp"
#include "session.hpp"
#include "tree_builder.hpp"

namespace alpha{

class ASTNormalize : public clang::RecursiveASTVisitor<ASTNormalize> {
    public:

        alpha::APISession* session;
        ASTNormalizedContext* context;
        clang::ASTContext *clangContext;

        alpha::TreeBuilder treeBuilder;

        ASTNormalize(alpha::APISession* session, alpha::ASTNormalizedContext* context, clang::ASTContext* clangContext);

        bool TraverseNamespaceDecl(clang::NamespaceDecl *Decl);
        bool TraverseCXXRecordDecl(clang::CXXRecordDecl *Decl);
        bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl *Decl);
        bool TraverseCXXMethodDecl(clang::CXXMethodDecl *Decl);
        bool TraverseFunctionTemplateDecl(clang::FunctionTemplateDecl *Decl);
        bool TraverseEnumDecl(clang::EnumDecl *Decl);
        bool TraverseFunctionDecl(clang::FunctionDecl *Decl);
        bool TraverseTypeAliasDecl(clang::TypeAliasDecl *Decl);
        bool TraverseVarDecl(clang::VarDecl *Decl);
        bool TraverseFieldDecl(clang::FieldDecl *Decl);
        bool TraverseTypedefDecl(clang::TypedefDecl *Decl);
        bool TraverseClassTemplateDecl(clang::ClassTemplateDecl *Decl);
        bool TraverseClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl *Decl);
        bool TraverseClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl *Decl);

        bool VisitNamespaceDecl(clang::NamespaceDecl *Decl);
        bool VisitCXXRecordDecl(clang::CXXRecordDecl *Decl);
        bool VisitEnumDecl(clang::EnumDecl *Decl);
        bool VisitFieldDecl(clang::FieldDecl *Decl);
        bool VisitFunctionDecl(clang::FunctionDecl *Decl);
        bool VisitTypeAliasDecl(clang::TypeAliasDecl *Decl);
        bool VisitVarDecl(clang::VarDecl *Decl);
        bool VisitTypedefDecl(clang::TypedefDecl *Decl);
};

class ASTNormalizeConsumer : public clang::ASTConsumer {
    public:
        alpha::APISession* session;
        alpha::ASTNormalizedContext* context;
        ASTNormalize *visitor;
        ASTNormalizeConsumer(alpha::APISession* session, alpha::ASTNormalizedContext* context);
        void HandleTranslationUnit(clang::ASTContext &Context) override;
};


class NormalizeAction : public clang::ASTFrontendAction {
    public:
        alpha::APISession* session;
        ASTNormalizedContext* context;
        NormalizeAction(alpha::APISession* session, alpha::ASTNormalizedContext* context);
        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &, clang::StringRef) override;
};

class NormalizeActionFactory : public clang::tooling::FrontendActionFactory {
    public:
        alpha::APISession* session;
        const std::string& fileName;
        explicit NormalizeActionFactory(alpha::APISession* session, const std::string& fileName);
        std::unique_ptr<clang::FrontendAction> create() override;
};

}