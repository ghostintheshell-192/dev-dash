set(CMAKE_FOLDER "external")

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

find_package(SDL3 REQUIRED)

add_library(ImGui-SDL3 STATIC)

target_sources(ImGui-SDL3
    PRIVATE
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_sdl3.h
)

target_link_libraries(ImGui-SDL3
    PUBLIC
        ImGui
        SDL3::SDL3
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
