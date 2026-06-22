// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "diff_utils.hpp"
#include "comm_def.hpp"
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
std::string INLINE = "inline";
std::string PARSED_STATUS = "parsed_status";
std::string UNPARSED_STATUS = "unparsed_status";
std::string HEADER_RESOLUTION_FAILURES = "headerResolutionFailures";
std::string AST_DIFF = "astDiff";
std::string CONST_EXPR = "constexpr";
std::string ACCESS_SPECIFIER = "AccessSpecifier";
std::string IS_OVERRIDE = "isOverride";
std::string IS_FINAL = "isFinal";
std::string IS_DELETE = "isDelete";
std::string IS_DEFAULT = "isDefault";
std::string IS_EXPLICIT = "isExplicit";
std::string IS_VOLATILE = "isVolatile";
std::string IS_CONST = "isConst";
std::string IS_FRIEND = "isFriend";

const std::string serialize(const APINodeStorageClass& storageClass) {
    switch (storageClass) {
        case APINodeStorageClass::Static:   return "Static";
        case APINodeStorageClass::Extern:   return "Extern";
        case APINodeStorageClass::Register: return "Register";
        case APINodeStorageClass::Auto:     return "Auto";
        default:                            return std::string{};
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
        case NodeKind::FriendFunction:         return "FriendFunction";
        case NodeKind::FriendClass:            return "FriendClass";
        case NodeKind::FriendUnion:            return "FriendUnion";
        case NodeKind::FriendStruct:           return "FriendStruct";
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

const std::string serialize(const AccessSpec& accessSpec) {
    switch (accessSpec) {
        case AccessSpec::Public:    return "Public";
        case AccessSpec::Protected: return "Protected";
        case AccessSpec::Private:   return "Private";
        case AccessSpec::None:      return "None";
        default:                    return std::string{};
    }
}

const std::string serialize(const ParsedDiffStatus& diff_status){
    switch (diff_status) {
        case ParsedDiffStatus::FATAL_ERRORS:           return "FATAL_ERRORS";
        case ParsedDiffStatus::UNSUPPORTED_UPDATES:    return "UNSUPPORTED_UPDATES";
        case ParsedDiffStatus::SUPPORTED_UPDATES:      return "SUPPORTED_UPDATES";
        case ParsedDiffStatus::COMMENTS_UPDATED:       return "COMMENTS_UPDATED";
        case ParsedDiffStatus::NON_FUNCTIONAL_CHANGES: return "NON_FUNCTIONAL_CHANGES";
        default:                                       return "UNKNOWN";
    }
}
const std::string serialize(const UnParsedDiffStatus& diff_status){
    switch (diff_status) {
        case UnParsedDiffStatus::UN_CHANGED: return "UNCHANGED";
        case UnParsedDiffStatus::CHANGED:    return "CHANGED";
        default:                             return "FATAL_ERRORS";
    }
}