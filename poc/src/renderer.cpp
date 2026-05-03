// Adapted from Germen Pulchrum (DPD85/Germen @ 037827b, MIT) — `Disegnatore.cpp`.
// Translated to English, restructured into a class, and stripped of:
// custom font loading, theme system, ImPlot, DPI scaling, and i18n.
// See poc/THIRD_PARTY_NOTICES.md for attribution details.

#include "renderer.h"

#include <algorithm>
#include <clocale>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <ranges>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <VkBootstrap.h>

#include "deletion_queue.h"

Renderer::~Renderer()
{
    if (_device.device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(_device);
}

int Renderer::Run()
{
    if (!Init()) return EXIT_FAILURE;
    if (!MainLoop()) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

bool Renderer::Init()
{
    if (!InitSdl())                return false;
    if (!CreateWindow())           return false;
    if (!CreateInstance())         return false;
    if (!CreateSurface())          return false;
    if (!SelectPhysicalDevice())   return false;
    if (!CreateDevice())           return false;
    if (!RetrieveQueues())         return false;
    if (!CreateSwapchain())        return false;
    if (!CreateImageViews())       return false;
    if (!CreateRenderPass())       return false;
    if (!InitImGui())              return false;
    if (!CreateFramebuffers())     return false;
    if (!CreateCommandPool())      return false;
    if (!CreateCommandBuffers())   return false;
    if (!CreateSyncObjects())      return false;
    return true;
}

bool Renderer::InitSdl()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        std::cerr << "[error] SDL_Init failed: " << SDL_GetError() << '\n';
        return false;
    }
    _deletionQueue.Add([] { SDL_Quit(); });
    return true;
}

bool Renderer::CreateWindow()
{
    _window = SDL_CreateWindow(
        "dev-dash PoC",
        kInitialWindowWidth,
        kInitialWindowHeight,
        SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    if (_window == nullptr)
    {
        std::cerr << "[error] SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        return false;
    }
    _deletionQueue.Add([this] { SDL_DestroyWindow(_window); });
    return true;
}

bool Renderer::CreateInstance()
{
    uint32_t           extensionCount = 0;
    const char *const *extensions     = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (extensions == nullptr)
    {
        std::cerr << "[error] SDL_Vulkan_GetInstanceExtensions failed: " << SDL_GetError() << '\n';
        return false;
    }

    vkb::InstanceBuilder builder;

    if constexpr (kEnableVulkanDebug)
    {
        const vkb::Result<vkb::SystemInfo> sysInfo = vkb::SystemInfo::get_system_info();
        if (sysInfo && sysInfo.value().validation_layers_available)
        {
            builder.use_default_debug_messenger();
            builder.enable_validation_layers();
        }
        else
        {
            std::cout << "[warn] Vulkan validation layers not available\n";
        }
    }

    const vkb::Result<vkb::Instance> result =
        builder.require_api_version(VK_API_VERSION_1_1).enable_extensions(extensionCount, extensions).build();
    if (!result)
    {
        std::cerr << "[error] Vulkan instance creation failed: " << result.error().message() << '\n';
        return false;
    }

    _instance = result.value();
    _deletionQueue.Add([this] { vkb::destroy_instance(_instance); });
    return true;
}

bool Renderer::CreateSurface()
{
    if (!SDL_Vulkan_CreateSurface(_window, _instance, nullptr, &_surface))
    {
        std::cerr << "[error] SDL_Vulkan_CreateSurface failed: " << SDL_GetError() << '\n';
        return false;
    }
    _deletionQueue.Add([this] { SDL_Vulkan_DestroySurface(_instance, _surface, nullptr); });
    return true;
}

bool Renderer::SelectPhysicalDevice()
{
    vkb::PhysicalDeviceSelector selector(_instance);
    const vkb::Result<vkb::PhysicalDevice> result =
        selector.set_surface(_surface)
            .require_present(true)
            .prefer_gpu_device_type(vkb::PreferredDeviceType::integrated)
            .disable_portability_subset()
            .select();
    if (!result)
    {
        std::cerr << "[error] Physical device selection failed: " << result.error().message() << '\n';
        return false;
    }

    _physicalDevice = result.value();
    std::cout << "Selected GPU: " << result.value().name << '\n';
    return true;
}

bool Renderer::CreateDevice()
{
    // vk-bootstrap doesn't let us request specific queue types during device
    // selection — we find a graphics-capable family and a present-capable
    // family ourselves and feed them in via custom_queue_setup.
    const std::vector<VkQueueFamilyProperties> families = _physicalDevice.get_queue_families();
    bool     graphicsFound  = false;
    bool     presentFound   = false;
    uint32_t graphicsFamily = 0;
    uint32_t presentFamily  = 0;

    for (size_t i = 0; i < families.size() && !(graphicsFound && presentFound); ++i)
    {
        if (!graphicsFound && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            graphicsFamily = static_cast<uint32_t>(i);
            graphicsFound  = true;
        }
        if (!presentFound)
        {
            VkBool32 supported = VK_FALSE;
            if (vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, static_cast<uint32_t>(i), _surface, &supported)
                    == VK_SUCCESS
                && supported)
            {
                presentFamily = static_cast<uint32_t>(i);
                presentFound  = true;
            }
        }
    }

    if (!graphicsFound) { std::cerr << "[error] No graphics queue family available\n"; return false; }
    if (!presentFound)  { std::cerr << "[error] No present queue family available\n";  return false; }

    _graphicsQueueFamilyIndex = graphicsFamily;

    std::vector<vkb::CustomQueueDescription> queueDescriptions;
    queueDescriptions.emplace_back(graphicsFamily, std::vector<float>{ 1.0f });
    if (presentFamily != graphicsFamily)
        queueDescriptions.emplace_back(presentFamily, std::vector<float>{ 1.0f });

    vkb::DeviceBuilder builder(_physicalDevice);
    const vkb::Result<vkb::Device> result = builder.custom_queue_setup(queueDescriptions).build();
    if (!result)
    {
        std::cerr << "[error] Logical device creation failed: " << result.error().message() << '\n';
        return false;
    }

    _device = result.value();
    _deletionQueue.Add([this] { vkb::destroy_device(_device); });
    return true;
}

