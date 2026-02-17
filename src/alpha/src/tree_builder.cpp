// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "tree_builder.hpp"
#include "ast_normalized_context.hpp"
#include "astnormalizer.hpp"
#include "comm_def.hpp"
#include "tree_builder_utils.hpp"
#include "iostream"
#include "node.hpp"
#include "logger.hpp"
#include "diff_utils.hpp"
#include "clang/AST/Decl.h"
#include "clang/Basic/Diagnostic.h"
#include <cassert>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include <string>

alpha::TreeBuilder::TreeBuilder(alpha::ASTNormalizedContext* context): context(context) {}

inline bool alpha::TreeBuilder::IsFromMainFileAndNotLocal(const clang::Decl* Decl) {
    clang::ASTContext* clangContext = &Decl->getASTContext();
    return clangContext->getSourceManager().isInMainFile(Decl->getLocation()) && Decl->getParentFunctionOrMethod() == nullptr;
}

inline void alpha::TreeBuilder::AddNode(const std::shared_ptr<APINode>& node) {

    assert(!node->hash.empty());

    if (!nodeStack.empty()) {
        if (nodeStack.back()->children == nullptr) {
            nodeStack.back()->children = std::make_unique<llvm::SmallVector<std::shared_ptr<const APINode>, 16>>();
        }
        nodeStack.back()->children->push_back(node);
    }
    else context->addRootNode(node);
    
    if (nodeStack.empty()) context->addNode(node->hash, node);
}

inline void alpha::TreeBuilder::PushNode(const std::shared_ptr<APINode>& node) {
    nodeStack.push_back(node);
}

inline void alpha::TreeBuilder::PopNode() {
    if (!nodeStack.empty()) {
        nodeStack.pop_back();
    }
}

inline void alpha::TreeBuilder::PushName(llvm::StringRef name) {
    qualifiedNames.push(name);
}

inline void alpha::TreeBuilder::PopName() {
    qualifiedNames.pop();
}

inline const std::string alpha::TreeBuilder::GetCurrentQualifiedName() {
    return qualifiedNames.getAsString();
}

void alpha::TreeBuilder::BuildReturnTypeNode(clang::QualType type) {
    auto returnNode = std::make_shared<APINode>();
    returnNode->kind = NodeKind::ReturnType;   
    PushName("(ReturnType)");
    returnNode->dataType = type.getAsString();
    returnNode->qualifiedName = GetCurrentQualifiedName();
    returnNode->hash = generateHash(returnNode->qualifiedName, NodeKind::ReturnType);
    AddNode(returnNode);
    PopName();

    armor::debug() << "BuildReturnType : " << returnNode->dataType << "\n";
}

void alpha::TreeBuilder::normalizeFunctionPointerType(const std::string& dataType, clang::FunctionProtoTypeLoc FTL) {
    
    auto functionPointerNode = std::make_shared<APINode>();
    functionPointerNode->kind = NodeKind::FunctionPointer;
    functionPointerNode->qualifiedName = GetCurrentQualifiedName();
    functionPointerNode->dataType = dataType;
    functionPointerNode->hash = generateHash(functionPointerNode->qualifiedName, NodeKind::FunctionPointer);
    
    AddNode(functionPointerNode);
    PushNode(functionPointerNode);
    
    const size_t numParams = FTL.getNumParams();
    for (unsigned int pos=0 ; pos < numParams ; ++pos) {
        normalizeValueDeclNode(FTL.getParam(pos),pos);
    }
    
    BuildReturnTypeNode(FTL.getReturnLoc().getType());
    PopNode();
}

