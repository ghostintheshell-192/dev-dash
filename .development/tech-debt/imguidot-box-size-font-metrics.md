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

Two causes, one much larger than the other:

1. **Points drawn as pixels** (the bulk of it). Graphviz works in typographic
   points; ImGuiDot converts every geometry to pixels with `PIXEL_PER_PPI`
   (96/72), but drew the text at `label->fontsize * zoom`, the points taken as
   pixels: text 25% smaller than the space measured for it. Fixed on the fork,
   branch `fix/label-font-size` (`c828e70`, one line in `DrawLabel`), which
   DevDash pins. To report upstream as a PR.
2. **Different fonts**. Graphviz measures the labels with its own metrics
   (`fontname`, Times by default, estimated when no font library is built
   in), ImGuiDot draws them with `ImGui::GetIO().Fonts->Fonts[0]`. What is
   left after the fix: some slack in wide boxes, and line spacing a little
   tighter than the box height expects.

## Possible Solutions

- **Point 1**: the PR with `c828e70`, rebased on Dario's main.
- **Point 2, Option A**: give Graphviz the metrics of the ImGui font: a
  text-layout plugin (`gvtextlayout`) that measures with
  `ImFont::CalcTextSizeA`. Exact, but the plugin API is involved.
- **Point 2, Option B**: let the caller set `fontname`/`fontsize` defaults
  closer to the ImGui font. Cheap, approximate.

## Recommended Approach

Point 1 as a PR now: a plain bug, behaviour defined by Graphviz. Point 2 to
discuss with Dario: it is a design choice of his library.
