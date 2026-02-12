
#!/usr/bin/env bash
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

set -euo pipefail

DEBUG_LEVEL="${DEBUG:-0}"          # 0 | 1 | trace
TRACE_PATTERNS="${TRACE_PATTERNS:-0}"

log_info()  { printf "[INFO]  %s\n" "$*" >&2; }
log_warn()  { printf "[WARN]  %s\n" "$*" >&2; }
log_err()   { printf "[ERR ]  %s\n" "$*" >&2; }
log_debug() { [[ "$DEBUG_LEVEL" == "1" || "$DEBUG_LEVEL" == "trace" ]] && printf "[DEBUG] %s\n" "$*" >&2; }

[[ "${DEBUG_LEVEL}" == "trace" ]] && { export PS4='+ ${BASH_SOURCE##*/}:${LINENO}: '; set -x; }

CONFIG_YAML="${1:?CONFIG_YAML is required}"
BRANCH="${2:?BRANCH is required}"
HEAD_PATH="${3:?HEAD_PATH is required}"
WORKSPACE="${4:-${GITHUB_WORKSPACE:-$(pwd)}}"

[[ -f "$CONFIG_YAML" ]] || { log_err "config not found: $CONFIG_YAML"; exit 1; }
[[ -d "$HEAD_PATH"   ]] || { log_err "HEAD_PATH not a directory: $HEAD_PATH"; exit 1; }
command -v yq >/dev/null 2>&1 || { log_err "yq is required but not installed"; exit 1; }

mkdir -p "$WORKSPACE"

REL_BLOCKING="$WORKSPACE/blocking_headers_final.txt"
REL_NONBLOCKING="$WORKSPACE/nonblocking_headers_final.txt"
HEADERS_TXT="$WORKSPACE/headers.txt"
: > "$REL_BLOCKING"; : > "$REL_NONBLOCKING"; : > "$HEADERS_TXT"

log_info "[parse_headers] config=$CONFIG_YAML branch=$BRANCH head=$HEAD_PATH ws=$WORKSPACE"

append_rel() {
  # $1 = mode ("blocking"|"non-blocking"), $2 = out_file
  local mode="$1" out="$2"
  local patt_file; patt_file="$(mktemp)"
  # Extract patterns safely; ignore errors if key missing
  yq ".branches.${BRANCH}.modes.${mode}.headers[]" "$CONFIG_YAML" >"$patt_file" || : 
  sort -u -o "$patt_file" "$patt_file" || :

  while IFS= read -r patt || [[ -n "$patt" ]]; do
    [[ -z "${patt//[[:space:]]/}" ]] && continue
    # Normalize leading "./"
    [[ "$patt" == ./* ]] && patt="${patt#./}"

    log_debug "expand(mode=${mode}) patt='${patt}'"
    # 1) Trailing slash / explicit dir -> recurse and pick headers
    if [[ "$patt" == */ || -d "$HEAD_PATH/$patt" ]]; then
      while IFS= read -r abs; do
        [[ -z "$abs" ]] && continue
        case "$abs" in
          *.h|*.hpp) ;;
          *) continue ;;
        esac
        # Convert absolute -> repo-relative
        if [[ "$abs" == "$HEAD_PATH/"* ]]; then
          printf '%s\n' "${abs#"$HEAD_PATH/"}" >>"$out"
          [[ "$TRACE_PATTERNS" == "1" ]] && printf '  -> %s\n' "${abs#"$HEAD_PATH/"}" >&2
        fi
      done < <(find "$HEAD_PATH/${patt%/}" -type f -print 2>/dev/null || true)
      continue
    fi

    # 2) Globs -> match by path
    if [[ "$patt" == *'*'* || "$patt" == *'?'* || "$patt" == *'['* ]]; then
      while IFS= read -r abs; do
        [[ -z "$abs" ]] && continue
        case "$abs" in
          *.h|*.hpp) ;;
          *) continue ;;
        esac
        if [[ "$abs" == "$HEAD_PATH/"* ]]; then
          printf '%s\n' "${abs#"$HEAD_PATH/"}" >>"$out"
          [[ "$TRACE_PATTERNS" == "1" ]] && printf '  -> %s\n' "${abs#"$HEAD_PATH/"}" >&2
        fi
      done < <(find "$HEAD_PATH" -type f -path "$HEAD_PATH/$patt" -print 2>/dev/null || true)
      continue
    fi

    # 3) Explicit file (relative to HEAD_PATH)
    if [[ -f "$HEAD_PATH/$patt" ]]; then
      case "$patt" in
        *.h|*.hpp)
          printf '%s\n' "$patt" >>"$out"
          [[ "$TRACE_PATTERNS" == "1" ]] && printf '  -> %s\n' "$patt" >&2
          ;;
      esac
    fi
  done < "$patt_file"

  # Dedup
  sort -u -o "$out" "$out" || :
  rm -f "$patt_file"
}

append_rel "blocking"      "$REL_BLOCKING"
append_rel "non-blocking"  "$REL_NONBLOCKING"

# headers.txt = union of finals
if [[ -s "$REL_BLOCKING" || -s "$REL_NONBLOCKING" ]]; then
  cat "$REL_BLOCKING" "$REL_NONBLOCKING" | sort -u > "$HEADERS_TXT" || {
    log_err "failed to write union to '$HEADERS_TXT'"; exit 1;
  }
else
  log_warn "Both repo-relative lists are empty; 'headers.txt' will be empty."
  : > "$HEADERS_TXT"
fi

# Summary
for f in "$REL_BLOCKING" "$REL_NONBLOCKING" "$HEADERS_TXT"; do
  if [[ -s "$f" ]]; then
    cnt=$(wc -l <"$f")
    log_info "  • $(basename "$f"): $cnt entries"
  else
    log_info "  • $(basename "$f"): (empty)"
   fi
done


