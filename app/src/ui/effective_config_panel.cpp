#include "effective_config_panel.h"
#include "document_panel_host.h"
#include "theme.h"
#include "../services/config_resolver.h"

#include <imgui.h>

namespace dev_dash::ui
{
    namespace
    {
        const char* LayerLabel(core::ConfigLayerKind kind)
        {
            switch (kind)
            {
            case core::ConfigLayerKind::kGlobal:    return "Global";
            case core::ConfigLayerKind::kWorkspace: return "Workspace";
            case core::ConfigLayerKind::kProject:   return "Project";
            }
            return "?";
        }

        ImVec4 LayerColor(core::ConfigLayerKind kind)
        {
            const Theme& t = CurrentTheme();
            switch (kind)
            {
            case core::ConfigLayerKind::kGlobal:    return t.info;
            case core::ConfigLayerKind::kWorkspace: return t.external;
            case core::ConfigLayerKind::kProject:   return t.accent;
            }
            return t.text;
        }

        const char* SectionIcon(core::ConfigSectionKind kind)
        {
            switch (kind)
            {
            case core::ConfigSectionKind::kClaudeMd:  return "CLAUDE.md";
            case core::ConfigSectionKind::kRules:     return "Rules";
            case core::ConfigSectionKind::kMemory:    return "Memory";
            case core::ConfigSectionKind::kSkills:    return "Skills";
            case core::ConfigSectionKind::kAgents:    return "Agents";
            case core::ConfigSectionKind::kMcpServers:return "MCP Servers";
            case core::ConfigSectionKind::kHooks:     return "Hooks";
            }
            return "?";
        }
    }

    EffectiveConfigPanel::EffectiveConfigPanel(services::ConfigResolver& resolver,
                                               DocumentPanelHost& docHost,
                                               const core::Project& project)
        : _resolver(resolver)
        , _docHost(docHost)
        , _project(project)
    {
        Refresh();
    }

    void EffectiveConfigPanel::Render()
    {
        // Reset per-frame flags before any button can re-raise them.
        _wantsBack         = false;
        _wantsScaffoldDiff = false;
        _wantsHistory      = false;

        const ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##effective_config",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                         | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Toolbar
        if (ImGui::Button("<- Back"))
            _wantsBack = true;

        ImGui::SameLine();
        ImGui::TextDisabled("%s", _project.path.string().c_str());

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 245.0f + ImGui::GetCursorPosX());
        if (ImGui::Button("History..."))
            _wantsHistory = true;

        ImGui::SameLine();
        if (ImGui::Button("Scaffold..."))
            _wantsScaffoldDiff = true;

        ImGui::SameLine();
        if (ImGui::Button("Refresh"))
            Refresh();

        ImGui::Separator();
        ImGui::Spacing();

        const ImGuiTableFlags tableFlags =
            ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;

        if (ImGui::BeginTable("##config_files", 2, tableFlags,
                              ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Layer", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("File",  ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            const auto& sections = _config.Sections();
            bool anyNode = false;

            int nodeId = 0;
            for (const auto& section : sections)
            {
                if (section.nodes.empty()) continue;
                anyNode = true;

                // Section header row
                const ImU32 sectionBg = ImGui::GetColorU32(CurrentTheme().sectionBg);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, sectionBg);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, sectionBg);
                ImGui::TextDisabled("%s", SectionIcon(section.kind));

                ImGui::TableSetColumnIndex(1);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, sectionBg);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, sectionBg);
                if (!section.alwaysInContext)
                    ImGui::TextDisabled("(on demand)");

                // Node rows — PushID(int) gives each Selectable a unique ID
                // even when multiple nodes share the same display label.
                for (const auto& node : section.nodes)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushStyleColor(ImGuiCol_Text, LayerColor(node.sourceLayer));
                    ImGui::TextUnformatted(LayerLabel(node.sourceLayer));
                    ImGui::PopStyleColor();

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushID(nodeId++);
                    const bool clicked = ImGui::Selectable(
                        node.relativePath.c_str(),
                        false,
                        ImGuiSelectableFlags_SpanAllColumns);
                    ImGui::PopID();
                    if (clicked && !node.sourceFilePath.empty())
                        _docHost.OpenPanel(node.sourceFilePath);
                }
            }

            if (!anyNode)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(1);
                ImGui::TextDisabled("No configuration files found.");
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }

    void EffectiveConfigPanel::Refresh()
    {
        core::ConfigLayer projectLayer{
            core::ConfigLayerKind::kProject,
            _project.path / ".claude",
            "Project"
        };
        _config = _resolver.ResolveWithDefaultGlobal(_project, projectLayer);
    }
}
