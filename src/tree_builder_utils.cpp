// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "tree_builder_utils.hpp"

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