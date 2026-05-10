# Downloads CPM.cmake on first configure and caches it under the build directory.
# Pattern documented at https://github.com/cpm-cmake/CPM.cmake#adding-cpm
#
# Why download on demand instead of vendoring CPM.cmake (~45 KB) in the repo?
# Less noise in source tree, version is explicit, and the cache is build-dir local.

set(CPM_DOWNLOAD_VERSION 0.40.8)

# TODO: pin EXPECTED_HASH once the version is confirmed working — supply-chain hygiene.
# Without it, a compromised GitHub release would go undetected. For PoC bootstrap only.

set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION} to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CPM_DOWNLOAD_LOCATION}"
    )
endif()

include(${CPM_DOWNLOAD_LOCATION})
