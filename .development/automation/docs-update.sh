#!/bin/bash
# Standard automation entry point (ADR-012): regenerate derived docs.
# Contract: runs from repo root; idempotent; always exit 0 unless a
# generator itself crashes. Prints which files it touched.
# Aggregates: ARCHITECTURE.md, INDEX.md, tech-debt/README.md.
set -euo pipefail

SCRIPTS=.development/scripts

run_generator()
{
    local label="$1"; shift
    if "$@" >/dev/null 2>&1; then
        echo "docs-update: $label regenerated"
    else
        echo "docs-update: WARNING - $label generator failed"
    fi
}

run_generator "ARCHITECTURE.md"      bash "$SCRIPTS/generate-architecture.sh"
run_generator "INDEX.md"             python3 "$SCRIPTS/generate-index.py"
run_generator "tech-debt/README.md"  python3 "$SCRIPTS/update-tech-debt-index.py"
