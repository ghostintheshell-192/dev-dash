#include "memory_adapter.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>

namespace dev_dash::services
{
    namespace
    {
        std::string EncodeProjectPath(const std::filesystem::path& projectPath)
        {
            std::string s = projectPath.string();
            std::replace(s.begin(), s.end(), '/', '-');
            return s;
        }
    }

    core::ConfigSection MemoryAdapter::Resolve(const core::Project& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kMemory, "Memory", {}, true
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
}
