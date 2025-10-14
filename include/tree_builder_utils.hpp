// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"

#include "node.hpp"

APINodeStorageClass getStorageClass(const clang::StorageClass storage);

clang::QualType unwrapType(clang::QualType type);

std::pair<std::string, clang::TypeLoc> unwrapTypeLoc(clang::TypeLoc TL);
