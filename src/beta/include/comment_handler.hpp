// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstdint>
#include <llvm-14/llvm/ADT/SmallVector.h>
#include <vector>
#include <string>

#include "ast_normalized_context.hpp"

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"

namespace beta {

void filterCommentsInInactiveRegions(beta::ASTNormalizedContext* context, clang::SourceManager* SM);

class CommentHandler : public clang::CommentHandler {

    public:
        explicit CommentHandler(clang::SourceManager* SM, beta::ASTNormalizedContext* context);
        
        bool HandleComment(clang::Preprocessor& PP, clang::SourceRange Comment) override;

        uint64_t generateHashFromSourceRange(clang::SourceRange Range);
        uint64_t generateHashFromOffsets(unsigned startOffset, unsigned endOffset);

        void finalize();

    private:

        llvm::SmallVector<beta::Range, 32> comments;
        llvm::DenseMap<uint64_t, int> commentsHashMap;
        
        clang::SourceManager* SM;
        beta::ASTNormalizedContext* context;
};

} 