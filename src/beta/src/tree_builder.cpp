// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "tree_builder.hpp"
#include "ast_normalized_context.hpp"
#include "astnormalizer.hpp"
#include "comm_def.hpp"
#include "custom_usr_generator.hpp"
#include "diff_utils.hpp"
#include "tree_builder_utils.hpp"
#include "qualified_name_generator.hpp"
#include "iostream"
#include "node.hpp"
#include "fibonacci_hash.hpp"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Type.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"
#include "type_utils.hpp"
#include <cassert>
#include <cstddef>
#include <llvm-14/llvm/ADT/SmallString.h>
#include <llvm-14/llvm/ADT/SmallVector.h>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <llvm-14/llvm/ADT/StringSet.h>
#include <llvm-14/llvm/Support/Casting.h>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include "logger.hpp"

beta::TreeBuilder::TreeBuilder(beta::ASTNormalizedContext* context): context(context) {}

inline bool beta::TreeBuilder::IsDeclFromMainFileAndNotLocal(const clang::Decl* Decl) {
    clang::ASTContext* clangContext = &Decl->getASTContext();
    return clangContext->getSourceManager().isInMainFile(Decl->getLocation()) && Decl->getParentFunctionOrMethod() == nullptr;
}

inline bool beta::TreeBuilder::IsStmtFromMainFile(const clang::Stmt* Stmt) {
    if (!Stmt) return false;

    clang::SourceManager& SM = context->getClangASTContext()->getSourceManager();
    clang::SourceLocation StartLoc = Stmt->getBeginLoc();

    return StartLoc.isValid() && SM.isInMainFile(StartLoc);
}

inline bool beta::TreeBuilder::isInTemplatedClass(const clang::Decl* Decl){

    const clang::DeclContext* DC = Decl->getDeclContext();

    while(DC && !DC->isTranslationUnit()){

        if(const clang::CXXRecordDecl* RD =
            llvm::dyn_cast_or_null<clang::CXXRecordDecl>(DC); RD && RD->isTemplateDecl()){
            return true;
        }

        if(llvm::isa<clang::ClassTemplateSpecializationDecl>(DC)){
            return true;
        }

        DC = DC->getParent();
    }

    return false;

}

inline bool beta::TreeBuilder::isWrittenInTemplatedClass(const clang::Decl* D) {

    if (!D) return false;

    const clang::DeclContext* LexicalDC = D->getLexicalDeclContext();

    while (LexicalDC && !LexicalDC->isTranslationUnit()) {

        if (const clang::CXXRecordDecl* RD =
            llvm::dyn_cast<clang::CXXRecordDecl>(LexicalDC); RD && RD->isTemplated()) {
            return true;
        }

        if (llvm::isa<clang::ClassTemplateSpecializationDecl>(LexicalDC)) {
            return true;
        }

        LexicalDC = LexicalDC->getParent();
    }

    return false;

}

uint64_t beta::TreeBuilder::generateSemanticHashFromDecl(const clang::Decl* Decl) {
    clang::SourceManager& SM = Decl->getASTContext().getSourceManager();
    clang::SourceLocation StartLoc = Decl->getBeginLoc();
    clang::SourceLocation EndLoc = Decl->getEndLoc();

    llvm::StringRef sourceText;

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);

    if(llvm::isa<clang::NamedDecl>(Decl)){
        llvm::dyn_cast<clang::NamedDecl>(Decl)->printName(OS);
        armor::debug()<< "Excluding : " << nameBuf << "\n";
    }
    else{
        armor::debug() << "Excluding : " << Decl->getDeclKindName() << "\n";
    }

    if (StartLoc.isValid() && EndLoc.isValid()) {
        clang::CharSourceRange Range = clang::CharSourceRange::getTokenRange(StartLoc, EndLoc);
        sourceText = clang::Lexer::getSourceText(Range, SM, Decl->getASTContext().getLangOpts());
        uint64_t semanticHash = FibonacciHash::hashFromSourceRange(&SM,clang::SourceRange(StartLoc, EndLoc));
        TEST_LOG << semanticHash << "\n";
        TEST_LOG << sourceText << "\n----------------------------------------\n";
        return semanticHash;
    }

    return 0;
}

uint64_t beta::TreeBuilder::generateSemanticHashFromStmt(const clang::Stmt* Stmt) {
    if (!Stmt) return 0;

    clang::SourceManager& SM = context->getClangASTContext()->getSourceManager();
    clang::SourceLocation StartLoc = Stmt->getBeginLoc();
    clang::SourceLocation EndLoc = Stmt->getEndLoc();

    llvm::StringRef sourceText;

    if (StartLoc.isValid() && EndLoc.isValid()) {
        clang::CharSourceRange Range = clang::CharSourceRange::getTokenRange(StartLoc, EndLoc);
        sourceText = clang::Lexer::getSourceText(Range, SM, context->getClangASTContext()->getLangOpts());
        uint64_t semanticHash = FibonacciHash::hashFromSourceRange(&SM, clang::SourceRange(StartLoc, EndLoc));
        TEST_LOG << semanticHash << "\n";
        TEST_LOG << sourceText << "\n----------------------------------------\n";
        return semanticHash;
    }

    return 0;
}

void beta::TreeBuilder::processUnhandledDecl(const clang::Decl* Decl) {
    uint64_t hash = generateSemanticHashFromDecl(Decl);
    context->getSourceRangeTracker().addUnhandledDeclHash(hash);
}

