// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/Lex/PreprocessingRecord.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "clang/Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"

namespace armor {

/// Generate a USR for a Decl, including the USR prefix.
/// \returns true if the results should be ignored, false otherwise.
bool generateQualifiedNameForDecl(const clang::Decl *D, llvm::SmallVectorImpl<char> &Buf);

} // namespace armor
