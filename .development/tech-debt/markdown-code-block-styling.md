---
type: feature
priority: low
status: open
discovered: 2026-05-09
related: []
related_decision: null
report: null
---

# Fenced code blocks rendered as flat yellow text — no syntax highlighting

## Problem

`MarkdownRenderer::BLOCK_CODE` currently distinguishes fenced code blocks from body text by pushing a single warm-yellow `ImGuiCol_Text` colour for the whole block. This is enough to make code visually distinct, but it loses every richer cue a developer expects:

- No language-aware syntax colouring (keywords, types, strings, comments, identifiers)
- No background panel — code visually floats inside the body text instead of reading as a contained block
- No monospace font — inherits the proportional default, so column alignment in code (tables, ASCII diagrams, indentation) is broken
- No indent/padding around the block

The PoC content (CLAUDE.md, ADR, spec files) frequently contains code samples in C#, C++, bash, JSON, YAML — they all currently render as flat yellow proportional text.

## Analysis

**Location**: `poc/src/rendering/markdown_r.cpp` — `MarkdownRenderer::BLOCK_CODE`

The base `imgui_md::BLOCK_CODE` does nothing visual; styling is fully delegated to derived classes. The `MD_BLOCK_CODE_DETAIL` parameter exposes the language tag (e.g. `cpp`, `python`) which we currently ignore.

ImGui itself has no syntax highlighter — adding colouring requires either:

- A standalone tokeniser per language we want to support
- A dedicated library (e.g. ImGuiColorTextEdit, tree-sitter binding, or similar)
- Pre-tokenising at preprocessing time (e.g. wrap MD4C output before rendering)

## Possible Solutions

- **Option A — minimal polish (no syntax highlighting)**: keep the colour push, add (a) a faint background rectangle via `GetWindowDrawList()->AddRectFilled` covering the block, (b) load a monospace font and push it inside `BLOCK_CODE`, (c) light indent. No language awareness but a clear visual block. Self-contained, ~30 lines.
- **Option B — language-aware via ImGuiColorTextEdit or similar**: integrate an existing ImGui-friendly editor/viewer that already does tokenisation. Adds a dependency, but earns full syntax colouring across many languages "for free".
- **Option C — tree-sitter integration**: full-blown parser for proper highlighting. Heavyweight but future-proof; only worth it if code rendering becomes a major feature (e.g. browsing source files).

## Recommended Approach

**Option A** when polish is needed. Cheap and immediately useful. **Option B** if the visual experience of reading code in markdown becomes a regular workflow — DevDash is documentation-first, but markdown content frequently embeds code.

Defer **Option C** until and unless DevDash becomes a code-reading tool too, which is currently outside scope (per ADR-006: DevDash is documentation/context, not execution).

## Notes

- Current implementation in `MarkdownRenderer::BLOCK_CODE`: pushes `ImVec4(1.0f, 0.85f, 0.40f, 1.0f)` on enter, pops on leave. Same colour as `SPAN_CODE` for inline code.
- ImGui 1.92's `ImGuiCol_ImageBorder` and similar styling colours might be reused for the code background to keep the palette consistent.
- A monospace font would also be useful for inline code spans, not just blocks.
