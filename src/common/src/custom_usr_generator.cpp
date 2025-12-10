// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include "custom_usr_generator.hpp"
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

// Helper function to unwrap type modifiers (pointers, references, arrays, etc.)
// but preserve typedefs and other sugar types

static bool printLoc(llvm::raw_ostream &OS, SourceLocation Loc,
                     const SourceManager &SM, bool IncludeOffset) {
  
  if (Loc.isInvalid()) {
    return true;
  }
  
  Loc = SM.getExpansionLoc(Loc);
  const std::pair<FileID, unsigned> &Decomposed = SM.getDecomposedLoc(Loc);
  const FileEntry *FE = SM.getFileEntryForID(Decomposed.first);
  
  if (FE) {
    std::string Filename = llvm::sys::path::filename(FE->getName()).str();
    OS << Filename;
  } 
  else {
    // This case really isn't interesting.
    return true;
  }
  
  if (IncludeOffset) {
    // Use the offest into the FileID to represent the location.  Using
    // a line/column can cause us to look back at the original source file,
    // which is expensive.
    OS << '@' << Decomposed.second;
  }
  
  return false;
}

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
    generatedLoc(false)
  {
    // Add the USR space prefix.
    Out << getUSRSpacePrefix();
  }

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
  void VisitTemplateTypeParmDecl(const TemplateTypeParmDecl *D);
  void VisitVarDecl(const VarDecl *D);
  void VisitBindingDecl(const BindingDecl *D);
  void VisitNonTypeTemplateParmDecl(const NonTypeTemplateParmDecl *D);
  void VisitTemplateTemplateParmDecl(const TemplateTemplateParmDecl *D);
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
    Out << "@UD@";

    bool EmittedDeclName = !EmitDeclName(D);
    assert(EmittedDeclName && "EmitDeclName can not fail for UsingDecls");
    (void)EmittedDeclName;
  }

  bool ShouldGenerateLocation(const NamedDecl *D);

  bool isLocal(const NamedDecl *D) {
    return D->getParentFunctionOrMethod() != nullptr;
  }

  void GenExtSymbolContainer(const NamedDecl *D);

  /// Generate the string component containing the location of the
  ///  declaration.
  bool GenLoc(const Decl *D, bool IncludeOffset);

  /// String generation methods used both by the visitation methods
  /// and from other clients that want to directly generate USRs.  These
  /// methods do not construct complete USRs (which incorporate the parents
  /// of an AST element), but only the fragments concerning the AST element
  /// itself.
  void VisitType(QualType T);
  void VisitTemplateParameterList(const TemplateParameterList *Params);
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
  return startSize == endSize;
}

bool USRGenerator::ShouldGenerateLocation(const NamedDecl *D) {
  // Always include location for parameters and template parameters
  if (isa<ParmVarDecl>(D)||
    isa<TemplateTypeParmDecl>(D) || 
      isa<NonTypeTemplateParmDecl>(D) || 
      isa<TemplateTemplateParmDecl>(D)) {
    return true;
  }
  
  // Always include location for local variables and declarations inside functions
  if (D->getParentFunctionOrMethod() != nullptr) {
    return true;
  }
  
  // For all other declarations in headers, exclude location information
  return false;
  
}

void USRGenerator::VisitDeclContext(const DeclContext *DC) {
  if (const NamedDecl *D = dyn_cast<NamedDecl>(DC))
    Visit(D);
  else if (isa<LinkageSpecDecl>(DC)) // Linkage specs are transparent in USRs.
    VisitDeclContext(DC->getParent());
}

void USRGenerator::VisitFieldDecl(const FieldDecl *D) {
  // The USR for an ivar declared in a class extension is based on the
  VisitDeclContext(D->getDeclContext());
  Out << "@FI@";
  if (EmitDeclName(D)) {
    // Bit fields can be anonymous.
    IgnoreResults = true;
    return;
  }
}

