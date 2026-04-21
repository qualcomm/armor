// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
/* Test header: valid C code using C++ keywords as identifiers */

/* --- Functions using C++ keywords as parameter names --- */

/* access/object keywords */
int fun_new(int new, int this);
int fun_delete(int delete, int class);
int fun_namespace(int namespace, int template);

/* type/cast keywords */
int fun_cast(int static_cast, int dynamic_cast);
int fun_reinterpret(int reinterpret_cast, int const_cast);

/* boolean/logic keywords */
int fun_bool(int true, int false, int bool);

/* exception keywords */
int fun_try(int try, int catch, int throw);

/* access specifier keywords */
int fun_access(int public, int private, int protected);

/* other C++ keywords */
int fun_misc(int virtual, int explicit);
int fun_operator(int operator, int friend, int using);
int fun_override(int override, int final);

/* --- Variables using C++ keywords as names --- */
typedef struct {
    int new;
    int delete;
    int class;
    int this;
    int virtual;
    int explicit;
    int operator;
    int friend;
    int namespace;
    int template;
    int override;
    int final;
    int true;
    int false;
    int bool;
    int try;
    int catch;
    int throw;
    int public;
    int private;
    int protected;
    int static_cast;
    int dynamic_cast;
    int reinterpret_cast;
    int const_cast;
    int typeid;
    int typename;
    int using;
    int export;
    int mutable;
    int noexcept;
    int constexpr;
    int decltype;
} cpp_keywords_as_fields_t;

/* --- Function pointers --- */
typedef int (*fn_new_t)(int new, int delete);
typedef int (*fn_class_t)(int class, int namespace);
typedef int (*fn_virtual_t)(int virtual, int override);