void beta::TreeBuilder::processUnhandledStmt(const clang::Stmt* Stmt, const std::shared_ptr<beta::APINode>& node) {
    uint64_t hash = generateSemanticHashFromStmt(Stmt);
    context->getSourceRangeTracker().addUnhandledDeclHash(hash);
    node->stmtHashes.emplace_back(hash);
}

inline void beta::TreeBuilder::AddNode(const std::shared_ptr<APINode>& node) {

    assert(!node->NSR.empty());

    if (!nodeStack.empty()) {
        if (nodeStack.back()->children == nullptr) {
            nodeStack.back()->children = std::make_unique<llvm::SmallVector<std::shared_ptr<const APINode>, 16>>();
        }
        nodeStack.back()->children->push_back(node);
    }
    else context->addRootNode(node);

    if (nodeStack.empty()) context->addNode(node->NSR, node);
}

inline void beta::TreeBuilder::PushNode(const std::shared_ptr<APINode>& node) {
    nodeStack.push_back(node);
}

inline void beta::TreeBuilder::PopNode() {
    if (!nodeStack.empty()) {
        nodeStack.pop_back();
    }
}

inline void beta::TreeBuilder::PushName(llvm::StringRef name) {
    qualifiedName.push(name);
}

inline void beta::TreeBuilder::PopName() {
    qualifiedName.pop();
}

inline const std::string beta::TreeBuilder::GetCurrentQualifiedName() {
    return qualifiedName.getAsString();
}

bool beta::TreeBuilder::isQualifiedNameOverriden(llvm::StringRef usr){
    return qualifiedName.isOverrideActive(usr);
}

void beta::TreeBuilder::popOverridenQualifiedName(llvm::StringRef usr){
    return qualifiedName.overridePop(usr);
}

void beta::TreeBuilder::BuildReturnTypeNode(clang::QualType type) {
    auto returnNode = std::make_shared<APINode>();
    returnNode->kind = NodeKind::ReturnType;
    auto [dataType,canonicalType] = getTypesWithAndWithoutTypeResolution(type, *context->getClangASTContext());
    PushName("(ReturnType)");
    returnNode->dataType = dataType;
    returnNode->caonicalType = canonicalType;
    returnNode->NSR = "(ReturnType)";
    returnNode->qualifiedName = GetCurrentQualifiedName();
    AddNode(returnNode);
    PopName();

    armor::debug() << "BuildReturnType V2: " << returnNode->dataType << "\n";
}

void beta::TreeBuilder::normalizeFunctionPointerType(std::string_view typeModifiers, const clang::FunctionProtoTypeLoc FTL, const clang::NamedDecl* Decl) {
    auto functionPointerNode = std::make_shared<APINode>();
    functionPointerNode->kind = NodeKind::FunctionPointer;
    functionPointerNode->qualifiedName = GetCurrentQualifiedName();
    functionPointerNode->dataType = typeModifiers;
    if(llvm::isa<clang::ParmVarDecl>(Decl)){
        // If ParamVarDecl is a functionPointer then the NSR is QualifiedName.
        functionPointerNode->NSR = functionPointerNode->qualifiedName;
    }
    else{
        functionPointerNode->NSR = generateNSRForDecl(Decl);
        functionPointerNode->USR = generateUSRForDecl(Decl);
    }

    AddNode(functionPointerNode);
    PushNode(functionPointerNode);

    armor::debug() << "BuildFunctionPointerType V2: " << functionPointerNode->qualifiedName << "\n";

    const size_t numParams = FTL.getNumParams();
    for (unsigned int pos=0 ; pos < numParams ; ++pos) {
        normalizeValueDeclNode(FTL.getParam(pos),pos);
    }

    BuildReturnTypeNode(FTL.getReturnLoc().getType());
    PopNode();
}

