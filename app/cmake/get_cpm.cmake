# Downloads CPM.cmake on first configure and caches it under the build directory.
# Pattern documented at https://github.com/cpm-cmake/CPM.cmake#adding-cpm
#
# Why download on demand instead of vendoring CPM.cmake (~45 KB) in the repo?
# Less noise in source tree, version is explicit, and the cache is build-dir local.

# Keep at least the version used by our dependencies (ImGuiDot ships 0.42.0):
# an older one makes CPM warn that a dependency uses a more recent CPM.
set(CPM_DOWNLOAD_VERSION 0.42.0)
# SHA-256 of the release file, as published in CPM's get_cpm.cmake: a tampered
# download fails the configure instead of running.
set(CPM_HASH_SUM "2020b4fc42dba44817983e06342e682ecfc3d2f484a581f11cc5731fbe4dce8a")

set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION} to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CPM_DOWNLOAD_LOCATION}"
        EXPECTED_HASH SHA256=${CPM_HASH_SUM}
    )
endif()

include(${CPM_DOWNLOAD_LOCATION})
