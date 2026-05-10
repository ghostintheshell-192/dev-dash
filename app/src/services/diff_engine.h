#pragma once

#include <filesystem>
#include <vector>

#include "../core/diff_entry.h"

namespace dev_dash::services
{
    class DiffEngine
    {
    public:
        DiffEngine() = default;

        std::vector<core::DiffEntry> CompareTrees(
            const std::filesystem::path& sourceRoot,
            const std::filesystem::path& targetRoot,
            bool includeContent = false);
    };
}
