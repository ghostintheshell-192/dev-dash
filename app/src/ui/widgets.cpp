#include "widgets.h"
#include "status_sink.h"
#include "theme.h"

namespace dev_dash::ui
{
    void StatusBadge(const char* label, const ImVec4& color)
    {
        const ImVec2 textSize = ImGui::CalcTextSize(label);
        const ImVec2 padding{8.0f, 2.0f};
        const ImVec2 pos  = ImGui::GetCursorScreenPos();
        const ImVec2 size{textSize.x + padding.x * 2.0f,
                          textSize.y + padding.y * 2.0f};

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(
            pos, {pos.x + size.x, pos.y + size.y},
            ImGui::GetColorU32({color.x, color.y, color.z, 0.16f}),
            size.y * 0.5f);
        drawList->AddText({pos.x + padding.x, pos.y + padding.y},
                          ImGui::GetColorU32(color), label);

        ImGui::Dummy(size);
    }

    void StatusHint(StatusSink& status, std::string text)
    {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            status.SetHint(std::move(text));
    }

    void HelpMarker(StatusSink& status, const char* text)
    {
        ImGui::TextDisabled("(?)");
        StatusHint(status, text);
    }

    bool GhostButton(const char* label)
    {
        const Theme& t = CurrentTheme();
        ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, t.activeBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              {t.accentActive.x, t.accentActive.y,
                               t.accentActive.z, 0.40f});
        ImGui::PushStyleColor(ImGuiCol_Text, t.textDim);
        const bool clicked = ImGui::SmallButton(label);
        ImGui::PopStyleColor(4);
        return clicked;
    }
}