void beta::TreeBuilder::normalizeValueDeclNode(const clang::ValueDecl *Decl, unsigned int pos) {

    const std::string USR = generateUSRForDecl(Decl);
    const auto it = context->usrNodeMap.find(USR);
    bool isCached = (it != context->usrNodeMap.end());
    std::shared_ptr<APINode> ValueNode = isCached ? it->second : std::make_shared<APINode>();
    clang::QualType unDecayedDeclType = clang::QualType();
    clang::TypeSourceInfo *TSI = nullptr;
    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);

    if (const auto *paramDecl = llvm::dyn_cast_or_null<clang::ParmVarDecl>(Decl)) {
        ValueNode->kind = NodeKind::Parameter;
        unDecayedDeclType = paramDecl->getOriginalType();
        TSI = paramDecl->getCanonicalDecl()->getTypeSourceInfo();
        assert(pos != -1);
        pos++;
        PushName(std::to_string(pos));
        if(paramDecl->hasInit()){
            armor::debug()<<"Excluding ParamVar init\n";
            TEST_LOG<<"ParamVar init\n";
            BuildValueInitExpr( paramDecl->getInit(),ValueNode);
        }
    }
    else if (const auto *fieldDecl = llvm::dyn_cast_or_null<clang::FieldDecl>(Decl)) {
        ValueNode->kind = NodeKind::Field;
        unDecayedDeclType = fieldDecl->getType();
        TSI = fieldDecl->getCanonicalDecl()->getTypeSourceInfo();
        Decl->printName(OS);
        PushName(nameBuf);
        ValueNode->access = getAccessSpecifier(Decl->getAccess());
        if (fieldDecl->hasInClassInitializer()) {
            armor::debug()<<"Excluding Field init\n" << nameBuf << "\n";
            TEST_LOG<<"Field init\n" << nameBuf << "\n";
            BuildValueInitExpr(fieldDecl->getInClassInitializer(), ValueNode);
        }
    }
    else if (const auto *varDecl = llvm::dyn_cast_or_null<clang::VarDecl>(Decl)) {
        ValueNode->kind = varDecl->isCXXClassMember() ? NodeKind::Field : NodeKind::Variable;
        ValueNode->storage = getStorageClass(varDecl->getStorageClass());
        ValueNode->isInlined = varDecl->isInlineSpecified();
        ValueNode->isConstExpr = varDecl->isConstexpr();
        unDecayedDeclType = varDecl->getType();
        TSI = varDecl->getCanonicalDecl()->getTypeSourceInfo();
        Decl->printName(OS);
        PushName(nameBuf);
        if(varDecl->hasInit()){
            armor::debug()<<"Excluding Var init\n" << nameBuf << "\n";
            TEST_LOG<<"Var init\n" << nameBuf << "\n";
            BuildValueInitExpr(varDecl->getInit(), ValueNode);
        }
    }
    else return;

    auto [dataType, canonicalType] = getTypesWithAndWithoutTypeResolution(unDecayedDeclType, Decl->getASTContext());

    if (llvm::isa<clang::ParmVarDecl>(Decl)) {
        // NSR for param Decl is the position as they should be identified by position.
        ValueNode->NSR = std::to_string(pos);
        if (it == context->usrNodeMap.end()) ValueNode->qualifiedName = GetCurrentQualifiedName();
        armor::debug() << "VisitParamDecl V2: " << ValueNode->qualifiedName << "\n";
    }
    else if (llvm::isa<clang::FieldDecl>(Decl)) {
        if (it == context->usrNodeMap.end()) ValueNode->qualifiedName = GetCurrentQualifiedName();
        ValueNode->NSR = generateNSRForDecl(Decl);
        ValueNode->USR = USR;
        context->usrNodeMap.insert_or_assign(std::move(USR),ValueNode);
        armor::debug() << "VisitFeildDecl V2: " << ValueNode->qualifiedName << "\n";
    }
    else if (llvm::dyn_cast_or_null<clang::VarDecl>(Decl)) {
        if (it == context->usrNodeMap.end()) ValueNode->qualifiedName = GetCurrentQualifiedName();
        ValueNode->NSR = generateNSRForDecl(Decl);
        ValueNode->USR = USR;
        context->usrNodeMap.insert_or_assign(std::move(USR),ValueNode);
        armor::debug() << "VisitVarDecl V2: " << ValueNode->qualifiedName << "\n";
    }

    if(!isCached){

        AddNode(ValueNode);

        if (TSI) {
            auto [typeModifiers,unwrappedTL] = unwrapTypeLoc(TSI->getTypeLoc());
            if (const clang::FunctionProtoTypeLoc FTL = unwrappedTL.getAs<clang::FunctionProtoTypeLoc>()) {
                PushNode(ValueNode);
                normalizeFunctionPointerType(typeModifiers, FTL, Decl);
                PopNode();
            }
            else{
                ValueNode->dataType = dataType;
                ValueNode->caonicalType = canonicalType;
            }
        }
    }

    PopName();
}

bool beta::TreeBuilder::BuildRecordNode(clang::RecordDecl* Decl) {

    // Building RecordDecl for C specifaically
    if (!IsDeclFromMainFileAndNotLocal(Decl)) return false;

    if(llvm::isa<clang::CXXRecordDecl>(Decl)) return true;

    const std::string USR = generateUSRForDecl(Decl);
    const std::string NSR = generateNSRForDecl(Decl);
    const auto it = context->usrNodeMap.find(USR);
    const bool isCached = (it != context->usrNodeMap.end());

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);
    Decl->printName(OS);

    if (!isCached) {
        if (!nameBuf.empty()) {
            PushName(nameBuf);
        }
        else {
            if (const auto *typedefForAnon = Decl->getTypedefNameForAnonDecl()) {
                typedefForAnon->printName(OS);
                PushName(nameBuf);
            }
            else {
                if (Decl->isEmbeddedInDeclarator() && !Decl->isFreeStanding()) {
                    if (const clang::ValueDecl* ValueDecl =
                            llvm::dyn_cast_or_null<clang::ValueDecl>(Decl->getNextDeclInContext())) {
                        const clang::QualType QT = armor::unwrapTypeModifiers(ValueDecl->getType());
                        if (QT->getAsTagDecl()->Equals(Decl)) {
                            ValueDecl->printName(OS);
                            PushName(nameBuf);
                        }
                    }
                    else if (const clang::TypedefDecl* TypeDefDecl =
                                 llvm::dyn_cast_or_null<clang::TypedefDecl>(Decl->getNextDeclInContext())) {
                        const clang::QualType QT = armor::unwrapTypeModifiers(TypeDefDecl->getUnderlyingType());
                        if (QT->getAsTagDecl()->Equals(Decl)) {
                            TypeDefDecl->printName(OS);
                            PushName(nameBuf);
                        }
                    }
                }
                else {
                    if (Decl->isStruct()) PushName("(Anon::Struct)");
                    if (Decl->isUnion())  PushName("(Anon::Union)");
                }
            }
        }
    }

    std::shared_ptr<APINode> recordNode = isCached ? it->second : std::make_shared<APINode>();
    recordNode->NSR = NSR;
    recordNode->USR = USR;
    if (!isCached) {
        recordNode->qualifiedName = GetCurrentQualifiedName();
        AddNode(recordNode);
    } else {
        qualifiedName.overridePush(recordNode->qualifiedName, USR);
    }
    context->usrNodeMap.insert_or_assign(std::move(USR), recordNode);

    armor::debug() << "VisitRecordDecl (C): " << recordNode->qualifiedName << "\n";

    if (Decl->isStruct()) {
        recordNode->kind = NodeKind::Struct;
    }
    else if (Decl->isUnion()) {
        recordNode->kind = NodeKind::Union;
    }

    PushNode(recordNode);
    return true;
}

