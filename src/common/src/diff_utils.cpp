// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include "diff_utils.hpp"
#include <llvm-14/llvm/ADT/SmallString.h>
#include <llvm-14/llvm/Support/raw_ostream.h>

std::string DATA_TYPE_PLACE_HOLDER = "(un-resolved Type)";

std::string ADDED = "added";
std::string REMOVED = "removed";
std::string MODIFIED = "modified";
std::string REORDERED = "re-ordered";

// JSON keys
std::string QUALIFIED_NAME = "qualifiedName";
std::string NODE_TYPE = "nodeType";
std::string TAG = "tag";
std::string CHILDREN = "children";
std::string DATA_TYPE = "dataType";
std::string STORAGE_QUALIFIER = "storageQualifier";
std::string CONST_QUALIFIER = "constQualifier";
std::string VIRTUAL_QUALIFIER = "virtualQualifier";
std::string FUNCTION_CALLING_CONVENTION = "functionCallingConvention";
std::string PACKED = "packed";
std::string INLINE = "inline";

const std::string serialize(const APINodeStorageClass& storageClass) {
    switch (storageClass) {
        case APINodeStorageClass::Static:   return "Static";
        case APINodeStorageClass::Extern:   return "Extern";
        case APINodeStorageClass::Register: return "Register";
        case APINodeStorageClass::Auto:     return "Auto";
        default:                            return std::string{};
    }
}

const std::string serialize(const ConstQualifier& qualifier) {
    switch (qualifier) {
        case ConstQualifier::Const:     return "Const";
        case ConstQualifier::ConstExpr: return "ConstExpr";
        default:                        return std::string{};
    }
}

const std::string serialize(const VirtualQualifier& qualifier) {
    switch (qualifier) {
        case VirtualQualifier::Virtual:     return "Virtual";
        case VirtualQualifier::PureVirtual: return "PureVirtual";
        case VirtualQualifier::Override:    return "Override";
        default:                            return std::string{};
    }
}

const std::string serialize(const NodeKind& node) {
    switch (node) {
        case NodeKind::Namespace:              return "Namespace";
        case NodeKind::Class:                  return "Class";
        case NodeKind::Struct:                 return "Struct";
        case NodeKind::Union:                  return "Union";
        case NodeKind::Enum:                   return "Enum";
        case NodeKind::Function:               return "Function";
        case NodeKind::Method:                 return "Method";
        case NodeKind::Field:                  return "Field";
        case NodeKind::Typedef:                return "Typedef";
        case NodeKind::TypeAlias:              return "TypeAlias";
        case NodeKind::Parameter:              return "Parameter";
        case NodeKind::TemplateParam:          return "TemplateParam";
        case NodeKind::BaseClass:              return "BaseClass";
        case NodeKind::Variable:               return "Variable";
        case NodeKind::ReturnType:             return "ReturnType";
        case NodeKind::Enumerator:             return "Enumerator";
        case NodeKind::Macro:                  return "Macro";
        case NodeKind::If:                     return "If";
        case NodeKind::Elif:                   return "Elif";
        case NodeKind::Ifdef:                  return "Ifdef";
        case NodeKind::Ifndef:                 return "Ifndef";
        case NodeKind::Elifndef:               return "Elifndef";
        case NodeKind::Else:                   return "Else";
        case NodeKind::Endif:                  return "Endif";
        case NodeKind::Elifdef:                return "Elifdef";
        case NodeKind::Define:                 return "Define";
        case NodeKind::ConditionalCompilation: return "ConditionalCompilation";
        case NodeKind::Unknown:                return "Unknown";
        case NodeKind::FunctionPointer:        return "FunctionPointer";
        default:                           return "Invalid";
    }
}


const std::string serialize(const std::string& str){
    return str;
}

const bool serialize(const bool& val){
    return val;
}
