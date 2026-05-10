#include "skills_adapter.h"
#include "adapter_utils.h"

namespace dev_dash::services
{
    SkillsAdapter::SkillsAdapter(ConfigFileScanner& scanner)
        : _scanner(scanner)
    {}

    core::ConfigSection SkillsAdapter::Resolve(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kSkills, "Skills", {}, true
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            const auto skillsDir = layer.path / "skills";
            AppendNodes(section.nodes,
                        _scanner.Scan(skillsDir, IsMd, /*recursive=*/false),
                        layer.kind,
                        skillsDir);
        };

        addLayer(global);
        addLayer(project);

        return section;
    }
}