bool beta::TreeBuilder::BuildCXXRecordNode(clang::CXXRecordDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl)) return false;

    if( isInTemplatedClass(Decl) || Decl->isTemplated() || llvm::isa<clang::ClassTemplateSpecializationDecl>(Decl) ){
        if(!isWrittenInTemplatedClass(Decl)){
            armor::debug()<<"Excluding CXXRecordNode\n";
            TEST_LOG<<"CXXRecordNode\n";
            processUnhandledDecl(Decl);
        }
        return true;
    }

    const std::string USR = generateUSRForDecl(Decl);
    const std::string NSR = generateNSRForDecl(Decl);
    const auto it = context->usrNodeMap.find(USR);
    const bool isCached = (it != context->usrNodeMap.end());

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);
    Decl->printName(OS);

    if (!isCached) {
        if(!nameBuf.empty()){
            PushName(nameBuf);
        }
        else{
            if (const auto *typedefForAnon = Decl->getTypedefNameForAnonDecl()) {
                typedefForAnon->printName(OS);
                PushName(nameBuf);
            }
            else{
                if(Decl->isEmbeddedInDeclarator() && !Decl->isFreeStanding()){
                    if(const clang::ValueDecl * ValueDecl = llvm::dyn_cast_or_null<clang::ValueDecl>(Decl->getNextDeclInContext())){
                        const clang::QualType QT = armor::unwrapTypeModifiers(ValueDecl->getType());
                        if(QT->getAsTagDecl()->Equals(Decl)){
                            ValueDecl->printName(OS);
                            PushName(nameBuf);
                        }
                    }
                    else if(const clang::TypedefDecl * TypeDefDecl = llvm::dyn_cast_or_null<clang::TypedefDecl>(Decl->getNextDeclInContext())){
                        const clang::QualType QT = armor::unwrapTypeModifiers(TypeDefDecl->getUnderlyingType());
                        if(QT->getAsTagDecl()->Equals(Decl)){
                            TypeDefDecl->printName(OS);
                            PushName(nameBuf);
                        }
                    }
                }
                else{
                    if(Decl->isStruct()) PushName("(Anon::Struct)");
                    if(Decl->isUnion()) PushName("(Anon::Union)");
                }
            }
        }
    }

    std::shared_ptr<APINode> cxxRecordNode = isCached ? it->second : std::make_shared<APINode>();
    cxxRecordNode->NSR = NSR;
    cxxRecordNode->USR = USR;

    if (!isCached) {
        cxxRecordNode->qualifiedName = GetCurrentQualifiedName();
        AddNode(cxxRecordNode);
    }
    else {
        qualifiedName.overridePush(cxxRecordNode->qualifiedName, USR);
    }
    context->usrNodeMap.insert_or_assign(std::move(USR), cxxRecordNode);

    armor::debug() << "VisitCxxRecordDecl V2: " << cxxRecordNode->qualifiedName << "\n";

    cxxRecordNode->access = getAccessSpecifier(Decl->getAccess());
    cxxRecordNode->isFinal = Decl->isEffectivelyFinal();

    if( Decl->isStruct() ){
        cxxRecordNode->kind = NodeKind::Struct;
    }
    else if (Decl->isUnion()) {
        cxxRecordNode->kind = NodeKind::Union;
    }
    else{
        cxxRecordNode->kind = NodeKind::Class;
    }

    PushNode(cxxRecordNode);

    if ( (Decl->isClass() || Decl->isStruct()) && Decl->isThisDeclarationADefinition()){
        for( const clang::CXXBaseSpecifier& baseSpecifier : Decl->bases() ){
            BuildBaseClassDecl(baseSpecifier);

        }
    }

    if( Decl->isThisDeclarationADefinition() && Decl->hasFriends() ){
        for( clang::FriendDecl* friendDecl : Decl->friends() ){
            BuildFriendDecl(friendDecl);
        }
    }

    return true;
}


