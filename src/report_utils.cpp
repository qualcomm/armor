// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <tuple>
#include <algorithm>

#include "report_utils.hpp"
#include "html_template.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {

// -----------------------------------------------------------------------------
// Small utilities
// -----------------------------------------------------------------------------

// Compose the API's display name as "qualifiedName:nodeType".
// Falls back to qualifiedName if nodeType is missing.
static std::string compose_api_name(const nlohmann::json& node) {
    const std::string qn = node.value("qualifiedName", "Unknown");
    const std::string nt = node.value("nodeType", "");
    return nt.empty() ? qn : (qn + ":" + nt);
}

// Proper HTML escape for table cells
static std::string html_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '\"': out += "&quot;"; break;
            case '\'': out += "&#39;";  break;
            default:   out += c;        break;
        }
    }
    return out;
}

// Escape then convert '\n' to <br/> so lines render separately within a cell
static std::string escape_nl2br(const std::string& s) {
    const std::string e = html_escape(s);
    std::string out;
    out.reserve(e.size() + 8);
    for (char c : e) {
        if (c == '\n') out += "<br/>";
        else out += c;
    }
    return out;
}

// Render colored compatibility text (only the text inside the cell, no classes)
static std::string render_colored_compatibility(const std::string& compRaw) {
    const bool isIncompatible = (compRaw == "backward_incompatible");
    const char* color = isIncompatible ? "#d32f2f" : "#2e7d32"; // red / green
    const std::string safe = escape_nl2br(compRaw);
    std::ostringstream oss;
    oss << "<span style=\"color:" << color << ";font-weight:600\">" << safe << "</span>";
    return oss.str();
}

// Convenience for appending description lines
static void add_desc_line(std::vector<std::string>& lines, const std::string& text) {
    lines.push_back(text);
}

// Return everything before the last "::", or the whole string if none.
static std::string qname_stem(const std::string& qn) {
    const auto pos = qn.rfind("::");
    return (pos == std::string::npos) ? qn : qn.substr(0, pos);
}

// Return the last "leaf" after the final "::"
static std::string qn_leaf(const std::string& qn) {
    const auto p = qn.rfind("::");
    return (p == std::string::npos) ? qn : qn.substr(p + 2);
}

// -----------------------------------------------------------------------------
// Overload & emission helpers (NO LAMBDAS)
// -----------------------------------------------------------------------------

// Comparator for sorting (position, type) pairs
static bool less_pair_first(const std::pair<int, std::string>& a,
                            const std::pair<int, std::string>& b) {
    return a.first < b.first;
}

// Collect ordered parameter types of a Function node by numeric leaf (1,2,3,...)
static std::vector<std::string> collect_param_types_ordered(const nlohmann::json& fnNode) {
    std::vector<std::pair<int, std::string> > tmp;
    const auto& kids = fnNode.value("children", nlohmann::json::array());
    for (size_t i = 0; i < kids.size(); ++i) {
        const auto& ch = kids[i];
        if (ch.value("nodeType","") != "Parameter") continue;
        const std::string leaf = qn_leaf(ch.value("qualifiedName",""));
        int pos = 0; // fallback if not numeric
        try { pos = std::stoi(leaf); } catch (...) {}
        tmp.push_back(std::make_pair(pos, ch.value("dataType","")));
    }
    std::sort(tmp.begin(), tmp.end(), less_pair_first);
    std::vector<std::string> out;
    out.reserve(tmp.size());
    for (size_t i = 0; i < tmp.size(); ++i) out.push_back(tmp[i].second);
    return out;
}

static std::string collect_return_type(const nlohmann::json& fnNode) {
    const auto& kids = fnNode.value("children", nlohmann::json::array());
    for (size_t i = 0; i < kids.size(); ++i) {
        const auto& ch = kids[i];
        if (ch.value("nodeType","") == "ReturnType")
            return ch.value("dataType","");
    }
    return "";
}

static std::string format_signature(const std::vector<std::string>& params,
                                    const std::string& ret) {
    std::ostringstream sig;
    sig << "(";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i) sig << ", ";
        sig << params[i];
    }
    sig << ")";
    if (!ret.empty()) sig << " -> " << ret;
    return sig.str();
}

// Function full qualified name from a Function node
static std::string func_qualified_of(const nlohmann::json& j) {
    return j.value("qualifiedName", "");
}

// Overload-wording emission (prints full qualified name)
static void emit_overload_event_line(std::vector<std::string>& lines,
                                     const std::string& funcQN,
                                     const nlohmann::json& fnNode,
                                     const std::string& kind /* "added"|"removed" */) {
    const std::vector<std::string> params = collect_param_types_ordered(fnNode);
    const std::string ret = collect_return_type(fnNode);
    const std::string sig = format_signature(params, ret);
    add_desc_line(lines, "Function '" + funcQN + " " + kind + ": " + sig);
}

// Key used to avoid emitting the same function event twice in one recursive pass.
// Format: "<funcQN>|<kind>|<signature>"
static std::string make_fn_event_key(const std::string& funcQN,
                                     const std::string& kind,
                                     const nlohmann::json& fnNode)
{
    const std::vector<std::string> params = collect_param_types_ordered(fnNode);
    const std::string ret = collect_return_type(fnNode);
    const std::string sig = format_signature(params, ret);
    return funcQN + "|" + kind + "|" + sig;
}

