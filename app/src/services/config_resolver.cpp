#include "config_resolver.h"

#include <algorithm>
#include <cstdlib>
#include <string>

namespace dev_dash::services
{
    namespace
    {
        bool IsMd(const std::filesystem::path& p)
        {
            return p.extension() == ".md";
        }

        // Converts an absolute path to the encoded name Claude Code uses under
        // ~/.claude/projects/: every '/' in the path string becomes '-'.
        std::string EncodeProjectPath(const std::filesystem::path& projectPath)
        {
            std::string s = projectPath.string();
            std::replace(s.begin(), s.end(), '/', '-');
            return s;
        }

        // Append nodes from `paths` into `nodes`, skipping duplicates by sourceFilePath.
        void AppendNodes(
            std::vector<core::ConfigNode>& nodes,
            const std::vector<std::filesystem::path>& paths,
            core::ConfigLayerKind layer,
            const std::filesystem::path& baseDir)
        {
            for (const auto& p : paths)
            {
                std::error_code ec;
                const std::string rel =
                    std::filesystem::relative(p, baseDir, ec).string();
                const std::string label = ec ? p.filename().string() : rel;

                const bool dup = std::any_of(nodes.begin(), nodes.end(),
                    [&](const core::ConfigNode& n) { return n.sourceFilePath == p; });
                if (!dup)
                    nodes.push_back({label, layer, p});
            }
        }
    }

    // -------------------------------------------------------------------------
    // Public API
    // -------------------------------------------------------------------------

    core::EffectiveConfig ConfigResolver::ResolveWithDefaultGlobal(
        const core::Project& project,
        const core::ConfigLayer& projectLayer)
    {
        const char* home = std::getenv("HOME");
        const std::filesystem::path homeDir = home ? home : std::filesystem::path{};

        const core::ConfigLayer global{
            core::ConfigLayerKind::kGlobal,
            homeDir.empty() ? std::filesystem::path{} : homeDir / ".claude",
            "Global"
        };
        const core::ConfigLayer workspace{};  // no workspace layer for now

        std::vector<core::ConfigSection> sections;
        sections.push_back(ResolveClaudeMdSection(global, projectLayer, workspace));
        sections.push_back(ResolveRulesSection(global, projectLayer));
        sections.push_back(ResolveMemorySection(project));
        sections.push_back(ResolveSkillsSection(global, projectLayer));
        sections.push_back(ResolveAgentsSection(global, projectLayer));
        sections.push_back(ResolveMcpSection(global, projectLayer));
        sections.push_back(ResolveHooksSection(global, projectLayer));

        return core::EffectiveConfig(std::move(sections));
    }

    // -------------------------------------------------------------------------
    // Section resolvers
    // -------------------------------------------------------------------------

    core::ConfigSection ConfigResolver::ResolveClaudeMdSection(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project,
        const core::ConfigLayer& workspace) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kClaudeMd,
            "CLAUDE.md",
            {},
            true
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            const auto claudeMd = layer.path / "CLAUDE.md";
            if (!std::filesystem::exists(claudeMd)) return;

            section.nodes.push_back({
                "CLAUDE.md",
                layer.kind,
                claudeMd
            });

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

    core::ConfigSection ConfigResolver::ResolveRulesSection(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kRules,
            "Rules",
            {},
            true
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            const auto rulesDir = layer.path / "rules";
            const auto paths = _scanner.Scan(rulesDir, IsMd, /*recursive=*/true);
            AppendNodes(section.nodes, paths, layer.kind, rulesDir);
        };

        addLayer(global);
        addLayer(project);

        return section;
    }

    core::ConfigSection ConfigResolver::ResolveMemorySection(
        const core::Project& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kMemory,
            "Memory",
            {},
            true
        };

        const char* home = std::getenv("HOME");
        if (!home) return section;

        const auto memoryMd =
            std::filesystem::path(home) / ".claude" / "projects"
            / EncodeProjectPath(project.path) / "memory" / "MEMORY.md";

        if (!std::filesystem::exists(memoryMd)) return section;

        section.nodes.push_back({
            "MEMORY.md",
            core::ConfigLayerKind::kGlobal,
            memoryMd
        });

        return section;
    }

    core::ConfigSection ConfigResolver::ResolveSkillsSection(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kSkills,
            "Skills",
            {},
            true   // descriptions always loaded; full content on invocation
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            const auto skillsDir = layer.path / "skills";
            const auto paths = _scanner.Scan(skillsDir, IsMd, /*recursive=*/false);
            AppendNodes(section.nodes, paths, layer.kind, skillsDir);
        };

        addLayer(global);
        addLayer(project);

        return section;
    }

    core::ConfigSection ConfigResolver::ResolveAgentsSection(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kAgents,
            "Agents",
            {},
            false   // loaded only when the subagent is spawned
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            const auto agentsDir = layer.path / "agents";
            const auto paths = _scanner.Scan(agentsDir, IsMd, /*recursive=*/false);
            AppendNodes(section.nodes, paths, layer.kind, agentsDir);
        };

        addLayer(global);
        addLayer(project);

        return section;
    }

    core::ConfigSection ConfigResolver::ResolveMcpSection(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kMcpServers,
            "MCP Servers",
            {},
            true   // tool names always loaded; full schemas on-demand
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            for (const auto& fname : {"settings.json", "settings.local.json"})
            {
                const auto p = layer.path / fname;
                for (auto& node : _settingsParser.ParseMcpServers(p, layer.kind))
                {
                    const bool dup = std::any_of(
                        section.nodes.begin(), section.nodes.end(),
                        [&](const core::ConfigNode& n) { return n.relativePath == node.relativePath; });
                    if (!dup)
                        section.nodes.push_back(std::move(node));
                }
            }
        };

        addLayer(global);
        addLayer(project);

        return section;
    }

    core::ConfigSection ConfigResolver::ResolveHooksSection(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kHooks,
            "Hooks",
            {},
            false   // hook config is zero tokens; only hook *output* enters context
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            for (const auto& fname : {"settings.json", "settings.local.json"})
            {
                const auto p = layer.path / fname;
                for (auto& node : _settingsParser.ParseHooks(p, layer.kind))
                    section.nodes.push_back(std::move(node));
            }
        };

        addLayer(global);
        addLayer(project);

        return section;
    }
}
