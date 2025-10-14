// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include<iostream>
#include <llvm-14/llvm/Support/Casting.h>

#include "astnormalizer.hpp"
#include "node.hpp"
#include "tree_builder.hpp"

// --- ASTNormalize ---
ASTNormalize::ASTNormalize(APISession* session, ASTNormalizedContext* context, clang::ASTContext* clangContext)
    : session(session), context(context), clangContext(clangContext), treeBuilder(TreeBuilder(context)) {}
// (Implementation of visitor methods remains the same conceptually)


// --- ASTNormalizeConsumer ---
// Constructor simply stores the pointers.
ASTNormalizeConsumer::ASTNormalizeConsumer(APISession* session, ASTNormalizedContext* context)
    : session(session), context(context) {}

void ASTNormalizeConsumer::HandleTranslationUnit(clang::ASTContext &clangContext) {
    // Creates the visitor, passing along the pointers to the session and the pre-existing context.
    context->addClangASTContext(&clangContext);
    ASTNormalize visitor(session, context, &clangContext);
    visitor.TraverseDecl(clangContext.getTranslationUnitDecl());
}


// --- NormalizeAction ---
// Constructor now receives the pre-existing context pointer.
NormalizeAction::NormalizeAction(APISession* session, ASTNormalizedContext* context)
    : session(session), context(context) {}

std::unique_ptr<clang::ASTConsumer> NormalizeAction::CreateASTConsumer(clang::CompilerInstance &, clang::StringRef) {
    // No creation happens here. It just passes the pointers it already has to the consumer.
    return std::make_unique<ASTNormalizeConsumer>(session, context);
}

// --- NormalizeActionFactory (The "Get and Pass" Logic) ---
NormalizeActionFactory::NormalizeActionFactory(APISession* session, const std::string& fileName) : session(session), fileName(fileName) {}

std::unique_ptr<clang::FrontendAction> NormalizeActionFactory::create() {
    // 2. Use the filename to get the pre-existing context from the session.
    ASTNormalizedContext* contextForThisFile = session->getContext(fileName);
    
    // --- Error Handling ---
    if (!contextForThisFile) {
        // This is a critical logic error. The context should have been created before processing.
        throw std::runtime_error("No ASTNormalizedContext was created for file: " + fileName);
    }

    // 3. Create the action, efficiently passing pointers to the session and the retrieved context.
    return std::make_unique<NormalizeAction>(session, contextForThisFile);
}

// === Visit and Traverse Methods ===
bool ASTNormalize::TraverseNamespaceDecl(clang::NamespaceDecl *Decl) {
    return true;
}

bool ASTNormalize::TraverseCXXRecordDecl(clang::CXXRecordDecl *Decl) {
    
    bool traversed = RecursiveASTVisitor<ASTNormalize>::TraverseCXXRecordDecl(Decl);

    if(traversed){
        treeBuilder.PopName();
        treeBuilder.PopNode();
    }

    return true;
}

bool ASTNormalize::TraverseCXXConstructorDecl(clang::CXXConstructorDecl *CD) {
    return true;
}
    
bool ASTNormalize::TraverseEnumDecl(clang::EnumDecl *Decl){
    RecursiveASTVisitor<ASTNormalize>::TraverseEnumDecl(Decl);
    return true;
}

bool ASTNormalize::TraverseFunctionDecl(clang::FunctionDecl *Decl){
    RecursiveASTVisitor<ASTNormalize>::TraverseFunctionDecl(Decl);
    return true;
}

bool ASTNormalize::TraverseTypeAliasDecl(clang::TypeAliasDecl *Decl){
    RecursiveASTVisitor<ASTNormalize>::TraverseTypeAliasDecl(Decl);
    return true;
}

bool ASTNormalize::TraverseVarDecl(clang::VarDecl *Decl){
    RecursiveASTVisitor<ASTNormalize>::TraverseVarDecl(Decl);
    return true;
}

bool ASTNormalize::TraverseFieldDecl(clang::FieldDecl *Decl){
    RecursiveASTVisitor<ASTNormalize>::TraverseFieldDecl(Decl);
    return true;
}

bool ASTNormalize::TraverseTypedefDecl(clang::TypedefDecl *Decl){
    RecursiveASTVisitor<ASTNormalize>::TraverseTypedefDecl(Decl);
    return true;
}

bool ASTNormalize::VisitNamespaceDecl(clang::NamespaceDecl *Decl) {
    return true;
}

bool ASTNormalize::VisitCXXRecordDecl(clang::CXXRecordDecl *Decl) {
    return treeBuilder.BuildCXXRecordNode(Decl);
}

bool ASTNormalize::VisitCXXConstructorDecl(clang::CXXConstructorDecl *CD) {
    return true;
}

bool ASTNormalize::VisitEnumDecl(clang::EnumDecl *Decl) {
    return treeBuilder.BuildEnumNode(Decl);
}

bool ASTNormalize::VisitFunctionDecl(clang::FunctionDecl *Decl) {
    return treeBuilder.BuildFunctionNode(Decl);
}

bool ASTNormalize::VisitTypeAliasDecl(clang::TypeAliasDecl *Decl) {
    return true;
}

bool ASTNormalize::VisitTypedefDecl(clang::TypedefDecl *Decl) {
    return treeBuilder.BuildTypedefDecl(Decl);
}

bool ASTNormalize::VisitVarDecl(clang::VarDecl *Decl) {
    return treeBuilder.BuildVarDecl(Decl);
}

bool ASTNormalize::VisitFieldDecl(clang::FieldDecl *Decl) {
    return treeBuilder.BuildFieldDecl(Decl);
}