void alpha::TreeBuilder::normalizeValueDeclNode(const clang::ValueDecl *Decl, unsigned int pos) {
    
    auto ValueNode = std::make_shared<APINode>();
    clang::QualType unDecayedDeclType;
    clang::TypeSourceInfo *TSI = nullptr;
    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);

    if (const auto *paramDecl = llvm::dyn_cast_or_null<clang::ParmVarDecl>(Decl)) {
        ValueNode->kind = NodeKind::Parameter;
        unDecayedDeclType = paramDecl->getOriginalType();
        TSI = paramDecl->getTypeSourceInfo();
        assert(pos != -1);
        pos++;
        PushName(std::to_string(pos));
    } 
    else if (const auto *fieldDecl = llvm::dyn_cast_or_null<clang::FieldDecl>(Decl)) {
        ValueNode->kind = NodeKind::Field;
        unDecayedDeclType = fieldDecl->getType();
        TSI = fieldDecl->getTypeSourceInfo();
        Decl->printName(OS);
        PushName(nameBuf);
    } 
    else if (const auto *varDecl = llvm::dyn_cast_or_null<clang::VarDecl>(Decl)) {
        ValueNode->kind = varDecl->isCXXClassMember() ? NodeKind::Field : NodeKind::Variable;
        ValueNode->storage = getStorageClass(varDecl->getStorageClass());
        ValueNode->isInclined = varDecl->isInlineSpecified();
        ValueNode->isConstExpr = varDecl->isConstexpr();
        unDecayedDeclType = varDecl->getType();
        TSI = varDecl->getTypeSourceInfo();
        Decl->printName(OS);
        PushName(nameBuf);
    } 
    else return;

    const std::string dataType = Decl->isInvalidDecl() ? DATA_TYPE_PLACE_HOLDER : unDecayedDeclType.getAsString();
    ValueNode->qualifiedName = GetCurrentQualifiedName();
    ValueNode->hash = generateHash(ValueNode->qualifiedName, ValueNode->kind);

    if (llvm::isa<clang::ParmVarDecl>(Decl)) {
        armor::debug() << "VisitParamDecl : " << ValueNode->qualifiedName << "\n";
    } 
    else if (llvm::isa<clang::FieldDecl>(Decl)) {
        armor::debug() << "VisitFieldDecl : " << ValueNode->qualifiedName << "\n";
    } 
    else if (llvm::isa<clang::VarDecl>(Decl)) {
        armor::debug() << "VisitVarDecl : " << ValueNode->qualifiedName << "\n";
    } 

    AddNode(ValueNode);

    if (TSI) {
        auto [typeModifiers, unwrappedTL] = unwrapTypeLoc(TSI->getTypeLoc());
        if (const clang::FunctionProtoTypeLoc FTL = unwrappedTL.getAs<clang::FunctionProtoTypeLoc>()) {
            PushNode(ValueNode);
            normalizeFunctionPointerType(typeModifiers, FTL);
            PopNode();
        }
        else ValueNode->dataType = dataType;
    }
    
    PopName();
}

bool alpha::TreeBuilder::BuildCXXRecordNode(clang::CXXRecordDecl* Decl) {

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);
    Decl->printName(OS);

    if (!IsFromMainFileAndNotLocal(Decl) || Decl->isClass() || Decl->isTemplateDecl() || !Decl->isThisDeclarationADefinition() 
    || llvm::isa<clang::ClassTemplateSpecializationDecl>(Decl) || nameBuf.empty()) return false;

    if (Decl->getTypedefNameForAnonDecl()) {
        return false;
    } 
    else{
        if(Decl->hasNameForLinkage()){
            PushName(nameBuf);
        }
    }

    const std::string qualifiedName = GetCurrentQualifiedName();
    auto cxxRecordNode = std::make_shared<APINode>();

    cxxRecordNode->qualifiedName = qualifiedName;

    armor::debug() << "VisitCxxRecordDecl : " << qualifiedName << "\n";

    if( Decl->isStruct() ){
        cxxRecordNode->kind = NodeKind::Struct;
        cxxRecordNode->hash = generateHash(qualifiedName, NodeKind::Struct);
    }
    else if (Decl->isUnion()) {
        cxxRecordNode->kind = NodeKind::Union;
        cxxRecordNode->hash = generateHash(qualifiedName, NodeKind::Union);
    }

    AddNode(cxxRecordNode);
    PushNode(cxxRecordNode);

    return true;
}