bool beta::TreeBuilder::BuildEnumNode(clang::EnumDecl* Decl){

    if (!IsDeclFromMainFileAndNotLocal(Decl)) return false;

    if (isInTemplatedClass(Decl) || Decl->isTemplated()){
        if( !isWrittenInTemplatedClass(Decl)){
            armor::debug() << "Excluding EnumNode\n";
            TEST_LOG << "EnumNode\n";
            processUnhandledDecl(Decl);
        }
        return true;
    }

    const std::string USR = generateUSRForDecl(Decl);
    const std::string NSR = generateNSRForDecl(Decl);
    const auto it = context->usrNodeMap.find(USR);
    const bool isCached = (it != context->usrNodeMap.end());

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);
    Decl->printName(OS);

    if (!isCached) {
        if(!nameBuf.empty()){
            PushName(nameBuf);
        }
        else{
            if (const auto *typedefForAnon = Decl->getTypedefNameForAnonDecl()) {
                typedefForAnon->printName(OS);
                PushName(nameBuf);
            }
            else{
                if(Decl->isEmbeddedInDeclarator() && !Decl->isFreeStanding()){
                    if(const clang::ValueDecl * ValueDecl = llvm::dyn_cast_or_null<clang::ValueDecl>(Decl->getNextDeclInContext())){
                        const clang::QualType QT = armor::unwrapTypeModifiers(ValueDecl->getType());
                        if(QT->getAsTagDecl()->Equals(Decl)){
                            ValueDecl->printName(OS);
                            PushName(nameBuf);
                        }
                    }
                    else if(const clang::TypedefDecl * TypeDefDecl = llvm::dyn_cast_or_null<clang::TypedefDecl>(Decl->getNextDeclInContext())){
                        const clang::QualType QT = armor::unwrapTypeModifiers(TypeDefDecl->getUnderlyingType());
                        if(QT->getAsTagDecl()->Equals(Decl)){
                            TypeDefDecl->printName(OS);
                            PushName(nameBuf);
                        }
                    }
                }
                else{
                    PushName("(Anon::Enum)");
                }
            }
        }
    }

    std::shared_ptr<APINode> enumNode = isCached ? it->second : std::make_shared<APINode>();
    enumNode->NSR = NSR;
    enumNode->USR = USR;

    if (!isCached) {
        enumNode->qualifiedName = GetCurrentQualifiedName();
        AddNode(enumNode);
    }
    else {
        qualifiedName.overridePush(enumNode->qualifiedName, USR);
    }
    context->usrNodeMap.insert_or_assign(std::move(USR), enumNode);

    armor::debug() << "VisitEnumDecl V2: " << enumNode->qualifiedName << "\n";

    enumNode->kind = NodeKind::Enum;
    PushNode(enumNode);

    const clang::QualType enumType = Decl->getIntegerType();
    std::string enumaratorDataType = enumType.getAsString();

    for (const auto* EnumConstDecl : Decl->enumerators()) {
        if(!Decl->isThisDeclarationADefinition()) continue;
        auto enumValNode = std::make_shared<APINode>();
        llvm::StringRef enumConstName = EnumConstDecl->getName();
        PushName(enumConstName);
        enumValNode->qualifiedName = GetCurrentQualifiedName();
        enumValNode->dataType = enumaratorDataType;
        enumValNode->NSR = generateNSRForDecl(EnumConstDecl);
        enumValNode->USR = generateUSRForDecl(EnumConstDecl);
        const clang::Expr* expr = EnumConstDecl->getInitExpr();
        if(expr){
            armor::debug() << "Excluding EnumConst\n" << nameBuf << ":" << enumConstName << "\n";
            TEST_LOG << "EnumConst\n" << nameBuf << ":" << enumConstName << "\n";
            processUnhandledStmt(EnumConstDecl->getInitExpr(), enumValNode);
        }
        armor::debug() << "VisitEnumConstDecl V2: "<< enumValNode->qualifiedName << "\n";
        PopName();
        enumValNode->kind = NodeKind::Enumerator;
        AddNode(enumValNode);
    }

    PopNode();

    if (!isCached) {
        PopName();
    }
    else {
        qualifiedName.overridePop(USR);
    }

    return true;
}


bool beta::TreeBuilder::BuildFunctionNode(clang::FunctionDecl* Decl){

    if (!IsDeclFromMainFileAndNotLocal(Decl)) return false;

    if (isInTemplatedClass(Decl) || Decl->isTemplated() || Decl->isFunctionTemplateSpecialization()){
        if(!isWrittenInTemplatedClass(Decl)){
            armor::debug()<<"Excluding FunctionNode\n";
            TEST_LOG<<"FunctionNode\n";
            processUnhandledDecl(Decl);
        }
        return true;
    }

    const std::string USR = generateUSRForDecl(Decl);
    const auto it = context->usrNodeMap.find(USR);
    const bool isCached = (it != context->usrNodeMap.end());

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);

    auto functionNode = isCached ? it->second : std::make_shared<APINode>();
    Decl->printName(OS);

    if(Decl->isThisDeclarationADefinition() && Decl->getBody() && !Decl->isExplicitlyDefaulted()){
        armor::debug()<<"Excluding Function Body : "<< nameBuf << "\n";
        TEST_LOG<<"Function Body\n" << nameBuf << "\n";
        processUnhandledStmt(Decl->getBody(), functionNode);
    }

    functionNode->kind = NodeKind::Function;
    functionNode->isInlined = Decl->isInlined();
    functionNode->storage = getStorageClass(Decl->getStorageClass());
    functionNode->NSR = generateNSRForDecl(Decl);
    functionNode->USR = USR;
    functionNode->access = getAccessSpecifier(Decl->getAccess());
    functionNode->virtualQualifier = Decl->isPure() ? VirtualQualifier::PureVirtual : VirtualQualifier::None;
    functionNode->virtualQualifier = Decl->isVirtualAsWritten() ? VirtualQualifier::Virtual : VirtualQualifier::None;

    if (const auto *methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(Decl)) {
        functionNode->isOveride = methodDecl->size_overridden_methods() > 0;
        functionNode->isConst = methodDecl->isConst();
        functionNode->isVolatile = methodDecl->isVolatile();
        if (const auto *constructorDecl = llvm::dyn_cast<clang::CXXConstructorDecl>(methodDecl)) {
            functionNode->isExplicit = constructorDecl->isExplicit();
        }
        else if (const auto *conversionDecl = llvm::dyn_cast<clang::CXXConversionDecl>(methodDecl)) {
            functionNode->isExplicit = conversionDecl->isExplicit();
        }
        // Final is a keyword, why use an attr isFinal() Not available
        // Will need to upgared to clang 17+
        functionNode->isFinal = methodDecl->hasAttr<clang::FinalAttr>();
    }
    functionNode->isDelete = Decl->isDeleted();
    functionNode->isDefault = Decl->isDefaulted();

    if(!isCached){
        PushName(nameBuf);
        functionNode->qualifiedName = GetCurrentQualifiedName();
        context->usrNodeMap.insert_or_assign(std::move(USR),functionNode);
    }
    else{
        qualifiedName.overridePush(functionNode->qualifiedName, USR);
    }

    armor::debug() << "VisitFunctionDecl V2: " << functionNode->qualifiedName << "\n";

    if(!isCached){
        AddNode(functionNode);
        PushNode(functionNode);

        const size_t numParams = Decl->param_size();
        for (unsigned int pos = 0; pos < numParams ; ++pos) {
            normalizeValueDeclNode(Decl->getParamDecl(pos), pos);
        }

        BuildReturnTypeNode(Decl->getReturnType());
        PopName();
        PopNode();
    }
    else{
        qualifiedName.overridePop(USR);
    }

    return true;
}


