// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include<iostream>
#include <llvm-14/llvm/Support/Casting.h>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include <memory>

#include "astnormalizer.hpp"
#include "node.hpp"
#include "session.hpp"
#include "tree_builder.hpp"
#include "comment_handler.hpp"
#include "preprocesor.hpp"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"

// --- beta::ASTNormalize ---
beta::ASTNormalize::ASTNormalize(beta::APISession* session, beta::ASTNormalizedContext* context, clang::ASTContext* clangContext)
    : session(session), context(context), clangContext(clangContext), treeBuilder(beta::TreeBuilder(context)) {}
// (Implementation of visitor methods remains the same conceptually)


// --- beta::ASTNormalizeConsumer ---
// Constructor simply stores the pointers.
beta::ASTNormalizeConsumer::ASTNormalizeConsumer(APISession* session, beta::ASTNormalizedContext* context)
    : session(session), context(context) {}

void beta::ASTNormalizeConsumer::HandleTranslationUnit(clang::ASTContext &clangContext) {
    // Creates the visitor, passing along the pointers to the session and the pre-existing context.
    context->addClangASTContext(&clangContext);

    beta::ASTNormalize visitor(session, context, &clangContext);
    visitor.TraverseDecl(clangContext.getTranslationUnitDecl());
}

// --- NormalizeAction ---
// Constructor now receives the pre-existing context pointer.
beta::NormalizeAction::NormalizeAction(APISession* session, beta::ASTNormalizedContext* context)
    : session(session), context(context), commentHandler(nullptr), preprocessor(nullptr), CI(nullptr) {}

std::unique_ptr<clang::ASTConsumer> beta::NormalizeAction::CreateASTConsumer(clang::CompilerInstance& CI, clang::StringRef) {
    // Store CI reference for cleanup
    this->CI = &CI;
    
    // Set up comment handler
    auto commentHandlerPtr = std::make_unique<beta::CommentHandler>(&CI.getSourceManager(), context);
    commentHandler = commentHandlerPtr.get(); // Store reference before releasing
    CI.getPreprocessor().addCommentHandler(commentHandlerPtr.release());
    
    // Set up preprocessor callbacks
    auto preprocessorPtr = std::make_unique<beta::ASTNormalizerPreprocessor>(&CI.getSourceManager(), context);
    preprocessor = preprocessorPtr.get(); // Store reference before moving
    CI.getPreprocessor().addPPCallbacks(std::move(preprocessorPtr)); // Preprocessor takes ownership

    // No creation happens here. It just passes the pointers it already has to the consumer.
    return std::make_unique<beta::ASTNormalizeConsumer>(session, context);
}

// --- NormalizeActionFactory (The "Get and Pass" Logic) ---
beta::NormalizeActionFactory::NormalizeActionFactory(APISession* session, const std::string& fileName) : session(session), fileName(fileName) {}

std::unique_ptr<clang::FrontendAction> beta::NormalizeActionFactory::create() {
    // 2. Use the filename to get the pre-existing context from the session.
    beta::ASTNormalizedContext* contextForThisFile = session->getContext(fileName);
    
    // --- Error Handling ---
    if (!contextForThisFile) {
        // This is a critical logic error. The context should have been created before processing.
        throw std::runtime_error("No beta::ASTNormalizedContext was created for file: " + fileName);
    }

    // 3. Create the action, efficiently passing pointers to the session and the retrieved context.
    return std::make_unique<NormalizeAction>(session, contextForThisFile);
}

void beta::NormalizeAction::EndSourceFileAction() {

    // Finalize preprocessor callbacks (must be done before Preprocessor is destroyed)
    if (preprocessor) {
        preprocessor->finalize();
        // Note: We don't delete preprocessor - the Preprocessor owns it and will clean it up
        preprocessor = nullptr;
    }

    // Finalize and cleanup comment handler
    if (commentHandler && CI) {
        commentHandler->finalize(); // Call finalize before removal
        CI->getPreprocessor().removeCommentHandler(commentHandler);
        delete commentHandler; // Now we can safely delete it
        commentHandler = nullptr;
    }
    
    filterCommentsInInactiveRegions(context, &context->getClangASTContext()->getSourceManager());
    
    // Call parent implementation
    clang::ASTFrontendAction::EndSourceFileAction();
}

