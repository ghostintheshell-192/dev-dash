#pragma once

#include <filesystem>

namespace dev_dash::platform
{
    // Resolve the bundled assets directory, trying two layouts in order:
    //   1. <executable dir>/assets        — build tree and portable/tarball runs
    //   2. <executable dir>/../share/dev-dash/assets — GNUInstallDirs install
    // Returns the first that exists; falls back to layout 1 so callers always
    // get a usable (if possibly missing) path to report against.
    std::filesystem::path ResolveAssetsDir();
}
