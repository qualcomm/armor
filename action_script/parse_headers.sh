
#!/usr/bin/env bash
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
set -euo pipefail

# --- Arguments ---
# 1: YAML file path (e.g., $GITHUB_WORKSPACE/public_headers/config.yml)
# 2: current branch name (e.g., inputs.branch-name)
# 3: HEAD_PATH (e.g., $GITHUB_WORKSPACE/head)
# 4: WORKSPACE
YAML_FILE="${1:?YAML file path required}"
CURRENT_BRANCH="${2:?Current branch name required}"
HEAD_PATH="${3:?Head path required}"
WORKSPACE="${4:-${GITHUB_WORKSPACE:-}}"

if [[ -z "${WORKSPACE}" ]]; then
  echo "WORKSPACE not provided and GITHUB_WORKSPACE not set"; exit 1
fi

# --- Sanity checks ---
[[ -f "$YAML_FILE" ]] || { echo "YAML not found: $YAML_FILE"; exit 1; }
command -v yq >/dev/null 2>&1 || { echo "yq is required but not installed"; exit 1; }

echo "Parsing $YAML_FILE for branch: $CURRENT_BRANCH"
echo "HEAD_PATH=$HEAD_PATH"
echo "WORKSPACE=$WORKSPACE"

# ----------------------------------------------------------------------
# Helpers
# ----------------------------------------------------------------------

# Extract patterns for a given mode into a target file (deduped)
# usage: extract_patterns "blocking" "$OUT_FILE"
extract_patterns() {
  local mode="$1"
  local out="$2"
  : > "$out"
  # Tolerate missing keys
  yq ".branches.${CURRENT_BRANCH}.modes.${mode}.headers[]" "$YAML_FILE" >> "$out" || true
  sort -u -o "$out" "$out" || true
  echo "Patterns (${mode}):"
  cat "$out" || true
}

# Expand one pattern against HEAD root and append matches to out
expand_pattern_head() {
  local root="$1"   # HEAD_PATH
  local patt="$2"
  local out="$3"

  # Normalize leading "./"
  case "$patt" in
    ./*) patt="${patt#./}";;
  esac

  # Pattern that points to a directory and wants recursive header discovery
  if [[ "$patt" == */ ]]; then
    local dir="${patt%/}"
    if [[ -d "$root/$dir" ]]; then
      find "$root/$dir" -type f \( -name "*.h" -o -name "*.hpp" \) -print >> "$out"
    fi
    return
  fi

  # If pattern is an existing directory, recurse
  if [[ -d "$root/$patt" ]]; then
    find "$root/$patt" -type f \( -name "*.h" -o -name "*.hpp" \) -print >> "$out"
    return
  fi

  # Glob-like patterns
  if [[ "$patt" == *"*"* || "$patt" == *"?"* || "$patt" == *"["*"]"* ]]; then
    local dir_part; dir_part="$(dirname "$patt")"
    local base_part; base_part="$(basename "$patt")"
    local search_dir="$root"
    [[ "$dir_part" != "." ]] && search_dir="$root/$dir_part"

    if [[ -d "$search_dir" ]]; then
      case "$base_part" in
        "**.h"|"**/*.h")   find "$search_dir" -type f -name "*.h"   -print >> "$out" ;;
        "**.hpp"|"**/*.hpp") find "$search_dir" -type f -name "*.hpp" -print >> "$out" ;;
        *)                 find "$search_dir" -type f -name "$base_part" -print >> "$out" ;;
      esac
    fi
    return
  fi

  # Explicit file path relative to HEAD root
  [[ -f "$root/$patt" ]] && echo "$root/$patt" >> "$out"
}

# Expand patterns only against HEAD, normalize to repo-relative paths, and produce final list
expand_and_normalize_head() {
  local patterns_file="$1"
  local expanded_head="$2"
  local final_out="$3"

  : > "$expanded_head"
  : > "$final_out"

  while IFS= read -r patt; do
    [[ -z "${patt// }" ]] && continue
    expand_pattern_head "$HEAD_PATH" "$patt" "$expanded_head"
  done < "$patterns_file"

  # Convert absolute -> repo-relative (using HEAD_PATH only)
  sed -E "s%^${HEAD_PATH}/%%" "$expanded_head" > "${final_out}.tmp1" || true

  # Keep only .h/.hpp, de-duplicate
  grep -E '\.(h|hpp)$' "${final_out}.tmp1" | sort -u > "$final_out" || true
  rm -f "${final_out}.tmp1"

  echo "Resolved headers → $final_out"
  cat "$final_out" || true
}

# ----------------------------------------------------------------------
# Orchestration over modes
# ----------------------------------------------------------------------
declare -A PATTERNS_FILE
declare -A EXP_HEAD
declare -A FINAL_OUT

PATTERNS_FILE["blocking"]="$WORKSPACE/blocking_patterns.txt"
PATTERNS_FILE["non-blocking"]="$WORKSPACE/nonblocking_patterns.txt"

EXP_HEAD["blocking"]="$WORKSPACE/expanded_blocking_head.txt"
FINAL_OUT["blocking"]="$WORKSPACE/blocking_headers_final.txt"

EXP_HEAD["non-blocking"]="$WORKSPACE/expanded_nonblocking_head.txt"
FINAL_OUT["non-blocking"]="$WORKSPACE/nonblocking_headers_final.txt"

for mode in "blocking" "non-blocking"; do
  extract_patterns "$mode" "${PATTERNS_FILE[$mode]}"
  expand_and_normalize_head \
    "${PATTERNS_FILE[$mode]}" \
    "${EXP_HEAD[$mode]}" \
    "${FINAL_OUT[$mode]}"
done

HEADERS="$WORKSPACE/headers.txt"
cat "${FINAL_OUT["blocking"]}" "${FINAL_OUT["non-blocking"]}" \
  | sort -u > "$HEADERS" || true

echo "Eligible headers → $HEADERS"
cat "$HEADERS" || true

# Emit a step output if GITHUB_OUTPUT is available
if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  echo "headers_path=$HEADERS" >> "$GITHUB_OUTPUT"
fi
