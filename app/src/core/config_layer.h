#pragma once

#include <filesystem>
#include <string>

namespace dev_dash::core
{
    // Where a configuration element comes from, from the widest scope to the
    // narrowest. kAncestor covers the CLAUDE.md files of the directories above
    // the project; kLocal the personal, per-project, uncommitted sources
    // (CLAUDE.local.md, the "local" MCP scope in ~/.claude.json).
    enum class ConfigLayerKind
    {
        kGlobal,
        kAncestor,
        kProject,
        kLocal
    };

    inline const char* LayerName(ConfigLayerKind kind)
    {
        switch (kind)
        {
        case ConfigLayerKind::kGlobal:   return "Global";
        case ConfigLayerKind::kAncestor: return "Ancestor";
        case ConfigLayerKind::kProject:  return "Project";
        case ConfigLayerKind::kLocal:    return "Local";
        }
        return "?";
    }

    struct ConfigLayer
    {
        ConfigLayerKind kind = ConfigLayerKind::kGlobal;
        std::filesystem::path path;
        std::string displayName;

        ConfigLayer() = default;
        ConfigLayer(ConfigLayerKind layerKind,
                    std::filesystem::path layerPath,
                    std::string layerDisplayName)
            : kind(layerKind)
            , path(std::move(layerPath))
            , displayName(std::move(layerDisplayName))
        {
        }
    };
}
