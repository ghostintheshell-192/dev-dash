#include "sidebar.h"
#include "status_sink.h"
#include "theme.h"
#include "widgets.h"
#include "../services/config_resolver.h"
#include "../services/scaffold_repository.h"
#include "../services/snapshot_service.h"

#include <algorithm>

#include <imgui.h>

namespace dev_dash::ui
{
    namespace
    {
        constexpr ImGuiTreeNodeFlags kSectionFlags =
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap;

        constexpr int kHistoryPreviewCount = 5;

        ImVec4 LayerColor(core::ConfigLayerKind kind)
        {
            const Theme& t = CurrentTheme();
            switch (kind)
            {
            case core::ConfigLayerKind::kGlobal:   return t.info;
            case core::ConfigLayerKind::kAncestor: return t.external;
            case core::ConfigLayerKind::kProject:  return t.accent;
            case core::ConfigLayerKind::kLocal:    return t.modified;
            }
            return t.text;
        }

        // Right-aligned small button overlapping a section header row.
        bool HeaderButton(const char* label)
        {
            ImGui::SameLine(ImGui::GetContentRegionAvail().x
                            - ImGui::CalcTextSize(label).x - 8.0f
                            + ImGui::GetCursorPosX());
            return ImGui::SmallButton(label);
        }

        // "YYYYMMDD-HHMMSS" → "YYYY-MM-DD HH:MM"
        std::string ShortTimestamp(const std::string& ts)
        {
            if (ts.size() < 13) return ts;
            return ts.substr(0, 4) + "-" + ts.substr(4, 2) + "-" + ts.substr(6, 2)
                 + " " + ts.substr(9, 2) + ":" + ts.substr(11, 2);
        }
    }

    Sidebar::Sidebar(services::ConfigResolver&     configResolver,
                     services::ScaffoldRepository& scaffoldRepository,
                     services::SnapshotService&    snapshotService,
                     StatusSink&                   status,
                     const core::Project&          project,
                     Callbacks                     callbacks)
        : _configResolver(configResolver)
        , _scaffoldRepo(scaffoldRepository)
        , _snapshotService(snapshotService)
        , _status(status)
        , _project(project)
        , _callbacks(std::move(callbacks))
        , _projectSlug(core::MakeProjectSlug(project.path))
    {
    }

    void Sidebar::Render()
    {
        if (_needsRefresh)
        {
            Refresh();
            _needsRefresh = false;
        }

        ImGui::TextDisabled("PROJECT");
        if (HeaderButton("refresh"))
        {
            _scaffoldRepo.Refresh();
            _needsRefresh = true;
        }
        ImGui::Spacing();

        RenderConfigSection();
        RenderScaffoldsSection();
        RenderHistorySection();

        RenderNewScaffoldModal();
        RenderDeleteConfirmModal();
    }

    // ── Config ────────────────────────────────────────────────────────────────

