#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace dev_dash::services
{
    struct LoadResult
    {
        std::string content;
        std::vector<std::filesystem::path> imports;
    };

    // Where a link clicked in a document leads.
    struct LinkTarget
    {
        enum class Kind
        {
            kImport,    // an @-import rewritten by Load() (claudeimport://)
            kDocument,  // an existing markdown file
            kFile,      // an existing file or directory that is not markdown
            kExternal,  // a URL with a scheme (https:, mailto:, ...)
            kAnchor,    // a fragment within the same document (#section)
            kMissing,   // a path that does not exist
        };

        Kind kind = Kind::kMissing;
        std::filesystem::path path;  // kImport, kDocument, kFile, kMissing
        std::string url;             // kExternal
    };

    class DocumentLoader
    {
    public:
        DocumentLoader() = default;

        // Fence-aware: @-imports inside fenced code blocks are not rewritten.
        // Returns empty content + empty imports on read failure.
        LoadResult Load(const std::filesystem::path& filePath);

        // Classifies a link found in `fromDocument`. Relative paths resolve
        // against the document's directory; percent-escapes are decoded and
        // any #fragment or ?query is dropped from paths.
        LinkTarget ResolveLink(const std::filesystem::path& fromDocument,
                               std::string_view url) const;

        // file:// URL for `path`, percent-encoding what a URL cannot carry.
        static std::string ToFileUrl(const std::filesystem::path& path);
    };
}
