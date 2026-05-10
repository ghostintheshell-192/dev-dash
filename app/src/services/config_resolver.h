#pragma once

#include "../core/config_layer.h"
#include "../core/effective_config.h"
#include "../core/project.h"
#include "config_file_scanner.h"
#include "settings_parser.h"

namespace dev_dash::services
{
    class ConfigResolver
    {
    public:
        ConfigResolver() = default;

        core::EffectiveConfig ResolveWithDefaultGlobal(
            const core::Project& project,
            const core::ConfigLayer& projectLayer);

    private:
        core::ConfigSection ResolveClaudeMdSection(
            const core::ConfigLayer& global,
            const core::ConfigLayer& project,
            const core::ConfigLayer& workspace) const;

        core::ConfigSection ResolveRulesSection(
            const core::ConfigLayer& global,
            const core::ConfigLayer& project) const;

        core::ConfigSection ResolveMemorySection(
            const core::Project& project) const;

        core::ConfigSection ResolveSkillsSection(
            const core::ConfigLayer& global,
            const core::ConfigLayer& project) const;

        core::ConfigSection ResolveAgentsSection(
            const core::ConfigLayer& global,
            const core::ConfigLayer& project) const;

        core::ConfigSection ResolveMcpSection(
            const core::ConfigLayer& global,
            const core::ConfigLayer& project) const;

        core::ConfigSection ResolveHooksSection(
            const core::ConfigLayer& global,
            const core::ConfigLayer& project) const;

        ConfigFileScanner _scanner;
        SettingsParser    _settingsParser;
    };
}