// === Visit and Traverse Methods ===
bool beta::ASTNormalize::TraverseNamespaceDecl(clang::NamespaceDecl *Decl) {
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseNamespaceDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseCXXRecordDecl(clang::CXXRecordDecl *Decl) {
    
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseCXXRecordDecl(Decl);
    if(treeBuilder.IsDeclFromMainFileAndNotLocal(Decl) && !Decl->isClass() 
    && !Decl->isTemplated() && !llvm::isa<clang::ClassTemplateSpecializationDecl>(Decl)){
        treeBuilder.PopName();
        treeBuilder.PopNode();
    }

    return true;
}

bool beta::ASTNormalize::TraverseCXXConstructorDecl(clang::CXXConstructorDecl *Decl) {
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseCXXConstructorDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseCXXMethodDecl(clang::CXXMethodDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseCXXMethodDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseClassTemplateDecl(clang::ClassTemplateDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseClassTemplateDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseClassTemplateSpecializationDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseClassTemplatePartialSpecializationDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseFunctionTemplateDecl(clang::FunctionTemplateDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseFunctionTemplateDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseEnumDecl(clang::EnumDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseEnumDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseFunctionDecl(clang::FunctionDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseFunctionDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseTypeAliasDecl(clang::TypeAliasDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseTypeAliasDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseVarDecl(clang::VarDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseVarDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseFieldDecl(clang::FieldDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseFieldDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseTypedefDecl(clang::TypedefDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseTypedefDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseUsingDecl(clang::UsingDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseUsingDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseUsingDirectiveDecl(clang::UsingDirectiveDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseUsingDirectiveDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseNamespaceAliasDecl(clang::NamespaceAliasDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseNamespaceAliasDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseStaticAssertDecl(clang::StaticAssertDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseStaticAssertDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseVarTemplateDecl(clang::VarTemplateDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseVarTemplateDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseVarTemplateSpecializationDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseVarTemplatePartialSpecializationDecl(clang::VarTemplatePartialSpecializationDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseVarTemplatePartialSpecializationDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseTypeAliasTemplateDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseCXXDeductionGuideDecl(clang::CXXDeductionGuideDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseCXXDeductionGuideDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseTemplateTypeParmDecl(clang::TemplateTypeParmDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseTemplateTypeParmDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseNonTypeTemplateParmDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl *Decl){
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseTemplateTemplateParmDecl(Decl);
    return true;
}

// bool beta::ASTNormalize::TraverseCompoundStmt(clang::CompoundStmt *Stmt) {
//     RecursiveASTVisitor<beta::ASTNormalize>::TraverseCompoundStmt(Stmt);
//     return true;
// }

bool beta::ASTNormalize::VisitNamespaceDecl(clang::NamespaceDecl *Decl) {
    treeBuilder.BuildNamespaceDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitCXXRecordDecl(clang::CXXRecordDecl *Decl) {
    return treeBuilder.BuildCXXRecordNode(Decl);
}

bool beta::ASTNormalize::VisitEnumDecl(clang::EnumDecl *Decl) {
    return treeBuilder.BuildEnumNode(Decl);
}

bool beta::ASTNormalize::VisitFunctionDecl(clang::FunctionDecl *Decl) {
    return treeBuilder.BuildFunctionNode(Decl);
}

bool beta::ASTNormalize::VisitTypeAliasDecl(clang::TypeAliasDecl *Decl) {
    treeBuilder.BuildTypeAliasDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitTypedefDecl(clang::TypedefDecl *Decl) {
    return treeBuilder.BuildTypedefDecl(Decl);
}

bool beta::ASTNormalize::VisitVarDecl(clang::VarDecl *Decl) {
    return treeBuilder.BuildVarDecl(Decl);
}

bool beta::ASTNormalize::VisitFieldDecl(clang::FieldDecl *Decl) {
    return treeBuilder.BuildFieldDecl(Decl);
}

bool beta::ASTNormalize::VisitFunctionTemplateDecl(clang::FunctionTemplateDecl *Decl){
    treeBuilder.BuildFunctionTemplateDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitClassTemplateDecl(clang::ClassTemplateDecl *Decl) {
    treeBuilder.BuildClassTemplateDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl *Decl) {
    treeBuilder.BuildClassTemplateSpecializationDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl *Decl) {
    treeBuilder.BuildClassTemplatePartialSpecializationDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitUsingDecl(clang::UsingDecl *Decl) {
    treeBuilder.BuildUsingDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitUsingDirectiveDecl(clang::UsingDirectiveDecl *Decl) {
    treeBuilder.BuildUsingDirectiveDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitNamespaceAliasDecl(clang::NamespaceAliasDecl *Decl) {
    treeBuilder.BuildNamespaceAliasDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitStaticAssertDecl(clang::StaticAssertDecl *Decl) {
    treeBuilder.BuildStaticAssertDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitVarTemplateDecl(clang::VarTemplateDecl *Decl) {
    treeBuilder.BuildVarTemplateDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl *Decl) {
    treeBuilder.BuildVarTemplateSpecializationDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitVarTemplatePartialSpecializationDecl(clang::VarTemplatePartialSpecializationDecl *Decl) {
    treeBuilder.BuildVarTemplatePartialSpecializationDecl(Decl);
    return true;
}

bool beta::ASTNormalize::VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *Decl) {
    treeBuilder.BuildTypeAliasTemplateDecl(Decl);
    return true;
}
