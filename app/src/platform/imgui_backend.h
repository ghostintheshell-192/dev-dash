#pragma once

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>

namespace dev_dash::platform
{
    class Window;
    class VulkanContext;
    class Swapchain;

    class ImGuiBackend
    {
    public:
        ImGuiBackend(const Window& window,
                     const VulkanContext& context,
                     const Swapchain& swapchain);
        ~ImGuiBackend();

        ImGuiBackend(const ImGuiBackend&)            = delete;
        ImGuiBackend& operator=(const ImGuiBackend&) = delete;

        void ProcessEvent(const SDL_Event& event);
        void NewFrame();
        void Render(VkCommandBuffer cmdBuffer);

        static void CheckVkResultFn(VkResult err);
    };
}
