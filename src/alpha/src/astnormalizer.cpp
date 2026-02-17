// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "astnormalizer.hpp"
#include "node.hpp"
#include "session.hpp"
#include "tree_builder.hpp"
#include "preprocesor.hpp"

#include <llvm-14/llvm/Support/raw_ostream.h>
#include "clang/Frontend/CompilerInstance.h"

// --- alpha::ASTNormalize ---
alpha::ASTNormalize::ASTNormalize(alpha::APISession* session, alpha::ASTNormalizedContext* context, clang::ASTContext* clangContext)
    : session(session), context(context), clangContext(clangContext), treeBuilder(alpha::TreeBuilder(context)) {}
// (Implementation of visitor methods remains the same conceptually)


// --- alpha::ASTNormalizeConsumer ---
// Constructor simply stores the pointers.
alpha::ASTNormalizeConsumer::ASTNormalizeConsumer(alpha::APISession* session, alpha::ASTNormalizedContext* context)
    : session(session), context(context) {}

void alpha::ASTNormalizeConsumer::HandleTranslationUnit(clang::ASTContext &clangContext) {
    // Creates the visitor, passing along the pointers to the session and the pre-existing context.
    context->addClangASTContext(&clangContext);
    alpha::ASTNormalize visitor(session, context, &clangContext);
    visitor.TraverseDecl(clangContext.getTranslationUnitDecl());
}


// --- NormalizeAction ---
// Constructor now receives the pre-existing context pointer.
alpha::NormalizeAction::NormalizeAction(alpha::APISession* session, alpha::ASTNormalizedContext* context)
    : session(session), context(context) {}

std::unique_ptr<clang::ASTConsumer> alpha::NormalizeAction::CreateASTConsumer(clang::CompilerInstance & CI, clang::StringRef) {
    // Set up preprocessor callbacks
    auto preprocessor = std::make_unique<alpha::ASTNormalizerPreprocessor>(&CI.getSourceManager(), context);
    CI.getPreprocessor().addPPCallbacks(std::move(preprocessor));
    
    return std::make_unique<alpha::ASTNormalizeConsumer>(session, context);
}

// --- NormalizeActionFactory (The "Get and Pass" Logic) ---
alpha::NormalizeActionFactory::NormalizeActionFactory(alpha::APISession* session, const std::string& fileName) : session(session), fileName(fileName) {}

std::unique_ptr<clang::FrontendAction> alpha::NormalizeActionFactory::create() {
    // 2. Use the filename to get the pre-existing context from the session.
    alpha::ASTNormalizedContext* contextForThisFile = session->getContext(fileName);
    
    // --- Error Handling ---
    if (!contextForThisFile) {
        // This is a critical logic error. The context should have been created before processing.
        throw std::runtime_error("No alpha::ASTNormalizedContext was created for file: " + fileName);
    }

    // 3. Create the action, efficiently passing pointers to the session and the retrieved context.
    return std::make_unique<NormalizeAction>(session, contextForThisFile);
}

// === Visit and Traverse Methods ===
bool alpha::ASTNormalize::TraverseNamespaceDecl(clang::NamespaceDecl *Decl) {
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseNamespaceDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseCXXRecordDecl(clang::CXXRecordDecl *Decl) {
    
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseCXXRecordDecl(Decl);

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);
    Decl->printName(OS);

    if(treeBuilder.IsFromMainFileAndNotLocal(Decl) && !Decl->isClass() && !Decl->isTemplateDecl() 
    && Decl->isThisDeclarationADefinition() && !llvm::isa<clang::ClassTemplateSpecializationDecl>(Decl)
    && !nameBuf.empty()){
        treeBuilder.PopName();
        treeBuilder.PopNode();
    }

    return true;
}

bool alpha::ASTNormalize::TraverseCXXConstructorDecl(clang::CXXConstructorDecl *Decl) {
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseCXXConstructorDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseCXXMethodDecl(clang::CXXMethodDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseCXXMethodDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseClassTemplateDecl(clang::ClassTemplateDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseClassTemplateDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseClassTemplateSpecializationDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseClassTemplatePartialSpecializationDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseFunctionTemplateDecl(clang::FunctionTemplateDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseFunctionTemplateDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseEnumDecl(clang::EnumDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseEnumDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseFunctionDecl(clang::FunctionDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseFunctionDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseTypeAliasDecl(clang::TypeAliasDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseTypeAliasDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseVarDecl(clang::VarDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseVarDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseFieldDecl(clang::FieldDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseFieldDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::TraverseTypedefDecl(clang::TypedefDecl *Decl){
    RecursiveASTVisitor<alpha::ASTNormalize>::TraverseTypedefDecl(Decl);
    return true;
}

bool alpha::ASTNormalize::VisitNamespaceDecl(clang::NamespaceDecl *Decl) {
    return false;
}

bool alpha::ASTNormalize::VisitCXXRecordDecl(clang::CXXRecordDecl *Decl) {
    return treeBuilder.BuildCXXRecordNode(Decl);
}

bool alpha::ASTNormalize::VisitEnumDecl(clang::EnumDecl *Decl) {
    return treeBuilder.BuildEnumNode(Decl);
}

bool alpha::ASTNormalize::VisitFunctionDecl(clang::FunctionDecl *Decl) {
    return treeBuilder.BuildFunctionNode(Decl);
}

bool alpha::ASTNormalize::VisitTypeAliasDecl(clang::TypeAliasDecl *Decl) {
    return true;
}

bool alpha::ASTNormalize::VisitTypedefDecl(clang::TypedefDecl *Decl) {
    return treeBuilder.BuildTypedefDecl(Decl);
}

bool alpha::ASTNormalize::VisitVarDecl(clang::VarDecl *Decl) {
    return treeBuilder.BuildVarDecl(Decl);
}

bool alpha::ASTNormalize::VisitFieldDecl(clang::FieldDecl *Decl) {
    return treeBuilder.BuildFieldDecl(Decl);
}