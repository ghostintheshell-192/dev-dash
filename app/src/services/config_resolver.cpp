#include "config_resolver.h"

#include <cstdlib>
#include <filesystem>

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
        const core::Project&     project,
        const core::ConfigLayer& projectLayer)
    {
        const char* home = std::getenv("HOME");
        const std::filesystem::path homeDir = home ? home : std::filesystem::path{};

        const core::ConfigLayer global{
            core::ConfigLayerKind::kGlobal,
            homeDir.empty() ? std::filesystem::path{} : homeDir / ".claude",
            "Global"
        };
        const core::ConfigLayer workspace{};

        std::vector<core::ConfigSection> sections;
        sections.push_back(_claudeMd.Resolve(global, workspace, projectLayer));
        sections.push_back(_rules.Resolve(global, projectLayer));
        sections.push_back(_memory.Resolve(project));
        sections.push_back(_skills.Resolve(global, projectLayer));
        sections.push_back(_agents.Resolve(global, projectLayer));
        sections.push_back(_mcp.Resolve(global, projectLayer));
        sections.push_back(_hooks.Resolve(global, projectLayer));

        return core::EffectiveConfig(std::move(sections));
    }
}
