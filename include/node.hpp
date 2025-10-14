// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

// Enums for categorization
enum class NodeKind {
    Namespace,
    Class,
    Struct,
    Union,
    Enum,
    Function,
    Method,
    Field,
    Typedef,
    TypeAlias,
    Parameter,
    TemplateParam,
    BaseClass,
    Variable,
    ReturnType,
    FunctionPointer,
    Enumerator,
    Macro,
    If,
    Elif,
    Ifdef,
    Ifndef,
    Elifndef,
    Else,
    Endif,
    Elifdef,
    Define,
    ConditionalCompilation,
    Unknown
};

enum class AccessSpec {
    Public,
    Protected,
    Private,
    None
};

enum class APINodeStorageClass {
    None,
    Static,
    Extern,
    Register,
    Auto
};

enum class ConstQualifier {
    None,
    Const,
    ConstExpr
};

enum class VirtualQualifier {
    None,
    Virtual,
    PureVirtual,
    Override
};

enum class FunctionCallingConvention {
    CDecl,
    StdCall,
    FastCall,
    ThisCall,
    VectorCall,
    Pascal,
    Win64,
    SysV,
    RegCall,
    AAPCS,
    AAPCS_VFP,
    IntelOclBicc,
    SpirFunction,
    OpenCLKernel,
    Swift,
    SwiftAsync,
    PreserveMost,
    PreserveAll,
    AArch64VectorCall,
    None
};


// Main Node structure
// string can be optimized by string_view powered by stringRef
struct APINode {
    NodeKind kind = NodeKind::Unknown;
    std::string qualifiedName;
    std::string typeName;         // To handle typdef of built-in/CxxRecordDecl/EnumDecl
    std::string dataType;         // Underlying datatype of variables .... (int/float/...)
    std::string value;            // Assigned value of variables, function params, enumuratoes ...
    AccessSpec access = AccessSpec::None;
    APINodeStorageClass storage = APINodeStorageClass::None;
    ConstQualifier constQualifier = ConstQualifier::None;
    VirtualQualifier virtualQualifier = VirtualQualifier::None;
    std::string functionCallingConvention;

    bool isInline = false;
    bool isPointer = false;
    bool isReference = false;
    bool isRValueRef = false;
    bool isPacked = false;

    std::string USR;
    std::unique_ptr<llvm::SmallVector<std::shared_ptr<const APINode>,16>> children;

    nlohmann::json diff(const std::shared_ptr<const APINode>& other) const;

    /*
    Specific to conditional compilation
    */

    std::string conditionString;
    std::string bodyString; 
    std::string hash; 
    bool isActive = false; 
};

// add const here as well.
typedef llvm::StringMap<std::shared_ptr<APINode>> normalizedTree;
typedef llvm::SmallVector<std::shared_ptr<const APINode>,64> rootApiNodes;
