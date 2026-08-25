// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "qualified_name_generator.hpp"
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
class QualifiedNameGenVisitor : public RecursiveASTVisitor<QualifiedNameGenVisitor> {
private:
  ASTContext *Context;
  llvm::raw_fd_ostream os_output;
  std::error_code EC;
  bool ShowDiff;

public:
  QualifiedNameGenVisitor(ASTContext *Context, StringRef OutputFile, bool ShowDiff)
      : Context(Context), os_output(OutputFile, EC, llvm::sys::fs::OF_None), ShowDiff(ShowDiff) {}

  bool VisitNamedDecl(clang::NamedDecl *Decl) {
    if (!IsFromMainFile(Decl) || isa<TemplateTypeParmDecl>(Decl) || isa<NonTypeTemplateParmDecl>(Decl) ||
        isa<TemplateTemplateParmDecl>(Decl) || isa<ParmVarDecl>(Decl))
      return true;

    SourceLocation Loc = Decl->getLocation();
    unsigned Line = -1;
    unsigned Column = -1;

    SourceManager &SM = Context->getSourceManager();
    Line = SM.getSpellingLineNumber(Loc);
    Column = SM.getSpellingColumnNumber(Loc);

    const std::string clangQualifiedName = Decl->getQualifiedNameAsString();

    SmallString<256> armorQualifiedName;
    bool ArmorSuccess = armor::generateQualifiedNameForDecl(Decl, armorQualifiedName);

    if (ShowDiff && armorQualifiedName.equals(clangQualifiedName)) return true;

    SmallString<256> clangUSR;
    index::generateUSRForDecl(Decl, clangUSR);

    os_output << "Declaration: " << Decl->getNameAsString() << "\n";
    os_output << "Location: Line " << Line << ", Column " << Column << "\n";
    os_output << "Kind: " << Decl->getDeclKindName() << "\n";
    os_output << "Armor QualifiedName: " << (!ArmorSuccess ? armorQualifiedName.str() : llvm::StringRef("ERROR")) << "\n";
    os_output << "Clang QUalifiedName: " << clangQualifiedName << "\n";
    os_output << "Clang USR : " << clangUSR << "\n";
    os_output << "-------------------------------------------\n\n";

    return true;
  }

  inline bool IsFromMainFile(const clang::Decl *Decl) {
    clang::ASTContext *clangContext = &Decl->getASTContext();
    return clangContext->getSourceManager().isInMainFile(Decl->getLocation());
  }

  ~QualifiedNameGenVisitor() { os_output.close(); }
};

class QualifiedNameGenConsumer : public ASTConsumer {
private:
  QualifiedNameGenVisitor Visitor;
public:
  explicit QualifiedNameGenConsumer(ASTContext *Context, const std::string &OutputFile, bool ShowDiff)
      : Visitor(Context, OutputFile, ShowDiff) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class QualifiedNameGenAction : public ASTFrontendAction {
private:
  std::string OutputFile;
  bool ShowDiff;
public:
  QualifiedNameGenAction(const std::string &OutputFile, bool ShowDiff) : OutputFile(OutputFile), ShowDiff(ShowDiff) {}

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    return std::make_unique<QualifiedNameGenConsumer>(&CI.getASTContext(), OutputFile, ShowDiff);
  }
};

class QualifiedNameGenActionFactory : public FrontendActionFactory {
private:
  std::string OutputFile;
  bool ShowDiff;
public:
  QualifiedNameGenActionFactory(const std::string &OutputFile, bool ShowDiff) : OutputFile(OutputFile), ShowDiff(ShowDiff) {}

  std::unique_ptr<FrontendAction> create() override {
    return std::make_unique<QualifiedNameGenAction>(OutputFile, ShowDiff);
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
    llvm::errs() << "  output-file: File to write qualified-name information to\n";
    llvm::errs() << "  show-diff: Optional. If 'true', only show declarations with different qualified names\n";
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

  auto Factory = std::make_unique<QualifiedNameGenActionFactory>(outputFile, showDiff);

  return Tool.run(Factory.get());
}
