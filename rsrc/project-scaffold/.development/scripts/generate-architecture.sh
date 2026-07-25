#!/bin/bash
# Generate ARCHITECTURE.md with project tree and file descriptions.
# Descriptions are extracted by a language-specific extract-summary script.
#
# Usage: .development/scripts/generate-architecture.sh

set -e

# ─── Common Setup ─────────────────────────────────────────────────────

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEV_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_ROOT="$(dirname "$DEV_DIR")"
OUTPUT_FILE="$DEV_DIR/ARCHITECTURE.md"
ADR_DIR="$DEV_DIR/reference/decisions"

YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m'

# ─── Project Configuration ────────────────────────────────────────────
# Modify this section for each project. DevDash substitutes the {PLACEHOLDER}
# tokens when applying the scaffold; edit them by hand otherwise.

PROJECT_NAME="{PROJECT_NAME}"
# Source-file globs to scan (e.g. "*.py" / "*.ts" "*.tsx" / "*.cpp" "*.h").
FILE_GLOBS=({FILE_GLOBS})
SKIP_FILES=()
EXCLUDE_DIRS=("build" "dist" "out" "node_modules" "target" "vendor" "external" ".git")
EXTRACT_CMD="$SCRIPT_DIR/extract-summary.sh"
MAX_DESC_LENGTH=200
DOCS_REF="\`docs/\`"

# Source directories to scan (e.g. "$PROJECT_ROOT/src").
SOURCE_DIRS=({SOURCE_DIRS})
# Base for relative path calculation (the printed "### dirname" headers).
REL_BASE="$PROJECT_ROOT"

# Project-specific header content (architecture/layer overview block).
# Substituted by DevDash; replace by hand with your own overview otherwise.
generate_project_header() {
    cat << 'EOF'
{PROJECT_LAYER_OVERVIEW}
EOF
}

# ─── Generic Logic (same across projects) ──────────────────────────────

is_excluded_dir() {
    local dirname="$1"
    for excl in "${EXCLUDE_DIRS[@]}"; do
        [[ "$dirname" == "$excl" ]] && return 0
    done
    return 1
}

is_skipped_file() {
    local filename="$1"
    for skip in "${SKIP_FILES[@]}"; do
        [[ "$filename" == "$skip" ]] && return 0
    done
    return 1
}

# Populate the global array FIND_NAME_ARGS with `find` -name predicates, -o
# between each pattern.
#
# Array form, spliced quoted into `find` — NOT a string through `eval`. With
# `eval` the shell performs pathname expansion on the unquoted globs against
# the CWD (the repo root): a source file sitting there that matches a glob
# makes `find` search for that literal filename instead (or abort on a
# malformed expression when several match), so the tree comes out empty while
# the script still reports success. See the dev-dash tech-debt note
# `scaffold-architecture-eval-glob-expansion` (Option A).
build_find_name_args() {
    FIND_NAME_ARGS=()
    local first=1
    for glob in "${FILE_GLOBS[@]}"; do
        if [ $first -eq 1 ]; then
            FIND_NAME_ARGS+=(-name "$glob")
            first=0
        else
            FIND_NAME_ARGS+=(-o -name "$glob")
        fi
    done
}

# True when at least one existing SOURCE_DIRS entry actually holds files.
# Used to tell "nothing to scan" (legitimate empty tree) apart from "the scan
# matched nothing" (the failure mode above), which must never pass silently.
source_dirs_populated() {
    for source_dir in "${SOURCE_DIRS[@]}"; do
        [[ -d "$source_dir" ]] || continue
        if [[ -n "$(find "$source_dir" -type f 2>/dev/null | head -n 1)" ]]; then
            return 0
        fi
    done
    return 1
}

generate_adr_list() {
    if [[ ! -d "$ADR_DIR" ]]; then
        echo "- See \`reference/decisions/\` for architecture decisions"
        return
    fi

    for adr in "$ADR_DIR"/[0-9]*.md; do
        [[ -f "$adr" ]] || continue
        local filename number title summary impact line
        filename=$(basename "$adr" .md)
        number="${filename%%-*}"
        # Real H1 title (strip leading "# " and the "ADR-NNN:" prefix), not the slug.
        title=$(head -1 "$adr" | sed 's/^#[[:space:]]*//; s/^ADR-[0-9]*:[[:space:]]*//')
        summary=$(grep -m1 "^\*\*Summary\*\*:" "$adr" | sed 's/^\*\*Summary\*\*:[[:space:]]*//')
        impact=$(grep -m1 "^\*\*Impact\*\*:" "$adr" | sed 's/^\*\*Impact\*\*:[[:space:]]*//')
        line="- [ADR-$number: $title](reference/decisions/$filename.md)"
        [[ -n "$impact" ]] && line="$line \`[$impact]\`"
        [[ -n "$summary" ]] && line="$line — $summary"
        printf '%s\n' "$line"
    done
}

