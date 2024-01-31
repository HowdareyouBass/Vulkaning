#pragma once

#include <SDL3/SDL_video.h>

namespace ving
{
namespace utils
{

vk::PhysicalDevice pick_physical_device(std::span<vk::PhysicalDevice> devices);
uint32_t find_queue_family(std::span<vk::QueueFamilyProperties> queue_families, vk::QueueFlags flags);
uint32_t find_present_queue(std::span<vk::QueueFamilyProperties> queue_families, vk::PhysicalDevice physical_device,
                            vk::SurfaceKHR surface);

uint32_t find_memory_type(vk::PhysicalDeviceMemoryProperties mem_properties, uint32_t type_filter,
                          vk::MemoryPropertyFlags prop_flags);
int get_format_size(vk::Format format);

// Create
vk::UniqueInstance create_instance(std::string_view app_name, uint32_t app_version, std::string_view engine_name,
                                   uint32_t engine_version, std::span<const char *> layers,
                                   std::span<const char *> extensions);
vk::UniqueSurfaceKHR create_SDL_window_surface(SDL_Window *window, vk::Instance instance);
vk::UniqueDevice create_device(vk::PhysicalDevice device, std::span<vk::DeviceQueueCreateInfo> queue_infos,
                               std::span<const char *> extensions, vk::PhysicalDeviceFeatures2 features2);
std::pair<vk::UniqueSwapchainKHR, vk::Format> create_swapchain(vk::PhysicalDevice physical_device, vk::Device device,
                                                               vk::SurfaceKHR surface, vk::Extent2D extent,
                                                               uint32_t queue_family_count, uint32_t image_count);
vk::UniqueCommandPool create_command_pool(vk::Device device, uint32_t queue_family, vk::CommandPoolCreateFlags flags);
std::vector<vk::UniqueCommandBuffer> allocate_command_buffers(vk::Device device, vk::CommandPool pool, uint32_t count);
vk::UniqueFence create_fence(vk::Device device, vk::FenceCreateFlags flags);
vk::UniqueSemaphore create_semaphore(vk::Device device);

// Images
void transition_image(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout current_layout,
                      vk::ImageLayout new_layout);
void copy_image_to_image(vk::CommandBuffer cmd, vk::Image source, vk::Image dst, vk::Extent2D src_size,
                         vk::Extent2D dst_size);
} // namespace utils
} // namespace ving
