#pragma once

#include "../core/config_layer.h"
#include "../core/effective_config.h"
#include "settings_parser.h"

namespace dev_dash::services
{
    class McpAdapter
    {
    public:
        explicit McpAdapter(SettingsParser& parser);

        core::ConfigSection Resolve(const core::ConfigLayer& global,
                                    const core::ConfigLayer& project) const;

    private:
        SettingsParser& _parser;
    };
}
