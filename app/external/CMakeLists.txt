set(CMAKE_FOLDER "external")

# ----- nlohmann/json -----
#
# Header-only JSON library used by SettingsParser to read settings.json
# (MCP servers and hooks configuration).

CPMAddPackage(
    NAME nlohmann_json
    VERSION 3.11.3
    GITHUB_REPOSITORY nlohmann/json
    OPTIONS
        "JSON_BuildTests OFF"
        "JSON_Install OFF"
)

# ----- SDL3 -----
#
# Built from source via CPM. Debian 12 (bookworm) does not yet ship libsdl3-dev
# (SDL3 stable arrived early 2025 and lands in Debian with trixie). Pulling
# SDL3 in via CPM removes the version-of-system dependency and gives us the
# same behaviour on any reasonably-recent Linux.
#
# Builds as a static library to avoid worrying about runtime .so paths.
# SDL_TESTS / SDL_EXAMPLES off — we only want the library.

CPMAddPackage(
    NAME SDL3
    VERSION 3.2.20
    GITHUB_REPOSITORY libsdl-org/SDL
    GIT_TAG release-3.2.20
    OPTIONS
        "SDL_TEST_LIBRARY OFF"
        "SDL_TESTS OFF"
        "SDL_EXAMPLES OFF"
        "SDL_SHARED OFF"
        "SDL_STATIC ON"
)

# ----- vk-bootstrap -----
#
# Pinned to 1.3.302 (last release of the 1.3.x branch). vk-bootstrap 1.4.x
# requires Vulkan 1.4 headers, which Debian 12 — even with bookworm-backports —
# does not provide yet. Once we move to a host with Vulkan 1.4 headers
# available (Debian 13 / current Fedora / SDK install), bump this back to 1.4.x.

CPMAddPackage(
    NAME vk-bootstrap
    VERSION 1.3.302
    GITHUB_REPOSITORY charles-lunarg/vk-bootstrap
)

# ----- Dear ImGui (docking branch) -----
#
# Downloaded sources only — we add it to the build manually below so we can
# wire the IMGUI_USER_CONFIG macro to point at our local imgui-config.h.

CPMAddPackage(
    NAME ImGui
    VERSION 1.92.6-docking
    GITHUB_REPOSITORY ocornut/imgui
    DOWNLOAD_ONLY ON
)

# ----- ImGui core static library -----

add_library(ImGui STATIC)

target_sources(ImGui
    PRIVATE
        ${ImGui_SOURCE_DIR}/imgui.cpp
        ${ImGui_SOURCE_DIR}/imgui.h
        ${ImGui_SOURCE_DIR}/imgui_demo.cpp
        ${ImGui_SOURCE_DIR}/imgui_draw.cpp
        ${ImGui_SOURCE_DIR}/imgui_internal.h
        ${ImGui_SOURCE_DIR}/imgui_tables.cpp
        ${ImGui_SOURCE_DIR}/imgui_widgets.cpp
        ${ImGui_SOURCE_DIR}/imstb_rectpack.h
        ${ImGui_SOURCE_DIR}/imstb_textedit.h
        ${ImGui_SOURCE_DIR}/imstb_truetype.h

        ${CMAKE_CURRENT_SOURCE_DIR}/imgui-config.h
)

# Routes ImGui's internal `#ifdef IMGUI_USER_CONFIG \ #include IMGUI_USER_CONFIG`
# to our local imgui-config.h.
target_compile_definitions(ImGui
    PUBLIC
        IMGUI_USER_CONFIG="imgui-config.h"
)

