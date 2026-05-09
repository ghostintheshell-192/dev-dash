#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <filesystem>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

#include "rendering/markdown_r.h"
#include "deletion_queue.h"

inline constexpr bool kEnableVulkanDebug = true;
inline constexpr bool kEnableVSync = true;
inline constexpr std::size_t kMaxFramesInFlight = 3;
inline constexpr int kInitialWindowWidth = 1280;
inline constexpr int kInitialWindowHeight = 800;

class Renderer
{
    struct MarkdownPanel
    {
        std::string title;
        std::string path;
        std::string content;
        bool open = true;
    };

public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;

    int Run();

private:
    bool Init();
    bool MainLoop();

    bool InitSdl();
    bool CreateWindow();
    bool CreateInstance();
    bool CreateSurface();
    bool SelectPhysicalDevice();
    bool CreateDevice();
    bool RetrieveQueues();
    bool CreateSwapchain();
    bool CreateImageViews();
    bool CreateRenderPass();
    bool CreateFramebuffers();
    bool CreateCommandPool();
    bool CreateCommandBuffers();
    bool CreateSyncObjects();
    bool InitImGui();
    bool RecreateSwapchain();

    void OpenPanel(const std::string &path);
    void RenderMarkdownWindow();

    static void CheckVkResultFn(VkResult err);
    static std::string PreprocessImports(const std::filesystem::path &filePath);

    SDL_Window *_window = nullptr;
    vkb::Instance _instance;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    vkb::PhysicalDevice _physicalDevice;
    vkb::Device _device;

    uint32_t _graphicsQueueFamilyIndex = 0;
    VkQueue _graphicsQueue = VK_NULL_HANDLE;
    VkQueue _presentQueue = VK_NULL_HANDLE;

    vkb::Swapchain _swapchain;
    std::vector<VkImageView> _imageViews;
    VkRenderPass _renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> _framebuffers;

    VkCommandPool _commandPool = VK_NULL_HANDLE;
    std::array<VkCommandBuffer, kMaxFramesInFlight> _commandBuffers{};
    std::array<VkSemaphore, kMaxFramesInFlight> _imageAvailableSemaphores{};
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::array<VkFence, kMaxFramesInFlight> _inFlightFences{};

    bool _swapchainDeleterRegistered = false;
    bool _imageViewsDeleterRegistered = false;
    bool _renderPassDeleterRegistered = false;
    bool _framebuffersDeleterRegistered = false;

    // ----- Markdown panel system -----

    std::vector<std::string> _pendingPanels; // paths queued from link callbacks

    std::vector<MarkdownPanel> _panels;

    DeletionQueue _deletionQueue;

    Rendering::MarkdownFonts _fonts;
    Rendering::MarkdownRenderer _markdown_r;
};
