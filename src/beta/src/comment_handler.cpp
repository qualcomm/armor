// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "comment_handler.hpp"
#include "ast_normalized_context.hpp"
#include "fibonacci_hash.hpp"
#include "logger.hpp"

#include "clang/Lex/Lexer.h"
#include "llvm/ADT/StringRef.h"
#include <cassert>
#include <llvm-14/llvm/Support/raw_ostream.h>

namespace beta {

CommentHandler::CommentHandler(clang::SourceManager* SM, beta::ASTNormalizedContext* context)
    : SM(SM), context(context) {}

uint64_t CommentHandler::generateHashFromSourceRange(clang::SourceRange Range) {
    if (!Range.isValid()) return 0;
    
    clang::SourceLocation StartLoc = Range.getBegin();
    clang::SourceLocation EndLoc = Range.getEnd();
    
    llvm::StringRef sourceText;
    
    if (StartLoc.isValid() && EndLoc.isValid()) {
        clang::CharSourceRange CharRange = clang::CharSourceRange::getTokenRange(StartLoc, EndLoc);
        sourceText = clang::Lexer::getSourceText(CharRange, *SM, clang::LangOptions());
        uint64_t semanticHash = FibonacciHash::hash(sourceText);
        TEST_LOG << semanticHash << "\n";
        TEST_LOG << sourceText << "\n----------------------------------------\n";
        return semanticHash;
    }
    
    return 0;
}

uint64_t CommentHandler::generateHashFromOffsets(unsigned startOffset, unsigned endOffset) {
    if (startOffset >= endOffset) return 0;
    
    llvm::StringRef buffer = SM->getBufferData(SM->getMainFileID());
    
    if (endOffset > buffer.size()) return 0;
    
    llvm::StringRef sourceText = buffer.substr(startOffset, endOffset - startOffset + 1);
    uint64_t semanticHash = FibonacciHash::hash(sourceText);
    TEST_LOG << semanticHash << "\n";
    TEST_LOG << sourceText << "\n----------------------------------------\n";

    return semanticHash;
}

bool CommentHandler::HandleComment(clang::Preprocessor& PP, clang::SourceRange Comment) {
    
    if (!context) return false;

    if(Comment.isInvalid()) return false;

    clang::SourceLocation CommentLoc = Comment.getBegin();
    
    if (!SM->isInMainFile(CommentLoc)) return false;

    uint64_t hash = generateHashFromSourceRange(Comment);
    unsigned startOffset = SM->getFileOffset(Comment.getBegin());
    unsigned endOffset = SM->getFileOffset(Comment.getEnd());
    comments.emplace_back(beta::Range(startOffset,endOffset,hash,true));
    commentsHashMap[hash]++;

    return false;
}

void CommentHandler::finalize(){
    beta::SourceRangeTracker& SRT = context->getSourceRangeTracker();
    SRT.moveComments(comments);
    SRT.moveCommentsHashMap(commentsHashMap);
}

void filterCommentsInInactiveRegions(beta::ASTNormalizedContext* context, clang::SourceManager* SM) {
    
    if (!context || !SM) return;
    
    auto& tracker = context->getSourceRangeTracker();
    const llvm::SmallVector<beta::Range, 32>& comments = tracker.getComments();
    const std::map<unsigned,beta::Range>& inactiveRegions = tracker.getInactivePPDirectives();
    llvm::DenseMap<uint64_t, int>& commentsHashMap = tracker.getCommentsHashMap();
    
    if (inactiveRegions.empty()) return;
    
    llvm::SmallVector<uint64_t, 32> hashesToDecrement;
    
    llvm::StringRef buffer = SM->getBufferData(SM->getMainFileID());

    for (const auto& commentRange : comments) {
        unsigned commentStartOffset = commentRange.startOffset;
        unsigned commentEndOffset = commentRange.endOffset;
        
        llvm::StringRef sourceText = buffer.substr(commentStartOffset, commentEndOffset - commentStartOffset);
        
        auto it = inactiveRegions.lower_bound(commentStartOffset);
        
        if (it != inactiveRegions.begin()) it--;

        const auto& [regionStartOffset, regionRange] = *it;
        unsigned regionEndOffset = regionRange.endOffset;
        
        assert(regionRange.hash != -1);

        if (commentStartOffset >= regionRange.startOffset && commentEndOffset <= regionRange.endOffset) {
            hashesToDecrement.push_back(commentRange.hash);
            TEST_LOG << "Flush : \n" << buffer.substr(commentStartOffset, commentEndOffset - commentStartOffset)
            << "\n-------------------------------------------\n";
        }

    }
    
    for (uint64_t hash : hashesToDecrement) {
        auto it = commentsHashMap.find(hash);
        if (it != commentsHashMap.end()) {
            it->second--;
            if (it->second <= 0) commentsHashMap.erase(it);
        }
    }
    
}

}