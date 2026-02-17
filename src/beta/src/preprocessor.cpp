// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "node.hpp"
#include "preprocesor.hpp"
#include "ast_normalized_context.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "fibonacci_hash.hpp"
#include "clang/Lex/Lexer.h"
#include <cassert>
#include <algorithm>
#include <cstddef>
#include <llvm-14/llvm/ADT/StringRef.h>
#include <llvm-14/llvm/Support/FileSystem.h>
#include <llvm-14/llvm/Support/Path.h>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include <utility>
#include "logger.hpp"

namespace beta{

inline bool isLocationInMainFile(clang::SourceManager* SM, clang::SourceLocation Loc) {
    if (!Loc.isValid()) return false;
    return SM->isInMainFile(Loc);
}

inline bool isBuiltinOrPredefinedMacro(clang::SourceManager* SM, const clang::MacroInfo* MI) {
    if (!MI) return false;
    
    // Check if it's a builtin macro like __LINE__, __FILE__, etc.
    if (MI->isBuiltinMacro()) return true;
    
    // Check if it's from the <built-in> buffer
    clang::SourceLocation Loc = MI->getDefinitionLoc();
    if (!Loc.isValid()) return false;
    
    clang::PresumedLoc PLoc = SM->getPresumedLoc(Loc);
    if (!PLoc.isValid()) return false;
    
    // Skip macros from <built-in> (predefines buffer)
    if (SM->isWrittenInBuiltinFile(Loc)) return true;

    // Skip macros from <command line> (-D flags)
    if (SM->isWrittenInCommandLineFile(Loc)) return true;

    return false;
}

inline bool isBuiltinOrPredefinedInclude(clang::SourceManager* SM, clang::SourceLocation HashLoc) {
    if (!HashLoc.isValid()) return false;
    
    clang::PresumedLoc PLoc = SM->getPresumedLoc(HashLoc);
    if (!PLoc.isValid()) return false;
    
    // Skip includes from <built-in> (predefines buffer)
    if (SM->isWrittenInBuiltinFile(HashLoc)) return true;

    // Skip includes from <command line> (-include flags)
    if (SM->isWrittenInCommandLineFile(HashLoc)) return true;

    return false;
}

ASTNormalizerPreprocessor::ASTNormalizerPreprocessor(clang::SourceManager* SM, ASTNormalizedContext* context) 
: SM(SM), context(context) {}

void ASTNormalizerPreprocessor::finalize(){
    
    removeNestedRanges();
    beta::SourceRangeTracker& SRT = context->getSourceRangeTracker();
    
    llvm::StringRef buffer = SM->getBufferData(SM->getMainFileID());
    
    for(beta::Range& R : PPDirectives){
        uint64_t hash = generateHashFromOffsets(R.startOffset, R.endOffset,R.isActive);
        if(hash != 0) {
            R.hash = hash;
            if(R.isActive) {
                SRT.addUnhandledDeclHash(hash);
            } 
            else {
                inactiveUnhandledDeclsHash[hash]++;;
                inactivePPDirectives[R.startOffset] = R;
            }
        }
    }
    
    SRT.moveInactivePPDirectives(inactivePPDirectives);
    SRT.moveInactiveUnhandledDeclsHashMap(inactiveUnhandledDeclsHash);

}

uint64_t ASTNormalizerPreprocessor::generateHashFromOffsets(unsigned startOffset, unsigned endOffset, bool isActive) {
    if (startOffset >= endOffset) return -1;
    
    llvm::StringRef buffer = SM->getBufferData(SM->getMainFileID());
    
    if (endOffset > buffer.size()) return -1;
    
    llvm::StringRef sourceText = buffer.substr(startOffset, endOffset - startOffset);
    uint64_t semanticHash = -1;
    if(isActive){
        semanticHash = FibonacciHash::hashFromOffsets(SM, startOffset, endOffset);
    }
    else{
        semanticHash = FibonacciHash::hash(sourceText);
    }
    TEST_LOG << semanticHash << "\n";
    TEST_LOG << sourceText << "\n----------------------------------------\n";
    
    return semanticHash;
}

void ASTNormalizerPreprocessor::addRange(clang::SourceRange range, bool active) {
    if (!range.isValid()) return;
    unsigned startOffset = SM->getFileOffset(range.getBegin());
    
    // Get the actual end of the last token, not just its start location
    clang::SourceLocation endLoc = clang::Lexer::getLocForEndOfToken(
        range.getEnd(), 0, *SM, clang::LangOptions());

    unsigned endOffset = SM->getFileOffset(endLoc);
    PPDirectives.emplace_back(beta::Range(startOffset, endOffset, -1, active));
}

void ASTNormalizerPreprocessor::removeNestedRanges() {
    if (PPDirectives.size() <= 1) return;
    
    llvm::SmallVector<beta::Range, 16> filteredRanges;
    
    size_t size = PPDirectives.size();
    unsigned refStartOffset = PPDirectives[size-1].startOffset;
    unsigned refEndOffset = PPDirectives[size-1].endOffset;
    bool refStatus = PPDirectives[size-1].isActive;

    llvm::StringRef buffer = SM->getBufferData(SM->getMainFileID());

    for (int idx = size - 2; idx >= 0; --idx) {
        unsigned currentStart = PPDirectives[idx].startOffset;
        unsigned currentEnd = PPDirectives[idx].endOffset;
        bool currentStatus = PPDirectives[idx].isActive;
    
        if (refStartOffset <= currentStart && currentEnd <= refEndOffset) {
            continue;
        }
        else if (currentStart < refStartOffset && (currentEnd <= refStartOffset || refStartOffset < currentEnd)) {
            filteredRanges.emplace_back(beta::Range(refStartOffset, refEndOffset,-1,refStatus));
            refStatus = currentStatus;
            refStartOffset = currentStart;
            refEndOffset = currentEnd;
        }
        else {
            armor::user_error()<<"Out of order PPDirective ranges detected\n";
            armor::user_error()<<"["<<currentStart<<","<<currentEnd<<"]"<<"\n";
            armor::user_error()<<"["<<refStartOffset<<","<<refEndOffset<<"]"<<"\n";
            // physically never possible
            assert(false && "Out of order ranges detected");
        }
    }
    
    filteredRanges.emplace_back(
        beta::Range(
            refStartOffset, 
            refEndOffset ,
            -1,
            refStatus
        )
    );
    
    std::reverse(filteredRanges.begin(), filteredRanges.end());

    PPDirectives = std::move(filteredRanges);

}

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
    
