#pragma once

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include "deletion_queue.h"

namespace dev_dash::platform
{
    class Window;

    inline constexpr bool kEnableVulkanDebug = true;

    class VulkanContext
    {
    public:
        explicit VulkanContext(const Window& window);
        ~VulkanContext() = default;

        VulkanContext(const VulkanContext&)            = delete;
        VulkanContext& operator=(const VulkanContext&) = delete;

        const vkb::Instance&       Instance()       const { return _instance; }
        const vkb::PhysicalDevice& PhysicalDevice() const { return _physicalDevice; }
        const vkb::Device&         DeviceWrapper()  const { return _device; }

        VkDevice     Device()                   const { return _device.device; }
        VkSurfaceKHR Surface()                  const { return _surface; }
        uint32_t     GraphicsQueueFamilyIndex() const { return _graphicsQueueFamilyIndex; }
        VkQueue      GraphicsQueue()            const { return _graphicsQueue; }
        VkQueue      PresentQueue()             const { return _presentQueue; }

    private:
        vkb::Instance         _instance;
        vkb::PhysicalDevice   _physicalDevice;
        vkb::Device           _device;
        VkSurfaceKHR          _surface = VK_NULL_HANDLE;
        uint32_t              _graphicsQueueFamilyIndex = 0;
        VkQueue               _graphicsQueue = VK_NULL_HANDLE;
        VkQueue               _presentQueue  = VK_NULL_HANDLE;
        DeletionQueue         _deletionQueue;
    };
}
