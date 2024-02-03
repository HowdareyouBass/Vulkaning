#pragma once

#include <vulkan/vulkan.hpp>

#include <SDL3/SDL_video.h>

#include "vk_types.hpp"

namespace ving
{
class RenderContext
{
    struct QueueFamiliesInfo
    {
        uint32_t graphics_family;
        uint32_t present_family;

        bool is_graphics_and_present_same() { return graphics_family == present_family; }
    };

  public:
    // TODO: Let the user enable or disable layers
    static constexpr bool enable_validation_layers = true;

    RenderContext(SDL_Window *window);

  public:
    vk::PhysicalDeviceMemoryProperties memory_properties;

  private:
    std::vector<const char *> m_required_instance_layers{};
    std::vector<const char *> m_required_instance_extensions{};
    std::vector<const char *> m_required_device_extensions{VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                                           VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::UniqueInstance m_instance;
    vk::PhysicalDevice m_physical_device;

    vk::UniqueSurfaceKHR m_surface;

    QueueFamiliesInfo m_queue_info;
    vk::UniqueDevice m_device;
    vktypes::Swapchain m_swapchain;
};
} // namespace ving