    if (!isLocationInMainFile(SM, HashLoc)) return;

    if (isBuiltinOrPredefinedInclude(SM, HashLoc)) return;

    if(!File){ 
        armor::debug() << "Failed include - RelativePath: " << RelativePath;
        
        if (HashLoc.isValid()) {
            clang::PresumedLoc PLoc = SM->getPresumedLoc(HashLoc);
            if (PLoc.isValid()) {
                armor::debug() << " at " << PLoc.getFilename();
            } else {
                armor::debug() << " at hash: " << HashLoc.getHashValue();
            }
        }
        
        armor::debug() << "\n";
    } 
    else {
        clang::SourceRange Range(HashLoc, FilenameRange.getEnd());
        if(HashLoc.isValid() && FilenameRange.getEnd().isValid()){
            addRange(Range);
        }
    }
}

void ASTNormalizerPreprocessor::MacroDefined(const clang::Token &MacroNameTok, const clang::MacroDirective *MD) {
    if (!MD) return;
    
    const clang::MacroInfo *MI = MD->getMacroInfo();
    if (!MI) return;
    
    if (!isLocationInMainFile(SM, MI->getDefinitionLoc())) return;
    
    if (isBuiltinOrPredefinedMacro(SM, MI)) return;
    
    clang::SourceLocation DefLoc = MI->getDefinitionLoc();
    if (!DefLoc.isValid()) return;
    
    unsigned LineNo = SM->getExpansionLineNumber(DefLoc);
    clang::FileID FID = SM->getFileID(DefLoc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, LineNo, 1);
    
    clang::SourceLocation End = MI->getDefinitionEndLoc();
    
    if (LineStart.isValid() && End.isValid()) {
        clang::SourceRange Range(LineStart, End);
        addRange(Range);
    }
}

