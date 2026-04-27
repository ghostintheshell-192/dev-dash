set(CMAKE_FOLDER "external")

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

CPMAddPackage(
    NAME vk-bootstrap
    VERSION 1.4.336
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
