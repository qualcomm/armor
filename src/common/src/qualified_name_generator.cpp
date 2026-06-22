// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "qualified_name_generator.hpp"
#include "type_utils.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/AST/Type.h"
#include "clang/Basic/FileEntry.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/PreprocessingRecord.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Index/USRGeneration.h"
#include <llvm-14/llvm/Support/Casting.h>

using namespace clang;

namespace armor {

static StringRef GetExternalSourceContainer(const NamedDecl *D) {
  if (!D)
    return StringRef();
  if (auto *attr = D->getExternalSourceSymbolAttr()) {
    return attr->getDefinedIn();
  }
  return StringRef();
}

namespace {
class USRGenerator : public ConstDeclVisitor<USRGenerator> {
  SmallVectorImpl<char> &Buf;
  llvm::raw_svector_ostream Out;
  bool IgnoreResults;
  ASTContext *Context;
  bool generatedLoc;

  llvm::DenseMap<const Type *, unsigned> TypeSubstitutions;

public:
  explicit USRGenerator(ASTContext *Ctx, SmallVectorImpl<char> &Buf)
  : Buf(Buf),
    Out(Buf),
    IgnoreResults(false),
    Context(Ctx),
    generatedLoc(false){}

  bool ignoreResults() const { return IgnoreResults; }

  // Visitation methods from generating USRs from AST elements.
  void VisitDeclContext(const DeclContext *D);
  void VisitFieldDecl(const FieldDecl *D);
  void VisitFunctionDecl(const FunctionDecl *D);
  void VisitNamedDecl(const NamedDecl *D);
  void VisitNamespaceDecl(const NamespaceDecl *D);
  void VisitNamespaceAliasDecl(const NamespaceAliasDecl *D);
  void VisitFunctionTemplateDecl(const FunctionTemplateDecl *D);
  void VisitClassTemplateDecl(const ClassTemplateDecl *D);
  void VisitTagDecl(const TagDecl *D);
  void VisitTypedefDecl(const TypedefDecl *D);
  void VisitVarDecl(const VarDecl *D);
  void VisitBindingDecl(const BindingDecl *D);
  void VisitUnresolvedUsingValueDecl(const UnresolvedUsingValueDecl *D);
  void VisitUnresolvedUsingTypenameDecl(const UnresolvedUsingTypenameDecl *D);

  void VisitLinkageSpecDecl(const LinkageSpecDecl *D) {
    IgnoreResults = true; // No USRs for linkage specs themselves.
  }

  void VisitUsingDirectiveDecl(const UsingDirectiveDecl *D) {
    IgnoreResults = true;
  }

  void VisitUsingDecl(const UsingDecl *D) {
    VisitDeclContext(D->getDeclContext());

    bool EmittedDeclName = !EmitDeclName(D);
    assert(EmittedDeclName && "EmitDeclName can not fail for UsingDecls");
    (void)EmittedDeclName;
  }

  void GenExtSymbolContainer(const NamedDecl *D);

  /// String generation methods used both by the visitation methods
  /// and from other clients that want to directly generate USRs.  These
  /// methods do not construct complete USRs (which incorporate the parents
  /// of an AST element), but only the fragments concerning the AST element
  /// itself.
  void VisitTemplateName(TemplateName Name);
  void VisitTemplateArgument(const TemplateArgument &Arg);

