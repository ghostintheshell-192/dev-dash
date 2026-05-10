#include "claude_md_adapter.h"

#include <algorithm>
#include <filesystem>

namespace dev_dash::services
{
    ClaudeMdAdapter::ClaudeMdAdapter(ConfigFileScanner& scanner)
        : _scanner(scanner)
    {}

    core::ConfigSection ClaudeMdAdapter::Resolve(
        const core::ConfigLayer& global,
        const core::ConfigLayer& workspace,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kClaudeMd, "CLAUDE.md", {}, true
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            const auto claudeMd = layer.path / "CLAUDE.md";
            if (!std::filesystem::exists(claudeMd)) return;

            section.nodes.push_back({"CLAUDE.md", layer.kind, claudeMd});

            for (const auto& included : _scanner.ResolveAtIncludes(claudeMd))
            {
                const bool dup = std::any_of(
                    section.nodes.begin(), section.nodes.end(),
                    [&](const core::ConfigNode& n) { return n.sourceFilePath == included; });
                if (dup) continue;

                std::error_code ec;
                const std::string rel =
                    std::filesystem::relative(included, layer.path, ec).string();
                section.nodes.push_back({
                    ec ? included.filename().string() : rel,
                    layer.kind,
                    included
                });
            }
        };

        addLayer(global);
        addLayer(workspace);
        addLayer(project);

        return section;
    }
}
