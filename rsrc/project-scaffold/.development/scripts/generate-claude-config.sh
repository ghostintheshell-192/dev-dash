#!/bin/bash
# Generate auto-generated Claude Code configuration files

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEV_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_ROOT="$(dirname "$DEV_DIR")"
CLAUDE_DIR="$PROJECT_ROOT/.claude"
RULES_DIR="$CLAUDE_DIR/rules"
ADR_DIR="$DEV_DIR/reference/decisions"

echo "Generating Claude Code configuration files..."
echo ""

# ==========================================================================
# Generate key-decisions.md  (ADRs with Impact >= high)
# ==========================================================================

echo "Generating key-decisions.md..."

OUTFILE="$CLAUDE_DIR/key-decisions.md"

# Header
echo "# Key Decisions" > "$OUTFILE"
echo "" >> "$OUTFILE"
echo "⚠️ **Auto-generated digest of high-impact architecture decisions (ADR Impact ≥ high).**" >> "$OUTFILE"
echo "Loaded into every session via the @include in \`.claude/CLAUDE.md\`. Open the linked ADR for full context." >> "$OUTFILE"
echo "" >> "$OUTFILE"

# Emit all ADRs of a given Impact level as a bulleted digest (title + Summary).
# Args: <impact-value> <section-heading> <section-blurb>
emit_level() {
    local want="$1" heading="$2" blurb="$3"
    local count=0
    local out=""
    for adr_file in "$ADR_DIR"/[0-9]*.md; do
        [ -f "$adr_file" ] || continue
        local impact
        impact=$(grep -m1 "^\*\*Impact\*\*:" "$adr_file" | sed 's/^\*\*Impact\*\*:[[:space:]]*//')
        [ "$impact" = "$want" ] || continue
        local filename title summary
        filename=$(basename "$adr_file")
        title=$(head -1 "$adr_file" | sed 's/^#[[:space:]]*//')
        summary=$(grep -m1 "^\*\*Summary\*\*:" "$adr_file" | sed 's/^\*\*Summary\*\*:[[:space:]]*//')
        out+="- **[$title](../.development/reference/decisions/$filename)** — $summary"$'\n'
        count=$((count + 1))
    done
    if [ "$count" -gt 0 ]; then
        echo "## $heading" >> "$OUTFILE"
        echo "" >> "$OUTFILE"
        echo "$blurb" >> "$OUTFILE"
        echo "" >> "$OUTFILE"
        printf '%s' "$out" >> "$OUTFILE"
        echo "" >> "$OUTFILE"
    fi
}

emit_level "critical" "Critical — must not be violated" "Constraints that apply across the whole codebase."
emit_level "high" "High-impact context" "Decisions that shape ongoing work — know these before deciding."

# Footer
echo "---" >> "$OUTFILE"
echo "" >> "$OUTFILE"
echo "*Auto-generated from ADRs with Impact ≥ high. Run \`.development/scripts/generate-claude-config.sh\` to update.*" >> "$OUTFILE"

kd_count=$(grep -lE "^\*\*Impact\*\*:[[:space:]]*(critical|high)" "$ADR_DIR"/[0-9]*.md 2>/dev/null | wc -l)
echo "✓ Generated $OUTFILE ($kd_count ADR(s) with Impact ≥ high)"

# ==========================================================================
# coding-standards.md
# ==========================================================================
# Hand-maintained at .claude/rules/coding-standards.md — NOT generated here.
# Listed for discoverability only.

echo ""
echo "Configuration files ready! Claude Code will load them via @includes in .claude/CLAUDE.md"
