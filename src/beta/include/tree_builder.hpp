// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include <llvm-14/llvm/ADT/SmallVector.h>
#include <llvm-14/llvm/ADT/StringRef.h>

#include "ast_normalized_context.hpp"
#include "node.hpp"
#include "qualified_name_builder.hpp"
#include "fibonacci_hash.hpp"

/**
 * @class TreeBuilder
 * @brief Builds an API node tree from Clang AST declarations.
 * 
 * This class encapsulates all the logic for creating and organizing APINode objects
 * based on Clang AST declarations. It works with an ASTNormalizedContext to store
 * the resulting nodes.
 */

namespace beta{

class TreeBuilder {
private:
    beta::ASTNormalizedContext* context;
    StringBuilder qualifiedName;
    std::vector<std::shared_ptr<beta::APINode>> nodeStack;
public:
    /**
     * @brief Constructs a TreeBuilder with the given context.
     * 
     * @param context The ASTNormalizedContext to store the resulting nodes.
     */
    explicit TreeBuilder(beta::ASTNormalizedContext* context);
    
    // Node management
    void AddNode(const std::shared_ptr<beta::APINode>& node);
    void PushNode(const std::shared_ptr<beta::APINode>& node);
    void PopNode();
    
    // Name management
    void PushName(llvm::StringRef name);
    void PopName();
    const std::string GetCurrentQualifiedName();
    
    // Utility methods
    bool IsDeclFromMainFileAndNotLocal(const clang::Decl* Decl);
    bool IsStmtFromMainFile(const clang::Stmt* Stmt);
    bool isInNameSpaceOrClass(const clang::Decl* Decl);
    bool isWrittenInClassOrNamespace(const clang::Decl* TD);
    void processUnhandledDecl(const clang::Decl* Decl);
    void processUnhandledStmt(const clang::Stmt* Stmt, const std::shared_ptr<beta::APINode>& node);
    uint64_t generateSemanticHashFromDecl(const clang::Decl* Decl);
    uint64_t generateSemanticHashFromStmt(const clang::Stmt* Stmt);
    void normalizeFunctionPointerType(std::string_view typeModifiers, clang::FunctionProtoTypeLoc FTL, const clang::NamedDecl* Decl);
    void normalizeValueDeclNode(const clang::ValueDecl *Decl, unsigned int pos = -1);

    // Node building methods (supported)
    bool BuildCXXRecordNode(clang::CXXRecordDecl* Decl);
    bool BuildEnumNode(clang::EnumDecl* Decl);
    bool BuildFunctionNode(clang::FunctionDecl* Decl);
    bool BuildTypedefDecl(clang::TypedefDecl *Decl);
    bool BuildVarDecl(clang::VarDecl *Decl);
    bool BuildFieldDecl(clang::FieldDecl *Decl);
    void BuildReturnTypeNode(clang::QualType type);
    
    // Unsupported declaration handlers (hash-only)
    void BuildNamespaceDecl(clang::NamespaceDecl* Decl);
    void BuildFunctionTemplateDecl(clang::FunctionTemplateDecl* Decl);
    void BuildClassTemplateDecl(clang::ClassTemplateDecl* Decl);
    void BuildClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl* Decl);
    void BuildClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl* Decl);
    void BuildTypeAliasDecl(clang::TypeAliasDecl* Decl);
    void BuildUsingDecl(clang::UsingDecl* Decl);
    void BuildUsingDirectiveDecl(clang::UsingDirectiveDecl* Decl);
    void BuildNamespaceAliasDecl(clang::NamespaceAliasDecl* Decl);
    void BuildStaticAssertDecl(clang::StaticAssertDecl* Decl);
    void BuildVarTemplateDecl(clang::VarTemplateDecl* Decl);
    void BuildVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl* Decl);
    void BuildVarTemplatePartialSpecializationDecl(clang::VarTemplatePartialSpecializationDecl* Decl);
    void BuildTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl* Decl);
    void BuildValueInitExpr(const clang::Expr* Expr, const std::shared_ptr<beta::APINode>& node);
};

}