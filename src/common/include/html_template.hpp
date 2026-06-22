// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>

#ifndef TOOL_VERSION
#define TOOL_VERSION ""
#endif

#ifndef TOOL_DOCS_URL
#define TOOL_DOCS_URL ""
#endif

#define HTML_STYLE_AND_TITLE \
    "<html><head><style>\n" \
    "table { border-collapse: collapse; width: 100%; }\n" \
    "th, td { border: 1px solid black; padding: 8px; text-align: left; }\n" \
    "th { background-color:#add8e6; }\n" \
    "tr:nth-child(even) { background-color:#f2f2f2; }\n" \
    "tr:hover { background-color: #ddd; }\n" \
    ".backward-incompatible { color: red; }\n" \
    ".backward-compatible { color: green; }\n" \
    "</style></head><body>" \
    "<h1>API Compatibility Report</h1>\n" \
    "<h2>ARMOR v" TOOL_VERSION " Report</h2>\n" \
    "<p style='margin:6px 0 12px;font-size:12px;'>" \
    "<a href='" TOOL_DOCS_URL "' target='_blank'" \
    " style='display:inline-block;padding:3px 10px;border-radius:4px;" \
    "background:#e8f0fe;color:#1a73e8;text-decoration:none;" \
    "font-weight:500;border:1px solid #c5d8fd;'>" \
    "&#128196; Docs: " TOOL_DOCS_URL "</a></p>\n"

#define HTML_TABLE_COLUMNS \
    "<tr><th>Header Name</th><th>API Name</th><th>Description</th>" \
    "<th>Change Type</th><th>Source Compatibility</th></tr>\n"

inline const std::string ALPHA_HTML_HEADER =
    HTML_STYLE_AND_TITLE
    "<p>Note: Compiler errors (For more info please run using DEBUG flags and check logs)</p>\n"
    "<table>\n"
    HTML_TABLE_COLUMNS;

inline const std::string BETA_HTML_HEADER =
    HTML_STYLE_AND_TITLE
    "<table>\n"
    HTML_TABLE_COLUMNS;

inline const std::string SIMPLE_HEADER =
    HTML_STYLE_AND_TITLE
    "<table>\n";

inline const std::string HTML_FOOTER = "</table></body></html>";
