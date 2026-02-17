// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "tree_builder_utils.hpp"
#include "custom_usr_generator.hpp"
#include "logger.hpp"
#include "nsr_generator.hpp"

#include "clang/AST/Decl.h"
#include "clang/Lex/Lexer.h"
#include <llvm-14/llvm/ADT/SmallString.h>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include <string>
#include "diff_utils.hpp"

clang::QualType unwrapType(clang::QualType type) {
    if (type.isNull()) return type;
    
    while (true) {
        
        clang::QualType unqualifiedType = type.getUnqualifiedType();
        if (unqualifiedType != type) {
            type = unqualifiedType;
            continue;
        }

        if (const auto* parenType = type->getAs<clang::ParenType>()) {
            type = parenType->getInnerType();
        } 
        else if (type->isPointerType() || type->isReferenceType()) {
            type = type->getPointeeType();
        } 
        else if (const auto* arrayType = type->getAsArrayTypeUnsafe()) {
            type = arrayType->getElementType();
        } 
        else if (const auto* attributedType = type->getAs<clang::AttributedType>()) {
            type = attributedType->getModifiedType();
        } 
        else break;
    }
    return type;
}

std::pair<std::string, clang::TypeLoc> unwrapTypeLoc(clang::TypeLoc TL) {
    if (TL.isNull()) {
        return {std::string{}, TL};
    }

    llvm::SmallVector<const char*, 128> modifiers;

    while (true) {
        bool unwrappedThisIteration = false;

        clang::Qualifiers quals = TL.getType().getLocalQualifiers();
        if (!quals.empty()) {
            if (quals.hasConst())    modifiers.emplace_back("const ");
            if (quals.hasVolatile()) modifiers.emplace_back("volatile ");
            if (quals.hasRestrict()) modifiers.emplace_back("restrict ");
            TL = TL.getUnqualifiedLoc();
            unwrappedThisIteration = true;
        }
        
        switch (TL.getTypePtr()->getTypeClass()) {
            case clang::Type::Pointer:
                modifiers.emplace_back("*");
                TL = TL.getAs<clang::PointerTypeLoc>().getPointeeLoc();
                unwrappedThisIteration = true;
                break;
            case clang::Type::LValueReference:
                modifiers.emplace_back("&");
                TL = TL.getAs<clang::LValueReferenceTypeLoc>().getPointeeLoc();
                unwrappedThisIteration = true;
                break;
            case clang::Type::RValueReference:
                modifiers.emplace_back("&&");
                TL = TL.getAs<clang::RValueReferenceTypeLoc>().getPointeeLoc();
                unwrappedThisIteration = true;
                break;
            case clang::Type::Paren:
                TL = TL.getAs<clang::ParenTypeLoc>().getInnerLoc();
                unwrappedThisIteration = true;
                break;
            case clang::Type::ConstantArray:
            case clang::Type::IncompleteArray:
            case clang::Type::VariableArray:
            case clang::Type::DependentSizedArray:
                TL = TL.getAs<clang::ArrayTypeLoc>().getElementLoc();
                unwrappedThisIteration = true;
                break;
            
            default:
                break;
        }

        if (!unwrappedThisIteration) {
            break;
        }
    }

    llvm::SmallString<32> resultBuffer;
    while (!modifiers.empty()) {
        const char* mod = modifiers.pop_back_val();
        resultBuffer.append(mod);
    }

    return {std::string(resultBuffer.str()), TL};
}

APINodeStorageClass getStorageClass(const clang::StorageClass storage) {
    switch (storage) {
        case clang::StorageClass::SC_Static:
            return APINodeStorageClass::Static;
        case clang::SC_Extern:
            return APINodeStorageClass::Extern;
        case clang::SC_Register:
            return APINodeStorageClass::Register;
        case clang::StorageClass::SC_Auto:
            return APINodeStorageClass::Auto;
        default:
            return APINodeStorageClass::None;
    }
};

const std::string generateUSRForDecl(const clang::NamedDecl * Decl){
    
    if (llvm::isa<clang::ParmVarDecl>(Decl)|| llvm::isa<clang::TemplateTypeParmDecl>(Decl) 
    || llvm::isa<clang::NonTypeTemplateParmDecl>(Decl) || llvm::isa<clang::TemplateTemplateParmDecl>(Decl)) {
        armor::info() << "No USR for Param type declerations \n";
        return std::string{};
    }
    
    llvm::SmallString<256> Buf;
    armor::generateUSRForDecl(Decl, Buf);

    return Buf.c_str();

}

const std::string generateNSRForDecl(const clang::NamedDecl * Decl){

    if (llvm::isa<clang::ParmVarDecl>(Decl)|| llvm::isa<clang::TemplateTypeParmDecl>(Decl) 
    || llvm::isa<clang::NonTypeTemplateParmDecl>(Decl) || llvm::isa<clang::TemplateTemplateParmDecl>(Decl)) {
        armor::info() << "No NSR for Param type declerations \n";
        return std::string{};
    }

    llvm::SmallString<256> Buf;
    armor::generateNSRForDecl(Decl, Buf);

    return Buf.c_str();

}


const std::pair<const std::string,const std::string> getTypesWithAndWithoutTypeResolution(const clang::QualType T, const clang::ASTContext &Ctx) {
    
    if (T.isNull()) {
        return {std::string{}, std::string{}};
    }
    
    clang::PrintingPolicy Policy1(Ctx.getLangOpts());
    Policy1.SuppressTagKeyword = false;
    Policy1.SuppressScope = false;
    Policy1.FullyQualifiedName = true;
    Policy1.AnonymousTagLocations = false;
    Policy1.PrintCanonicalTypes = false;
    
    std::string TypeStr;
    llvm::raw_string_ostream OS(TypeStr);
    
    try {
        T.print(OS, Policy1);
    } 
    catch (...) {
        TypeStr = std::string{};
    }

    clang::PrintingPolicy Policy2(Ctx.getLangOpts());
    Policy2.SuppressTagKeyword = false;
    Policy2.SuppressScope = false;
    Policy2.FullyQualifiedName = true;
    Policy2.AnonymousTagLocations = false;
    Policy2.PrintCanonicalTypes = false;
    Policy2.PrintCanonicalTypes = true;

    std::string CanonicalTypeStr;
    llvm::raw_string_ostream COS(CanonicalTypeStr);

    try {
        T.getCanonicalType().print(COS, Policy2);
    } 
    catch (...) {
        CanonicalTypeStr = std::string{};
    }
    
    return {TypeStr,CanonicalTypeStr};
    
}

const std::string generateHash( llvm::StringRef qualifiedName , const NodeKind& node ){
    
    llvm::SmallString<128> hashBuf;
    llvm::raw_svector_ostream OS(hashBuf);

    OS << serialize(node) << ":" <<  qualifiedName;
    
    return hashBuf.c_str();

}