bool alpha::TreeBuilder::BuildEnumNode(clang::EnumDecl* Decl){

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);
    Decl->printName(OS);

    if (!IsFromMainFileAndNotLocal(Decl) || Decl->isClass() || Decl->isTemplated() || nameBuf.empty()) return false;

    if (const clang::CXXRecordDecl* recordDecl = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(
        Decl->getParent()); recordDecl && recordDecl->isClass()) {
        return false;
    }

    bool isAnonymousEnumFieldVar = false;

    if (Decl->getIdentifier() == nullptr) {
        if (clang::Decl *Next = Decl->getNextDeclInContext()) {
            if (const clang::FieldDecl *fieldDecl = clang::dyn_cast_or_null<clang::FieldDecl>(Next)) {
                if (auto *ET = fieldDecl->getType()->getAs<clang::EnumType>()) {
                    if (ET->getDecl() == Decl) isAnonymousEnumFieldVar = true;
                }
            }
            else if (const clang::VarDecl *varDecl = clang::dyn_cast_or_null<clang::VarDecl>(Next)) {
                if (auto *ET = varDecl->getType()->getAs<clang::EnumType>()) {
                    if (ET->getDecl() == Decl) isAnonymousEnumFieldVar = true;
                }
            }
        }
    }

    if(isAnonymousEnumFieldVar) return false;

    auto enumNode = std::make_shared<APINode>();

    if (Decl->getTypedefNameForAnonDecl()) return false;
    else{
        if( nameBuf.empty() ) return false;
        PushName(nameBuf);
        enumNode->qualifiedName = GetCurrentQualifiedName();
    }

    armor::debug() << "VisitEnumDecl: " << enumNode->qualifiedName << "\n";

    enumNode->kind = NodeKind::Enum;
    enumNode->hash = generateHash(enumNode->qualifiedName, NodeKind::Enum);
    
    if(!Decl->enumerators().empty()) nodeStack.push_back(enumNode);

    const clang::QualType enumType = Decl->getIntegerType();
    std::string enumaratorDataType = Decl->isInvalidDecl() ? DATA_TYPE_PLACE_HOLDER : enumType.getAsString();
     
    for (const auto* EnumConstDecl : Decl->enumerators()) {
        auto enumValNode = std::make_shared<APINode>();
        PushName(EnumConstDecl->getName());
        enumValNode->qualifiedName = GetCurrentQualifiedName();
        enumValNode->hash = generateHash(enumValNode->qualifiedName, NodeKind::Enumerator);
        enumValNode->dataType = enumaratorDataType;
        PopName();
        enumValNode->kind = NodeKind::Enumerator;
        AddNode(enumValNode);
    }

    if(!Decl->enumerators().empty()) PopNode();

    PopName();
    AddNode(enumNode);

    return true;
}


bool alpha::TreeBuilder::BuildFunctionNode(clang::FunctionDecl* Decl){

    if (!IsFromMainFileAndNotLocal(Decl) || Decl->isTemplated()) return false;

    if (const clang::CXXRecordDecl* recordDecl = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(
        Decl->getParent()); recordDecl && recordDecl->isClass()) {
        return false;
    }

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);

    auto functionNode = std::make_shared<APINode>();
    Decl->printName(OS);
    PushName(nameBuf);
    std::string qualifiedName = GetCurrentQualifiedName();
    std::string hash = generateHash(qualifiedName, NodeKind::Function);
    
    if(context->hashSet.contains(hash)){
        context->excludeNodes.insert(hash);
        armor::debug() << "Excluding Function Overloads : " << qualifiedName << "\n";
        PopName();
        return true;
    }
    
    functionNode->qualifiedName = qualifiedName;
    functionNode->kind = NodeKind::Function;
    functionNode->hash = hash;
    functionNode->storage = getStorageClass(Decl->getStorageClass());
    functionNode->isInclined = Decl->isInlined();

    armor::debug() << "VisitFunctionDecl : " << functionNode->qualifiedName << "\n";
    context->hashSet.try_emplace(hash);

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


