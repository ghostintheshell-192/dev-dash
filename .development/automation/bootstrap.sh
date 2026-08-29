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

# 2. Register the merge driver the derived docs are marked with in
#    .gitattributes. `true` exits 0 without touching the file, which leaves our
#    side in place; post-merge then regenerates it from the merged tree, so
#    which side survived the merge does not matter for a file a generator owns
#    end to end. Registered here rather than shipped, because git keeps merge
#    drivers in config for the same reason it will not auto-enable hooks: a
#    repository must not be able to run code on clone.
#
#    This belongs next to core.hooksPath above and not somewhere else: without
#    the hooks, the attribute would quietly keep one side and never regenerate,
#    which is worse than the conflict it replaces. The two go in together.
git config merge.generated.name "keep either side; post-merge regenerates"
git config merge.generated.driver true
echo "✓ merge.generated -> registered (derived docs resolve by regeneration)"

# 3. Verify prerequisites of the hook modules.
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

# 4. Make sure entry points and hooks are executable.
chmod +x .development/automation/*.sh .githooks/pre-commit .githooks/post-checkout \
    .githooks/post-merge .githooks/pre-commit.d/* 2>/dev/null || true
echo "✓ entry points and hooks executable"

echo ""
echo "Done. Project hooks are active for this clone."
echo "Branch protection on main/develop is now enforced."
