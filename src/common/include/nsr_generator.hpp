// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/Lex/PreprocessingRecord.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "clang/Basic/LLVM.h"

namespace armor {

static llvm::StringRef getNSRSpacePrefix() {
  return "c:";
}

/// Generate a USR for a Decl, including the USR prefix.
/// \returns true if the results should be ignored, false otherwise.
bool generateNSRForDecl(const clang::Decl *D, llvm::SmallVectorImpl<char> &Buf);

/// Generate USR fragment for a global (non-nested) enum.
void generateNSRForGlobalEnum(llvm::StringRef EnumName, llvm::raw_ostream &OS,
                              llvm::StringRef ExtSymbolDefinedIn = "");

/// Generate a USR fragment for an enum constant.
void generateNSRForEnumConstant(llvm::StringRef EnumConstantName, llvm::raw_ostream &OS);

/// Generate a USR for a macro, including the USR prefix.
///
/// \returns true on error, false on success.
bool generateNSRForMacro(const clang::MacroDefinitionRecord *MD,
                         const clang::SourceManager &SM, llvm::SmallVectorImpl<char> &Buf);
bool generateNSRForMacro(llvm::StringRef MacroName, clang::SourceLocation Loc,
                         const clang::SourceManager &SM, llvm::SmallVectorImpl<char> &Buf);

/// Generates a USR for a type.
///
/// \return true on error, false on success.
bool generateNSRForType(clang::QualType T, clang::ASTContext &Ctx, llvm::SmallVectorImpl<char> &Buf);

} // namespace armor
