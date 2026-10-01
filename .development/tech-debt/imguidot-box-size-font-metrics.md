---
type: bug
priority: medium
status: open
discovered: 2026-10-01
resolved: null
related: [imguidot-diagram-size-api.md]
related_decision: null
report: null
upstream: ImGuiDot
upstream_link: null
---

# ImGuiDot: node boxes far larger than their text

## Problem

In the code graph every box is much wider and taller than the text it holds:
a class box with five members, at zoom 1, has roughly as much empty space as
text. It is the most visible defect of the diagrams in DevDash.

## Analysis

Graphviz lays out the graph measuring the labels with its own text metrics
(the font named by `fontname`, Times by default, estimated when no font
library is built in) and sizes the nodes from them. ImGuiDot then draws the
labels with `ImGui::GetIO().Fonts->Fonts[0]` at the layout's `fontsize`: a
different font with different widths. The boxes keep the size Graphviz gave
them, the text drawn inside is smaller.

## Possible Solutions

- **Option A**: give Graphviz the metrics of the ImGui font: a text-layout
  plugin (`gvtextlayout`) that measures with `ImFont::CalcTextSizeA`. Exact,
  but the plugin API is involved.
- **Option B**: scale the drawn text to the width Graphviz expects for each
  label (`textspan_t::size`). Simple; text size then varies a little between
  labels.
- **Option C**: let the caller set `fontname`/`fontsize` defaults that match
  the ImGui font closely enough. Cheapest, approximate.

## Recommended Approach

To be discussed with Dario: it is a design choice of his library. Option A
is the clean one if the plugin API allows it.
