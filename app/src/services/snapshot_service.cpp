#include "snapshot_service.h"

namespace dev_dash::services
{
    SnapshotService::SnapshotService(ApplyEngine& applyEngine)
        : _applyEngine(applyEngine)
    {
    }

    core::Snapshot SnapshotService::SaveExplicit(
        const core::Project& /*project*/,
        std::string_view /*name*/,
        std::string_view /*description*/)
    {
        return {};
    }

    core::Snapshot SnapshotService::SaveAuto(
        const core::Project& /*project*/,
        std::string_view /*action*/)
    {
        return {};
    }

    std::vector<core::Snapshot> SnapshotService::List(std::string_view /*projectSlug*/)
    {
        return {};
    }

    bool SnapshotService::Restore(
        const core::Snapshot& /*snapshot*/,
        const core::Project& /*target*/)
    {
        return false;
    }

    int SnapshotService::PruneAuto(std::string_view /*projectSlug*/, int /*maxAutoSnapshots*/)
    {
        return 0;
    }
}
