#include "rules_adapter.h"
#include "adapter_utils.h"

namespace dev_dash::services
{
    RulesAdapter::RulesAdapter(ConfigFileScanner& scanner)
        : _scanner(scanner)
    {}

    core::ConfigSection RulesAdapter::Resolve(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kRules, "Rules", {}, true
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            const auto rulesDir = layer.path / "rules";
            AppendNodes(section.nodes,
                        _scanner.Scan(rulesDir, IsMd, /*recursive=*/true),
                        layer.kind,
                        rulesDir);
        };

        addLayer(global);
        addLayer(project);

        return section;
    }
}
