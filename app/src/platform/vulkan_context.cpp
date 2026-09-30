#include "vulkan_context.h"
#include "window.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <SDL3/SDL_vulkan.h>

namespace dev_dash::platform
{
    VulkanContext::VulkanContext(const Window& window)
    {
        // Instance
        {
            uint32_t extensionCount = 0;
            const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
            if (!extensions)
                throw std::runtime_error(std::string("SDL_Vulkan_GetInstanceExtensions failed: ") + SDL_GetError());

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
                builder.require_api_version(VK_API_VERSION_1_1)
                       .enable_extensions(extensionCount, extensions)
                       .build();
            if (!result)
                throw std::runtime_error("Vulkan instance creation failed: " + result.error().message());

            _instance = result.value();
            _deletionQueue.Add([this] { vkb::destroy_instance(_instance); });
        }

        // Surface
        {
            if (!SDL_Vulkan_CreateSurface(window.Handle(), _instance, nullptr, &_surface))
                throw std::runtime_error(std::string("SDL_Vulkan_CreateSurface failed: ") + SDL_GetError());
            _deletionQueue.Add([this] { SDL_Vulkan_DestroySurface(_instance, _surface, nullptr); });
        }

        // Physical device
        {
            vkb::PhysicalDeviceSelector selector(_instance);
            const vkb::Result<vkb::PhysicalDevice> result =
                selector.set_surface(_surface)
                        .require_present(true)
                        .prefer_gpu_device_type(vkb::PreferredDeviceType::integrated)
                        .disable_portability_subset()
                        .select();
            if (!result)
                throw std::runtime_error("Physical device selection failed: " + result.error().message());

            _physicalDevice = result.value();
            std::cout << "Selected GPU: " << _physicalDevice.name << '\n';
        }

        // Logical device + queues
        {
            const std::vector<VkQueueFamilyProperties> families = _physicalDevice.get_queue_families();
            bool graphicsFound = false;
            bool presentFound  = false;
            uint32_t graphicsFamily = 0;
            uint32_t presentFamily  = 0;

            for (std::size_t i = 0; i < families.size() && !(graphicsFound && presentFound); ++i)
            {
                if (!graphicsFound && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                {
                    graphicsFamily = static_cast<uint32_t>(i);
                    graphicsFound  = true;
                }
                if (!presentFound)
                {
                    VkBool32 supported = VK_FALSE;
                    if (vkGetPhysicalDeviceSurfaceSupportKHR(
                            _physicalDevice, static_cast<uint32_t>(i), _surface, &supported) == VK_SUCCESS
                        && supported)
                    {
                        presentFamily = static_cast<uint32_t>(i);
                        presentFound  = true;
                    }
                }
            }

            if (!graphicsFound)
                throw std::runtime_error("No graphics queue family available");
            if (!presentFound)
                throw std::runtime_error("No present queue family available");

            _graphicsQueueFamilyIndex = graphicsFamily;

            std::vector<vkb::CustomQueueDescription> queueDescriptions;
            queueDescriptions.emplace_back(graphicsFamily, std::vector<float>{1.0f});
            if (presentFamily != graphicsFamily)
                queueDescriptions.emplace_back(presentFamily, std::vector<float>{1.0f});

            vkb::DeviceBuilder builder(_physicalDevice);
            const vkb::Result<vkb::Device> result =
                builder.custom_queue_setup(queueDescriptions).build();
            if (!result)
                throw std::runtime_error("Logical device creation failed: " + result.error().message());

            _device = result.value();
            _deletionQueue.Add([this] { vkb::destroy_device(_device); });

            const vkb::Result<VkQueue> graphics = _device.get_queue(vkb::QueueType::graphics);
            if (!graphics)
                throw std::runtime_error("Graphics queue retrieval failed: " + graphics.error().message());
            _graphicsQueue = graphics.value();

            const vkb::Result<VkQueue> present = _device.get_queue(vkb::QueueType::present);
            if (!present)
                throw std::runtime_error("Present queue retrieval failed: " + present.error().message());
            _presentQueue = present.value();
        }
    }
}