bool Renderer::RetrieveQueues()
{
    vkb::Result<VkQueue> graphics = _device.get_queue(vkb::QueueType::graphics);
    if (!graphics)
    {
        std::cerr << "[error] Graphics queue retrieval failed: " << graphics.error().message() << '\n';
        return false;
    }
    _graphicsQueue = graphics.value();

    vkb::Result<VkQueue> present = _device.get_queue(vkb::QueueType::present);
    if (!present)
    {
        std::cerr << "[error] Present queue retrieval failed: " << present.error().message() << '\n';
        return false;
    }
    _presentQueue = present.value();
    return true;
}

bool Renderer::CreateSwapchain()
{
    int width  = 0;
    int height = 0;
    if (!SDL_GetWindowSizeInPixels(_window, &width, &height))
    {
        std::cerr << "[error] SDL_GetWindowSizeInPixels failed: " << SDL_GetError() << '\n';
        return false;
    }

    constexpr VkPresentModeKHR presentMode =
        kEnableVSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;

    vkb::SwapchainBuilder builder(_device);
    const vkb::Result<vkb::Swapchain> result =
        builder.set_desired_min_image_count(3)
            .set_desired_extent(width, height)
            .set_desired_format({ .format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR })
            .add_fallback_format({ .format = VK_FORMAT_R8G8B8A8_UNORM, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR })
            .add_fallback_format({ .format = VK_FORMAT_B8G8R8_UNORM,   .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR })
            .add_fallback_format({ .format = VK_FORMAT_R8G8B8_UNORM,   .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR })
            .set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            .set_desired_present_mode(presentMode)
            .set_old_swapchain(_swapchain)
            .build();
    if (!result)
    {
        std::cerr << "[error] Swapchain creation failed: " << result.error().message() << '\n';
        _swapchain.swapchain = VK_NULL_HANDLE;
        return false;
    }

    vkb::destroy_swapchain(_swapchain);
    _swapchain = result.value();

    if (!_swapchainDeleterRegistered)
    {
        _deletionQueue.Add([this] { vkb::destroy_swapchain(_swapchain); });
        _swapchainDeleterRegistered = true;
    }
    return true;
}