// Emit function add/remove exactly once, with wording controlled by 'asOverload'.
//  - asOverload = true  -> "Function '<funcQN>' overload <kind>: (sig)"
//  - asOverload = false -> "Function '<funcQN>' <kind>: (sig)"
static void emit_fn_event_once(std::vector<std::string>& lines,
                               std::set<std::string>& emitted,
                               const std::string& funcQN,
                               const nlohmann::json& fnNode,
                               const std::string& kind, /* "added"|"removed" */
                               bool asOverload)
{
    const std::string key = make_fn_event_key(funcQN, kind, fnNode);
    if (emitted.find(key) != emitted.end()) return; // already emitted
    emitted.insert(key);

    if (asOverload) {
        emit_overload_event_line(lines, funcQN, fnNode, kind);
    } else {
        const std::vector<std::string> params = collect_param_types_ordered(fnNode);
        const std::string ret = collect_return_type(fnNode);
        const std::string sig = format_signature(params, ret);
        add_desc_line(lines, "Function '" + funcQN + "' " + kind + ": " + sig);
    }
}

// Helper for tracking consumed function children in overload-churn reporting
typedef std::tuple<std::string,std::string,std::string> ChildKey; // (qn, nodeType, tag)
static ChildKey make_child_key(const nlohmann::json& j) {
    return ChildKey(j.value("qualifiedName",""),
                    j.value("nodeType",""),
                    j.value("tag",""));
}

// -----------------------------------------------------------------------------
// Change category + row adapter
// -----------------------------------------------------------------------------

static std::string to_change_category(const std::string& rawChange, bool isTopLevelAddition) {
    if (rawChange == "added" && isTopLevelAddition) return "Functionality_changed";
    return "Compatibility_changed";
}

struct AtomicChange {
    std::string headerfile;
    std::string apiName;
    std::string detail;
    std::string rawChange;    // "added", "removed", "modified", "attr_changed"
    bool        topLevel = false;
    std::string compatibility;    // optional override
};

static json to_record(const AtomicChange& c) {
    const std::string category =
        to_change_category(c.rawChange, c.topLevel);

    const std::string compat =
        !c.compatibility.empty()
            ? c.compatibility
            : ((category == "Compatibility_changed") ? "backward_incompatible" : "backward_compatible");

    return json{
        {"headerfile",    c.headerfile},
        {"name",          c.apiName},
        {"description",   c.detail},
        {"changetype",    category},
        {"compatibility", compat}
    };
}

// -----------------------------------------------------------------------------
// Function-diff helpers
// -----------------------------------------------------------------------------

static bool looks_like_rename(const json& removedParam, const json& addedParam) {
    if (removedParam.value("nodeType", "") != "Parameter" ||
        addedParam.value("nodeType",   "") != "Parameter") {
        return false;
    }
    const std::string dtR = removedParam.value("dataType", "");
    const std::string dtA = addedParam.value("dataType", "");
    return !dtR.empty() && dtR == dtA;
}

static void add_attr_change(std::vector<AtomicChange>& out,
                            const std::string& headerFile,
                            const std::string& funcName,
                            const std::string& attr,
                            const std::string& oldV,
                            const std::string& newV)
{
    if (oldV == newV) return;
    AtomicChange row;
    row.headerfile = headerFile;
    row.apiName    = funcName;
    row.rawChange  = "attr_changed";
    row.topLevel   = false;

    std::ostringstream oss;
    if (!oldV.empty() && newV.empty()) {
        oss << "Function attribute " << attr << " removed '" << oldV << "'";
    } else if (oldV.empty() && !newV.empty()) {
        oss << "Function attribute " << attr << " added '" << newV << "'";
    } else {
        oss << "Function attribute " << attr << " changed from '" << oldV
            << "' to '" << newV << "'";
    }
    row.detail = oss.str();
    out.push_back(std::move(row));
}

static std::string inline_to_str(const json& j) {
    if (j.contains("inline") && j["inline"].is_boolean()) {
        return j["inline"].get<bool>() ? "true" : "false";
    }
    return "";
}

static std::vector<AtomicChange> diff_function_attributes(const std::string& headerFile,
                                                          const std::string& funcName,
                                                          const json& removedFn,
                                                          const json& addedFn)
{
    std::vector<AtomicChange> out;
    const json& oldJ = removedFn.is_null() ? json::object() : removedFn;
    const json& newJ = addedFn.is_null() ? json::object() : addedFn;

    add_attr_change(out, headerFile, funcName, "storageQualifier",
                    oldJ.value("storageQualifier",""),
                    newJ.value("storageQualifier",""));

    add_attr_change(out, headerFile, funcName, "functionCallingConvention",
                    oldJ.value("functionCallingConvention",""),
                    newJ.value("functionCallingConvention",""));

    add_attr_change(out, headerFile, funcName, "inline",
                    inline_to_str(oldJ),
                    inline_to_str(newJ));

    return out;
}

