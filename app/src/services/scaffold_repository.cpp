#include "scaffold_repository.h"

#include <algorithm>
#include <fstream>
#include <string>

namespace dev_dash::services
{
    std::vector<core::Scaffold> ScaffoldRepository::List()
    {
        if (!_cached)
            Refresh();
        return _cache;
    }

    const core::Scaffold* ScaffoldRepository::Find(std::string_view name)
    {
        if (!_cached)
            Refresh();
        for (const auto& s : _cache)
            if (s.name == name)
                return &s;
        return nullptr;
    }

    const core::Scaffold* ScaffoldRepository::GetDefault()
    {
        if (!_cached)
            Refresh();
        for (const auto& s : _cache)
            if (s.isDefault)
                return &s;
        return _cache.empty() ? nullptr : &_cache[0];
    }

    void ScaffoldRepository::Refresh()
    {
        _cache.clear();
        _cached = true;

        if (_scaffoldRoot.empty() || !std::filesystem::exists(_scaffoldRoot))
            return;

        std::error_code ec;
        for (const auto& entry :
             std::filesystem::directory_iterator(_scaffoldRoot, ec))
        {
            if (!entry.is_directory(ec) || ec)
            {
                ec.clear();
                continue;
            }

            core::Scaffold scaffold;
            scaffold.name    = entry.path().filename().string();
            scaffold.path    = entry.path();
            scaffold.isDefault =
                std::filesystem::exists(entry.path() / ".devdash-default");

            // Optional one-line description from README.md heading
            const auto readme = entry.path() / "README.md";
            if (std::filesystem::exists(readme))
            {
                std::ifstream f(readme);
                std::string line;
                while (std::getline(f, line))
                {
                    const auto start = line.find_first_not_of(" \t#");
                    if (start == std::string::npos) continue;
                    scaffold.description = line.substr(start);
                    break;
                }
            }

            _cache.push_back(std::move(scaffold));
        }

        std::sort(_cache.begin(), _cache.end(),
            [](const core::Scaffold& a, const core::Scaffold& b)
            { return a.name < b.name; });
    }

    bool ScaffoldRepository::CreateEmpty(const std::string& name)
    {
        std::error_code ec;
        std::filesystem::create_directories(_scaffoldRoot / name, ec);
        if (ec) return false;
        _cached = false;
        return true;
    }

    bool ScaffoldRepository::CreateCopy(const std::filesystem::path& sourcePath,
                                         const std::string& newName)
    {
        const auto dst = _scaffoldRoot / newName;
        std::error_code ec;
        std::filesystem::copy(sourcePath, dst,
            std::filesystem::copy_options::recursive
            | std::filesystem::copy_options::overwrite_existing, ec);
        if (ec) return false;
        _cached = false;
        return true;
    }

    bool ScaffoldRepository::Delete(const std::filesystem::path& scaffoldPath)
    {
        std::error_code ec;
        std::filesystem::remove_all(scaffoldPath, ec);
        if (ec) return false;
        _cached = false;
        return true;
    }
}
