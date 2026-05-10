#include "scaffold_diff_panel.h"
#include "document_panel_host.h"
#include "../services/scaffold_repository.h"
#include "../services/diff_engine.h"
#include "../services/promote_engine.h"

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

        bool IsPromotable(core::DiffKind kind)
        {
            return kind == core::DiffKind::kModified || kind == core::DiffKind::kCustom;
        }

        // Section header row spanning all columns.
        void RenderSectionHeader(const char* label)
        {
            ImGui::TableNextRow();
            for (int col = 0; col < 4; ++col)
            {
                ImGui::TableSetColumnIndex(col);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(45, 45, 55, 255));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(45, 45, 55, 255));
            }
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("%s", label);
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
                                         services::PromoteEngine&      promoteEngine,
                                         DocumentPanelHost&            docHost,
                                         const core::Project&          project)
        : _repo(repo)
        , _diffEngine(diffEngine)
        , _promoteEngine(promoteEngine)
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
                    _selectedForPromote.clear();
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

        // Promote button — right-aligned, visible only when something is selected
        const float rightEdge = ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX();
        const float refreshW  = 70.0f;
        const float promoteW  = _selectedForPromote.empty() ? 0.0f : 160.0f;
        const float spacing   = _selectedForPromote.empty() ? 0.0f : 8.0f;

        ImGui::SameLine(rightEdge - refreshW - spacing - promoteW);

        if (!_selectedForPromote.empty())
        {
            const std::string promoteLabel =
                "Promote (" + std::to_string(_selectedForPromote.size()) + ")";
            if (ImGui::Button(promoteLabel.c_str()))
                _showPromoteConfirm = true;
            ImGui::SameLine();
        }

        if (ImGui::Button("Refresh"))
        {
            _repo.Refresh();
            _selectedForPromote.clear();
            _needsRefresh = true;
        }

        ImGui::Separator();

        // ── Status message ────────────────────────────────────────────────────
        if (!_promoteStatusMsg.empty())
        {
            ImGui::Spacing();
            ImGui::TextUnformatted(_promoteStatusMsg.c_str());
        }

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

        // ── Four-column table ─────────────────────────────────────────────────
        // Scaffold (left) | Status (centre, fixed) | Project (right) | Promote (fixed)
        const ImGuiTableFlags tableFlags =
            ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;

        if (!ImGui::BeginTable("##diff4", 4, tableFlags,
                               ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
        {
            ImGui::End();
            return;
        }

        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Scaffold", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Status",   ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Project",  ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##promo",  ImGuiTableColumnFlags_WidthFixed, 24.0f);
        ImGui::TableHeadersRow();

        const auto& scaffold = _scaffolds[_selectedIdx];

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
                if (entry.kind != core::DiffKind::kCustom)
                {
                    if (FileCell(rowId * 3, entry.relativePath.c_str(), dimmed))
                        _docHost.OpenPanel(scaffold.path / entry.relativePath);
                }
                else
                {
                    ImGui::TextDisabled("—");
                }

                // Centre cell — status badge; Modified is clickable → diff viewer
                ImGui::TableSetColumnIndex(1);
                ImGui::PushStyleColor(ImGuiCol_Text, KindColor(entry.kind));
                if (entry.kind == core::DiffKind::kModified)
                {
                    ImGui::PushID(rowId * 3 + 2);
                    if (ImGui::Selectable(KindLabel(entry.kind)))
                        _diffViewer.Open(
                            scaffold.path / entry.relativePath, scaffold.name,
                            _project.path / entry.relativePath, "Project");
                    ImGui::PopID();
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Click to view diff");
                }
                else
                {
                    ImGui::TextUnformatted(KindLabel(entry.kind));
                }
                ImGui::PopStyleColor();

                // Right cell — project side
                ImGui::TableSetColumnIndex(2);
                if (entry.kind != core::DiffKind::kMissing)
                {
                    if (FileCell(rowId * 3 + 1, entry.relativePath.c_str(), dimmed))
                        _docHost.OpenPanel(_project.path / entry.relativePath);
                }
                else
                {
                    ImGui::TextDisabled("—");
                }

                // Promote column — checkbox for promotable entries
                ImGui::TableSetColumnIndex(3);
                if (IsPromotable(entry.kind))
                {
                    bool checked = _selectedForPromote.count(entry.relativePath) > 0;
                    ImGui::PushID(rowId);
                    if (ImGui::Checkbox("##p", &checked))
                    {
                        _promoteStatusMsg.clear();
                        if (checked)
                            _selectedForPromote.insert(entry.relativePath);
                        else
                            _selectedForPromote.erase(entry.relativePath);
                    }
                    ImGui::PopID();
                }

                ++rowId;
            }
        }

        ImGui::EndTable();

        RenderPromoteConfirmModal();

        ImGui::End();

        _diffViewer.Render();
    }

    // ── Promote confirm modal ─────────────────────────────────────────────────

    void ScaffoldDiffPanel::RenderPromoteConfirmModal()
    {
        if (_showPromoteConfirm)
        {
            ImGui::OpenPopup("Promote to scaffold?");
            _showPromoteConfirm = false;
        }

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Promote to scaffold?", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            const auto& scaffold = _scaffolds[_selectedIdx];
            ImGui::Text("Copy %d file(s) from project to scaffold:",
                        static_cast<int>(_selectedForPromote.size()));
            ImGui::TextDisabled("  %s", scaffold.path.string().c_str());
            ImGui::Spacing();

            for (const auto& rel : _selectedForPromote)
                ImGui::BulletText("%s", rel.c_str());

            ImGui::Spacing();
            ImGui::TextColored({1.0f, 0.8f, 0.2f, 1.0f},
                               "Existing scaffold files will be overwritten.");
            ImGui::Spacing();

            if (ImGui::Button("Promote", ImVec2(120, 0)))
            {
                const std::vector<std::string> paths(
                    _selectedForPromote.begin(), _selectedForPromote.end());

                const auto result = _promoteEngine.Promote(
                    _project.path, scaffold.path, paths);

                if (result.Ok())
                {
                    _promoteStatusMsg = "Promoted " + std::to_string(result.copiedCount)
                                        + " file(s) to scaffold.";
                }
                else
                {
                    _promoteStatusMsg = "Promote completed with errors:";
                    for (const auto& e : result.errors)
                        _promoteStatusMsg += "\n  " + e;
                }

                _selectedForPromote.clear();
                RunDiff();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    // ── Private ───────────────────────────────────────────────────────────────

    void ScaffoldDiffPanel::Refresh()
    {
        _scaffolds = _repo.List();
        if (_selectedIdx >= static_cast<int>(_scaffolds.size()))
            _selectedIdx = 0;
        _diff.clear();
        _selectedForPromote.clear();
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