// Handle a "modified" Parameter or ReturnType node with {removed, added} children
static std::vector<AtomicChange> diff_nested_mod_node(const std::string& headerFile,
                                                      const std::string& apiName,
                                                      const json& modNode)
{
    std::vector<AtomicChange> out;
    const auto& kids = modNode.value("children", json::array());
    json removed, added;
    for (const auto& ch : kids) {
        const std::string tag = ch.value("tag", "");
        if (tag == "removed") removed = ch;
        else if (tag == "added") added = ch;
    }

    const std::string nodeType = modNode.value("nodeType", "");
    if (!removed.is_null() && !added.is_null()) {
        const std::string subType = removed.value("nodeType", nodeType);
        const std::string dtR = removed.value("dataType", "");
        const std::string dtA = added.value("dataType", "");

        if (subType == "ReturnType") {
            if (dtR != dtA) {
                AtomicChange row;
                row.headerfile = headerFile;
                row.apiName    = apiName;
                row.topLevel   = false;
                std::ostringstream oss;
                oss << "Return type changed from '" << dtR
                    << "' to '" << dtA << "'";
                row.detail    = oss.str();
                row.rawChange = "modified";
                out.push_back(std::move(row));
            }
            return out;
        }

        // Parameter case
        const std::string qnR = removed.value("qualifiedName", "");
        const std::string qnA = added.value("qualifiedName", "");
        const std::string leafR = qn_leaf(qnR);
        const std::string leafA = qn_leaf(qnA);

        if (dtR != dtA) {
            AtomicChange row;
            row.headerfile = headerFile;
            row.apiName    = apiName;
            row.topLevel   = false;
            std::ostringstream oss;
            oss << "Parameter '" << leafR << "' type changed from '"
                << dtR << "' to '" << dtA << "'";
            row.detail    = oss.str();
            row.rawChange = "modified";
            out.push_back(std::move(row));
        } else {
            AtomicChange rr;
            rr.headerfile = headerFile;
            rr.apiName    = apiName;
            rr.topLevel   = false;
            {
                std::ostringstream oss;
                oss << "Parameter '" << leafR << "' removed (type '" << dtR << "')";
                rr.detail = oss.str();
            }
            rr.rawChange = "removed";
            out.push_back(std::move(rr));

            AtomicChange ar;
            ar.headerfile = headerFile;
            ar.apiName    = apiName;
            ar.topLevel   = false;
            {
                std::ostringstream oss;
                oss << "Parameter '" << leafA << "' added (type '" << dtA << "')";
                ar.detail = oss.str();
            }
            ar.rawChange = "added";
            out.push_back(std::move(ar));
        }
    }
    return out;
}

// Handle direct Parameter add/remove under a modified Function (+ simple rename inference)
static std::vector<AtomicChange> diff_direct_param_nodes(const std::string& headerFile,
                                                         const std::string& apiName,
                                                         const std::vector<json>& removedParams,
                                                         const std::vector<json>& addedParams)
{
    std::vector<AtomicChange> out;

    std::unordered_map<std::string, const json*> remByPos, addByPos;
    for (const auto& r : removedParams)
        remByPos[ qn_leaf(r.value("qualifiedName", "")) ] = &r;
    for (const auto& a : addedParams)
        addByPos[ qn_leaf(a.value("qualifiedName", "")) ] = &a;

    std::set<const json*> matchedRemoved, matchedAdded;

    // 1) Paired by position/name -> if type differs, emit "modified"
    for (const auto& kv : remByPos) {
        const std::string& posKey = kv.first;
        const json* rptr = kv.second;
        auto itA = addByPos.find(posKey);
        if (itA == addByPos.end()) continue;
        const json* aptr = itA->second;

        const std::string dtR = rptr->value("dataType", "");
        const std::string dtA = aptr->value("dataType", "");
        if (dtR != dtA) {
            AtomicChange row;
            row.headerfile = headerFile;
            row.apiName    = apiName;
            row.topLevel   = false;
            std::ostringstream oss;
            // NOTE: apiName is "qualifiedName:nodeType"; the description lines below
            // include the full qualified function name elsewhere in the pipeline.
            oss << "Parameter '" << posKey << "' type changed from '"
                << dtR << "' to '" << dtA << "'";
            row.detail    = oss.str();
            row.rawChange = "modified";
            out.push_back(std::move(row));
            matchedRemoved.insert(rptr);
            matchedAdded.insert(aptr);
        }
    }

    // 2) For remaining, try type-based pairing to detect renames (same type) -> removed+added
    std::multimap<std::string, const json*> removedByType, addedByType;
    for (const auto& r : removedParams)
        if (!matchedRemoved.count(&r))
            removedByType.emplace(r.value("dataType", ""), &r);
    for (const auto& a : addedParams)
        if (!matchedAdded.count(&a))
            addedByType.emplace(a.value("dataType", ""), &a);

    for (const auto& kv : removedByType) {
        const json* rptr = kv.second;
        const json& r = *rptr;
        auto range = addedByType.equal_range(kv.first);
        for (auto it = range.first; it != range.second; ++it) {
            const json* aptr = it->second;
            if (matchedAdded.count(aptr)) continue;
            if (looks_like_rename(r, *aptr)) {
                std::string rn = qn_leaf(r.value("qualifiedName", ""));
                std::string an = qn_leaf(aptr->value("qualifiedName", ""));

                // removed
                {
                    AtomicChange rr;
                    rr.headerfile = headerFile;
                    rr.apiName    = apiName;
                    rr.topLevel   = false;
                    std::ostringstream oss;
                    oss << "Parameter '" << rn << "' removed (type '" << kv.first << "')";
                    rr.detail = oss.str();
                    rr.rawChange = "removed";
                    out.push_back(std::move(rr));
                }
                // added
                {
                    AtomicChange ar;
                    ar.headerfile = headerFile;
                    ar.apiName    = apiName;
                    ar.topLevel   = false;
                    std::ostringstream oss;
                    oss << "Parameter '" << an << "' added (type '" << kv.first << "')";
                    ar.detail = oss.str();
                    ar.rawChange = "added";
                    out.push_back(std::move(ar));
                }
                matchedRemoved.insert(rptr);
                matchedAdded.insert(aptr);
                break;
            }
        }
    }

    // 3) Any unmatched removed -> parameter removed
    for (const auto& kv : removedByType) {
        const json* rptr = kv.second;
        if (matchedRemoved.count(rptr)) continue;
        const json& r = *rptr;
        std::string rn = qn_leaf(r.value("qualifiedName", ""));
        AtomicChange row;
        row.headerfile = headerFile;
        row.apiName    = apiName;
        row.topLevel   = false;
        std::ostringstream oss;
        oss << "Parameter '" << rn << "' removed (type '" << kv.first << "')";
        row.detail    = oss.str();
        row.rawChange = "removed";
        out.push_back(std::move(row));
    }

    // 4) Any unmatched added -> parameter added
    for (const auto& kv : addedByType) {
        const json* aptr = kv.second;
        if (matchedAdded.count(aptr)) continue;
        const json& a = *aptr;
        std::string an = qn_leaf(a.value("qualifiedName", ""));
        AtomicChange row;
        row.headerfile = headerFile;
        row.apiName    = apiName;
        row.topLevel   = false;
        std::ostringstream oss;
        oss << "Parameter '" << an << "' added (type '" << kv.first << "')";
        row.detail    = oss.str();
        row.rawChange = "added";
        out.push_back(std::move(row));
    }

    return out;
}

