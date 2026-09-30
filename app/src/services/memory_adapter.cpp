#include "memory_adapter.h"
#include "adapter_utils.h"

#include <filesystem>
#include <string>

namespace dev_dash::services
{
    namespace
    {
        // Claude Code's encoding of a project path under ~/.claude/projects/:
        // every character that is not an ASCII letter or digit becomes '-'.
        // Paths whose encoding exceeds 200 characters are truncated and given
        // a hash suffix by Claude Code; that case is not reproduced here.
        std::string EncodeProjectPath(const std::filesystem::path& projectPath)
        {
            std::string s = projectPath.string();
            for (char& c : s)
            {
                const bool ascii = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                                   || (c >= '0' && c <= '9');
                if (!ascii)
                    c = '-';
            }
            return s;
        }
    }

    core::ConfigSection MemoryAdapter::Resolve(
        const core::Project& project,
        const std::filesystem::path& homeDir) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kMemory, "Memory", {}, true
        };

        if (homeDir.empty()) return section;

        const auto memoryMd =
            homeDir / ".claude" / "projects"
            / EncodeProjectPath(ProjectRoot(project)) / "memory" / "MEMORY.md";

        if (!std::filesystem::exists(memoryMd)) return section;

        section.nodes.push_back({
            "MEMORY.md",
            core::ConfigLayerKind::kGlobal,
            memoryMd
        });

        return section;
    }
}
