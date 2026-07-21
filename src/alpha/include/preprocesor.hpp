// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "clang/Basic/SourceManager.h"
#include "clang/Lex/PPCallbacks.h"

#include "ast_normalized_context.hpp"

namespace armor { namespace alpha {

class ASTNormalizerPreprocessor : public clang::PPCallbacks {

    public:

        ASTNormalizerPreprocessor(clang::SourceManager* SM, armor::ASTNormalizedContext* context);

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
        armor::ASTNormalizedContext* context;
};

} } // namespace armor::alpha
