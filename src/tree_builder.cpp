// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "tree_builder.hpp"
#include "ast_normalized_context.hpp"
#include "astnormalizer.hpp"
#include "tree_builder_utils.hpp"
#include "iostream"
#include "node.hpp"
#include "debug_config.hpp"
#include "diff_utils.hpp"

TreeBuilder::TreeBuilder(ASTNormalizedContext* context): context(context) {}

inline bool TreeBuilder::IsFromMainFile(const clang::Decl* Decl) {
    clang::ASTContext* clangContext = &Decl->getASTContext();
    return clangContext->getSourceManager().isInMainFile(Decl->getLocation());
}

inline void TreeBuilder::AddNode(const std::shared_ptr<APINode>& node) {
    if (!nodeStack.empty()) {
        if (nodeStack.back()->children == nullptr) {
            nodeStack.back()->children = std::make_unique<llvm::SmallVector<std::shared_ptr<const APINode>, 16>>();
        }
        nodeStack.back()->children->push_back(node);
    }
    else context->addRootNode(node);
    
    if (nodeStack.empty()) context->addNode(node->qualifiedName, node);
}

inline void TreeBuilder::PushNode(const std::shared_ptr<APINode>& node) {
    nodeStack.push_back(node);
}

inline void TreeBuilder::PopNode() {
    if (!nodeStack.empty()) {
        nodeStack.pop_back();
    }
}

inline void TreeBuilder::PushName(llvm::StringRef name) {
    qualifiedNames.push(name);
}

inline void TreeBuilder::PopName() {
    qualifiedNames.pop();
}

inline const std::string TreeBuilder::GetCurrentQualifiedName() {
    return qualifiedNames.getAsString();
}

void TreeBuilder::normalizeFunctionPointerType(const std::string& dataType, clang::FunctionProtoTypeLoc FTL) {
    
    auto functionPointerNode = std::make_shared<APINode>();
    functionPointerNode->kind = NodeKind::FunctionPointer;
    functionPointerNode->qualifiedName = GetCurrentQualifiedName();
    functionPointerNode->dataType = dataType;
    
    AddNode(functionPointerNode);
    PushNode(functionPointerNode);
    
    for (clang::ParmVarDecl *paramDecl : FTL.getParams()) {
        normalizeValueDeclNode(paramDecl);
    }
    
    auto returnNode = std::make_shared<APINode>();
    returnNode->kind = NodeKind::ReturnType;
    returnNode->dataType = FTL.getReturnLoc().getType()->isIncompleteType() ? DATA_TYPE_PLACE_HOLDER : FTL.getReturnLoc().getType().getAsString();
    PushName("(returnType)");
    returnNode->qualifiedName = GetCurrentQualifiedName();
    PopName();
    
    AddNode(returnNode);
    PopNode();
}

void TreeBuilder::normalizeValueDeclNode(const clang::ValueDecl *Decl) {
    
    auto ValueNode = std::make_shared<APINode>();
    clang::QualType initialDeclType;
    clang::TypeSourceInfo *TSI = nullptr;

    if (const auto *paramDecl = llvm::dyn_cast_or_null<clang::ParmVarDecl>(Decl)) {
        ValueNode->kind = NodeKind::Parameter;
        initialDeclType = paramDecl->getOriginalType();
        TSI = paramDecl->getTypeSourceInfo();
    } 
    else if (const auto *fieldDecl = llvm::dyn_cast_or_null<clang::FieldDecl>(Decl)) {
        ValueNode->kind = NodeKind::Field;
        initialDeclType = fieldDecl->getType();
        TSI = fieldDecl->getTypeSourceInfo();
    } 
    else if (const auto *varDecl = llvm::dyn_cast_or_null<clang::VarDecl>(Decl)) {
        ValueNode->kind = NodeKind::Variable;
        ValueNode->storage = getStorageClass(varDecl->getStorageClass());
        initialDeclType = varDecl->getType();
        TSI = varDecl->getTypeSourceInfo();
    } 
    else return;

    std::string dataType = Decl->isInvalidDecl() ? DATA_TYPE_PLACE_HOLDER : initialDeclType.getAsString();
    if (Decl->getName().empty()) {
        PushName(std::string("(anonymous::parameter)::").append(dataType));
    } 
    else PushName(Decl->getName());
    
    ValueNode->qualifiedName = qualifiedNames.getAsString();

    if (const auto *paramDecl = llvm::dyn_cast_or_null<clang::ParmVarDecl>(Decl)) {
        DebugConfig::instance().log("VisitParamDecl : " + ValueNode->qualifiedName, DebugConfig::Level::DEBUG);
    } 
    else if (const auto *fieldDecl = llvm::dyn_cast_or_null<clang::FieldDecl>(Decl)) {
        DebugConfig::instance().log("VisitFeildDecl : " + ValueNode->qualifiedName, DebugConfig::Level::DEBUG);
    } 
    else if (const auto *varDecl = llvm::dyn_cast_or_null<clang::VarDecl>(Decl)) {
        DebugConfig::instance().log("VisitVarDecl : " + ValueNode->qualifiedName, DebugConfig::Level::DEBUG);
    } 

    AddNode(ValueNode);

    if (TSI) {
        auto unwrappedTL = unwrapTypeLoc(TSI->getTypeLoc());
        if (const clang::FunctionProtoTypeLoc FTL = unwrappedTL.second.getAs<clang::FunctionProtoTypeLoc>()) {
            nodeStack.push_back(ValueNode);
            normalizeFunctionPointerType(unwrappedTL.first, FTL);
            PopNode();
        }
        else ValueNode->dataType = dataType;
    }
    
    PopName();
}

