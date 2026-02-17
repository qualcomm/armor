// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include <string>

// Enums for categorization
enum class NodeKind {
    Namespace,
    Class,
    Struct,
    Union,
    Enum,
    Function,
    Method,
    Field,
    Typedef,
    TypeAlias,
    Parameter,
    TemplateParam,
    BaseClass,
    Variable,
    ReturnType,
    FunctionPointer,
    Enumerator,
    Macro,
    Comment,
    Templet,
    Unknown
};

enum class AccessSpec {
    Public,
    Protected,
    Private,
    None
};

enum class APINodeStorageClass {
    None,
    Static,
    Extern,
    Register,
    Auto
};

enum class VirtualQualifier {
    None,
    Virtual,
    PureVirtual,
    Override
};

enum PARSING_STATUS {
    FATAL_ERRORS = 0,
    NO_FATAL_ERRORS = 1
};

enum PARSER{
    ALPHA_PARSER = 0,
    BETA_PARSER = 1
};

const std::string LOG_FILE_PATH = "debug_output/logs/diagnostics.log";

const std::string TEST_LOG_FILE_PATH = "output.txt";