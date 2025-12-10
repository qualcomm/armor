// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include "tree_builder.hpp"
#include "ast_normalized_context.hpp"
#include "astnormalizer.hpp"
#include "custom_usr_generator.hpp"
#include "diff_utils.hpp"
#include "tree_builder_utils.hpp"
#include "iostream"
#include "node.hpp"
#include "debug_config.hpp"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Type.h"
#include "type_utils.hpp"
#include <cassert>
#include <cstddef>
#include <llvm-14/llvm/ADT/SmallString.h>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <llvm-14/llvm/ADT/StringSet.h>
#include <llvm-14/llvm/Support/Casting.h>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

beta::TreeBuilder::TreeBuilder(beta::ASTNormalizedContext* context): context(context) {}

inline bool beta::TreeBuilder::IsFromMainFileAndNotLocal(const clang::Decl* Decl) {
    clang::ASTContext* clangContext = &Decl->getASTContext();
    return clangContext->getSourceManager().isInMainFile(Decl->getLocation()) && Decl->getParentFunctionOrMethod() == nullptr;
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

    DebugConfig::instance().log("BuildReturnType V2: " + returnNode->dataType, DebugConfig::Level::DEBUG);
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
    
    DebugConfig::instance().log("BuildFunctionPointerType V2: " + functionPointerNode->qualifiedName, DebugConfig::Level::DEBUG);
    
    const size_t numParams = FTL.getNumParams();
    for (unsigned int pos=0 ; pos < numParams ; ++pos) {
        normalizeValueDeclNode(FTL.getParam(pos),pos);
    }
    
    BuildReturnTypeNode(FTL.getReturnLoc().getType());
    PopNode();
}

void beta::TreeBuilder::normalizeValueDeclNode(const clang::ValueDecl *Decl, unsigned int pos) {
    
    const std::string USR = generateUSRForDecl(Decl);
    if( context->usrNodeMap.find(USR) != context->usrNodeMap.end() ) return;

    auto ValueNode = std::make_shared<APINode>();
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
    } 
    else if (const auto *fieldDecl = llvm::dyn_cast_or_null<clang::FieldDecl>(Decl)) {
        ValueNode->kind = NodeKind::Field;
        unDecayedDeclType = fieldDecl->getType();
        TSI = fieldDecl->getCanonicalDecl()->getTypeSourceInfo();
        Decl->printName(OS);
        PushName(nameBuf);
    } 
    else if (const auto *varDecl = llvm::dyn_cast_or_null<clang::VarDecl>(Decl)) {
        ValueNode->kind = NodeKind::Variable;
        ValueNode->storage = getStorageClass(varDecl->getStorageClass());
        unDecayedDeclType = varDecl->getType();
        TSI = varDecl->getCanonicalDecl()->getTypeSourceInfo();
        Decl->printName(OS);
        PushName(nameBuf);
    } 
    else return;

    auto [dataType, canonicalType] = getTypesWithAndWithoutTypeResolution(unDecayedDeclType, Decl->getASTContext());

    if (llvm::isa<clang::ParmVarDecl>(Decl)) {
        // NSR for param Decl is the position as they should be identified by position.
        ValueNode->NSR = std::to_string(pos);
        ValueNode->qualifiedName = GetCurrentQualifiedName();
        DebugConfig::instance().log("VisitParamDecl V2: " + ValueNode->qualifiedName, DebugConfig::Level::DEBUG);
    } 
    else if (llvm::isa<clang::FieldDecl>(Decl)) {
        ValueNode->qualifiedName = GetCurrentQualifiedName();
        ValueNode->NSR = generateNSRForDecl(Decl);
        ValueNode->USR = USR;
        context->usrNodeMap.insert_or_assign(std::move(USR),ValueNode);
        DebugConfig::instance().log("VisitFeildDecl V2: " + ValueNode->qualifiedName, DebugConfig::Level::DEBUG);
    } 
    else if (llvm::dyn_cast_or_null<clang::VarDecl>(Decl)) {
        ValueNode->qualifiedName = GetCurrentQualifiedName();
        ValueNode->NSR = generateNSRForDecl(Decl);
        ValueNode->USR = USR;
        context->usrNodeMap.insert_or_assign(std::move(USR),ValueNode);
        DebugConfig::instance().log("VisitVarDecl V2: " + ValueNode->qualifiedName, DebugConfig::Level::DEBUG);
    } 

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
    
    PopName();
}

bool beta::TreeBuilder::BuildCXXRecordNode(clang::CXXRecordDecl* Decl) {
    if (!IsFromMainFileAndNotLocal(Decl) || Decl->isClass() || Decl->isTemplated() 
    || llvm::isa<clang::ClassTemplateSpecializationDecl>(Decl)) return false;

    if (const clang::CXXRecordDecl* recordDecl = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(
        Decl->getParent()); recordDecl && recordDecl->isClass()) {
        return false;
    }
    
    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);
    Decl->printName(OS);

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

    const std::string USR = generateUSRForDecl(Decl);
    const std::string NSR = generateNSRForDecl(Decl);

    const auto it = context->usrNodeMap.find(USR);
    std::shared_ptr<APINode> cxxRecordNode = (it != context->usrNodeMap.end()) ? it->second : std::make_shared<APINode>();
    cxxRecordNode->NSR = NSR;
    cxxRecordNode->USR = USR;
    if(it == context->usrNodeMap.end()) AddNode(cxxRecordNode);
    cxxRecordNode->qualifiedName = GetCurrentQualifiedName();
    context->usrNodeMap.insert_or_assign(std::move(USR),cxxRecordNode);

    DebugConfig::instance().log("VisitCxxRecordDecl V2: " + cxxRecordNode->qualifiedName, DebugConfig::Level::DEBUG);

    if( Decl->isStruct() ){
        cxxRecordNode->kind = NodeKind::Struct;
    }
    else if (Decl->isUnion()) {
        cxxRecordNode->kind = NodeKind::Union;
    }
    
    PushNode(cxxRecordNode);

    return true;
}


