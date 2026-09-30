#include "asset_paths.h"

#include <SDL3/SDL.h>

namespace dev_dash::platform
{
    std::filesystem::path ResolveAssetsDir()
    {
        std::filesystem::path base;
        if (const char* p = SDL_GetBasePath())
            base = p;

        const std::filesystem::path adjacent = base / "assets";
        if (std::filesystem::exists(adjacent))
            return adjacent;

        const std::filesystem::path installed =
            base / ".." / "share" / "dev-dash" / "assets";
        if (std::filesystem::exists(installed))
            return installed;

        // Neither found — return the adjacent candidate so the caller surfaces
        // a meaningful "missing assets at <path>" rather than an empty string.
        return adjacent;
    }
}
