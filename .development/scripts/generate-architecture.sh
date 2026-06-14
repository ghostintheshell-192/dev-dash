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
# Modify this section for each project.

PROJECT_NAME="DevDash"
FILE_GLOBS=("*.cpp" "*.cc" "*.cxx" "*.h" "*.hpp" "*.hxx")
SKIP_FILES=()
EXCLUDE_DIRS=("obj" "bin" "build" "external" "cpm-cache" ".git")
EXTRACT_CMD="$SCRIPT_DIR/extract-summary.sh"
MAX_DESC_LENGTH=200
DOCS_REF="\`docs/\`"

# Source directories to scan.
# poc/src/ — original PoC (reference, kept intact post-refactor)
# app/src/ — layered skeleton (real project, post ADR-010 split)
SOURCE_DIRS=("$PROJECT_ROOT/poc/src" "$PROJECT_ROOT/app/src")
# Base for relative path calculation (the printed "### dirname" headers).
REL_BASE="$PROJECT_ROOT"

# Project-specific header content (Layer Overview block).
generate_project_header() {
    cat << 'EOF'
## Layer Overview (`app/src/` — layered architecture, ADR-010)

| Layer | Path | Purpose |
|-------|------|---------|
| Entry point | `app/src/main.cpp` | `int main` → `dev_dash::app::App().Run()` |
| Composition root | `app/src/app/app.{h,cpp}` | `App` owns all layers via `unique_ptr`; wires construction order and main loop. |
| Platform | `app/src/platform/` | SDL3/Vulkan/ImGui plumbing: `SdlSession`, `Window`, `VulkanContext`, `Swapchain`, `FrameResources`, `ImGuiBackend`, `DeletionQueue`. |
| UI | `app/src/ui/` | `FontLibrary`, `MarkdownRenderer` (derives `imgui_md`), `DocumentPanelHost`. |
| Services | `app/src/services/` | Domain logic: `DocumentLoader` (fully implemented); stubs for `ConfigResolver`, `DiffEngine`, `ApplyEngine`, `SnapshotService`, `ScaffoldRepository`. |
| Core | `app/src/core/` | Pure value types (header-only): `Project`, `ConfigLayer`, `EffectiveConfig`, `Scaffold`, `Snapshot`, `DiffEntry`. |
| PoC (reference) | `poc/src/` | Original monolithic `Renderer` class — kept as reference pre-refactor. |
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

# Build a `find` -name argument list with -o between each pattern.
# Usage: find ... \( $(build_find_name_args) \) ...
build_find_name_args() {
    local first=1
    for glob in "${FILE_GLOBS[@]}"; do
        if [ $first -eq 1 ]; then
            printf -- '-name %s ' "$glob"
            first=0
        else
            printf -- '-o -name %s ' "$glob"
        fi
    done
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
        summary=$(grep -m1 "^\*\*Sommario\*\*:" "$adr" | sed 's/^\*\*Sommario\*\*:[[:space:]]*//')
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
    local find_args
    find_args=$(build_find_name_args)
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(eval "find \"$dir\" -maxdepth 1 -type f \\( $find_args \\) -print0 2>/dev/null" | sort -z)

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
    local find_args
    find_args=$(build_find_name_args)

    for source_dir in "${SOURCE_DIRS[@]}"; do
        [[ -d "$source_dir" ]] || continue

        # Build excluded-dirs prune args
        local prune_args=""
        for excl in "${EXCLUDE_DIRS[@]}"; do
            prune_args+=" -path '*/${excl}/*' -prune -o"
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
        done < <(eval "find \"$source_dir\" $prune_args \\( $find_args \\) -type f -print0 2>/dev/null")
    done

    echo "$total $missing"
}

main() {
    echo "Generating architecture reference..."

    {
        generate_header
        generate_tree
        generate_footer
    } > "$OUTPUT_FILE"

    echo -e "${GREEN}Generated:${NC} $OUTPUT_FILE"

    local stats
    stats=$(count_stats)
    local total="${stats% *}"
    local missing="${stats#* }"

    echo ""
    echo "Stats: $total files, $missing without summary"

    if [[ $missing -gt 0 ]]; then
        echo -e "${YELLOW}Tip:${NC} Add a top-of-file // comment block to describe the file/class purpose"
    fi
}

main "$@"
