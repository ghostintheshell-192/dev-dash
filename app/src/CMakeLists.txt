add_executable(dev-dash)

target_sources(dev-dash PRIVATE main.cpp)

target_precompile_headers(dev-dash PRIVATE pch.h)

# Generated version.h (configure_file in the top-level CMakeLists).
target_include_directories(dev-dash PRIVATE ${CMAKE_BINARY_DIR}/generated)

add_subdirectory(core)
add_subdirectory(services)
add_subdirectory(ui)
add_subdirectory(platform)
add_subdirectory(app)

target_link_libraries(dev-dash
    PRIVATE
        dev-dash-warnings
        dev-dash-services
        ImGui
        imgui_md
        ImGui-Dot
        ImGui-SDL3
        ImGui-Vulkan
        vk-bootstrap::vk-bootstrap
)

add_custom_command(TARGET dev-dash POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:dev-dash>/assets"
    COMMENT "Copying assets to output directory"
)
