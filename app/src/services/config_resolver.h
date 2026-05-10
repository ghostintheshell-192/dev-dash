#pragma once

#include "../core/config_layer.h"
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

        core::EffectiveConfig ResolveWithDefaultGlobal(
            const core::Project&    project,
            const core::ConfigLayer& projectLayer);

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