    void Sidebar::RenderConfigSection()
    {
        const bool open = ImGui::CollapsingHeader("Config", kSectionFlags);
        if (HeaderButton("table"))
            _callbacks.openConfigView();
        if (!open)
            return;

        int nodeId = 0;
        for (const auto& section : _config.Sections())
        {
            if (section.nodes.empty())
                continue;

            if (!ImGui::TreeNode(section.label.c_str()))
                continue;

            for (const auto& node : section.nodes)
            {
                ImGui::PushID(nodeId++);
                ImGui::TextColored(LayerColor(node.sourceLayer), "*");
                ImGui::SameLine();
                if (node.shadowed)
                    ImGui::PushStyleColor(ImGuiCol_Text, CurrentTheme().textDim);
                const bool clicked = ImGui::Selectable(node.relativePath.c_str());
                if (node.shadowed)
                    ImGui::PopStyleColor();
                if (clicked && !node.sourceFilePath.empty())
                    _callbacks.openDocument(node.sourceFilePath);
                StatusHint(_status, node.note.empty()
                                        ? node.sourceFilePath.string()
                                        : node.note + "  —  " + node.sourceFilePath.string());
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }

    // ── Scaffolds ─────────────────────────────────────────────────────────────

    void Sidebar::RenderScaffoldsSection()
    {
        const bool open = ImGui::CollapsingHeader("Scaffolds", kSectionFlags);
        if (HeaderButton("+"))
        {
            _newName[0]        = '\0';
            _newMode           = 0;
            _modalScaffoldName = _scaffolds.empty() ? "" : _scaffolds[0].name;
            _showNewModal      = true;
        }
        if (!open)
            return;

        if (_scaffolds.empty())
        {
            ImGui::TextDisabled("(none — create one with +)");
            return;
        }

        for (const auto& s : _scaffolds)
        {
            ImGui::PushID(s.name.c_str());

            const bool nodeOpen = ImGui::TreeNodeEx(
                "##scaffold", ImGuiTreeNodeFlags_AllowOverlap,
                "%s", s.name.c_str());

            // Same action menu, two ways in: right-click on the entry, or
            // the explicit "..." button — the menu must be discoverable,
            // not a secret handshake.
            if (ImGui::BeginPopupContextItem("scaffold_menu"))
            {
                RenderScaffoldMenu(s);
                ImGui::EndPopup();
            }
            StatusHint(_status, s.path.string());

            if (s.isDefault)
            {
                ImGui::SameLine();
                ImGui::TextColored(CurrentTheme().accent, "(default)");
            }

            if (HeaderButton("..."))
                ImGui::OpenPopup("scaffold_menu");

            if (nodeOpen)
            {
                // Lazy: list the files only once the node is expanded.
                auto it = _scaffoldFiles.find(s.name);
                if (it == _scaffoldFiles.end())
                    it = _scaffoldFiles
                             .emplace(s.name,
                                      BuildFileTree(_scaffoldRepo.ListFiles(s.path)))
                             .first;

                RenderFileTree(it->second, s.path, {});
                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }

    void Sidebar::RenderScaffoldMenu(const core::Scaffold& s)
    {
        if (ImGui::MenuItem("Compare with project..."))
            _callbacks.compareScaffold(s.name);
        if (ImGui::MenuItem("Set as default", nullptr, false, !s.isDefault))
        {
            if (_scaffoldRepo.SetDefault(s.path))
            {
                _status.Set(StatusSink::Level::kSuccess,
                            "\"" + s.name + "\" is now the default scaffold.");
                _needsRefresh = true;
            }
            else
                _status.Set(StatusSink::Level::kError,
                            "Failed to set \"" + s.name + "\" as default.");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("New scaffold..."))
        {
            _newName[0]        = '\0';
            _newMode           = 0;
            _modalScaffoldName = s.name;
            _showNewModal      = true;
        }
        if (ImGui::MenuItem("Delete..."))
        {
            _modalScaffoldName = s.name;
            _showDeleteConfirm = true;
        }
    }

    // ── History ───────────────────────────────────────────────────────────────

    void Sidebar::RenderHistorySection()
    {
        const bool open = ImGui::CollapsingHeader("History", kSectionFlags);
        if (HeaderButton("all"))
            _callbacks.openHistoryView();
        if (!open)
            return;

        if (_snapshots.empty())
        {
            ImGui::TextDisabled("(no snapshots yet)");
            return;
        }

        const int count = std::min(static_cast<int>(_snapshots.size()),
                                   kHistoryPreviewCount);
        for (int i = 0; i < count; ++i)
        {
            const auto& snap = _snapshots[i];
            ImGui::PushID(i);
            const std::string label =
                ShortTimestamp(snap.timestamp) + "  " + snap.name;
            if (ImGui::Selectable(label.c_str()))
                _callbacks.openHistoryView();
            ImGui::PopID();
        }
        if (static_cast<int>(_snapshots.size()) > count)
            ImGui::TextDisabled("(%d more — open the full view)",
                                static_cast<int>(_snapshots.size()) - count);
    }

    // ── File tree ─────────────────────────────────────────────────────────────

    Sidebar::FileTree Sidebar::BuildFileTree(const std::vector<std::string>& paths)
    {
        FileTree root;
        for (const auto& rel : paths)
        {
            FileTree* node = &root;
            std::string remaining = rel;
            for (auto pos = remaining.find('/'); pos != std::string::npos;
                 pos = remaining.find('/'))
            {
                node      = &node->dirs[remaining.substr(0, pos)];
                remaining = remaining.substr(pos + 1);
            }
            node->files.push_back(remaining);
        }
        return root;
    }

    void Sidebar::RenderFileTree(const FileTree&              node,
                                 const std::filesystem::path& root,
                                 const std::filesystem::path& relBase)
    {
        for (const auto& [name, child] : node.dirs)
        {
            if (ImGui::TreeNode(name.c_str()))
            {
                RenderFileTree(child, root, relBase / name);
                ImGui::TreePop();
            }
        }

        int id = 0;
        for (const auto& file : node.files)
        {
            ImGui::PushID(id++);
            if (ImGui::Selectable(file.c_str()))
                _callbacks.openDocument(root / relBase / file);
            StatusHint(_status, (root / relBase / file).string());
            ImGui::PopID();
        }
    }

    // ── Modals ────────────────────────────────────────────────────────────────

    void Sidebar::RenderNewScaffoldModal()
    {
        if (_showNewModal)
        {
            ImGui::OpenPopup("New Scaffold");
            _showNewModal = false;
        }

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(380, 0), ImGuiCond_Appearing);

        if (ImGui::BeginPopupModal("New Scaffold", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Name:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(240.0f);
            ImGui::InputText("##new_name", _newName, sizeof(_newName));

            ImGui::Spacing();
            ImGui::RadioButton("Empty", &_newMode, 0);
            if (!_modalScaffoldName.empty())
            {
                ImGui::SameLine();
                const std::string copyLabel =
                    "Copy of \"" + _modalScaffoldName + "\"";
                ImGui::RadioButton(copyLabel.c_str(), &_newMode, 1);
            }

            ImGui::Spacing();

            const bool nameOk = _newName[0] != '\0'
                && !std::filesystem::exists(_scaffoldRepo.ScaffoldRoot() / _newName);

            if (!nameOk)
                ImGui::BeginDisabled();
            if (ImGui::Button("Create", ImVec2(120, 0)))
            {
                bool ok = false;
                if (_newMode == 1 && !_modalScaffoldName.empty())
                    ok = _scaffoldRepo.CreateCopy(
                        _scaffoldRepo.ScaffoldRoot() / _modalScaffoldName, _newName);
                else
                    ok = _scaffoldRepo.CreateEmpty(_newName);

                _status.Set(ok ? StatusSink::Level::kSuccess
                               : StatusSink::Level::kError,
                            ok ? std::string("Scaffold \"") + _newName + "\" created."
                               : std::string("Failed to create scaffold \"")
                                     + _newName + "\".");
                _needsRefresh = true;
                ImGui::CloseCurrentPopup();
            }
            if (!nameOk)
                ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            if (_newName[0] != '\0'
                && std::filesystem::exists(_scaffoldRepo.ScaffoldRoot() / _newName))
            {
                ImGui::Spacing();
                ImGui::TextColored(CurrentTheme().removed,
                                   "A scaffold with that name already exists.");
            }

            ImGui::EndPopup();
        }
    }

    void Sidebar::RenderDeleteConfirmModal()
    {
        if (_showDeleteConfirm)
        {
            ImGui::OpenPopup("Delete scaffold?");
            _showDeleteConfirm = false;
        }

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Delete scaffold?", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Permanently delete scaffold:");
            ImGui::TextColored(CurrentTheme().removed, "  %s",
                               _modalScaffoldName.c_str());
            ImGui::TextDisabled(
                "  %s",
                (_scaffoldRepo.ScaffoldRoot() / _modalScaffoldName).string().c_str());
            ImGui::Spacing();
            ImGui::TextColored(CurrentTheme().modified, "This cannot be undone.");
            ImGui::Spacing();

            if (ImGui::Button("Delete", ImVec2(120, 0)))
            {
                const bool ok = _scaffoldRepo.Delete(
                    _scaffoldRepo.ScaffoldRoot() / _modalScaffoldName);
                _status.Set(ok ? StatusSink::Level::kSuccess
                               : StatusSink::Level::kError,
                            ok ? "Scaffold \"" + _modalScaffoldName + "\" deleted."
                               : "Failed to delete scaffold \""
                                     + _modalScaffoldName + "\".");
                _needsRefresh = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    void Sidebar::Refresh()
    {
        _config    = _configResolver.ResolveWithDefaultGlobal(_project);
        _scaffolds = _scaffoldRepo.List();
        _scaffoldFiles.clear();
        _snapshots = _snapshotService.List(_projectSlug);
    }
}