bool Renderer::CreateImageViews()
{
    const vkb::Result<std::vector<VkImageView>> result = _swapchain.get_image_views();
    if (!result)
    {
        std::cerr << "[error] Swapchain image view creation failed: " << result.error().message() << '\n';
        return false;
    }
    _imageViews = result.value();

    if (!_imageViewsDeleterRegistered)
    {
        _deletionQueue.Add([this] { _swapchain.destroy_image_views(_imageViews); });
        _imageViewsDeleterRegistered = true;
    }
    return true;
}

bool Renderer::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format  = _swapchain.image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // Clear instead of DONT_CARE: areas not covered by ImGui draw calls (e.g.
    // when no fullscreen dockspace is present) would otherwise show
    // uninitialized GPU memory and flicker across swapchain images.
    colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments    = &colorAttachment;
    info.subpassCount    = 1;
    info.pSubpasses      = &subpass;
    info.dependencyCount = 1;
    info.pDependencies   = &dependency;

    if (vkCreateRenderPass(_device, &info, nullptr, &_renderPass) != VK_SUCCESS)
    {
        std::cerr << "[error] Render pass creation failed\n";
        return false;
    }

    if (!_renderPassDeleterRegistered)
    {
        _deletionQueue.Add([this] { vkDestroyRenderPass(_device, _renderPass, nullptr); });
        _renderPassDeleterRegistered = true;
    }
    return true;
}

bool Renderer::CreateFramebuffers()
{
    _framebuffers.resize(_imageViews.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < _imageViews.size(); ++i)
    {
        const VkImageView attachments[] = { _imageViews[i] };

        VkFramebufferCreateInfo info = {};
        info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass      = _renderPass;
        info.attachmentCount = 1;
        info.pAttachments    = attachments;
        info.width           = _swapchain.extent.width;
        info.height          = _swapchain.extent.height;
        info.layers          = 1;

        if (vkCreateFramebuffer(_device, &info, nullptr, &_framebuffers[i]) != VK_SUCCESS)
        {
            std::cerr << "[error] Framebuffer " << i << " creation failed\n";
            return false;
        }
    }

    if (!_framebuffersDeleterRegistered)
    {
        _deletionQueue.Add([this] {
            for (const VkFramebuffer fb : _framebuffers)
                vkDestroyFramebuffer(_device, fb, nullptr);
        });
        _framebuffersDeleterRegistered = true;
    }
    return true;
}

bool Renderer::CreateCommandPool()
{
    VkCommandPoolCreateInfo info = {};
    info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = _graphicsQueueFamilyIndex;

    if (vkCreateCommandPool(_device, &info, nullptr, &_commandPool) != VK_SUCCESS)
    {
        std::cerr << "[error] Command pool creation failed\n";
        return false;
    }
    _deletionQueue.Add([this] { vkDestroyCommandPool(_device, _commandPool, nullptr); });
    return true;
}

bool Renderer::CreateCommandBuffers()
{
    VkCommandBufferAllocateInfo info = {};
    info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool        = _commandPool;
    info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

    if (vkAllocateCommandBuffers(_device, &info, _commandBuffers.data()) != VK_SUCCESS)
    {
        std::cerr << "[error] Command buffer allocation failed\n";
        return false;
    }
    return true;
}