bool alpha::TreeBuilder::BuildTypedefDecl(clang::TypedefDecl *Decl) {
    if(!IsFromMainFileAndNotLocal(Decl) || Decl->isTemplated()) return false;

    const clang::QualType underlyingType = Decl->getUnderlyingType();
    const clang::Type *typePtr = underlyingType.getTypePtr();

    llvm::SmallString<128> nameBuf;
    llvm::raw_svector_ostream OS(nameBuf);

    if (const clang::TagDecl *tagDecl = unwrapType(underlyingType)->getAsTagDecl()) {
        if (tagDecl->isEmbeddedInDeclarator()) {
            if (const clang::Decl * nextDecl = tagDecl->getNextDeclInContext()) {
                if(!nextDecl->isInvalidDecl()){
                    const clang::TagDecl* nextTagDecl = nullptr;
                    
                    switch (nextDecl->getKind()) {
                        case clang::Decl::Var:
                            if (const clang::VarDecl* varDecl = llvm::dyn_cast_or_null<clang::VarDecl>(nextDecl))
                                nextTagDecl = unwrapType(varDecl->getType())->getAsTagDecl();
                            break;
                            
                        case clang::Decl::Field:
                            if (const clang::FieldDecl* fieldDecl = llvm::dyn_cast_or_null<clang::FieldDecl>(nextDecl))
                                nextTagDecl = unwrapType(fieldDecl->getType())->getAsTagDecl();
                            break;
                        default: break;
                    }
                    
                    if (nextTagDecl == tagDecl) return false;
                }
            }

            tagDecl->printName(OS);
            PushName(nameBuf);
            
            NodeKind kind;

            if(tagDecl->isStruct()){
                kind = NodeKind::Struct;
            }
            else if(tagDecl->isUnion()){
                kind = NodeKind::Union;
            }
            else if(tagDecl->isEnum()){
                kind = NodeKind::Enum;
            }
            else{
                kind = NodeKind::Class;
            }

            std::string hash = generateHash(GetCurrentQualifiedName(), kind);

            context->excludeNodes.try_emplace(hash);
            PopName();
        }
    }
    else if (Decl->getTypeSourceInfo()) {
        auto [_, unwrappedTL] = unwrapTypeLoc(Decl->getTypeSourceInfo()->getTypeLoc());
        if (unwrappedTL.getAs<clang::FunctionProtoTypeLoc>()) {
            Decl->printName(OS);
            PushName(nameBuf);
            std::string hash = generateHash(GetCurrentQualifiedName(), NodeKind::Typedef);
            context->excludeNodes.try_emplace(hash);
            PopName();
        }
    }

    return false;

}

bool alpha::TreeBuilder::BuildVarDecl(clang::VarDecl *Decl) {
    if (!IsFromMainFileAndNotLocal(Decl) || !Decl->hasGlobalStorage() || Decl->isInvalidDecl() || Decl->isTemplated()) return false;

    if(llvm::isa<clang::VarTemplateDecl>(Decl) || llvm::isa<clang::VarTemplatePartialSpecializationDecl>(Decl) 
    || llvm::isa<clang::VarTemplateSpecializationDecl>(Decl)) return true;

    if(const clang::TagDecl* tagDecl = unwrapType(Decl->getType())->getAsTagDecl()){
        if(IsFromMainFileAndNotLocal(tagDecl) && !tagDecl->hasNameForLinkage()) return false;
    }
    
    normalizeValueDeclNode(Decl);
    return true;
}

bool alpha::TreeBuilder::BuildFieldDecl(clang::FieldDecl *Decl) {
    if (!IsFromMainFileAndNotLocal(Decl) || Decl->isTemplated()) return false;

    if(const clang::TagDecl* tagDecl = unwrapType(Decl->getType())->getAsTagDecl()){
        if(IsFromMainFileAndNotLocal(tagDecl) && !tagDecl->hasNameForLinkage()) return false;
    }

    normalizeValueDeclNode(Decl);
    return true;
}