#pragma once

#include <filesystem>
#include <vector>

#include "../core/config_layer.h"
#include "../core/effective_config.h"

namespace dev_dash::services
{
    class SettingsParser
    {
    public:
        // Parse MCP server entries from a settings.json file.
        // Each ConfigNode: relativePath = server name, sourceFilePath = settingsPath.
        std::vector<core::ConfigNode> ParseMcpServers(
            const std::filesystem::path& settingsPath,
            core::ConfigLayerKind layer) const;

        // Parse hook entries from a settings.json file.
        // Each ConfigNode: relativePath = "Event / matcher", sourceFilePath = settingsPath.
        std::vector<core::ConfigNode> ParseHooks(
            const std::filesystem::path& settingsPath,
            core::ConfigLayerKind layer) const;
    };
}
