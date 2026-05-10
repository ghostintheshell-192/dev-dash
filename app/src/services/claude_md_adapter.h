#pragma once

#include "../core/config_layer.h"
#include "../core/effective_config.h"
#include "config_file_scanner.h"

namespace dev_dash::services
{
    class ClaudeMdAdapter
    {
    public:
        explicit ClaudeMdAdapter(ConfigFileScanner& scanner);

        core::ConfigSection Resolve(const core::ConfigLayer& global,
                                    const core::ConfigLayer& workspace,
                                    const core::ConfigLayer& project) const;

    private:
        ConfigFileScanner& _scanner;
    };
}
