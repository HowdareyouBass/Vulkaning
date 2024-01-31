#pragma once

#define VULKAN_HPP_NO_TO_STRING
#include <vulkan/vulkan.hpp>

#include <SDL3/SDL_vulkan.h>

#include "ving_image.hpp"

namespace ving
{
class Engine
{
  public:
    struct FrameData
    {
        vk::UniqueCommandPool command_pool;
        vk::UniqueCommandBuffer command_buffer;

        vk::UniqueSemaphore swapchain_semaphore, render_semaphore;
        vk::UniqueFence render_fence;
    };
    struct QueueFamilyInfo
    {
        uint32_t graphics_family;
        uint32_t present_family;
    };

    Engine();
    ~Engine();

    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;
    Engine(Engine &&) = delete;
    Engine &operator=(Engine &&) = delete;

    // HARD: More frames in flight (tripple buffering)
    static constexpr int frames_in_flight = 2;
    static constexpr bool enable_validation_layers = true;

    static constexpr int start_window_width = 800;
    static constexpr int start_window_height = 600;

    // TODO: Maybe abstract this into device class
    vk::PhysicalDeviceMemoryProperties memory_properties;

    std::vector<const char *> required_layers{};
    std::vector<const char *> required_instance_extensions{};
    std::vector<const char *> required_device_extensions{VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                                         VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  public:
    void run();

    FrameData &get_current_frame() { return m_frames[m_frame_number % frames_in_flight]; }

  private:
    void draw();

    void init_window();
    void init_vulkan();
    void init_frames();

  private:
    vk::Extent2D m_window_extent{start_window_width, start_window_height};

    SDL_Window *m_window;

    vk::UniqueInstance m_instance;
    vk::UniqueSurfaceKHR m_surface;
    vk::PhysicalDevice m_physical_device;
    vk::UniqueDevice m_device;

    QueueFamilyInfo m_queue_family_info;

    vk::Queue m_graphics_queue;
    vk::Queue m_present_queue;

    vk::UniqueSwapchainKHR m_swapchain;
    vk::Format m_swapchain_image_format;
    vk::Extent2D m_swapchain_extent;
    std::vector<vk::Image> m_swapchain_images;
    std::vector<vk::UniqueImageView> m_swapchain_image_views;

    uint32_t m_frame_number{0};
    std::array<FrameData, frames_in_flight> m_frames;

    Image2D m_draw_image;
    vk::Extent2D m_draw_extent;
};
} // namespace ving
