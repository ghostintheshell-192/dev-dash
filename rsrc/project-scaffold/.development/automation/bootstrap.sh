#!/bin/bash
# One-time per-clone activation of the project automation (ADR-012).
# Git refuses to auto-enable hooks shipped inside a repo (by design);
# this script makes the opt-in a single explicit gesture.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

echo "DevDash automation bootstrap"
echo "----------------------------"

# 1. Activate project hooks (local config wins over any global hooksPath).
git config core.hooksPath .githooks
echo "✓ core.hooksPath -> .githooks (local to this clone)"

GLOBAL_HOOKS=$(git config --global core.hooksPath 2>/dev/null || true)
if [[ -n "$GLOBAL_HOOKS" ]]; then
    echo "  note: global hooksPath ($GLOBAL_HOOKS) is now overridden for this repo"
fi

# 2. Verify prerequisites of the hook modules.
for tool in python3 bash; do
    if command -v "$tool" >/dev/null 2>&1; then
        echo "✓ $tool available"
    else
        echo "✗ MISSING: $tool (required by hook modules)"
    fi
done
if ! command -v clang-format >/dev/null 2>&1; then
    echo "- clang-format not installed (format hooks stay no-op; fine until .clang-format exists)"
fi

# 3. Make sure entry points and hooks are executable.
chmod +x .development/automation/*.sh .githooks/pre-commit .githooks/post-checkout .githooks/pre-commit.d/* 2>/dev/null || true
echo "✓ entry points and hooks executable"

echo ""
echo "Done. Project hooks are active for this clone."
echo "Branch protection on main/develop is now enforced."
