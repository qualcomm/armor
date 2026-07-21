// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/Tooling/Tooling.h"
#include <memory>

#include "node.hpp"
#include "ast_normalized_context.hpp"
#include "session.hpp"
#include "tree_builder.hpp"
#include "comment_handler.hpp"
#include "preprocesor.hpp"

namespace armor { namespace beta {

class ASTNormalize : public clang::RecursiveASTVisitor<ASTNormalize> {
    public:

        armor::APISession* session;
        armor::ASTNormalizedContext* context;
        clang::ASTContext *clangContext;
        beta::TreeBuilder treeBuilder;

        ASTNormalize(armor::APISession* session, armor::ASTNormalizedContext* context, clang::ASTContext* clangContext);

        bool TraverseNamespaceDecl(clang::NamespaceDecl *Decl);
        bool TraverseRecordDecl(clang::RecordDecl *Decl);
        bool TraverseCXXRecordDecl(clang::CXXRecordDecl *Decl);
        bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl *Decl);
        bool TraverseCXXMethodDecl(clang::CXXMethodDecl* Decl);
        bool TraverseFunctionTemplateDecl(clang::FunctionTemplateDecl *Decl);
        bool TraverseEnumDecl(clang::EnumDecl *Decl);
        bool TraverseFunctionDecl(clang::FunctionDecl *Decl);
        bool TraverseTypeAliasDecl(clang::TypeAliasDecl *Decl);
        bool TraverseVarDecl(clang::VarDecl *Decl);
        bool TraverseFriendDecl(clang::FriendDecl* Decl);
        bool TraverseFieldDecl(clang::FieldDecl *Decl);
        bool TraverseTypedefDecl(clang::TypedefDecl *Decl);
        bool TraverseClassTemplateDecl(clang::ClassTemplateDecl *Decl);
        bool TraverseClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl *Decl);
        bool TraverseClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl *Decl);
        bool TraverseUsingDecl(clang::UsingDecl *Decl);
        bool TraverseUsingDirectiveDecl(clang::UsingDirectiveDecl *Decl);
        bool TraverseNamespaceAliasDecl(clang::NamespaceAliasDecl *Decl);
        bool TraverseStaticAssertDecl(clang::StaticAssertDecl *Decl);
        bool TraverseVarTemplateDecl(clang::VarTemplateDecl *Decl);
        bool TraverseVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl *Decl);
        bool TraverseVarTemplatePartialSpecializationDecl(clang::VarTemplatePartialSpecializationDecl *Decl);
        bool TraverseTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *Decl);
        bool TraverseCXXDeductionGuideDecl(clang::CXXDeductionGuideDecl *Decl);
        bool TraverseTemplateTypeParmDecl(clang::TemplateTypeParmDecl *Decl);
        bool TraverseNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl *Decl);
        bool TraverseTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl *Decl);

        bool VisitNamespaceDecl(clang::NamespaceDecl *Decl);
        bool VisitRecordDecl(clang::RecordDecl *Decl);
        bool VisitCXXRecordDecl(clang::CXXRecordDecl *Decl);
        bool VisitEnumDecl(clang::EnumDecl *Decl);
        bool VisitFieldDecl(clang::FieldDecl *Decl);
        bool VisitFriendDecl(clang::FriendDecl* Decl);
        bool VisitFunctionDecl(clang::FunctionDecl *Decl);
        bool VisitTypeAliasDecl(clang::TypeAliasDecl *Decl);
        bool VisitVarDecl(clang::VarDecl *Decl);
        bool VisitTypedefDecl(clang::TypedefDecl *Decl);
        bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl *Decl);
        bool VisitClassTemplateDecl(clang::ClassTemplateDecl *Decl);
        bool VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl *Decl);
        bool VisitClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl *Decl);
        bool VisitUsingDecl(clang::UsingDecl *Decl);
        bool VisitUsingDirectiveDecl(clang::UsingDirectiveDecl *Decl);
        bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl *Decl);
        bool VisitStaticAssertDecl(clang::StaticAssertDecl *Decl);
        bool VisitVarTemplateDecl(clang::VarTemplateDecl *Decl);
        bool VisitVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl *Decl);
        bool VisitVarTemplatePartialSpecializationDecl(clang::VarTemplatePartialSpecializationDecl *Decl);
        bool VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *Decl);
};

class ASTNormalizeConsumer : public clang::ASTConsumer {
    public:
        armor::APISession* session;
        armor::ASTNormalizedContext* context;
        ASTNormalize *visitor;
        ASTNormalizeConsumer(armor::APISession* session, armor::ASTNormalizedContext* context);
        void HandleTranslationUnit(clang::ASTContext &Context) override;
};


class NormalizeAction : public clang::ASTFrontendAction {
    public:
        armor::APISession* session;
        armor::ASTNormalizedContext* context;
        beta::CommentHandler* commentHandler;
        beta::ASTNormalizerPreprocessor* preprocessor;

        NormalizeAction(armor::APISession* session, armor::ASTNormalizedContext* context);
        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &, clang::StringRef) override;
        void EndSourceFileAction() override;

    private:
        clang::CompilerInstance* CI;
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

} } // namespace armor::beta
