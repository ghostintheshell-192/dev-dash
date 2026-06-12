#include "file_diff_panel.h"
#include "theme.h"

#include <fstream>
#include <sstream>

#include <imgui.h>

namespace dev_dash::ui
{
    namespace
    {
        std::string ReadFile(const std::filesystem::path& path)
        {
            std::ifstream f(path);
            if (!f) return {};
            std::ostringstream ss;
            ss << f.rdbuf();
            return ss.str();
        }
    }

    void FileDiffPanel::Open(const std::filesystem::path& pathA,
                              const std::string&            labelA,
                              const std::filesystem::path& pathB,
                              const std::string&            labelB)
    {
        _pathA  = pathA;
        _pathB  = pathB;
        _labelA = labelA;
        _labelB = labelB;

        // Fixed ImGui ID so position/size persist across different files.
        _windowTitle = pathA.filename().string() + "##diff_viewer";
        _open = true;

        const auto contentA = ReadFile(pathA);
        const auto contentB = ReadFile(pathB);

        _diff     = core::DiffLines(contentA, contentB);
        _tooLarge = _diff.empty() && (!contentA.empty() || !contentB.empty());

        _addedCount = _removedCount = 0;
        for (const auto& line : _diff)
        {
            if (line.kind == core::DiffLineKind::kAdded)   ++_addedCount;
            if (line.kind == core::DiffLineKind::kRemoved) ++_removedCount;
        }
    }

    void FileDiffPanel::Render()
    {
        if (!_open) return;

        ImGui::SetNextWindowSize(ImVec2(820, 620), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(_windowTitle.c_str(), &_open))
        {
            ImGui::End();
            return;
        }

        // ── Header: a legend in words, not just git jargon ────────────────────
        const Theme& theme = CurrentTheme();
        const float  sw    = ImGui::GetTextLineHeight();
        constexpr ImGuiColorEditFlags kSwatchFlags =
            ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop;

        if (_tooLarge)
        {
            ImGui::TextDisabled("File too large for inline diff (> 2000 lines each).");
            ImGui::End();
            return;
        }
        if (_addedCount == 0 && _removedCount == 0)
        {
            ImGui::TextDisabled("Files are identical.");
            ImGui::End();
            return;
        }

        ImGui::ColorButton("##sw_removed", theme.removed, kSwatchFlags,
                           ImVec2(sw, sw));
        ImGui::SameLine();
        ImGui::Text("%d line(s) only in %s", _removedCount, _labelA.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("%s", _pathA.string().c_str());

        ImGui::ColorButton("##sw_added", theme.added, kSwatchFlags,
                           ImVec2(sw, sw));
        ImGui::SameLine();
        ImGui::Text("%d line(s) only in %s", _addedCount, _labelB.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("%s", _pathB.string().c_str());

        ImGui::Spacing();
        ImGui::Separator();

        // ── Diff lines: full-row background tint does the talking, the
        //    -/+ gutter stays for the git-trained eye ──────────────────────────
        ImGui::BeginChild("##diff_lines", ImVec2(0, 0), ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar);

        const ImU32 removedBg =
            ImGui::GetColorU32({theme.removed.x, theme.removed.y,
                                theme.removed.z, 0.16f});
        const ImU32 addedBg =
            ImGui::GetColorU32({theme.added.x, theme.added.y,
                                theme.added.z, 0.16f});

        const ImGuiTableFlags tableFlags =
            ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX;

        if (ImGui::BeginTable("##diff_table", 2, tableFlags))
        {
            ImGui::TableSetupColumn("##sign", ImGuiTableColumnFlags_WidthFixed,
                                    ImGui::CalcTextSize("+").x + 8.0f);
            ImGui::TableSetupColumn("##text", ImGuiTableColumnFlags_WidthStretch);

            for (const auto& line : _diff)
            {
                ImGui::TableNextRow();

                const char* sign  = " ";
                ImVec4      color = theme.textDim;
                switch (line.kind)
                {
                case core::DiffLineKind::kRemoved:
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, removedBg);
                    sign  = "-";
                    color = theme.removed;
                    break;
                case core::DiffLineKind::kAdded:
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, addedBg);
                    sign  = "+";
                    color = theme.added;
                    break;
                case core::DiffLineKind::kContext:
                    break;
                }

                ImGui::TableSetColumnIndex(0);
                ImGui::TextColored(color, "%s", sign);

                ImGui::TableSetColumnIndex(1);
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      line.kind == core::DiffLineKind::kContext
                                          ? theme.textDim
                                          : theme.text);
                ImGui::TextUnformatted(line.text.c_str());
                ImGui::PopStyleColor();
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
        ImGui::End();
    }
}
