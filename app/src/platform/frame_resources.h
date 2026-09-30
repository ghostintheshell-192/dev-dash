#pragma once

#include <array>
#include <cstddef>
#include <vector>
#include <vulkan/vulkan.h>

#include "deletion_queue.h"

namespace dev_dash::platform
{
    class VulkanContext;
    class Swapchain;

    inline constexpr std::size_t kMaxFramesInFlight = 3;

    class FrameResources
    {
    public:
        FrameResources(const VulkanContext& context, const Swapchain& swapchain);
        ~FrameResources() = default;

        FrameResources(const FrameResources&)            = delete;
        FrameResources& operator=(const FrameResources&) = delete;

        std::size_t CurrentFrameIndex() const { return _currentFrame; }

        VkCommandBuffer CurrentCommandBuffer()    const { return _commandBuffers[_currentFrame]; }
        VkSemaphore     ImageAvailableSemaphore() const { return _imageAvailableSemaphores[_currentFrame]; }
        VkFence         InFlightFence()           const { return _inFlightFences[_currentFrame]; }

        VkSemaphore RenderFinishedSemaphoreForImage(uint32_t imageIndex) const;

        void AdvanceFrame();

    private:
        VkDevice      _device      = VK_NULL_HANDLE;
        VkCommandPool _commandPool = VK_NULL_HANDLE;
        std::array<VkCommandBuffer, kMaxFramesInFlight> _commandBuffers{};
        std::array<VkSemaphore, kMaxFramesInFlight>     _imageAvailableSemaphores{};
        std::array<VkFence, kMaxFramesInFlight>         _inFlightFences{};
        std::vector<VkSemaphore> _renderFinishedSemaphoresPerImage;
        std::size_t _currentFrame = 0;
        DeletionQueue _deletionQueue;
    };
}
