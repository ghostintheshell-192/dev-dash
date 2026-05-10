#include "config_file_scanner.h"

#include <algorithm>
#include <fstream>
#include <string>

namespace dev_dash::services
{
    std::vector<std::filesystem::path> ConfigFileScanner::Scan(
        const std::filesystem::path& dir,
        const std::function<bool(const std::filesystem::path&)>& predicate,
        bool recursive) const
    {
        std::vector<std::filesystem::path> result;
        if (dir.empty() || !std::filesystem::exists(dir))
            return result;

        std::error_code ec;

        auto collect = [&](const auto& entry)
        {
            if (entry.is_regular_file(ec) && !ec && predicate(entry.path()))
                result.push_back(entry.path());
            ec.clear();
        };

        if (recursive)
        {
            for (const auto& e :
                 std::filesystem::recursive_directory_iterator(dir, ec))
                collect(e);
        }
        else
        {
            for (const auto& e : std::filesystem::directory_iterator(dir, ec))
                collect(e);
        }

        std::sort(result.begin(), result.end());
        return result;
    }

    std::vector<std::filesystem::path> ConfigFileScanner::ResolveAtIncludes(
        const std::filesystem::path& file,
        int maxDepth) const
    {
        std::vector<std::filesystem::path> result;
        CollectAtIncludes(file, 0, maxDepth, result);
        return result;
    }

    void ConfigFileScanner::CollectAtIncludes(
        const std::filesystem::path& file,
        int depth,
        int maxDepth,
        std::vector<std::filesystem::path>& result) const
    {
        if (depth >= maxDepth) return;
        if (!std::filesystem::exists(file)) return;

        std::ifstream stream(file);
        if (!stream) return;

        const std::filesystem::path dir = file.parent_path();
        std::string line;
        while (std::getline(stream, line))
        {
            const auto start = line.find_first_not_of(" \t");
            if (start == std::string::npos || line[start] != '@') continue;

            std::filesystem::path ref(line.substr(start + 1));
            if (ref.is_relative())
                ref = dir / ref;
            ref = ref.lexically_normal();

            if (!std::filesystem::exists(ref)) continue;
            if (std::find(result.begin(), result.end(), ref) != result.end()) continue;

            result.push_back(ref);
            CollectAtIncludes(ref, depth + 1, maxDepth, result);
        }
    }
}