bool beta::TreeBuilder::BuildTypedefDecl(clang::TypedefDecl *Decl) {

    if(!IsDeclFromMainFileAndNotLocal(Decl) || isInTemplatedClass(Decl) || Decl->isTemplated()){
        if(IsDeclFromMainFileAndNotLocal(Decl) && !isWrittenInTemplatedClass(Decl)){
            armor::debug()<<"Excluding TypedefDecl\n";
            TEST_LOG<<"TypedefDecl\n";
            processUnhandledDecl(Decl);
        }
        return false;
    }

    const std::string USR = generateUSRForDecl(Decl);
    if( context->usrNodeMap.find(USR) != context->usrNodeMap.end() ) return true;

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);

    const clang::QualType underlyingType = Decl->getUnderlyingType();

    auto typeDefNode = std::make_shared<APINode>();
    Decl->printName(OS);
    PushName(nameBuf);
    typeDefNode->qualifiedName = GetCurrentQualifiedName();
    typeDefNode->kind = NodeKind::Typedef;
    auto [dataType, canonicalType] = getTypesWithAndWithoutTypeResolution(underlyingType, Decl->getASTContext());
    typeDefNode->dataType = dataType;
    typeDefNode->caonicalType = canonicalType;
    typeDefNode->USR = USR;
    typeDefNode->NSR = generateNSRForDecl(Decl);
    context->usrNodeMap.insert_or_assign(std::move(USR), typeDefNode);

    armor::debug() << "VisitTypeDefDecl V2: " << typeDefNode->qualifiedName << "\n";

    if (!llvm::isa<clang::TypedefType>(underlyingType)) {
        if (const clang::TypeSourceInfo *TSI = Decl->getTypeSourceInfo()) {
            auto [typeModifiers,unwrappedTL] = unwrapTypeLoc(TSI->getTypeLoc());
            if (const clang::FunctionProtoTypeLoc FTL = unwrappedTL.getAs<clang::FunctionProtoTypeLoc>()) {
                typeDefNode->dataType = std::string{};
                PushNode(typeDefNode);
                normalizeFunctionPointerType(typeModifiers, FTL, Decl);
                PopNode();
            }
        }
    }

    PopName();
    AddNode(typeDefNode);

    return true;
}

bool beta::TreeBuilder::BuildVarDecl(clang::VarDecl *Decl) {

    if (!IsDeclFromMainFileAndNotLocal(Decl) || isInTemplatedClass(Decl) || !Decl->hasGlobalStorage()
    || Decl->isTemplated()){
        if(Decl->hasGlobalStorage() && IsDeclFromMainFileAndNotLocal(Decl) && !isWrittenInTemplatedClass(Decl) && (isInTemplatedClass(Decl) || Decl->isTemplated())){
            armor::debug()<<"Excluding TemplatedVarDecl\n";
            TEST_LOG<<"TemplatedVarDecl\n";
            processUnhandledDecl(Decl);
        }
        return false;
    }

    if(llvm::isa<clang::VarTemplateDecl>(Decl) || llvm::isa<clang::VarTemplatePartialSpecializationDecl>(Decl)
    || llvm::isa<clang::VarTemplateSpecializationDecl>(Decl)){
        if(!isWrittenInTemplatedClass(Decl)){
            armor::debug()<<"Excluding TempletSpecVarDecl\n";
            TEST_LOG<<"TempletSpecVarDecl\n";
            processUnhandledDecl(Decl);
        }
        return true;
    }

    normalizeValueDeclNode(Decl);
    return true;
}

bool beta::TreeBuilder::BuildFieldDecl(clang::FieldDecl *Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isInTemplatedClass(Decl)
    || Decl->isTemplated()){
        return false;
    }

    normalizeValueDeclNode(Decl);
    return true;
}

