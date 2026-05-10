add_executable(dev-dash)

target_sources(dev-dash PRIVATE main.cpp)

target_precompile_headers(dev-dash PRIVATE pch.h)

add_subdirectory(core)
add_subdirectory(services)
add_subdirectory(ui)
add_subdirectory(platform)
add_subdirectory(app)

target_link_libraries(dev-dash
    PRIVATE
        ImGui
        ImGui-MD
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