void USRGenerator::VisitFunctionDecl(const FunctionDecl *D) {
  if (ShouldGenerateLocation(D) && GenLoc(D, /*IncludeOffset=*/isLocal(D)))
    return;

  const unsigned StartSize = Buf.size();
  VisitDeclContext(D->getDeclContext());
  if (Buf.size() == StartSize)
    GenExtSymbolContainer(D);

  bool IsTemplate = false;
  if (FunctionTemplateDecl *FunTmpl = D->getDescribedFunctionTemplate()) {
    IsTemplate = true;
    Out << "@FT@";
    VisitTemplateParameterList(FunTmpl->getTemplateParameters());
  } else
    Out << "@F@";

  PrintingPolicy Policy(Context->getLangOpts());
  // Forward references can have different template argument names. Suppress the
  // template argument names in constructors to make their USR more stable.
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
      Out << '#';
      VisitTemplateArgument(SpecArgs->get(I));
    }
    Out << '>';
  }

  // Mangle in type information for the arguments.
  for (auto PD : D->parameters()) {
    Out << '#';
    VisitType(PD->getType());
  }
  if (D->isVariadic())
    Out << '.';
  if (IsTemplate) {
    // Function templates can be overloaded by return type, for example:
    // \code
    //   template <class T> typename T::A foo() {}
    //   template <class T> typename T::B foo() {}
    // \endcode
    Out << '#';
    VisitType(D->getReturnType());
  }
  Out << '#';
  if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(D)) {
    if (MD->isStatic())
      Out << 'S';
    // FIXME: OpenCL: Need to consider address spaces
    if (unsigned quals = MD->getMethodQualifiers().getCVRUQualifiers())
      Out << (char)('0' + quals);
    switch (MD->getRefQualifier()) {
    case RQ_None: break;
    case RQ_LValue: Out << '&'; break;
    case RQ_RValue: Out << "&&"; break;
    }
  }
}

void USRGenerator::VisitNamedDecl(const NamedDecl *D) {
  VisitDeclContext(D->getDeclContext());
  Out << "@";

  if (EmitDeclName(D)) {
    // The string can be empty if the declaration has no name; e.g., it is
    // the ParmDecl with no name for declaration of a function pointer type,
    // e.g.: void  (*f)(void *);
    // In this case, don't generate a USR.
    IgnoreResults = true;
  }
}

void USRGenerator::VisitVarDecl(const VarDecl *D) {
  // VarDecls can be declared 'extern' within a function or method body,
  // but their enclosing DeclContext is the function, not the TU.  We need
  // to check the storage class to correctly generate the USR.
  if (ShouldGenerateLocation(D) && GenLoc(D, /*IncludeOffset=*/isLocal(D)))
    return;

  VisitDeclContext(D->getDeclContext());

  if (VarTemplateDecl *VarTmpl = D->getDescribedVarTemplate()) {
    Out << "@VT";
    VisitTemplateParameterList(VarTmpl->getTemplateParameters());
  } else if (const VarTemplatePartialSpecializationDecl *PartialSpec
             = dyn_cast<VarTemplatePartialSpecializationDecl>(D)) {
    Out << "@VP";
    VisitTemplateParameterList(PartialSpec->getTemplateParameters());
  }

  // Variables always have simple names.
  StringRef s = D->getName();

  // The string can be empty if the declaration has no name; e.g., it is
  // the ParmDecl with no name for declaration of a function pointer type, e.g.:
  //    void  (*f)(void *);
  // In this case, don't generate a USR.
  if (s.empty())
    IgnoreResults = true;
  else
    Out << '@' << s;

  // For a template specialization, mangle the template arguments.
  if (const VarTemplateSpecializationDecl *Spec
                              = dyn_cast<VarTemplateSpecializationDecl>(D)) {
    const TemplateArgumentList &Args = Spec->getTemplateArgs();
    Out << '>';
    for (unsigned I = 0, N = Args.size(); I != N; ++I) {
      Out << '#';
      VisitTemplateArgument(Args.get(I));
    }
  }
}

