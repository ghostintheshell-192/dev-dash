#include "snapshot_history_panel.h"
#include "../services/snapshot_service.h"

#include <imgui.h>

namespace dev_dash::ui
{
    namespace
    {
        // "YYYYMMDD-HHMMSS" → "YYYY-MM-DD HH:MM:SS"
        std::string FormatTimestamp(const std::string& ts)
        {
            if (ts.size() < 15) return ts;
            return ts.substr(0, 4) + "-" + ts.substr(4, 2) + "-" + ts.substr(6, 2)
                 + " " + ts.substr(9, 2) + ":" + ts.substr(11, 2) + ":" + ts.substr(13, 2);
        }
    }

    SnapshotHistoryPanel::SnapshotHistoryPanel(services::SnapshotService& service,
                                               const core::Project&        project)
        : _service(service)
        , _project(project)
        , _projectSlug(core::MakeProjectSlug(project.path))
    {
    }

    void SnapshotHistoryPanel::Render()
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
        ImGui::Begin("##snapshot_history",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                         | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // ── Toolbar ───────────────────────────────────────────────────────────
        if (ImGui::Button("<- Back"))
            _wantsBack = true;

        ImGui::SameLine();
        ImGui::TextDisabled("History: %s", _project.path.string().c_str());

        const float rightEdge = ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX();
        ImGui::SameLine(rightEdge - 185.0f);
        if (ImGui::Button("Save snapshot..."))
        {
            _newName[0] = '\0';
            _newDesc[0] = '\0';
            _showSaveModal = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh"))
            _needsRefresh = true;

        ImGui::Separator();

        if (!_statusMsg.empty())
        {
            ImGui::Spacing();
            ImGui::TextUnformatted(_statusMsg.c_str());
        }

        ImGui::Spacing();

        if (_snapshots.empty())
        {
            ImGui::TextDisabled("No snapshots yet.");
            ImGui::TextDisabled("Click \"Save snapshot...\" to create one.");

            RenderSaveModal();
            ImGui::End();
            return;
        }

        // ── Snapshot table ────────────────────────────────────────────────────
        const ImGuiTableFlags tableFlags =
            ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;

        const float tableH = ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeightWithSpacing();

        if (!ImGui::BeginTable("##snapshots", 5, tableFlags, ImVec2(0.0f, tableH)))
        {
            RenderSaveModal();
            ImGui::End();
            return;
        }

        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Timestamp",   ImGuiTableColumnFlags_WidthFixed,   160.0f);
        ImGui::TableSetupColumn("Kind",        ImGuiTableColumnFlags_WidthFixed,    60.0f);
        ImGui::TableSetupColumn("Name",        ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##action",    ImGuiTableColumnFlags_WidthFixed,    70.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(_snapshots.size()); ++i)
        {
            const auto& snap = _snapshots[i];
            const bool isAuto = snap.kind == core::SnapshotKind::kAuto;

            ImGui::TableNextRow();
            ImGui::PushID(i);

            if (isAuto)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(FormatTimestamp(snap.timestamp).c_str());

            ImGui::TableSetColumnIndex(1);
            if (isAuto)
                ImGui::TextDisabled("auto");
            else
                ImGui::TextColored({0.45f, 0.85f, 0.55f, 1.0f}, "explicit");

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(snap.name.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(snap.description.c_str());

            if (isAuto)
                ImGui::PopStyleColor();

            ImGui::TableSetColumnIndex(4);
            if (ImGui::SmallButton("Restore"))
            {
                _selectedIdx = i;
                _showRestoreConfirm = true;
            }

            ImGui::PopID();
        }

        ImGui::EndTable();

        RenderSaveModal();
        RenderRestoreConfirmModal();

        ImGui::End();
    }

    // ── Save modal ────────────────────────────────────────────────────────────

    void SnapshotHistoryPanel::RenderSaveModal()
    {
        if (_showSaveModal)
        {
            ImGui::OpenPopup("Save snapshot");
            _showSaveModal = false;
        }

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_Appearing);

        if (ImGui::BeginPopupModal("Save snapshot", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Name:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(260.0f);
            ImGui::InputText("##snap_name", _newName, sizeof(_newName));

            ImGui::Text("Description (optional):");
            ImGui::SetNextItemWidth(360.0f);
            ImGui::InputText("##snap_desc", _newDesc, sizeof(_newDesc));

            ImGui::Spacing();

            const bool nameOk = _newName[0] != '\0';
            if (!nameOk) ImGui::BeginDisabled();
            if (ImGui::Button("Save", ImVec2(120, 0)))
            {
                const auto snap = _service.SaveExplicit(_project, _newName, _newDesc);
                if (!snap.path.empty())
                {
                    _statusMsg = "Snapshot \"" + snap.name + "\" saved.";
                    _needsRefresh = true;
                }
                else
                {
                    _statusMsg = "Failed to save snapshot.";
                }
                ImGui::CloseCurrentPopup();
            }
            if (!nameOk) ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    // ── Restore confirm modal ─────────────────────────────────────────────────

    void SnapshotHistoryPanel::RenderRestoreConfirmModal()
    {
        if (_showRestoreConfirm)
        {
            ImGui::OpenPopup("Restore snapshot?");
            _showRestoreConfirm = false;
        }

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Restore snapshot?", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (_selectedIdx < 0 || _selectedIdx >= static_cast<int>(_snapshots.size()))
            {
                ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                return;
            }

            const auto& snap = _snapshots[_selectedIdx];
            ImGui::Text("Restore snapshot:");
            ImGui::TextColored({0.45f, 0.85f, 0.55f, 1.0f}, "  %s", snap.name.c_str());
            ImGui::TextDisabled("  %s", FormatTimestamp(snap.timestamp).c_str());
            ImGui::Spacing();
            ImGui::TextWrapped("This overwrites the current project config.");
            ImGui::TextColored({0.45f, 0.75f, 1.0f, 1.0f},
                               "A pre-restore autosnapshot will be created first.");
            ImGui::Spacing();

            if (ImGui::Button("Restore", ImVec2(120, 0)))
            {
                const bool ok = _service.Restore(snap, _project);
                _statusMsg = ok
                    ? "Restored to \"" + snap.name + "\"."
                    : "Restore failed.";
                _needsRefresh = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    void SnapshotHistoryPanel::Refresh()
    {
        _snapshots = _service.List(_projectSlug);
        if (_selectedIdx >= static_cast<int>(_snapshots.size()))
            _selectedIdx = -1;
    }
}
