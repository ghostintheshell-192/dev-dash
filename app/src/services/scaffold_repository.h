#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

#include "../core/scaffold.h"

namespace dev_dash::services
{
    class ScaffoldRepository
    {
    public:
        ScaffoldRepository() = default;

        std::vector<core::Scaffold> List();
        const core::Scaffold* Find(std::string_view name);
        const core::Scaffold* GetDefault();
        void Refresh();

        std::filesystem::path ScaffoldRoot() const { return _scaffoldRoot; }
        void SetScaffoldRoot(std::filesystem::path root) { _scaffoldRoot = std::move(root); _cached = false; }

    private:
        std::filesystem::path _scaffoldRoot;
        std::vector<core::Scaffold> _cache;
        bool _cached = false;
    };
}
