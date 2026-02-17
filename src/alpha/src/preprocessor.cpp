// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "ast_normalized_context.hpp"
#include "preprocesor.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"
#include <cassert>
#include <llvm-14/llvm/Support/FileSystem.h>
#include <llvm-14/llvm/Support/Path.h>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include "logger.hpp"

namespace alpha{

ASTNormalizerPreprocessor::ASTNormalizerPreprocessor(clang::SourceManager* SM, ASTNormalizedContext* context) 
: SM(SM), context(context) {}

void ASTNormalizerPreprocessor::InclusionDirective(
    clang::SourceLocation HashLoc, 
    const clang::Token &IncludeTok, 
    clang::StringRef FileName, 
    bool IsAngled, 
    clang::CharSourceRange FilenameRange, 
    const clang::FileEntry *File, 
    clang::StringRef SearchPath, 
    clang::StringRef RelativePath, 
    const clang::Module *Imported, 
    clang::SrcMgr::CharacteristicKind FileType){
    
    if (!HashLoc.isValid()) return;
    
    if(!File){

        if (HashLoc.isValid()) {
            clang::PresumedLoc PLoc = SM->getPresumedLoc(HashLoc);
            if (PLoc.isValid()) {
                armor::debug() << "Failed include - RelativePath: " << RelativePath << " at " << PLoc.getFilename() << "\n";
                context->getSourceRangeTracker().addFatalDirective(RelativePath,PLoc.getFilename());
            }
        }
    
    }

}

}