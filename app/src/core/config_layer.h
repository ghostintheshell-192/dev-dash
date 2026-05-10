#pragma once

#include <filesystem>
#include <string>

namespace dev_dash::core
{
    enum class ConfigLayerKind
    {
        kGlobal,
        kWorkspace,
        kProject
    };

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
