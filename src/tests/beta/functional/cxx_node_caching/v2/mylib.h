// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

// ============================================================
// isInlineForwardDeclOfDeclType coverage: FieldDecl branch
// ============================================================

// -- Happy path: forward decl embedded, immediately followed by a field of that type --
struct FieldOwner {
    struct EmbeddedField* fptr;
};
struct EmbeddedField {
    long a;
};

// -- Sad path: forward decl never defined, still adjacent to field --
struct FieldOwnerSad {
    struct EmbeddedFieldNeverDefined** fptr;
};

// ============================================================
// isInlineForwardDeclOfDeclType coverage: FunctionDecl (return type) branch
// ============================================================

// -- Happy path: forward decl immediately followed by function returning that type, later defined --
struct RetOwnerHelper;
struct RetOwnerHelper* retFn();
struct RetOwnerHelper {
    long val;
};

// -- Sad path: forward decl immediately followed by function returning that type, never defined --
struct RetOwnerHelperSad;
struct RetOwnerHelperSad** retFnSad();

// ============================================================
// isInlineForwardDeclOfDeclType coverage: VarDecl branch
// ============================================================

// -- Happy path: forward decl immediately followed by a global var of that type, later defined --
struct VarHelper;
VarHelper* varGlobal;
struct VarHelper {
    long val;
};

// -- Sad path: forward decl immediately followed by a global var of that type, never defined --
struct VarHelperSad;
VarHelperSad** varGlobalSad;

// -- Sad path: adjacency broken by an intervening decl, so NOT treated as inline forward decl --
long interveningVar;
struct AfterVarNotAdjacent;
long anotherInterveningVar;
struct AfterVarNotAdjacent* varGlobalNotAdjacent;
struct AfterVarNotAdjacent {
    long val;
};

// ============================================================
// isInlineForwardDeclOfDeclType coverage: TypedefDecl branch
// ============================================================

// -- Happy path: forward decl immediately followed by typedef of that type, later defined --
struct TdHelper;
typedef struct TdHelper* TdHelper_t;
struct TdHelper {
    long val;
};

// -- Sad path: forward decl immediately followed by typedef of that type, never defined --
struct TdHelperSad;
typedef struct TdHelperSad** TdHelperSad_t;

// ============================================================
// Repeated forward decl still adjacent to the using decl on its last redecl
// ============================================================

struct RedeclaredRetHelper;
struct RedeclaredRetHelper;
struct RedeclaredRetHelper* redeclaredRetFn();
struct RedeclaredRetHelper {
    long val;
};
