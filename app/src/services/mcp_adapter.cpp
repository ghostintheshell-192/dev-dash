#include "mcp_adapter.h"
#include "adapter_utils.h"

#include <string>
#include <vector>

namespace dev_dash::services
{
    McpAdapter::McpAdapter(SettingsParser& parser)
        : _parser(parser)
    {}

    core::ConfigSection McpAdapter::Resolve(
        const core::Project& project,
        const std::filesystem::path& homeDir) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kMcpServers, "MCP Servers", {}, true
        };

        auto append = [&](std::vector<core::ConfigNode> nodes)
        {
            for (auto& node : nodes)
                section.nodes.push_back(std::move(node));
        };

        const auto projectDir = ProjectRoot(project);

        if (!homeDir.empty())
        {
            const auto claudeJson = homeDir / ".claude.json";
            append(_parser.ParseMcpServers(claudeJson, core::ConfigLayerKind::kGlobal));
            append(_parser.ParseProjectMcpServers(
                claudeJson, projectDir.string(), core::ConfigLayerKind::kLocal));
        }
        append(_parser.ParseMcpServers(projectDir / ".mcp.json",
                                       core::ConfigLayerKind::kProject));

        std::vector<std::string> names;
        for (const auto& node : section.nodes)
            names.push_back(node.relativePath);
        MarkShadowed(section.nodes, names, [](core::ConfigLayerKind kind)
        {
            switch (kind)
            {
            case core::ConfigLayerKind::kLocal:   return 2;
            case core::ConfigLayerKind::kProject: return 1;
            default:                              return 0;
            }
        });

        return section;
    }
}
