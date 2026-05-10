#include "apply_engine.h"

namespace dev_dash::services
{
    bool ApplyEngine::Apply(
        const std::filesystem::path& /*sourceRoot*/,
        const std::filesystem::path& /*targetRoot*/,
        const ApplyConfig& /*config*/,
        SnapshotService* /*snapshotService*/)
    {
        return false;
    }
}