void inline beta::TreeBuilder::BuildBaseClassDecl(clang::CXXBaseSpecifier Decl){
    const clang::QualType Type = Decl.getType();
    if(Type->getAsCXXRecordDecl() == nullptr) return;

    const clang::CXXRecordDecl * cxxBaseRecordDecl = Type->getAsCXXRecordDecl();
    const auto cxxBaseRecord = std::make_shared<APINode>();
    cxxBaseRecord->qualifiedName = generateQualifiedNameForDecl(cxxBaseRecordDecl);
    cxxBaseRecord->kind = NodeKind::BaseClass;
    cxxBaseRecord->NSR = generateNSRForDecl(cxxBaseRecordDecl);
    cxxBaseRecord->access = getAccessSpecifier(Decl.getAccessSpecifier());

    armor::info()<<"VisitBaseClassDecl V2 : "<< cxxBaseRecord->qualifiedName << "\n";

    auto [dataType, canonicalType] = getTypesWithAndWithoutTypeResolution(Type, *context->getClangASTContext());
    cxxBaseRecord->dataType = dataType;
    cxxBaseRecord->caonicalType = canonicalType;
    cxxBaseRecord->virtualQualifier = Decl.isVirtual() ? VirtualQualifier::Virtual : VirtualQualifier::None;
    AddNode(cxxBaseRecord);
}

void inline beta::TreeBuilder::BuildFriendDecl(clang::FriendDecl *Decl){

    if(!IsDeclFromMainFileAndNotLocal(Decl) || Decl->isTemplateDecl()){
        if(IsDeclFromMainFileAndNotLocal(Decl) && !isWrittenInTemplatedClass(Decl)){
            armor::debug()<<"Excluding FriendDecl\n";
            TEST_LOG<<"FriendDecl\n";
            processUnhandledDecl(Decl);
        }
        return;
    }

    if (const clang::NamedDecl* namedDecl = Decl->getFriendDecl()) {
        if (const auto *functionDecl = llvm::dyn_cast_or_null<clang::FunctionDecl>(namedDecl)) {
            BuildFriendFunctionDecl(functionDecl, Decl);
        }
        else return;
    }
    else if (Decl->getFriendType()) {
        const clang::QualType QT = Decl->getFriendType()->getType();
        if (const clang::CXXRecordDecl* cxxRecordDecl = QT.getTypePtr()->getAsCXXRecordDecl()) {
            BuildFriendCxxRecordDecl(cxxRecordDecl, Decl);
        }
        else return;
    }
    else return;

   armor::debug()<<"VisitFriendDecl V2 \n";

}

void beta::TreeBuilder::BuildFriendCxxRecordDecl(const clang::CXXRecordDecl *Decl, const clang::FriendDecl *FriendDecl){
    // Friend Decl can be in the same file or a different file as well
    const std::string USR = generateUSRForDecl(FriendDecl);
    const std::string NSR = generateNSRForDecl(FriendDecl);

    llvm::SmallString<256> keyBuf;
    llvm::raw_svector_ostream OS(keyBuf);

    OS << nodeStack.back()->USR << ':' << USR;

    const auto it = context->usrNodeMap.find(keyBuf);
    std::shared_ptr<APINode> friendCxxRecordNode = (it != context->usrNodeMap.end()) ? it->second : std::make_shared<APINode>();
    friendCxxRecordNode->NSR = NSR;
    friendCxxRecordNode->USR = USR;

    const clang::QualType Type = FriendDecl->getFriendType()->getType();
    auto [dataType, canonicalType] = getTypesWithAndWithoutTypeResolution(Type, Decl->getASTContext());
    friendCxxRecordNode->dataType = dataType;
    friendCxxRecordNode->caonicalType = canonicalType;
    if( Decl->isStruct() ){
        friendCxxRecordNode->kind = NodeKind::FriendStruct;
    }
    else if (Decl->isUnion()) {
        friendCxxRecordNode->kind = NodeKind::FriendUnion;
    }
    else{
        friendCxxRecordNode->kind = NodeKind::FriendClass;
    }

    if(it == context->usrNodeMap.end()) AddNode(friendCxxRecordNode);

    friendCxxRecordNode->qualifiedName = generateQualifiedNameForDecl(Decl);
    context->usrNodeMap.insert_or_assign(std::move(keyBuf),friendCxxRecordNode);

    armor::debug()<<"VisitFriendCxxRecordDecl V2 : " << friendCxxRecordNode->qualifiedName << "\n";
}