bool TreeBuilder::BuildCXXRecordNode(clang::CXXRecordDecl* Decl) {
    if (!IsFromMainFile(Decl) || Decl->isClass() || Decl->getName().empty() || !Decl->isThisDeclarationADefinition()) return false;

    if (const auto *typedefForAnon = Decl->getTypedefNameForAnonDecl()) {
        return false;
    } 
    else{
        if(Decl->hasNameForLinkage()){
            PushName(Decl->getName());
        }
    }

    const std::string qualifiedName = GetCurrentQualifiedName();

    if(!Decl->isThisDeclarationADefinition()){
        if(context->getTree().count(qualifiedName)){
            PopName();
            return false;
        }
    }

    auto cxxRecordNode = std::make_shared<APINode>();

    cxxRecordNode->qualifiedName = qualifiedName;

    DebugConfig::instance().log("VisitCxxRecordDecl : " + qualifiedName, DebugConfig::Level::DEBUG);

    cxxRecordNode->isPacked = Decl->hasAttr<clang::PackedAttr>();
    
    if( Decl->isClass() ){
        cxxRecordNode->kind = NodeKind::Class;
    }
    else if( Decl->isStruct() ){
        cxxRecordNode->kind = NodeKind::Struct;
    }
    else if (Decl->isUnion()) {
        cxxRecordNode->kind = NodeKind::Union;
    }
    
    AddNode(cxxRecordNode);
    PushNode(cxxRecordNode);

    return true;
}


