add_executable(dev-dash-poc)

target_sources(dev-dash-poc
    PRIVATE
        main.cpp
)

target_link_libraries(dev-dash-poc
    PRIVATE
        ImGui
        ImGui-SDL3
        ImGui-Vulkan
        vk-bootstrap::vk-bootstrap
)