void USRGenerator::VisitBindingDecl(const BindingDecl *D) {
  if (isLocal(D) && GenLoc(D, /*IncludeOffset=*/true))
    return;
  VisitNamedDecl(D);
}

void USRGenerator::VisitNonTypeTemplateParmDecl(
                                        const NonTypeTemplateParmDecl *D) {
  GenLoc(D, /*IncludeOffset=*/true);
}

void USRGenerator::VisitTemplateTemplateParmDecl(
                                        const TemplateTemplateParmDecl *D) {
  GenLoc(D, /*IncludeOffset=*/true);
}

void USRGenerator::VisitNamespaceDecl(const NamespaceDecl *D) {
  if (D->isAnonymousNamespace()) {
    Out << "@aN";
    return;
  }

  VisitDeclContext(D->getDeclContext());
  if (!IgnoreResults)
    Out << "@N@" << D->getName();
}

void USRGenerator::VisitFunctionTemplateDecl(const FunctionTemplateDecl *D) {
  VisitFunctionDecl(D->getTemplatedDecl());
}

void USRGenerator::VisitClassTemplateDecl(const ClassTemplateDecl *D) {
  VisitTagDecl(D->getTemplatedDecl());
}

void USRGenerator::VisitNamespaceAliasDecl(const NamespaceAliasDecl *D) {
  VisitDeclContext(D->getDeclContext());
  if (!IgnoreResults)
    Out << "@NA@" << D->getName();
}

void USRGenerator::VisitTagDecl(const TagDecl *D) {
  // Add the location of the tag decl to handle resolution across
  // translation units.
  if (!isa<EnumDecl>(D) &&
      ShouldGenerateLocation(D) && GenLoc(D, /*IncludeOffset=*/isLocal(D)))
    return;

  GenExtSymbolContainer(D);

  D = D->getCanonicalDecl();
  VisitDeclContext(D->getDeclContext());

  bool AlreadyStarted = false;
  if (const CXXRecordDecl *CXXRecord = dyn_cast<CXXRecordDecl>(D)) {
    if (ClassTemplateDecl *ClassTmpl = CXXRecord->getDescribedClassTemplate()) {
      AlreadyStarted = true;

      switch (D->getTagKind()) {
      case TTK_Interface:
      case TTK_Class:
      case TTK_Struct: Out << "@ST"; break;
      case TTK_Union:  Out << "@UT"; break;
      case TTK_Enum: llvm_unreachable("enum template");
      }
      VisitTemplateParameterList(ClassTmpl->getTemplateParameters());
    } else if (const ClassTemplatePartialSpecializationDecl *PartialSpec
                = dyn_cast<ClassTemplatePartialSpecializationDecl>(CXXRecord)) {
      AlreadyStarted = true;

      switch (D->getTagKind()) {
      case TTK_Interface:
      case TTK_Class:
      case TTK_Struct: Out << "@SP"; break;
      case TTK_Union:  Out << "@UP"; break;
      case TTK_Enum: llvm_unreachable("enum partial specialization");
      }
      VisitTemplateParameterList(PartialSpec->getTemplateParameters());
    }
  }

  if (!AlreadyStarted) {
    switch (D->getTagKind()) {
      case TTK_Interface:
      case TTK_Class:
      case TTK_Struct: Out << "@S"; break;
      case TTK_Union:  Out << "@U"; break;
      case TTK_Enum:   Out << "@E"; break;
    }
  }

  Out << '@';
  assert(Buf.size() > 0);
  const unsigned off = Buf.size() - 1;

  if (EmitDeclName(D)) {
    if (const TypedefNameDecl *TD = D->getTypedefNameForAnonDecl()) {
      Buf[off] = 'A';
      Out << '@' << *TD;
    }
    else {
      // Completely anonymous tag decl
        if (D->isEmbeddedInDeclarator() && !D->isFreeStanding()) {
          // Definig the anonymous struct within a field
          // struct { int x; } <type_modifiers> field; union { int x. } <type_modifiers> field; enum { x } <type_modifiers> field;
          if(const ValueDecl * VD = dyn_cast_or_null<clang::ValueDecl>(D->getNextDeclInContext())){
            const QualType QT = unwrapTypeModifiers(VD->getType());
            if(QT->getAsTagDecl()->Equals(D)){
              Out << "#vf#";
              VD->printName(Out);
            }
            else{
              // Not an intresting case
              printLoc(Out, D->getLocation(), Context->getSourceManager(), true);
            }
          }
          else if(const TypedefDecl * TD = dyn_cast_or_null<clang::TypedefDecl>(D->getNextDeclInContext())){
            const QualType QT = unwrapTypeModifiers(TD->getUnderlyingType());
            if(QT->getAsTagDecl()->Equals(D)){
              Buf[off] = 'A';
              Out << '@';
              TD->printName(Out);
            }
          }
          else{
            printLoc(Out, D->getLocation(), Context->getSourceManager(), true);
          }
        } 
        else {
          // Completely anonymous tag decl.
          Buf[off] = 'a';
      }
    }
  }

  // For a class template specialization, mangle the template arguments.
  if (const ClassTemplateSpecializationDecl *Spec
                              = dyn_cast<ClassTemplateSpecializationDecl>(D)) {
    const TemplateArgumentList &Args = Spec->getTemplateArgs();
    Out << '>';
    for (unsigned I = 0, N = Args.size(); I != N; ++I) {
      Out << '#';
      VisitTemplateArgument(Args.get(I));
    }
  }
}

