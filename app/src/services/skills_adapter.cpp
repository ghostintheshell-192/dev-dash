#include "skills_adapter.h"
#include "adapter_utils.h"

#include <filesystem>
#include <string>
#include <vector>

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
            core::ConfigSectionKind::kSkills, "Skills", {}, false
        };
        std::vector<std::string> names;

        // A skill is a directory holding SKILL.md; its name is the directory
        // name. Listed on demand: only name and description sit in context
        // until the skill is invoked.
        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            const auto skillsDir = layer.path / "skills";
            if (!std::filesystem::is_directory(skillsDir)) return;

            std::vector<std::filesystem::path> skillFiles;
            std::error_code ec;
            for (const auto& entry : std::filesystem::directory_iterator(skillsDir, ec))
            {
                const auto skillMd = entry.path() / "SKILL.md";
                if (entry.is_directory(ec) && std::filesystem::is_regular_file(skillMd))
                    skillFiles.push_back(skillMd);
            }
            std::sort(skillFiles.begin(), skillFiles.end());

            for (const auto& skillMd : skillFiles)
            {
                const std::string name = skillMd.parent_path().filename().string();
                section.nodes.push_back({name, layer.kind, skillMd});
                names.push_back(name);
            }
        };

        addLayer(global);
        addLayer(project);

        // Same name at two levels: the user's skill wins over the project's.
        MarkShadowed(section.nodes, names, [](core::ConfigLayerKind kind)
        {
            return kind == core::ConfigLayerKind::kGlobal ? 1 : 0;
        });

        return section;
    }
}
