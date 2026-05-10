#pragma once

#include "../core/effective_config.h"
#include "../core/project.h"

namespace dev_dash::services
{
    class MemoryAdapter
    {
    public:
        core::ConfigSection Resolve(const core::Project& project) const;
    };
}
