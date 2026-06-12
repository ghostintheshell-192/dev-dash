#include "theme.h"

namespace dev_dash::ui
{
    namespace
    {
        constexpr ImVec4 FromHex(unsigned rgb, float alpha = 1.0f)
        {
            return {static_cast<float>((rgb >> 16) & 0xFF) / 255.0f,
                    static_cast<float>((rgb >> 8) & 0xFF) / 255.0f,
                    static_cast<float>(rgb & 0xFF) / 255.0f,
                    alpha};
        }

        constexpr ImVec4 WithAlpha(ImVec4 c, float alpha)
        {
            return {c.x, c.y, c.z, alpha};
        }

        Theme g_current = {};
    }

    const Theme& GrafiteAmbraTheme()
    {
        static const Theme theme = []
        {
            Theme t;

            t.windowBg = FromHex(0x181614);
            t.panelBg  = FromHex(0x1f1d1a);
            t.activeBg = FromHex(0x2a2722);
            t.border   = FromHex(0x353026);

            t.text    = FromHex(0xd8d3c7);
            t.textDim = FromHex(0x8a8478);

            t.accent       = FromHex(0xe0a64e);
            t.accentHover  = FromHex(0xf0b95e);
            t.accentActive = FromHex(0xc9913e);

            t.tableRowBgAlt = FromHex(0x1c1a17);
            t.sectionBg     = FromHex(0x262320);

            t.added    = FromHex(0x8fbf6e);
            t.removed  = FromHex(0xd9776a);
            t.modified = FromHex(0xe0a64e);
            t.external = FromHex(0xb08ad6);
            t.info     = FromHex(0x7fa8c9);

            t.windowRounding = 5.0f;
            t.frameRounding  = 4.0f;
            t.windowPadding  = {12.0f, 12.0f};
            t.framePadding   = {8.0f, 5.0f};
            t.itemSpacing    = {8.0f, 6.0f};
            t.cellPadding    = {6.0f, 3.0f};
            t.readingPadding = {18.0f, 16.0f};

            return t;
        }();
        return theme;
    }

    void ApplyTheme(const Theme& t)
    {
        g_current = t;

        ImGuiStyle& style = ImGui::GetStyle();

        style.WindowRounding    = t.windowRounding;
        style.ChildRounding     = t.windowRounding;
        style.PopupRounding     = t.windowRounding;
        style.FrameRounding     = t.frameRounding;
        style.GrabRounding      = t.frameRounding;
        style.TabRounding       = t.frameRounding;
        style.ScrollbarRounding = 6.0f;
        style.ScrollbarSize     = 12.0f;

        style.WindowPadding = t.windowPadding;
        style.FramePadding  = t.framePadding;
        style.ItemSpacing   = t.itemSpacing;
        style.CellPadding   = t.cellPadding;

        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize  = 0.0f;
        style.PopupBorderSize  = 1.0f;
        style.FrameBorderSize  = 0.0f;

        ImVec4* c = style.Colors;

        c[ImGuiCol_Text]         = t.text;
        c[ImGuiCol_TextDisabled] = t.textDim;

        c[ImGuiCol_WindowBg] = t.windowBg;
        c[ImGuiCol_ChildBg]  = {0, 0, 0, 0};
        c[ImGuiCol_PopupBg]  = t.panelBg;
        c[ImGuiCol_Border]   = t.border;
        c[ImGuiCol_BorderShadow] = {0, 0, 0, 0};

        c[ImGuiCol_FrameBg]        = t.activeBg;
        c[ImGuiCol_FrameBgHovered] = t.border;
        c[ImGuiCol_FrameBgActive]  = WithAlpha(t.accentActive, 0.40f);

        c[ImGuiCol_TitleBg]          = t.panelBg;
        c[ImGuiCol_TitleBgActive]    = t.activeBg;
        c[ImGuiCol_TitleBgCollapsed] = t.panelBg;
        c[ImGuiCol_MenuBarBg]        = t.panelBg;

        c[ImGuiCol_ScrollbarBg]          = {0, 0, 0, 0};
        c[ImGuiCol_ScrollbarGrab]        = t.activeBg;
        c[ImGuiCol_ScrollbarGrabHovered] = t.border;
        c[ImGuiCol_ScrollbarGrabActive]  = WithAlpha(t.accent, 0.60f);

        c[ImGuiCol_CheckMark]        = t.accent;
        c[ImGuiCol_SliderGrab]       = t.accent;
        c[ImGuiCol_SliderGrabActive] = t.accentHover;

        c[ImGuiCol_Button]        = t.activeBg;
        c[ImGuiCol_ButtonHovered] = t.border;
        c[ImGuiCol_ButtonActive]  = WithAlpha(t.accentActive, 0.50f);

        c[ImGuiCol_Header]        = WithAlpha(t.activeBg, 0.80f);
        c[ImGuiCol_HeaderHovered] = t.activeBg;
        c[ImGuiCol_HeaderActive]  = WithAlpha(t.accentActive, 0.40f);

        c[ImGuiCol_Separator]        = t.border;
        c[ImGuiCol_SeparatorHovered] = WithAlpha(t.accent, 0.50f);
        c[ImGuiCol_SeparatorActive]  = t.accent;

        c[ImGuiCol_ResizeGrip]        = WithAlpha(t.border, 0.50f);
        c[ImGuiCol_ResizeGripHovered] = WithAlpha(t.accent, 0.50f);
        c[ImGuiCol_ResizeGripActive]  = t.accent;

        c[ImGuiCol_Tab]                = t.panelBg;
        c[ImGuiCol_TabHovered]         = t.activeBg;
        c[ImGuiCol_TabSelected]        = t.activeBg;
        c[ImGuiCol_TabDimmed]          = t.windowBg;
        c[ImGuiCol_TabDimmedSelected]  = t.panelBg;
        c[ImGuiCol_TabSelectedOverline] = t.accent;

        c[ImGuiCol_TableHeaderBg]     = t.panelBg;
        c[ImGuiCol_TableBorderStrong] = t.border;
        c[ImGuiCol_TableBorderLight]  = WithAlpha(t.border, 0.50f);
        c[ImGuiCol_TableRowBg]        = {0, 0, 0, 0};
        c[ImGuiCol_TableRowBgAlt]     = t.tableRowBgAlt;

        c[ImGuiCol_TextSelectedBg] = WithAlpha(t.accent, 0.35f);
        c[ImGuiCol_DragDropTarget] = t.accent;
        c[ImGuiCol_NavCursor]      = t.accent;

        c[ImGuiCol_NavWindowingHighlight] = WithAlpha(t.accent, 0.70f);
        c[ImGuiCol_NavWindowingDimBg]     = {0.10f, 0.09f, 0.08f, 0.60f};
        c[ImGuiCol_ModalWindowDimBg]      = {0.06f, 0.05f, 0.05f, 0.55f};

        c[ImGuiCol_PlotLines]            = t.textDim;
        c[ImGuiCol_PlotLinesHovered]     = t.accentHover;
        c[ImGuiCol_PlotHistogram]        = t.accent;
        c[ImGuiCol_PlotHistogramHovered] = t.accentHover;
    }

    const Theme& CurrentTheme()
    {
        return g_current;
    }
}
