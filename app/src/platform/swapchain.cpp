#include "swapchain.h"
#include "vulkan_context.h"
#include "window.h"

#include <iostream>
#include <stdexcept>

namespace dev_dash::platform
{
    inline constexpr bool kEnableVSync = true;

    Swapchain::Swapchain(const VulkanContext& context, const Window& window)
        : _device(context.Device())
    {
        Build(context, window);
    }

    Swapchain::~Swapchain()
    {
        DestroyDependents();
        vkb::destroy_swapchain(_swapchain);
    }

    void Swapchain::Recreate(const VulkanContext& context, const Window& window)
    {
        vkDeviceWaitIdle(_device);
        DestroyDependents();
        Build(context, window);
    }

    void Swapchain::Build(const VulkanContext& context, const Window& window)
    {
        int width = 0;
        int height = 0;
        window.GetDrawableSize(width, height);

        constexpr VkPresentModeKHR presentMode =
            kEnableVSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;

        vkb::SwapchainBuilder builder(context.DeviceWrapper());
        const vkb::Result<vkb::Swapchain> result =
            builder.set_desired_min_image_count(3)
                   .set_desired_extent(static_cast<uint32_t>(width), static_cast<uint32_t>(height))
                   .set_desired_format({VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR})
                   .add_fallback_format({VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR})
                   .set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                   .set_desired_present_mode(presentMode)
                   .set_old_swapchain(_swapchain)
                   .build();
        if (!result)
            throw std::runtime_error("Swapchain creation failed: " + result.error().message());

        vkb::destroy_swapchain(_swapchain);
        _swapchain = result.value();

        // Image views
        const vkb::Result<std::vector<VkImageView>> views = _swapchain.get_image_views();
        if (!views)
            throw std::runtime_error("Swapchain image view creation failed: " + views.error().message());
        _imageViews = views.value();

        // Render pass
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format         = _swapchain.image_format;
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        // Clear: areas not covered by ImGui show uninitialized GPU memory otherwise.
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
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

        VkRenderPassCreateInfo rpInfo = {};
        rpInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpInfo.attachmentCount = 1;
        rpInfo.pAttachments    = &colorAttachment;
        rpInfo.subpassCount    = 1;
        rpInfo.pSubpasses      = &subpass;
        rpInfo.dependencyCount = 1;
        rpInfo.pDependencies   = &dependency;

        if (vkCreateRenderPass(_device, &rpInfo, nullptr, &_renderPass) != VK_SUCCESS)
            throw std::runtime_error("Render pass creation failed");

        // Framebuffers
        _framebuffers.resize(_imageViews.size(), VK_NULL_HANDLE);
        for (std::size_t i = 0; i < _imageViews.size(); ++i)
        {
            const VkImageView attachments[] = {_imageViews[i]};
            VkFramebufferCreateInfo fbInfo  = {};
            fbInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass      = _renderPass;
            fbInfo.attachmentCount = 1;
            fbInfo.pAttachments    = attachments;
            fbInfo.width           = _swapchain.extent.width;
            fbInfo.height          = _swapchain.extent.height;
            fbInfo.layers          = 1;

            if (vkCreateFramebuffer(_device, &fbInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("Framebuffer creation failed");
        }
    }

    void Swapchain::DestroyDependents()
    {
        for (const VkFramebuffer fb : _framebuffers)
            vkDestroyFramebuffer(_device, fb, nullptr);
        _framebuffers.clear();

        if (_renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(_device, _renderPass, nullptr);
            _renderPass = VK_NULL_HANDLE;
        }

        _swapchain.destroy_image_views(_imageViews);
        _imageViews.clear();
    }
}