bool Renderer::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semInfo = {};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    _imageAvailableSemaphores.fill(VK_NULL_HANDLE);
    for (size_t i = 0; i < _imageAvailableSemaphores.size(); ++i)
    {
        if (vkCreateSemaphore(_device, &semInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS)
        {
            std::cerr << "[error] Image-available semaphore " << i << " creation failed\n";
            return false;
        }
    }
    _deletionQueue.Add([this] {
        for (const VkSemaphore s : _imageAvailableSemaphores)
            vkDestroySemaphore(_device, s, nullptr);
    });

    // One render-finished semaphore per swapchain image (not per frame in flight),
    // per Vulkan spec recommendations to avoid signal-reuse races.
    _renderFinishedSemaphores.assign(_swapchain.image_count, VK_NULL_HANDLE);
    for (size_t i = 0; i < _renderFinishedSemaphores.size(); ++i)
    {
        if (vkCreateSemaphore(_device, &semInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            std::cerr << "[error] Render-finished semaphore " << i << " creation failed\n";
            return false;
        }
    }
    _deletionQueue.Add([this] {
        for (const VkSemaphore s : _renderFinishedSemaphores)
            vkDestroySemaphore(_device, s, nullptr);
    });

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    _inFlightFences.fill(VK_NULL_HANDLE);
    for (size_t i = 0; i < _inFlightFences.size(); ++i)
    {
        if (vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
        {
            std::cerr << "[error] In-flight fence " << i << " creation failed\n";
            return false;
        }
    }
    _deletionQueue.Add([this] {
        for (const VkFence f : _inFlightFences)
            vkDestroyFence(_device, f, nullptr);
    });

    return true;
}

bool Renderer::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    _deletionQueue.Add([] { ImGui::DestroyContext(); });

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigWindowsResizeFromEdges = true;
    io.IniFilename                  = nullptr;

    ImGui::GetPlatformIO().Platform_LocaleDecimalPoint =
        static_cast<unsigned char>(*std::localeconv()->decimal_point);

    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL3_InitForVulkan(_window))
    {
        std::cerr << "[error] ImGui SDL3 backend init failed\n";
        return false;
    }
    _deletionQueue.Add([] { ImGui_ImplSDL3_Shutdown(); });

    ImGui_ImplVulkan_InitInfo info    = {};
    info.Instance                     = _instance;
    info.PhysicalDevice               = _physicalDevice;
    info.Device                       = _device;
    info.QueueFamily                  = _graphicsQueueFamilyIndex;
    info.Queue                        = _graphicsQueue;
    info.DescriptorPoolSize           = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
    info.MinImageCount                = _swapchain.requested_min_image_count;
    info.ImageCount                   = _swapchain.image_count;
    info.PipelineInfoMain.RenderPass  = _renderPass;
    info.PipelineInfoMain.Subpass     = 0;
    info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.CheckVkResultFn              = &Renderer::CheckVkResultFn;

    if (!ImGui_ImplVulkan_Init(&info))
    {
        std::cerr << "[error] ImGui Vulkan backend init failed\n";
        return false;
    }
    _deletionQueue.Add([] { ImGui_ImplVulkan_Shutdown(); });

    return true;
}

bool Renderer::RecreateSwapchain()
{
    if (vkDeviceWaitIdle(_device) != VK_SUCCESS)
        std::cerr << "[warn] vkDeviceWaitIdle failed before swapchain recreation\n";

    for (const VkFramebuffer fb : _framebuffers)
        vkDestroyFramebuffer(_device, fb, nullptr);
    vkDestroyRenderPass(_device, _renderPass, nullptr);
    for (const VkImageView v : _imageViews)
        vkDestroyImageView(_device, v, nullptr);

    return CreateSwapchain() && CreateImageViews() && CreateRenderPass() && CreateFramebuffers();
}

