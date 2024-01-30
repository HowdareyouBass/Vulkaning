#include "ving_engine.hpp"

#include <SDL3/SDL.h>
#include <iostream>

#include "ving_utils.hpp"

namespace ving
{
Engine::Engine()
{
    if constexpr (enable_validation_layers)
    {
        required_layers.push_back("VK_LAYER_KHRONOS_validation");
        required_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    init_window();
    init_vulkan();
}

void Engine::run()
{
    SDL_Event event;
    bool running = true;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            // TODO: Stop rendering on window minimized
        }
    }
}

void Engine::draw()
{
}
void Engine::init_window()
{
    if (SDL_Init(SDL_InitFlags::SDL_INIT_VIDEO) < 0)
    {
        throw std::runtime_error(std::format("Couldn't initialize SDL: {}", SDL_GetError()));
    }

    m_window = SDL_CreateWindow("Vulkaning", m_window_extent.width, m_window_extent.height, SDL_WINDOW_VULKAN);

    if (!m_window)
    {
        throw std::runtime_error(std::format("Failed to create SDL window: {}", SDL_GetError()));
    }

    uint32_t count = 0;
    const char *const *window_extensions = SDL_Vulkan_GetInstanceExtensions(&count);

    required_instance_extensions.reserve(required_instance_extensions.size() + count);
    for (uint32_t i = 0; i < count; ++i)
    {
        required_instance_extensions.push_back(window_extensions[i]);
    }
}
void Engine::init_vulkan()
{
    m_instance = utils::create_instance("None", VK_MAKE_VERSION(0, 0, 1), "Vulkaning", VK_MAKE_VERSION(0, 0, 1),
                                        required_layers, required_instance_extensions);

    m_surface = utils::create_SDL_window_surface(m_window, *m_instance);

    std::vector<vk::PhysicalDevice> available_devices = m_instance->enumeratePhysicalDevices();
    m_physical_device = utils::pick_physical_device(available_devices);

    std::vector<vk::QueueFamilyProperties> queue_families = m_physical_device.getQueueFamilyProperties();
    uint32_t graphics_family_index = utils::find_queue_family(queue_families, vk::QueueFlagBits::eGraphics);
    if (graphics_family_index == queue_families.size())
    {
        throw std::runtime_error("Failed to find graphics queue on current device");
    }
    uint32_t present_family_index = utils::find_present_queue(queue_families, m_physical_device, *m_surface);
    if (present_family_index == queue_families.size())
    {
        throw std::runtime_error("Failed to find present queue on current device");
    }

    // HARD: Hardcoded
    float queue_priority = 1.0f;

    std::vector<vk::DeviceQueueCreateInfo> queue_infos = {
        vk::DeviceQueueCreateInfo{}
            .setQueueCount(1)
            .setQueuePriorities(queue_priority)
            .setQueueFamilyIndex(graphics_family_index),
    };

    if (graphics_family_index != present_family_index)
    {
        queue_infos.push_back(vk::DeviceQueueCreateInfo{}
                                  .setQueueCount(1)
                                  .setQueuePriorities(queue_priority)
                                  .setQueueFamilyIndex(present_family_index));
    }

    m_device = utils::create_device(m_physical_device, queue_infos, required_device_extensions);

    // WARN: Could actually be the same queue
    m_graphics_queue = m_device->getQueue(graphics_family_index, 0);
    m_present_queue = m_device->getQueue(present_family_index, 0);

    m_swapchain = utils::create_swapchain(m_physical_device, *m_device, *m_surface, m_window_extent,
                                          graphics_family_index == present_family_index ? 1 : 2);
}
} // namespace ving