void USRGenerator::VisitTypedefDecl(const TypedefDecl *D) {
  if (ShouldGenerateLocation(D) && GenLoc(D, /*IncludeOffset=*/isLocal(D)))
    return;
  const DeclContext *DC = D->getDeclContext();
  if (const NamedDecl *DCN = dyn_cast<NamedDecl>(DC))
    Visit(DCN);
  Out << "@T@";
  Out << D->getName();
}

void USRGenerator::VisitTemplateTypeParmDecl(const TemplateTypeParmDecl *D) {
  GenLoc(D, /*IncludeOffset=*/true);
}

void USRGenerator::GenExtSymbolContainer(const NamedDecl *D) {
  StringRef Container = GetExternalSourceContainer(D);
  if (!Container.empty())
    Out << "@M@" << Container;
}

bool USRGenerator::GenLoc(const Decl *D, bool IncludeOffset) {
  llvm::outs() << "[DEBUG] GenLoc called for: " 
               << (D ? D->getDeclKindName() : "null") 
               << ", IncludeOffset: " << (IncludeOffset ? "true" : "false") 
               << ", generatedLoc: " << (generatedLoc ? "true" : "false") 
               << ", IgnoreResults: " << (IgnoreResults ? "true" : "false") << "\n";
  
  if (generatedLoc) {
    llvm::outs() << "[DEBUG] GenLoc returning early because generatedLoc is true\n";
    return IgnoreResults;
  }
  generatedLoc = true;

  // Guard against null declarations in invalid code.
  if (!D) {
    llvm::outs() << "[DEBUG] GenLoc received null declaration\n";
    IgnoreResults = true;
    return true;
  }

  // Use the location of canonical decl.
  D = D->getCanonicalDecl();

  if (const NamedDecl *ND = dyn_cast<NamedDecl>(D)) {
    llvm::outs() << "[DEBUG] GenLoc for named decl: " << ND->getNameAsString() << "\n";
  }
  
  llvm::outs() << "[DEBUG] GenLoc calling printLoc with BeginLoc: " 
               << (D->getBeginLoc().isValid() ? "valid" : "invalid") << "\n";
  
  bool PrevIgnoreResults = IgnoreResults;
  IgnoreResults =
      IgnoreResults || printLoc(Out, D->getBeginLoc(),
                                Context->getSourceManager(), IncludeOffset);
  
  llvm::outs() << "[DEBUG] GenLoc after printLoc, IgnoreResults changed from " 
               << (PrevIgnoreResults ? "true" : "false") 
               << " to " << (IgnoreResults ? "true" : "false") << "\n";

  return IgnoreResults;
}

