---
type: code-quality
priority: low
status: open
discovered: 2026-10-02
resolved: null
related: []
related_decision: null
report: null
upstream: ImGuiDot
upstream_link: null
---

# ImGuiDot: two leftovers of the style colours (#19)

## Problem

Two leftovers in `DPD85/ImGuiDot` `main` (`98beb2d`, #19 merged), found
re-reading the merge. Neither breaks DevDash today.

1. **Stale default comments.** In `src/ImGuiDot.h`, the `StyleColour_` enum
   still says *"Default: transparent"* for `StyleColour_ShapeBackground` and
   `StyleColour_DiagramBorder`. After `d3b0488` they are Auto:
   `ImGuiCol_FrameBg` and `ImGuiCol_Border`. The Readme table is right, so
   header and Readme contradict each other.
2. **`GetStyleColourU32()` no longer applies the global alpha.** Its doc says
   *"with the global alpha of the ImGui style applied (as ImGui::GetColorU32()
   does)"*. The body became `return GetStyleColour(index);`, the plain
   `ImColor` → `ImU32` conversion, without `style.Alpha`. A diagram inside a
   fading window or a `BeginDisabled()` block does not fade with the rest.

## Possible Solutions

- **Point 1**: fix the two comments (Auto: `ImGuiCol_FrameBg`, Auto:
  `ImGuiCol_Border`).
- **Point 2, Option A**: `return ImGui::GetColorU32(GetStyleColour(index).Value);`.
  Code matches the doc.
- **Point 2, Option B**: if Dario wants the global alpha ignored, fix the doc
  instead.

## Recommended Approach

One small PR to Dario with both points, separate from #18 (multi-line labels)
so the topics stay apart. Point 2: propose A, ask him which he prefers.