// -----------------------------------------------------------------------------
// Aggregate helpers for "only added fields" (deep) compatibility override
// -----------------------------------------------------------------------------

static bool isAggregateNodeType(const std::string& nt) {
    return nt == "Struct" ||
           nt == "Class"  ||
           nt == "Union"  ||
           nt == "Record";
}

static bool isFieldLikeNodeType(const std::string& nt) {
    return nt == "Field" ||
           nt == "FieldDecl" ||
           nt == "Member" ||
           nt == "DataMember" ||
           nt == "StructField" ||
           nt == "UnionField";
}

static bool subtree_only_added_fields(const json& n, bool& sawAdd) {
    const std::string tg = n.value("tag", "");
    const std::string nt = n.value("nodeType", "");
    const auto& ch = n.value("children", json::array());

    if (tg.empty()) {
        for (const auto& g : ch) {
            if (!subtree_only_added_fields(g, sawAdd)) return false;
        }
        return true;
    }

    if (tg == "added") {
        sawAdd = true;
        if (isFieldLikeNodeType(nt)) return true;
        if (isAggregateNodeType(nt)) {
            for (const auto& g : ch) {
                if (!subtree_only_added_fields(g, sawAdd)) return false;
            }
            return true;
        }
        return false;
    }

    if (tg == "modified") {
        if (isAggregateNodeType(nt)) {
            for (const auto& g : ch) {
                if (!subtree_only_added_fields(g, sawAdd)) return false;
            }
            return true;
        }
        return false;
    }

    return false;
}

static bool is_aggregate_only_field_additions_deep(const json& node) {
    const std::string nodeType = node.value("nodeType", "");
    if (!isAggregateNodeType(nodeType)) return false;

    const auto& kids = node.value("children", json::array());
    if (kids.empty()) return false;

    bool sawAnyAddition = false;
    for (const auto& c : kids) {
        if (!subtree_only_added_fields(c, sawAnyAddition)) return false;
    }
    return sawAnyAddition;
}

// Detect an Enum that only added new enumerators (no removes/other modifications).
static bool is_enum_only_value_additions(const json& node) {
    const std::string nodeType = node.value("nodeType", "");
    if (nodeType != "Enum" && nodeType != "Enumeration") return false;
    const auto& children = node.value("children", json::array());
    bool sawAdded = false;
    for (const auto& ch : children) {
        const std::string tag = ch.value("tag", "");
        if (tag.empty()) continue;
        const std::string chType = ch.value("nodeType", "");
        if (tag == "added") {
            if (chType != "EnumValue" && chType != "Enumerator" && chType != "EnumField") {
                return false;
            }
            sawAdded = true;
        } else {
            return false;
        }
    }
    return sawAdded;
}

// -----------------------------------------------------------------------------
// Non-Function recursive describer
// -----------------------------------------------------------------------------

static void append_child_desc(std::vector<std::string>& lines,
                              const std::string& chType,
                              const json& j,
                              const std::string& what)
{
    const std::string dt = j.value("dataType", "");
    const std::string qn = j.value("qualifiedName", "");
    if (!dt.empty()) {
        add_desc_line(lines, chType + " " + what + ": '" + qn + "' with type '" + dt + "'");
    } else {
        add_desc_line(lines, chType + " " + what + ": '" + qn + "'");
    }
}