generate_header() {
    echo "# Architecture Reference"
    echo ""
    echo "Quick reference for navigating the $PROJECT_NAME codebase."
    echo "For detailed documentation, see $DOCS_REF."
    echo ""

    generate_project_header

    echo ""
    echo "## Key Decisions"
    echo ""

    generate_adr_list

    echo ""
    echo "## Project Tree"
    echo ""
    echo "> Auto-generated from source code."
    echo "> Run \`.development/scripts/generate-architecture.sh\` to update."
    echo ""
}

process_directory() {
    local dir="$1"
    local reldir="${dir#$REL_BASE/}"

    # Get files directly in this directory matching any of FILE_GLOBS.
    local files=()
    build_find_name_args
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find "$dir" -maxdepth 1 -type f \( "${FIND_NAME_ARGS[@]}" \) -print0 2>/dev/null | sort -z)

    # Filter out skipped files
    local filtered=()
    for filepath in "${files[@]}"; do
        local filename
        filename=$(basename "$filepath")
        is_skipped_file "$filename" || filtered+=("$filepath")
    done

    # Only print header if there are non-skipped files
    if [[ ${#filtered[@]} -gt 0 ]]; then
        echo ""
        echo "### $reldir"

        for filepath in "${filtered[@]}"; do
            local file
            file=$(basename "$filepath")
            local desc
            desc=$($EXTRACT_CMD "$filepath" 2>/dev/null || true)

            if [[ -z "$desc" ]]; then
                echo "- \`$file\`"
            elif [[ ${#desc} -gt $MAX_DESC_LENGTH ]]; then
                echo "- \`$file\` — ${desc:0:$MAX_DESC_LENGTH}..."
            else
                echo "- \`$file\` — $desc"
            fi
        done
    fi

    # Process subdirectories
    local subdirs=()
    while IFS= read -r -d '' subdir; do
        subdirs+=("$subdir")
    done < <(find "$dir" -maxdepth 1 -mindepth 1 -type d -print0 2>/dev/null | sort -z)

    for subdir in "${subdirs[@]}"; do
        local dirname
        dirname=$(basename "$subdir")
        is_excluded_dir "$dirname" && continue
        process_directory "$subdir"
    done
}

generate_tree() {
    for source_dir in "${SOURCE_DIRS[@]}"; do
        [[ -d "$source_dir" ]] || continue
        process_directory "$source_dir"
    done
}

generate_footer() {
    echo ""
    echo "---"
    echo ""
    echo "*Auto-generated by \`.development/scripts/generate-architecture.sh\`*"
}

count_stats() {
    local total=0
    local missing=0
    build_find_name_args

    for source_dir in "${SOURCE_DIRS[@]}"; do
        [[ -d "$source_dir" ]] || continue

        # Build excluded-dirs prune args (array, same reason as above)
        local prune=()
        for excl in "${EXCLUDE_DIRS[@]}"; do
            prune+=(-path "*/${excl}/*" -prune -o)
        done

        while IFS= read -r -d '' filepath; do
            local filename
            filename=$(basename "$filepath")
            is_skipped_file "$filename" && continue
            ((total++)) || true
            local desc
            desc=$($EXTRACT_CMD "$filepath" 2>/dev/null || true)
            if [[ -z "$desc" ]]; then
                ((missing++)) || true
            fi
        done < <(find "$source_dir" "${prune[@]}" \( "${FIND_NAME_ARGS[@]}" \) -type f -print0 2>/dev/null)
    done

    echo "$total $missing"
}

main() {
    echo "Generating architecture reference..."

    local tree
    tree=$(generate_tree)

    {
        generate_header
        if [[ -n "$tree" ]]; then
            printf '%s\n' "$tree"
        fi
        generate_footer
    } > "$OUTPUT_FILE"

    echo -e "${GREEN}Generated:${NC} $OUTPUT_FILE"

    if [[ -z "$tree" ]] && source_dirs_populated; then
        echo -e "${YELLOW}Warning:${NC} project tree is EMPTY while the source directories contain files." >&2
        echo "         The scan matched nothing — check FILE_GLOBS and SOURCE_DIRS in this script." >&2
    fi

    local stats
    stats=$(count_stats)
    local total="${stats% *}"
    local missing="${stats#* }"

    echo ""
    echo "Stats: $total files, $missing without summary"

    if [[ $missing -gt 0 ]]; then
        echo -e "${YELLOW}Tip:${NC} Add a top-of-file comment block to describe the file/class purpose"
    fi
}

main "$@"