static void printQualifier(llvm::raw_ostream &Out, ASTContext &Ctx, NestedNameSpecifier *NNS) {
  // FIXME: Encode the qualifier, don't just print it.
  PrintingPolicy PO(Ctx.getLangOpts());
  PO.SuppressTagKeyword = true;
  PO.SuppressUnwrittenScope = true;
  PO.ConstantArraySizeAsWritten = false;
  PO.AnonymousTagLocations = false;
  NNS->print(Out, PO);
}

void USRGenerator::VisitType(QualType T) {
  // This method mangles in USR information for types.  It can possibly
  // just reuse the naming-mangling logic used by codegen, although the
  // requirements for USRs might not be the same.
  ASTContext &Ctx = *Context;

  do {
    T = Ctx.getCanonicalType(T);
    Qualifiers Q = T.getQualifiers();
    unsigned qVal = 0;
    if (Q.hasConst())
      qVal |= 0x1;
    if (Q.hasVolatile())
      qVal |= 0x2;
    if (Q.hasRestrict())
      qVal |= 0x4;
    if(qVal)
      Out << ((char) ('0' + qVal));
    if (const PackExpansionType *Expansion = T->getAs<PackExpansionType>()) {
      Out << 'P';
      T = Expansion->getPattern();
    }

    if (const BuiltinType *BT = T->getAs<BuiltinType>()) {
      unsigned char c = '\0';
      switch (BT->getKind()) {
        case BuiltinType::Void:
          c = 'v'; break;
        case BuiltinType::Bool:
          c = 'b'; break;
        case BuiltinType::UChar:
          c = 'c'; break;
        case BuiltinType::Char8:
          c = 'u'; break; // FIXME: Check this doesn't collide
        case BuiltinType::Char16:
          c = 'q'; break;
        case BuiltinType::Char32:
          c = 'w'; break;
        case BuiltinType::UShort:
          c = 's'; break;
        case BuiltinType::UInt:
          c = 'i'; break;
        case BuiltinType::ULong:
          c = 'l'; break;
        case BuiltinType::ULongLong:
          c = 'k'; break;
        case BuiltinType::UInt128:
          c = 'j'; break;
        case BuiltinType::Char_U:
        case BuiltinType::Char_S:
          c = 'C'; break;
        case BuiltinType::SChar:
          c = 'r'; break;
        case BuiltinType::WChar_S:
        case BuiltinType::WChar_U:
          c = 'W'; break;
        case BuiltinType::Short:
          c = 'S'; break;
        case BuiltinType::Int:
          c = 'I'; break;
        case BuiltinType::Long:
          c = 'L'; break;
        case BuiltinType::LongLong:
          c = 'K'; break;
        case BuiltinType::Int128:
          c = 'J'; break;
        case BuiltinType::Float16:
        case BuiltinType::Half:
          c = 'h'; break;
        case BuiltinType::Float:
          c = 'f'; break;
        case BuiltinType::Double:
          c = 'd'; break;
        case BuiltinType::Ibm128: // FIXME: Need separate tag
        case BuiltinType::LongDouble:
          c = 'D'; break;
        case BuiltinType::Float128:
          c = 'Q'; break;
        case BuiltinType::NullPtr:
          c = 'n'; break;
#define BUILTIN_TYPE(Id, SingletonId)
#define PLACEHOLDER_TYPE(Id, SingletonId) case BuiltinType::Id:
#include "clang/AST/BuiltinTypes.def"
        case BuiltinType::Dependent:
#define IMAGE_TYPE(ImgType, Id, SingletonId, Access, Suffix) \
        case BuiltinType::Id:
#include "clang/Basic/OpenCLImageTypes.def"
#define EXT_OPAQUE_TYPE(ExtType, Id, Ext) \
        case BuiltinType::Id:
#include "clang/Basic/OpenCLExtensionTypes.def"
        case BuiltinType::OCLEvent:
        case BuiltinType::OCLClkEvent:
        case BuiltinType::OCLQueue:
        case BuiltinType::OCLReserveID:
        case BuiltinType::OCLSampler:
#define SVE_TYPE(Name, Id, SingletonId) \
        case BuiltinType::Id:
#include "clang/Basic/AArch64SVEACLETypes.def"
#define PPC_VECTOR_TYPE(Name, Id, Size) \
        case BuiltinType::Id:
#include "clang/Basic/PPCTypes.def"
#define RVV_TYPE(Name, Id, SingletonId) case BuiltinType::Id:
#include "clang/Basic/RISCVVTypes.def"
        case BuiltinType::ShortAccum:
        case BuiltinType::Accum:
        case BuiltinType::LongAccum:
        case BuiltinType::UShortAccum:
        case BuiltinType::UAccum:
        case BuiltinType::ULongAccum:
        case BuiltinType::ShortFract:
        case BuiltinType::Fract:
        case BuiltinType::LongFract:
        case BuiltinType::UShortFract:
        case BuiltinType::UFract:
        case BuiltinType::ULongFract:
        case BuiltinType::SatShortAccum:
        case BuiltinType::SatAccum:
        case BuiltinType::SatLongAccum:
        case BuiltinType::SatUShortAccum:
        case BuiltinType::SatUAccum:
        case BuiltinType::SatULongAccum:
        case BuiltinType::SatShortFract:
        case BuiltinType::SatFract:
        case BuiltinType::SatLongFract:
        case BuiltinType::SatUShortFract:
        case BuiltinType::SatUFract:
        case BuiltinType::SatULongFract:
        case BuiltinType::BFloat16:
          IgnoreResults = true;
          return;
        case BuiltinType::ObjCId:
          break;
        case BuiltinType::ObjCClass:
          break;
        case BuiltinType::ObjCSel:
          break;
      }
      Out << c;
      return;
    }

    // If we have already seen this (non-built-in) type, use a substitution
    // encoding.
    llvm::DenseMap<const Type *, unsigned>::iterator Substitution
      = TypeSubstitutions.find(T.getTypePtr());
    if (Substitution != TypeSubstitutions.end()) {
      Out << 'S' << Substitution->second << '_';
      return;
    } else {
      // Record this as a substitution.
      unsigned Number = TypeSubstitutions.size();
      TypeSubstitutions[T.getTypePtr()] = Number;
    }

    if (const PointerType *PT = T->getAs<PointerType>()) {
      Out << '*';
      T = PT->getPointeeType();
      continue;
    }
    if (const RValueReferenceType *RT = T->getAs<RValueReferenceType>()) {
      Out << "&&";
      T = RT->getPointeeType();
      continue;
    }
    if (const ReferenceType *RT = T->getAs<ReferenceType>()) {
      Out << '&';
      T = RT->getPointeeType();
      continue;
    }
    if (const FunctionProtoType *FT = T->getAs<FunctionProtoType>()) {
      Out << 'F';
      VisitType(FT->getReturnType());
      Out << '(';
      for (const auto &I : FT->param_types()) {
        Out << '#';
        VisitType(I);
      }
      Out << ')';
      if (FT->isVariadic())
        Out << '.';
      return;
    }
    if (const BlockPointerType *BT = T->getAs<BlockPointerType>()) {
      Out << 'B';
      T = BT->getPointeeType();
      continue;
    }
    if (const ComplexType *CT = T->getAs<ComplexType>()) {
      Out << '<';
      T = CT->getElementType();
      continue;
    }
    if (const TagType *TT = T->getAs<TagType>()) {
      Out << '$';
      VisitTagDecl(TT->getDecl());
      return;
    }
    if (const TemplateTypeParmType *TTP = T->getAs<TemplateTypeParmType>()) {
      Out << 't' << TTP->getDepth() << '.' << TTP->getIndex();
      return;
    }
    if (const TemplateSpecializationType *Spec
                                    = T->getAs<TemplateSpecializationType>()) {
      Out << '>';
      VisitTemplateName(Spec->getTemplateName());
      Out << Spec->getNumArgs();
      for (unsigned I = 0, N = Spec->getNumArgs(); I != N; ++I)
        VisitTemplateArgument(Spec->getArg(I));
      return;
    }
    if (const DependentNameType *DNT = T->getAs<DependentNameType>()) {
      Out << '^';
      printQualifier(Out, Ctx, DNT->getQualifier());
      Out << ':' << DNT->getIdentifier()->getName();
      return;
    }
    if (const InjectedClassNameType *InjT = T->getAs<InjectedClassNameType>()) {
      T = InjT->getInjectedSpecializationType();
      continue;
    }
    if (const auto *VT = T->getAs<VectorType>()) {
      Out << (T->isExtVectorType() ? ']' : '[');
      Out << VT->getNumElements();
      T = VT->getElementType();
      continue;
    }
    if (const auto *const AT = dyn_cast<ArrayType>(T)) {
      Out << '{';
      switch (AT->getSizeModifier()) {
      case ArrayType::Static:
        Out << 's';
        break;
      case ArrayType::Star:
        Out << '*';
        break;
      case ArrayType::Normal:
        Out << 'n';
        break;
      }
      if (const auto *const CAT = dyn_cast<ConstantArrayType>(T))
        Out << CAT->getSize();

      T = AT->getElementType();
      continue;
    }

    // Unhandled type.
    Out << ' ';
    break;
  } while (true);
}

