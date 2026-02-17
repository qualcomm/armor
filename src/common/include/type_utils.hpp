// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "clang/AST/Type.h"

namespace armor {

/**
 * Unwraps type modifiers like pointers, references, arrays, etc.
 * but preserves typedefs and other sugar types
 */
clang::QualType unwrapTypeModifiers(clang::QualType OriginalType);

} // namespace armor
