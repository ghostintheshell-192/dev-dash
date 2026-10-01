---
type: feature
priority: medium
status: open
discovered: 2026-10-01
resolved: null
related: []
related_decision: null
report: null
upstream: ImGuiDot
upstream_link: null
---

# ImGuiDot: `shape=record` not drawn

## Problem

A UML class box has compartments (name / attributes / methods), which DOT
expresses as `shape=record` with a label like `{Name|+ a : int\l|+ Run()\l}`.
ImGuiDot does not draw records, so the code graph falls back to plain boxes
with the members on separate lines (`DiagramOptions::recordShapes = false`)
and loses the compartments.

## Analysis

Graphviz computes the geometry of every field of a record (`field_t`, in
`ND_shape_info`): rectangles, separators, and a text label per field, already
split into lines like the multi-line labels (ImGuiDot PR #18). Drawing a
record means walking the field tree: the separator lines and each field's
lines of text.

## Possible Solutions

- **Option A**: draw records from the `field_t` tree Graphviz fills.
- **Option B**: HTML-like labels instead: more powerful (bold name), much more
  work.

## Recommended Approach

Option A, after the multi-line labels are merged, since it reuses them. Ask
Dario first: it is a sizeable feature of his library.
