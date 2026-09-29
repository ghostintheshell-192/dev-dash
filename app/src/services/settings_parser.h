#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../core/config_layer.h"
#include "../core/effective_config.h"

namespace dev_dash::services
{
    class SettingsParser
    {
    public:
        // Parse the top-level `mcpServers` object of a JSON file: `.mcp.json`
        // (project scope) or `~/.claude.json` (user scope).
        // Each ConfigNode: relativePath = server name, sourceFilePath = jsonPath.
        std::vector<core::ConfigNode> ParseMcpServers(
            const std::filesystem::path& jsonPath,
            core::ConfigLayerKind layer) const;

        // Parse `projects[projectKey].mcpServers` from `~/.claude.json`: the
        // "local" scope, private to one project. projectKey is the project's
        // absolute path.
        std::vector<core::ConfigNode> ParseProjectMcpServers(
            const std::filesystem::path& claudeJsonPath,
            const std::string& projectKey,
            core::ConfigLayerKind layer) const;

        // Parse hook entries from a settings.json file.
        // Each ConfigNode: relativePath = "Event / matcher", sourceFilePath = settingsPath.
        std::vector<core::ConfigNode> ParseHooks(
            const std::filesystem::path& settingsPath,
            core::ConfigLayerKind layer) const;
    };
}
