// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include "type_utils.hpp"

using namespace clang;

namespace armor {

QualType unwrapTypeModifiers(QualType OriginalType) {
    if (OriginalType.isNull())
        return OriginalType;
    
    QualType QT = OriginalType;
    
    // Unwrap only type modifiers, not sugar types
    while (!QT.isNull()) {
        // Handle pointer types
        if (const PointerType* PT = QT->getAs<PointerType>()) {
            QT = PT->getPointeeType();
            continue;
        }
        
        // Handle reference types
        if (const ReferenceType* RT = QT->getAs<ReferenceType>()) {
            QT = RT->getPointeeType();
            continue;
        }
        
        // Handle array types
        if (const ArrayType* AT = QT->getAsArrayTypeUnsafe()) {
            QT = AT->getElementType();
            continue;
        }
        
        // Handle member pointers
        if (const MemberPointerType* MPT = QT->getAs<MemberPointerType>()) {
            QT = MPT->getPointeeType();
            continue;
        }
        
        // If we get here, we've unwrapped all modifiers
        break;
    }
    
    return QT;
}

} // namespace armor