static void emit_added_removed_children(const json& node,
                                        std::vector<std::string>& lines,
                                        const std::string& parentTag)
{
    const auto& children = node.value("children", json::array());
    if (children.empty()) return;

    std::set<std::string> emittedFnEvents;

    for (const auto& ch : children) {
        const std::string chType = ch.value("nodeType", "");
        const std::string chQN   = ch.value("qualifiedName", "");
        const std::string chDT   = ch.value("dataType", "");
        const std::string chTag  = ch.value("tag", "");

        const std::string effTag = chTag.empty() ? parentTag : chTag;

        if (effTag == "added") {
            if (chType == "Function") {
                const std::string funcQN = ch.value("qualifiedName","");
                emit_fn_event_once(lines, emittedFnEvents, funcQN, ch, "added", /*asOverload*/false);
            } else {
                append_child_desc(lines, chType, ch, "added");
            }
        } else if (effTag == "removed") {
            if (chType == "Function") {
                const std::string funcQN = ch.value("qualifiedName","");
                emit_fn_event_once(lines, emittedFnEvents, funcQN, ch, "removed", /*asOverload*/false);
            } else {
                append_child_desc(lines, chType, ch, "removed");
            }
        } else if (effTag == "modified") {
            // recurse later if needed
        } else {
            if (!chDT.empty())
                add_desc_line(lines, chType + " present: '" + chQN + "' (type '" + chDT + "')");
            else
                add_desc_line(lines, chType + " present: '" + chQN + "'");
        }

        if (chType != "Function" && ch.contains("children") && ch["children"].is_array() && !ch["children"].empty()) {
            emit_added_removed_children(ch, lines, effTag);
        }
    }
}

