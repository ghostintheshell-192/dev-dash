#pragma once

#include <filesystem>

#include "../core/effective_config.h"
#include "../core/project.h"
#include "settings_parser.h"

namespace dev_dash::services
{
    // MCP servers from the three scopes Claude Code reads: local
    // (~/.claude.json, under the project's path), project (./.mcp.json) and
    // user (~/.claude.json, top level). Same name in two scopes: local wins
    // over project, project over user; the others are kept, marked shadowed.
    class McpAdapter
    {
    public:
        explicit McpAdapter(SettingsParser& parser);

        core::ConfigSection Resolve(const core::Project& project,
                                    const std::filesystem::path& homeDir) const;

    private:
        SettingsParser& _parser;
    };
}
