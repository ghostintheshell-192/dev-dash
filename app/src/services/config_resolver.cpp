#include "config_resolver.h"

#include <cstdlib>

namespace dev_dash::services
{
    core::EffectiveConfig ConfigResolver::Resolve(
        const core::Project& /*project*/,
        const core::ConfigLayer& /*global*/,
        const core::ConfigLayer& /*projectLayer*/,
        const core::ConfigLayer& /*workspace*/)
    {
        return core::EffectiveConfig{};
    }

    core::EffectiveConfig ConfigResolver::ResolveWithDefaultGlobal(
        const core::Project& project,
        const core::ConfigLayer& projectLayer)
    {
        core::ConfigLayer global{
            core::ConfigLayerKind::kGlobal,
            std::filesystem::path(std::getenv("HOME") ? std::getenv("HOME") : "") / ".claude",
            "Global"
        };
        return Resolve(project, global, projectLayer);
    }
}
