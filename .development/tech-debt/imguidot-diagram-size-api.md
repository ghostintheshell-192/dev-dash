---
type: feature
priority: low
status: open
discovered: 2026-10-01
resolved: null
related: [imguidot-box-size-font-metrics.md]
related_decision: null
report: null
upstream: ImGuiDot
upstream_link: null
---

# ImGuiDot: no API for the size of a diagram

## Problem

To fit a diagram to the view the caller needs its size. ImGuiDot does not
expose it: DevDash's "Fit" draws the diagram once at the current zoom with
pivot 0 and reads the size of the item `Draw` reserves
(`ImGui::GetItemRectSize()`), then corrects the zoom on the next frame.

## Analysis

The size is the graph's bounding box (`GD_bb`) in points, converted to pixels
— what `Draw` already computes internally.

## Possible Solutions

- **Option A**: `ImVec2 GetSize(const DiagramState&, float zoom = 1.0f)`.

## Recommended Approach

A new API: ask Dario before writing it (ADR-014, contributions upstream).
Pairs naturally with the hit-testing API the code graph needs for phase 4.
