#pragma once

#include "../core/effective_config.h"
#include "../core/project.h"
#include "../core/config_layer.h"

namespace dev_dash::services
{
    class ConfigResolver
    {
    public:
        ConfigResolver() = default;

        core::EffectiveConfig Resolve(
            const core::Project& project,
            const core::ConfigLayer& global,
            const core::ConfigLayer& projectLayer,
            const core::ConfigLayer& workspace = core::ConfigLayer{});

        core::EffectiveConfig ResolveWithDefaultGlobal(
            const core::Project& project,
            const core::ConfigLayer& projectLayer);
    };
}
