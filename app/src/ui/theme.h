#pragma once

#include <imgui.h>

namespace dev_dash::ui
{
    // A complete visual identity: palette + style metrics, applied to the
    // global ImGuiStyle by the composition root. Themes are data, not code:
    // adding a theme means adding another factory function, panels only ever
    // talk to CurrentTheme().
    //
    // Semantic colors follow one rule: one meaning, one color, everywhere.
    // Panels must not hardcode ImVec4 literals for anything covered here.
    struct Theme
    {
        // ── Surfaces ──────────────────────────────────────────────────────
        ImVec4 windowBg;
        ImVec4 panelBg;        // child windows, popups, table headers
        ImVec4 activeBg;       // hovered/active widgets, selected rows
        ImVec4 border;

        // ── Text ──────────────────────────────────────────────────────────
        ImVec4 text;
        ImVec4 textDim;

        // ── Accent (focus, selection, headings, default markers) ─────────
        ImVec4 accent;
        ImVec4 accentHover;
        ImVec4 accentActive;

        // ── Tables ────────────────────────────────────────────────────────
        ImVec4 tableRowBgAlt;  // light zebra striping
        ImVec4 sectionBg;      // full-width section header rows

        // ── Semantic status ───────────────────────────────────────────────
        ImVec4 added;          // new content, success
        ImVec4 removed;        // deleted content, errors
        ImVec4 modified;       // changed content, warnings (= accent family)
        ImVec4 external;       // missing / lives elsewhere (scaffold, global)
        ImVec4 info;           // neutral informational notes

        // ── Metrics ───────────────────────────────────────────────────────
        float windowRounding;
        float frameRounding;
        ImVec2 windowPadding;
        ImVec2 framePadding;
        ImVec2 itemSpacing;
        ImVec2 cellPadding;       // tables stay dense
        ImVec2 readingPadding;    // document/reading panes stay airy
    };

    // The single theme for now ("Grafite & Ambra"). Future themes: add a
    // factory here and a way to pick it; nothing else changes.
    const Theme& GrafiteAmbraTheme();

    // Applies the theme to ImGui::GetStyle() and makes it CurrentTheme().
    // uiScale multiplies every metric (display content scale, ≥ 1) so
    // paddings and roundings keep pace with the DPI-scaled fonts.
    void ApplyTheme(const Theme& theme, float uiScale = 1.0f);

    // The theme last applied. Valid after the first ApplyTheme() call.
    const Theme& CurrentTheme();
}