void USRGenerator::VisitTemplateParameterList(
                                         const TemplateParameterList *Params) {
  if (!Params)
    return;
  Out << '>' << Params->size();
  for (TemplateParameterList::const_iterator P = Params->begin(),
                                          PEnd = Params->end();
       P != PEnd; ++P) {
    Out << '#';
    if (isa<TemplateTypeParmDecl>(*P)) {
      if (cast<TemplateTypeParmDecl>(*P)->isParameterPack())
        Out<< 'p';
      Out << 'T';
      continue;
    }

    if (NonTypeTemplateParmDecl *NTTP = dyn_cast<NonTypeTemplateParmDecl>(*P)) {
      if (NTTP->isParameterPack())
        Out << 'p';
      Out << 'N';
      VisitType(NTTP->getType());
      continue;
    }

    TemplateTemplateParmDecl *TTP = cast<TemplateTemplateParmDecl>(*P);
    if (TTP->isParameterPack())
      Out << 'p';
    Out << 't';
    VisitTemplateParameterList(TTP->getTemplateParameters());
  }
}

void USRGenerator::VisitTemplateName(TemplateName Name) {
  if (TemplateDecl *Template = Name.getAsTemplateDecl()) {
    if (TemplateTemplateParmDecl *TTP
                              = dyn_cast<TemplateTemplateParmDecl>(Template)) {
      Out << 't' << TTP->getDepth() << '.' << TTP->getIndex();
      return;
    }

    Visit(Template);
    return;
  }

  // FIXME: Visit dependent template names.
}

