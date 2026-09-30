cmake_minimum_required(VERSION 3.28)

project("dev-dash" VERSION 0.1.1 LANGUAGES C CXX)

# Generated header carrying the project version into C++ (PROJECT_VERSION is
# set by project(... VERSION ...) above). Lands in the build tree's generated/
# dir, which src/ adds to its include path.
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
    ${CMAKE_BINARY_DIR}/generated/version.h
    @ONLY
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ----- Platform / compiler dispatch -----

if(NOT "${CMAKE_SIZEOF_VOID_P}" MATCHES 8)
    message(FATAL_ERROR "32-bit builds are not supported")
endif()

if(LINUX)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        include(cmake/compilers/gcc.cmake)
    else()
        message(FATAL_ERROR "Only GCC is supported on Linux for now (C=${CMAKE_C_COMPILER_ID}, CXX=${CMAKE_CXX_COMPILER_ID})")
    endif()
else()
    message(FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME} not supported yet (Linux only)")
endif()

# ----- Options -----

option(DEVDASH_BUILD_TESTS "Build the dev-dash unit tests (Catch2)" ON)

# ----- CPM -----

include(cmake/get_cpm.cmake)

# ----- Subprojects -----

add_subdirectory(external)
add_subdirectory(src)

if(DEVDASH_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ----- Install rules (GNUInstallDirs) -----
#
# Layout matches platform::ResolveAssetsDir()'s second candidate:
#   <prefix>/bin/dev-dash
#   <prefix>/share/dev-dash/assets/...   (fonts, etc.)
#   <prefix>/share/applications/dev-dash.desktop

include(GNUInstallDirs)

install(TARGETS dev-dash
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Runtime assets. The .desktop file lives in assets/ but belongs under
# share/applications, so it is excluded from the asset tree copy below.
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dev-dash/assets
    PATTERN "dev-dash.desktop" EXCLUDE)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/dev-dash.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

# ----- CPack (TGZ tarball) -----

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_NAME "dev-dash")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "dev-dash-${PROJECT_VERSION}-linux-x86_64")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Documentation-first project dashboard for Claude Code")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/ghostintheshell-192/dev-dash")
include(CPack)
