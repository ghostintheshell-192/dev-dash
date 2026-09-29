#include "config_resolver.h"

#include <cstdlib>

namespace dev_dash::services
{
    ConfigResolver::ConfigResolver()
        : _claudeMd(_scanner)
        , _rules(_scanner)
        , _skills(_scanner)
        , _agents(_scanner)
        , _mcp(_parser)
        , _hooks(_parser)
    {}

    core::EffectiveConfig ConfigResolver::ResolveWithDefaultGlobal(
        const core::Project& project)
    {
        const char* home = std::getenv("HOME");
        return Resolve(project, home ? std::filesystem::path(home) : std::filesystem::path{});
    }

    core::EffectiveConfig ConfigResolver::Resolve(
        const core::Project&         project,
        const std::filesystem::path& homeDir)
    {
        const core::ConfigLayer global{
            core::ConfigLayerKind::kGlobal,
            homeDir.empty() ? std::filesystem::path{} : homeDir / ".claude",
            "Global"
        };
        const core::ConfigLayer projectLayer{
            core::ConfigLayerKind::kProject,
            project.path / ".claude",
            "Project"
        };

        std::vector<core::ConfigSection> sections;
        sections.push_back(_claudeMd.Resolve(global, project));
        sections.push_back(_rules.Resolve(global, projectLayer));
        sections.push_back(_memory.Resolve(project, homeDir));
        sections.push_back(_skills.Resolve(global, projectLayer));
        sections.push_back(_agents.Resolve(global, projectLayer));
        sections.push_back(_mcp.Resolve(project, homeDir));
        sections.push_back(_hooks.Resolve(global, projectLayer));

        return core::EffectiveConfig(std::move(sections));
    }
}
