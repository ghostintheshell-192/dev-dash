#include "claude_md_adapter.h"
#include "adapter_utils.h"

#include <algorithm>
#include <filesystem>
#include <vector>

namespace dev_dash::services
{
    ClaudeMdAdapter::ClaudeMdAdapter(ConfigFileScanner& scanner)
        : _scanner(scanner)
    {}

    core::ConfigSection ClaudeMdAdapter::Resolve(
        const core::ConfigLayer& global,
        const core::Project& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kClaudeMd, "CLAUDE.md", {}, true
        };

        auto contains = [&](const std::filesystem::path& p)
        {
            return std::any_of(section.nodes.begin(), section.nodes.end(),
                [&](const core::ConfigNode& n) { return n.sourceFilePath == p; });
        };

        // Labels are relative to baseDir: ~/.claude for the user file, the
        // project root for everything else (ancestors read as "../CLAUDE.md").
        auto addFile = [&](const std::filesystem::path& file,
                           core::ConfigLayerKind layer,
                           const std::filesystem::path& baseDir)
        {
            if (!std::filesystem::is_regular_file(file) || contains(file)) return;

            auto label = [&](const std::filesystem::path& p)
            {
                std::error_code ec;
                const auto rel = std::filesystem::relative(p, baseDir, ec);
                return ec || rel.empty() ? p.filename().string() : rel.string();
            };

            section.nodes.push_back({label(file), layer, file});
            for (const auto& included : _scanner.ResolveAtIncludes(file))
            {
                if (!contains(included))
                    section.nodes.push_back({label(included), layer, included});
            }
        };

        if (!global.path.empty())
            addFile(global.path / "CLAUDE.md", core::ConfigLayerKind::kGlobal, global.path);

        const auto projectDir = ProjectRoot(project);

        // Walk up to, but not including, the filesystem root; then emit
        // outermost first, the order Claude Code concatenates them in.
        std::vector<std::filesystem::path> ancestors;
        for (auto dir = projectDir.parent_path();
             dir.has_relative_path();
             dir = dir.parent_path())
            ancestors.push_back(dir);
        std::reverse(ancestors.begin(), ancestors.end());

        for (const auto& dir : ancestors)
        {
            addFile(dir / "CLAUDE.md", core::ConfigLayerKind::kAncestor, projectDir);
            addFile(dir / "CLAUDE.local.md", core::ConfigLayerKind::kAncestor, projectDir);
        }

        addFile(projectDir / "CLAUDE.md", core::ConfigLayerKind::kProject, projectDir);
        addFile(projectDir / ".claude" / "CLAUDE.md", core::ConfigLayerKind::kProject, projectDir);
        addFile(projectDir / "CLAUDE.local.md", core::ConfigLayerKind::kLocal, projectDir);

        return section;
    }
}
