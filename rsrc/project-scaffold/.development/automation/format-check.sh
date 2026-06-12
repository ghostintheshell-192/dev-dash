#!/bin/bash
# Standard automation entry point (ADR-012): verify code formatting.
# Contract: runs from repo root; no args = check all tracked sources;
# with args = check only the given files (non-source files are ignored).
# Exit != 0 = at least one file is not formatted. If the project has not
# opted into formatting (no .clang-format at root), this is a declared
# no-op: exit 0.
set -euo pipefail

if [[ ! -f .clang-format ]]; then
    echo "format-check: no .clang-format at root, formatting not enforced (no-op)"
    exit 0
fi

if ! command -v clang-format >/dev/null 2>&1; then
    echo "format-check: WARNING - clang-format not installed, skipping"
    exit 0
fi

# Select candidate files: args if given, otherwise all tracked sources.
if [[ $# -gt 0 ]]; then
    CANDIDATES=("$@")
else
    mapfile -t CANDIDATES < <(git ls-files 'app/**')
fi

FAILED=()
for file in "${CANDIDATES[@]}"; do
    [[ "$file" =~ \.(c|cc|cpp|cxx|h|hpp|hxx)$ ]] || continue
    [[ -f "$file" ]] || continue
    if ! diff -q <(clang-format "$file") "$file" >/dev/null 2>&1; then
        FAILED+=("$file")
    fi
done

if [[ ${#FAILED[@]} -ne 0 ]]; then
    echo "format-check: ${#FAILED[@]} file(s) not formatted:"
    printf '  %s\n' "${FAILED[@]}"
    echo "fix with: .development/automation/format-fix.sh ${FAILED[*]}"
    exit 1
fi

echo "format-check: OK"
