#include "diff_engine.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace dev_dash::services
{
    namespace
    {
        std::string ReadFile(const std::filesystem::path& p)
        {
            std::ifstream f(p, std::ios::binary);
            return std::string(std::istreambuf_iterator<char>(f), {});
        }

        using FileMap = std::unordered_map<std::string, std::filesystem::path>;

        FileMap CollectFiles(const std::filesystem::path& root)
        {
            FileMap result;
            if (root.empty() || !std::filesystem::exists(root))
                return result;

            std::error_code ec;
            for (const auto& entry :
                 std::filesystem::recursive_directory_iterator(root, ec))
            {
                if (!entry.is_regular_file(ec) || ec)
                {
                    ec.clear();
                    continue;
                }
                std::string rel =
                    std::filesystem::relative(entry.path(), root, ec).string();
                if (!ec)
                    result.emplace(std::move(rel), entry.path());
            }
            return result;
        }
    }

    // sourceRoot = scaffold (reference), targetRoot = project.
    // kMissing  — in scaffold, absent from project.
    // kModified — in both, content differs.
    // kUnchanged — in both, content identical.
    // kCustom   — in project only (not in scaffold).
    std::vector<core::DiffEntry> DiffEngine::CompareTrees(
        const std::filesystem::path& sourceRoot,
        const std::filesystem::path& targetRoot,
        bool includeContent)
    {
        const FileMap sourceFiles = CollectFiles(sourceRoot);
        const FileMap targetFiles = CollectFiles(targetRoot);

        std::vector<core::DiffEntry> result;
        result.reserve(sourceFiles.size() + targetFiles.size());

        for (const auto& [rel, srcPath] : sourceFiles)
        {
            core::DiffEntry entry;
            entry.relativePath = rel;

            auto it = targetFiles.find(rel);
            if (it == targetFiles.end())
            {
                entry.kind = core::DiffKind::kMissing;
                if (includeContent)
                    entry.sourceContent = ReadFile(srcPath);
            }
            else
            {
                const std::string srcContent = ReadFile(srcPath);
                const std::string tgtContent = ReadFile(it->second);
                entry.kind = (srcContent == tgtContent)
                    ? core::DiffKind::kUnchanged
                    : core::DiffKind::kModified;
                if (includeContent)
                {
                    entry.sourceContent = srcContent;
                    entry.targetContent = tgtContent;
                }
            }

            result.push_back(std::move(entry));
        }

        for (const auto& [rel, tgtPath] : targetFiles)
        {
            if (sourceFiles.find(rel) != sourceFiles.end()) continue;
            core::DiffEntry entry;
            entry.relativePath = rel;
            entry.kind         = core::DiffKind::kCustom;
            if (includeContent)
                entry.targetContent = ReadFile(tgtPath);
            result.push_back(std::move(entry));
        }

        std::sort(result.begin(), result.end(),
            [](const core::DiffEntry& a, const core::DiffEntry& b)
            { return a.relativePath < b.relativePath; });

        return result;
    }
}
