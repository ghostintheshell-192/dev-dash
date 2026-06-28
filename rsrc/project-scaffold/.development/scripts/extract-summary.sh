#!/bin/bash
# Extract a short description from a source file, used by generate-architecture.sh
# to annotate the project tree.
#
# Extend the `case` below for your project's language. Shipped handlers:
#   - C# files: the body of the first /// <summary>...</summary> XML doc comment.
#   - C/C++ files (.cpp/.cc/.cxx/.h/.hpp/.hxx): the first contiguous //
#     comment block at the top of the file (file-level attribution / purpose,
#     or class-level "why it exists" comment if no file-level block exists).
# For other languages, add a branch that prints the file's leading doc comment
# (docstring, JSDoc, etc.) on stdout.
#
# Usage: extract-summary.sh <filepath>
# Output: description text on stdout (empty if none found).

filepath="$1"
[[ -f "$filepath" ]] || exit 0

case "$filepath" in
    *.cs)
        awk '
            /\/\/\/ <summary>/ {
                in_summary = 1
                summary = ""
                next
            }
            /\/\/\/ <\/summary>/ {
                in_summary = 0
                next
            }
            in_summary && /\/\/\// {
                line = $0
                gsub(/^[[:space:]]*\/\/\/[[:space:]]*/, "", line)
                if (summary != "") summary = summary " "
                summary = summary line
            }
            /^[[:space:]]*(public|internal|private|protected)?[[:space:]]*(sealed|abstract|static|partial)?[[:space:]]*(class|interface|record|struct|enum)[[:space:]]/ {
                if (summary != "") {
                    print summary
                    exit
                }
            }
        ' "$filepath" 2>/dev/null
        ;;
    *.cpp|*.cc|*.cxx|*.h|*.hpp|*.hxx)
        # C++: first contiguous // (or ///) comment block. Skips blank lines
        # and preprocessor directives before the block; ends at the first
        # non-comment, non-blank, non-preprocessor line after the block opens.
        # Output is emitted once via END (avoids double-print on early exit).
        awk '
            BEGIN { state = "before"; summary = "" }

            # Lines starting with // or /// at any indent are comment lines.
            /^[[:space:]]*\/\// {
                line = $0
                gsub(/^[[:space:]]*\/\/+[[:space:]]*/, "", line)
                state = "in_block"
                if (summary != "") summary = summary " "
                summary = summary line
                next
            }

            # Blank lines: ignored.
            /^[[:space:]]*$/ { next }

            # Preprocessor directives: if we are already in a block, end it.
            # Otherwise (still before the block), ignore them.
            /^[[:space:]]*#/ {
                if (state == "in_block") exit
                next
            }

            # Any other non-comment, non-blank line: end the block (or never started).
            { exit }

            END {
                if (state == "in_block" && summary != "") print summary
            }
        ' "$filepath" 2>/dev/null
        ;;
esac
