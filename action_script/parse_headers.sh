
#!/usr/bin/env bash
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
set -euo pipefail

# --- Arguments ---
# 1: YAML file path (e.g., $GITHUB_WORKSPACE/public_headers/armor_config.yaml)
# 2: current branch name (e.g., inputs.branch-name)
# 3: BASE_PATH (e.g., $GITHUB_WORKSPACE/base)
# 4: HEAD_PATH (e.g., $GITHUB_WORKSPACE/head)
# 5: WORKSPACE (optional; defaults to $GITHUB_WORKSPACE)

YAML_FILE="${1:?YAML file path required}"
CURRENT_BRANCH="${2:?Current branch name required}"
BASE_PATH="${3:?Base path required}"
HEAD_PATH="${4:?Head path required}"
WORKSPACE="${5:-${GITHUB_WORKSPACE:-}}"

if [[ -z "${WORKSPACE}" ]]; then
  echo "WORKSPACE not provided and GITHUB_WORKSPACE not set"; exit 1
fi

# --- Sanity checks ---
[[ -f "$YAML_FILE" ]] || { echo "YAML not found: $YAML_FILE"; exit 1; }
command -v yq >/dev/null 2>&1 || { echo "yq is required but not installed"; exit 1; }

echo "Parsing $YAML_FILE for branch: $CURRENT_BRANCH"
echo "BASE_PATH=$BASE_PATH"
echo "HEAD_PATH=$HEAD_PATH"
echo "WORKSPACE=$WORKSPACE"

# -----------------------------
# Helpers
# -----------------------------

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

# Expand one pattern against a root and append matches to out
expand_pattern() {
  local root="$1"
  local patt="$2"
  local out="$3"

  # Normalize leading "./"
  case "$patt" in
    ./*) patt="${patt#./}" ;;
  esac

  # If pattern points to a directory, find headers recursively
  if [[ "$patt" == */ ]]; then
    local dir="${patt%/}"
    if [[ -d "$root/$dir" ]]; then
      find "$root/$dir" -type f \( -name "*.h" -o -name "*.hpp" \) -print >> "$out"
    fi
    return
  fi

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

  # Explicit file path relative to root
  [[ -f "$root/$patt" ]] && echo "$root/$patt" >> "$out"
}

# Expand patterns (base/head), normalize to repo-relative paths, and produce final list
expand_and_normalize() {
  local patterns_file="$1"
  local expanded_base="$2"
  local expanded_head="$3"
  local final_out="$4"

  : > "$expanded_base"
  : > "$expanded_head"
  : > "$final_out"

  while IFS= read -r patt; do
    [[ -z "${patt// }" ]] && continue
    expand_pattern "$BASE_PATH" "$patt" "$expanded_base"
    expand_pattern "$HEAD_PATH" "$patt" "$expanded_head"
  done < "$patterns_file"

  # Convert absolute -> repo-relative
  sed -E "s%^${BASE_PATH}/%%" "$expanded_base" >  "${final_out}.tmp1" || true
  sed -E "s%^${HEAD_PATH}/%%" "$expanded_head" >> "${final_out}.tmp1" || true

  # Keep only .h/.hpp, de-duplicate
  grep -E '\.(h|hpp)$' "${final_out}.tmp1" | sort -u > "$final_out" || true
  rm -f "${final_out}.tmp1"

  echo "Resolved headers → $final_out"
  cat "$final_out" || true
}

# -----------------------------
# Orchestration over modes
# -----------------------------
declare -A PATTERNS_FILE
declare -A EXP_BASE
declare -A EXP_HEAD
declare -A FINAL_OUT

PATTERNS_FILE["blocking"]="$WORKSPACE/blocking_patterns.txt"
PATTERNS_FILE["non-blocking"]="$WORKSPACE/nonblocking_patterns.txt"

EXP_BASE["blocking"]="$WORKSPACE/expanded_blocking_base.txt"
EXP_HEAD["blocking"]="$WORKSPACE/expanded_blocking_head.txt"
FINAL_OUT["blocking"]="$WORKSPACE/blocking_headers_final.txt"

EXP_BASE["non-blocking"]="$WORKSPACE/expanded_nonblocking_base.txt"
EXP_HEAD["non-blocking"]="$WORKSPACE/expanded_nonblocking_head.txt"
FINAL_OUT["non-blocking"]="$WORKSPACE/nonblocking_headers_final.txt"

for mode in "blocking" "non-blocking"; do
  extract_patterns "$mode" "${PATTERNS_FILE[$mode]}"
  expand_and_normalize \
    "${PATTERNS_FILE[$mode]}" \
    "${EXP_BASE[$mode]}" \
    "${EXP_HEAD[$mode]}" \
    "${FINAL_OUT[$mode]}"
done

# Preserve previous combined behavior
COMBINED_HEADERS="$WORKSPACE/headers.txt"
cat "${FINAL_OUT["blocking"]}" "${FINAL_OUT["non-blocking"]}" | sort -u > "$COMBINED_HEADERS" || true
echo "Combined headers → $COMBINED_HEADERS"
cat "$COMBINED_HEADERS" || true

# Emit a step output if GITHUB_OUTPUT is available
if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  echo "headers_path=$COMBINED_HEADERS" >> "$GITHUB_OUTPUT"
fi
