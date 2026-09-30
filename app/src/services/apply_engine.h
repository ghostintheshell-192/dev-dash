#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace dev_dash::services
{
    class SnapshotService;

    class ApplyEngine
    {
    public:
        struct ApplyConfig
        {
            std::vector<std::string> filesToApply;
            bool createAutosnapshot = true;
            bool forceOverwrite     = false;
            std::string autosnapshotAction = "pre-apply";
        };

        struct ApplyResult
        {
            int applied = 0;  // file copied successfully
            int skipped = 0;  // dest existed and !forceOverwrite, or source missing
            int failed  = 0;  // copy/mkdir errored
        };

        ApplyEngine() = default;

        ApplyResult Apply(
            const std::filesystem::path& sourceRoot,
            const std::filesystem::path& targetRoot,
            const ApplyConfig& config,
            SnapshotService* snapshotService = nullptr);
    };
}
