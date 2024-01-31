#include "ving_utils.hpp"

#include <SDL3/SDL_vulkan.h>

namespace ving
{
namespace utils
{
vk::UniqueInstance create_instance(std::string_view app_name, uint32_t app_version, std::string_view engine_name,
                                   uint32_t engine_version, std::span<const char *> layers,
                                   std::span<const char *> extensions)
{
    auto app_info = vk::ApplicationInfo{}
                        .setPApplicationName(app_name.data())
                        .setApplicationVersion(app_version)
                        .setPEngineName(engine_name.data())
                        .setEngineVersion(engine_version)
                        .setApiVersion(VK_API_VERSION_1_3);
    auto instance_create_info =
        vk::InstanceCreateInfo{}.setPApplicationInfo(&app_info).setPEnabledLayerNames(layers).setPEnabledExtensionNames(
            extensions);

    return vk::createInstanceUnique(instance_create_info);
}
vk::UniqueSurfaceKHR create_SDL_window_surface(SDL_Window *window, vk::Instance instance)
{

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), nullptr, &surface))
    {
        throw std::runtime_error(std::format("Failed to create vulkan surface{}", SDL_GetError()));
    }

    auto surface_deleter = vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>{instance};

    return vk::UniqueSurfaceKHR{surface, surface_deleter};
}
vk::PhysicalDevice pick_physical_device(std::span<vk::PhysicalDevice> devices)
{
    for (auto &&d : devices)
    {
        auto features2 = d.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
                                        vk::PhysicalDeviceVulkan12Features>();

        const vk::PhysicalDeviceVulkan13Features &features13 = features2.get<vk::PhysicalDeviceVulkan13Features>();

        if (features13.dynamicRendering)
        {
            return d;
        }
    }

    throw std::runtime_error("Could not find suitable Vulkan device");
}

// Return queue family index or queue_families.size() on fail
uint32_t find_queue_family(std::span<vk::QueueFamilyProperties> queue_families, vk::QueueFlags flags)
{
    auto queue_it = std::find_if(queue_families.begin(), queue_families.end(),
                                 [flags](vk::QueueFamilyProperties props) { return props.queueFlags | flags; });

    return std::distance(queue_families.begin(), queue_it);
}
uint32_t find_present_queue(std::span<vk::QueueFamilyProperties> queue_families, vk::PhysicalDevice physical_device,
                            vk::SurfaceKHR surface)
{
    uint32_t qsize = static_cast<uint32_t>(queue_families.size());

    for (uint32_t i = 0; i < qsize; ++i)
    {
        if (physical_device.getSurfaceSupportKHR(i, surface))
        {
            return i;
        }
    }
    return qsize;
}
vk::UniqueDevice create_device(vk::PhysicalDevice device, std::span<vk::DeviceQueueCreateInfo> queue_infos,
                               std::span<const char *> extensions)
{
    auto info = vk::DeviceCreateInfo{}.setPEnabledExtensionNames(extensions).setQueueCreateInfos(queue_infos);

    return device.createDeviceUnique(info);
}
vk::UniqueSwapchainKHR create_swapchain(vk::PhysicalDevice physical_device, vk::Device device, vk::SurfaceKHR surface,
                                        vk::Extent2D extent, uint32_t queue_family_count)
{
    std::vector<vk::SurfaceFormatKHR> formats = physical_device.getSurfaceFormatsKHR(surface);
    assert(!formats.empty());

    vk::Format format = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;

    vk::SurfaceCapabilitiesKHR surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);

    // HARD: Other present modes
    vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;

    uint32_t image_count = surface_capabilities.minImageCount + 1;

    auto info =
        vk::SwapchainCreateInfoKHR{}
            .setSurface(surface)
            .setMinImageCount(image_count)
            .setImageFormat(format)
            .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
            .setImageExtent(extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eTransferDst)
            .setQueueFamilyIndexCount(queue_family_count)
            .setPreTransform(surface_capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(present_mode)
            .setImageSharingMode(queue_family_count > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive);

    //.setImageSharingMode(sharing);

    return device.createSwapchainKHRUnique(info);
}
vk::UniqueCommandPool create_command_pool(vk::Device device, uint32_t queue_family, vk::CommandPoolCreateFlags flags)
{
    auto info = vk::CommandPoolCreateInfo{}.setQueueFamilyIndex(queue_family).setFlags(flags);

    return device.createCommandPoolUnique(info);
}
std::vector<vk::UniqueCommandBuffer> allocate_command_buffers(vk::Device device, vk::CommandPool pool, uint32_t count)
{
    // HARD: Secondary command buffers
    auto info = vk::CommandBufferAllocateInfo{}
                    .setLevel(vk::CommandBufferLevel::ePrimary)
                    .setCommandPool(pool)
                    .setCommandBufferCount(count);

    return device.allocateCommandBuffersUnique(info);
}
vk::UniqueFence create_fence(vk::Device device, vk::FenceCreateFlags flags)
{
    auto info = vk::FenceCreateInfo{}.setFlags(flags);

    return device.createFenceUnique(info);
}
vk::UniqueSemaphore create_semaphore(vk::Device device)
{
    auto info = vk::SemaphoreCreateInfo{};

    return device.createSemaphoreUnique(info);
}
} // namespace utils
} // namespace ving
