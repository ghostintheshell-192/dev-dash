#include "frame_resources.h"
#include "vulkan_context.h"
#include "swapchain.h"

#include <iostream>
#include <stdexcept>

namespace dev_dash::platform
{
    FrameResources::FrameResources(const VulkanContext& context, const Swapchain& swapchain)
        : _device(context.Device())
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = context.GraphicsQueueFamilyIndex();

        if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
            throw std::runtime_error("Command pool creation failed");
        _deletionQueue.Add([this] { vkDestroyCommandPool(_device, _commandPool, nullptr); });

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = _commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

        if (vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("Command buffer allocation failed");

        VkSemaphoreCreateInfo semInfo = {};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        _imageAvailableSemaphores.fill(VK_NULL_HANDLE);
        for (std::size_t i = 0; i < _imageAvailableSemaphores.size(); ++i)
        {
            if (vkCreateSemaphore(_device, &semInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS)
                throw std::runtime_error("Image-available semaphore creation failed");
        }
        _deletionQueue.Add([this]
        {
            for (const VkSemaphore s : _imageAvailableSemaphores)
                vkDestroySemaphore(_device, s, nullptr);
        });

        // One render-finished semaphore per swapchain image (not per frame in flight),
        // per Vulkan spec recommendations to avoid signal-reuse races.
        _renderFinishedSemaphoresPerImage.assign(swapchain.ImageCount(), VK_NULL_HANDLE);
        for (std::size_t i = 0; i < _renderFinishedSemaphoresPerImage.size(); ++i)
        {
            if (vkCreateSemaphore(_device, &semInfo, nullptr, &_renderFinishedSemaphoresPerImage[i]) != VK_SUCCESS)
                throw std::runtime_error("Render-finished semaphore creation failed");
        }
        _deletionQueue.Add([this]
        {
            for (const VkSemaphore s : _renderFinishedSemaphoresPerImage)
                vkDestroySemaphore(_device, s, nullptr);
        });

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        _inFlightFences.fill(VK_NULL_HANDLE);
        for (std::size_t i = 0; i < _inFlightFences.size(); ++i)
        {
            if (vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
                throw std::runtime_error("In-flight fence creation failed");
        }
        _deletionQueue.Add([this]
        {
            for (const VkFence f : _inFlightFences)
                vkDestroyFence(_device, f, nullptr);
        });
    }

    VkSemaphore FrameResources::RenderFinishedSemaphoreForImage(uint32_t imageIndex) const
    {
        return _renderFinishedSemaphoresPerImage[imageIndex];
    }

    void FrameResources::AdvanceFrame()
    {
        _currentFrame = (_currentFrame + 1) % kMaxFramesInFlight;
    }
}
