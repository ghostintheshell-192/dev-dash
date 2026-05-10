#include "scaffold_diff_panel.h"
#include "document_panel_host.h"
#include "../services/scaffold_repository.h"
#include "../services/diff_engine.h"

#include <algorithm>
#include <imgui.h>

namespace dev_dash::ui
{
    namespace
    {
        // ── Filtering ────────────────────────────────────────────────────────
        // Only compare config-relevant paths: root CLAUDE*.md, .claude/**,
        // .githooks/**. Everything else (source files, build output, docs,
        // git objects…) is intentionally excluded.
        bool IsConfigRelevant(const std::string& rel)
        {
            if (rel == "CLAUDE.md" || rel == "CLAUDE.local.md") return true;
            if (rel.size() > 7  && rel.compare(0, 7,  ".claude")  == 0) return true;
            if (rel.size() > 9  && rel.compare(0, 9,  ".githooks") == 0) return true;
            return false;
        }

        // ── Section grouping ─────────────────────────────────────────────────
        enum class ScaffoldSection
        {
            kClaudeMd, kRules, kSkills, kAgents, kSettings, kHooks, kOther
        };

        ScaffoldSection ClassifyPath(const std::string& rel)
        {
            if (rel == "CLAUDE.md" || rel == "CLAUDE.local.md")
                return ScaffoldSection::kClaudeMd;
            if (rel.size() > 14 && rel.compare(0, 14, ".claude/rules/")  == 0)
                return ScaffoldSection::kRules;
            if (rel.size() > 15 && rel.compare(0, 15, ".claude/skills/") == 0)
                return ScaffoldSection::kSkills;
            if (rel.size() > 15 && rel.compare(0, 15, ".claude/agents/") == 0)
                return ScaffoldSection::kAgents;
            if (rel == ".claude/settings.json" || rel == ".claude/settings.local.json"
                || rel == ".claude/CLAUDE.md"  || rel.compare(0, 7, ".claude") == 0)
                return ScaffoldSection::kOther;
            if (rel.size() > 10 && rel.compare(0, 10, ".githooks/") == 0)
                return ScaffoldSection::kHooks;
            return ScaffoldSection::kOther;
        }

        const char* SectionLabel(ScaffoldSection s)
        {
            switch (s)
            {
            case ScaffoldSection::kClaudeMd: return "CLAUDE.md";
            case ScaffoldSection::kRules:    return "Rules";
            case ScaffoldSection::kSkills:   return "Skills";
            case ScaffoldSection::kAgents:   return "Agents";
            case ScaffoldSection::kSettings: return "Settings";
            case ScaffoldSection::kHooks:    return "Hooks";
            case ScaffoldSection::kOther:    return ".claude/ (other)";
            }
            return "?";
        }

        // ── Status colours ───────────────────────────────────────────────────
        const char* KindLabel(core::DiffKind kind)
        {
            switch (kind)
            {
            case core::DiffKind::kMissing:   return "Missing";
            case core::DiffKind::kModified:  return "Modified";
            case core::DiffKind::kUnchanged: return "Unchanged";
            case core::DiffKind::kCustom:    return "Only in project";
            }
            return "?";
        }

        ImVec4 KindColor(core::DiffKind kind)
        {
            switch (kind)
            {
            case core::DiffKind::kMissing:   return {1.00f, 0.40f, 0.40f, 1.0f};
            case core::DiffKind::kModified:  return {1.00f, 0.80f, 0.20f, 1.0f};
            case core::DiffKind::kUnchanged: return {0.50f, 0.50f, 0.50f, 1.0f};
            case core::DiffKind::kCustom:    return {0.45f, 0.75f, 1.00f, 1.0f};
            }
            return {1.0f, 1.0f, 1.0f, 1.0f};
        }

