#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "../core/config_layer.h"
#include "../core/effective_config.h"
#include "../core/project.h"

namespace dev_dash::services
{
    inline bool IsMd(const std::filesystem::path& p)
    {
        return p.extension() == ".md";
    }

    // The project's absolute path without a trailing separator: the form
    // Claude Code uses as a key in ~/.claude.json and encodes for
    // ~/.claude/projects/.
    inline std::filesystem::path ProjectRoot(const core::Project& project)
    {
        const auto p = std::filesystem::absolute(project.path).lexically_normal();
        return p.has_filename() ? p : p.parent_path();
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

    // Value of a top-level `field:` in a markdown file's YAML frontmatter, or
    // "" when the file has no frontmatter or no such field. Only plain
    // single-line scalars are read; surrounding quotes are stripped.
    inline std::string ReadFrontmatterField(const std::filesystem::path& file,
                                            const std::string& field)
    {
        std::ifstream stream(file);
        std::string line;
        if (!std::getline(stream, line) || line.rfind("---", 0) != 0)
            return {};

        const std::string prefix = field + ":";
        while (std::getline(stream, line))
        {
            if (line.rfind("---", 0) == 0) break;
            if (line.rfind(prefix, 0) != 0) continue;

            std::string value = line.substr(prefix.size());
            const auto first = value.find_first_not_of(" \t");
            const auto last  = value.find_last_not_of(" \t\r");
            if (first == std::string::npos) return {};
            value = value.substr(first, last - first + 1);
            if (value.size() >= 2
                && (value.front() == '"' || value.front() == '\'')
                && value.back() == value.front())
                value = value.substr(1, value.size() - 2);
            return value;
        }
        return {};
    }

    // Override resolution for families where one name has one winner
    // (MCP servers, skills, agents). keys[i] is the identity of nodes[i];
    // rank orders layers, higher wins. Losers stay in the list, marked
    // shadowed and annotated with the winning layer, so the view can show
    // both. Ties go to the node that comes first.
    inline void MarkShadowed(
        std::vector<core::ConfigNode>&                     nodes,
        const std::vector<std::string>&                    keys,
        const std::function<int(core::ConfigLayerKind)>&   rank)
    {
        for (std::size_t i = 0; i < nodes.size(); ++i)
        {
            std::size_t winner = i;
            for (std::size_t j = 0; j < nodes.size(); ++j)
            {
                if (keys[j] != keys[i]) continue;
                const int rw = rank(nodes[winner].sourceLayer);
                const int rj = rank(nodes[j].sourceLayer);
                if (rj > rw || (rj == rw && j < winner))
                    winner = j;
            }
            if (winner == i) continue;

            nodes[i].shadowed = true;
            nodes[i].note = std::string("shadowed by ")
                            + core::LayerName(nodes[winner].sourceLayer);
        }
    }
}
