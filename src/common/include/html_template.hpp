// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>

const std::string ALPHA_HTML_HEADER = R"(
<html><head><style>
table { border-collapse: collapse; width: 100%; }
th, td { border: 1px solid black; padding: 8px; text-align: left; }
th { background-color:#add8e6; }
tr:nth-child(even) { background-color:#f2f2f2; }
tr:hover { background-color: #ddd; }
.backward-incompatible { color: red; }
.backward-compatible { color: green; }
</style></head><body><h1>API Compatibility Report</h1>
<h2>ARMOR Report</h2>
<p>Note: Compiler errors (For more info please run using DEBUG flags and check logs)</p>
<table>
<tr><th>Header Name</th><th>API Name</th><th>Description</th><th>Change Type</th><th>Source Compatibility</th></tr>
)";

const std::string BETA_HTML_HEADER = R"(
<html><head><style>
table { border-collapse: collapse; width: 100%; }
th, td { border: 1px solid black; padding: 8px; text-align: left; }
th { background-color:#add8e6; }
tr:nth-child(even) { background-color:#f2f2f2; }
tr:hover { background-color: #ddd; }
.backward-incompatible { color: red; }
.backward-compatible { color: green; }
</style></head><body><h1>API Compatibility Report</h1>
<h2>ARMOR Report</h2>
<table>
<tr><th>Header Name</th><th>API Name</th><th>Description</th><th>Change Type</th><th>Source Compatibility</th></tr>
)";

const std::string HTML_FOOTER = "</table></body></html>";
