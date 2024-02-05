#include "ving_core.hpp"

#include <SDL3/SDL_vulkan.h>
#include <iostream>

#include "ving_utils.hpp"

namespace ving
{
Core::Core(SDL_Window *window) : m_window{window}
{
    if constexpr (enable_validation_layers)
    {
        m_required_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
        m_required_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    uint32_t count = 0;
    const char *const *window_extensions = SDL_Vulkan_GetInstanceExtensions(&count);

    m_required_instance_extensions.reserve(m_required_instance_extensions.size() + count);

    for (uint32_t i = 0; i < count; ++i)
    {
        m_required_instance_extensions.push_back(window_extensions[i]);
    }

    m_instance = utils::create_instance("None", VK_MAKE_VERSION(0, 0, 1), "Vulkaning", VK_MAKE_VERSION(0, 0, 2),
                                        m_required_instance_layers, m_required_instance_extensions);

    std::vector<vk::PhysicalDevice> available_devices = m_instance->enumeratePhysicalDevices();
    m_physical_device = utils::pick_physical_device(available_devices);
    memory_properties = m_physical_device.getMemoryProperties();

    m_surface = utils::create_SDL_window_surface(window, *m_instance);

    std::vector<vk::QueueFamilyProperties> queue_families = m_physical_device.getQueueFamilyProperties();

    m_queue_info.graphics_family = utils::find_queue_family(queue_families, vk::QueueFlagBits::eGraphics);
    if (m_queue_info.graphics_family == queue_families.size())
        throw std::runtime_error("Failed to find graphics queue on current device");

    if (m_physical_device.getSurfaceSupportKHR(m_queue_info.graphics_family, *m_surface))
    {
        m_queue_info.present_family = m_queue_info.graphics_family;
    }
    else
    {
        m_queue_info.present_family = utils::find_present_queue(queue_families, m_physical_device, *m_surface);
        if (m_queue_info.present_family == queue_families.size())
            throw std::runtime_error("Failed to find present queue on current device");
    }

    // HARD: Graphics should have more priority??

    float queue_priority = 1.0f;

    std::vector<vk::DeviceQueueCreateInfo> device_queue_infos = {
        vk::DeviceQueueCreateInfo{}
            .setQueueCount(1)
            .setQueuePriorities(queue_priority)
            .setQueueFamilyIndex(m_queue_info.graphics_family),
    };

    if (!m_queue_info.is_graphics_and_present_same())
    {
        device_queue_infos.push_back(vk::DeviceQueueCreateInfo{}
                                         .setQueueCount(1)
                                         .setQueuePriorities(queue_priority)
                                         .setQueueFamilyIndex(m_queue_info.present_family));
    }

    auto features2 = m_physical_device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
                                                    vk::PhysicalDeviceVulkan12Features>();

    m_device = utils::create_device(m_physical_device, device_queue_infos, m_required_device_extensions,
                                    features2.get<vk::PhysicalDeviceFeatures2>());

    m_command_pool = utils::create_command_pool(*m_device, m_queue_info.graphics_family,
                                                vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
}
vktypes::Swapchain Core::create_swapchain(uint32_t image_count) const
{
    int width = 0, height = 0;
    int res = SDL_GetWindowSize(m_window, &width, &height);
    if (res != 0)
    {
        std::cout << SDL_GetError() << "\n";
    }

    vk::Extent2D window_extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    return utils::create_swapchain(m_physical_device, *m_device, *m_surface, window_extent,
                                   m_queue_info.is_graphics_and_present_same() ? 1 : 2, image_count);
}
vk::UniqueSemaphore Core::create_semaphore() const
{
    return utils::create_semaphore(*m_device);
}
vk::UniqueFence Core::create_fence(bool state) const
{
    auto info = vk::FenceCreateInfo{};

    if (state)
        info.setFlags(vk::FenceCreateFlagBits::eSignaled);

    return m_device->createFenceUnique(info);
}
std::vector<vk::UniqueCommandBuffer> Core::allocate_command_buffers(uint32_t count) const
{
    return utils::allocate_command_buffers(*m_device, *m_command_pool, count);
}
} // namespace ving