bool Renderer::MainLoop()
{
    SDL_ShowWindow(_window);

    bool   exitRequested  = false;
    bool   paused         = false;
    bool   recreateNeeded = false;
    size_t frameIndex     = 0;

    while (!exitRequested)
    {
        // Use SDL_WaitEvent while paused (window minimized/hidden) to suspend
        // drawing entirely; SDL_PollEvent otherwise so the render loop runs.
        SDL_Event event;
        while ((!paused && SDL_PollEvent(&event)) || (paused && SDL_WaitEvent(&event)))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type)
            {
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                case SDL_EVENT_QUIT:
                    exitRequested = true;
                    break;
                case SDL_EVENT_WINDOW_MINIMIZED:
                case SDL_EVENT_WINDOW_HIDDEN:
                    paused = true;
                    break;
                case SDL_EVENT_WINDOW_RESTORED:
                case SDL_EVENT_WINDOW_SHOWN:
                    paused = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    recreateNeeded = true;
                    break;
                default:
                    break;
            }
            if (exitRequested) break;
        }
        if (exitRequested) break;

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();

        if (vkWaitForFences(_device, 1, &_inFlightFences[frameIndex], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
        {
            std::cerr << "[error] vkWaitForFences failed\n";
            return false;
        }

        uint32_t       imageIndex = 0;
        const VkResult acquire = vkAcquireNextImageKHR(
            _device, _swapchain, UINT64_MAX, _imageAvailableSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex);
        if (acquire == VK_ERROR_OUT_OF_DATE_KHR || acquire == VK_SUBOPTIMAL_KHR)
        {
            recreateNeeded = true;
        }
        else if (acquire != VK_SUCCESS)
        {
            std::cerr << "[error] vkAcquireNextImageKHR failed\n";
            return false;
        }

        if (vkResetFences(_device, 1, &_inFlightFences[frameIndex]) != VK_SUCCESS)
        {
            std::cerr << "[error] vkResetFences failed\n";
            return false;
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (vkBeginCommandBuffer(_commandBuffers[frameIndex], &beginInfo) != VK_SUCCESS)
        {
            std::cerr << "[error] vkBeginCommandBuffer failed\n";
            return false;
        }

        constexpr VkClearValue clearColor = { .color = { .float32 = { 0.06f, 0.06f, 0.06f, 1.0f } } };

        VkRenderPassBeginInfo rpBegin = {};
        rpBegin.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBegin.renderPass        = _renderPass;
        rpBegin.framebuffer       = _framebuffers[imageIndex];
        rpBegin.renderArea.offset = { 0, 0 };
        rpBegin.renderArea.extent = _swapchain.extent;
        rpBegin.clearValueCount   = 1;
        rpBegin.pClearValues      = &clearColor;
        vkCmdBeginRenderPass(_commandBuffers[frameIndex], &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _commandBuffers[frameIndex]);

        vkCmdEndRenderPass(_commandBuffers[frameIndex]);

        if (vkEndCommandBuffer(_commandBuffers[frameIndex]) != VK_SUCCESS)
        {
            std::cerr << "[error] vkEndCommandBuffer failed\n";
            return false;
        }

        const VkSemaphore         waitSemaphores[]   = { _imageAvailableSemaphores[frameIndex] };
        const VkSemaphore         signalSemaphores[] = { _renderFinishedSemaphores[imageIndex] };
        constexpr VkPipelineStageFlags waitStages[]  = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submit = {};
        submit.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount   = 1;
        submit.pWaitSemaphores      = waitSemaphores;
        submit.pWaitDstStageMask    = waitStages;
        submit.commandBufferCount   = 1;
        submit.pCommandBuffers      = &_commandBuffers[frameIndex];
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores    = signalSemaphores;

        if (vkQueueSubmit(_graphicsQueue, 1, &submit, _inFlightFences[frameIndex]) != VK_SUCCESS)
        {
            std::cerr << "[error] vkQueueSubmit failed\n";
            return false;
        }

        const VkSwapchainKHR swapchains[] = { _swapchain };
        VkPresentInfoKHR     present      = {};
        present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores    = signalSemaphores;
        present.swapchainCount     = 1;
        present.pSwapchains        = swapchains;
        present.pImageIndices      = &imageIndex;

        const VkResult presentResult = vkQueuePresentKHR(_presentQueue, &present);
        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
        {
            recreateNeeded = true;
        }
        else if (presentResult != VK_SUCCESS)
        {
            std::cerr << "[error] vkQueuePresentKHR failed\n";
            return false;
        }

        if (recreateNeeded)
        {
            if (!RecreateSwapchain()) return false;
            recreateNeeded = false;
        }

        frameIndex = (frameIndex + 1) % kMaxFramesInFlight;
    }

    return true;
}

void Renderer::CheckVkResultFn(VkResult err)
{
    if (err != VK_SUCCESS)
        std::cerr << "[error] ImGui-Vulkan check: VkResult = " << err << '\n';
}
