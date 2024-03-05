#pragma once

#include "vk_types.hpp"

#include <filesystem>

#include <SDL3/SDL_video.h>

namespace ving
{
class Image2D;
class Core;

namespace utils
{
// Core Creation
vk::PhysicalDevice pick_physical_device(std::span<vk::PhysicalDevice> devices);
uint32_t find_queue_family(std::span<vk::QueueFamilyProperties> queue_families, vk::QueueFlags flags);
uint32_t find_present_queue(std::span<vk::QueueFamilyProperties> queue_families, vk::PhysicalDevice physical_device,
                            vk::SurfaceKHR surface);

// Pipeline creation
uint32_t find_memory_type(vk::PhysicalDeviceMemoryProperties mem_properties, uint32_t type_filter,
                          vk::MemoryPropertyFlags prop_flags);
int get_format_size(vk::Format format);
std::vector<uint32_t> read_shader_file(std::filesystem::path path);

// Create
vk::UniqueInstance create_instance(std::string_view app_name, uint32_t app_version, std::string_view engine_name,
                                   uint32_t engine_version, std::span<const char *> layers,
                                   std::span<const char *> extensions);
vk::UniqueSurfaceKHR create_SDL_window_surface(SDL_Window *window, vk::Instance instance);
vk::UniqueDevice create_device(vk::PhysicalDevice device, std::span<vk::DeviceQueueCreateInfo> queue_infos,
                               std::span<const char *> extensions, vk::PhysicalDeviceFeatures2 features2);

[[deprecated]] std::pair<vk::UniqueSwapchainKHR, vk::Format> create_swapchain_old(
    vk::PhysicalDevice physical_device, vk::Device device, vk::SurfaceKHR surface, vk::Extent2D extent,
    uint32_t queue_family_count, uint32_t image_count);

vktypes::Swapchain create_swapchain(vk::PhysicalDevice physical_device, vk::Device device, vk::SurfaceKHR surface,
                                    vk::Extent2D extent, uint32_t queue_family_count, uint32_t image_count);
std::vector<vk::UniqueImageView> create_image_views(vk::Device device, std::span<vk::Image> images, vk::Format format);

vk::UniqueCommandPool create_command_pool(vk::Device device, uint32_t queue_family, vk::CommandPoolCreateFlags flags);
std::vector<vk::UniqueCommandBuffer> allocate_command_buffers(vk::Device device, vk::CommandPool pool, uint32_t count);
vk::UniqueFence create_fence(vk::Device device, vk::FenceCreateFlags flags);
vk::UniqueSemaphore create_semaphore(vk::Device device);
vk::UniqueShaderModule create_shader_module(vk::Device device, std::string_view shader_path);

// Images
void transition_image(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout current_layout,
                      vk::ImageLayout new_layout);
void copy_image_to_image(vk::CommandBuffer cmd, vk::Image source, vk::Image dst, vk::Extent2D src_size,
                         vk::Extent2D dst_size);
Image2D load_cube_map(std::string_view filepath, const Core &core);

// Other

// Taken from SaschaWillems Vulkan-Samples repository
inline uint32_t aligned_size(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

} // namespace utils
} // namespace ving
