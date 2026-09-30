#include "agents_adapter.h"
#include "adapter_utils.h"

#include <string>
#include <vector>

namespace dev_dash::services
{
    AgentsAdapter::AgentsAdapter(ConfigFileScanner& scanner)
        : _scanner(scanner)
    {}

    core::ConfigSection AgentsAdapter::Resolve(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kAgents, "Agents", {}, false
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            const auto agentsDir = layer.path / "agents";
            AppendNodes(section.nodes,
                        _scanner.Scan(agentsDir, IsMd, /*recursive=*/false),
                        layer.kind,
                        agentsDir);
        };

        addLayer(global);
        addLayer(project);

        // A subagent is identified by its frontmatter `name:`, falling back
        // to the file name. Same name at two levels: the project's wins.
        std::vector<std::string> names;
        for (const auto& node : section.nodes)
        {
            std::string name = ReadFrontmatterField(node.sourceFilePath, "name");
            names.push_back(name.empty() ? node.sourceFilePath.stem().string() : name);
        }
        MarkShadowed(section.nodes, names, [](core::ConfigLayerKind kind)
        {
            return kind == core::ConfigLayerKind::kProject ? 1 : 0;
        });

        return section;
    }
}
