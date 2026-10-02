---
type: bug
priority: low
status: open
discovered: 2026-10-01
resolved: null
related: []
related_decision: null
report: null
upstream: ImGuiDot
upstream_link: null
---

# ImGuiDot: reserved space cuts the outer borders

## Problem

`Draw` reserves exactly the diagram's bounding box with `ImGui::Dummy`. The
borders of the outermost nodes lie on that edge, half of the line outside
it: in a scrolling child with no padding the scroll stops on the edge and the
last border stays clipped, visible when zoomed in.

## Analysis

Found in DevDash's code graph at 4x zoom (right and bottom borders). DevDash
works around it giving the canvas the window padding
(`ImGuiChildFlags_AlwaysUseWindowPadding`, `ui/code_graph_panel.cpp`), but
every user of the library meets it.

## Possible Solutions

- **Option A**: reserve the bounding box plus the line thickness (a pixel or
  two) on each side.

## Recommended Approach

Option A: small, local, a PR can go directly. Once merged, the padding in
DevDash can stay (harmless) or go.
