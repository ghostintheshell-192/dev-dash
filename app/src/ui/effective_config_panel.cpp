#include "effective_config_panel.h"
#include "document_panel_host.h"
#include "theme.h"
#include "../services/config_resolver.h"

#include <imgui.h>

namespace dev_dash::ui
{
    namespace
    {
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

    void EffectiveConfigPanel::Render(bool* open)
    {
        if (!ImGui::Begin("Config", open))
        {
            ImGui::End();
            return;
        }

        // Toolbar
        ImGui::TextDisabled("Effective Claude Code configuration");
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 70.0f
                        + ImGui::GetCursorPosX());
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
                    ImGui::TextUnformatted(core::LayerName(node.sourceLayer));
                    ImGui::PopStyleColor();

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushID(nodeId++);
                    if (node.shadowed)
                        ImGui::PushStyleColor(ImGuiCol_Text, CurrentTheme().textDim);
                    const bool clicked = ImGui::Selectable(
                        node.relativePath.c_str(),
                        false,
                        ImGuiSelectableFlags_SpanAllColumns
                            | ImGuiSelectableFlags_AllowOverlap);
                    if (node.shadowed)
                        ImGui::PopStyleColor();
                    ImGui::PopID();
                    if (!node.note.empty())
                    {
                        ImGui::SameLine();
                        ImGui::TextDisabled("(%s)", node.note.c_str());
                    }
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
        _config = _resolver.ResolveWithDefaultGlobal(_project);
    }
}
