#pragma once

#include <filesystem>

#include "../core/effective_config.h"
#include "../core/project.h"
#include "config_file_scanner.h"
#include "settings_parser.h"
#include "claude_md_adapter.h"
#include "rules_adapter.h"
#include "memory_adapter.h"
#include "skills_adapter.h"
#include "agents_adapter.h"
#include "mcp_adapter.h"
#include "hooks_adapter.h"

namespace dev_dash::services
{
    class ConfigResolver
    {
    public:
        ConfigResolver();

        // Resolve against the current user's home directory ($HOME).
        core::EffectiveConfig ResolveWithDefaultGlobal(const core::Project& project);

        // Resolve with an explicit home directory: the user layer is
        // homeDir/.claude, user and local MCP servers come from
        // homeDir/.claude.json. An empty homeDir skips those sources.
        core::EffectiveConfig Resolve(const core::Project& project,
                                      const std::filesystem::path& homeDir);

    private:
        // Owned utilities — must be declared before adapters that hold refs to them.
        ConfigFileScanner _scanner;
        SettingsParser    _parser;

        ClaudeMdAdapter _claudeMd;
        RulesAdapter    _rules;
        MemoryAdapter   _memory;
        SkillsAdapter   _skills;
        AgentsAdapter   _agents;
        McpAdapter      _mcp;
        HooksAdapter    _hooks;
    };
}