void USRGenerator::VisitTemplateArgument(const TemplateArgument &Arg) {
  switch (Arg.getKind()) {
  case TemplateArgument::Null:
    break;

  case TemplateArgument::Declaration:
    Visit(Arg.getAsDecl());
    break;

  case TemplateArgument::NullPtr:
    break;

  case TemplateArgument::TemplateExpansion:
    Out << 'P'; // pack expansion of...
    LLVM_FALLTHROUGH;
  case TemplateArgument::Template:
    VisitTemplateName(Arg.getAsTemplateOrTemplatePattern());
    break;

  case TemplateArgument::Expression:
    // FIXME: Visit expressions.
    break;

  case TemplateArgument::Pack:
    Out << 'p' << Arg.pack_size();
    for (const auto &P : Arg.pack_elements())
      VisitTemplateArgument(P);
    break;

  case TemplateArgument::Type:
    VisitType(Arg.getAsType());
    break;

  case TemplateArgument::Integral:
    Out << 'V';
    VisitType(Arg.getIntegralType());
    Out << Arg.getAsIntegral();
    break;
  }
}

void USRGenerator::VisitUnresolvedUsingValueDecl(const UnresolvedUsingValueDecl *D) {
  if (ShouldGenerateLocation(D) && GenLoc(D, /*IncludeOffset=*/isLocal(D)))
    return;
  VisitDeclContext(D->getDeclContext());
  Out << "@UUV@";
  printQualifier(Out, D->getASTContext(), D->getQualifier());
  EmitDeclName(D);
}

