add_executable(dev-dash-poc)

target_sources(dev-dash-poc
    PRIVATE
        deletion_queue.h
        renderer.h
        renderer.cpp
        main.cpp
)

target_link_libraries(dev-dash-poc
    PRIVATE
        ImGui
        ImGui-Markdown
        ImGui-SDL3
        ImGui-Vulkan
        vk-bootstrap::vk-bootstrap
)

add_custom_command(TARGET dev-dash-poc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:dev-dash-poc>/assets"
    COMMENT "Copying assets to output directory"
)
