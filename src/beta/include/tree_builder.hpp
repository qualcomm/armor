// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include <llvm-14/llvm/ADT/StringRef.h>

#include "ast_normalized_context.hpp"
#include "node.hpp"
#include "qualified_name_builder.hpp"

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
    bool IsFromMainFileAndNotLocal(const clang::Decl* Decl);
    void normalizeFunctionPointerType(std::string_view typeModifiers, clang::FunctionProtoTypeLoc FTL, const clang::NamedDecl* Decl);
    void normalizeValueDeclNode(const clang::ValueDecl *Decl, unsigned int pos = -1);

    // Node building methods
    bool BuildCXXRecordNode(clang::CXXRecordDecl* Decl);
    bool BuildEnumNode(clang::EnumDecl* Decl);
    bool BuildFunctionNode(clang::FunctionDecl* Decl);
    bool BuildTypedefDecl(clang::TypedefDecl *Decl);
    bool BuildVarDecl(clang::VarDecl *Decl);
    bool BuildFieldDecl(clang::FieldDecl *Decl);

    void BuildReturnTypeNode(clang::QualType type);

};

}