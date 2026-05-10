#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

#include "../core/project.h"
#include "../core/snapshot.h"

namespace dev_dash::services
{
    class ApplyEngine;

    class SnapshotService
    {
    public:
        explicit SnapshotService(ApplyEngine& applyEngine);

        core::Snapshot SaveExplicit(
            const core::Project& project,
            std::string_view name,
            std::string_view description = {});

        core::Snapshot SaveAuto(
            const core::Project& project,
            std::string_view action);

        std::vector<core::Snapshot> List(std::string_view projectSlug);

        bool Restore(
            const core::Snapshot& snapshot,
            const core::Project& target);

        int PruneAuto(std::string_view projectSlug, int maxAutoSnapshots = 10);

        std::filesystem::path SnapshotRoot() const { return _snapshotRoot; }
        void SetSnapshotRoot(std::filesystem::path root) { _snapshotRoot = std::move(root); }

    private:
        ApplyEngine& _applyEngine;
        std::filesystem::path _snapshotRoot;
    };
}
