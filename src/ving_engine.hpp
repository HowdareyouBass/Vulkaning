#pragma once

#include <vulkan/vulkan.hpp>

#include <SDL3/SDL_vulkan.h>

namespace ving
{
class Engine
{
  public:
    Engine();

    static constexpr bool enable_validation_layers = true;

    static constexpr int start_window_width = 800;
    static constexpr int start_window_height = 800;

    std::vector<const char *> required_layers{};
    std::vector<const char *> required_instance_extensions{};
    std::vector<const char *> required_device_extensions{VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                                         VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  public:
    void run();

  private:
    void draw();

    void init_window();
    void init_vulkan();

  private:
    vk::Extent2D m_window_extent{start_window_width, start_window_height};

    SDL_Window *m_window;

    vk::UniqueInstance m_instance;
    vk::UniqueSurfaceKHR m_surface;
    vk::PhysicalDevice m_physical_device;
    vk::UniqueDevice m_device;

    vk::Queue m_graphics_queue;
    vk::Queue m_present_queue;

    vk::UniqueSwapchainKHR m_swapchain;
};
} // namespace ving
