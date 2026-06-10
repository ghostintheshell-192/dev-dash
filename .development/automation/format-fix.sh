#!/bin/bash
# Standard automation entry point (ADR-012): apply code formatting.
# Contract: runs from repo root; no args = fix all tracked sources;
# with args = fix only the given files. No-op without .clang-format.
set -euo pipefail

if [[ ! -f .clang-format ]]; then
    echo "format-fix: no .clang-format at root, nothing to apply (no-op)"
    exit 0
fi

if ! command -v clang-format >/dev/null 2>&1; then
    echo "format-fix: ERROR - clang-format not installed"
    exit 1
fi

if [[ $# -gt 0 ]]; then
    CANDIDATES=("$@")
else
    mapfile -t CANDIDATES < <(git ls-files 'app/**')
fi

COUNT=0
for file in "${CANDIDATES[@]}"; do
    [[ "$file" =~ \.(c|cc|cpp|cxx|h|hpp|hxx)$ ]] || continue
    [[ -f "$file" ]] || continue
    clang-format -i "$file"
    COUNT=$((COUNT + 1))
done

echo "format-fix: formatted $COUNT file(s)"
