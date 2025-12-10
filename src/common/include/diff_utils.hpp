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
extern std::string FUNCTION_CALLING_CONVENTION;
extern std::string PACKED;
extern std::string INLINE;

const std::string serialize(const APINodeStorageClass& storageClass);

const std::string serialize(const ConstQualifier& qualifier);

const std::string serialize(const VirtualQualifier& qualifier);

const std::string serialize(const NodeKind& node);

const std::string serialize(const std::string& str);

const bool serialize(const bool& val);
