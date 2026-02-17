// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "llvm/ADT/SmallVector.h"
#include <utility>

#include "node.hpp"
#include "ast_normalized_context.hpp"

namespace beta {

class ASTNormalizerPreprocessor : public clang::PPCallbacks {

    public:

    ASTNormalizerPreprocessor(clang::SourceManager* SM, ASTNormalizedContext* context);
    
    void InclusionDirective(
        clang::SourceLocation HashLoc,
        const clang::Token &IncludeTok,
        clang::StringRef FileName,
        bool IsAngled,
        clang::CharSourceRange FilenameRange,
        const clang::FileEntry *File,
        clang::StringRef SearchPath,
        clang::StringRef RelativePath,
        const clang::Module *Imported,
        clang::SrcMgr::CharacteristicKind FileType);
    
    void MacroDefined(const clang::Token &MacroNameTok, const clang::MacroDirective *MD);

    void MacroUndefined(const clang::Token &MacroNameTok, const clang::MacroDefinition &MD, const clang::MacroDirective *Undef);

    void If(clang::SourceLocation Loc, clang::SourceRange ConditionRange,
                  clang::PPCallbacks::ConditionValueKind ConditionValue);
    
    void Elif(clang::SourceLocation Loc, clang::SourceRange ConditionRange,
                    clang::PPCallbacks::ConditionValueKind ConditionValue, clang::SourceLocation IfLoc);
    
    void Ifdef(clang::SourceLocation Loc, const clang::Token &MacroNameTok,
                   const clang::MacroDefinition &MD);

    void Elifdef(clang::SourceLocation Loc, clang::SourceRange ConditionRange,
                   clang::SourceLocation IfLoc);

    void Ifndef(clang::SourceLocation Loc, const clang::Token &MacroNameTok,
                   const clang::MacroDefinition &MD);

    void Elifndef(clang::SourceLocation Loc, clang::SourceRange ConditionRange,
                    clang::SourceLocation IfLoc);

    void Else(clang::SourceLocation Loc, clang::SourceLocation IfLoc);

    void Endif(clang::SourceLocation Loc, clang::SourceLocation IfLoc);

    void SourceRangeSkipped(clang::SourceRange Range, clang::SourceLocation EndifLoc);

    void finalize();

    private:

    clang::SourceManager* SM;
    ASTNormalizedContext* context;

    // Temporary storage for preprocessing
    llvm::SmallVector<beta::Range, 16> PPDirectives;
    std::map<unsigned, beta::Range> inactivePPDirectives;
    llvm::DenseMap<uint64_t, int> inactiveUnhandledDeclsHash;
    
    uint64_t generateHashFromOffsets(unsigned startOffset, unsigned endOffset, bool isActive);
    void addRange(clang::SourceRange range, bool active=true);
    void removeNestedRanges();
    void printRanges();
};

}