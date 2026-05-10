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
        };

        ApplyEngine() = default;

        bool Apply(
            const std::filesystem::path& sourceRoot,
            const std::filesystem::path& targetRoot,
            const ApplyConfig& config,
            SnapshotService* snapshotService = nullptr);
    };
}
