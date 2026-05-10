#include "mcp_adapter.h"

#include <algorithm>

namespace dev_dash::services
{
    McpAdapter::McpAdapter(SettingsParser& parser)
        : _parser(parser)
    {}

    core::ConfigSection McpAdapter::Resolve(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kMcpServers, "MCP Servers", {}, true
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            for (const auto& fname : {"settings.json", "settings.local.json"})
            {
                for (auto& node : _parser.ParseMcpServers(layer.path / fname, layer.kind))
                {
                    const bool dup = std::any_of(
                        section.nodes.begin(), section.nodes.end(),
                        [&](const core::ConfigNode& n) {
                            return n.relativePath == node.relativePath;
                        });
                    if (!dup)
                        section.nodes.push_back(std::move(node));
                }
            }
        };

        addLayer(global);
        addLayer(project);

        return section;
    }
}
