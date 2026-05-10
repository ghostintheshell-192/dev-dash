#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace dev_dash::services
{
    struct LoadResult
    {
        std::string content;
        std::vector<std::filesystem::path> imports;
    };

    class DocumentLoader
    {
    public:
        DocumentLoader() = default;

        // Fence-aware: @-imports inside fenced code blocks are not rewritten.
        // Returns empty content + empty imports on read failure.
        LoadResult Load(const std::filesystem::path& filePath);
    };
}
