#pragma once

#include <filesystem>
#include <functional>
#include <vector>

namespace dev_dash::services
{
    class ConfigFileScanner
    {
    public:
        // Scan a directory for files matching predicate.
        // Non-recursive by default; pass recursive=true for subdirectories.
        std::vector<std::filesystem::path> Scan(
            const std::filesystem::path& dir,
            const std::function<bool(const std::filesystem::path&)>& predicate,
            bool recursive = false) const;

        // Follow @path directives in a markdown file and return all referenced
        // files (transitively, up to maxDepth hops). Paths resolved relative to
        // each including file's directory.
        std::vector<std::filesystem::path> ResolveAtIncludes(
            const std::filesystem::path& file,
            int maxDepth = 5) const;

    private:
        void CollectAtIncludes(
            const std::filesystem::path& file,
            int depth,
            int maxDepth,
            std::vector<std::filesystem::path>& result) const;
    };
}
