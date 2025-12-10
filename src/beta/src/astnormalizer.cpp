// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include<iostream>
#include <llvm-14/llvm/Support/Casting.h>

#include "astnormalizer.hpp"
#include "node.hpp"
#include "session.hpp"
#include "tree_builder.hpp"
#include "clang/AST/RecursiveASTVisitor.h"

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
    : session(session), context(context) {}

std::unique_ptr<clang::ASTConsumer> beta::NormalizeAction::CreateASTConsumer(clang::CompilerInstance &, clang::StringRef) {
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

// === Visit and Traverse Methods ===
bool beta::ASTNormalize::TraverseNamespaceDecl(clang::NamespaceDecl *Decl) {
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseNamespaceDecl(Decl);
    return true;
}

bool beta::ASTNormalize::TraverseCXXRecordDecl(clang::CXXRecordDecl *Decl) {
    
    RecursiveASTVisitor<beta::ASTNormalize>::TraverseCXXRecordDecl(Decl);

    if(treeBuilder.IsFromMainFileAndNotLocal(Decl) && !Decl->isClass() 
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

bool beta::ASTNormalize::VisitNamespaceDecl(clang::NamespaceDecl *Decl) {
    return false;
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
