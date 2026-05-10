#include "scaffold_repository.h"

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
    }
}
