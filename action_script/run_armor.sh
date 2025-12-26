
#!/usr/bin/env bash
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

set -euo pipefail

# ==============================================================================
# Usage:
#   run_armor.sh <BASE_PATH> <HEAD_PATH> <INTERSECTION_HEADERS_PATH> <ARMOR_BINS_PATH?>
#
# Environment (optional):
#   PROJECT, BRANCH, GITHUB_EVENT, PR_NUMBER, HEADER_DIR, INCLUDE_PATHS, MACRO_FLAGS,
#   REPORT_FORMAT=json, LOG_LEVEL, DUMP_AST_DIFF, ARMOR_CMD, HEAD_SHA, BASE_SHA
# ==============================================================================

log()  { printf "\033[1;34m[INFO]\033[0m %s\n" "$*" >&2; }
warn() { printf "\033[1;33m[WARN]\033[0m %s\n" "$*" >&2; }
die()  { printf "\033[1;31m[ERR]\033[0m %s\n" "$*" >&2; exit 1; }

BASE_PATH="${1:-}"; HEAD_PATH="${2:-}"; INTERSECTION_FILE="${3:-}"; ARMOR_BINS_PATH="${4:-}"
[[ -d "$BASE_PATH" ]] || die "BASE_PATH not a directory"
[[ -d "$HEAD_PATH" ]] || die "HEAD_PATH not a directory"
[[ -f "$INTERSECTION_FILE" ]] || die "Intersection file not found"

PROJECT_URL="${PROJECT_URL:-unknown}"
BRANCH="${BRANCH:-unknown}"
GITHUB_EVENT="${GITHUB_EVENT:-unknown}"
PR_NUMBER="${PR_NUMBER:-}"
workflow_url="${workflow_url:-}"
REPORT_FORMAT="${REPORT_FORMAT:-json}"
LOG_LEVEL="${LOG_LEVEL:-INFO}"
DUMP_AST_DIFF="${DUMP_AST_DIFF:-false}"
HEADER_DIR="${HEADER_DIR:-}"
INCLUDE_PATHS="${INCLUDE_PATHS:-}"
MACRO_FLAGS="${MACRO_FLAGS:-}"
GITHUB_WORKSPACE="${GITHUB_WORKSPACE:-$PWD}"
HEAD_SHA="${HEAD_SHA:-$(git -C "$HEAD_PATH" rev-parse HEAD 2>/dev/null || echo unknown)}"
BASE_SHA="${BASE_SHA:-}"

ARMOR_CMD="${ARMOR_CMD:-${ARMOR_BINS_PATH:-armor}}"
command -v "$ARMOR_CMD" >/dev/null || [[ -x "$ARMOR_CMD" ]] || die "armor CLI not found"
command -v jq >/dev/null || die "jq missing in container"

OUT_ROOT="${GITHUB_WORKSPACE}/armor_output/${HEAD_SHA}"
mkdir -p "$OUT_ROOT"

INCOMPATIBLE_BLOCKING="${OUT_ROOT}/incompatible_blocking.txt"
INCOMPATIBLE_NONBLOCKING="${OUT_ROOT}/incompatible_nonblocking.txt"
: > "$INCOMPATIBLE_BLOCKING"; : > "$INCOMPATIBLE_NONBLOCKING"

BLOCKING_FILE="${GITHUB_WORKSPACE}/blocking_headers_final.txt"
NONBLOCKING_FILE="${GITHUB_WORKSPACE}/nonblocking_headers_final.txt"

mapfile -t HEADERS < <(grep -v '^[[:space:]]*$' "$INTERSECTION_FILE" || true)

METADATA_NDJSON="${OUT_ROOT}/.headers.ndjson"; : > "$METADATA_NDJSON"

