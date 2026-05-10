#include "diff_engine.h"

namespace dev_dash::services
{
    std::vector<core::DiffEntry> DiffEngine::CompareTrees(
        const std::filesystem::path& /*sourceRoot*/,
        const std::filesystem::path& /*targetRoot*/,
        bool /*includeContent*/)
    {
        return {};
    }
}
