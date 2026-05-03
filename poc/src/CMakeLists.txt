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
