#pragma once

#include <SDL3/SDL_video.h>

namespace ving
{
namespace utils
{

vk::UniqueInstance create_instance(std::string_view app_name, uint32_t app_version, std::string_view engine_name,
                                   uint32_t engine_version, std::span<const char *> layers,
                                   std::span<const char *> extensions);
vk::UniqueSurfaceKHR create_SDL_window_surface(SDL_Window *window, vk::Instance instance);
vk::PhysicalDevice pick_physical_device(std::span<vk::PhysicalDevice> devices);

uint32_t find_queue_family(std::span<vk::QueueFamilyProperties> queue_families, vk::QueueFlags flags);
uint32_t find_present_queue(std::span<vk::QueueFamilyProperties> queue_families, vk::PhysicalDevice physical_device,
                            vk::SurfaceKHR surface);
vk::UniqueDevice create_device(vk::PhysicalDevice device, std::span<vk::DeviceQueueCreateInfo> queue_infos,
                               std::span<const char *> extensions);

} // namespace utils
} // namespace ving