void beta::TreeBuilder::BuildFriendFunctionDecl(const clang::FunctionDecl *Decl, const clang::FriendDecl *FriendDecl){

    const std::string USR = generateUSRForDecl(FriendDecl);

    llvm::SmallString<256> keyBuf;
    llvm::raw_svector_ostream OS(keyBuf);

    OS << nodeStack.back()->USR << ':' << USR;

    if( context->usrNodeMap.find(keyBuf) != context->usrNodeMap.end() ) return;

    auto friendFunctionNode = std::make_shared<APINode>();
    friendFunctionNode->qualifiedName = generateQualifiedNameForDecl(Decl);

    qualifiedName.overridePush(friendFunctionNode->qualifiedName, USR);

    friendFunctionNode->kind = NodeKind::FriendFunction;
    friendFunctionNode->storage = getStorageClass(Decl->getStorageClass());
    friendFunctionNode->NSR = generateNSRForDecl(FriendDecl);
    friendFunctionNode->USR = USR;
    friendFunctionNode->access = getAccessSpecifier(Decl->getAccess());
    friendFunctionNode->virtualQualifier = Decl->isPure() ? VirtualQualifier::PureVirtual : VirtualQualifier::None;
    friendFunctionNode->virtualQualifier = Decl->isVirtualAsWritten() ? VirtualQualifier::Virtual : VirtualQualifier::None;

    if (const auto *methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(Decl)) {
        friendFunctionNode->isConst = methodDecl->isConst();
        friendFunctionNode->isVolatile = methodDecl->isVolatile();
        if (const auto *constructorDecl = llvm::dyn_cast<clang::CXXConstructorDecl>(methodDecl)) {
            friendFunctionNode->isExplicit = constructorDecl->isExplicit();
        }
        else if (const auto *conversionDecl = llvm::dyn_cast<clang::CXXConversionDecl>(methodDecl)) {
            friendFunctionNode->isExplicit = conversionDecl->isExplicit();
        }
        // Final is a keyword, why use an attr isFinal() Not available
        // Will need to upgared to clang 17+
        friendFunctionNode->isFinal = methodDecl->hasAttr<clang::FinalAttr>();
    }
    friendFunctionNode->isDelete = Decl->isDeleted();
    friendFunctionNode->isDefault = Decl->isDefaulted();
    context->usrNodeMap.insert_or_assign(std::move(keyBuf),friendFunctionNode);

    armor::debug() << "VisitFriendFunctionDecl V2 : " << friendFunctionNode->qualifiedName << "\n";

    AddNode(friendFunctionNode);
    PushNode(friendFunctionNode);

    const size_t numParams = Decl->param_size();
    for (unsigned int pos = 0; pos < numParams ; ++pos) {
        normalizeValueDeclNode(Decl->getParamDecl(pos), pos);
    }

    BuildReturnTypeNode(Decl->getReturnType());

    PopNode();
    qualifiedName.overridePop(USR);

}

// === Unsupported Declaration Handlers ===

// void beta::TreeBuilder::BuildNamespaceDecl(clang::NamespaceDecl* Decl) {
//     if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

//     armor::debug()<<"Excluding NamespaceDecl\n";
//     TEST_LOG<<"NamespaceDecl\n";

//     processUnhandledDecl(Decl);
// }

void beta::TreeBuilder::BuildFunctionTemplateDecl(clang::FunctionTemplateDecl* Decl){
    if(!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding FunctionTemplateDecl\n";
    TEST_LOG << "FunctionTemplateDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildClassTemplateDecl(clang::ClassTemplateDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding ClassTemplateDecl\n";
    TEST_LOG << "ClassTemplateDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding ClassTemplateSpecializationDecl\n";
    TEST_LOG << "ClassTemplateSpecializationDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding ClassTemplatePartialSpecializationDecl\n";
    TEST_LOG << "ClassTemplatePartialSpecializationDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildTypeAliasDecl(clang::TypeAliasDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding TypeAliasDecl\n";
    TEST_LOG << "TypeAliasDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildUsingDecl(clang::UsingDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding UsingDecl\n";
    TEST_LOG << "UsingDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildUsingDirectiveDecl(clang::UsingDirectiveDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding UsingDirectiveDecl\n";
    TEST_LOG << "UsingDirectiveDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildNamespaceAliasDecl(clang::NamespaceAliasDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding NamespaceAliasDecl\n";
    TEST_LOG << "NamespaceAliasDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildStaticAssertDecl(clang::StaticAssertDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding StaticAssertDecl\n";
    TEST_LOG << "StaticAssertDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildVarTemplateDecl(clang::VarTemplateDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding VarTemplateDecl\n";
    TEST_LOG << "VarTemplateDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding VarTemplateSpecializationDecl\n";
    TEST_LOG << "VarTemplateSpecializationDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildVarTemplatePartialSpecializationDecl(clang::VarTemplatePartialSpecializationDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding VarTemplatePartialSpecializationDecl\n";
    TEST_LOG << "VarTemplatePartialSpecializationDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl* Decl) {
    if (!IsDeclFromMainFileAndNotLocal(Decl) || isWrittenInTemplatedClass(Decl)) return;

    armor::debug() << "Excluding TypeAliasTemplateDecl\n";
    TEST_LOG << "TypeAliasTemplateDecl\n";

    processUnhandledDecl(Decl);
}

void beta::TreeBuilder::BuildValueInitExpr(const clang::Expr* Expr, const std::shared_ptr<beta::APINode>& node){
    if (Expr && !IsStmtFromMainFile(Expr)) return;

    if (const clang::CXXConstructExpr* cxxConstructExpr = llvm::dyn_cast<clang::CXXConstructExpr>(Expr)) {
        if(cxxConstructExpr->getNumArgs()){
            TEST_LOG <<"(ARG-EXPR) : "<<cxxConstructExpr->getNumArgs();
            TEST_LOG<<"\n----------------------------------------\n";
            for(const clang::Expr *argExpr : cxxConstructExpr->arguments() ){
                processUnhandledStmt(argExpr, node);
            }
        }
        else{
            TEST_LOG<<"(NO ARG-EXPR)\n----------------------------------------\n";
        }
    }
    else{
        processUnhandledStmt(Expr, node);
    }
}