void ASTNormalizerPreprocessor::MacroUndefined(const clang::Token &MacroNameTok, const clang::MacroDefinition &MD, const clang::MacroDirective *Undef) {
    if (!isLocationInMainFile(SM, MacroNameTok.getLocation())) return;
    
    const clang::MacroInfo *MI = MD.getMacroInfo();
    
    if (MI && isBuiltinOrPredefinedMacro(SM, MI)) return;
    
    clang::SourceLocation UndefLoc = MacroNameTok.getLocation();
    if (!UndefLoc.isValid()) return;
    
    unsigned LineNo = SM->getExpansionLineNumber(UndefLoc);
    clang::FileID FID = SM->getFileID(UndefLoc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, LineNo, 1);
    
    clang::SourceLocation End = MacroNameTok.getEndLoc();
    
    if (LineStart.isValid() && End.isValid()) {
        clang::SourceRange Range(LineStart, End);
        addRange(Range);
    }
}

void ASTNormalizerPreprocessor::If(clang::SourceLocation Loc, clang::SourceRange ConditionRange, clang::PPCallbacks::ConditionValueKind ConditionValue) {
    if (!isLocationInMainFile(SM, Loc)) return;
    
    if (!Loc.isValid() || !ConditionRange.isValid()) return;
    
    unsigned LineNo = SM->getExpansionLineNumber(Loc);
    clang::FileID FID = SM->getFileID(Loc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, LineNo, 1);
    
    clang::SourceLocation End = ConditionRange.getEnd();
    
    if (LineStart.isValid() && End.isValid()) {
        clang::SourceRange Range(LineStart, End);
        addRange(Range);
    }
}

void ASTNormalizerPreprocessor::Elif(clang::SourceLocation Loc, clang::SourceRange ConditionRange, clang::PPCallbacks::ConditionValueKind ConditionValue, clang::SourceLocation IfLoc) {
    if (!isLocationInMainFile(SM, Loc)) return;
    
    if (!Loc.isValid() || !ConditionRange.isValid()) return;
    
    unsigned LineNo = SM->getExpansionLineNumber(Loc);
    clang::FileID FID = SM->getFileID(Loc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, LineNo, 1);
    
    clang::SourceLocation End = ConditionRange.getEnd();
    
    if (LineStart.isValid() && End.isValid()) {
        clang::SourceRange Range(LineStart, End);
        addRange(Range);
    }
}

void ASTNormalizerPreprocessor::Ifdef(clang::SourceLocation Loc, const clang::Token &MacroNameTok, const clang::MacroDefinition &MD) {
    if (!isLocationInMainFile(SM, Loc)) return;
    
    if (!Loc.isValid() || !MacroNameTok.getLocation().isValid()) return;
    
    unsigned LineNo = SM->getExpansionLineNumber(Loc);
    clang::FileID FID = SM->getFileID(Loc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, LineNo, 1);
    
    clang::SourceLocation End = MacroNameTok.getEndLoc();
    
    if (LineStart.isValid() && End.isValid()) {
        clang::SourceRange Range(LineStart, End);
        addRange(Range);
    }
}

void ASTNormalizerPreprocessor::Elifdef(clang::SourceLocation Loc, clang::SourceRange ConditionRange, clang::SourceLocation IfLoc) {
    if (!isLocationInMainFile(SM, Loc)) return;
    
    if (!Loc.isValid() || !ConditionRange.isValid()) return;
    
    unsigned LineNo = SM->getExpansionLineNumber(Loc);
    clang::FileID FID = SM->getFileID(Loc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, LineNo, 1);
    
    clang::SourceLocation End = ConditionRange.getEnd();
    
    if (LineStart.isValid() && End.isValid()) {
        clang::SourceRange Range(LineStart, End);
        addRange(Range);
    }
}

