#include "scaffold_diff_panel.h"
#include "document_panel_host.h"
#include "status_sink.h"
#include "theme.h"
#include "widgets.h"
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
            case ScaffoldSection::kClaudeMd:   return "CLAUDE.MD";
            case ScaffoldSection::kRules:      return "RULES";
            case ScaffoldSection::kSkills:     return "SKILLS";
            case ScaffoldSection::kCommands:   return "COMMANDS";
            case ScaffoldSection::kAgents:     return "AGENTS";
            case ScaffoldSection::kSettings:   return "SETTINGS";
            case ScaffoldSection::kHooks:      return "GIT HOOKS";
            case ScaffoldSection::kAutomation: return "AUTOMATION ENTRY POINTS";
            case ScaffoldSection::kOther:      return "OTHER";
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

        // Section label row: spaced uppercase accent text — must read as a
        // heading, not as another file name.
        void RenderSectionHeader(const char* label)
        {
            const Theme& t = CurrentTheme();
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(1);
            ImGui::Dummy({0.0f, ImGui::GetTextLineHeight() * 0.4f});
            ImGui::TextColored({t.accent.x, t.accent.y, t.accent.z, 0.85f},
                               "%s", label);
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

        if (!ImGui::Begin("Compare", open))
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
                StatusHint(_status, _scaffolds[i].path.string());
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
        StatusHint(_status, _scaffolds[_selectedIdx].path.string());

        ImGui::SameLine();
        ImGui::TextDisabled("vs");
        ImGui::SameLine();
        ImGui::TextDisabled("%s", _project.path.string().c_str());

        // Right-aligned action cluster: Apply (?) Promote (?) Refresh.
        // Both operations stay visible — greyed out when inapplicable —
        // so the view always shows what it can do.
        constexpr const char* kApplyHelp =
            "Apply copies the scaffold's missing and modified files INTO "
            "the project (a pre-apply snapshot is taken first).";
        constexpr const char* kPromoteHelp =
            "Promote copies the checked project files BACK INTO the "
            "scaffold, updating the template for future projects.";

        int applyableCount = 0;
        for (const auto& e : _diff)
            if (e.kind == core::DiffKind::kMissing || e.kind == core::DiffKind::kModified)
                ++applyableCount;

        const std::string promoteLabel =
            "Promote (" + std::to_string(_selectedForPromote.size()) + ")";

        const ImGuiStyle& style = ImGui::GetStyle();
        const auto buttonW = [&](const char* label)
        { return ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f; };
        const float helpW = ImGui::CalcTextSize("(?)").x;
        const float clusterW =
            buttonW("Apply...") + helpW + buttonW(promoteLabel.c_str()) + helpW
            + buttonW("Refresh") + style.ItemSpacing.x * 4.0f;

        const float rightEdge = ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX();
        ImGui::SameLine(rightEdge - clusterW);

        if (!applyableCount) ImGui::BeginDisabled();
        if (ImGui::Button("Apply..."))
            _showApplyConfirm = true;
        if (!applyableCount) ImGui::EndDisabled();
        ImGui::SameLine();
        HelpMarker(_status, kApplyHelp);
        ImGui::SameLine();

        if (_selectedForPromote.empty()) ImGui::BeginDisabled();
        if (ImGui::Button(promoteLabel.c_str()))
            _showPromoteConfirm = true;
        if (_selectedForPromote.empty()) ImGui::EndDisabled();
        ImGui::SameLine();
        HelpMarker(_status, kPromoteHelp);
        ImGui::SameLine();

        if (ImGui::Button("Refresh"))
        {
            _repo.Refresh();
            _selectedForPromote.clear();
            _needsRefresh = true;
        }

        ImGui::Separator();

        // ── Summary bar: status pills ─────────────────────────────────────────
        int counts[4] = {};
        for (const auto& e : _diff)
            ++counts[static_cast<int>(e.kind)];

        ImGui::Spacing();
        bool anyStat = false;
        auto stat = [&](core::DiffKind k, const char* lbl)
        {
            if (!counts[static_cast<int>(k)]) return;
            if (anyStat) ImGui::SameLine();
            const std::string text =
                std::to_string(counts[static_cast<int>(k)]) + " " + lbl;
            StatusBadge(text.c_str(), KindColor(k));
            anyStat = true;
        };
        stat(core::DiffKind::kMissing,   "missing");
        stat(core::DiffKind::kModified,  "modified");
        stat(core::DiffKind::kCustom,    "only in project");
        if (!anyStat) ImGui::TextDisabled("Everything matches the scaffold.");

        // Unchanged rows are noise on first sight: opt-in.
        if (counts[static_cast<int>(core::DiffKind::kUnchanged)] > 0)
        {
            if (anyStat) ImGui::SameLine();
            const std::string toggleLabel =
                "Show " + std::to_string(
                    counts[static_cast<int>(core::DiffKind::kUnchanged)])
                + " unchanged";
            ImGui::Checkbox(toggleLabel.c_str(), &_showUnchanged);
        }
        ImGui::Spacing();

        // ── File list ─────────────────────────────────────────────────────────
        // One row per file: [select] path | status pill | quiet actions.
        // The path appears once — where the file lives is answered by the
        // action buttons and the tooltip, not by twin columns.
        const ImGuiTableFlags tableFlags =
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 5.0f));
        if (!ImGui::BeginTable("##diff_list", 4, tableFlags,
                               ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
        {
            ImGui::PopStyleVar();
            ImGui::End();
            return;
        }

        const auto& scaffold = _scaffolds[_selectedIdx];

        ImGui::TableSetupColumn("##select",  ImGuiTableColumnFlags_WidthFixed, 26.0f);
        ImGui::TableSetupColumn("##path",    ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##status",  ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("##actions", ImGuiTableColumnFlags_WidthFixed, 190.0f);

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
                if (!_showUnchanged
                    && entry.kind == core::DiffKind::kUnchanged) continue;

                if (!headerShown)
                {
                    RenderSectionHeader(SectionLabel(sec));
                    headerShown = true;
                }

                ImGui::TableNextRow();
                ImGui::PushID(rowId);

                const bool dimmed = (entry.kind == core::DiffKind::kUnchanged);
                const auto scaffoldFile = scaffold.path / entry.relativePath;
                const auto projectFile  = _project.path / entry.relativePath;
                const bool inScaffold   = entry.kind != core::DiffKind::kCustom;
                const bool inProject    = entry.kind != core::DiffKind::kMissing;

                // Path cell first: the row-spanning selectable must be
                // submitted before the widgets that overlap it.
                // Highlight starts at the file name (checkbox stays out),
                // frame-height tall, text vertically centred.
                ImGui::TableSetColumnIndex(1);
                if (dimmed) ImGui::PushStyleColor(ImGuiCol_Text,
                                                  CurrentTheme().textDim);
                ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign,
                                    ImVec2(0.0f, 0.5f));
                const bool rowClicked = ImGui::Selectable(
                    entry.relativePath.c_str(), false,
                    ImGuiSelectableFlags_AllowOverlap,
                    ImVec2(0.0f, ImGui::GetFrameHeight()));
                ImGui::PopStyleVar();
                if (dimmed) ImGui::PopStyleColor();

                if (rowClicked)
                {
                    // Smart default: Modified opens the diff, otherwise
                    // open the file where it exists.
                    if (entry.kind == core::DiffKind::kModified)
                        _diffViewer.Open(scaffoldFile, scaffold.name,
                                         projectFile, "Project");
                    else if (inProject)
                        _docHost.OpenPanel(projectFile);
                    else
                        _docHost.OpenPanel(scaffoldFile);
                }
                StatusHint(_status, "scaffold: " + (inScaffold ? scaffoldFile.string() : std::string("—"))
                                        + "  ·  project: " + (inProject ? projectFile.string() : std::string("—")));

                // Select-for-promote checkbox, leftmost where it is seen.
                ImGui::TableSetColumnIndex(0);
                if (IsPromotable(entry.kind))
                {
                    bool checked = _selectedForPromote.count(entry.relativePath) > 0;
                    if (ImGui::Checkbox("##p", &checked))
                    {
                        if (checked)
                            _selectedForPromote.insert(entry.relativePath);
                        else
                            _selectedForPromote.erase(entry.relativePath);
                    }
                }

                // Status pill.
                ImGui::TableSetColumnIndex(2);
                StatusBadge(KindLabel(entry.kind), KindColor(entry.kind));

                // Quiet explicit actions.
                ImGui::TableSetColumnIndex(3);
                if (inScaffold)
                {
                    if (GhostButton("scaffold"))
                        _docHost.OpenPanel(scaffoldFile);
                    ImGui::SameLine();
                }
                if (inProject)
                {
                    if (GhostButton("project"))
                        _docHost.OpenPanel(projectFile);
                    ImGui::SameLine();
                }
                if (entry.kind == core::DiffKind::kModified
                    && GhostButton("diff"))
                    _diffViewer.Open(scaffoldFile, scaffold.name,
                                     projectFile, "Project");

                ImGui::PopID();
                ++rowId;
            }
        }

        ImGui::EndTable();
        ImGui::PopStyleVar();

        RenderPromoteConfirmModal();
        RenderApplyConfirmModal();

        ImGui::End();
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
