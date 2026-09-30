#pragma once

#include "../core/config_layer.h"
#include "../core/effective_config.h"
#include "../core/project.h"
#include "config_file_scanner.h"

namespace dev_dash::services
{
    // The CLAUDE.md chain Claude Code loads at startup, in load order:
    // user (~/.claude/CLAUDE.md), the ancestors of the project from the
    // outermost down, the project (./CLAUDE.md and ./.claude/CLAUDE.md), and
    // the personal ./CLAUDE.local.md. Each file brings its @-includes.
    // Subdirectory CLAUDE.md files load on demand and are not listed.
    class ClaudeMdAdapter
    {
    public:
        explicit ClaudeMdAdapter(ConfigFileScanner& scanner);

        core::ConfigSection Resolve(const core::ConfigLayer& global,
                                    const core::Project& project) const;

    private:
        ConfigFileScanner& _scanner;
    };
}
