// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <string>

constexpr const char* CRITICAL_PARSING_ERRORS = "Critical parsing errors occurred.";

constexpr const char* CHANGED_UNSUPPORTED_COMPATIBLE = "File changed with unsupported updates. Changes are also made in un-parsed code regions as well. Cannot comment on compatibility.";
constexpr const char* CHANGED_UNSUPPORTED_INCOMPATIBLE = "File changed with unsupported updates. Changes are also made in un-parsed code regions as well. But changes are in-compatibile.";

constexpr const char* CHANGED_SUPPORTED_COMPATIBLE = "File changed with supported updates. Changes are also made in inactive code regions. Cannot comment on compatibility.";
constexpr const char* CHANGED_SUPPORTED_INCOMPATIBLE = "File changed with supported updates. Changes are also made in inactive code regions. Some changes are in-compatible.";

constexpr const char* CHANGED_COMMENTS = "File changed with comments or documentation updates. Changes are also made in inactive code regions. Cannot comment on compatibility.";
constexpr const char* CHANGED_NON_FUNCTIONAL = "File changed with non-functional updates (formatting, whitespace). Changes are also made in inactive code regions. Cannot comment on compatibility.";

constexpr const char* UNCHANGED_UNSUPPORTED_COMPATIBLE = "File changed with unsupported updates. Cannot comment on compatibility.";
constexpr const char* UNCHANGED_UNSUPPORTED_INCOMPATIBLE = "File changed with unsupported updates. Some changes are incompatible.";

constexpr const char* UNCHANGED_SUPPORTED_COMPATIBLE = "File changed with supported updates. Changes are compatible.";
constexpr const char* UNCHANGED_SUPPORTED_INCOMPATIBLE = "File changed with supported updates. Changes are incompatible.";

constexpr const char* UNCHANGED_COMMENTS = "File changed with only comments or documentation updates. Changes are compatible.";
constexpr const char* UNCHANGED_NON_FUNCTIONAL = "File changed with non-functional updates (formatting, whitespace). Changes are compatible.";

constexpr const char* ERROR_INVALID_PARSED_STATUS = "File changed. Invalid parsed status encountered. System error.";
constexpr const char* ERROR_INVALID_UNPARSED_STATUS = "Invalid unparsed status encountered. System error.";
constexpr const char* ERROR_UNKNOWN = "Unknown categorization error.";

enum class OverAllStatus{
	FATAL_ERRORS = 0,
  BACKWARD_COMPATIABLE = 1,
  BACKWARD_INCOMPATIBLE =2,     
  UNSUPPORTED_UPDATES = 3,     
  SUPPORTED_UPDATES = 4,      
  COMMENTS_UPDATED = 5,   
  NON_FUNCTIONAL_CHANGES = 6,
  IN_ACTIVE = 7,
  CATEGORIZATION_ERROR=8,
  NOT_CATEGORIZED = 9
};

const char* serialize(OverAllStatus status);

const char* getOverAllCategory(unsigned int parsedDiffStatus, unsigned int unParsedDiffStatus, bool compatibility);

const char* getReasonForCategorization(unsigned int parsedDiffStatus, unsigned int unParsedDiffStatus, bool compatibility);