for header in "${HEADERS[@]}"; do
  hdr_arg="$header"; [[ -n "$HEADER_DIR" ]] && hdr_arg="$(basename "$header")"
  safe="$(echo "$header" | sed 's/[^A-Za-z0-9_.-]/_/g')"
  WORK_DIR="$(mktemp -d "${GITHUB_WORKSPACE}/.armor_${safe}.XXXXXX")"
  pushd "$WORK_DIR" >/dev/null

  args=(-r "$REPORT_FORMAT" --log-level "$LOG_LEVEL")
  [[ "$DUMP_AST_DIFF" == "true" ]] && args+=(--dump-ast-diff)
  [[ -n "$HEADER_DIR" ]] && args+=(--header-dir "$HEADER_DIR")
  [[ -n "$INCLUDE_PATHS" ]] && args+=($INCLUDE_PATHS)
  [[ -n "$MACRO_FLAGS" ]] && args+=(-m $MACRO_FLAGS)

  base_header_path="$BASE_PATH/$header"
  if [[ ! -f "$base_header_path" ]]; then
    log "Base header missing; creating empty placeholder: $base_header_path"
    mkdir -p "$(dirname "$base_header_path")"
    : > "$base_header_path"
  fi

  head_header_path="$HEAD_PATH/$header"
  if [[ ! -f "$head_header_path" ]]; then
    log "HEAD header missing; creating empty placeholder: $head_header_path"
    mkdir -p "$(dirname "$head_header_path")"
    : > "$head_header_path"
  fi

  "$ARMOR_CMD" "${args[@]}" "$BASE_PATH" "$HEAD_PATH" "$hdr_arg" || warn "armor failed for $header"

  json_report="$WORK_DIR/armor_reports/json_reports/api_diff_report_$(basename "$hdr_arg").json"
  api_names=(); compatibility="backward_compatible"
  if [[ -f "$json_report" ]]; then
    while IFS= read -r line; do
      comp="$(jq -r '.compatibility' <<<"$line")"
      name="$(jq -r '.name' <<<"$line")"
      [[ "$comp" == "backward_incompatible" ]] && compatibility="backward_incompatible"
      [[ -n "$name" && "$name" != "null" ]] && api_names+=("$name")
    done < <(jq -c '.[]' "$json_report")
  fi

  api_json="$(printf '%s\0' "${api_names[@]}" | awk -v RS='\0' 'BEGIN{print "["} {if(NR>1) printf ","; printf "\"" $0 "\""} END{print "]"}')"
  jq -n \
    --arg header "$header" \
    --argjson api_names "$api_json" \
    --arg comp "$compatibility" \
    '{header:$header, api_names:$api_names, compatibility:$comp}' >> "$METADATA_NDJSON"

  if [[ "$compatibility" == "backward_incompatible" ]]; then
    [[ -f "$BLOCKING_FILE" && -s "$BLOCKING_FILE" ]] && grep -Fxq "$header" "$BLOCKING_FILE" && echo "$header" >> "$INCOMPATIBLE_BLOCKING"
    [[ -f "$NONBLOCKING_FILE" && -s "$NONBLOCKING_FILE" ]] && grep -Fxq "$header" "$NONBLOCKING_FILE" && echo "$header" >> "$INCOMPATIBLE_NONBLOCKING"
  fi

  dest="${OUT_ROOT}/${safe}"
  mkdir -p "$dest"
  tar -cf - . 2>/dev/null | tar -xf - -C "$dest" 2>/dev/null || true
  popd >/dev/null; rm -rf "$WORK_DIR"
done

sort -u -o "$INCOMPATIBLE_BLOCKING" "$INCOMPATIBLE_BLOCKING"
sort -u -o "$INCOMPATIBLE_NONBLOCKING" "$INCOMPATIBLE_NONBLOCKING"

headers_array="$(jq -s '.' "$METADATA_NDJSON")"
ts="$(date -u +'%Y-%m-%dT%H:%M:%SZ')"
jq -n \
  --arg project_url "$PROJECT_URL" \
  --arg branch "$BRANCH" \
  --arg event "$GITHUB_EVENT" \
  --argjson pr "$( [[ -n "$PR_NUMBER" ]] && printf '%s' "$PR_NUMBER" || printf 'null' )" \
  --arg timestamp "$ts" \
  --arg head_sha "$HEAD_SHA" \
  --arg base_sha "$BASE_SHA" \
  --arg artifacts "$workflow_url#artifacts" \
  --argjson headers "$headers_array" \
  '{project_url:$project_url, branch:$branch, github_event:$event, pr_number:$pr, timestamp:$timestamp, head_sha:$head_sha, base_sha:$base_sha, artifacts:$artifacts, headers:$headers}' \
  > "${OUT_ROOT}/metadata.json"

overall_status="success"
[[ -s "$INCOMPATIBLE_BLOCKING" ]] && overall_status="failure"

echo "$overall_status" > "${GITHUB_WORKSPACE}/.armor_status"
echo "$OUT_ROOT" > "${GITHUB_WORKSPACE}/.armor_out_root"

[[ -n "${GITHUB_OUTPUT:-}" ]] && {
  echo "status=$overall_status" >> "$GITHUB_OUTPUT"
  echo "metadata_path=${OUT_ROOT}/metadata.json" >> "$GITHUB_OUTPUT"
  echo "incompatible_blocking=$INCOMPATIBLE_BLOCKING" >> "$GITHUB_OUTPUT"
  echo "incompatible_nonblocking=$INCOMPATIBLE_NONBLOCKING" >> "$GITHUB_OUTPUT"
}

log "Status: $overall_status"
log "Metadata: ${OUT_ROOT}/metadata.json"
log "Blocking incompatible: $INCOMPATIBLE_BLOCKING"