bool beta::TreeBuilder::BuildEnumNode(clang::EnumDecl* Decl){

    if (!IsFromMainFileAndNotLocal(Decl) || Decl->isClass() || Decl->isTemplated()) return false;

    if (const clang::CXXRecordDecl* recordDecl = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(
        Decl->getParent()); recordDecl && recordDecl->isClass()) {
        return false;
    }

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);
    Decl->printName(OS);

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

    const std::string USR = generateUSRForDecl(Decl);
    const std::string NSR = generateNSRForDecl(Decl);

    const auto it = context->usrNodeMap.find(USR);
    std::shared_ptr<APINode> enumNode = (it != context->usrNodeMap.end()) ? it->second : std::make_shared<APINode>();
    enumNode->NSR = NSR;
    enumNode->USR = USR;
    if(it == context->usrNodeMap.end()) AddNode(enumNode);
    enumNode->qualifiedName = GetCurrentQualifiedName();
    context->usrNodeMap.insert_or_assign(std::move(USR),enumNode);
    
    DebugConfig::instance().log("VisitEnumDecl V2: " + enumNode->qualifiedName, DebugConfig::Level::DEBUG);
    
    enumNode->kind = NodeKind::Enum;
    PushNode(enumNode);

    const clang::QualType enumType = Decl->getIntegerType();
    std::string enumaratorDataType = enumType.getAsString();
     
    for (const auto* EnumConstDecl : Decl->enumerators()) {
        auto enumValNode = std::make_shared<APINode>();
        PushName(EnumConstDecl->getName());
        enumValNode->qualifiedName = GetCurrentQualifiedName();
        enumValNode->dataType = enumaratorDataType;
        enumValNode->NSR = generateNSRForDecl(EnumConstDecl);
        enumValNode->USR = generateUSRForDecl(EnumConstDecl);
        PopName();
        enumValNode->kind = NodeKind::Enumerator;
        AddNode(enumValNode);
    }

    PopNode();
    PopName();

    return true;
}


bool beta::TreeBuilder::BuildFunctionNode(clang::FunctionDecl* Decl){

    if (!IsFromMainFileAndNotLocal(Decl) || Decl->isTemplated()) return false;

    if (const clang::CXXRecordDecl* recordDecl = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(
        Decl->getParent()); recordDecl && recordDecl->isClass()) {
        return false;
    }

    const std::string USR = generateUSRForDecl(Decl);
    if( context->usrNodeMap.find(USR) != context->usrNodeMap.end() ) return true;

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);

    auto functionNode = std::make_shared<APINode>();
    Decl->printName(OS);
    PushName(nameBuf);
    functionNode->qualifiedName = GetCurrentQualifiedName();
    functionNode->kind = NodeKind::Function;
    functionNode->storage = getStorageClass(Decl->getStorageClass());
    functionNode->NSR = generateNSRForDecl(Decl);
    functionNode->USR = USR;
    context->usrNodeMap.insert_or_assign(std::move(USR),functionNode);

    DebugConfig::instance().log("VisitFunctionDecl V2: " + functionNode->qualifiedName, DebugConfig::Level::DEBUG);

    AddNode(functionNode);
    PushNode(functionNode);
    
    const size_t numParams = Decl->param_size();
    for (unsigned int pos = 0; pos < numParams ; ++pos) {
        normalizeValueDeclNode(Decl->getParamDecl(pos), pos);
    }
    
    BuildReturnTypeNode(Decl->getReturnType());

    PopName();
    PopNode();
    
    return true;
}


bool beta::TreeBuilder::BuildTypedefDecl(clang::TypedefDecl *Decl) {
    if(!IsFromMainFileAndNotLocal(Decl) || Decl->isTemplated()) return false;

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
    
    DebugConfig::instance().log("VisitTypeDefDecl V2: " + typeDefNode->qualifiedName, DebugConfig::Level::DEBUG);

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
    if (!IsFromMainFileAndNotLocal(Decl) || !Decl->hasGlobalStorage() || Decl->isTemplated()) return false;

    if(llvm::isa<clang::VarTemplateDecl>(Decl) || llvm::isa<clang::VarTemplatePartialSpecializationDecl>(Decl) 
    || llvm::isa<clang::VarTemplateSpecializationDecl>(Decl)) return true;

    normalizeValueDeclNode(Decl);
    return true;
}

bool beta::TreeBuilder::BuildFieldDecl(clang::FieldDecl *Decl) {
    if (!IsFromMainFileAndNotLocal(Decl) || Decl->isTemplated()) return false;

    normalizeValueDeclNode(Decl);
    return true;
}