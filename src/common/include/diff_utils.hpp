// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <llvm-14/llvm/ADT/SmallString.h>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <string>
#include "comm_def.hpp"

extern std::string DATA_TYPE_PLACE_HOLDER;

extern std::string ADDED;
extern std::string REMOVED;
extern std::string MODIFIED;
extern std::string REORDERED;

// JSON keys
extern std::string QUALIFIED_NAME;
extern std::string NODE_TYPE;
extern std::string TAG;
extern std::string CHILDREN;
extern std::string DATA_TYPE;
extern std::string STORAGE_QUALIFIER;
extern std::string CONST_QUALIFIER;
extern std::string VIRTUAL_QUALIFIER;
extern std::string INLINE;
extern std::string PARSED_STATUS;
extern std::string UNPARSED_STATUS;
extern std::string HEADER_RESOLUTION_FAILURES;
extern std::string AST_DIFF;
extern std::string CONST_EXPR;

enum class ParsedDiffStatus {
    FATAL_ERRORS = 0,          // Critical errors occurred (e.g., header resolution failures)
    UNSUPPORTED_UPDATES = 1,          // At least one change is not supported (may include comments or whitespace changes or supported changes)
    SUPPORTED_UPDATES = 2,            // All changes are supported (may include comments or whitespace changes)
    COMMENTS_UPDATED = 3,     // Only comments or whitespace/formatting were modified
    NON_FUNCTIONAL_CHANGES  = 4   // No code changes
};

enum class UnParsedDiffStatus{
    UN_CHANGES =0,
    CHANGED = 1
};

const std::string serialize(const APINodeStorageClass& storageClass);

const std::string serialize(const VirtualQualifier& qualifier);

const std::string serialize(const NodeKind& node);

const std::string serialize(const std::string& str);

const bool serialize(const bool& val);

const std::string serialize(const ParsedDiffStatus& diff_status);

const std::string serialize(const UnParsedDiffStatus& diff_status);