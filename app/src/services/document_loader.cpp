#include "document_loader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string_view>

namespace dev_dash::services
{
    namespace
    {
        constexpr std::string_view kImportScheme = "claudeimport://";
        constexpr std::string_view kFileScheme   = "file://";

        // True for "scheme:..." as in RFC 3986: a letter, then letters,
        // digits, '+', '-' or '.', then ':'.
        bool HasScheme(std::string_view url)
        {
            const auto colon = url.find(':');
            if (colon == std::string_view::npos || colon == 0
                || !std::isalpha(static_cast<unsigned char>(url[0])))
                return false;
            return std::all_of(url.begin(), url.begin() + colon, [](char c)
            {
                return std::isalnum(static_cast<unsigned char>(c))
                    || c == '+' || c == '-' || c == '.';
            });
        }

        int HexValue(char c)
        {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        }

        std::string PercentDecode(std::string_view text)
        {
            std::string decoded;
            decoded.reserve(text.size());
            for (std::size_t i = 0; i < text.size(); ++i)
            {
                if (text[i] == '%' && i + 2 < text.size())
                {
                    const int high = HexValue(text[i + 1]);
                    const int low  = HexValue(text[i + 2]);
                    if (high >= 0 && low >= 0)
                    {
                        decoded += static_cast<char>(high * 16 + low);
                        i += 2;
                        continue;
                    }
                }
                decoded += text[i];
            }
            return decoded;
        }

        bool IsMarkdown(const std::filesystem::path& path)
        {
            std::string ext = path.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            return ext == ".md" || ext == ".markdown";
        }
    }

    LoadResult DocumentLoader::Load(const std::filesystem::path& filePath)
    {
        std::ifstream file(filePath);
        if (!file)
        {
            std::cerr << "[warn] DocumentLoader: could not open: " << filePath << '\n';
            return {};
        }

        const std::filesystem::path dir = filePath.parent_path();
        LoadResult result;
        std::string line;
        bool isCodeBlock = false;

        while (std::getline(file, line))
        {
            if (line.starts_with("```"))
                isCodeBlock = !isCodeBlock;

            const auto firstNonSpace = line.find_first_not_of(" \t");
            if (!isCodeBlock
                && firstNonSpace != std::string::npos
                && line[firstNonSpace] == '@'
                && firstNonSpace + 1 < line.size()
                && line[firstNonSpace + 1] != ' ')
            {
                const std::string importPath = line.substr(firstNonSpace + 1);
                const auto resolved = std::filesystem::weakly_canonical(dir / importPath);
                const std::string label = resolved.filename().string();
                result.content += "[" + label + "](" + std::string(kImportScheme)
                                + resolved.string() + ")\n";
                result.imports.push_back(resolved);
            }
            else
            {
                result.content += line + '\n';
            }
        }

        return result;
    }

    LinkTarget DocumentLoader::ResolveLink(const std::filesystem::path& fromDocument,
                                           std::string_view url) const
    {
        LinkTarget target;

        if (url.starts_with(kImportScheme))
        {
            target.kind = LinkTarget::Kind::kImport;
            target.path = std::string(url.substr(kImportScheme.size()));
            return target;
        }

        if (url.empty() || url.front() == '#')
        {
            target.kind = LinkTarget::Kind::kAnchor;
            return target;
        }

        std::string_view pathPart = url;
        if (url.starts_with(kFileScheme))
            pathPart = url.substr(kFileScheme.size());
        else if (HasScheme(url))
        {
            target.kind = LinkTarget::Kind::kExternal;
            target.url  = std::string(url);
            return target;
        }

        pathPart = pathPart.substr(0, pathPart.find_first_of("#?"));
        const std::filesystem::path linked = PercentDecode(pathPart);
        const std::filesystem::path resolved = std::filesystem::weakly_canonical(
            linked.is_absolute() ? linked : fromDocument.parent_path() / linked);

        target.path = resolved;
        std::error_code ec;
        if (!std::filesystem::exists(resolved, ec))
            target.kind = LinkTarget::Kind::kMissing;
        else if (std::filesystem::is_regular_file(resolved, ec) && IsMarkdown(resolved))
            target.kind = LinkTarget::Kind::kDocument;
        else
            target.kind = LinkTarget::Kind::kFile;
        return target;
    }

    std::string DocumentLoader::ToFileUrl(const std::filesystem::path& path)
    {
        constexpr std::string_view kHex = "0123456789ABCDEF";
        std::string url(kFileScheme);
        for (const unsigned char c : path.string())
        {
            if (std::isalnum(c) || c == '/' || c == '-' || c == '_' || c == '.' || c == '~')
                url += static_cast<char>(c);
            else
            {
                url += '%';
                url += kHex[c >> 4];
                url += kHex[c & 0x0F];
            }
        }
        return url;
    }
}
