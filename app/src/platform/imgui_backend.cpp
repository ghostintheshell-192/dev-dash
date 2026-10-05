#include "imgui_backend.h"
#include "window.h"
#include "vulkan_context.h"
#include "swapchain.h"

#include <clocale>
#include <iostream>
#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

namespace dev_dash::platform
{
    ImGuiBackend::ImGuiBackend(const Window& window,
                               const VulkanContext& context,
                               const Swapchain& swapchain)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigWindowsResizeFromEdges = true;
        // No layout file here: the app sets one once it knows where it goes.
        io.IniFilename = nullptr;

        ImGui::GetPlatformIO().Platform_LocaleDecimalPoint =
            static_cast<unsigned char>(*std::localeconv()->decimal_point);

        ImGui::StyleColorsDark();

        if (!ImGui_ImplSDL3_InitForVulkan(window.Handle()))
            throw std::runtime_error("ImGui SDL3 backend init failed");

        ImGui_ImplVulkan_InitInfo info = {};
        info.Instance              = context.Instance();
        info.PhysicalDevice        = context.PhysicalDevice();
        info.Device                = context.Device();
        info.QueueFamily           = context.GraphicsQueueFamilyIndex();
        info.Queue                 = context.GraphicsQueue();
        info.DescriptorPoolSize    = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
        info.MinImageCount         = swapchain.RequestedMinImageCount();
        info.ImageCount            = swapchain.ImageCount();
        info.PipelineInfoMain.RenderPass  = swapchain.RenderPass();
        info.PipelineInfoMain.Subpass     = 0;
        info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        info.CheckVkResultFn       = &ImGuiBackend::CheckVkResultFn;

        if (!ImGui_ImplVulkan_Init(&info))
            throw std::runtime_error("ImGui Vulkan backend init failed");
    }

    ImGuiBackend::~ImGuiBackend()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
    }

    void ImGuiBackend::ProcessEvent(const SDL_Event& event)
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    void ImGuiBackend::NewFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiBackend::Render(VkCommandBuffer cmdBuffer)
    {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);
    }

    void ImGuiBackend::CheckVkResultFn(VkResult err)
    {
        if (err != VK_SUCCESS)
            std::cerr << "[error] ImGui-Vulkan check: VkResult = " << err << '\n';
    }
}