void ASTNormalizerPreprocessor::Ifndef(clang::SourceLocation Loc, const clang::Token &MacroNameTok, const clang::MacroDefinition &MD) {
    if (!isLocationInMainFile(SM, Loc)) return;
    
    if (!Loc.isValid() || !MacroNameTok.getLocation().isValid()) return;

    unsigned LineNo = SM->getExpansionLineNumber(Loc);
    clang::FileID FID = SM->getFileID(Loc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, LineNo, 1);
    
    clang::SourceLocation End = MacroNameTok.getEndLoc();
    
    if (LineStart.isValid() && End.isValid()) {
        clang::SourceRange Range(LineStart, End);
        addRange(Range);
    }
}

void ASTNormalizerPreprocessor::Elifndef(clang::SourceLocation Loc, clang::SourceRange ConditionRange, clang::SourceLocation IfLoc) {
    if (!isLocationInMainFile(SM, Loc)) return;
    
    if (!Loc.isValid() || !ConditionRange.isValid()) return;
    
    unsigned LineNo = SM->getExpansionLineNumber(Loc);
    clang::FileID FID = SM->getFileID(Loc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, LineNo, 1);
    
    clang::SourceLocation End = ConditionRange.getEnd();
    
    if (LineStart.isValid() && End.isValid()) {
        clang::SourceRange Range(LineStart, End);
        addRange(Range);
    }
}

void ASTNormalizerPreprocessor::Else(clang::SourceLocation Loc, clang::SourceLocation IfLoc) {
    if (!isLocationInMainFile(SM, Loc)) return;
    
    if (!Loc.isValid()) return;

    unsigned LineNo = SM->getExpansionLineNumber(Loc);
    clang::FileID FID = SM->getFileID(Loc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, LineNo, 1);
    
    clang::Token Token;
    if (clang::Lexer::getRawToken(Loc, Token, *SM, clang::LangOptions())) {
        clang::SourceRange Range(LineStart, Loc.getLocWithOffset(3));
        addRange(Range);
        return;
    }

    clang::SourceLocation TokenEnd = Token.getEndLoc();
    if (LineStart.isValid() && TokenEnd.isValid()) {
        clang::SourceRange Range(LineStart, TokenEnd);
        addRange(Range);
    }
}

void ASTNormalizerPreprocessor::Endif(clang::SourceLocation Loc, clang::SourceLocation IfLoc) {
    if (!isLocationInMainFile(SM, Loc)) return;
    
    if (!Loc.isValid()) return;

    unsigned LineNo = SM->getExpansionLineNumber(Loc);
    clang::FileID FID = SM->getFileID(Loc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, LineNo, 1);
    
    clang::Token Token;
    if (clang::Lexer::getRawToken(Loc, Token, *SM, clang::LangOptions())) {
        clang::SourceRange Range(LineStart, Loc.getLocWithOffset(5));
        addRange(Range);
        return;
    }

    clang::SourceLocation TokenEnd = Token.getEndLoc();
    if (LineStart.isValid() && TokenEnd.isValid()) {
        clang::SourceRange Range(LineStart, TokenEnd);
        addRange(Range);
    }
}

void ASTNormalizerPreprocessor::SourceRangeSkipped(clang::SourceRange Range, clang::SourceLocation EndifLoc) {
    if (!isLocationInMainFile(SM, Range.getBegin())) return;
    
    clang::SourceLocation StartLoc = Range.getBegin();
    
    if (!StartLoc.isValid()) return;
    
    unsigned StartLineNo = SM->getExpansionLineNumber(StartLoc);
    clang::FileID FID = SM->getFileID(StartLoc);
    clang::SourceLocation LineStart = SM->translateLineCol(FID, StartLineNo, 1);
    
    if (LineStart.isValid() && Range.getEnd().isValid()) {
        clang::SourceRange FullRange(LineStart, Range.getEnd());
        addRange(FullRange, false);
    }

}

}