target_include_directories(ImGui
    PUBLIC
        ${ImGui_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ----- ImGui SDL3 backend -----
# (SDL3 itself is built above via CPM; the SDL3::SDL3 / SDL3::SDL3-static
# targets are exported by SDL's own CMake.)

add_library(ImGui-SDL3 STATIC)

target_sources(ImGui-SDL3
    PRIVATE
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_sdl3.h
)

target_link_libraries(ImGui-SDL3
    PUBLIC
        ImGui
        SDL3::SDL3-static
)

target_include_directories(ImGui-SDL3 PUBLIC ${ImGui_SOURCE_DIR}/backends)

# ----- ImGui Vulkan backend -----

find_package(Vulkan REQUIRED)

add_library(ImGui-Vulkan STATIC)

target_sources(ImGui-Vulkan
    PRIVATE
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
)

target_link_libraries(ImGui-Vulkan
    PUBLIC
        ImGui
        Vulkan::Vulkan
)

target_include_directories(ImGui-Vulkan PUBLIC ${ImGui_SOURCE_DIR}/backends)

# ----- MD4C (CommonMark parser, C library) -----
#
# Pulled via CPM. Exports the `md4c` static library target. We disable the
# md2html executable since we only need the parser. The companion target
# `md4c-html` (HTML renderer) is also irrelevant for our ImGui rendering path.
# EXCLUDE_FROM_ALL keeps MD4C's own install rules (static libs, headers,
# CMake and pkg-config files) out of `cmake --install` and the CPack tarball.

CPMAddPackage(
    NAME md4c
    VERSION 0.5.3
    GITHUB_REPOSITORY mity/md4c
    GIT_TAG release-0.5.3
    EXCLUDE_FROM_ALL YES
    OPTIONS
        "BUILD_MD2HTML_EXECUTABLE OFF"
        "BUILD_SHARED_LIBS OFF"
)

# ----- imgui_md (bridge: MD4C events → ImGui rendering) -----
#
# Fork of mekhontsev/imgui_md (MIT, by Dmitry Mekhontsev) maintained at
# DPD85/imgui_md. Upstream is inactive since 2022; the fork tracks ImGui
# API changes (PushFont signature in 1.92.x) and any dev-dash-specific
# extensions. The fork ships its own CMakeLists.txt that defines the
# `imgui_md` target. It builds ImGui/md4c internally by default; we already
# provide both targets above, so we opt out via BUILD_IMGUI/BUILD_MD4C OFF.
# (The target names ImGui/md4c match the fork's IMGUI_TARGET/MD4C_TARGET
# defaults, so those need no overriding.)

# Pinned to DPD85/imgui_md main after PR #2 (standalone CMakeLists) merged.
# For co-development of the fork, temporarily swap GITHUB_REPOSITORY/GIT_TAG
# with: SOURCE_DIR /data/repos/imgui_md
CPMAddPackage(
    NAME imgui_md
    GITHUB_REPOSITORY DPD85/imgui_md
    GIT_TAG 11832f47e0218d15cd1d222b2034e8d0aecc9e6e
    OPTIONS
        "BUILD_IMGUI OFF"
        "BUILD_MD4C OFF"
)

# ----- ImGuiDot (Graphviz DOT → ImGui draw list) -----
#
# Dario's diagram library (DPD85/ImGuiDot, MIT): Graphviz 15 parses DOT and
# computes the layout, ImGuiDot draws it with the ImGui draw list. Base of the
# code graph (feature-code-graph, ADR-014 addendum, ADR-017). We provide ImGui,
# so BUILD_IMGUI OFF (the default IMGUI_TARGET "ImGui" matches our target).
# Graphviz is built from source: bison and flex are needed at configure time.
#
# Pinned to fix/multiline-labels on Valentina's fork: ImGuiDot::Style (colours
# from the ImGui style) plus multi-line labels, pending their merge upstream;
# then switch to DPD85.
# For co-development: -DCPM_ImGuiDot_SOURCE=/path/to/ImGuiDot at configure.
CPMAddPackage(
    NAME ImGuiDot
    GITHUB_REPOSITORY ghostintheshell-192/ImGuiDot
    GIT_TAG 7a602410af62d2e65602a299e6bd9509c39f5d0a
    OPTIONS
        "BUILD_IMGUI OFF"
)

# ----- tree-sitter + C++ grammar (code graph extractor) -----
#
# The base reader of the code graph (ADR-017): tree-sitter parses the source
# files, our resolver links the names. Both are plain C (MIT). Downloaded
# sources only and built here as two small static libraries: the runtime is a
# single amalgamated unit (lib/src/lib.c), the grammar is the generated
# parser.c plus its hand-written scanner.c. Linked by the services layer.

CPMAddPackage(
    NAME tree-sitter
    VERSION 0.26.13
    GITHUB_REPOSITORY tree-sitter/tree-sitter
    DOWNLOAD_ONLY ON
)

add_library(tree-sitter STATIC ${tree-sitter_SOURCE_DIR}/lib/src/lib.c)
target_include_directories(tree-sitter
    PUBLIC  ${tree-sitter_SOURCE_DIR}/lib/include
    PRIVATE ${tree-sitter_SOURCE_DIR}/lib/src ${tree-sitter_SOURCE_DIR}/lib/src/wasm)
# The project builds C without GNU extensions (CMAKE_C_EXTENSIONS OFF): the
# runtime needs the POSIX/BSD declarations its own build enables (fdopen,
# le16toh...), same definitions as tree-sitter's CMakeLists.txt.
target_compile_definitions(tree-sitter
    PRIVATE _POSIX_C_SOURCE=200112L _DEFAULT_SOURCE _BSD_SOURCE _DARWIN_C_SOURCE)
set_target_properties(tree-sitter PROPERTIES C_STANDARD 11)

CPMAddPackage(
    NAME tree-sitter-cpp
    VERSION 0.23.4
    GITHUB_REPOSITORY tree-sitter/tree-sitter-cpp
    DOWNLOAD_ONLY ON
)

add_library(tree-sitter-cpp STATIC
    ${tree-sitter-cpp_SOURCE_DIR}/src/parser.c
    ${tree-sitter-cpp_SOURCE_DIR}/src/scanner.c)
target_include_directories(tree-sitter-cpp PRIVATE ${tree-sitter-cpp_SOURCE_DIR}/src)
target_link_libraries(tree-sitter-cpp PUBLIC tree-sitter)
set_target_properties(tree-sitter-cpp PROPERTIES C_STANDARD 11)

# ----- Catch2 v3 (unit testing — test target only) -----
#
# Pulled only when DEVDASH_BUILD_TESTS is ON. Exports Catch2::Catch2WithMain
# (links a main() that runs the registered tests) and ships the Catch.cmake
# module under <source>/extras for catch_discover_tests — tests/CMakeLists.txt
# adds that directory to CMAKE_MODULE_PATH.

if(DEVDASH_BUILD_TESTS)
    CPMAddPackage(
        NAME Catch2
        VERSION 3.7.1
        GITHUB_REPOSITORY catchorg/Catch2
        GIT_TAG v3.7.1
    )
endif()
