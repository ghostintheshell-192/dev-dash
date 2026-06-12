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

        // ── Header ────────────────────────────────────────────────────────────
        const Theme& theme = CurrentTheme();

        ImGui::PushStyleColor(ImGuiCol_Text, theme.removed);
        ImGui::Text("- %s", _labelA.c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("%s", _pathA.string().c_str());

        ImGui::PushStyleColor(ImGuiCol_Text, theme.added);
        ImGui::Text("+ %s", _labelB.c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("%s", _pathB.string().c_str());

        // ── Summary ───────────────────────────────────────────────────────────
        ImGui::Spacing();
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

        ImGui::PushStyleColor(ImGuiCol_Text, theme.removed);
        ImGui::Text("-%d", _removedCount);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, theme.added);
        ImGui::Text("+%d", _addedCount);
        ImGui::PopStyleColor();

        ImGui::Separator();

        // ── Diff lines ────────────────────────────────────────────────────────
        ImGui::BeginChild("##diff_lines", ImVec2(0, 0), false,
                          ImGuiWindowFlags_HorizontalScrollbar);

        for (const auto& line : _diff)
        {
            switch (line.kind)
            {
            case core::DiffLineKind::kRemoved:
                ImGui::PushStyleColor(ImGuiCol_Text, theme.removed);
                ImGui::TextUnformatted(("- " + line.text).c_str());
                ImGui::PopStyleColor();
                break;

            case core::DiffLineKind::kAdded:
                ImGui::PushStyleColor(ImGuiCol_Text, theme.added);
                ImGui::TextUnformatted(("+ " + line.text).c_str());
                ImGui::PopStyleColor();
                break;

            case core::DiffLineKind::kContext:
                ImGui::PushStyleColor(ImGuiCol_Text, theme.textDim);
                ImGui::TextUnformatted(("  " + line.text).c_str());
                ImGui::PopStyleColor();
                break;
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }
}