bool TreeBuilder::BuildEnumNode(clang::EnumDecl* Decl){

    if (!IsFromMainFile(Decl)) return false;

    bool isAnonymousEnumFieldVar = false;

    if (Decl->getIdentifier() == nullptr) {
        if (clang::Decl *Next = Decl->getNextDeclInContext()) {
            if (auto *fieldDecl = clang::dyn_cast_or_null<clang::FieldDecl>(Next)) {
                if (auto *ET = fieldDecl->getType()->getAs<clang::EnumType>()) {
                    if (ET->getDecl() == Decl) isAnonymousEnumFieldVar = true;
                }
            }
            else if (auto *varDecl = clang::dyn_cast_or_null<clang::VarDecl>(Next)) {
                if (auto *ET = varDecl->getType()->getAs<clang::EnumType>()) {
                    if (ET->getDecl() == Decl) isAnonymousEnumFieldVar = true;
                }
            }
        }
    }

    if(isAnonymousEnumFieldVar){
        return false;
    }

    auto enumNode = std::make_shared<APINode>();

    if (const auto *typedefForAnon = Decl->getTypedefNameForAnonDecl()) {
        return false;
    } 
    else{
        if(!isAnonymousEnumFieldVar){
            if( Decl->getName().empty() ){
                return false;
            }
            else PushName(Decl->getName());
        }
        enumNode->qualifiedName = GetCurrentQualifiedName();
    }

    DebugConfig::instance().log("VisitEnumDecl: " + enumNode->qualifiedName, DebugConfig::Level::DEBUG);

    enumNode->kind = NodeKind::Enum;
    
    if(!Decl->enumerators().empty()) nodeStack.push_back(enumNode);

    const clang::QualType enumType = Decl->getIntegerType();
    std::string enumaratorDataType = Decl->isInvalidDecl() ? DATA_TYPE_PLACE_HOLDER : enumType.getAsString();
     
    for (const auto* EnumConstDecl : Decl->enumerators()) {
        auto enumValNode = std::make_shared<APINode>();
        PushName(EnumConstDecl->getName());
        enumValNode->qualifiedName = GetCurrentQualifiedName();
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


bool TreeBuilder::BuildFunctionNode(clang::FunctionDecl* Decl){

    if (!IsFromMainFile(Decl)) return false;

    auto functionNode = std::make_shared<APINode>();
    PushName(Decl->getName());
    functionNode->qualifiedName = GetCurrentQualifiedName();
    functionNode->kind = NodeKind::Function;
    functionNode->storage = getStorageClass(Decl->getStorageClass());

    clang::ASTContext* clangContext = &Decl->getASTContext();
    const clang::TypeSourceInfo *TSI = Decl->getTypeSourceInfo();
    const clang::LangOptions LangOpts = clangContext->getLangOpts();
    clang::SourceManager &SM = clangContext->getSourceManager();
    const clang::SourceLocation begin = TSI->getTypeLoc().getEndLoc();
    const clang::SourceLocation end = clang::Lexer::getLocForEndOfToken(Decl->getTypeSpecEndLoc(), 0, SM, LangOpts);
    
    if (SM.getFileOffset(end) - SM.getFileOffset(begin) > 1) {
        const char *begin_char = SM.getCharacterData(begin);
        const char *end_char = SM.getCharacterData(end);
        functionNode->functionCallingConvention = std::string(begin_char, end_char - begin_char);
        DebugConfig::instance().log("functionCallingConvention : " + functionNode->functionCallingConvention, DebugConfig::Level::DEBUG);
    }
    
    if(Decl->isInlined()) functionNode->isInline = true;

    DebugConfig::instance().log("VisitFunctionDecl : " + functionNode->qualifiedName, DebugConfig::Level::DEBUG);

    nodeStack.push_back(functionNode);
    
    for (const auto *ParamVarDecl : Decl->parameters()) {
        normalizeValueDeclNode(ParamVarDecl);
    }
    
    auto returnNode = std::make_shared<APINode>();
    clang::QualType returnType = Decl->getReturnType();
    returnNode->dataType = returnType->isIncompleteType() ? DATA_TYPE_PLACE_HOLDER : returnType.getAsString();
    PushName(clang::StringRef("returnType"));
    returnNode->qualifiedName = GetCurrentQualifiedName();
    DebugConfig::instance().log("VisitFunctionReturnDecl : " + returnNode->qualifiedName, DebugConfig::Level::DEBUG);
    PopName();
    returnNode->kind = NodeKind::ReturnType;
    AddNode(returnNode);
    
    qualifiedNames.pop();
    nodeStack.pop_back();
    AddNode(functionNode);

    return true;
}


bool TreeBuilder::BuildTypedefDecl(clang::TypedefDecl *Decl) {
    if(!IsFromMainFile(Decl)) return false;

    clang::QualType underlyingType = Decl->getUnderlyingType();
    const clang::Type *typePtr = underlyingType.getTypePtr();

    if (const auto *tagDecl = typePtr->getAsTagDecl()) {
        if (tagDecl->isEmbeddedInDeclarator()) {
            if (auto nextDecl = tagDecl->getNextDeclInContext()) {
                if(!nextDecl->isInvalidDecl()){
                    const clang::TagDecl* nextTagDecl = nullptr;
                    
                    switch (nextDecl->getKind()) {
                        case clang::Decl::Var:
                            if (auto varDecl = llvm::dyn_cast_or_null<clang::VarDecl>(nextDecl))
                                nextTagDecl = unwrapType(varDecl->getType())->getAsTagDecl();
                            break;
                            
                        case clang::Decl::Field:
                            if (auto fieldDecl = llvm::dyn_cast_or_null<clang::FieldDecl>(nextDecl))
                                nextTagDecl = unwrapType(fieldDecl->getType())->getAsTagDecl();
                            break;
                        default: break;
                    }
                    
                    if (nextTagDecl == tagDecl) return false;
                }
            }

            PushName(tagDecl->getName());
            context->excludeNodes.try_emplace(GetCurrentQualifiedName());
            PopName();
        }
    }
    else if (Decl->getTypeSourceInfo()) {
        auto unwrappedTL = unwrapTypeLoc(Decl->getTypeSourceInfo()->getTypeLoc());
        if (unwrappedTL.second.getAs<clang::FunctionProtoTypeLoc>()) {
            PushName(Decl->getName());
            context->excludeNodes.try_emplace(GetCurrentQualifiedName());
            PopName();
        }
    }

    return false;
}

bool TreeBuilder::BuildVarDecl(clang::VarDecl *Decl) {
    if (!IsFromMainFile(Decl) || !Decl->hasGlobalStorage() || Decl->isInvalidDecl()) return false;

    if(const clang::TagDecl* tagDecl = Decl->getType()->getAsTagDecl()){
        if(IsFromMainFile(tagDecl) && !tagDecl->hasNameForLinkage()) return false;
    }
    
    normalizeValueDeclNode(Decl);
    return true;
}

bool TreeBuilder::BuildFieldDecl(clang::FieldDecl *Decl) {
    if (!IsFromMainFile(Decl)) return false;

    if(const clang::TagDecl* tagDecl = Decl->getType()->getAsTagDecl()){
        if(IsFromMainFile(tagDecl) && !tagDecl->hasNameForLinkage()) return false;
    }

    normalizeValueDeclNode(Decl);
    return true;
}
