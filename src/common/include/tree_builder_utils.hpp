// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"

#include "custom_usr_generator.hpp"
#include "comm_def.hpp"

APINodeStorageClass getStorageClass(const clang::StorageClass storage);

clang::QualType unwrapType(clang::QualType type);

std::pair<std::string, clang::TypeLoc> unwrapTypeLoc(clang::TypeLoc TL);

const std::string generateUSRForDecl(const clang::NamedDecl * Decl);

const std::string generateNSRForDecl(const clang::NamedDecl * Decl);

const std::pair<const std::string,const std::string> getTypesWithAndWithoutTypeResolution(const clang::QualType T, const clang::ASTContext &Ctx);

const std::string generateHash( llvm::StringRef qualifiedName , const NodeKind& node );