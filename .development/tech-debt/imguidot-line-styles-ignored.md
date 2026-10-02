---
type: feature
priority: medium
status: open
discovered: 2026-10-01
resolved: null
related: [imguidot-fillcolor-without-filled.md]
related_decision: null
report: null
upstream: ImGuiDot
upstream_link: null
---

# ImGuiDot: `style=dashed` and `style=dotted` ignored

## Problem

The code graph writes UML dependencies as dashed edges, uncertain relations
as dotted ones, and the neighbours of the selection ("ghosts") with a dashed
border. ImGuiDot draws them all with solid lines, so dependency and
association look the same, and ghosts differ from the selected classes only
by colour.

## Analysis

ImGuiDot never reads the `style` attribute of nodes or edges: lines are
always drawn with `AddLine`/`AddBezierCubic`/`AddPolyline`. The ImGui draw
list has no dash pattern, so dashes have to be built as segments along the
path (straight lines and Bézier curves alike).

## Possible Solutions

- **Option A**: support `dashed`, `dotted` (and `solid`, `invis`) on edges and
  node borders, splitting each path into segments by arc length.
- **Option B**: edges only first, as they carry the UML meaning.

## Recommended Approach

Option A, behaviour defined by Graphviz, so no API design is needed: a
contribution that can go as a PR directly.
