// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"

#include "node.hpp"
#include "ast_normalized_context.hpp"

namespace alpha {

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
    
    private:

        clang::SourceManager* SM;
        ASTNormalizedContext* context;
};

}