static void describe_non_function_recursive(const json& node, std::vector<std::string>& lines) {
    const std::string tag          = node.value("tag", "");
    const std::string nodeType     = node.value("nodeType", "");
    const std::string qualifiedName= node.value("qualifiedName", "");
    const std::string dataType     = node.value("dataType", "");
    const auto& children           = node.value("children", json::array());

    if (tag == "added") {
        if (!dataType.empty())
            add_desc_line(lines, nodeType + std::string(" added: '") + qualifiedName + "' with type '" + dataType + "'");
        else
            add_desc_line(lines, nodeType + std::string(" added: '") + qualifiedName + "'");
        emit_added_removed_children(node, lines, "added");
        return;
    }

    if (tag == "removed") {
        if (!dataType.empty())
            add_desc_line(lines, nodeType + std::string(" removed: '") + qualifiedName + "' with type '" + dataType + "'");
        else
            add_desc_line(lines, nodeType + std::string(" removed: '") + qualifiedName + "'");
        emit_added_removed_children(node, lines, "removed");
        return;
    }

    if (tag != "modified") {
        return;
    }

    std::set<std::string> emittedFnEvents;

    struct FnGroup {
        std::vector<json> added;
        std::vector<json> removed;
        std::vector<json> modified;
    };
    std::unordered_map<std::string, FnGroup> fnGroups; // key = function leaf

    // Group by function *leaf* to detect overload churn at this level
    for (size_t i = 0; i < children.size(); ++i) {
        const auto& ch = children[i];
        const std::string chType = ch.value("nodeType","");
        if (chType != "Function") continue;
        const std::string chTag  = ch.value("tag","");
        const std::string leaf   = qn_leaf(ch.value("qualifiedName",""));
        FnGroup& g = fnGroups[leaf];
        if (chTag == "added")        g.added.push_back(ch);
        else if (chTag == "removed") g.removed.push_back(ch);
        else if (chTag == "modified")g.modified.push_back(ch);
    }

    std::set<ChildKey> consumedForOverload;
    for (std::unordered_map<std::string, FnGroup>::const_iterator it = fnGroups.begin();
         it != fnGroups.end(); ++it)
    {
        const FnGroup& g = it->second;
        if (!g.added.empty() && !g.removed.empty()) {
            // Overloading case: emit explicit overload lines (use full qualifiedName)
            for (size_t r = 0; r < g.removed.size(); ++r) {
                const std::string funcQN = g.removed[r].value("qualifiedName","");
                emit_fn_event_once(lines, emittedFnEvents, funcQN, g.removed[r], "removed", /*asOverload*/true);
                consumedForOverload.insert(make_child_key(g.removed[r]));
            }
            for (size_t a = 0; a < g.added.size(); ++a) {
                const std::string funcQN = g.added[a].value("qualifiedName","");
                emit_fn_event_once(lines, emittedFnEvents, funcQN, g.added[a], "added", /*asOverload*/true);
                consumedForOverload.insert(make_child_key(g.added[a]));
            }
        }
    }

    using Key = std::pair<std::string,std::string>;
    std::map<Key, json> removedItems, addedItems;

    for (size_t i = 0; i < children.size(); ++i) {
        const auto& ch   = children[i];
        const std::string chTag  = ch.value("tag", "");
        const std::string chType = ch.value("nodeType", "");
        const std::string chQN   = ch.value("qualifiedName", "");

        if (!consumedForOverload.empty()) {
            ChildKey ck = make_child_key(ch);
            if (consumedForOverload.count(ck)) continue;
        }

        Key key(chQN, chType);
        if (chTag == "removed") {
            removedItems[key] = ch;
        } else if (chTag == "added") {
            addedItems[key] = ch;
        } else if (chTag == "modified") {
            describe_non_function_recursive(ch, lines);
        } else if (chTag.empty() && ch.contains("children")) {
            describe_non_function_recursive(ch, lines);
        }
    }

    std::set<Key> consumedAddedKeys;

    // Pairs that look like direct type changes
    for (const auto& entry : removedItems) {
        const auto& key = entry.first;
        const json& removed = entry.second;
        const std::string subNodeType = removed.value("nodeType", "");
        const std::string paramQN     = removed.value("qualifiedName", "");

        auto itExact = addedItems.find(key);
        if (itExact != addedItems.end()) {
            const auto& added = itExact->second;

            if (subNodeType == "Function") {
                // True non-overload: show param/return diffs with full function QN
                std::unordered_map<std::string, std::string> rParams, aParams;
                std::string rRet, aRet;

                for (const auto& n : removed.value("children", json::array())) {
                    const std::string nt = n.value("nodeType","");
                    if (nt == "Parameter") {
                        rParams[ qn_leaf(n.value("qualifiedName","")) ] = n.value("dataType","");
                    } else if (nt == "ReturnType") {
                        rRet = n.value("dataType","");
                    }
                }
                for (const auto& n : added.value("children", json::array())) {
                    const std::string nt = n.value("nodeType","");
                    if (nt == "Parameter") {
                        aParams[ qn_leaf(n.value("qualifiedName","")) ] = n.value("dataType","");
                    } else if (nt == "ReturnType") {
                        aRet = n.value("dataType","");
                    }
                }

                const std::string funcQN = removed.value("qualifiedName","");
                if (!rRet.empty() && !aRet.empty() && rRet != aRet) {
                    add_desc_line(lines, "Function '" + funcQN + "': Return type changed from '" + rRet + "' to '" + aRet + "'");
                }
                std::set<std::string> allKeys;
                for (auto& kv : rParams) allKeys.insert(kv.first);
                for (auto& kv : aParams) allKeys.insert(kv.first);

                for (const auto& k : allKeys) {
                    const auto itR = rParams.find(k);
                    const auto itA = aParams.find(k);
                    const bool hasR = (itR != rParams.end());
                    const bool hasA = (itA != aParams.end());
                    if (hasR && hasA) {
                        const std::string& dtR = itR->second;
                        const std::string& dtA = itA->second;
                        if (dtR != dtA) {
                            add_desc_line(lines, "Function '" + funcQN + "': Parameter '" + k + "' type changed from '" + dtR + "' to '" + dtA + "'");
                        }
                    } else if (hasR && !hasA) {
                        const std::string& dtR = itR->second;
                        add_desc_line(lines, "Function '" + funcQN + "': Parameter '" + k + "' removed" + (dtR.empty() ? "" : " (type '" + dtR + "')"));
                    } else if (!hasR && hasA) {
                        const std::string& dtA = itA->second;
                        add_desc_line(lines, "Function '" + funcQN + "': Parameter '" + k + "' added" + (dtA.empty() ? "" : " (type '" + dtA + "')"));
                    }
                }
            } else {
                const std::string dtR = removed.value("dataType", "");
                const std::string dtA = added.value("dataType", "");
                const std::string displayQN = (subNodeType == "ReturnType") ? qname_stem(paramQN) : paramQN;
                if (!dtR.empty() && !dtA.empty()) {
                    if (subNodeType == "Parameter") {
                        const std::string funcQN = qname_stem(paramQN);
                        const std::string paramLeaf= qn_leaf(paramQN);
                        add_desc_line(lines, "Function '" + funcQN + "': Parameter '" + paramLeaf + "' type changed from '" + dtR + "' to '" + dtA + "'");
                    } else {
                        add_desc_line(lines, subNodeType + " '" + displayQN + "' type changed from '" + dtR + "' to '" + dtA + "'");
                    }
                } else {
                    if (subNodeType == "Parameter") {
                        const std::string funcQN = qname_stem(paramQN);
                        const std::string paramLeaf= qn_leaf(paramQN);
                        add_desc_line(lines, "Function '" + funcQN + "': Parameter '" + paramLeaf + "' modified");
                    } else {
                        add_desc_line(lines, subNodeType + " modified: '" + displayQN + "'");
                    }
                }
            }
            consumedAddedKeys.insert(itExact->first);
            continue;
        }

        if (subNodeType == "Parameter") {
            // relaxed pairing for Parameter (by stem)
            const std::string stemR = qname_stem(paramQN); // this IS the full function QN
            const std::string dtR   = removed.value("dataType", "");
            Key bestKey{std::string(), std::string()};
            const json* bestAdded = nullptr;

            for (const auto& addEntry : addedItems) {
                const Key& aKey = addEntry.first;
                const json& a = addEntry.second;
                if (a.value("nodeType","") != "Parameter") continue;
                if (consumedAddedKeys.count(aKey)) continue;
                if (qname_stem(a.value("qualifiedName","")) == stemR) {
                    bestKey = aKey;
                    bestAdded = &a;
                    break;
                }
            }

            if (bestAdded) {
                const std::string dtA = bestAdded->value("dataType", "");
                const std::string funcQN = stemR;
                if (!dtR.empty() && !dtA.empty()) {
                    add_desc_line(lines, "Function '" + funcQN + "': Parameter modified: type changed from '" + dtR + "' to '" + dtA + "'");
                } else {
                    add_desc_line(lines, "Function '" + funcQN + "': Parameter modified");
                }
                consumedAddedKeys.insert(bestKey);
                continue; // handled as a modification
            }
        }

        // ---- No match -> true removal
        if (subNodeType == "Function") {
            const std::string funcQN = removed.value("qualifiedName","");
            emit_fn_event_once(lines, emittedFnEvents, funcQN, removed, "removed", /*asOverload*/false);
        } else {
            const std::string dt = removed.value("dataType", "");
            if (subNodeType == "Parameter") {
                const std::string funcQN  = qname_stem(paramQN);
                const std::string paramLeaf= qn_leaf(paramQN);
                add_desc_line(lines, "Function '" + funcQN + "': Parameter '" + paramLeaf + "' removed" + (dt.empty() ? "" : " (type '" + dt + "')"));
            } else {
                if (!dt.empty())
                    add_desc_line(lines, subNodeType + " removed: '" + paramQN + "' with type '" + dt + "'");
                else
                    add_desc_line(lines, subNodeType + " removed: '" + paramQN + "'");
            }
        }
    }

    // Added items with no matching removed counterpart
    for (const auto& entry : addedItems) {
        const auto& key   = entry.first;
        if (removedItems.find(key) != removedItems.end()) continue;
        if (consumedAddedKeys.count(key)) continue;

        const json& added = entry.second;
        const std::string subNodeType = added.value("nodeType", "");
        const std::string qn          = added.value("qualifiedName", "");
        const std::string dt          = added.value("dataType", "");

        if (subNodeType == "Function") {
            const std::string funcQN = added.value("qualifiedName","");
            emit_fn_event_once(lines, emittedFnEvents, funcQN, added, "added", /*asOverload*/false);
        } else {
            if (subNodeType == "Parameter") {
                const std::string funcQN = qname_stem(qn);
                const std::string paramLeaf= qn_leaf(qn);
                add_desc_line(lines, "Function '" + funcQN + "': Parameter '" + paramLeaf + "' added" + (dt.empty() ? "" : " (type '" + dt + "')"));
            } else {
                if (!dt.empty())
                    add_desc_line(lines, subNodeType + " added: '" + qn + "' with type '" + dt + "'");
                else
                    add_desc_line(lines, subNodeType + " added: '" + qn + "'");
            }
        }
    }
}

