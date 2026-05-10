#pragma once

#include <filesystem>
#include <string>

namespace dev_dash::core
{
    enum class SnapshotKind
    {
        kExplicit,
        kAuto
    };

    struct Snapshot
    {
        SnapshotKind kind = SnapshotKind::kExplicit;
        std::string projectSlug;
        std::string name;
        std::filesystem::path path;
        std::string timestamp;
        std::string description;
        std::string originatingAction;
    };
}
