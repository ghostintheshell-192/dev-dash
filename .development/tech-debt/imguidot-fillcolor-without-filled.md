---
type: bug
priority: low
status: open
discovered: 2026-10-01
resolved: null
related: [imguidot-line-styles-ignored.md]
related_decision: null
report: null
upstream: ImGuiDot
upstream_link: null
---

# ImGuiDot: `fillcolor` applied without `style=filled`

## Problem

In Graphviz a node is filled only when its `style` includes `filled`;
`fillcolor` alone does nothing. ImGuiDot fills every node whose `fillcolor`
(inherited from the graph's node defaults too) is visible. In the code graph
the ghost nodes inherit the fill of the classes and look filled, unlike in
Graphviz.

## Analysis

`DrawNodes` reads `fillcolor` and fills when the colour is not transparent,
without looking at `style`. Same root as the ignored line styles: `style` is
never parsed.

## Possible Solutions

- **Option A**: fill only when `style` contains `filled` (Graphviz rule),
  keeping the style colour for `ShapeBackground` as the default fill colour.

## Recommended Approach

Option A, together with the line styles: both need the same parsing of
`style`.
