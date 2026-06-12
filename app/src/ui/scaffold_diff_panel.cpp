#include "scaffold_diff_panel.h"
#include "document_panel_host.h"
#include "status_sink.h"
#include "theme.h"
#include "../services/scaffold_repository.h"
#include "../services/diff_engine.h"
#include "../services/promote_engine.h"
#include "../services/apply_engine.h"
#include "../services/snapshot_service.h"

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
            // Standard automation entry points (ADR-012): part of the
            // project's configuration identity, carried by scaffolds.
            if (rel.size() > 23
                && rel.compare(0, 23, ".development/automation") == 0) return true;
            return false;
        }

        // ── Section grouping ─────────────────────────────────────────────────
        enum class ScaffoldSection
        {
            kClaudeMd, kRules, kSkills, kCommands, kAgents, kSettings,
            kHooks, kAutomation, kOther
        };

        ScaffoldSection ClassifyPath(const std::string& rel)
        {
            if (rel == "CLAUDE.md" || rel == "CLAUDE.local.md"
                || rel == ".claude/CLAUDE.md")
                return ScaffoldSection::kClaudeMd;
            if (rel.size() > 14 && rel.compare(0, 14, ".claude/rules/")  == 0)
                return ScaffoldSection::kRules;
            if (rel.size() > 15 && rel.compare(0, 15, ".claude/skills/") == 0)
                return ScaffoldSection::kSkills;
            if (rel.size() > 17 && rel.compare(0, 17, ".claude/commands/") == 0)
                return ScaffoldSection::kCommands;
            if (rel.size() > 15 && rel.compare(0, 15, ".claude/agents/") == 0)
                return ScaffoldSection::kAgents;
            if (rel == ".claude/settings.json" || rel == ".claude/settings.local.json")
                return ScaffoldSection::kSettings;
            if (rel.size() > 10 && rel.compare(0, 10, ".githooks/") == 0)
                return ScaffoldSection::kHooks;
            if (rel.size() > 24 && rel.compare(0, 24, ".development/automation/") == 0)
                return ScaffoldSection::kAutomation;
            return ScaffoldSection::kOther;
        }

        const char* SectionLabel(ScaffoldSection s)
        {
            switch (s)
            {
            case ScaffoldSection::kClaudeMd:   return "CLAUDE.md";
            case ScaffoldSection::kRules:      return "Rules";
            case ScaffoldSection::kSkills:     return "Skills";
            case ScaffoldSection::kCommands:   return "Commands";
            case ScaffoldSection::kAgents:     return "Agents";
            case ScaffoldSection::kSettings:   return "Settings";
            case ScaffoldSection::kHooks:      return "Git hooks";
            case ScaffoldSection::kAutomation: return "Automation entry points";
            case ScaffoldSection::kOther:      return "Other";
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
            const Theme& t = CurrentTheme();
            switch (kind)
            {
            case core::DiffKind::kMissing:   return t.external;
            case core::DiffKind::kModified:  return t.modified;
            case core::DiffKind::kUnchanged: return t.textDim;
            case core::DiffKind::kCustom:    return t.info;
            }
            return t.text;
        }

        bool IsPromotable(core::DiffKind kind)
        {
            return kind == core::DiffKind::kModified || kind == core::DiffKind::kCustom;
        }

        // Section header row spanning all columns.
        void RenderSectionHeader(const char* label)
        {
            const ImU32 bg = ImGui::GetColorU32(CurrentTheme().sectionBg);
            ImGui::TableNextRow();
            for (int col = 0; col < 4; ++col)
            {
                ImGui::TableSetColumnIndex(col);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, bg);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, bg);
            }
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("%s", label);
        }

        // Render a single selectable file path cell; returns true if clicked.
        // The tooltip shows the absolute path, so every row says where the
        // file physically lives (scaffold dir vs project dir).
        bool FileCell(int id, const char* label, bool dim,
                      const std::filesystem::path& fullPath)
        {
            bool clicked = false;
            ImGui::PushID(id);
            if (dim) ImGui::PushStyleColor(ImGuiCol_Text, CurrentTheme().textDim);
            clicked = ImGui::Selectable(label, false, ImGuiSelectableFlags_None);
            if (dim) ImGui::PopStyleColor();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("%s", fullPath.string().c_str());
            ImGui::PopID();
            return clicked;
        }
    }

    // ── Constructor ───────────────────────────────────────────────────────────

    ScaffoldDiffPanel::ScaffoldDiffPanel(services::ScaffoldRepository& repo,
                                         services::DiffEngine&         diffEngine,
                                         services::PromoteEngine&      promoteEngine,
                                         services::ApplyEngine&        applyEngine,
                                         services::SnapshotService&    snapshotService,
                                         DocumentPanelHost&            docHost,
                                         StatusSink&                   status,
                                         const core::Project&          project)
        : _repo(repo)
        , _diffEngine(diffEngine)
        , _promoteEngine(promoteEngine)
        , _applyEngine(applyEngine)
        , _snapshotService(snapshotService)
        , _docHost(docHost)
        , _status(status)
        , _project(project)
    {
    }

    void ScaffoldDiffPanel::SelectScaffold(const std::string& name)
    {
        if (_needsRefresh)
        {
            Refresh();
            _needsRefresh = false;
        }
        for (int i = 0; i < static_cast<int>(_scaffolds.size()); ++i)
            if (_scaffolds[i].name == name)
            {
                _selectedIdx = i;
                _selectedForPromote.clear();
                RunDiff();
                break;
            }
    }

    // ── Render ────────────────────────────────────────────────────────────────

    void ScaffoldDiffPanel::Render(bool* open)
    {
        if (_needsRefresh)
        {
            Refresh();
            _needsRefresh = false;
        }

        if (!ImGui::Begin("Scaffold diff", open))
        {
            ImGui::End();
            return;
        }

        // ── Toolbar ───────────────────────────────────────────────────────────
        if (_scaffolds.empty())
        {
            ImGui::TextDisabled("No scaffolds found in: %s",
                                _repo.ScaffoldRoot().string().c_str());
            ImGui::TextDisabled("Create one in the Scaffolds view first.");
            if (ImGui::Button("Refresh"))
            {
                _repo.Refresh();
                _needsRefresh = true;
            }
            ImGui::End();
            return;
        }

        // Scaffold picker. Labelled and width-bounded so it reads as a
        // selector, not as a static caption (2026-06-11 dogfooding finding).
        ImGui::TextUnformatted("Scaffold:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(240.0f);
        const std::string comboPreview = _scaffolds[_selectedIdx].isDefault
            ? _scaffolds[_selectedIdx].name + "  (default)"
            : _scaffolds[_selectedIdx].name;
        if (ImGui::BeginCombo("##scaffold_pick", comboPreview.c_str()))
        {
            for (int i = 0; i < static_cast<int>(_scaffolds.size()); ++i)
            {
                if (ImGui::Selectable(_scaffolds[i].name.c_str(), i == _selectedIdx))
                {
                    _selectedIdx = i;
                    _selectedForPromote.clear();
                    RunDiff();
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                    ImGui::SetTooltip("%s", _scaffolds[i].path.string().c_str());
                if (_scaffolds[i].isDefault)
                {
                    ImGui::SameLine();
                    ImGui::TextColored(CurrentTheme().accent, "(default)");
                }
                if (i == _selectedIdx)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
            ImGui::SetTooltip("%s", _scaffolds[_selectedIdx].path.string().c_str());

        ImGui::SameLine();
        ImGui::TextDisabled("vs");
        ImGui::SameLine();
        ImGui::TextDisabled("%s", _project.path.string().c_str());

        // Right-aligned: Apply + Promote + Refresh
        const float rightEdge = ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX();
        const float refreshW  = 70.0f;
        const float promoteW  = _selectedForPromote.empty() ? 0.0f : 160.0f;
        const float promoteSpacing = _selectedForPromote.empty() ? 0.0f : 8.0f;

        // Count applyable files (missing + modified)
        int applyableCount = 0;
        for (const auto& e : _diff)
            if (e.kind == core::DiffKind::kMissing || e.kind == core::DiffKind::kModified)
                ++applyableCount;
        const float applyW       = applyableCount ? 80.0f : 0.0f;
        const float applySpacing = applyableCount ? 8.0f  : 0.0f;

        ImGui::SameLine(rightEdge - refreshW - promoteSpacing - promoteW
                                  - applySpacing - applyW);

        if (applyableCount)
        {
            if (ImGui::Button("Apply..."))
                _showApplyConfirm = true;
            ImGui::SameLine();
        }

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

        const auto& scaffold = _scaffolds[_selectedIdx];

        // Column headers name the actual locations, so each side of a row
        // says where the file physically lives (2026-06-11 dogfooding
        // finding: scaffold vs project was ambiguous).
        const std::string scaffoldHeader = "Scaffold: " + scaffold.name;
        const std::string projectHeader  =
            "Project: " + _project.path.filename().string();

        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn(scaffoldHeader.c_str(), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Status",   ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(projectHeader.c_str(), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##promo",  ImGuiTableColumnFlags_WidthFixed, 24.0f);
        ImGui::TableHeadersRow();

        constexpr ScaffoldSection kAllSections[] = {
            ScaffoldSection::kClaudeMd,
            ScaffoldSection::kRules,
            ScaffoldSection::kSkills,
            ScaffoldSection::kCommands,
            ScaffoldSection::kAgents,
            ScaffoldSection::kSettings,
            ScaffoldSection::kHooks,
            ScaffoldSection::kAutomation,
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
                    if (FileCell(rowId * 3, entry.relativePath.c_str(), dimmed,
                                 scaffold.path / entry.relativePath))
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
                    if (FileCell(rowId * 3 + 1, entry.relativePath.c_str(), dimmed,
                                 _project.path / entry.relativePath))
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
        RenderApplyConfirmModal();

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
            const Theme& t = CurrentTheme();

            // The destination scaffold is the decision being confirmed —
            // name it loudly (2026-06-11 dogfooding finding: two promotes
            // landed on the wrong scaffold unnoticed).
            ImGui::Text("Copy %d file(s) from project to scaffold",
                        static_cast<int>(_selectedForPromote.size()));
            ImGui::SameLine();
            ImGui::TextColored(t.accent, "%s", scaffold.name.c_str());
            if (scaffold.isDefault)
            {
                ImGui::SameLine();
                ImGui::TextDisabled("(default)");
            }
            ImGui::TextDisabled("  %s", scaffold.path.string().c_str());
            ImGui::Spacing();

            for (const auto& rel : _selectedForPromote)
                ImGui::BulletText("%s", rel.c_str());

            ImGui::Spacing();
            ImGui::TextColored(t.modified,
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
                    _status.Set(StatusSink::Level::kSuccess,
                                "Promoted " + std::to_string(result.copiedCount)
                                    + " file(s) to scaffold \"" + scaffold.name + "\".");
                }
                else
                {
                    std::string msg = "Promote to \"" + scaffold.name
                                    + "\" completed with errors:";
                    for (const auto& e : result.errors)
                        msg += "  " + e;
                    _status.Set(StatusSink::Level::kError, std::move(msg));
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

    // ── Apply confirm modal ───────────────────────────────────────────────────

    void ScaffoldDiffPanel::RenderApplyConfirmModal()
    {
        if (_showApplyConfirm)
        {
            ImGui::OpenPopup("Apply scaffold to project?");
            _showApplyConfirm = false;
        }

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Apply scaffold to project?", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            const auto& scaffold = _scaffolds[_selectedIdx];
            ImGui::Text("Apply scaffold \"%s\" to project:", scaffold.name.c_str());
            ImGui::TextDisabled("  %s", _project.path.string().c_str());
            ImGui::Spacing();

            int count = 0;
            for (const auto& e : _diff)
            {
                if (e.kind != core::DiffKind::kMissing
                    && e.kind != core::DiffKind::kModified) continue;
                ImGui::BulletText("%s  (%s)", e.relativePath.c_str(),
                    e.kind == core::DiffKind::kMissing ? "new" : "overwrite");
                ++count;
            }

            ImGui::Spacing();
            ImGui::TextColored(CurrentTheme().info,
                               "A pre-apply autosnapshot will be created first.");
            ImGui::TextDisabled("Existing project files not in scaffold are untouched.");
            ImGui::Spacing();

            if (ImGui::Button("Apply", ImVec2(120, 0)))
            {
                services::ApplyEngine::ApplyConfig cfg;
                cfg.forceOverwrite     = true;
                cfg.createAutosnapshot = true;
                cfg.autosnapshotAction  = "pre-apply-" + scaffold.name;

                for (const auto& e : _diff)
                    if (e.kind == core::DiffKind::kMissing
                        || e.kind == core::DiffKind::kModified)
                        cfg.filesToApply.push_back(e.relativePath);

                const auto result = _applyEngine.Apply(
                    scaffold.path, _project.path, cfg, &_snapshotService);

                _status.Set(result.failed > 0 ? StatusSink::Level::kError
                                              : StatusSink::Level::kSuccess,
                            "Applied " + std::to_string(result.applied)
                                + ", skipped " + std::to_string(result.skipped)
                                + ", failed " + std::to_string(result.failed) + ".");
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
        const bool firstLoad = _scaffolds.empty();
        _scaffolds = _repo.List();
        if (_selectedIdx >= static_cast<int>(_scaffolds.size()))
            _selectedIdx = 0;

        // On first load, start from the default scaffold instead of
        // whatever sorts first (PROVA1 used to win over dev-dash-standard).
        if (firstLoad)
            for (int i = 0; i < static_cast<int>(_scaffolds.size()); ++i)
                if (_scaffolds[i].isDefault)
                {
                    _selectedIdx = i;
                    break;
                }

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
