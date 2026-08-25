// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "nsr_generator.hpp"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Index/USRGeneration.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/ADT/SmallString.h"
#include <iostream>
#include <string>
#include <sstream>
#include <system_error>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

#ifndef CLANG_FLAGS
#define CLANG_FLAGS ""
#endif

std::vector<std::string> getClangFlags() {
    std::vector<std::string> flags;
    std::istringstream iss(CLANG_FLAGS);
    std::string flag;
    while (iss >> flag) {
        flags.emplace_back(std::move(flag));
    }
    return flags;
}

namespace {
class NSRGenVisitor : public RecursiveASTVisitor<NSRGenVisitor> {
private:
  ASTContext *Context;
  llvm::raw_fd_ostream os_output;
  std::error_code EC;
  bool ShowDiff;

public:
  NSRGenVisitor(ASTContext *Context, StringRef OutputFile, bool ShowDiff)
      : Context(Context), os_output(OutputFile, EC, llvm::sys::fs::OF_None), ShowDiff(ShowDiff) {}

  void ProcessDecl(const clang::Decl *Decl, StringRef DeclName) {
    SourceLocation Loc = Decl->getLocation();
    unsigned Line = -1;
    unsigned Column = -1;

    SourceManager &SM = Context->getSourceManager();
    Line = SM.getSpellingLineNumber(Loc);
    Column = SM.getSpellingColumnNumber(Loc);

    SmallString<256> ClangUSR;
    bool ClangSuccess = index::generateUSRForDecl(Decl, ClangUSR);

    SmallString<256> ArmorNSR;
    bool ArmorSuccess = armor::generateNSRForDecl(Decl, ArmorNSR);

    if (ShowDiff && ArmorNSR.equals(ClangUSR)) return;

    os_output << "Declaration: " << DeclName << "\n";
    os_output << "Location: Line " << Line << ", Column " << Column << "\n";
    os_output << "Kind: " << Decl->getDeclKindName() << "\n";

    os_output << "Clang USR: " << (!ClangSuccess ? ClangUSR.str() : llvm::StringRef("ERROR")) << "\n";
    os_output << "Armor NSR: " << (!ArmorSuccess ? ArmorNSR.str() : llvm::StringRef("ERROR")) << "\n";
    os_output << "-------------------------------------------\n\n";
  }

  bool VisitNamedDecl(clang::NamedDecl *Decl) {
    if (!IsFromMainFile(Decl) || isa<ParmVarDecl>(Decl) || isa<TemplateTypeParmDecl>(Decl) ||
        isa<NonTypeTemplateParmDecl>(Decl) || isa<TemplateTemplateParmDecl>(Decl))
      return true;

    ProcessDecl(Decl, Decl->getNameAsString());
    return true;
  }

  bool VisitFriendDecl(clang::FriendDecl *Decl) {
    if (!IsFromMainFile(Decl))
      return true;

    ProcessDecl(Decl, "Friend");
    return true;
  }

  inline bool IsFromMainFile(const clang::Decl *Decl) {
    clang::ASTContext *clangContext = &Decl->getASTContext();
    return clangContext->getSourceManager().isInMainFile(Decl->getLocation());
  }

  ~NSRGenVisitor() { os_output.close(); }
};

class NSRGenConsumer : public ASTConsumer {
private:
  NSRGenVisitor Visitor;
public:
  explicit NSRGenConsumer(ASTContext *Context, const std::string &OutputFile, bool ShowDiff)
      : Visitor(Context, OutputFile, ShowDiff) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class NSRGenAction : public ASTFrontendAction {
private:
  std::string OutputFile;
  bool ShowDiff;
public:
  NSRGenAction(const std::string &OutputFile, bool ShowDiff) : OutputFile(OutputFile), ShowDiff(ShowDiff) {}

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    return std::make_unique<NSRGenConsumer>(&CI.getASTContext(), OutputFile, ShowDiff);
  }
};

class NSRGenActionFactory : public FrontendActionFactory {
private:
  std::string OutputFile;
  bool ShowDiff;
public:
  NSRGenActionFactory(const std::string &OutputFile, bool ShowDiff) : OutputFile(OutputFile), ShowDiff(ShowDiff) {}

  std::unique_ptr<FrontendAction> create() override {
    return std::make_unique<NSRGenAction>(OutputFile, ShowDiff);
  }
};

}

int main(int argc, const char **argv) {
  std::vector<std::string> CompileCommands = getClangFlags();

  std::string CurrentDir = ".";
  std::unique_ptr<CompilationDatabase> Compilations =
      std::make_unique<FixedCompilationDatabase>(CurrentDir, CompileCommands);

  if (argc < 3) {
    llvm::errs() << "Usage: " << argv[0] << " <input-file> <output-file> [show-diff]\n";
    llvm::errs() << "  input-file: C++ source or header file to process\n";
    llvm::errs() << "  output-file: File to write NSR information to\n";
    llvm::errs() << "  show-diff: Optional. If 'true', only show declarations with different USR/NSR\n";
    return 1;
  }

  std::string fileToProcess = argv[1];
  std::string outputFile = argv[2];

  bool showDiff = false;
  if (argc >= 4) {
    std::string showDiffArg = argv[3];
    showDiff = (showDiffArg == "true" || showDiffArg == "1" ||
                showDiffArg == "yes" || showDiffArg == "y");
  }

  ClangTool Tool(*Compilations, fileToProcess);

  auto Factory = std::make_unique<NSRGenActionFactory>(outputFile, showDiff);

  return Tool.run(Factory.get());
}
