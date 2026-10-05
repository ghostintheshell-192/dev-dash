---
captured: 2026-09-26
status: open
context: "review of llm-programming-setup (to be archived) against raid-sandbox and this scaffold; rsrc/project-scaffold/.claude/rules/coding-standards.md"
tags: [scaffold, coding-standards, spec-driven, templating]
---

# Replace the `{LANGUAGE_SPECIFIC_STANDARDS}` slot: coding standards are derived from code, not templated from a language

## What this is

The scaffold's `coding-standards.md` carries a `{LANGUAGE_SPECIFIC_STANDARDS}`
placeholder, meant to be filled from language templates under
`rsrc/legacy-workspace-rules/scaffold-rules/coding-standards/`. That directory
no longer exists in the repo: the slot is orphaned. The templates it pointed to
descend from `llm-programming-setup` (June 2025) — generic, ~200-line manuals
per language (header guards, RAII, PEP 8 import order…).

The proposal is to drop the slot rather than restore the templates, and
replace it with one of — or both of:

1. **A short personal-preferences card per language** (~10 lines): only the
   choices among equally legitimate alternatives that a model cannot guess in
   an empty project — brace style, private-field prefix, method casing,
   constant prefix. Extracted from the two live projects, not from the old
   templates.
2. **A step in the spec-driven flow**: `coding-standards.md` starts near-empty
   (just the card) and, after the first component is implemented, is
   *derived from the code* and fixed — conventions plus the *why* of each.

## Why it deserves attention

Both live projects already did (2) by hand, and neither kept anything from the
templates:

- **dev-dash** (`.claude/rules/coding-standards.md`): naming table with a
  rationale per choice (e.g. the `k` prefix because types and constants
  interleave densely in ImGui/Vulkan code), layer-boundary rules, how to
  re-style code ported from Germen. Nothing of `c-cpp.md`'s 10-chapter layout.
- **raid-sandbox**: IIFE-on-namespace module pattern, factories over classes,
  file headers stating what the file does *not* do. Nothing of
  `javascript-typescript.md`.

Generic language guidance is what the model already knows: injecting it costs
context without changing behaviour. What changes behaviour is what is specific
to the project or to Valentina — and that comes from the code and from her
choices, not from the language. Language-driven filling is the part of the
design that has aged, not just the files.

It also fits dev-dash's purpose as a spec-driven tool: making "derive the
standards once there is code to derive them from" an explicit, visible step is
the same move ADR-015 made for placeholders — show the step instead of leaving
it to be rediscovered.

## Minimal next step

Discuss before touching the scaffold. Open questions:

- Card, flow step, or both? Where does the card live — scaffold, or
  `~/.claude/` (it is personal, not per-project; but ADR-012 wants projects
  self-contained)?
- If it is a flow step: is it a spec convention, a skill, or something dev-dash
  surfaces in the UI (e.g. a `coding-standards.md` flagged "not yet derived")?
- Meanwhile, the scaffold file should at least stop pointing at a directory
  that does not exist.