// Build a single multi-line description for a non-Function node
static std::string generate_non_function_description(const json& item) {
    std::vector<std::string> lines;
    describe_non_function_recursive(item, lines);
    if (lines.empty()) {
        const std::string nodeType = item.value("nodeType", "");
        const std::string tag      = item.value("tag", "");
        const std::string qn       = item.value("qualifiedName", "");
        return nodeType + " " + tag + ": '" + qn + "'";
    }
    std::ostringstream oss;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i) oss << "\n";
        oss << lines[i];
    }
    return oss.str();
}

// -----------------------------------------------------------------------------
// Group rows by (headerfile, name) so each API has a single description cell
// -----------------------------------------------------------------------------

static std::vector<json> group_records_by_function(const std::vector<json>& rows) {
    using Key = std::pair<std::string,std::string>;
    struct Agg {
        std::string headerfile;
        std::string name;
        std::vector<std::string> descriptions;
        bool anyCompatibilityChanged = false;
        bool anyFunctionalityChanged = false;
        bool anyBackwardIncompatible = false;
    };

    std::map<Key, Agg> buckets;
    for (const auto& row : rows) {
        const std::string hf   = row.value("headerfile", "");
        const std::string nm   = row.value("name", "");
        const std::string ct   = row.value("changetype", "");
        const std::string desc = row.value("description", "");
        const std::string comp = row.value("compatibility", "");

        Key k{hf, nm};
        auto& agg = buckets[k];
        if (agg.headerfile.empty()) {
            agg.headerfile = hf;
            agg.name       = nm;
        }
        if (!desc.empty()) agg.descriptions.push_back(desc);
        if (ct == "Compatibility_changed") agg.anyCompatibilityChanged = true;
        else if (ct == "Functionality_changed") agg.anyFunctionalityChanged = true;

        if (comp == "backward_incompatible") agg.anyBackwardIncompatible = true;
    }

    std::vector<json> out;
    out.reserve(buckets.size());
    for (const auto& kv : buckets) {
        const Agg& a = kv.second;
        const bool anyCompChanged = a.anyCompatibilityChanged;
        const std::string changetype = anyCompChanged ? "Compatibility Changed" : "Functionality Added";
        const std::string compatibility = a.anyBackwardIncompatible ? "backward_incompatible" : "backward_compatible";

        std::ostringstream d;
        for (size_t i = 0; i < a.descriptions.size(); ++i) {
            if (i) d << "\n";
            d << a.descriptions[i];
        }

        out.push_back(json{
            {"headerfile",    a.headerfile},
            {"name",          a.name},
            {"description",   d.str()},
            {"changetype",    changetype},
            {"compatibility", compatibility}
        });
    }
    return out;
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

std::vector<json> preprocess_api_changes(const json& api_differences,
                                         const std::string& header_file_path)
{
    std::vector<json> processed;

    for (const auto& change : api_differences) {
        const std::string nodeType = change.value("nodeType", "");
        const std::string tag      = change.value("tag", "");
        const std::string api_name = compose_api_name(change);

        // ---------------- Non-Function nodes
        if (nodeType != "Function") {
            AtomicChange row;
            row.headerfile = header_file_path;
            row.apiName    = api_name;
            row.detail     = generate_non_function_description(change);
            row.rawChange  = tag;
            row.topLevel   = (tag == "added");

            if (tag == "modified" &&
                (is_enum_only_value_additions(change) ||
                 is_aggregate_only_field_additions_deep(change))) {
                row.compatibility = "backward_compatible";
            }

            processed.push_back(to_record(row));
            continue;
        }

        // ---------------- Function nodes
        if (tag == "added") {
            AtomicChange row{header_file_path, api_name, "Function added", "added", /*topLevel*/true, ""};
            processed.push_back(to_record(row));
            continue;
        }
        if (tag == "removed") {
            AtomicChange row{header_file_path, api_name, "Function removed", "removed", /*topLevel*/false, ""};
            processed.push_back(to_record(row));
            continue;
        }

        // tag == "modified" -> inspect internals
        const auto& children = change.value("children", json::array());
        std::vector<AtomicChange> rows;
        std::vector<json> directAddedParams, directRemovedParams;
        json removedFn, addedFn;

        for (const auto& ch : children) {
            const std::string chType = ch.value("nodeType", "");
            const std::string chTag  = ch.value("tag", "");

            if (chType == "Function" && (chTag == "removed" || chTag == "added")) {
                if (chTag == "removed") removedFn = ch;
                else                     addedFn  = ch;
                continue;
            }

            if ((chType == "Parameter" || chType == "ReturnType") && chTag == "modified") {
                auto sub = diff_nested_mod_node(header_file_path, api_name, ch);
                rows.insert(rows.end(), sub.begin(), sub.end());
                continue;
            }

            if (chType == "Parameter" && (chTag == "added" || chTag == "removed")) {
                if (chTag == "added") directAddedParams.push_back(ch);
                else                  directRemovedParams.push_back(ch);
                continue;
            }
        }

        if (!removedFn.is_null() || !addedFn.is_null()) {
            auto attrRows = diff_function_attributes(header_file_path, api_name, removedFn, addedFn);
            rows.insert(rows.end(), attrRows.begin(), attrRows.end());
        }

        if (!directAddedParams.empty() || !directRemovedParams.empty()) {
            auto paramRows = diff_direct_param_nodes(header_file_path, api_name,
                                                     directRemovedParams, directAddedParams);
            rows.insert(rows.end(), paramRows.begin(), paramRows.end());
        }

        if (rows.empty()) {
            AtomicChange row;
            row.headerfile = header_file_path;
            row.apiName    = api_name;
            row.detail     = "Function modified";
            row.rawChange  = "modified";
            row.topLevel   = false;
            rows.push_back(std::move(row));
        }

        for (auto& r : rows) {
            r.topLevel = false;
            processed.push_back(to_record(r));
        }
    }

    return processed;
}

void generate_html_report(const std::vector<json>& processed_data,
                          const std::string& output_html_path
                        ) {
    std::ofstream html(output_html_path);

    if (processed_data.empty()) {
        html << "<h2 style=\"margin-bottom: 10px;\">ARMOR Report</h2>\n";
        html << "<table border=\"1\" style=\"border-collapse: collapse; width: 100%; background-color: #f2f2f2;\">\n";
        html << "  <tr>\n";
        html << "    <td style=\"text-align: center; padding: 10px;\">\n";
        html << "      Skipping ARMOR report generation as these API type changes are currently unsupported in the tool.<br>\n";
        html << "      Support will be added in future updates. For more details, refer to the <a href=\"https://confluence.qualcomm.com/confluence/display/Linux/ARMOR+Tool+Onboarding+Guide+for+Tech+Teams#ARMORToolOnboardingGuideforTechTeams-ARMORToolOverview\" target=\"_blank\">ARMOR Tool Onboarding Guide</a>.\n";
        html << "    </td>\n";
        html << "  </tr>\n";
        html << "</table>\n";
    } 
    else {
        html << HTML_HEADER;
        auto grouped = group_records_by_function(processed_data);
        for (const auto& entry : grouped) {
            html << "<tr>\n";
            html << "<td> " << escape_nl2br(entry.value("headerfile", ""))   << " </td>\n";
            html << "<td> " << escape_nl2br(entry.value("name", ""))         << " </td>\n";
            html << "<td> " << escape_nl2br(entry.value("description", ""))  << " </td>\n";
            html << "<td> " << escape_nl2br(entry.value("changetype", ""))   << " </td>\n";

            const std::string comp = entry.value("compatibility", "");
            html << "<td> " << render_colored_compatibility(comp) << " </td>\n";
            html << "</tr>\n";
        }
    }

    html << HTML_FOOTER;
    html.close();
}

void generate_json_report(const std::vector<json>& processed_data,
                          const std::string& output_json_path)
{
    if (output_json_path.empty()) return;
    std::ofstream jf(output_json_path);
    auto grouped = group_records_by_function(processed_data);
    jf << json(grouped).dump(4);
    jf.close();
}
