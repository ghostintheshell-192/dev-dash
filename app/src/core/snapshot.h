#pragma once

#include <filesystem>
#include <optional>
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
        // Populated only for kAuto snapshots — the action that triggered the auto-save
        // (e.g. "pre-apply", "pre-restore"). Always nullopt for kExplicit.
        std::optional<std::string> originatingAction;
    };
}