  /// Emit a Decl's name using NamedDecl::printName() and return true if
  ///  the decl had no name.
  bool EmitDeclName(const NamedDecl *D);
};
} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Generating USRs from ASTS.
//===----------------------------------------------------------------------===//

bool USRGenerator::EmitDeclName(const NamedDecl *D) {
  const unsigned startSize = Buf.size();
  D->printName(Out);
  const unsigned endSize = Buf.size();
  bool isEmpty = (startSize == endSize);
  return isEmpty;
}

void USRGenerator::VisitDeclContext(const DeclContext *DC) {
  if (const NamedDecl *D = dyn_cast<NamedDecl>(DC)){
    Visit(D);
  }
  else if (isa<LinkageSpecDecl>(DC)){ // Linkage specs are transparent in qualified Name.
    VisitDeclContext(DC->getParent());
  }
}

void USRGenerator::VisitFieldDecl(const FieldDecl *D) {
  // The qualified Name for an ivar declared in a class extension is based on the
  const unsigned startSize = Buf.size();
  VisitDeclContext(D->getDeclContext());
  const unsigned endSize = Buf.size();
  
  endSize == startSize ? Out : Out << "::";
  
  if (EmitDeclName(D)) {
    // Bit fields can be anonymous.
    IgnoreResults = true;
    return;
  }
}

void USRGenerator::VisitFunctionDecl(const FunctionDecl *D) {
  const unsigned StartSize = Buf.size();
  VisitDeclContext(D->getDeclContext());
  if (Buf.size() == StartSize)
    GenExtSymbolContainer(D);

  Buf.size() == StartSize ? Out : Out << "::";

  PrintingPolicy Policy(Context->getLangOpts());
  // Forward references can have different template argument names. Suppress the
  // template argument names in constructors to make their qualified Name more stable.
  Policy.SuppressTemplateArgsInCXXConstructors = true;
  D->getDeclName().print(Out, Policy);

  ASTContext &Ctx = *Context;
  if ((!Ctx.getLangOpts().CPlusPlus || D->isExternC()) &&
      !D->hasAttr<OverloadableAttr>())
    return;

  if (const TemplateArgumentList *
        SpecArgs = D->getTemplateSpecializationArgs()) {
    Out << '<';
    for (unsigned I = 0, N = SpecArgs->size(); I != N; ++I) {
      VisitTemplateArgument(SpecArgs->get(I));
      I != N - 1 ? Out << ", " : Out;
    }
    Out << '>';
  }
   
}

void USRGenerator::VisitNamedDecl(const NamedDecl *D) {
  const unsigned startSize = Buf.size();
  VisitDeclContext(D->getDeclContext());
  const unsigned endSize = Buf.size();
  
  startSize == endSize ? Out : Out << "::";
  
  if (EmitDeclName(D)) {
    // The string can be empty if the declaration has no name; e.g., it is
    // the ParmDecl with no name for declaration of a function pointer type,
    // e.g.: void  (*f)(void *);
    // In this case, don't generate a qualified Name.
    IgnoreResults = true;
  }
}

void USRGenerator::VisitVarDecl(const VarDecl *D) {
  // VarDecls can be declared 'extern' within a function or method body,
  // but their enclosing DeclContext is the function, not the TU.  We need
  // to check the storage class to correctly generate the qualified Name.
  const unsigned startSize = Buf.size();
  VisitDeclContext(D->getDeclContext());
  const unsigned endSize = Buf.size();

  startSize == endSize ? Out : Out << "::";
  // Variables always have simple names.
  StringRef s = D->getName();

  // The string can be empty if the declaration has no name; e.g., it is
  // the ParmDecl with no name for declaration of a function pointer type, e.g.:
  //    void  (*f)(void *);
  // In this case, don't generate a qualified Name.
  if (s.empty())
    IgnoreResults = true;
  else{
    Out << s;
  }

  // For a template specialization, get complete template arguments.
  if (const VarTemplateSpecializationDecl *Spec
                              = dyn_cast<VarTemplateSpecializationDecl>(D)) {
    const TemplateArgumentList &Args = Spec->getTemplateArgs();
    Out << '<';
    for (unsigned I = 0, N = Args.size(); I != N; ++I) {
      VisitTemplateArgument(Args.get(I));
      I != N - 1 ? Out << ", " : Out;
    }
    Out << '>';
  }
}

void USRGenerator::VisitBindingDecl(const BindingDecl *D) {
  VisitNamedDecl(D);
}

void USRGenerator::VisitNamespaceDecl(const NamespaceDecl *D) {
  const unsigned startSize = Buf.size();
  VisitDeclContext(D->getDeclContext());
  const unsigned endSize = Buf.size();

  startSize == endSize ? Out : Out << "::";

  if (D->isAnonymousNamespace()) {
    Out << "(anonymous namespace)";
    return;
  }

  if (!IgnoreResults){
    Out << D->getName();
  }
}

void USRGenerator::VisitFunctionTemplateDecl(const FunctionTemplateDecl *D) {
  VisitFunctionDecl(D->getTemplatedDecl());
}

void USRGenerator::VisitClassTemplateDecl(const ClassTemplateDecl *D) {
  VisitTagDecl(D->getTemplatedDecl());
}

void USRGenerator::VisitNamespaceAliasDecl(const NamespaceAliasDecl *D) {
  const unsigned startSize = Buf.size();
  VisitDeclContext(D->getDeclContext());
  const unsigned endSize = Buf.size();
  if (!IgnoreResults){
    startSize == endSize ? Out : Out << "::";
    Out << D->getName();
  }
}

void USRGenerator::VisitTagDecl(const TagDecl *D) {  
  // Add the location of the tag decl to handle resolution across
  // translation units.
  GenExtSymbolContainer(D);

  const unsigned startSize = Buf.size();
  VisitDeclContext(D->getDeclContext());
  const unsigned endSize = Buf.size();

  startSize == endSize ? Out : Out << "::";
  
  if (EmitDeclName(D)) {
    if (const TypedefNameDecl *TD = D->getTypedefNameForAnonDecl()) {
      Out << *TD;
    }
    else {
      // Completely anonymous tag decl
        if (D->isEmbeddedInDeclarator() && !D->isFreeStanding()) {
          // Definig the anonymous struct within a field
          // struct { int x; } field; union { int x. } field; enum { x } field;
          if(const ValueDecl * VD = dyn_cast_or_null<clang::ValueDecl>(D->getNextDeclInContext())){
            const QualType QT = unwrapTypeModifiers(VD->getType());
            if(QT->getAsTagDecl() && QT->getAsTagDecl()->Equals(D)){
              VD->printName(Out);
            }
          }
          else if(const TypedefDecl * TD = dyn_cast_or_null<clang::TypedefDecl>(D->getNextDeclInContext())){
            const QualType QT = unwrapTypeModifiers(TD->getUnderlyingType());
            if(QT->getAsTagDecl() && QT->getAsTagDecl()->Equals(D)){
              TD->printName(Out);
            }
          }
        } 
        else {
          // Completely anonymous tag decl.
          switch (D->getTagKind()) {
            case TTK_Interface:
            case TTK_Class:  Out << "(Anon::Class)"; break;
            case TTK_Struct: Out << "(Anon::Struct)"; break;
            case TTK_Union:  Out << "(Anon::Union)"; break;
            case TTK_Enum:   Out << "(Anon::Enum)"; break;
          }
      }
    }
  }

  // For a class template specialization, mangle the template arguments.
  if (const ClassTemplateSpecializationDecl *Spec
                              = dyn_cast<ClassTemplateSpecializationDecl>(D)) {
    const TemplateArgumentList &Args = Spec->getTemplateArgs();
    Out << '<';
    for (unsigned I = 0, N = Args.size(); I != N; ++I) {
      VisitTemplateArgument(Args.get(I));
      I != N - 1 ? Out << ", " : Out;
    }
    Out << '>';
  }
}

void USRGenerator::VisitTypedefDecl(const TypedefDecl *D) {
  const unsigned startSize = Buf.size();
  const DeclContext *DC = D->getDeclContext();
  if (const NamedDecl *DCN = dyn_cast<NamedDecl>(DC)) Visit(DCN);
  const unsigned endSize = Buf.size();
  startSize == endSize ? Out : Out << "::";
  Out << D->getName();
}

void USRGenerator::GenExtSymbolContainer(const NamedDecl *D) {
  StringRef Container = GetExternalSourceContainer(D);
  if (!Container.empty())
    Out << Container;
}

void USRGenerator::VisitTemplateName(TemplateName Name) {
  if (TemplateDecl *Template = Name.getAsTemplateDecl()) {
    if (TemplateTemplateParmDecl *TTP
                              = dyn_cast<TemplateTemplateParmDecl>(Template)) {
      TTP->printName(Out);
      return;
    }

    Visit(Template);
    return;
  }

  // FIXME: Visit dependent template names.
}

void USRGenerator::VisitTemplateArgument(const TemplateArgument &Arg) {
  PrintingPolicy Policy(Context->getLangOpts());
  switch (Arg.getKind()) {
  case TemplateArgument::Null:
    break;

  case TemplateArgument::Declaration:
    Visit(Arg.getAsDecl());
    break;

  case TemplateArgument::NullPtr:
    break;

  case TemplateArgument::TemplateExpansion:
    LLVM_FALLTHROUGH;
  case TemplateArgument::Template:
    VisitTemplateName(Arg.getAsTemplateOrTemplatePattern());
    break;

  case TemplateArgument::Expression:
    // FIXME: Visit expressions.
    break;

  case TemplateArgument::Pack:
    for (const auto &P : Arg.pack_elements())
      VisitTemplateArgument(P);
    break;

  case TemplateArgument::Type:
    Arg.print(Policy, Out, true);
    break;

  case TemplateArgument::Integral:
    Arg.getAsIntegral().print(Out, true);
    break;
  }
}

void USRGenerator::VisitUnresolvedUsingValueDecl(const UnresolvedUsingValueDecl *D) {
  const unsigned startSize = Buf.size();
  VisitDeclContext(D->getDeclContext());
  const unsigned endSize = Buf.size();
  startSize == endSize ? Out : Out << "::";
  EmitDeclName(D);
}

void USRGenerator::VisitUnresolvedUsingTypenameDecl(const UnresolvedUsingTypenameDecl *D) {
  const unsigned startSize = Buf.size();
  VisitDeclContext(D->getDeclContext());
  const unsigned endSize = Buf.size();
  startSize == endSize ? Out : Out << "::";
  Out << D->getName(); // Simple name.
}

//===----------------------------------------------------------------------===//
// QualifiedName generation functions.
//===----------------------------------------------------------------------===//

bool generateQualifiedNameForDecl(const Decl *D,
                                      SmallVectorImpl<char> &Buf) {
  
  if (!D) return true;
  
  USRGenerator UG(&D->getASTContext(), Buf);
  UG.Visit(D);
  bool ignoreResult = UG.ignoreResults();
  
  return ignoreResult;
}

} // namespace armor