        // Section header row spanning all columns.
        void RenderSectionHeader(const char* label)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(45, 45, 55, 255));
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(45, 45, 55, 255));
            ImGui::TextDisabled("%s", label);
            ImGui::TableSetColumnIndex(1);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(45, 45, 55, 255));
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(45, 45, 55, 255));
            ImGui::TableSetColumnIndex(2);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(45, 45, 55, 255));
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(45, 45, 55, 255));
        }

        // Render a single selectable file path cell; returns true if clicked.
        bool FileCell(int id, const char* label, bool dim)
        {
            bool clicked = false;
            ImGui::PushID(id);
            if (dim) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            clicked = ImGui::Selectable(label, false, ImGuiSelectableFlags_None);
            if (dim) ImGui::PopStyleColor();
            ImGui::PopID();
            return clicked;
        }
    }

    // ── Constructor ───────────────────────────────────────────────────────────

    ScaffoldDiffPanel::ScaffoldDiffPanel(services::ScaffoldRepository& repo,
                                         services::DiffEngine&         diffEngine,
                                         DocumentPanelHost&            docHost,
                                         const core::Project&          project)
        : _repo(repo)
        , _diffEngine(diffEngine)
        , _docHost(docHost)
        , _project(project)
    {
    }

    // ── Render ────────────────────────────────────────────────────────────────

    void ScaffoldDiffPanel::Render()
    {
        _wantsBack = false;

        if (_needsRefresh)
        {
            Refresh();
            _needsRefresh = false;
        }

        const ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##scaffold_diff",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                         | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // ── Toolbar ───────────────────────────────────────────────────────────
        if (ImGui::Button("<- Back"))
            _wantsBack = true;

        ImGui::SameLine();

        if (_scaffolds.empty())
        {
            ImGui::TextDisabled("No scaffolds found");
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 70.0f + ImGui::GetCursorPosX());
            if (ImGui::Button("Refresh"))
            {
                _repo.Refresh();
                _needsRefresh = true;
            }
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextDisabled("No scaffolds found in: %s",
                                _repo.ScaffoldRoot().string().c_str());
            ImGui::TextDisabled("Create a subdirectory there to add a scaffold.");
            ImGui::End();
            return;
        }

        // Scaffold picker
        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::BeginCombo("##scaffold_pick", _scaffolds[_selectedIdx].name.c_str()))
        {
            for (int i = 0; i < static_cast<int>(_scaffolds.size()); ++i)
            {
                if (ImGui::Selectable(_scaffolds[i].name.c_str(), i == _selectedIdx))
                {
                    _selectedIdx = i;
                    RunDiff();
                }
                if (i == _selectedIdx)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        ImGui::TextDisabled("vs");
        ImGui::SameLine();
        ImGui::TextDisabled("%s", _project.path.string().c_str());

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 70.0f + ImGui::GetCursorPosX());
        if (ImGui::Button("Refresh"))
        {
            _repo.Refresh();
            _needsRefresh = true;
        }

        ImGui::Separator();

        // ── Summary bar ───────────────────────────────────────────────────────
        int counts[4] = {};
        for (const auto& e : _diff)
            ++counts[static_cast<int>(e.kind)];

        ImGui::Spacing();
        bool anyStat = false;
        auto stat = [&](core::DiffKind k, const char* lbl)
        {
            if (!counts[static_cast<int>(k)]) return;
            if (anyStat) ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, KindColor(k));
            ImGui::Text("%d %s", counts[static_cast<int>(k)], lbl);
            ImGui::PopStyleColor();
            anyStat = true;
        };
        stat(core::DiffKind::kMissing,   "missing");
        stat(core::DiffKind::kModified,  "modified");
        stat(core::DiffKind::kCustom,    "only in project");
        stat(core::DiffKind::kUnchanged, "unchanged");
        if (!anyStat) ImGui::TextDisabled("No config files found in scaffold.");
        ImGui::Spacing();

        // ── Three-column table ────────────────────────────────────────────────
        // Scaffold (left) | Status (centre, fixed) | Project (right)
        const ImGuiTableFlags tableFlags =
            ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;

        if (!ImGui::BeginTable("##diff3", 3, tableFlags,
                               ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
        {
            ImGui::End();
            return;
        }

        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Scaffold", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Status",   ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Project",  ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        const auto& scaffold = _scaffolds[_selectedIdx];

        // Group entries by section and render each group.
        constexpr ScaffoldSection kAllSections[] = {
            ScaffoldSection::kClaudeMd,
            ScaffoldSection::kRules,
            ScaffoldSection::kSkills,
            ScaffoldSection::kAgents,
            ScaffoldSection::kSettings,
            ScaffoldSection::kHooks,
            ScaffoldSection::kOther,
        };

        int rowId = 0;
        for (ScaffoldSection sec : kAllSections)
        {
            // Collect entries for this section.
            bool headerShown = false;
            for (const auto& entry : _diff)
            {
                if (ClassifyPath(entry.relativePath) != sec) continue;

                if (!headerShown)
                {
                    RenderSectionHeader(SectionLabel(sec));
                    headerShown = true;
                }

                ImGui::TableNextRow();
                const bool dimmed = (entry.kind == core::DiffKind::kUnchanged);

                // Left cell — scaffold side
                ImGui::TableSetColumnIndex(0);
                const bool hasScaffold = (entry.kind != core::DiffKind::kCustom);
                if (hasScaffold)
                {
                    if (FileCell(rowId * 3, entry.relativePath.c_str(), dimmed))
                        _docHost.OpenPanel(scaffold.path / entry.relativePath);
                }
                else
                {
                    ImGui::TextDisabled("—");
                }

                // Centre cell — status badge
                ImGui::TableSetColumnIndex(1);
                ImGui::PushStyleColor(ImGuiCol_Text, KindColor(entry.kind));
                ImGui::TextUnformatted(KindLabel(entry.kind));
                ImGui::PopStyleColor();

                // Right cell — project side
                ImGui::TableSetColumnIndex(2);
                const bool hasProject = (entry.kind != core::DiffKind::kMissing);
                if (hasProject)
                {
                    if (FileCell(rowId * 3 + 1, entry.relativePath.c_str(), dimmed))
                        _docHost.OpenPanel(_project.path / entry.relativePath);
                }
                else
                {
                    ImGui::TextDisabled("—");
                }

                ++rowId;
            }
        }

        ImGui::EndTable();
        ImGui::End();
    }

    // ── Private ───────────────────────────────────────────────────────────────

    void ScaffoldDiffPanel::Refresh()
    {
        _scaffolds = _repo.List();
        if (_selectedIdx >= static_cast<int>(_scaffolds.size()))
            _selectedIdx = 0;
        _diff.clear();
        if (!_scaffolds.empty())
            RunDiff();
    }

    void ScaffoldDiffPanel::RunDiff()
    {
        if (_scaffolds.empty()) { _diff.clear(); return; }

        const auto& scaffold = _scaffolds[_selectedIdx];
        auto all = _diffEngine.CompareTrees(scaffold.path, _project.path);

        _diff.clear();
        for (auto& e : all)
            if (IsConfigRelevant(e.relativePath))
                _diff.push_back(std::move(e));
    }
}
