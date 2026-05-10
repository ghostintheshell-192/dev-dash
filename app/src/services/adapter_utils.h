#pragma once

#include <algorithm>
#include <filesystem>
#include <vector>

#include "../core/config_layer.h"
#include "../core/effective_config.h"

namespace dev_dash::services
{
    inline bool IsMd(const std::filesystem::path& p)
    {
        return p.extension() == ".md";
    }

    // Append scanner results as ConfigNodes into `nodes`, skipping duplicates
    // by sourceFilePath. Labels are relative to baseDir when possible.
    inline void AppendNodes(
        std::vector<core::ConfigNode>&             nodes,
        const std::vector<std::filesystem::path>&  paths,
        core::ConfigLayerKind                      layer,
        const std::filesystem::path&               baseDir)
    {
        for (const auto& p : paths)
        {
            std::error_code ec;
            const std::string rel = std::filesystem::relative(p, baseDir, ec).string();
            const std::string label = ec ? p.filename().string() : rel;

            const bool dup = std::any_of(nodes.begin(), nodes.end(),
                [&](const core::ConfigNode& n) { return n.sourceFilePath == p; });
            if (!dup)
                nodes.push_back({label, layer, p});
        }
    }
}
