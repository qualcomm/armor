// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "report_utils.hpp"
#include "html_template.hpp"

using json = nlohmann::json;

namespace {

// ---------------------------------------------------------------------
// Small utilities
// ---------------------------------------------------------------------

// Proper HTML escape for table cells
static std::string html_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
        case '&':  out += "&amp;";  break;
        case '<':  out += "&lt;";   break;
        case '>':  out += "&gt;";   break;
        case '"':  out += "&quot;"; break;
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
    // Decide color by compatibility value
    const bool isIncompatible = (compRaw == "backward_incompatible");
    const char* color = isIncompatible ? "#d32f2f" : "#2e7d32"; // red / green

    // Preserve escaping and <br/> conversions if there are multiple lines
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

// ---------------------------------------------------------------------
// Change category + row adapter
// ---------------------------------------------------------------------

// Only top-level additions are "Functionality_changed"; everything else "Compatibility_changed"
static std::string to_change_category(const std::string& rawChange, bool isTopLevelAddition) {
    if (rawChange == "added" && isTopLevelAddition) return "Functionality_changed";
    return "Compatibility_changed";
}

struct AtomicChange {
    std::string headerfile;
    std::string apiName;
    std::string detail;       // Human-readable detail (Description column)
    std::string rawChange;    // "added", "removed", "modified", "attr_changed"
    bool topLevel = false;    // true if top-level addition
    std::string compatibility;// ignored/overridden at serialization
};

// Enforce rule centrally:
// - Compatibility_changed  -> backward_incompatible
// - Functionality_changed  -> backward_compatible
static json to_record(const AtomicChange& c) {
    const std::string category = to_change_category(c.rawChange, c.topLevel);
    const std::string compat   = (category == "Compatibility_changed")
                               ? "backward_incompatible"
                               : "backward_compatible";
    return json{
        {"headerfile",   c.headerfile},
        {"name",         c.apiName},
        {"description",  c.detail},
        {"changetype",   category},
        {"compatibility",compat}
    };
}

// ---------------------------------------------------------------------
// Function-diff helpers
// ---------------------------------------------------------------------

static bool looks_like_rename(const json& removedParam, const json& addedParam) {
    if (removedParam.value("nodeType", "") != "Parameter" ||
        addedParam.value("nodeType", "") != "Parameter") {
        return false;
    }
    const std::string dtR = removedParam.value("dataType", "");
    const std::string dtA = addedParam.value("dataType", "");
    return !dtR.empty() && dtR == dtA;
}

// Attribute change row for functions
// Attribute change row for functions
static void add_attr_change(std::vector<AtomicChange>& out,
                            const std::string& headerFile,
                            const std::string& funcName,
                            const std::string& attr,
                            const std::string& oldV,
                            const std::string& newV) {
    if (oldV == newV) return;

    AtomicChange row;
    row.headerfile = headerFile;
    row.apiName    = funcName;
    row.rawChange  = "attr_changed";
    row.topLevel   = false;

    std::ostringstream oss;
    if (!oldV.empty() && newV.empty()) {
        // Attribute disappeared in the new snapshot
        oss << "Function attribute " << attr << " removed '" << oldV << "'";
    } else if (oldV.empty() && !newV.empty()) {
        // Attribute newly present in the new snapshot
        oss << "Function attribute " << attr << " added '" << newV << "'";
    } else {
        // Attribute changed from one explicit value to another
        oss << "Function attribute " << attr
            << " changed from '" << oldV << "' to '" << newV << "'";
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

// Compare function-level attributes between removed/added snapshots for a modified function
static std::vector<AtomicChange>
diff_function_attributes(const std::string& headerFile,
                         const std::string& funcName,
                         const json& removedFn, const json& addedFn) {
    std::vector<AtomicChange> out;
    const json& oldJ = removedFn.is_null() ? json::object() : removedFn;
    const json& newJ = addedFn.is_null()   ? json::object() : addedFn;

    add_attr_change(out, headerFile, funcName, "storageQualifier",
                    oldJ.value("storageQualifier", ""), newJ.value("storageQualifier", ""));
    add_attr_change(out, headerFile, funcName, "functionCallingConvention",
                    oldJ.value("functionCallingConvention", ""), newJ.value("functionCallingConvention", ""));
    add_attr_change(out, headerFile, funcName, "inline",
                    inline_to_str(oldJ), inline_to_str(newJ));
    return out;
}

// Handle a "modified" Parameter or ReturnType node with {removed, added} children
static std::vector<AtomicChange>
diff_nested_mod_node(const std::string& headerFile,
                     const std::string& apiName,
                     const json& modNode) {
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
        const std::string qnR     = removed.value("qualifiedName", "");
        std::string nameLeaf = qnR;
        const auto pos = nameLeaf.rfind("::");
        if (pos != std::string::npos) nameLeaf = nameLeaf.substr(pos + 2);
        const std::string dtR = removed.value("dataType", "");
        const std::string dtA = added.value("dataType", "");
        AtomicChange row;
        row.headerfile = headerFile;
        row.apiName    = apiName;
        row.topLevel   = false;
        std::ostringstream oss;
        if (subType == "ReturnType") {
            oss << "Return type changed from '" << dtR << "' to '" << dtA << "'";
        } else {
            oss << subType << " '" << nameLeaf
                << "' type changed from '" << dtR << "' to '" << dtA << "'";
        }
        row.detail    = oss.str();
        row.rawChange = "modified";
        out.push_back(std::move(row));
    }
    return out;
}

// Handle direct Parameter add/remove under a modified Function (+ simple rename inference)
static std::vector<AtomicChange>
diff_direct_param_nodes(const std::string& headerFile,
                        const std::string& apiName,
                        const std::vector<json>& removedParams,
                        const std::vector<json>& addedParams) {
    std::vector<AtomicChange> out;

    std::multimap<std::string, const json*> removedByType;
    std::multimap<std::string, const json*> addedByType;

    for (const auto& r : removedParams) removedByType.emplace(r.value("dataType",""), &r);
    for (const auto& a : addedParams)   addedByType.emplace(a.value("dataType",""), &a);

    std::set<const json*> matchedRemoved, matchedAdded;

    // Try rename pairings
    for (const auto& kv : removedByType) {
        const json* rptr = kv.second;
        const json& r = *rptr;
        auto range = addedByType.equal_range(kv.first);
        for (auto it = range.first; it != range.second; ++it) {
            const json* aptr = it->second;
            if (matchedAdded.count(aptr)) continue;
            if (looks_like_rename(r, *aptr)) {
                std::string rn = r.value("qualifiedName", "");
                std::string an = aptr->value("qualifiedName", "");
                auto posR = rn.rfind("::"); if (posR != std::string::npos) rn = rn.substr(posR + 2);
                auto posA = an.rfind("::"); if (posA != std::string::npos) an = an.substr(posA + 2);

                AtomicChange row;
                row.headerfile = headerFile;
                row.apiName    = apiName;
                row.topLevel   = false;

                std::ostringstream oss;
                oss << "Parameter renamed from '" << rn << "' to '" << an
                    << "' (type '" << kv.first << "')";
                row.detail    = oss.str();
                row.rawChange = "modified";
                out.push_back(std::move(row));

                matchedRemoved.insert(rptr);
                matchedAdded.insert(aptr);
                break;
            }
        }
    }

    // Any unmatched removed -> parameter removed
    for (const auto& kv : removedByType) {
        const json* rptr = kv.second;
        if (matchedRemoved.count(rptr)) continue;
        const json& r = *rptr;
        std::string rn = r.value("qualifiedName", "");
        auto posR = rn.rfind("::");
        if (posR != std::string::npos) rn = rn.substr(posR + 2);

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

    // Any unmatched added -> parameter added
    for (const auto& kv : addedByType) {
        const json* aptr = kv.second;
        if (matchedAdded.count(aptr)) continue;
        const json& a = *aptr;
        std::string an = a.value("qualifiedName", "");
        auto posA = an.rfind("::");
        if (posA != std::string::npos) an = an.substr(posA + 2);

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

// ---------------------------------------------------------------------
// Non-Function recursive describer
// ---------------------------------------------------------------------

// Emit children summary for added/removed non-Function nodes,
// even if the children themselves do not carry explicit tags.
static void emit_added_removed_children(const json& node,
                                        std::vector<std::string>& lines,
                                        const std::string& parentTag) {
    const auto& children = node.value("children", json::array());
    if (children.empty()) return;

    for (const auto& ch : children) {
        const std::string chType = ch.value("nodeType", "");
        const std::string chQN   = ch.value("qualifiedName", "");
        const std::string chDT   = ch.value("dataType", "");
        const std::string chTag  = ch.value("tag", ""); // might be empty

        // Prefer child's tag, otherwise inherit parent's semantic (added/removed)
        const std::string effTag = chTag.empty() ? parentTag : chTag;

        if (effTag == "added") {
            if (!chDT.empty())
                add_desc_line(lines, chType + " added: '" + chQN + "' with type '" + chDT + "'");
            else
                add_desc_line(lines, chType + " added: '" + chQN + "'");
        } else if (effTag == "removed") {
            if (!chDT.empty())
                add_desc_line(lines, chType + " removed: '" + chQN + "' with type '" + chDT + "'");
            else
                add_desc_line(lines, chType + " removed: '" + chQN + "'");
        } else if (effTag == "modified") {
            // Defensive: if grandchildren exist, enumerate with simple pairing
            const auto& gkids = ch.value("children", json::array());
            if (!gkids.empty()) {
                using Key = std::pair<std::string, std::string>;
                std::map<Key, json> rem, add;
                for (const auto& gk : gkids) {
                    std::string gt = gk.value("tag", "");
                    std::string nt = gk.value("nodeType", "");
                    std::string qn = gk.value("qualifiedName", "");
                    Key k{qn, nt};
                    if (gt == "removed") rem[k] = gk;
                    else if (gt == "added") add[k] = gk;
                }
                for (const auto& kv : rem) {
                    const Key& k = kv.first;
                    const json& r = kv.second;
                    auto it = add.find(k);
                    const std::string subNodeType = r.value("nodeType", "");
                    const std::string pname       = r.value("qualifiedName", "");
                    if (it != add.end()) {
                        const json& a = it->second;
                        const std::string dtR = r.value("dataType", "");
                        const std::string dtA = a.value("dataType", "");
                        const std::string displayQN =
                            (subNodeType == "ReturnType") ? qname_stem(pname) : pname;
                        if (!dtR.empty() && !dtA.empty())
                            add_desc_line(lines, subNodeType + " '" + displayQN +
                                                 "' type changed from '" + dtR + "' to '" + dtA + "'");
                        else
                            add_desc_line(lines, subNodeType + " modified: '" + displayQN + "'");
                    } else {
                        const std::string dt = r.value("dataType", "");
                        if (!dt.empty())
                            add_desc_line(lines, subNodeType + " removed: '" + pname + "' with type '" + dt + "'");
                        else
                            add_desc_line(lines, subNodeType + " removed: '" + pname + "'");
                    }
                }
                for (const auto& kv : add) {
                    if (rem.find(kv.first) != rem.end()) continue;
                    const json& a = kv.second;
                    const std::string subNodeType = a.value("nodeType", "");
                    const std::string pname       = a.value("qualifiedName", "");
                    const std::string dt          = a.value("dataType", "");
                    if (!dt.empty())
                        add_desc_line(lines, subNodeType + " added: '" + pname + "' with type '" + dt + "'");
                    else
                        add_desc_line(lines, subNodeType + " added: '" + pname + "'");
                }
            }
        } else {
            // Unknown/empty tag on child: still print its presence with type if any
            if (!chDT.empty())
                add_desc_line(lines, chType + " present: '" + chQN + "' (type '" + chDT + "')");
            else
                add_desc_line(lines, chType + " present: '" + chQN + "'");
        }

        // If the child itself is a container and has grandchildren, enumerate them too.
        if (ch.contains("children") && ch["children"].is_array() && !ch["children"].empty()) {
            emit_added_removed_children(ch, lines, effTag);
        }
    }
}

// Recursively generate detailed description lines for non-Function trees.
static void describe_non_function_recursive(const json& node,
                                            std::vector<std::string>& lines) {
    const std::string tag         = node.value("tag", "");
    const std::string nodeType    = node.value("nodeType", "");
    const std::string qualifiedName= node.value("qualifiedName", "");
    const std::string dataType    = node.value("dataType", "");
    const auto& children          = node.value("children", json::array());

    if (tag == "added") {
        if (!dataType.empty())
            add_desc_line(lines, nodeType + std::string(" added: '") + qualifiedName + "' with type '" + dataType + "'");
        else
            add_desc_line(lines, nodeType + std::string(" added: '") + qualifiedName + "'");
        // Enumerate children under an added container
        emit_added_removed_children(node, lines, "added");
        return;
    }

    if (tag == "removed") {
        if (!dataType.empty())
            add_desc_line(lines, nodeType + std::string(" removed: '") + qualifiedName + "' with type '" + dataType + "'");
        else
            add_desc_line(lines, nodeType + std::string(" removed: '") + qualifiedName + "'");
        // Enumerate children under a removed container
        emit_added_removed_children(node, lines, "removed");
        return;
    }

    if (tag != "modified") {
        // Unknown/no tag: nothing to do
        return;
    }

    using Key = std::pair<std::string, std::string>;
    std::map<Key, json> removedItems, addedItems;

    for (const auto& ch : children) {
        const std::string chTag  = ch.value("tag", "");
        const std::string chType = ch.value("nodeType", "");
        const std::string chQN   = ch.value("qualifiedName", "");
        Key key{chQN, chType};

        if (chTag == "removed") {
            removedItems[key] = ch;
        } else if (chTag == "added") {
            addedItems[key] = ch;
        } else if (chTag == "modified") {
            // Recurse into nested modified nodes (e.g., Field modified; FunctionPointer modified)
            describe_non_function_recursive(ch, lines);
        } else if (chTag.empty() && ch.contains("children")) {
            // Container child with no explicit tag but with children – recurse defensively
            describe_non_function_recursive(ch, lines);
        }
    }

    // Track "added" entries consumed via exact or relaxed pairing
    std::set<Key> consumedAddedKeys;

    // Pairs that look like direct type changes
    for (const auto& entry : removedItems) {
        const auto& key = entry.first;
        const json& removed = entry.second;

        const std::string subNodeType = removed.value("nodeType", "");
        const std::string paramQN     = removed.value("qualifiedName", "");

        // Exact match first
        auto itExact = addedItems.find(key);
        if (itExact != addedItems.end()) {
            const auto& added = itExact->second;
            const std::string dtR = removed.value("dataType", "");
            const std::string dtA = added.value("dataType", "");
            const std::string displayQN =
                (subNodeType == "ReturnType") ? qname_stem(paramQN) : paramQN;
            if (!dtR.empty() && !dtA.empty()) {
                add_desc_line(
                    lines,
                    subNodeType + " '" + displayQN + "' type changed from '" + dtR + "' to '" + dtA + "'"
                );
            } else {
                add_desc_line(lines, subNodeType + " modified: '" + displayQN + "'");
            }
            consumedAddedKeys.insert(itExact->first);
            continue;
        }

        if (subNodeType == "Parameter") {
            const std::string stemR = qname_stem(paramQN);
            const std::string dtR   = removed.value("dataType", "");
            Key bestKey{};
            const json* bestAdded = nullptr;

            for (const auto& addEntry : addedItems) {
                const Key& aKey  = addEntry.first;
                const json& a    = addEntry.second;
                if (a.value("nodeType", "") != "Parameter") continue;
                if (consumedAddedKeys.count(aKey)) continue;
                if (qname_stem(a.value("qualifiedName", "")) == stemR) {
                    bestKey  = aKey;
                    bestAdded= &a;
                    break;
                }
            }

            if (bestAdded) {
                const std::string dtA   = bestAdded->value("dataType", "");
                // const std::string newQN = bestAdded->value("qualifiedName", ""); // no longer used
                // NEW: use the stem (without trailing ::type) in the quoted name
                const std::string displayQN = stemR; // or qname_stem(bestAdded->value("qualifiedName",""))

                if (!dtR.empty() && !dtA.empty()) {
                    add_desc_line(
                        lines,
                        "Parameter modified: '" + displayQN +
                        "' type changed from '" + dtR + "' to '" + dtA + "'"
                    );
                } else {
                    add_desc_line(lines, "Parameter modified: '" + displayQN + "'");
                }
                consumedAddedKeys.insert(bestKey);
                continue; // handled as a modification
            }
        }

        // ---------------------------------------------------------------------------

        // No match -> true removal (existing behavior)
        const std::string dt = removed.value("dataType", "");
        if (!dt.empty())
            add_desc_line(lines, subNodeType + " removed: '" + paramQN + "' with type '" + dt + "'");
        else
            add_desc_line(lines, subNodeType + " removed: '" + paramQN + "'");
    }

    // Added items with no matching removed counterpart (skip ones we consumed)
    for (const auto& entry : addedItems) {
        const auto& key = entry.first;
        if (removedItems.find(key) != removedItems.end()) continue;
        if (consumedAddedKeys.count(key)) continue; // skip paired entries

        const json& added = entry.second;
        const std::string subNodeType = added.value("nodeType", "");
        const std::string paramName   = added.value("qualifiedName", "");
        const std::string dt          = added.value("dataType", "");
        if (!dt.empty())
            add_desc_line(lines, subNodeType + " added: '" + paramName + "' with type '" + dt + "'");
        else
            add_desc_line(lines, subNodeType + " added: '" + paramName + "'");
    }
}

// Build a single multi-line description for a non-Function node
static std::string generate_non_function_description(const json& item) {
    std::vector<std::string> lines;
    describe_non_function_recursive(item, lines);
    if (lines.empty()) {
        // Fallback if nothing was discovered
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

// ---------------------------------------------------------------------
// Group rows by (headerfile, name) so each API has a single description cell
// ---------------------------------------------------------------------

static std::vector<json> group_records_by_function(const std::vector<json>& rows) {
    using Key = std::pair<std::string, std::string>;
    struct Agg {
        std::string headerfile;
        std::string name;
        std::vector<std::string> descriptions;
        bool anyCompatibilityChanged = false;
        bool anyFunctionalityChanged = false;
    };
    std::map<Key, Agg> buckets;

    for (const auto& row : rows) {
        const std::string hf = row.value("headerfile", "");
        const std::string nm = row.value("name", "");
        const std::string ct = row.value("changetype", "");
        const std::string desc = row.value("description", "");
        Key k{hf, nm};
        auto& agg = buckets[k];
        if (agg.headerfile.empty()) {
            agg.headerfile = hf;
            agg.name = nm;
        }
        if (!desc.empty()) agg.descriptions.push_back(desc);
        if (ct == "Compatibility_changed")      agg.anyCompatibilityChanged = true;
        else if (ct == "Functionality_changed") agg.anyFunctionalityChanged = true;
    }

    std::vector<json> out;
    out.reserve(buckets.size());
    for (const auto& kv : buckets) {
        const Agg& a = kv.second;

        // Decide grouped change type (conservative):
        // If any compatibility-affecting row exists -> group is Compatibility Changed
        const bool compat = a.anyCompatibilityChanged;
        const std::string changetype   = compat ? "Compatibility Changed"
                                                : "Functionality Added";
        const std::string compatibility= compat ? "backward_incompatible"
                                                : "backward_compatible";

        // Build a single multi-line description (bulleted / concatenated)
        std::ostringstream d;
        for (size_t i = 0; i < a.descriptions.size(); ++i) {
            if (i) d << "\n";
            d << a.descriptions[i];
        }

        out.push_back(json{
            {"headerfile",   a.headerfile},
            {"name",         a.name},
            {"description",  d.str()},
            {"changetype",   changetype},
            {"compatibility",compatibility}
        });
    }
    return out;
}

} // anonymous namespace

// ---------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------

std::vector<json>
preprocess_api_changes(const json& api_differences, const std::string& header_file_path) {
    std::vector<json> processed;

    for (const auto& change : api_differences) {
        const std::string nodeType = change.value("nodeType", "");
        const std::string tag      = change.value("tag", "");
        const std::string api_name = change.value("qualifiedName", "Unknown");

        // ---------------------- Non-Function nodes ----------------------
        if (nodeType != "Function") {
            AtomicChange row;
            row.headerfile = header_file_path;
            row.apiName    = api_name;
            row.detail     = generate_non_function_description(change); // recursive (may be multi-line)
            row.rawChange  = tag;                       // added/removed/modified
            row.topLevel   = (tag == "added");          // only top-level 'added' => Functionality_changed
            processed.push_back(to_record(row));        // compatibility set from Change Type
            continue;
        }

        // ------------------------ Function nodes ------------------------
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

            // Function attribute snapshots (sibling removed/added)
            if (chType == "Function" && (chTag == "removed" || chTag == "added")) {
                if (chTag == "removed") removedFn = ch;
                else addedFn = ch;
                continue;
            }

            // Parameter/ReturnType node that is "modified" with old/new kids
            if ((chType == "Parameter" || chType == "ReturnType") && chTag == "modified") {
                auto sub = diff_nested_mod_node(header_file_path, api_name, ch);
                rows.insert(rows.end(), sub.begin(), sub.end());
                continue;
            }

            // Direct Parameter add/remove under the function
            if (chType == "Parameter" && (chTag == "added" || chTag == "removed")) {
                if (chTag == "added") directAddedParams.push_back(ch);
                else directRemovedParams.push_back(ch);
                continue;
            }
        }

        // Compare function-level attributes
        // diff_function_attributes already treats nulls as empty objects.
        if (!removedFn.is_null() || !addedFn.is_null()) {
            auto attrRows = diff_function_attributes(header_file_path, api_name, removedFn, addedFn);
            rows.insert(rows.end(), attrRows.begin(), attrRows.end());
        }


        // Handle direct param add/remove (+ simple rename inference)
        if (!directAddedParams.empty() || !directRemovedParams.empty()) {
            auto paramRows = diff_direct_param_nodes(header_file_path, api_name, directRemovedParams, directAddedParams);
            rows.insert(rows.end(), paramRows.begin(), paramRows.end());
        }

        // Fallback: generic "Function modified" (when nothing specific detected)
        if (rows.empty()) {
            AtomicChange row;
            row.headerfile = header_file_path;
            row.apiName    = api_name;
            row.detail     = "Function modified";
            row.rawChange  = "modified";
            row.topLevel   = false;
            rows.push_back(std::move(row));
        }

        // Append rows (ensure nested changes are not treated as top-level additions)
        for (auto& r : rows) {
            r.topLevel = false;
            processed.push_back(to_record(r));
        }
    }

    return processed;
}

void generate_html_report(const std::vector<json>& processed_data,
                          const std::string& output_html_path) {
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


    } else {
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
                          const std::string& output_json_path) {
    if (output_json_path.empty()) return;
    std::ofstream jf(output_json_path);
    auto grouped = group_records_by_function(processed_data);
    jf << json(grouped).dump(4);
    jf.close();
}