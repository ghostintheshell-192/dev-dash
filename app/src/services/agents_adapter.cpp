#include "agents_adapter.h"
#include "adapter_utils.h"

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

        return section;
    }
}