void USRGenerator::VisitUnresolvedUsingTypenameDecl(const UnresolvedUsingTypenameDecl *D) {
  if (ShouldGenerateLocation(D) && GenLoc(D, /*IncludeOffset=*/isLocal(D)))
    return;
  VisitDeclContext(D->getDeclContext());
  Out << "@UUT@";
  printQualifier(Out, D->getASTContext(), D->getQualifier());
  Out << D->getName(); // Simple name.
}

//===----------------------------------------------------------------------===//
// USR generation functions.
//===----------------------------------------------------------------------===//

void generateUSRForGlobalEnum(StringRef EnumName, raw_ostream &OS,
                                            StringRef ExtSymDefinedIn) {
  if (!ExtSymDefinedIn.empty())
    OS << "@M@" << ExtSymDefinedIn;
  OS << "@E@" << EnumName;
}

void generateUSRForEnumConstant(StringRef EnumConstantName,
                                              raw_ostream &OS) {
  OS << '@' << EnumConstantName;
}

bool generateUSRForDecl(const Decl *D,
                                      SmallVectorImpl<char> &Buf) {
  if (!D)
    return true;
  // We don't ignore decls with invalid source locations. Implicit decls, like
  // C++'s operator new function, can have invalid locations but it is fine to
  // create USRs that can identify them.

  USRGenerator UG(&D->getASTContext(), Buf);
  UG.Visit(D);
  return UG.ignoreResults();
}

bool generateUSRForMacro(const MacroDefinitionRecord *MD,
                                       const SourceManager &SM,
                                       SmallVectorImpl<char> &Buf) {
  if (!MD)
    return true;
  return generateUSRForMacro(MD->getName()->getName(), MD->getLocation(),
                             SM, Buf);

}

bool generateUSRForMacro(StringRef MacroName, SourceLocation Loc,
                                       const SourceManager &SM,
                                       SmallVectorImpl<char> &Buf) {
  if (MacroName.empty())
    return true;

  llvm::raw_svector_ostream Out(Buf);

  // Assume that system headers are sane.  Don't put source location
  // information into the USR if the macro comes from a system header.
  bool ShouldGenerateLocation = Loc.isValid() && !SM.isInSystemHeader(Loc);

  Out << getUSRSpacePrefix();
  if (ShouldGenerateLocation)
    printLoc(Out, Loc, SM, /*IncludeOffset=*/true);
  Out << "@macro@";
  Out << MacroName;
  return false;
}

bool generateUSRForType(QualType T, ASTContext &Ctx,
                                      SmallVectorImpl<char> &Buf) {
  if (T.isNull())
    return true;
  T = T.getCanonicalType();

  USRGenerator UG(&Ctx, Buf);
  UG.VisitType(T);
  return UG.ignoreResults();
}

} // namespace armor