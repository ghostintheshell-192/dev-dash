---
type: bug
priority: low
status: open
discovered: 2026-05-09
related: []
related_decision: null
report: null
---

# PreprocessImports does not recognise indented fenced code blocks

## Problem

`Renderer::PreprocessImports` tracks whether the current line is inside a fenced code block to suppress the `@import` → `claudeimport://` transformation when the `@`-line is part of a code example. The detection uses `line.starts_with("```")`, which only matches fences at column 0.

CommonMark allows fenced code blocks to be preceded by up to 3 spaces of indentation (`   ```cpp` is a valid fence). A markdown file with such an indented fence would not toggle the `inCodeBlock` state, and any `@import-like` line within it would be incorrectly transformed into a `claudeimport://` link.

## Analysis

**Location**: `poc/src/renderer.cpp` — `Renderer::PreprocessImports`

In our actual content (CLAUDE.md, ADR, spec files) fences are always at column 0, so this is not a problem in practice today. The issue is theoretical but real per CommonMark spec.

## Possible Solutions

- **Option A**: Strip leading whitespace before the `starts_with("```")` check. Reuse the `firstNonSpace` calculation that already happens later in the loop. Tiny patch, no behaviour change for non-indented fences.
- **Option B**: Switch the preprocessor to use MD4C events instead of line-by-line scanning. Would be CommonMark-correct by construction, but introduces a dependency cycle (preprocessor needs the parser the renderer needs to consume) and significantly larger change.

## Recommended Approach

**Option A** when/if a real document hits this case. The fix is one extra branch in the scan loop. Until then it can stay open as a known edge case — the line-based preprocessor is the simpler tool and we accept its limits.

## Notes

- Discovered while reviewing the imgui_md migration in branch `experiment/imgui-md-migration` (May 2026)
- Same theoretical limitation applies to nested code structures — if CommonMark adds new fence-like syntax in the future, line-based scanning will need updates
