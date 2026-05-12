#include "apply_engine.h"
#include "snapshot_service.h"
#include "../core/project.h"

#include <filesystem>

namespace dev_dash::services
{
    bool ApplyEngine::Apply(
        const std::filesystem::path& sourceRoot,
        const std::filesystem::path& targetRoot,
        const ApplyConfig& config,
        SnapshotService* snapshotService)
    {
        if (config.createAutosnapshot && snapshotService)
        {
            const core::Project targetProject{targetRoot};
            snapshotService->SaveAuto(targetProject, config.autosnaphotAction);
        }

        bool allOk = true;
        for (const auto& rel : config.filesToApply)
        {
            const auto src  = sourceRoot / rel;
            const auto dest = targetRoot / rel;

            if (!std::filesystem::exists(src)) continue;

            // MVP: skip existing unless forcing
            if (!config.forceOverwrite && std::filesystem::exists(dest)) continue;

            std::error_code ec;
            std::filesystem::create_directories(dest.parent_path(), ec);
            if (ec) { allOk = false; continue; }

            std::filesystem::copy_file(src, dest,
                std::filesystem::copy_options::overwrite_existing, ec);
            if (ec) allOk = false;
        }
        return allOk;
    }
}
