#pragma once

#include <functional>

#include <vulkan/vulkan.hpp>

#include "ving_gpu_buffer.hpp"
#include "ving_image.hpp"
#include "vk_types.hpp"

struct SDL_Window;

namespace ving
{
class Core
{
    struct QueueFamiliesInfo
    {
        uint32_t graphics_family;
        uint32_t present_family;
        uint32_t transfer_family;

        bool is_graphics_and_present_same() const { return graphics_family == present_family; }
    };

  public:
    // HARD: Let the user enable or disable layers
    static constexpr bool enable_validation_layers = true;

    Core(SDL_Window *window);

    Image2D create_image2d(vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage,
                           vk::ImageLayout layout) const;
    GPUBuffer create_gpu_buffer(void *data, uint64_t size, vk::BufferUsageFlags usage) const;
    vktypes::Swapchain create_swapchain(uint32_t image_count) const;
    vk::UniqueSemaphore create_semaphore() const;
    vk::UniqueFence create_fence(bool state) const;
    std::vector<vk::UniqueCommandBuffer> allocate_command_buffers(uint32_t count) const;

    void wait_for_fence(vk::Fence fence) const
    {
        vk::resultCheck(m_device->waitForFences(fence, true, 1000000), "Wait for fences failed");
    }
    void wait_idle() const { m_device->waitIdle(); }
    void reset_fence(vk::Fence fence) const { m_device->resetFences(fence); }
    uint32_t acquire_swapchain_image(vk::SwapchainKHR swapchain, vk::Semaphore semaphore) const
    {
        return m_device->acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), semaphore).value;
    }

    vk::Queue get_present_queue() const noexcept { return m_device->getQueue(m_queue_info.present_family, 0); }
    vk::Queue get_graphics_queue() const noexcept { return m_device->getQueue(m_queue_info.graphics_family, 0); }
    vk::Extent2D get_window_extent() const noexcept { return m_window_extent; }
    vk::CommandPool get_command_pool() const noexcept { return *m_command_pool; }

  public:
    vk::PhysicalDeviceMemoryProperties memory_properties;

  private:
    void immediate_transfer(std::function<void(vk::CommandBuffer)> &&function) const;

    std::vector<const char *> m_required_instance_layers{};
    std::vector<const char *> m_required_instance_extensions{};
    std::vector<const char *> m_required_device_extensions{VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                                           VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::Extent2D m_window_extent;

    vk::UniqueInstance m_instance;
    vk::PhysicalDevice m_physical_device;

    vk::UniqueSurfaceKHR m_surface;

    QueueFamiliesInfo m_queue_info;
    vk::UniqueDevice m_device;

    vk::UniqueCommandPool m_command_pool;

    // TODO: abstract this into transfer queue class
    vk::Queue m_transfer_queue;
    vk::UniqueCommandPool m_transfer_pool;
    vk::UniqueCommandBuffer m_transfer_commands;
    vk::UniqueFence m_transfer_fence;
};
} // namespace ving
