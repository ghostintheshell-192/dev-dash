#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

namespace dev_dash::platform
{
    class VulkanContext;
    class Window;

    class Swapchain
    {
    public:
        Swapchain(const VulkanContext& context, const Window& window);
        ~Swapchain();

        Swapchain(const Swapchain&)            = delete;
        Swapchain& operator=(const Swapchain&) = delete;

        VkFormat       ImageFormat() const { return _swapchain.image_format; }
        VkExtent2D     Extent()      const { return _swapchain.extent; }
        uint32_t       ImageCount()  const { return _swapchain.image_count; }
        VkSwapchainKHR Handle()      const { return _swapchain.swapchain; }

        VkRenderPass                      RenderPass()   const { return _renderPass; }
        const std::vector<VkFramebuffer>& Framebuffers() const { return _framebuffers; }

        uint32_t RequestedMinImageCount() const { return _swapchain.requested_min_image_count; }

        void Recreate(const VulkanContext& context, const Window& window);

    private:
        void Build(const VulkanContext& context, const Window& window);
        void DestroyDependents();

        vkb::Swapchain             _swapchain;
        std::vector<VkImageView>   _imageViews;
        VkRenderPass               _renderPass   = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> _framebuffers;
        VkDevice                   _device       = VK_NULL_HANDLE;
    };
}
